#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "request_handler.h"

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

#endif // STATIC_FILE_HANDLER_H