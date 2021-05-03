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

response
EchoRequestHandler::generate_response(const http::server3::request &req) const {
  std::string response_body = req.raw_header_str + req.raw_body_str;
  response resp;
  resp.status = response::OK;
  resp.headers.push_back({"Content-Type", "text/plain"});
  resp.headers.push_back(
      {"Content-Length", std::to_string(response_body.length())});
  resp.body = response_body;
  return resp;
}

bool EchoRequestHandler::has_root(std::string root) const {
  return roots.find(root) != roots.end();
}

response StaticFileRequestHandler::generate_response(
    const http::server3::request &req) const {
  std::optional<std::string> root_opt = this->get_root_from_uri(req.uri);
  if (!root_opt) {
    return response::get_stock_response(response::BAD_REQUEST);
  }
  std::string content_type =
      this->parse_content_type_from_uri(req.uri, root_opt.value());
  std::string path = this->parse_path_from_uri(req.uri, root_opt.value());
  std::ifstream ifs(path, std::ifstream::in);
  if (ifs.fail())
    return response::get_stock_response(response::NOT_FOUND);

  std::stringstream ss;
  char c;
  while (ifs.get(c)) {
    ss << c;
  }
  std::string static_file_content = ss.str();

  response resp;
  resp.status = response::OK;
  resp.headers.push_back({"Content-Type", content_type});
  resp.headers.push_back(
      {"Content-Length", std::to_string(static_file_content.length())});
  resp.body = static_file_content;
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