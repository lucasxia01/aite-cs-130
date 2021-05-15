#include "request_handler.h"

http::response
EchoRequestHandler::handle_request(const http::request &req) const {
  std::ostringstream ss;
  ss << req;
  std::string request_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/plain");
  resp.content_length(request_content.length());
  resp.body() = request_content;
  return resp;
}

// shows error page if uri path does not match any of the other request handler
// serving paths
http::response
NotFoundRequestHandler::handle_request(const http::request &req) const {
  return show_error_page(http::status::not_found,
                         "Uri path " + std::string(req.target()) +
                             " not found");
}

StaticFileRequestHandler::StaticFileRequestHandler(const std::string &location,
                                                   const NginxConfig &config)
    : location(location) {
  std::vector<std::string> values = configLookup(config, {}, "root");
  if (values.size() != 1) {
    LOG_FATAL << "Invalid number of root specifiers for " << location
              << ", only one root should be listed per location";
  }

  this->set_root_path(values[0]);
}
void StaticFileRequestHandler::set_root_path(std::string root_path) {
  root = convertToAbsolutePath(root_path);
  std::filesystem::path p(root);
  std::error_code err;

  if (std::filesystem::is_directory(p, err)) {
    root = (root.back() != '/') ? root + "/" : root;
  }
  if (err) {
    LOG_ERROR << "Error checking if path " << root << " is a directory";
  }
}
http::response
StaticFileRequestHandler::handle_request(const http::request &req) const {
  std::string http_uri(req.target());
  std::string content_type = this->parse_content_type_from_uri(http_uri);
  std::string path = this->parse_path_from_uri(http_uri);
  std::ifstream ifs(path, std::ifstream::in);

  if (ifs.fail()) {

    return show_error_page(http::status::not_found,
                           "File " + http_uri + " not found");
  }
  std::stringstream ss;
  char c;
  while (ifs.get(c)) {
    ss << c;
  }
  std::string static_file_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, content_type);
  resp.content_length(static_file_content.length());
  resp.body() = static_file_content;
  return resp;
}

/**
 * Parse relative file path of requested file
 **/
std::string
StaticFileRequestHandler::parse_path_from_uri(std::string uri) const {
  boost::filesystem::path p(uri), loc(location);
  std::string file_extension = p.extension().string();
  std::string prefix = root;
  return prefix + boost::filesystem::relative(p, loc).string();
}

/**
 * Parse HTML content type of requested file
 **/
std::string
StaticFileRequestHandler::parse_content_type_from_uri(std::string uri) const {
  boost::filesystem::path p(uri);
  std::string file_extension = p.extension().string();
  if (file_extension == ".txt")
    return "text/plain";
  else if (file_extension == ".html")
    return "text/html";
  else if (file_extension == ".jpeg")
    return "image/jpeg";
  else if (file_extension == ".png")
    return "image/png";
  else if (file_extension == ".zip")
    return "application/zip";
  else
    return "text/plain";
}

ReverseProxyRequestHandler::ReverseProxyRequestHandler(
    const std::string &location, const NginxConfig &config)
    : location(location) {
  std::vector<std::string> host_values = configLookup(config, {}, "host");
  if (host_values.size() != 1) {
    LOG_FATAL << "Invalid number of host specifiers for " << location
              << ", only one host should be listed per location";
  }
  host = host_values[0];

  std::vector<std::string> port_values = configLookup(config, {}, "port");
  if (port_values.size() != 1) {
    LOG_FATAL << "Invalid number of port specifiers for " << location
              << ", only one port should be listed per location";
  }
  port = port_values[0];

  std::vector<std::string> max_depth_values =
      configLookup(config, {}, "max_depth");
  try {
    max_depth = stoi(max_depth_values[0]);
  } catch (std::exception &e) {
    LOG_FATAL << "Invalid max depth value for " << location << ": "
              << max_depth_values[0];
  }
}

