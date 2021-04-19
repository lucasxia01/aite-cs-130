#include "session.h"
#include "mock_socket.h"
#include "tcp_socket_wrapper.h"

template <class TSocket>
session<TSocket>::session(boost::asio::io_service &io_service)
    : socket_(io_service) {}
template <class TSocket> TSocket &session<TSocket>::socket() { return socket_; }

template <class TSocket> void session<TSocket>::start() {
  session::read_header();
}

template <class TSocket> std::string session<TSocket>::get_response_str() {
  return request_.raw_header_str + request_.raw_body_str;
}

// Read from socket and bind read handler
template <class TSocket> void session<TSocket>::read_header() {
  memset(data_, 0, max_length);
  socket_.read_some(boost::asio::buffer(data_, max_length),
                    boost::bind(&session::handle_read_header, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
}

template <class TSocket>
void session<TSocket>::read_body(size_t content_length) {
  char *body_buffer = new char[content_length];
  socket_.read(boost::asio::buffer(body_buffer, content_length),
               [this, body_buffer](const boost::system::error_code &error,
                                   size_t bytes_transferred) {
                 if (!error) {
                   // Add remainder of body into request object and echo
                   // response
                   request_.raw_body_str.append(std::string(body_buffer));
                   session::write(response_utils::prepare_echo_buffer(
                       response_utils::OK, request_));
                 } else {
                   delete this;
                 }
                 delete[] body_buffer;
               });
}

// Process data upon socket read
template <class TSocket>
void session<TSocket>::handle_read_header(
    const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error) {
    // Parse header
    boost::tribool request_parse_result;
    char *header_read_end;
    boost::tie(request_parse_result, header_read_end) =
        request_parser_.parse(request_, data_, data_ + bytes_transferred);

    size_t content_length = request_.get_content_length_header();
    if (!request_parse_result || content_length < 0) {
      // Invalid request headers or invalid Content-Length
      // Send back INVALID_REQUEST_MESSAGE
      session::write(response_utils::prepare_echo_buffer(
          response_utils::BAD_REQUEST, request_));
    } else if (request_parse_result) {
      // Valid header parsed
      if (content_length == 0) {
        // If header does not contain content-length, there's no body so just
        // return the header
        session::write(
            response_utils::prepare_echo_buffer(response_utils::OK, request_));
      } else {
        size_t request_header_bytes = header_read_end - data_;
        // Otherwise read the remainder of the buffer as the start of the body,
        // then read content-length - remainder more with asio async_read
        size_t request_body_bytes = bytes_transferred - request_header_bytes;
        if (request_body_bytes >= content_length) {
          request_.raw_body_str.append(
              std::string(header_read_end, header_read_end + content_length));
          session::write(response_utils::prepare_echo_buffer(response_utils::OK,
                                                             request_));
        } else {
          // Append the remainder of the read in data as the first part of the
          // request body Then read in the remainder of the body not included in
          // this read_some
          request_.raw_body_str.append(
              std::string(header_read_end, data_ + bytes_transferred));
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
template <class TSocket>
void session<TSocket>::write(std::string response_str) {
  char *response_buffer = new char[response_str.length() + 1];
  strcpy(response_buffer, response_str.c_str());
  socket_.write(
      boost::asio::buffer(response_buffer, strlen(response_buffer)),
      [this](const boost::system::error_code &error, size_t bytes_transferred) {
        if (!error) {
          request_.reset();
          request_parser_.reset();
          read_header();
        } else {
          delete this;
        }
      });
}

// tcp_socket_wrapper used by server.cc
template class session<tcp_socket_wrapper>;
// mock_socket used by session_test.cc
template class session<mock_socket>;