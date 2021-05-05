#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <vector>

#include "logger.h"
#include "mock_socket.h"
#include "request_handler.h"
#include "request_parser.h"
#include "server.h"
#include "tcp_socket_wrapper.h"

class server;

template <class TSocket> class session {
public:
  session(boost::asio::io_service &io_service,
          const server *const parent_server);

  TSocket &socket();

  void start();

private:
  void read_header();
  void read_body(size_t content_length);
  void write();

  void handle_read_header(const boost::system::error_code &error,
                          size_t bytes_transferred);

  TSocket socket_;
  request_parser request_parser_;
  http::request request_;
  http::response response_;

  const RequestHandler *request_handler_;
  const server *const parent_server_;

  enum { max_length = 1024 };
  char data_[max_length];
};

#endif // SESSION_H