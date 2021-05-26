#ifndef MOCK_HTTP_CLIENT_H
#define MOCK_HTTP_CLIENT_H

#include "http_client.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

#define response response<http::string_body>
#define request request<http::string_body>

class mock_http_client : public HttpClient {
private:
  unsigned result_code;
  std::string location;

public:
  mock_http_client(unsigned result_code = 200,
                   std::string location = "trololol.com")
      : result_code(result_code), location(location) {}

  http::response perform_request(const std::string &host,
                                 const std::string &port,
                                 const http::request &req) {
    http::response resp;
    std::string response_content = "SUCCESS";
    resp.result(result_code);
    resp.set(http::field::content_type, "text/html");
    resp.set(http::field::location, location);
    resp.content_length(response_content.length());
    resp.body() = response_content;
    return resp;
  }
};

#endif // HTTP_CLIENT_H