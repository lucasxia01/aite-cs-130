#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <algorithm>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <math.h>
#include <sstream>
#include <string>

#include "logger.h"
#include "server.h"
#include "utils.h"

class server;

/**
 * Abstract base class for request handlers
 */
class RequestHandler {
public:
  RequestHandler() {}
  virtual http::response handle_request(const http::request &req) const = 0;
};

#endif // REQUEST_HANDLER_H