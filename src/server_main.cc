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

void close_handler(const boost::system::error_code &, int) {
  LOG_INFO << "Server closed";
  exit(1);
}

int main(int argc, char *argv[]) {
  Logger::init();
  try {
    if (argc != 2) {
      LOG_FATAL << "Usage: server <path_to_config_file>";
      return 1;
    }
    boost::asio::io_service io_service;

    int port_num;
    std::set<std::string> echo_roots;
    std::map<std::string, std::string> root_to_base_dir;
    LOG_DEBUG << "Attempting to parse config file";
    parseConfigFile(argv[1], port_num, echo_roots, root_to_base_dir);

    LOG_INFO << "Successfully parsed port number: " << port_num;
    server s(io_service, port_num);
    LOG_INFO << "Running server on port number: " << port_num;

    boost::asio::signal_set signals(io_service, SIGINT);
    signals.async_wait(close_handler);
    io_service.run();
  } catch (std::exception &e) {
    LOG_ERROR << "Exception: " << e.what();
  }

  return 0;
}
