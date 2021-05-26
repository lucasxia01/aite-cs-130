#ifndef DUMMY_HANDLER_H
#define DUMMY_HANDLER_H

#include "request_handler.h"

/**
 * Handler that's solely used for testing purposes
 */
class DummyRequestHandler : public RequestHandler {
public:
  DummyRequestHandler(const std::string &location);
  http::response handle_request(const http::request &req) const;

  std::string get_location() const;

private:
  std::string location;
};

#endif // DUMMY_HANDLER_H