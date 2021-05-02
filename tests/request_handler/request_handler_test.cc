#include "request_handler.h"
#include "gtest/gtest.h"

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
  std::string root;
  bool found = echo_request_handler->get_root_from_uri("/echo/file.txt", root);
  EXPECT_EQ(root, "/echo");
  EXPECT_TRUE(found);
}

TEST_F(EchoRequestHandlerTest, EchoGetRootFound2) {
  std::string root;
  bool found =
      echo_request_handler->get_root_from_uri("/echo/morestuff/file.txt", root);
  EXPECT_EQ(root, "/echo");
  EXPECT_TRUE(found);
}

TEST_F(EchoRequestHandlerTest, EchoGetRootFoundMultiLevel) {
  std::string root;
  bool found =
      echo_request_handler->get_root_from_uri("/echo/echo/file.txt", root);
  EXPECT_EQ(root, "/echo/echo");
  EXPECT_TRUE(found);
}

TEST_F(EchoRequestHandlerTest, EchoGetRootNotFound) {
  std::string root;
  bool found = echo_request_handler->get_root_from_uri("/nope/file.txt", root);
  EXPECT_FALSE(found);
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
  std::string root;
  bool found =
      static_file_request_handler->get_root_from_uri("/static/file.txt", root);
  EXPECT_EQ(root, "/static");
  EXPECT_TRUE(found);
}

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootFound2) {
  std::string root;
  bool found = static_file_request_handler->get_root_from_uri(
      "/static2/morestuff/file.txt", root);
  EXPECT_EQ(root, "/static2");
  EXPECT_TRUE(found);
}

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootFoundMultiLevel) {
  std::string root;
  bool found = static_file_request_handler->get_root_from_uri(
      "/static/static/file.txt", root);
  EXPECT_EQ(root, "/static/static");
  EXPECT_TRUE(found);
}

TEST_F(StaticFileRequestHandlerTest, StaticFileGetRootNotFound) {
  std::string root;
  bool found =
      static_file_request_handler->get_root_from_uri("/nope/file.txt", root);
  EXPECT_FALSE(found);
}

TEST_F(StaticFileRequestHandlerTest, StaticFileNotFound) {
  std::string root;
  http::server3::request req;
  req.uri = "/static/nope/file.txt";
  response received_response =
      static_file_request_handler->generate_response(req);

  std::stringstream ss;
  ss << "HTTP/1.1 " << response::get_status_string(response::NOT_FOUND)
     << "\r\n"
     << "Content-Type: text/plain\r\n"
     << "Content-Length: 15\r\n\r\n"
     << "File not found\n";
  std::string expected_response = ss.str();

  // get response from server through mock socket
  EXPECT_EQ(expected_response, received_response.to_string());
}