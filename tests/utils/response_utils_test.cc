#include "request.h"
#include "response_utils.h"
#include "gtest/gtest.h"
#include <sstream>
#include <string>

class ResponseUtilsTest : public testing::Test {
protected:
  http::server3::request req;
  std::stringstream ss;

  virtual void SetUp() {
    req.raw_header_str = "test_header";
    req.raw_body_str = "test_body";
  }
};

TEST_F(ResponseUtilsTest, testBadResponse) {
  std::string buf = response_utils::prepare_echo_buffer(
      response_utils::status_type::BAD_REQUEST, req);
  ss << "HTTP/1.1 400 Bad Request\r\nContent-Type: "
        "text/plain\r\nContent-Length: "
     << response_utils::INVALID_REQUEST_MESSAGE.length() << "\r\n\r\n"
     << response_utils::INVALID_REQUEST_MESSAGE;
  EXPECT_EQ(buf, ss.str());
}

TEST_F(ResponseUtilsTest, testOkResponse) {
  std::string buf =
      response_utils::prepare_echo_buffer(response_utils::status_type::OK, req);
  std::string expected_msg = "test_headertest_body";
  ss << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
     << expected_msg.length() << "\r\n\r\n"
     << expected_msg;
  EXPECT_EQ(buf, ss.str());
}
