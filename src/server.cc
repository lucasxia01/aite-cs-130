#include "server.h"

server::server(boost::asio::io_service &io_service, short port)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  start_accept();
}
void server::create_and_add_handler(HandlerType type,
                                    const std::string &location,
                                    const NginxConfig &config) {
  std::string loc = convertToAbsolutePath(location);
  const RequestHandler *handler;
  switch (type) {
  case HANDLER_ECHO:
    handler = new EchoRequestHandler(loc, config);
    break;
  case HANDLER_STATIC_FILE:
    handler = new StaticFileRequestHandler(loc, config);
    break;
  case HANDLER_NOT_FOUND:
    handler = new NotFoundRequestHandler(loc, config);
    break;
  }

  location_to_handler_[loc] = handler;
}
session<tcp_socket_wrapper> *server::start_accept() {
  LOG_DEBUG << "Starting accept";
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
  boost::filesystem::path uri(convertToAbsolutePath(request_uri));
  // look through all path prefixes and check if any match a valid location
  // prioritizing longest path prefix
  while (uri.has_parent_path()) {
    if (location_to_handler_.find(uri.string()) != location_to_handler_.end()) {
      return location_to_handler_.at(uri.string());
    }
    uri = uri.parent_path();
  }
  return nullptr;
}
