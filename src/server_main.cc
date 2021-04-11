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

#include "server.h"
#include "config_parser.h"

int ParsePortNumber(const char* file_name) {
  NginxConfigParser config_parser;
  NginxConfig config;
  config_parser.Parse(file_name, &config);
  printf("%s", config.ToString().c_str());
  return -1;
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    int port_num = ParsePortNumber(argv[1]);
    if (port_num == -1) 
    {
      std::cerr << "Could not parse a port number from config file\n";
      return 2;
    }
    server s(io_service, port_num);

    io_service.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
