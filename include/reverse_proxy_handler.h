#ifndef REVERSE_PROXY_HANDLER_H
#define REVERSE_PROXY_HANDLER_H

#include "http_client.h"
#include "request_handler.h"

/**
 *  Handler that serves as a proxy between other HTTP resources
 */
class ReverseProxyRequestHandler : public RequestHandler {
public:
  ReverseProxyRequestHandler(const std::string &location,
                             const NginxConfig &config);

  ReverseProxyRequestHandler(const std::string &location,
                             const NginxConfig &config,
                             std::unique_ptr<HttpClient> client);
  /**
   * serves requested static file to client
   *
   * @param req http request from client
   * @return http response with resource fetched from proxy destination
   */
  http::response handle_request(const http::request &req) const;

private:
  std::string replaced_absolute_paths(const std::string &body,
                                      const std::string &prefix,
                                      const std::string &separator) const;
  std::string parse_host_from_http_url(const std::string &url) const;
  std::string parse_target_from_http_url(const std::string &url) const;
  void prepare_proxied_resp(http::response &resp) const;

  std::unique_ptr<HttpClient> http_client_;
  const std::string location;
  std::string host;
  std::string port;
};

#endif // REVERSE_PROXY_HANDLER_H
