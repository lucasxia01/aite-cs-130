#include "request_handler.h"
http::response
EchoRequestHandler::handle_request(const http::request &req) const {
  std::ostringstream ss;
  ss << req;
  std::string request_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/plain");
  resp.content_length(request_content.length());
  resp.body() = request_content;
  return resp;
}