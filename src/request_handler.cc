#include "request_handler.h"

/**
 * Check if uri is a valid path containing a valid root, and if so return true
 * and set root to the parsed value
 **/
std::optional<std::string>
RequestHandler::get_root_from_uri(std::string uri) const {
  boost::filesystem::path p(uri);
  // look through all path prefixes and check if any match a valid root
  // prioritizing longest path prefix
  while (p.has_parent_path()) {
    if (this->has_root(p.string())) {
      return p.string();
    }
    p = p.parent_path();
  }
  return std::nullopt;
}

http::response RequestHandler::get_stock_response(http::status status_code) {
  std::string response_body;
  switch (status_code) {
  case http::status::ok:
    response_body = "";
    break;
  case http::status::bad_request:
    response_body = "Invalid request\n";
    break;
  case http::status::not_found:
    response_body = "File not found\n";
    break;
  default:
    response_body = "";
    break;
  }

  http::response resp;
  resp.result(status_code);
  resp.set(http::field::content_type, "text/plain");
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

bool EchoRequestHandler::has_root(std::string root) const {
  return roots.find(root) != roots.end();
}

http::response
StaticFileRequestHandler::handle_request(const http::request &req) const {
  std::string http_uri(req.target());
  std::optional<std::string> root_opt = this->get_root_from_uri(http_uri);
  if (!root_opt) {
    return RequestHandler::get_stock_response(http::status::bad_request);
  }

  std::string content_type =
      this->parse_content_type_from_uri(http_uri, root_opt.value());
  std::string path = this->parse_path_from_uri(http_uri, root_opt.value());
  std::ifstream ifs(path, std::ifstream::in);
  if (ifs.fail()) {
    return RequestHandler::get_stock_response(http::status::not_found);
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

bool StaticFileRequestHandler::has_root(std::string root) const {
  return root_to_base_dir.find(root) != root_to_base_dir.end();
}

/**
 * Parse relative file path of requested file
 **/
std::string
StaticFileRequestHandler::parse_path_from_uri(std::string uri,
                                              std::string root) const {
  boost::filesystem::path p(uri), root_path(root);
  std::string file_extension = p.extension().string();
  auto it = root_to_base_dir.find(root);
  std::string prefix = it == root_to_base_dir.end() ? "" : it->second;
  return prefix + boost::filesystem::relative(p, root_path).string();
}

/**
 * Parse HTML content type of requested file
 **/
std::string
StaticFileRequestHandler::parse_content_type_from_uri(std::string uri,
                                                      std::string root) const {
  boost::filesystem::path p(uri), root_path(root);
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