#include "request_handler.h"
#include <algorithm>

/**
 * Check if uri is a valid path containing a valid root, and if so return true
 * and set root to the parsed value
 **/
bool RequestHandler::get_root_from_uri(std::string uri, std::string &root) {
  boost::filesystem::path p(uri);
  root = p.string();

  // look through all path prefixes and check if any match a valid root
  // prioritizing longest path prefix
  while (p.has_parent_path()) {
    if (this->has_root(p.string())) {
      root = p.string();
      return true;
    }
    p = p.parent_path();
  }

  return false;
}

response
EchoRequestHandler::generate_response(const http::server3::request &req) {
  std::string response_body = req.raw_header_str + req.raw_body_str;
  response resp;
  resp.status = response::OK;
  resp.headers.push_back({"Content-Type", "text/plain"});
  resp.headers.push_back(
      {"Content-Length", std::to_string(response_body.length())});
  resp.body = response_body;
  return resp;
}

bool EchoRequestHandler::has_root(std::string root) {
  return roots.find(root) != roots.end();
}

response
StaticFileRequestHandler::generate_response(const http::server3::request &req) {
  std::string root, path, content_type;
  bool suc = this->get_root_from_uri(req.uri, root);
  if (!suc)
    return response::get_stock_response(response::BAD_REQUEST);

  this->parse_uri(req.uri, root, path, content_type);
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

bool StaticFileRequestHandler::has_root(std::string root) {
  return root_to_base_dir.find(root) != root_to_base_dir.end();
}

/**
 * Get relative file path and HTML content type of requested file
 **/
void StaticFileRequestHandler::parse_uri(std::string uri, std::string root,
                                         std::string &path,
                                         std::string &content_type) {
  boost::filesystem::path p(uri), root_path(root);
  std::string file_extension = p.extension().string();
  path = root_to_base_dir[root] +
         boost::filesystem::relative(p, root_path).string();

  if (file_extension == ".txt")
    content_type = "text/plain";
  else if (file_extension == ".html")
    content_type = "text/html";
  else if (file_extension == ".jpeg")
    content_type = "image/jpeg";
  else if (file_extension == ".png")
    content_type = "image/png";
  else if (file_extension == ".zip")
    content_type = "application/zip";
  else
    content_type = "text/plain";
}
