#include "request.h"
#include <iostream>
namespace http {
namespace server3 {

void request::reset() {
  method = "";
  uri = "";
  raw_header_str = "";
  raw_body_str = "";
  http_version_major = 0;
  http_version_minor = 0;
  headers.clear();
}

size_t request::get_content_length_header() {
  for (header h : headers) {
    if (!h.name.compare("Content-Length")) {
      try {
        return stoi(h.value);
      } catch (...) {
        LOG_ERROR << "Invalid content length: " << h.value;
        return -1;
      }
    }
  }
  return 0;
}
} // namespace server3
} // namespace http