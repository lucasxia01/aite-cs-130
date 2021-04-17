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

  friend bool operator==(const request &l, const request &r) {
    return std::tie(l.method, l.uri, l.http_version_major, l.http_version_minor,
                    l.headers) ==
           std::tie(r.method, r.uri, r.http_version_major, r.http_version_minor,
                    r.headers); // keep the same order
  }
};
} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_H