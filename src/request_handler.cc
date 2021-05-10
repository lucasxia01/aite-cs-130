#include "request_handler.h"

http::response RequestHandler::show_error_page(http::status status_code,
                                               std::string message) {
  std::ostringstream ss;
  ss << "<!DOCTYPE html><html>"
     << "<head><title>Error</title></head><body>"
     << "<h1>" << status_code << " Error"
     << "</h1>"
     << "<p>"
     << "Description: " << message << "</p>"
     << "</body></html>";
  std::string response_body = ss.str();

  http::response resp;
  resp.result(status_code);
  resp.set(http::field::content_type, "text/html");
  resp.content_length(response_body.length());
  resp.body() = response_body;
  return resp;
}

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
  return RequestHandler::show_error_page(
      http::status::not_found,
      "Uri path " + std::string(req.target()) + " not found");
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
  // remove any enclosing quotes
  root = (root_path[0] == '"') ? root_path.substr(1, root_path.length() - 2)
                               : root_path;
  root = convertToAbsolutePath(root_path);
  std::filesystem::path p(root);
  std::error_code err;

  if (std::filesystem::is_directory(p, err)) {
    root = (root.back() != '/') ? root + "/" : root;
  }
  if (err) {
    LOG_ERROR << "Error checking if path is a directory";
  }
}
http::response
StaticFileRequestHandler::handle_request(const http::request &req) const {
  std::string http_uri(req.target());
  std::string content_type = this->parse_content_type_from_uri(http_uri);
  std::string path = this->parse_path_from_uri(http_uri);
  std::ifstream ifs(path, std::ifstream::in);

  if (ifs.fail()) {

    return RequestHandler::show_error_page(http::status::not_found,
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

DummyRequestHandler::DummyRequestHandler(const std::string &location)
    : location(location){};

std::string DummyRequestHandler::get_location() const { return location; }

http::response
DummyRequestHandler::handle_request(const http::request &req) const {
  http::response resp;
  return resp;
}
std::string RequestHandler::get_location() const { return ""; }