#include "server_utils.h"

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