//
// header.h
// ~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_HEADER_H
#define HTTP_SERVER3_HEADER_H

#include <string>
#include <tuple>

namespace http {
namespace server3 {

struct header {
  std::string name;
  std::string value;
  friend bool operator==(const header &l, const header &r) {
    return std::tie(l.name, l.value) ==
           std::tie(r.name, r.value); // keep the same order
  }
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_HEADER_H