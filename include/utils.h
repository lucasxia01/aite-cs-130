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


int getPortNumber(const NginxConfig &config);
std::vector<std::string> configLookup(const NginxConfig &config,
                                      std::vector<std::string> block_names,
                                      std::string field_name);

std::string convertToAbsolutePath(std::string);

#endif // UTILS_H