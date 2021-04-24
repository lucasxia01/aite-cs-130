#include "server.h"

server::server(boost::asio::io_service &io_service, short port,
               std::set<std::string> echo_roots,
               std::map<std::string, std::string> root_to_base_dir)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      start_accept_called(0), echo_request_handler(echo_roots),
      static_file_request_handler(root_to_base_dir) {
  start_accept();
}

session<tcp_socket_wrapper> *server::start_accept() {
  LOG_DEBUG << "Starting accept";
  start_accept_called++;
  session<tcp_socket_wrapper> *new_session = new session<tcp_socket_wrapper>(
      io_service_, echo_request_handler, static_file_request_handler);
  LOG_DEBUG << "Waiting to accept new connection...";
  acceptor_.async_accept((new_session->socket()).get_socket(),
                         boost::bind(&server::handle_accept, this, new_session,
                                     boost::asio::placeholders::error));
  return new_session;
}

void server::handle_accept(session<tcp_socket_wrapper> *new_session,
                           const boost::system::error_code &error) {
  if (!error) {
    LOG_INFO << "Accepted connection with address: "
             << (new_session->socket()).get_endpoint_address();
    new_session->start();
  } else {
    LOG_ERROR << "Error when accepting socket connection: " << error.message();
    delete new_session;
  }

  start_accept();
}