
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cstring>
#include <string>

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
    size_t bytes_transferred = fake_input_buffer.length();
    memcpy(boost::asio::buffer_cast<char *>(buf), fake_input_buffer.c_str(),
           bytes_transferred);
    fake_input_buffer = "";
    myFunc(make_error_code(boost::system::errc::success), bytes_transferred);
  }

  // copies the first buf.size() bytes of input buffer into buf
  void read(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {
    std::string to_transfer = fake_input_buffer.substr(0, buf.size());
    memcpy(boost::asio::buffer_cast<char *>(buf), to_transfer.c_str(), to_transfer.length());
    myFunc(make_error_code(boost::system::errc::success), to_transfer.length());
  }

  // writes the contents of buf into output buffer
  void write(
      boost::asio::mutable_buffer buf,
      boost::function<void(const boost::system::error_code &, size_t)> myFunc) {
    char *buffer = boost::asio::buffer_cast<char *>(buf);
    fake_output_buffer.assign(buffer, strlen(buffer));
    myFunc(make_error_code(boost::system::errc::success), strlen(buffer));
  }

  void set_input_buffer(std::string buf) { fake_input_buffer = buf; }

  std::string get_output_buffer() { return fake_output_buffer; }

private:
  std::string fake_input_buffer;
  std::string fake_output_buffer;
};