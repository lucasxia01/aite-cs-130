#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "response.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <map>
#include <request.h>
#include <set>
#include <sstream>
#include <string>

class RequestHandler {

public:
  RequestHandler() {}
  virtual response generate_response(response::status_type status,
                                     const http::server3::request &req) = 0;

  static void parse_uri(std::string uri);
};

class EchoRequestHandler : public RequestHandler {
  std::set<std::string> roots;

public:
  EchoRequestHandler(std::set<std::string> roots) : roots(roots) {}
  bool get_root(std::string uri, std::string &root);
  response generate_response(response::status_type status,
                             const http::server3::request &req);
};

class StaticFileRequestHandler : public RequestHandler {

public:
  StaticFileRequestHandler(std::map<std::string, std::string> root_to_base_dir)
      : root_to_base_dir(root_to_base_dir) {}
  bool get_root(std::string uri, std::string &root);
  response generate_response(response::status_type status,
                             const http::server3::request &req);

private:
  std::map<std::string, std::string> root_to_base_dir;

  void parse_uri(std::string uri, std::string root, std::string &path,
                 std::string &content_type);
};

#endif // REQUEST_HANDLER_H