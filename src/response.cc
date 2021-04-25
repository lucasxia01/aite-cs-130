#include "response.h"

response response::getStockResponse(status_type status) {
  std::string response_body;
  switch (status) {
  case OK:
    response_body = "";
  case BAD_REQUEST:
    response_body = "Invalid request\n";
  case NOT_FOUND:
    return "File not found\n";
  }
  response resp;
  resp.status = status;
  resp.headers.push_back({"Content-Type", "text/plain"});
  resp.headers.push_back(
      {"Content-Length", std::to_string(response_body.length())});
  resp.body = response_body;
  return resp;
}

std::string response::toString() {
  std::stringstream ss;
  ss << "HTTP/1.1 " << response::getStatusString(status) << "\r\n";
  for (auto &h : headers) {
    ss << h.name << ": " << h.value << "\r\n";
  }
  ss << "\r\n" << body;
  return ss.str();
}

std::string response::getStatusString(status_type status) {
  switch (status) {
  case OK:
    return "200 OK";
  case BAD_REQUEST:
    return "400 Bad Request";
  case NOT_FOUND:
    return "404 Not Found";
  }
}