#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <algorithm>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <map>
#include <math.h>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "http_client.h"
#include "logger.h"
#include "server.h"
#include "utils.h"

namespace beast = boost::beast;
namespace http = beast::http;

class server;

/**
 * Abstract base class for request handlers
 */
class RequestHandler {
public:
  RequestHandler() {}
  virtual http::response handle_request(const http::request &req) const = 0;
};

/**
 * Handler to echo requests back to client
 */
class EchoRequestHandler : public RequestHandler {
public:
  EchoRequestHandler(const std::string &location, const NginxConfig &config){};
  /**
   * echoes http request to client
   *
   * @param req http request from client
   * @return http response with echoed http request as body
   */
  http::response handle_request(const http::request &req) const;
};

/**
 * Handler that handles 404 not found errors
 */
class NotFoundRequestHandler : public RequestHandler {
public:
  NotFoundRequestHandler(const std::string &location,
                         const NginxConfig &config) {}
  /**
   * displays error page for uri paths not handled by other request handlers
   *
   * @param req http request from client
   * @return http response with 404 error pages as body
   */
  http::response handle_request(const http::request &req) const;
};

/**
 * Handler that serves client the requested static files
 */
class StaticFileRequestHandler : public RequestHandler {
public:
  StaticFileRequestHandler(const std::string &location,
                           const NginxConfig &config);
  /**
   * serves requested static file to client
   *
   * @param req http request from client
   * @return http response with static file as body
   */
  http::response handle_request(const http::request &req) const;

private:
  /**
   * find the corresponding MIME type from the request uri based on file
   * extension
   *
   * @param uri uri from client's http request
   * @return string with correponding MIME type
   */
  std::string parse_content_type_from_uri(std::string uri) const;
  /**
   *  constructs file system path given a request uri
   *
   * @param uri uri from client's http reqeust
   * @return path of file to serve
   */
  std::string parse_path_from_uri(std::string uri) const;
  void set_root_path(std::string location);

  std::string root; // corresponding base directory specified from config file
  const std::string location; // serving path of this request handler
};

/**
 *  Handler that serves as a proxy between other HTTP resources
 */
class ReverseProxyRequestHandler : public RequestHandler {
public:
  ReverseProxyRequestHandler(const std::string &location,
                             const NginxConfig &config);

  ReverseProxyRequestHandler(const std::string &location,
                             const NginxConfig &config,
                             std::unique_ptr<HttpClient> client);
  /**
   * serves requested static file to client
   *
   * @param req http request from client
   * @return http response with resource fetched from proxy destination
   */
  http::response handle_request(const http::request &req) const;

private:
  std::string replaced_absolute_paths(const std::string &body,
                                      const std::string &prefix,
                                      const std::string &separator) const;
  std::string parse_host_from_http_url(const std::string &url) const;
  std::string parse_target_from_http_url(const std::string &url) const;
  void prepare_proxied_resp(http::response &resp) const;

  std::unique_ptr<HttpClient> http_client_;
  const std::string location;
  std::string host;
  std::string port;
};
/*
 * handler that displays information on the status of the webserver
 */
class StatusRequestHandler : public RequestHandler {
public:
  StatusRequestHandler(const std::string &location, const NginxConfig &config);
  void initStatus(server *parent_server);
  http::response handle_request(const http::request &req) const;

private:
  server *parent_server_;
};

/**
 * Handler that's soley used for testing purposes
 */
class DummyRequestHandler : public RequestHandler {
public:
  DummyRequestHandler(const std::string &location);
  http::response handle_request(const http::request &req) const;

  std::string get_location() const;

private:
  std::string location;
};

#endif // REQUEST_HANDLER_H