#include "server_utils.h"
#include "gtest/gtest.h"

TEST(ConfigSemanticsTest, parsePortFound) {
  std::string file = "basic_config";
  const char *file_name = file.c_str();
  size_t port = ParsePortNumber(file_name);
  EXPECT_EQ(8080, port);
}
TEST(ConfigSemanticsTest, parsePortNotFound) {
  std::string file = "empty_config";
  const char *file_name = file.c_str();
  size_t port = ParsePortNumber(file_name);
  EXPECT_EQ(-1, port);
}
