#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cstring>
#include <string>
#include <iostream>

/**
 * Mocks out the tcp_socker_wrapper class for unit tests for methods in session
 **/
class mock_socket {

public:
  mock_socket(boost::asio::io_service &io_service) {}

  // copies all contents of input buffer into buf
  void read_some(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {

    if (!fake_input_buffer.length())
      return;

    size_t bytes_transferred = std::min(buf.size(), fake_input_buffer.length());
    memcpy(boost::asio::buffer_cast<char *>(buf),
           fake_input_buffer.substr(0, bytes_transferred).c_str(),
           bytes_transferred);

    fake_input_buffer = fake_input_buffer.substr(bytes_transferred);
    myFunc(make_error_code(boost::system::errc::success), bytes_transferred);
  }

  // copies the first buf.size() bytes of input buffer into buf
  void read(
      boost::asio::streambuf &buf,
      int to_read,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {

    std::string to_transfer = fake_input_buffer.substr(0, to_read);

    size_t bytes_transferred = to_transfer.length();
    std::ostream os(&buf);
    os << to_transfer;

    fake_input_buffer = fake_input_buffer.substr(bytes_transferred);
    myFunc(make_error_code(boost::system::errc::success), bytes_transferred);
  }

  // writes the contents of buf into output buffer
  void write(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {

    char *buffer = boost::asio::buffer_cast<char *>(buf);
    fake_output_buffer.assign(buffer, buf.size());

    myFunc(make_error_code(boost::system::errc::success), buf.size());
  }

  void set_input_buffer(std::string buf) { fake_input_buffer = buf; }

  std::string get_output_buffer() { return fake_output_buffer; }

  std::string get_endpoint_address() const { return "1.2.3.4"; }

private:
  std::string fake_input_buffer;
  std::string fake_output_buffer;
};

#endif // MOCK_SOCKET_H