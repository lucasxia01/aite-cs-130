#include "header.h"
#include <sstream>
#include <string>
#include <vector>

struct response {
  enum status_type { OK = 200, BAD_REQUEST = 400 } status;

  std::vector<http::server3::header> headers;
  std::string body;

  static response getStockResponse(status_type);
  static std::string getStatusString(status_type);
  std::string toString();
};