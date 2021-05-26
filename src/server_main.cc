//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <cstdlib>
#include <logger.h>
#include <vector>

#include "server.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  Logger::init();
  try {
    if (argc != 3) {
      LOG_FATAL << "Usage: server <num_threads> <path_to_config_file>";
      return 1;
    }

    int num_threads;
    try {
      num_threads = std::stoi(argv[1]);
    } catch (...) {
      LOG_FATAL << "Usage: server <num_threads> <path_to_config_file>\nInvalid "
                   "number of threads.";
      return 1;
    }

    NginxConfigParser config_parser;
    NginxConfig config;
    LOG_DEBUG << "Attempting to parse config file";
    bool success = config_parser.Parse(argv[2], &config);
    if (!success) {
      LOG_ERROR << "Failed to parse config file";
      return 1;
    }

    server s(num_threads, config);
    s.run();
  } catch (std::exception &e) {
    LOG_ERROR << "Exception: " << e.what();
  }

  return 0;
}
