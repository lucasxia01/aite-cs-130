#include "request_handler.h"

/*
 * handler that displays information on the status of the webserver
 */
class StatusRequestHandler : public RequestHandler {
public:
  StatusRequestHandler(const std::string &location, const NginxConfig &config);
  void initStatus(server *parent_server);
  http::response handle_request(const http::request &req) const;

private:
  server *parent_server_;
};