//
// request_parser.h
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_REQUEST_PARSER_H
#define HTTP_SERVER3_REQUEST_PARSER_H

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
// #include <iostream> // For debugging parser

namespace http {
namespace server3 {

struct request;

/// Parser for incoming requests.
class request_parser {
public:
  /// Construct ready to parse the request method.
  request_parser();

  /// Reset to initial parser state.
  void reset();

  /// Parse some data. The tribool return value is true when a complete request
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. The InputIterator return value indicates how much of the
  /// input has been consumed.
  virtual boost::tuple<boost::tribool, char *> parse(request &req, char *begin,
                                                     char *end);

  /// Handle the next character of input.
  boost::tribool consume(request &req, char input);

  /// Check if a byte is an HTTP character.
  static bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static bool is_digit(int c);

  /// The current state of the parser.
  enum state {
    method_start = 1,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } state_;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_PARSER_H