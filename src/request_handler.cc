#include "request_handler.h"
#include <algorithm>

response
EchoRequestHandler::generate_response(response::status_type status,
                                      const http::server3::request &req) {
  std::string response_body = req.raw_header_str + req.raw_body_str;
  response resp;
  resp.status = response::OK;
  resp.headers.push_back({"Content-Type", "text/plain"});
  resp.headers.push_back(
      {"Content-Length", std::to_string(response_body.length())});
  resp.body = response_body;
  return resp;
}

/**
 * Check if uri is a valid path containing a valid static file root, and if so
 * return true and set root to the parsed value
 **/
bool EchoRequestHandler::get_root(std::string uri, std::string &root) {
  boost::filesystem::path p(uri);
  root = p.string();

  // look through all path prefixes and check if any match a valid root
  // prioritizing longest path prefix
  while (p.has_parent_path()) {
    if (roots.find(p.string()) != roots.end()) {
      root = p.string();
      return true;
    }
    p = p.parent_path();
  }

  return false;
}

response
StaticFileRequestHandler::generate_response(response::status_type status,
                                            const http::server3::request &req) {
  std::string root, path, content_type;
  StaticFileRequestHandler::get_root(req.uri, root);
  StaticFileRequestHandler::parse_uri(req.uri, root, path, content_type);

  std::ifstream ifs(path, std::ifstream::in);
  if (ifs.fail())
    return response::get_stock_response(response::NOT_FOUND);

  char c = ifs.get();
  std::stringstream ss;
  while (ifs.good()) {
    ss << c;
    c = ifs.get();
  }
  std::string response_body = ss.str();

  response resp;
  resp.status = response::OK;
  resp.headers.push_back({"Content-Type", content_type});
  resp.headers.push_back(
      {"Content-Length", std::to_string(response_body.length())});
  resp.body = response_body;
  return resp;
}

/**
 * Check if uri is a valid path containing a valid static file root, and if so
 * return true and set root to the parsed value
 **/
bool StaticFileRequestHandler::get_root(std::string uri, std::string &root) {
  boost::filesystem::path p(uri);
  root = p.string();

  // look through all path prefixes and check if any match a valid root
  // prioritizing longest path prefix
  while (p.has_parent_path()) {
    if (root_to_base_dir.find(p.string()) != root_to_base_dir.end()) {
      root = p.string();
      return true;
    }
    p = p.parent_path();
  }

  return false;
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
