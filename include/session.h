#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class session {
public:
  session(boost::asio::io_service &io_service) : socket_(io_service) {}

  tcp::socket &socket();

  void start();

  static const int MAX_RESPONSE_HEADER_SIZE = 100;

private:
  void read();
  void write(char *response_buffer);

  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transferred);

  void handle_write(const boost::system::error_code &error);

  tcp::socket socket_;
  enum { max_length = 10240 };
  char data_[max_length];
};