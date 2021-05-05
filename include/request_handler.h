#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "config_parser.h"

#include <algorithm>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;

#define response response<http::string_body>
#define request request<http::string_body>

class RequestHandler {
public:
  RequestHandler() {}

  virtual http::response handle_request(const http::request &req) const = 0;

  std::optional<std::string> get_root_from_uri(std::string uri) const;

  static http::response get_stock_response(http::status status_code);

private:
  virtual bool has_root(std::string root) const = 0;
};

class EchoRequestHandler : public RequestHandler {
public:
  EchoRequestHandler(std::set<std::string> roots) : roots(roots) {}
  http::response handle_request(const http::request &req) const;

private:
  std::set<std::string> roots;
  bool has_root(std::string root) const;
};

class StaticFileRequestHandler : public RequestHandler {
public:
  StaticFileRequestHandler(std::map<std::string, std::string> root_to_base_dir)
      : root_to_base_dir(root_to_base_dir) {}

  http::response handle_request(const http::request &req) const;

private:
  std::map<std::string, std::string> root_to_base_dir;

  bool has_root(std::string root) const;

  std::string parse_path_from_uri(std::string uri, std::string root) const;

  std::string parse_content_type_from_uri(std::string uri,
                                          std::string root) const;
};

#endif // REQUEST_HANDLER_H