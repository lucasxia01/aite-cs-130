#ifndef RESPONSE_H
#define RESPONSE_H

#include "header.h"
#include <sstream>
#include <string>
#include <vector>

struct response {
  enum status_type { OK = 200, BAD_REQUEST = 400, NOT_FOUND = 404 } status;

  std::vector<http::server3::header> headers;
  std::string body;

  static response get_stock_response(status_type);
  static std::string get_status_string(status_type);
  std::string to_string();
};

#endif // RESPONSE_H