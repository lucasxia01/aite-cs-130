#include "utils.h"
#include "logger.h"

std::vector<std::string> configLookup(const NginxConfig &config,
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

int getPortNumber(const NginxConfig &config) {
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

std::string convertToAbsolutePath(std::string path) {
  // remove any enclosing quotes
  path = (path[0] == '"' && path[path.length()-1] == '"') ? path.substr(1, path.length() - 2)
                               : path;
  boost::filesystem::path full_path = boost::filesystem::system_complete(path);
  path = full_path.string();
  // simplify '.' and '..' directories
  std::vector<std::string> path_comps;
  for (int i = 0; i < path.length(); i++) {
    std::string dir = "";
    while (i < path.length() && path[i] != '/') {
      dir += path[i];
      i++;
    }
    if (dir == "..") {
      if (path_comps.empty()) {
        LOG_ERROR << "invalid path";
      }
      path_comps.pop_back();
    } else if (dir == "." || dir == "") {
      continue;
    } else {
      path_comps.push_back(dir);
    }
  }
  path = "";
  for (auto dir : path_comps) {
    path += "/" + dir;
  }
  
  return path;
}