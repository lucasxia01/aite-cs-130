#include "mock_socket.h"
#include "request_parser.h"
#include "session.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>
#include <sstream>

class SessionTest : public testing::Test {
  std::set<std::string> echo_roots = {"/echo"};
  std::map<std::string, std::string> root_to_base_dir = {
      {"/static", "/usr/src/projects/"}};

protected:
  boost::asio::io_service io_service;
  EchoRequestHandler echo_request_handler;
  StaticFileRequestHandler static_file_request_handler;
  boost::system::error_code error;
  session<mock_socket> *testSess;
  SessionTest()
      : echo_request_handler(echo_roots),
        static_file_request_handler(root_to_base_dir) {}
  void SetUp(void) {
    testSess = new session<mock_socket>(io_service, echo_request_handler,
                                        static_file_request_handler);
  }
  void TearDown(void) { delete testSess; }
};

TEST_F(SessionTest, NoBodySuccess) {
  std::string request = "GET /echo HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
                        "curl/7.68.0\r\nContent-Length: 0\r\n\r\n";
  // client sends request to server through mock socket
  (testSess->socket()).set_input_buffer(request);
  // server starts listening
  testSess->start();

  std::stringstream ss;
  ss << "HTTP/1.1 200 OK\r\n"
     << "Content-Type: text/plain\r\n"
     << "Content-Length: " << request.length() << "\r\n\r\n"
     << request;
  std::string expected_response = ss.str();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  EXPECT_EQ(expected_response, obtained_response);
}

TEST_F(SessionTest, ReadBody) {
  std::string body(1500, 'a');
  std::stringstream ss;
  ss << "GET /echo HTTP/1.1\r\n"
     << "Host: localhost\r\n"
     << "User-Agent: curl/7.68.0\r\n"
     << "Content-Length: " << body.length() << "\r\n\r\n"
     << body;
  std::string request = ss.str();

  // client sends request to server through mock socket
  (testSess->socket()).set_input_buffer(request);
  // server starts listening
  testSess->start();

  std::stringstream ss_exp;
  ss_exp << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/plain\r\n"
         << "Content-Length: " << request.length() << "\r\n\r\n"
         << request;
  std::string expected_response = ss_exp.str();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  EXPECT_EQ(expected_response, obtained_response);
}

TEST_F(SessionTest, StaticFile) {
  std::string file_content = "test content";
  std::ofstream f("/usr/src/projects/aite/tests/static_file.txt");
  f << file_content;
  f.close();

  std::string request =
      "GET /static/aite/tests/static_file.txt HTTP/1.1\r\nHost: "
      "localhost\r\nUser-Agent: curl/7.68.0\r\nContent-Length: 0\r\n\r\n";
  // client sends request to server through mock socket
  (testSess->socket()).set_input_buffer(request);
  // server starts listening
  testSess->start();

  std::stringstream ss;
  ss << "HTTP/1.1 200 OK\r\n"
     << "Content-Type: text/plain\r\n"
     << "Content-Length: " << file_content.length() << "\r\n\r\n"
     << file_content;
  std::string expected_response = ss.str();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  EXPECT_EQ(expected_response, obtained_response);
}