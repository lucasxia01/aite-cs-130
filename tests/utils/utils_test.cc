#include "utils.h"
#include "gtest/gtest.h"
#include <string.h>
#include <filesystem>

class UtilsTest : public testing::Test {
protected:
  NginxConfigParser parser;
  NginxConfig out_config;
};

TEST_F(UtilsTest, OneBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "8080");
  EXPECT_TRUE(success);
}

TEST_F(UtilsTest, TwoBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server", "stupid"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "80");
  EXPECT_TRUE(success);
}

TEST_F(UtilsTest, ThreeBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server", "garbage", "deepergarbage"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "420");
  EXPECT_TRUE(success);
}

TEST_F(UtilsTest, SecondBlockPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"wtf"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "8000");
  EXPECT_TRUE(success);
}

TEST_F(UtilsTest, WrongBlockPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"thisdoesntexist"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 0);
  EXPECT_TRUE(success);
}

TEST_F(UtilsTest, WrongFieldPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server"};
  std::string field_name = "ucantseeme";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 0);
  EXPECT_TRUE(success);
}

TEST_F(UtilsTest, getPortFound) {
  bool success = parser.Parse("basic_config", &out_config);
  size_t port = getPortNumber(out_config);
  EXPECT_EQ(8080, port);
  EXPECT_TRUE(success);
}
TEST_F(UtilsTest, getPortNotFound) {
  bool success = parser.Parse("empty_config", &out_config);
  size_t port = getPortNumber(out_config);
  EXPECT_EQ(-1, port);
  EXPECT_TRUE(success);
}

TEST(AbsolutePath, relativePath) {
  std::string cwd = std::filesystem::current_path().parent_path();
  std::string expected = cwd + "/path";
  std::string path = "../some_random/.././path";

  std::string res = convertToAbsolutePath(path);
  EXPECT_EQ(res, expected);
}

TEST(AbsolutePath, alreadyAbsolute) {
  std::string path = "/some_random/path";
  std::string res = convertToAbsolutePath(path);
  EXPECT_EQ(res, path);
}
