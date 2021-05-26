#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include "request_handler.h"

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

#endif // NOT_FOUND_HANDLER_H