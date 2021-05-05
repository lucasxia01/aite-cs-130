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
  session<tcp_socket_wrapper> *new_session =
      new session<tcp_socket_wrapper>(io_service_, this);
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

/**
 * Get the appropriate request handler (echo or static file) based on the
 * request URI. If neither valid echo nor valid static file roots are found,
 * return null.
 **/
const RequestHandler *
server::get_request_handler(std::string request_uri) const {
  std::optional<std::string> echo_root_opt =
      echo_request_handler.get_root_from_uri(request_uri);
  std::optional<std::string> static_file_root_opt =
      static_file_request_handler.get_root_from_uri(request_uri);
  if (echo_root_opt && static_file_root_opt) {
    LOG_DEBUG << "Found both echo (" << echo_root_opt.value()
              << ") and static (" << static_file_root_opt.value()
              << ") roots from uri: " << request_uri;
    return nullptr;
  } else if (echo_root_opt) {
    return &echo_request_handler;
  } else if (static_file_root_opt) {
    return &static_file_request_handler;
  } else {
    LOG_DEBUG << request_uri << " contains neither echo or static file root\n";
    return nullptr;
  }
}
