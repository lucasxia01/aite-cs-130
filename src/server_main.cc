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
#include <vector>

#include "config_parser.h"
#include "server.h"

int ParsePortNumber(const char *file_name) {
  NginxConfigParser config_parser;
  NginxConfig config;
  config_parser.Parse(file_name, &config);
  // we want to look for the port number in the server block
  std::vector<std::string> block_names = {"server"};
  // field name should be port
  std::string field_name = "port";
  std::string field_value =
      config_parser.configLookup(config, block_names, field_name);
  std::cerr << field_value << '\n';
  // we did not find a port number
  if (field_value == "")
    return -1;
  using namespace std;
  return atoi(field_value.c_str());
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    int port_num = ParsePortNumber(argv[1]);
    if (port_num == -1) {
      std::cerr << "Could not parse a port number from config file\n";
      return 2;
    }
    printf("Parsed port number %d\n", port_num);
    server s(io_service, port_num);

    io_service.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
