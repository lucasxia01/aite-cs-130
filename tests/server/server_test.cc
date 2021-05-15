#include "server.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

class ServerTest : public testing::Test {
  boost::asio::io_service io_service;
  NginxConfig config;

protected:
  server *s;
  void SetUp() override { s = new server(io_service, config); }
  void TearDown() override { delete s; }
};

TEST_F(ServerTest, longestPrefix) {
  s->location_to_handler_["/echo/echo"] = new DummyRequestHandler("/echo/echo");
  s->location_to_handler_["/echo"] = new DummyRequestHandler("/echo");
  s->location_to_handler_["/echo/echo/echo"] =
      new DummyRequestHandler("/echo/echo/echo");
  std::string request_uri = "/echo/echo/echo";
  const RequestHandler *handler = s->get_request_handler(request_uri);
  const DummyRequestHandler *handle =
      static_cast<const DummyRequestHandler *>(handler);
  EXPECT_EQ(handle->get_location(), "/echo/echo/echo");
}
