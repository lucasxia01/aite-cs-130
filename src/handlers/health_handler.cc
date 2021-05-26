#include "health_handler.h"

HealthRequestHandler::HealthRequestHandler(const std::string &location,
                                           const NginxConfig &config)
    : parent_server_(nullptr) {}
void HealthRequestHandler::initHealth(server *parent_server) {
  parent_server_ = parent_server;
}

http::response
HealthRequestHandler::handle_request(const http::request &req) const {
  std::ostringstream ss;
  std::ostringstream temp;

  if (parent_server_ == nullptr) {
    LOG_FATAL << "Health Handler was not correctly initialized";
    return show_error_page(http::status::internal_server_error,
                           "Health Handler was not correctly initialized");
  } else {
    ss << "OK";
  }
  std::string response_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/plain");
  resp.body() = response_content;
  resp.prepare_payload();
  return resp;
}
