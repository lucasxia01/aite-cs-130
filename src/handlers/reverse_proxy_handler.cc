#include "request_handler.h"
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