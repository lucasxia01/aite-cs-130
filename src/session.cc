#include "session.h"

tcp::socket &session::socket() { return socket_; }

void session::start() { session::read(); }

void session::read() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred) {
  if (!error) {
    int buf_size = strlen(data_) + MAX_RESPONSE_HEADER_SIZE;
    char response_buffer[buf_size];
    sprintf(response_buffer,
            "%s %s\nContent-Type: %s\nContent-Length: %li\n\n%s", "HTTP/1.1",
            "200 OK", "text/plain", strlen(data_), data_);
    session::write(response_buffer);
  } else {
    delete this;
  }
}

void session::write(char *response_buffer) {
  boost::asio::async_write(
      socket_, boost::asio::buffer(response_buffer, strlen(response_buffer)),
      boost::bind(&session::handle_write, this,
                  boost::asio::placeholders::error));
}

void session::handle_write(const boost::system::error_code &error) {
  if (!error) {
    session::read();
  } else {
    delete this;
  }
}