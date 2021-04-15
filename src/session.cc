#include "session.h"

session::session(boost::asio::io_service &io_service)
    : socket_(io_service), response_buffer_(nullptr), body_buffer_(nullptr) {}
tcp::socket &session::socket() { return socket_; }

const int session::MAX_RESPONSE_HEADER_SIZE = 100;
const std::string session::INVALID_REQUEST_MESSAGE = "Invalid request\n";

void session::start() { session::read_header(); }

void session::reset_request_parser() {
  response_agg_.clear();
  request_parser_.reset();
  request_ = {};
}

void session::set_response_buffer(http::response::status_type status,
                                  std::string response_body) {
  if (response_buffer_ != nullptr) {
    delete[] response_buffer_;
  }
  std::string response_status_message;
  switch (status) {
  case http::response::OK:
    response_status_message = "200 OK\0";
    break;
  case http::response::BAD_REQUEST:
    response_status_message = "400 Bad Request\0";
    break;
  }
  int buf_size = MAX_RESPONSE_HEADER_SIZE + strlen(response_body.c_str());
  response_buffer_ = new char[buf_size];
  sprintf(response_buffer_,
          "%s %s\r\nContent-Type: %s\r\nContent-Length: %li\r\n\r\n%s",
          "HTTP/1.1", response_status_message.c_str(), "text/plain",
          strlen(response_body.c_str()), response_body.c_str());
}

// Read from socket and bind read handler
void session::read_header() {
  memset(data_, 0, max_length);
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read_header, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

// Read from socket and bind read handler
void session::read_body(size_t content_length) {
  if (body_buffer_ != nullptr) {
    delete[] body_buffer_;
  }
  body_buffer_ = new char[content_length];
  boost::asio::async_read(
      socket_, boost::asio::buffer(body_buffer_, content_length),
      boost::bind(&session::handle_read_body, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

size_t
session::get_content_length_header(const http::server3::request &request) {
  for (http::server3::header h : request.headers) {
    if (!h.name.compare("Content-Length")) {
      try {
        return stoi(h.value);
      } catch (...) {
        return -1;
      }
    }
  }
  return 0;
}

std::string
session::get_aggregate_response(const std::vector<std::string> &response_agg) {
  std::ostringstream ss;
  for (int i = 0; i < response_agg.size(); i++) {
    ss << response_agg[i];
  }
  return ss.str();
}

void session::handle_read_body(const boost::system::error_code &error,
                               size_t bytes_transferred) {
  if (!error) {
    response_agg_.push_back(std::string(body_buffer_));
    session::set_response_buffer(
        http::response::OK, session::get_aggregate_response(response_agg_));
    session::write(response_buffer_);
  } else {
    delete this;
  }
}
// Process data upon socket read
void session::handle_read_header(const boost::system::error_code &error,
                                 size_t bytes_transferred) {
  if (!error) {
    // Parse header
    boost::tribool request_parse_result;
    size_t header_bytes_parsed;
    boost::tie(request_parse_result, boost::tuples::ignore,
               header_bytes_parsed) =
        request_parser_.parse(request_, data_, data_ + bytes_transferred);

    // Store any parts of the header that get parsed
    response_agg_.push_back(std::string(data_, data_ + header_bytes_parsed));

    size_t content_length = session::get_content_length_header(request_);

    if (!request_parse_result || content_length < 0) {
      // Send back INVALID_REQUEST_MESSAGE
      session::set_response_buffer(http::response::BAD_REQUEST,
                                   INVALID_REQUEST_MESSAGE);
      session::write(response_buffer_);
    } else if (request_parse_result) {
      // If header does not contain content-length, there's no body so just
      // return the header
      if (content_length == 0) {
        session::set_response_buffer(
            http::response::OK, session::get_aggregate_response(response_agg_));
        session::write(response_buffer_);
      } else {
        // Otherwise read the remainder of the buffer as the start of the body,
        // then read content-length - remainder more with asio async_read
        size_t request_body_bytes = bytes_transferred - header_bytes_parsed;
        if (request_body_bytes >= content_length) {
          response_agg_.push_back(
              std::string(data_ + header_bytes_parsed,
                          data_ + header_bytes_parsed + content_length));
          session::set_response_buffer(
              http::response::OK,
              session::get_aggregate_response(response_agg_));
          session::write(response_buffer_);
        } else {
          response_agg_.push_back(std::string(data_ + header_bytes_parsed,
                                              data_ + bytes_transferred));
          session::read_body(content_length - request_body_bytes);
        }
      }
    } else {
      // Handle another read;
      session::read_header();
    }
  } else {
    delete this;
  }
}

// Write to socket and bind write handler
void session::write(char *response_buffer) {
  boost::asio::async_write(
      socket_, boost::asio::buffer(response_buffer, strlen(response_buffer)),
      boost::bind(&session::handle_write, this,
                  boost::asio::placeholders::error));
}

// Re-enter read stage upon writing data
void session::handle_write(const boost::system::error_code &error) {
  if (!error) {
    session::reset_request_parser();
    session::read_header();
  } else {
    delete this;
  }
}