http::response
ReverseProxyRequestHandler::handle_request(const http::request &req) const {
  // Create modifiable copy of request.
  http::request proxied_request(req);
  // Remove proxy base path from request path.
  auto proxied_target = std::string(req.target()).substr(location.length());
  if (proxied_target.length() == 0) {
    // If the remaining path is empty, request the root.
    proxied_target = "/";
  }
  proxied_request.target(proxied_target);
  // Change Host header to proxy destination.
  proxied_request.set(http::field::host, host + ":" + port);
  // Only allow plaintext response so we can manipulate the body.
  proxied_request.set(http::field::accept_encoding, "identity");
  http::response resp =
      http_client_->perform_request(host, port, proxied_request);

  auto location_hdr = resp.find(http::field::location);
  int status_code = resp.result_int();
  int redirects_taken = 0;
  while (location_hdr != resp.end() && status_code >= 300 &&
         status_code < 400 && redirects_taken < max_depth) {
    auto redirect = std::string((*location_hdr).value());
    std::string redirect_host = host + ":" + port;
    if (redirect.rfind('/', 0) == 0) {
      // Prepend proxy path to redirect path.
      proxied_target = location + redirect;
    } else {
      // We only support http requests
      // If we encounter a non-http redirect location, return the 30X
      // response without processing it
      if (redirect.substr(0, 7) != "http://") {
        prepare_proxied_resp(resp);
        return resp;
      }
      redirect_host = parse_host_from_http_url(redirect);
      proxied_target = parse_target_from_http_url(redirect);
    }

    LOG_DEBUG << "Redirected to: " << redirect;

    proxied_request.target(proxied_target);
    proxied_request.set(http::field::host, redirect_host + ":" + port);
    proxied_request.set(http::field::accept_encoding, "identity");

    resp = http_client_->perform_request(redirect_host, port, proxied_request);
    location_hdr = resp.find(http::field::location);
    status_code = resp.result_int();
    redirects_taken++;
  }

  prepare_proxied_resp(resp);
  return resp;
}

std::string ReverseProxyRequestHandler::replaced_absolute_paths(
    const std::string &body, const std::string &prefix,
    const std::string &separator) const {
  std::string new_body(body);
  const std::string from = prefix + separator;
  const std::string to = prefix + location + separator;
  size_t pos = 0;
  while ((pos = new_body.find(from, pos)) != std::string::npos) {
    // Skip double-slashes (e.g. //www.washington.edu/)
    size_t suffix = pos + from.length();
    if (new_body.length() > suffix && new_body[suffix] == '/') {
      pos += from.length();
      continue;
    }
    new_body.replace(pos, from.length(), to);
    pos += to.length();
  }
  return new_body;
}

std::string ReverseProxyRequestHandler::parse_host_from_http_url(
    const std::string &url) const {
  std::string method_removed_url = url.substr(7); // Omit "http://"
  size_t pos = method_removed_url.find("/");
  if (pos != std::string::npos) {
    return method_removed_url.substr(0, pos);
  }
  return method_removed_url;
}

std::string ReverseProxyRequestHandler::parse_target_from_http_url(
    const std::string &url) const {
  std::string method_removed_url = url.substr(7); // Omit "http://"
  size_t pos = method_removed_url.find("/");
  if (pos != std::string::npos) {
    return method_removed_url.substr(pos);
  }
  return "/";
}

void ReverseProxyRequestHandler::prepare_proxied_resp(
    http::response &resp) const {
  resp.chunked(false);
  // Rewrite common absolute paths in body to work with proxy.
  std::string body = resp.body();
  body = replaced_absolute_paths(body, "href=\"", "/");
  body = replaced_absolute_paths(body, "src=\"", "/");
  body = replaced_absolute_paths(body, "url(\"", "/");
  body = replaced_absolute_paths(body, "url(", "\\2f ");
  resp.body() = body;
  resp.prepare_payload();
}

DummyRequestHandler::DummyRequestHandler(const std::string &location)
    : location(location){};

std::string DummyRequestHandler::get_location() const { return location; }

http::response
DummyRequestHandler::handle_request(const http::request &req) const {
  http::response resp;
  return resp;
}