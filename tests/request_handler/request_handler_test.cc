#include "request_handler.h"
#include "gtest/gtest.h"
#include <optional>
class EchoRequestHandlerTest : public testing::Test {
protected:
  EchoRequestHandler *echo_request_handler;
  std::set<std::string> roots = {"/echo", "/echo2", "/echo/echo"};
  void SetUp() override {
    echo_request_handler = new EchoRequestHandler(roots);
  }
  void TearDown() override { delete echo_request_handler; }
};

TEST_F(EchoRequestHandlerTest, EchoGetRootFound) {
  std::optional<std::string> root_opt =
      echo_request_handler->get_root_from_uri("/echo/file.txt");
  EXPECT_EQ(root_opt.value(), "/echo");
  EXPECT_TRUE(root_opt.has_value());
}

TEST_F(EchoRequestHandlerTest, EchoGetRootFound2) {
  std::optional<std::string> root_opt =
      echo_request_handler->get_root_from_uri("/echo/morestuff/file.txt");
  EXPECT_EQ(root_opt.value(), "/echo");
  EXPECT_TRUE(root_opt.has_value());
}

TEST_F(EchoRequestHandlerTest, EchoGetRootFoundMultiLevel) {
  std::optional<std::string> root_opt =
      echo_request_handler->get_root_from_uri("/echo/echo/file.txt");
  EXPECT_EQ(root_opt.value(), "/echo/echo");
  EXPECT_TRUE(root_opt.has_value());
}

TEST_F(EchoRequestHandlerTest, EchoGetRootNotFound) {
  std::optional<std::string> root_opt =
      echo_request_handler->get_root_from_uri("/nope/file.txt");
  EXPECT_FALSE(root_opt.has_value());
}

class StaticFileRequestHandlerTest : public testing::Test {
protected:
  StaticFileRequestHandler *static_file_request_handler;
  std::map<std::string, std::string> root_to_base_dir = {
      {"/static", "/test"}, {"/static2", "/test2"}, {"/static/static", "/"}};
  void SetUp() override {
    static_file_request_handler =
        new StaticFileRequestHandler(root_to_base_dir);
  }
  void TearDown() override { delete static_file_request_handler; }
};

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootFound) {
  std::optional<std::string> root_opt =
      static_file_request_handler->get_root_from_uri("/static/file.txt");
  EXPECT_EQ(root_opt.value(), "/static");
  EXPECT_TRUE(root_opt.has_value());
}

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootFound2) {
  std::optional<std::string> root_opt =
      static_file_request_handler->get_root_from_uri(
          "/static2/morestuff/file.txt");
  EXPECT_EQ(root_opt.value(), "/static2");
  EXPECT_TRUE(root_opt.has_value());
}

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootFoundMultiLevel) {
  std::optional<std::string> root_opt =
      static_file_request_handler->get_root_from_uri("/static/static/file.txt");
  EXPECT_EQ(root_opt.value(), "/static/static");
  EXPECT_TRUE(root_opt.has_value());
}

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootNotFound) {
  std::string root;
  std::optional<std::string> root_opt =
      static_file_request_handler->get_root_from_uri("/nope/file.txt");
  EXPECT_FALSE(root_opt.has_value());
}

TEST_F(StaticFileRequestHandlerTest, StaticFileNotFound) {
  std::string root;
  http::request req;
  req.target("/static/nope/file.txt");
  std::stringstream received_response;
  received_response << static_file_request_handler->handle_request(req);

  std::stringstream expected_response;
  expected_response << "HTTP/1.1 404 Not Found"
                    << "\r\n"
                    << "Content-Type: text/plain\r\n"
                    << "Content-Length: 15\r\n\r\n"
                    << "File not found\n";

  // get response from server through mock socket
  EXPECT_EQ(expected_response.str(), received_response.str());
}