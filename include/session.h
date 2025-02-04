#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <vector>

#include "logger.h"
#include "mock_socket.h"
#include "request_handler.h"
#include "request_parser.h"
#include "server.h"
#include "tcp_socket_wrapper.h"

class server;
class RequestHandler;

template <class TSocket>
class session : public boost::enable_shared_from_this<session<TSocket>> {
public:
  session(boost::asio::io_service &io_service, server *const parent_server);

  // getter for socket_
  TSocket &socket();
  // server uses it to start a session
  void start();

private:
  // reads the header of an http request
  void read_header();
  // reads the body of an http reqeust based on content length specified in http
  // request header
  void read_body(size_t content_length);
  // Write to socket and bind write handler
  void write();
  // Callback for read_header, parses the request, dispatches the request to the
  // corresponding handler based on server level mapping of location path to
  // handler
  void handle_read_header(const boost::system::error_code &error,
                          size_t bytes_transferred);

  // Callback for read body, appends what was read into message body into
  // request body, calls request handler, then writes back
  void handle_read_body(const boost::system::error_code &error,
                        size_t bytes_transferred);
  void handle_write(const boost::system::error_code &error,
                    size_t bytes_transferred);

  void log_session_metric() const;

  TSocket socket_;
  request_parser request_parser_;
  http::request request_;
  http::response response_;

  const RequestHandler *request_handler_;
  server *const parent_server_;

  enum { max_length = 1024 };
  char data_[max_length];
  boost::asio::streambuf body_data;
};

#endif // SESSION_H