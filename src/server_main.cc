//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <logger.h>
#include <vector>

#include "server.h"
#include "server_utils.h"

int main(int argc, char *argv[]) {
  Logger::init();
  try {
    if (argc != 2) {
      LOG_FATAL << "Usage: server <path_to_config_file>";
      return 1;
    }
    boost::asio::io_service io_service;
    LOG_DEBUG << "Attempting to parse port number";
    int port_num = ParsePortNumber(argv[1]);
    if (port_num == -1) {
      LOG_FATAL << "Could not parse a port number from config file";
      return 2;
    }
    LOG_DEBUG << "Parsed port number: " << port_num;
    server s(io_service, port_num);
    LOG_INFO << "Running server on port number: " << port_num;
    io_service.run();
  } catch (std::exception &e) {
    LOG_ERROR << "Exception: " << e.what();
  }

  return 0;
}
