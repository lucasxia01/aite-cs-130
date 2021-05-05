#include "request_parser.h"
#include "gtest/gtest.h"
#include <stdio.h>
#include <string.h>

class RequestParserTest : public testing::Test {
protected:
  request_parser rp;
  boost::tribool request_parse_result;
  char *raw_request;
  char *header_read_end;
  http::request parsed_request;
  virtual void SetUp() { raw_request = new char[100]; }
  virtual void TearDown() { delete raw_request; }
};

// valid request with single header value
TEST_F(RequestParserTest, simpleValid) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));

  EXPECT_TRUE(request_parse_result);
  EXPECT_EQ(parsed_request.method_string(), "GET");
  EXPECT_EQ(parsed_request.target().to_string(), "/path");
  EXPECT_EQ(parsed_request.version(), 11);
  EXPECT_EQ(parsed_request[http::field::connection].to_string(), "keep-alive");
  EXPECT_EQ(header_read_end, raw_request + strlen(raw_request));
}

// request with indeterminate result(not all bytes from raw_request were read)
TEST_F(RequestParserTest, indeterminate) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
  // don't read in all the bytes to get to indeterminate result
  boost::tie(request_parse_result, header_read_end) = rp.parse(
      parsed_request, raw_request, raw_request + strlen(raw_request) - 6);
  // check if result is indeterminate
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
  EXPECT_EQ(header_read_end, raw_request + strlen(raw_request) - 6);
}

// check state correctly gets reset to the start when reset is called
TEST_F(RequestParserTest, resetState) {
  sprintf(raw_request,
          "GET /path HTTP/10.10\r\nConnection: keep-alive\r\n\r\n");
  rp.reset();
  EXPECT_EQ(rp.state_, 1); // method_start state = 1
}

// check that multiline headers are valid (use ' ' or '\t' for multiline)
TEST_F(RequestParserTest, multilineHeaderEmpty) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n \r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}

// check that multiline headers are valid with text on the next line
TEST_F(RequestParserTest, multilineHeaderText) {
  sprintf(raw_request,
          "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n rest\r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}

// check double multiline headers are valid
TEST_F(RequestParserTest, doubleMultilineHeader) {
  sprintf(raw_request,
          "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n \t\r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_TRUE(boost::indeterminate(request_parse_result));
}

// check multiline with ctl (ASCII c>=0 && c<=31 || c ==127) is invalid
TEST_F(RequestParserTest, multilineHeaderCtl) {
  sprintf(raw_request,
          "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n \b\r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Check that special characters are invalid starts to headers
TEST_F(RequestParserTest, specialCharStartHeader) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\n(Connection: keep-alive\r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Check that special characters cannot be in header names
TEST_F(RequestParserTest, specialCharInHeader) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nC(onnection: keep-alive\r\n");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// uri cannot start with ctl (ASCII c>=0 && c<=31 || c ==127)
TEST_F(RequestParserTest, ctlStartUri) {
  sprintf(raw_request, "GET \b/path");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected 'H'
TEST_F(RequestParserTest, notHTTP1) {
  sprintf(raw_request, "GET /path A");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected 'T'
TEST_F(RequestParserTest, notHTTP2) {
  sprintf(raw_request, "GET /path HA");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected 'T'
TEST_F(RequestParserTest, notHTTP3) {
  sprintf(raw_request, "GET /path HTA");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected 'P'
TEST_F(RequestParserTest, notHTTP4) {
  sprintf(raw_request, "GET /path HTTA");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected '/'
TEST_F(RequestParserTest, noSlashHTTP) {
  sprintf(raw_request, "GET /path HTTPA");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected number
TEST_F(RequestParserTest, httpMajorNotDigit) {
  sprintf(raw_request, "GET /path HTTP/A");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected '.'
TEST_F(RequestParserTest, httpNoDot) {
  sprintf(raw_request, "GET /path HTTP/1A");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected number
TEST_F(RequestParserTest, httpMinorNotDigit) {
  sprintf(raw_request, "GET /path HTTP/1.A");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected '\r'
TEST_F(RequestParserTest, httpMinorNoCRLF) {
  sprintf(raw_request, "GET /path HTTP/1.1A");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected '\n'
TEST_F(RequestParserTest, noLFAfterCR) {
  sprintf(raw_request, "GET /path HTTP/1.1\rA");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
}

// Expected space after header_name:
TEST_F(RequestParserTest, noSpaceBeforeHeaderValue) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nConnection:st");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
  // expects ' ' when reading 's' --> returns false and doesn't read 't'
  EXPECT_EQ(header_read_end, raw_request + strlen(raw_request) - 1);
}

// ctl (ASCII c>=0 && c<=31 || c ==127) invalid in header value
TEST_F(RequestParserTest, noCtlInHeaderValue) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nConnection: s\b");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
  EXPECT_EQ(header_read_end, raw_request + strlen(raw_request));
}

// Expected \n after \r
TEST_F(RequestParserTest, noNewLineAfterCR2) {
  sprintf(raw_request, "GET /path HTTP/1.1\r\nConnection: s\rABC");
  boost::tie(request_parse_result, header_read_end) =
      rp.parse(parsed_request, raw_request, raw_request + strlen(raw_request));
  EXPECT_FALSE(request_parse_result);
  // Expects \n when reading A --> return false and doesn't read B or C
  EXPECT_EQ(header_read_end, raw_request + strlen(raw_request) - 2);
}