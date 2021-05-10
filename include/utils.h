#ifndef UTILS_H
#define UTILS_H

#include "config_parser.h"
#include <boost/filesystem.hpp>
#include <filesystem>
#include <map>
#include <set>
#include <vector>
#include <filesystem>
#include <boost/filesystem.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace beast = boost::beast;
namespace http = beast::http;

#define response response<http::string_body>
#define request request<http::string_body>

int getPortNumber(const NginxConfig &config);
std::vector<std::string> configLookup(const NginxConfig &config,
                                      std::vector<std::string> block_names,
                                      std::string field_name);

std::string convertToAbsolutePath(std::string);
  /**
   * generates error page given http status and error description details
   *
   * @param status_code http status code
   * @param message error description message, defaulted to N/A
   * @return html error page http response
   */
http::response show_error_page(http::status status_code,
                                        std::string message = "N/A");

#endif // UTILS_H