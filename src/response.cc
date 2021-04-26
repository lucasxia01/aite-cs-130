#include "response.h"

response response::get_stock_response(status_type status) {
  std::string response_body;
  switch (status) {
  case OK:
    response_body = "";
    break;
  case BAD_REQUEST:
    response_body = "Invalid request\n";
    break;
  case NOT_FOUND:
    response_body = "File not found\n";
    break;
  }
  response resp;
  resp.status = status;
  resp.headers.push_back({"Content-Type", "text/plain"});
  resp.headers.push_back(
      {"Content-Length", std::to_string(response_body.length())});
  resp.body = response_body;
  return resp;
}

std::string response::to_string() {
  std::stringstream ss;
  ss << "HTTP/1.1 " << response::get_status_string(status) << "\r\n";
  for (auto &h : headers) {
    ss << h.name << ": " << h.value << "\r\n";
  }
  ss << "\r\n" << body;
  return ss.str();
}

std::string response::get_status_string(status_type status) {
  switch (status) {
  case BAD_REQUEST:
    return "400 Bad Request";
  case NOT_FOUND:
    return "404 Not Found";
  case OK:
  default:
    return "200 OK";
  }
}