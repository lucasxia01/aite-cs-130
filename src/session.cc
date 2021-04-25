#include "session.h"
#include "mock_socket.h"
#include "tcp_socket_wrapper.h"
#include <iostream>

template <class TSocket>
session<TSocket>::session(boost::asio::io_service &io_service,
                          EchoRequestHandler echo_request_handler,
                          StaticFileRequestHandler static_file_request_handler)
    : socket_(io_service), echo_request_handler_(echo_request_handler),
      static_file_request_handler_(static_file_request_handler) {}
template <class TSocket> TSocket &session<TSocket>::socket() { return socket_; }

template <class TSocket> void session<TSocket>::start() {
  LOG_DEBUG << socket_.get_endpoint_address() << ": Starting session";
  session::read_header();
}

template <class TSocket> std::string session<TSocket>::get_response_str() {
  return request_.raw_header_str + request_.raw_body_str;
}

// Read from socket and bind read handler
template <class TSocket> void session<TSocket>::read_header() {
  LOG_DEBUG << socket_.get_endpoint_address() << ": Reading header";
  memset(data_, 0, max_length);
  socket_.read_some(boost::asio::buffer(data_, max_length),
                    boost::bind(&session::handle_read_header, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
}

template <class TSocket>
void session<TSocket>::read_body(size_t content_length) {
  LOG_DEBUG << socket_.get_endpoint_address() << ": Reading body of length "
            << content_length;
  char *body_buffer = new char[content_length];
  socket_.read(
      boost::asio::buffer(body_buffer, content_length),
      [this, body_buffer](const boost::system::error_code &error,
                          size_t bytes_transferred) {
        if (!error) {
          // Add remainder of body into request object and echo
          // response
          request_.raw_body_str.append(std::string(body_buffer));
          response_ =
              request_handler_
                  ? request_handler_->generate_response(response::OK, request_)
                  : response::get_stock_response(response::BAD_REQUEST);
          session::write();
        } else {
          LOG_ERROR << socket_.get_endpoint_address()
                    << ": Error in read body:" << error.message();
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
      LOG_DEBUG << socket_.get_endpoint_address()
                << ": Bad request header or content length...\n"
                << "\tContent length: " << content_length;
      response_ = response::get_stock_response(response::BAD_REQUEST);
      session::write();
    } else if (request_parse_result) {
      LOG_DEBUG << socket_.get_endpoint_address()
                << ": Succesfully read header";
      LOG_DEBUG << socket_.get_endpoint_address()
                << ": Request URI:" << request_.uri;
      for (auto &header : request_.headers) {
        LOG_DEBUG << socket_.get_endpoint_address() << ": " << header.name
                  << ", " << header.value;
      }
      request_handler_ = session::get_request_handler(request_.uri);
      if (!request_handler_) {
        // Bad URI
        LOG_DEBUG << socket_.get_endpoint_address()
                  << ": Bad URI:" << request_.uri;
        response_ = response::get_stock_response(response::BAD_REQUEST);
        session::write();
        return;
      }
      // Valid header parsed
      if (content_length == 0) {
        // If header does not contain content-length, there's no body so just
        // return the header
        response_ = request_handler_->generate_response(response::OK, request_);
        LOG_DEBUG << socket_.get_endpoint_address()
                  << ": Response status: " << response_.status;
        session::write();
      } else {
        // Otherwise read the remainder of the buffer as the start of the body,
        // then read content-length - remainder more with asio async_read
        size_t request_header_bytes = header_read_end - data_;
        size_t request_body_bytes = bytes_transferred - request_header_bytes;

        if (request_body_bytes >= content_length) {
          request_.raw_body_str.append(
              std::string(header_read_end, header_read_end + content_length));
          response_ =
              request_handler_->generate_response(response::OK, request_);
          LOG_DEBUG << socket_.get_endpoint_address()
                    << ": Response status: " << response_.status;
          session::write();
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
      LOG_DEBUG << socket_.get_endpoint_address() << ": Read remaining header";
      // Handle another read;
      session::read_header();
    }
  } else {
    LOG_ERROR << socket_.get_endpoint_address()
              << ": Error in read header: " << error.message();
    delete this;
  }
}

/**
 * Get the appropriate request handler (echo or static file) based on the
 * request URI. If neither valid echo nor valid static file roots are found,
 * return null.
 **/
template <class TSocket>
RequestHandler *session<TSocket>::get_request_handler(std::string requestUri) {
  std::string echo_root, static_file_root;
  bool is_echo_root = echo_request_handler_.get_root(requestUri, echo_root);
  bool is_static_file_root =
      static_file_request_handler_.get_root(requestUri, static_file_root);
  if (is_echo_root && is_static_file_root) {
    LOG_DEBUG << socket_.get_endpoint_address() << "Root \"" << echo_root
              << "\" is both echo and static file root\n";
    return nullptr;
  } else if (is_echo_root) {
    return &echo_request_handler_;
  } else if (is_static_file_root) {
    return &static_file_request_handler_;
  } else {
    LOG_DEBUG << socket_.get_endpoint_address() << "Root \"" << echo_root
              << "\" is neither echo or static file root\n";
    return nullptr;
  }
}

// Write to socket and bind write handler
template <class TSocket> void session<TSocket>::write() {
  std::string response_str = response_.to_string();

  LOG_DEBUG << socket_.get_endpoint_address()
            << ": Starting write to socket, response length "
            << response_str.length();

  char *response_buffer = new char[response_str.length() + 1];
  memcpy(response_buffer, response_str.c_str(), response_str.length());
  socket_.write(
      boost::asio::buffer(response_buffer, response_str.length()),
      [this](const boost::system::error_code &error, size_t bytes_transferred) {
        if (!error) {
          LOG_DEBUG << socket_.get_endpoint_address()
                    << ": Completed write. Preparing for next request";
          request_.reset();
          request_parser_.reset();
          read_header();
        } else {
          LOG_ERROR << socket_.get_endpoint_address()
                    << ": Error writing to socket: " << error.message();
          delete this;
        }
      });
}

// tcp_socket_wrapper used by server.cc
template class session<tcp_socket_wrapper>;
// mock_socket used by session_test.cc
template class session<mock_socket>;