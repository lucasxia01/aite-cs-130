#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <request.h>
#include <request_parser.h>
#include <response.h>
#include <sstream>

using boost::asio::ip::tcp;

class session {
public:
  session(boost::asio::io_service &io_service);

  tcp::socket &socket();

  void start();

  static const int MAX_RESPONSE_HEADER_SIZE;
  static const std::string INVALID_REQUEST_MESSAGE;

  void read_header();
  void read_body(size_t content_length);
  void write(char *response_buffer);

  void handle_read_header(const boost::system::error_code &error,
                          size_t bytes_transferred);
  void handle_read_body(const boost::system::error_code &error,
                        size_t bytes_transferred);
  void handle_write(const boost::system::error_code &error);

  void set_response_buffer(http::response::status_type status,
                           std::string response_body);

  void reset_request_parser();
  std::string
  get_aggregate_response(const std::vector<std::string> &response_agg_);
  size_t get_content_length_header(const http::server3::request &request);

  std::vector<std::string> response_agg_;
  tcp::socket socket_;
  http::server3::request_parser request_parser_;
  http::server3::request request_;
  char *response_buffer_;
  char *body_buffer_;

  enum { max_length = 1024 };
  char data_[max_length];
};