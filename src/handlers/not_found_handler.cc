#include "not_found_handler.h"
// shows error page if uri path does not match any of the other request handler
// serving paths
http::response
NotFoundRequestHandler::handle_request(const http::request &req) const {
  return show_error_page(http::status::not_found,
                         "Uri path " + std::string(req.target()) +
                             " not found");
}