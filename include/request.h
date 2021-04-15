//
// request.h
// ~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_H
#define HTTP_SERVER3_REQUEST_H

#include "header.h"
#include <string>
#include <vector>

namespace http {
namespace server3 {

/// A request received from a client.
struct request {
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_H