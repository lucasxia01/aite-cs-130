#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "request.h"
#include "response.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
class RequestHandler {
public:
  RequestHandler() {}
  virtual response
  generate_response(const http::server3::request &req) const = 0;
  std::optional<std::string> get_root_from_uri(std::string uri) const;

private:
  virtual bool has_root(std::string root) const = 0;
};

class EchoRequestHandler : public RequestHandler {
public:
  EchoRequestHandler(std::set<std::string> roots) : roots(roots) {}
  response generate_response(const http::server3::request &req) const;

private:
  std::set<std::string> roots;
  bool has_root(std::string root) const;
};

class StaticFileRequestHandler : public RequestHandler {
public:
  StaticFileRequestHandler(std::map<std::string, std::string> root_to_base_dir)
      : root_to_base_dir(root_to_base_dir) {}
  response generate_response(const http::server3::request &req) const;

private:
  std::map<std::string, std::string> root_to_base_dir;

  bool has_root(std::string root) const;
  std::string parse_path_from_uri(std::string uri, std::string root) const;
  std::string parse_content_type_from_uri(std::string uri,
                                          std::string root) const;
};

#endif // REQUEST_HANDLER_H