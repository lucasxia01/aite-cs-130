#ifndef SLEEP_HANDLER_H
#define SLEEP_HANDLER_H

#include "request_handler.h"

/**
 * Handler that's used for testing multithreading
 */
class SleepHandler : public RequestHandler {
public:
  SleepHandler() {}
  http::response handle_request(const http::request &req) const;
};

#endif // SLEEP_HANDLER_H