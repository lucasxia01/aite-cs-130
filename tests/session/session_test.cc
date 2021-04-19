#include "mock_socket.h"
#include "request_parser.h"
#include "session.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <sstream>

class SessionTest : public testing::Test {
protected:
  boost::asio::io_service io_service;
  boost::system::error_code error;
  session<mock_socket> *testSess;
  void SetUp(void) { testSess = new session<mock_socket>(io_service); }
  void TearDown(void) { delete testSess; }
};

TEST_F(SessionTest, NoBodySuccess) {
  std::string request = "GET /path HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
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