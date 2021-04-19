#include "server.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

TEST(ServerTest, callsStartAndHandle) {
  boost::asio::io_service io_service;
  boost::system::error_code error;
  short port = 8080;
  server s(io_service, port);
  EXPECT_EQ(s.start_accept_called, 1);
  session<tcp_socket_wrapper> *sess = s.start_accept();
  s.handle_accept(sess, error);
  error = boost::asio::error::access_denied;
  s.handle_accept(sess, error);
  EXPECT_EQ(s.start_accept_called, 4);
}
