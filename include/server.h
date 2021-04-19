#include "session.h"
#include "tcp_socket_wrapper.h"

class server {
public:
  server(boost::asio::io_service &io_service, short port);

  session<tcp_socket_wrapper> *start_accept();

  void handle_accept(session<tcp_socket_wrapper> *new_session,
                     const boost::system::error_code &error);

  boost::asio::io_service &io_service_;
  tcp::acceptor acceptor_;
  int start_accept_called;
};