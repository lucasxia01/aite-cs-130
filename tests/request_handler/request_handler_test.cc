#include "request_handler.h"
#include "gtest/gtest.h"
#include <optional>

NginxConfigParser config_parser;
NginxConfig static_config;
bool success1 = config_parser.Parse("static_file_config", &static_config);
NginxConfig empty_config;
bool success2 = config_parser.Parse("empty_config", &empty_config);
class StaticFileRequestHandlerTest : public testing::Test {
protected:
  StaticFileRequestHandler *static_file_request_handler;
  void SetUp() override {
    static_file_request_handler =
        new StaticFileRequestHandler("/files", static_config);
  }
  void TearDown() override { delete static_file_request_handler; }
};

TEST_F(StaticFileRequestHandlerTest, StaticFileFound) {
  std::string root;
  http::request req;
  req.target("/files/static_file.txt");
  std::stringstream received_response;
  received_response << static_file_request_handler->handle_request(req);

  std::stringstream expected_response;

  expected_response << "HTTP/1.1 200 OK"
                    << "\r\n"
                    << "Content-Type: text/plain\r\n"
                    << "Content-Length: 12"
                    << "\r\n\r\n"
                    << "test content";

  // get response from server through mock socket
  EXPECT_EQ(expected_response.str(), received_response.str());
}

TEST_F(StaticFileRequestHandlerTest, StaticFileNotFound) {
  std::string root;
  http::request req;
  req.target("/static/nope/file.txt");
  std::stringstream received_response;
  received_response << static_file_request_handler->handle_request(req);

  std::stringstream expected_response_header;
  std::stringstream expected_response_body;
  expected_response_body
      << "<!DOCTYPE html><html>"
      << "<head><title>Error</title></head><body>"
      << "<h1>Not Found Error</h1>"
      << "<p>"
      << "Description: File /static/nope/file.txt not found</p>"
      << "</body></html>";
  expected_response_header << "HTTP/1.1 404 Not Found"
                           << "\r\n"
                           << "Content-Type: text/html\r\n"
                           << "Content-Length: "
                           << expected_response_body.str().length()
                           << "\r\n\r\n";
  std::string expected_response =
      expected_response_header.str() + expected_response_body.str();

  // get response from server through mock socket
  EXPECT_EQ(expected_response, received_response.str());
}

class EchoRequestHandlerTest : public testing::Test {
protected:
  EchoRequestHandler *echo_request_handler;

  void SetUp() override {
    echo_request_handler = new EchoRequestHandler("/echo", empty_config);
  }
  void TearDown() override { delete echo_request_handler; }
};

TEST_F(EchoRequestHandlerTest, EchoResponse) {
  std::string root;
  http::request req;
  req.method(http::verb::get);
  req.target("/echo");
  std::stringstream received_response;
  received_response << echo_request_handler->handle_request(req);

  std::stringstream expected_response;
  expected_response << "HTTP/1.1 200 OK"
                    << "\r\n"
                    << "Content-Type: text/plain\r\n"
                    << "Content-Length: 22\r\n\r\n"
                    << "GET /echo HTTP/1.1\r\n\r\n";

  // get response from server through mock socket
  EXPECT_EQ(expected_response.str(), received_response.str());
}

class NotFoundRequestHandlerTest : public testing::Test {
protected:
  NotFoundRequestHandler *not_found_request_handler;

  void SetUp() override {
    not_found_request_handler = new NotFoundRequestHandler("/", empty_config);
  }
  void TearDown() override { delete not_found_request_handler; }
};

TEST_F(NotFoundRequestHandlerTest, NotFoundResponse) {
  std::string root;
  http::request req;
  req.method(http::verb::get);
  req.target("/doesnotexist/something.txt");
  std::stringstream received_response;
  received_response << not_found_request_handler->handle_request(req);

  std::stringstream expected_response_header;
  std::stringstream expected_response_body;
  expected_response_body
      << "<!DOCTYPE html><html>"
      << "<head><title>Error</title></head><body>"
      << "<h1>Not Found Error</h1>"
      << "<p>"
      << "Description: Uri path /doesnotexist/something.txt not found</p>"
      << "</body></html>";
  expected_response_header << "HTTP/1.1 404 Not Found"
                           << "\r\n"
                           << "Content-Type: text/html\r\n"
                           << "Content-Length: "
                           << expected_response_body.str().length()
                           << "\r\n\r\n";
  std::string expected_response =
      expected_response_header.str() + expected_response_body.str();
  // get response from server through mock socket
  EXPECT_EQ(expected_response, received_response.str());
}

class StatusRequestHandlerTest : public testing::Test {
protected:
  StatusRequestHandler *status_request_handler;
  boost::asio::io_service io_service;
  NginxConfig config;
  server *s;

  void SetUp() override {
    status_request_handler = new StatusRequestHandler("/status", empty_config);
    s = new server(io_service, config);
  }
  void TearDown() override {
    delete status_request_handler;
    delete s;
    ;
  }
};

TEST_F(StatusRequestHandlerTest, StatusResponse) {
  status_request_handler->initStatus(s);
  std::string root;
  http::request req;
  req.method(http::verb::get);
  req.target("/status");
  std::stringstream received_response;
  received_response << status_request_handler->handle_request(req);

  std::stringstream expected_response_header;
  std::stringstream expected_response_body;
  expected_response_body
      << "<!DOCTYPE html><html><link rel ='preconnect' "
         "href='https://fonts.gstatic.com'>"
         "<link href"
         "='https://fonts.googleapis.com/css2?family=Handlee&display=swap' "
         "rel='stylesheet'>"
         "<style> body{ font-family: 'Handlee', cursive; font-size: 20px; "
         "display: block } "
         ".code1{ background-color: cyan } "
         ".code2{ background-color: green } "
         ".code3{ background-color: yellow } "
         ".code4{ background-color: red } "
         ".code5{ background-color: orange } "
         "td,th{ text-align: center; padding-right: 12px; border: 1px solid "
         "black } "
         "td{ max-width: 200px; word-wrap: break-word } "
         "table{ border: 1px solid black } "
         "#handlers-list { display: flex; flex-wrap: wrap } "
         ".handler{ margin-right:12px } "
         "</style><head><title>Status Page</title></head><body><h1>Status "
         "Page</h1>"
         "<b>Total Number of Requests Served : </b>0<br><br></body></html>";
  expected_response_header << "HTTP/1.1 200 OK"
                           << "\r\n"
                           << "Content-Type: text/html\r\n"
                           << "Content-Length: "
                           << expected_response_body.str().length()
                           << "\r\n\r\n";
  std::string expected_response =
      expected_response_header.str() + expected_response_body.str();
  // get response from server through mock socket
  EXPECT_EQ(expected_response, received_response.str());
}
