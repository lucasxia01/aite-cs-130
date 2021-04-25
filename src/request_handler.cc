#include "request_handler.h"
#include <map>
#include <string>

response
EchoRequestHandler::generateResponse(response::status_type status,
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

response
StaticFileRequestHandler::generateResponse(response::status_type status,
                                           const http::server3::request &req) {
  std::string root, path, content_type;
  StaticFileRequestHandler::parseUri(req.uri, root, path, content_type);

  std::ifstream ifs(path, std::ifstream::in);
  if (ifs.fail())
    return response::getStockResponse(response::BAD_REQUEST);

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

void StaticFileRequestHandler::parseUri(std::string uri, std::string &root,
                                        std::string &path,
                                        std::string &content_type) {
  size_t pos = uri.find("/", 1);
  root = uri.substr(0, pos);
  path = root_to_base_dir[root] + uri.substr(pos);

  std::string file_extension = uri.substr(uri.rfind(".") + 1);
  if (file_extension == "txt")
    content_type = "text/plain";
  else if (file_extension == "html")
    content_type = "text/html";
  else if (file_extension == "jpeg")
    content_type = "image/jpeg";
  else if (file_extension == "png")
    content_type = "image/png";
  else if (file_extension == "zip")
    content_type = "application/zip";
  else
    content_type = "text/plain";
}
