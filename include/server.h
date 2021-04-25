#ifndef SERVER_H
#define SERVER_H

#include "logger.h"
#include "session.h"
#include "tcp_socket_wrapper.h"
#include <map>
#include <set>
#include <string>

class server {
public:
  server(boost::asio::io_service &io_service, short port,
         std::set<std::string> echo_roots,
         std::map<std::string, std::string> root_to_base_dir);

  session<tcp_socket_wrapper> *start_accept();

  void handle_accept(session<tcp_socket_wrapper> *new_session,
                     const boost::system::error_code &error);

  boost::asio::io_service &io_service_;
  tcp::acceptor acceptor_;
  int start_accept_called;

  EchoRequestHandler echo_request_handler;
  StaticFileRequestHandler static_file_request_handler;
};

#endif // SERVER_H