#include "mock_socket.h"
#include "request_parser.h"
#include "server.h"
#include "session.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdio>
#include <fstream>
#include <sstream>

class SessionTest : public testing::Test {
  std::set<std::string> echo_roots = {"/echo"};
  std::map<std::string, std::string> root_to_base_dir = {
      {"/static", "/usr/src/projects/aite/"}};

protected:
  boost::asio::io_service io_service;
  boost::system::error_code error;
  session<mock_socket> *testSess;
  server *testServer;
  void SetUp(void) {
    testServer = new server(io_service, 8080, echo_roots, root_to_base_dir);
    testSess = new session<mock_socket>(io_service, testServer);
  }
  void TearDown(void) {
    delete testSess;
    delete testServer;
  }
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
  const char *path = "/usr/src/projects/aite/tests/static_test_files/file.txt";
  std::ofstream f(path);
  f << file_content;
  f.close();

  std::string request =
      "GET /static/tests/static_test_files/file.txt HTTP/1.1\r\nHost: "
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

  std::remove(path);
}

TEST_F(SessionTest, NegativeContentLength) {
  std::string request =
      "GET /echo HTTP/1.1\r\nHost: "
      "localhost\r\nUser-Agent: curl/7.68.0\r\nContent-Length: -1\r\n\r\n";
  // client sends request to server through mock socket
  (testSess->socket()).set_input_buffer(request);
  // server starts listening
  testSess->start();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  EXPECT_EQ(response::get_stock_response(response::BAD_REQUEST).to_string(),
            obtained_response);
}

TEST_F(SessionTest, InvalidContentLength) {
  std::string request =
      "GET /echo HTTP/1.1\r\nHost: "
      "localhost\r\nUser-Agent: curl/7.68.0\r\nContent-Length: garbage\r\n\r\n";
  // client sends request to server through mock socket
  (testSess->socket()).set_input_buffer(request);
  // server starts listening
  testSess->start();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  EXPECT_EQ(response::get_stock_response(response::BAD_REQUEST).to_string(),
            obtained_response);
}