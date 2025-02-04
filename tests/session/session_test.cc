#include "mock_socket.h"
#include "request_handler.h"
#include "request_parser.h"
#include "server.h"
#include "session.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

class SessionTest : public testing::Test {
  std::unique_ptr<NginxConfig> config = std::make_unique<NginxConfig>(
      NginxConfig{{std::make_shared<NginxConfigStatement>(NginxConfigStatement{
          {"server"},
          std::make_unique<NginxConfig>(NginxConfig{
              {std::make_shared<NginxConfigStatement>(
                   NginxConfigStatement{{"port", "80"}, nullptr}),
               std::make_shared<NginxConfigStatement>(NginxConfigStatement{
                   {"location", "/echo", "EchoHandler"},
                   std::make_unique<NginxConfig>(NginxConfig{})}),
               std::make_shared<NginxConfigStatement>(NginxConfigStatement{
                   {"location", "/static", "StaticHandler"},
                   std::make_unique<NginxConfig>(
                       NginxConfig{{std::make_shared<NginxConfigStatement>(
                           NginxConfigStatement{{"root", "/"},
                                                nullptr})}})})}})})}});

protected:
  boost::system::error_code error;
  boost::shared_ptr<session<mock_socket>> testSess;
  boost::shared_ptr<server> testServer;
  void SetUp(void) {
    chdir("../../");

    testServer = boost::make_shared<server>(5, *config);
    testSess = boost::make_shared<session<mock_socket>>(
        testServer->get_io_service(), testServer.get());

  }
};

TEST_F(SessionTest, NoBodySuccess) {
  std::string req = "GET /echo HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
                    "curl/7.68.0\r\nContent-Length: 0\r\n\r\n";
  // client sends request to server through mock socket
  (testSess->socket()).set_input_buffer(req);
  // server starts listening
  testSess->start();

  std::stringstream ss;
  ss << "HTTP/1.1 200 OK\r\n"
     << "Content-Type: text/plain\r\n"
     << "Content-Length: " << req.length() << "\r\n\r\n"
     << req;
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
  std::string req = ss.str();

  // client sends req to server through mock socket
  (testSess->socket()).set_input_buffer(req);
  // server starts listening
  testSess->start();

  std::stringstream ss_exp;
  ss_exp << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/plain\r\n"
         << "Content-Length: " << req.length() << "\r\n\r\n"
         << req;
  std::string expected_response = ss_exp.str();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  EXPECT_EQ(expected_response, obtained_response);
}

TEST_F(SessionTest, StaticFile) {
  std::string file_content = "test content";
  const char *path = "tests/static_test_files/file.txt";
  std::ofstream f(path);
  f << file_content;
  f.close();

  std::string req =
      "GET /static/tests/static_test_files/file.txt HTTP/1.1\r\nHost: "
      "localhost\r\nUser-Agent: curl/7.68.0\r\nContent-Length: 0\r\n\r\n";
  // client sends req to server through mock socket
  (testSess->socket()).set_input_buffer(req);
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
  std::string req = "GET /echo HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
                    "curl/7.68.0\r\nContent-Length: -1\r\n\r\n";
  // client sends req to server through mock socket
  (testSess->socket()).set_input_buffer(req);
  // server starts listening
  testSess->start();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  std::ostringstream expected_response;
  expected_response << show_error_page(
      http::status::bad_request, "Invalid request headers or content length");
  EXPECT_EQ(expected_response.str(), obtained_response);
}

TEST_F(SessionTest, InvalidContentLength) {
  std::string req = "GET /echo HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
                    "curl/7.68.0\r\nContent-Length: garbage\r\n\r\n";
  // client sends req to server through mock socket
  (testSess->socket()).set_input_buffer(req);
  // server starts listening
  testSess->start();

  // get response from server through mock socket
  std::string obtained_response = (testSess->socket()).get_output_buffer();
  std::ostringstream expected_response;
  expected_response << show_error_page(
      http::status::bad_request, "Invalid request headers or content length");
  EXPECT_EQ(expected_response.str(), obtained_response);
}