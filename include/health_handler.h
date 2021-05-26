#ifndef HEALTH_HANDLER_H
#define HEALTH_HANDLER_H

#include "request_handler.h"

/*
 * Handler that pings health status
 */
class HealthRequestHandler : public RequestHandler {
public:
  HealthRequestHandler(const std::string &location, const NginxConfig &config);
  void initHealth(server *parent_server);
  http::response handle_request(const http::request &req) const;

private:
  server *parent_server_;
};

#endif // HEALTH_HANDLER_H