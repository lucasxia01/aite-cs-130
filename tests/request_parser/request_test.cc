#include "request.h"
#include "gtest/gtest.h"

namespace http {
namespace server3 {
class RequestTest : public testing::Test {
protected:
  request req;
  virtual void SetUp() {
    req.method = "test_method";
    req.uri = "test_uri";
    req.http_version_major = 1;
    req.http_version_minor = 1;
    req.raw_header_str = "test_header";
    req.raw_body_str = "test_body";
  }
};

TEST_F(RequestTest, testReset) {
  req.reset();
  EXPECT_EQ(req.method, "");
  EXPECT_EQ(req.uri, "");
  EXPECT_EQ(req.http_version_major, 0);
  EXPECT_EQ(req.http_version_minor, 0);
  EXPECT_TRUE(req.headers.empty());
  EXPECT_EQ(req.raw_header_str, "");
  EXPECT_EQ(req.raw_body_str, "");
}

TEST_F(RequestTest, testValidContentLength) {
  req.headers.push_back({"Content-Length", "100"});
  size_t content_length = req.get_content_length_header();
  EXPECT_EQ(content_length, 100);
}

TEST_F(RequestTest, testInvalidContentLength) {
  req.headers.push_back({"Content-Length", "garbage"});
  size_t content_length = req.get_content_length_header();
  EXPECT_EQ(content_length, -1);
}

TEST_F(RequestTest, testNoContentLength) {
  size_t content_length = req.get_content_length_header();
  EXPECT_EQ(content_length, 0);
}
} // namespace server3
} // namespace http