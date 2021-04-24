#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "logger.h"
#include <request.h>
#include <request_parser.h>
#include <response_utils.h>
#include <vector>

using boost::asio::ip::tcp;

template <class TSocket> class session {
public:
  session(boost::asio::io_service &io_service);

  TSocket &socket();

  void start();
  static const int MAX_RESPONSE_HEADER_SIZE;
  static const std::string INVALID_REQUEST_MESSAGE;

  std::string get_response_str();

private:
  void read_header();
  void read_body(size_t content_length);
  void write(std::string response_str);

  void handle_read_header(const boost::system::error_code &error,
                          size_t bytes_transferred);

  TSocket socket_;
  http::server3::request_parser request_parser_;
  http::server3::request request_;

  enum { max_length = 1024 };
  char data_[max_length];
};