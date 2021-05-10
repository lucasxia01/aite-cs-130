#ifndef SERVER_H
#define SERVER_H

#include "logger.h"
#include "request_handler.h"
#include "session.h"
#include "tcp_socket_wrapper.h"
#include <boost/asio.hpp>
#include <map>
#include <optional>
#include <set>
#include <string>

using namespace boost::asio::ip;

template <class TSocket> class session;

class server {
public:
  server(boost::asio::io_service &io_service, const NginxConfig &config);

  // starts a session
  session<tcp_socket_wrapper> *start_accept();
  // callback function for start_accept
  void handle_accept(session<tcp_socket_wrapper> *new_session,
                     const boost::system::error_code &error);

  
  const RequestHandler *get_request_handler(std::string request_uri) const;
  boost::asio::io_service &io_service_;
  std::unique_ptr<tcp::acceptor> acceptor_;
  std::map<const std::string, const RequestHandler *> location_to_handler_;

private:
  // different request handler types supported
  enum HandlerType {
    HANDLER_ECHO = 0,
    HANDLER_STATIC_FILE = 1,
    HANDLER_NOT_FOUND = 2
  };
  void getHandlers(const NginxConfig &config);
  void create_and_add_handler(HandlerType type, const std::string &location,
                              const NginxConfig &config);
};

#endif // SERVER_H