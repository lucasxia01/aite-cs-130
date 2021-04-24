#include "server.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

TEST(ServerTest, callsStartAndHandle) {
  boost::asio::io_service io_service;
  boost::system::error_code error;
  short port = 8080;
  std::set<std::string> echo_roots = {"/echo", "/echo1", "/repeat"};
  std::map<std::string, std::string> root_to_base_dir = {{"/static", "/"}};
  server s(io_service, port, echo_roots, root_to_base_dir);
  EXPECT_EQ(s.start_accept_called, 1);
  session<tcp_socket_wrapper> *sess = s.start_accept();
}
