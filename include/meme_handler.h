#ifndef NEME_HANDLER_H
#define NEME_HANDLER_H

#include "request_handler.h"

/**
 * Handler that's used for testing multithreading
 */
class MemeGenHandler : public RequestHandler {
public:
  MemeGenHandler() {}
  http::response handle_request(const http::request &req) const;
};

#endif // NEME_HANDLER_H