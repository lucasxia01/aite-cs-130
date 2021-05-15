#include "http_client.h"
#include "logger.h"
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

http::response HttpClient::perform_request(
    const std::string &host, const std::string &port,
    const http::request &req) {
  // These objects perform our I/O
  boost::asio::io_service ios;
  tcp::resolver resolver(ios);
  beast::tcp_stream stream(ios);

  // Look up the domain name
  tcp::resolver::query query(host, port);
  auto const results = resolver.resolve(query);

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  // Send the HTTP request to the remote host
  http::write(stream, req);

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Declare a container to hold the response
  http::response resp;

  // Receive the HTTP response
  http::read(stream, buffer, resp);
  // Close socket connection when done.
  beast::error_code err;
  stream.socket().shutdown(tcp::socket::shutdown_both, err);

  // not_connected happens sometimes
  // so don't bother reporting it.
  if (err && err != beast::errc::not_connected) {
    LOG_ERROR << "Error occurred while closing socket: " << err;
    throw beast::system_error{err};
  }
  return resp;
}