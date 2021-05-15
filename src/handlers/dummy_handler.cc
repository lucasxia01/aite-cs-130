#include "request_handler.h"

DummyRequestHandler::DummyRequestHandler(const std::string &location)
    : location(location){};

std::string DummyRequestHandler::get_location() const { return location; }

http::response
DummyRequestHandler::handle_request(const http::request &req) const {
  http::response resp;
  return resp;
}