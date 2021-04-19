#ifndef RESPONSE_UTILS_H
#define RESPONSE_UTILS_H

#include <request.h>
#include <sstream>
#include <string>

namespace response_utils {
const int MAX_RESPONSE_HEADER_SIZE = 100;
const std::string INVALID_REQUEST_MESSAGE = "Invalid request\n";

enum status_type { OK = 200, BAD_REQUEST = 400 };

std::string prepare_echo_buffer(status_type status,
                                const http::server3::request &req);
} // namespace response_utils

#endif