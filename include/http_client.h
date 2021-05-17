#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <boost/beast.hpp>
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

#define response response<http::string_body>
#define request request<http::string_body>

class HttpClient {
public:
  HttpClient();
  virtual http::response perform_request(const std::string &host,
                                         const std::string &port,
                                         const http::request &req);
};
#endif // HTTP_CLIENT_H