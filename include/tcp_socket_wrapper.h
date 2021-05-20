#ifndef TCP_SOCKET_WRAPPER_H
#define TCP_SOCKET_WRAPPER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cstring>
#include <string>

/**
 * Wrapper class for boost::asio::ip::tcp::socket
 * Converts async_read and async_write to member functions
 **/
class tcp_socket_wrapper {

public:
  tcp_socket_wrapper(boost::asio::io_service &io_service)
      : socket_(io_service) {}
  void read_some(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {
    socket_.async_read_some(buf, myFunc);
  }

  void read(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {
    boost::asio::async_read(socket_, buf, myFunc);
  }

  void write(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {
    boost::asio::async_write(socket_, buf, myFunc);
  }

  boost::asio::ip::tcp::socket &get_socket() { return socket_; }

  std::string get_endpoint_address() const {
    return socket_.remote_endpoint().address().to_string();
  }

private:
  boost::asio::ip::tcp::socket socket_;
};

#endif // TCP_SOCKET_WRAPPER_H