#include "server_utils.h"
#include <iostream>

std::vector<std::string> configLookup(NginxConfig &config,
                                      std::vector<std::string> block_names,
                                      std::string field_name) {
  int len = block_names.size();
  if (len == 0) {
    // look for field name
    for (const auto &statement : config.statements_) {
      // if its a block, it can't be a field
      if (statement->child_block_)
        continue;
      std::vector<std::string> tokens = statement->tokens_;
      if (tokens.size() >= 1 && tokens[0] == field_name) {
        std::vector<std::string> field_values;
        for (unsigned int i = 1; i < tokens.size(); ++i) {
          field_values.push_back(tokens[i]);
        }
        return field_values;
      }
    }
    return {};
  }
  // else we have to look through more layers of blocks
  for (const auto &statement : config.statements_) {
    // check if its a block
    if (statement->child_block_) {
      std::vector<std::string> tokens = statement->tokens_;
      // if it matches current block name, we want to recurse into it
      if (tokens.size() > 0 && tokens[0] == block_names[0]) {
        block_names.erase(block_names.begin());
        return configLookup(*statement->child_block_, block_names, field_name);
      }
    }
  }
  return {};
}

int getPortNumber(NginxConfig config) {
  // we want to look for the port number in the server block
  std::vector<std::string> block_names = {"server"};
  // field name should be port
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(config, block_names, field_name);
  // we did not find a port number
  if (field_values.size() != 1)
    return -1;
  return std::atoi(field_values[0].c_str());
}

std::map<std::string, std::string> getRootToBaseDirMapping(NginxConfig config) {
  std::vector<std::string> block_names = {"server", "static"};
  std::vector<std::string> roots = configLookup(config, block_names, "root");
  std::vector<std::string> base_dirs =
      configLookup(config, block_names, "path");

  std::map<std::string, std::string> root_to_base_dir;

  // if the number of roots is not equal to the number of base_dirs, return
  // empty map
  if (roots.size() != base_dirs.size())
    return root_to_base_dir;
  for (size_t i = 0; i < roots.size(); i++) {
    root_to_base_dir[roots[i]] = base_dirs[i];
  }
  return root_to_base_dir;
}

// return is error code
bool parseConfigFile(const char *file_name, int &port,
                     std::set<std::string> &echo_roots,
                     std::map<std::string, std::string> &root_to_base_dir) {
  NginxConfigParser config_parser;
  NginxConfig config;
  config_parser.Parse(file_name, &config);

  port = getPortNumber(config);
  if (port == -1) {
    std::cerr << "Could not parse a port number from config file\n";
    return false;
  }
  std::vector<std::string> list_echo_roots =
      configLookup(config, {"server", "echo"}, "root");
  echo_roots =
      std::set<std::string>(list_echo_roots.begin(), list_echo_roots.end());
  if (echo_roots.empty()) {
    std::cerr << "Could not parse an echo root\n";
    return false;
  }
  std::cerr << *echo_roots.begin() << std::endl;

  root_to_base_dir = getRootToBaseDirMapping(config);
  if (root_to_base_dir.empty()) {
    std::cerr << "Could not parse a mapping\n";
    return false;
  }
  return true;
}
