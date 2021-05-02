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

  virtual bool has_root(std::string root) = 0;

public:
  RequestHandler() {}
  virtual response generate_response(const http::server3::request &req) = 0;
  bool get_root_from_uri(std::string uri, std::string &root);
};

class EchoRequestHandler : public RequestHandler {
public:
  EchoRequestHandler(std::set<std::string> roots) : roots(roots) {}
  response generate_response(const http::server3::request &req);

private:
  std::set<std::string> roots;
  bool has_root(std::string root);
};

class StaticFileRequestHandler : public RequestHandler {
public:
  StaticFileRequestHandler(std::map<std::string, std::string> root_to_base_dir)
      : root_to_base_dir(root_to_base_dir) {}
  response generate_response(const http::server3::request &req);

private:
  std::map<std::string, std::string> root_to_base_dir;

  bool has_root(std::string root);
  void parse_uri(std::string uri, std::string root, std::string &path,
                 std::string &content_type);
};

#endif // REQUEST_HANDLER_H