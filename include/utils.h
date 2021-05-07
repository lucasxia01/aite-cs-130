#ifndef UTILS_H
#define UTILS_H

#include "config_parser.h"
#include <boost/filesystem.hpp>
#include <filesystem>
#include <map>
#include <set>
#include <vector>

int getPortNumber(NginxConfig config);
std::vector<std::string> configLookup(const NginxConfig &config,
                                      std::vector<std::string> block_names,
                                      std::string field_name);
std::string getEchoRoot(NginxConfig config);
std::map<std::string, std::string> getRootToBaseDirMapping(NginxConfig config);
bool parseConfigFile(const char *file_name, int &port,
                     std::set<std::string> &echo_roots,
                     std::map<std::string, std::string> &root_to_base_dir);

std::string convertToAbsolutePath(std::string);

#endif // UTILS_H