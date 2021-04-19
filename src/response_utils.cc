#include "response_utils.h"

namespace response_utils {
std::string prepare_echo_buffer(status_type status,
                                const http::server3::request &req) {
  std::string response_status_message;
  std::string response_body;
  switch (status) {
  case OK:
    response_status_message = "200 OK\0";
    response_body = req.raw_header_str + req.raw_body_str;
    break;
  case BAD_REQUEST:
    response_status_message = "400 Bad Request\0";
    response_body = INVALID_REQUEST_MESSAGE;
    break;
  }
  int buf_size = MAX_RESPONSE_HEADER_SIZE + response_body.length();
  std::stringstream ss;
  ss << "HTTP/1.1 " << response_status_message
     << "\r\nContent-Type: text/plain\r\nContent-Length: "
     << response_body.length() << "\r\n\r\n"
     << response_body;
  return ss.str();
}
} // namespace response_utils