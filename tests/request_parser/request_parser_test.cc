#include "request.h"
#include "request_parser.h"
#include "gtest/gtest.h"
#include <stdio.h>
#include <string.h>

namespace http {
namespace server3 {
class RequestParserTest : public testing::Test {
protected:
  request_parser rp;
  header h;
  boost::tribool request_parse_result;
  size_t header_bytes_parsed;
  std::vector<header> headers;
  char *req;
  request res = {};
  request expectedRes = {};
  virtual void SetUp() { req = new char[100]; }
  virtual void TearDown() { delete req; }
};
// valid request with single header value
TEST_F(RequestParserTest, simpleValid) {
  h = {
      "Connection",
      "keep-alive",
  };
  headers.push_back(h);
  expectedRes = {
      "GET", "/path", 1, 1, headers,
  };
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 46);
  EXPECT_TRUE(request_parse_result);
  EXPECT_EQ(res.method, expectedRes.method);
  EXPECT_EQ(res.uri, expectedRes.uri);
  EXPECT_EQ(res.http_version_major, expectedRes.http_version_major);
  EXPECT_EQ(res.http_version_minor, expectedRes.http_version_minor);
  EXPECT_EQ(res.headers[0], expectedRes.headers[0]);
  EXPECT_EQ(res.headers[res.headers.size() - 1],
            expectedRes.headers[expectedRes.headers.size() - 1]);
}
// request with indeterminate result(not all bytes from req were read +40
// instead of +46)
TEST_F(RequestParserTest, indeterminate) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
  // don't read in all the bytes to get to indeterminate result
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 40);
  // check if result is indeterminate
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}
// Check HTTP with doubleDigit version numbers are valid
TEST_F(RequestParserTest, versionsDoubleDigit) {
  h = {
      "Connection",
      "keep-alive",
  };
  headers.push_back(h);
  expectedRes = {
      "GET", "/path", 10, 10, headers,
  };
  sprintf(req, "GET /path HTTP/10.10\r\nConnection: keep-alive\r\n\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 48);
  EXPECT_TRUE(request_parse_result);
  EXPECT_EQ(res.method, expectedRes.method);
  EXPECT_EQ(res.uri, expectedRes.uri);
  EXPECT_EQ(res.http_version_major, expectedRes.http_version_major);
  EXPECT_EQ(res.http_version_minor, expectedRes.http_version_minor);
  EXPECT_EQ(res.headers[0], expectedRes.headers[0]);
  EXPECT_EQ(res.headers[res.headers.size() - 1],
            expectedRes.headers[expectedRes.headers.size() - 1]);
}
// check state correctly gets reset to the start when reset is called
TEST_F(RequestParserTest, resetState) {
  sprintf(req, "GET /path HTTP/10.10\r\nConnection: keep-alive\r\n\r\n");
  rp.reset();
  EXPECT_EQ(rp.state_, 1); // method_start state = 1
}
// check that multiline headers are valid (use ' ' or '\t' for multiline)
TEST_F(RequestParserTest, multilineHeaderEmpty) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n \r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 47);
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}
// check that multiline headers are valid with text on the next line
TEST_F(RequestParserTest, multilineHeaderText) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n rest\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 51);
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}
// check double multiline headers are valid
TEST_F(RequestParserTest, doubleMultilineHeader) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n \t\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 48);
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}
// check multiline with ctl (ASCII c>=0 && c<=31 || c ==127) is invalid
TEST_F(RequestParserTest, multilineHeaderCtl) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n \b\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 48);
  EXPECT_FALSE(request_parse_result);
}
// Check that special characters are invalid starts to headers
TEST_F(RequestParserTest, specialCharStartHeader) {
  sprintf(req, "GET /path HTTP/1.1\r\n(Connection: keep-alive\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 47);
  EXPECT_FALSE(request_parse_result);
}
// Check that special characters cannot be in header names
TEST_F(RequestParserTest, specialCharInHeader) {
  sprintf(req, "GET /path HTTP/1.1\r\nC(onnection: keep-alive\r\n");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 47);
  EXPECT_FALSE(request_parse_result);
}
// uri cannot start with ctl (ASCII c>=0 && c<=31 || c ==127)
TEST_F(RequestParserTest, ctlStartUri) {
  sprintf(req, "GET \b/path");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 10);
  EXPECT_FALSE(request_parse_result);
}
// Expected 'H'
TEST_F(RequestParserTest, notHTTP1) {
  sprintf(req, "GET /path A");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 11);
  EXPECT_FALSE(request_parse_result);
}
// Expected 'T'
TEST_F(RequestParserTest, notHTTP2) {
  sprintf(req, "GET /path HA");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 12);
  EXPECT_FALSE(request_parse_result);
}
// Expected 'T'
TEST_F(RequestParserTest, notHTTP3) {
  sprintf(req, "GET /path HTA");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 13);
  EXPECT_FALSE(request_parse_result);
}
// Expected 'P'
TEST_F(RequestParserTest, notHTTP4) {
  sprintf(req, "GET /path HTTA");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 14);
  EXPECT_FALSE(request_parse_result);
}
// Expected '/'
TEST_F(RequestParserTest, noSlashHTTP) {
  sprintf(req, "GET /path HTTPA");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 15);
  EXPECT_FALSE(request_parse_result);
}
// Expected number
TEST_F(RequestParserTest, httpMajorNotDigit) {
  sprintf(req, "GET /path HTTP/A");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 16);
  EXPECT_FALSE(request_parse_result);
}
// Expected '.'
TEST_F(RequestParserTest, httpNoDot) {
  sprintf(req, "GET /path HTTP/1A");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 17);
  EXPECT_FALSE(request_parse_result);
}
// Expected number
TEST_F(RequestParserTest, httpMinorNotDigit) {
  sprintf(req, "GET /path HTTP/1.A");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 18);
  EXPECT_FALSE(request_parse_result);
}
// Expected '\r'
TEST_F(RequestParserTest, httpMinorNoCRLF) {
  sprintf(req, "GET /path HTTP/1.1A");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 19);
  EXPECT_FALSE(request_parse_result);
}
// Expected '\n'
TEST_F(RequestParserTest, noLFAfterCR) {
  sprintf(req, "GET /path HTTP/1.1\rA");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 20);
  EXPECT_FALSE(request_parse_result);
}
// Expected space after header_name:
TEST_F(RequestParserTest, noSpaceBeforeHeaderValue) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection:st");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 33);
  EXPECT_FALSE(request_parse_result);
  // checks that not all bytes parsed if returned false
  EXPECT_EQ(header_bytes_parsed, 32);
}
// ctl (ASCII c>=0 && c<=31 || c ==127) invalid in header value
TEST_F(RequestParserTest, noCtlInHeaderValue) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: s\b");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 34);
  EXPECT_FALSE(request_parse_result);
  EXPECT_EQ(header_bytes_parsed, 34);
}
// Expected \n after \r
TEST_F(RequestParserTest, noNewLineAfterCR2) {
  sprintf(req, "GET /path HTTP/1.1\r\nConnection: s\rABC");
  boost::tie(request_parse_result, boost::tuples::ignore, header_bytes_parsed) =
      rp.parse(res, req, req + 37);
  EXPECT_FALSE(request_parse_result);
  EXPECT_EQ(header_bytes_parsed, 35);
}

} // namespace server3
} // namespace http