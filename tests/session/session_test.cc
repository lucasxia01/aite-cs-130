#include "session.h"
#include "gtest/gtest.h"

class SessionTest : public testing::Test {
protected:
  boost::asio::io_service io_service;
  boost::system::error_code error;
  session *testSess;
  void SetUp(void) { testSess = new session(io_service); }
  void TearDown(void) { delete testSess; }
};

TEST_F(SessionTest, normal) {
  testSess->start();
  sprintf(testSess->data_,
          "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
  size_t length = 46;
  testSess->handle_read_header(error, length);
  EXPECT_EQ((testSess->request_).raw_header_str,
            "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
}
TEST_F(SessionTest, withBody) {
  testSess->start();
  sprintf(testSess->data_, "GET /path HTTP/1.1\r\nConnection: "
                           "keep-alive\r\nContent-Length: 8\r\n\r\nsomethin");
  size_t length = 74;
  testSess->handle_read_header(error, length);
  EXPECT_EQ((testSess->request_).raw_header_str,
            "GET /path HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: "
            "8\r\n\r\n");
  EXPECT_EQ((testSess->request_).raw_body_str, "somethin");
}