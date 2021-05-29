#include "session.h"

template <class TSocket>
session<TSocket>::session(boost::asio::io_service &io_service,
                          server *const parent_server)
    : socket_(io_service), parent_server_(parent_server) {}
template <class TSocket> TSocket &session<TSocket>::socket() { return socket_; }

template <class TSocket> void session<TSocket>::start() {
  LOG_DEBUG << socket_.get_endpoint_address() << ": Starting session";
  session::read_header();
}

template <class TSocket> void session<TSocket>::log_session_metric() const {
  LOG_INFO << "[ResponseMetric]"
           << "[" << socket_.get_endpoint_address() << "]"
           << "[" << parent_server_->get_handler_type(request_handler_) << "]"
           << "[" << request_.target() << "]"
           << "[" << response_.result_int() << "]";
}

// Read from socket and bind read handler
template <class TSocket> void session<TSocket>::read_header() {
  LOG_DEBUG << socket_.get_endpoint_address() << ": Reading header";
  memset(data_, 0, max_length);
  socket_.read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(
          &session::handle_read_header,
          boost::enable_shared_from_this<session<TSocket>>::shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

template <class TSocket>
void session<TSocket>::read_body(size_t content_length) {
  LOG_DEBUG << socket_.get_endpoint_address() << ": Reading body of length "
            << content_length;
  socket_.read(
      body_data, content_length,
      boost::bind(
          &session::handle_read_body,
          boost::enable_shared_from_this<session<TSocket>>::shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
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

    int content_length;
    try {
      content_length =
          request_[http::field::content_length].empty()
              ? 0
              : std::stoi(std::string(request_[http::field::content_length]));
    } catch (...) {
      content_length = -1;
    }

    if (!request_parse_result || content_length < 0) {
      // Invalid request headers or invalid Content-Length
      // Send back INVALID_REQUEST_MESSAGE
      LOG_DEBUG << socket_.get_endpoint_address()
                << ": Bad request header or content length...\n"
                << "\tContent length: " << content_length;
      response_ = show_error_page(http::status::bad_request,
                                  "Invalid request headers or content length");
      session::write();
    } else if (request_parse_result) {
      LOG_DEBUG << socket_.get_endpoint_address()
                << ": Successfully read header with content length "
                << content_length;
      LOG_DEBUG << socket_.get_endpoint_address()
                << ": Request URI:" << request_.target();
      request_handler_ =
          parent_server_->get_request_handler(std::string(request_.target()));
      if (!request_handler_) {
        // Bad URI
        LOG_DEBUG << socket_.get_endpoint_address()
                  << ": Bad URI:" << request_.target();
        response_ = show_error_page(http::status::bad_request,
                                    "Error getting handler for uri " +
                                        std::string(request_.target()));
        session::write();
        return;
      }
      // Valid header parsed
      if (content_length == 0) {
        // If header does not contain content-length, there's no body so just
        // return the header
        response_ = request_handler_->handle_request(request_);
        LOG_DEBUG << socket_.get_endpoint_address()
                  << ": Response status: " << response_.result_int();
        session::write();
      } else {
        // Otherwise read the remainder of the buffer as the start of the body,
        // then read content-length - remainder more with asio async_read
        size_t request_header_bytes = header_read_end - data_;
        size_t request_body_bytes = bytes_transferred - request_header_bytes;
        LOG_DEBUG << socket_.get_endpoint_address()
                  << ": request_header_bytes: " << request_header_bytes
                  << ", bytes_transferred: " << bytes_transferred
                  << ", request_body_bytes: " << request_body_bytes
                  << ", content_length: " << content_length;

        if (request_body_bytes >= content_length) {
          request_.body().append(
              std::string(header_read_end, header_read_end + content_length));
          response_ = request_handler_->handle_request(request_);
          LOG_DEBUG << socket_.get_endpoint_address()
                    << ": Response status: " << response_.result_int();
          session::write();
        } else {
          // Append the remainder of the read in data as the first part of the
          // request body Then read in the remainder of the body not included in
          // this read_some
          request_.body().append(
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
  }
}

// Process data upon socket read
template <class TSocket>
void session<TSocket>::handle_read_body(const boost::system::error_code &error,
                                        size_t bytes_transferred) {
  if (!error) {
    // Add remainder of body into request object and echo
    // response
    LOG_DEBUG << "INSIDE HANDLE_READ_BODY";
    auto d = body_data.data();
    std::string s(boost::asio::buffers_begin(d),
                  boost::asio::buffers_begin(d) + body_data.size());
    LOG_DEBUG << "parsed body in handle read body: " << s;
    request_.body().append(s);
    response_ = request_handler_ ? request_handler_->handle_request(request_)
                                 : show_error_page(http::status::bad_request);
    session::write();
  } else {
    LOG_ERROR << socket_.get_endpoint_address()
              << ": Error in read body:" << error.message();
  }
}

template <class TSocket>
void session<TSocket>::handle_write(const boost::system::error_code &error,
                                    size_t bytes_transferred) {
  if (!error) {
    LOG_DEBUG << socket_.get_endpoint_address()
              << ": Completed write. Preparing for next request";
    request_parser_.reset();
    read_header();
  } else {
    LOG_ERROR << socket_.get_endpoint_address()
              << ": Error writing to socket: " << error.message();
  }
}

// Write to socket and bind write handler
template <class TSocket> void session<TSocket>::write() {
  log_session_metric();
  std::ostringstream ss;
  ss << response_;
  std::string response_str = ss.str();

  LOG_DEBUG << socket_.get_endpoint_address()
            << ": Starting write to socket, response length "
            << response_str.length();

  char *response_buffer = new char[response_str.length() + 1];
  memcpy(response_buffer, response_str.c_str(), response_str.length());
  socket_.write(
      boost::asio::buffer(response_buffer, response_str.length()),
      boost::bind(
          &session::handle_write,
          boost::enable_shared_from_this<session<TSocket>>::shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));

  boost::lock_guard<boost::mutex> guard(parent_server_->mtx_);
  parent_server_->log_request(
      std::make_pair(std::string(request_.target()), response_.result()));
}

// tcp_socket_wrapper used by server.cc
template class session<tcp_socket_wrapper>;
// mock_socket used by session_test.cc
template class session<mock_socket>;