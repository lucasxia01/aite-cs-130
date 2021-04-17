#include "request_parser.h"
#include "session.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using ::testing::AtLeast;
using ::testing::Return;

class MockRequestParser : public http::server3::request_parser {
public:
  MOCK_METHOD((boost::tuple<boost::tribool, char *, size_t>), parse,
              (http::server3::request & req, char *begin, char *end),
              (override));
};

class SessionTest : public testing::Test {
protected:
  boost::asio::io_service io_service;
  boost::system::error_code error;
  session *testSess;
  void SetUp(void) { testSess = new session(io_service); }
  void TearDown(void) { delete testSess; }
};

TEST_F(SessionTest, reset) {
  testSess->reset_request_parser();
  EXPECT_EQ(testSess->response_agg_.size(), 0);
}
TEST_F(SessionTest, normal) {
  testSess->start();
  sprintf(testSess->data_,
          "GET /path HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
  size_t length = 46;
  testSess->handle_read_header(error, length);
  EXPECT_TRUE(true);
}
// for some reason when i forgot to give a number for content length, there was
// a bad_alloc call that should've been caught
TEST_F(SessionTest, withBody) {
  testSess->start();
  sprintf(testSess->data_, "GET /path HTTP/1.1\r\nConnection: "
                           "keep-alive\r\nContent-Length: 8\r\n\r\nsomething");
  size_t length = 74;
  testSess->handle_read_header(error, length);
  EXPECT_TRUE(true);
}
TEST_F(SessionTest, invalid) {
  sprintf(testSess->data_,
          "GET /path HTTP/1.\r\nConnection: keep-alive\r\n\r\n");
  size_t length = 45;
  testSess->handle_read_header(error, length);
  EXPECT_TRUE(true);
}
// TODO; figure out how to use mock and how to test servicing reads that are
// more than 1024 bytes
/*
TEST_F(SessionTest, canParse) {
  testSess->request_parser_ = new MockRequestParser;
  EXPECT_CALL(*(testSess->request_parser_), parse(testSess->request_,
testSess->data_, testSess->data_+46)) .Times(AtLeast(1)); testSess->start();
  sprintf(testSess->data_, "GET /path HTTP/1.1\r\nConnection:
keep-alive\r\n\r\n"); size_t length= 46; testSess -> handle_read_header(error,
length);
}
*/

/*
TEST_F(SessionTest, normal) {
  testSess -> start();
  testSess ->reset_request_parser();
  char buffer[2048];
  sprintf(testSess->data_,
  "GET /path HTTP/1.1\r\nHost: 34.82.33.1\r\nConnection:
keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows
NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrom/90.0.4430.72
Safari/537.36\r\nAccept:
text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,m/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nAccept-Encoding:
gzip, deflate\r\nAccept-Language: en-US,en;q=0.9\r\nContent-Length:
1024\r\n\r\n5V7XBROGxqkDCyZQcx8Kvv1PWyOhYAoQbuKcgCCncVwoK7iwgcmzEKbF7fU02F4s5BOvpLolkGkn2oqOfRcvIClgFXv9yFSho270DStrlRFeuxZsWWFHU5vdg3d3sKyjHTebZBMPP5i9aItRLUv0dYQRrgDHzrmnPX8XJQqVGiLaxDtXKA9gxURTZvBhGkQns3ZV55Dxt2efLjIXfTDpQJWx1jcovHZAIfVeID8hhzZO0zlbLeSGIKGASG3mMdROGwM29QcmIkqaBCSa4RAfEEhSJ7rQpQbbFsjR9IeYAPxDvYdtUffhyphHcr0rZm1PWQwMXBRPI0Or99KaIpxNSAUlIAeGTr8ldEdiTLLHNA0zqWcl7MBU3OsVN3F81NuxCCYhgYiqbpylbZsK6fZR4TFL5vSuTHmRWKUE8hC0hVpTnH30EuJfTppD6BGWYU4ChYqzptxXtgyulcomZbmwZS83q4ZrNjrfv5FvNv3POU2rRpoGE7N9hRRIwLrv51fv5iph1OXjJL0QwVCV0qXpVxgsxwCegqHrUKmy2z2X1Sy4jjuQMNG88FnAmcBjoIHb062wXxhkJUJfmXAIZcCUxm1XMoNq8Ep3c4quSkQJzGVHz3mR1c5JFJDhUYrH8ANDDQzVrZL7qTkkLHC8lZXgDPBXMhUOvgS2jONqDo9028NbwJdgZL0D3FhsJkWs42uUhH20e0L86G8gmF8FkK8b4if75w9oKDtoJavgwXUz8m8XJWPUE1pZgtcklfhFIEF6Zbqn5MFkH21q3YhdEf5OTOpSO2wNCRPFPuyAmVimBOfhGnslgIpcQTq1IbnZQzU2SvM16iE3rrox97D34sDlw39tCYX5c7d6VaMbvuTHZF6MCe0G70blmZzvRbOBQn4WPCIkZokd0hzxHaEMHrmhZ5r6dOaPheVD2R8AMzbkbppko8qORUhImh4qpl7tOqntXLjUxaM4UMLYgOHgBPJq2v0IsUFOE7ZKAcufCUd9XVjK9LP38lmSwuNw1StAgXC8");
  size_t length= 1023;
  testSess -> handle_read_header(error, length);
  EXPECT_TRUE(true);
}*/