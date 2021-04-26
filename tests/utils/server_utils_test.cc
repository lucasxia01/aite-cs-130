#include "server_utils.h"
#include "gtest/gtest.h"

class ServerUtilsTest : public testing::Test {
protected:
  NginxConfigParser parser;
  NginxConfig out_config;
};

TEST_F(ServerUtilsTest, OneBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "8080");
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, TwoBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server", "stupid"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "80");
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, ThreeBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server", "garbage", "deepergarbage"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "420");
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, SecondBlockPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"wtf"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 1);
  EXPECT_EQ(field_values[0], "8000");
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, WrongBlockPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"thisdoesntexist"};
  std::string field_name = "port";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 0);
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, WrongFieldPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server"};
  std::string field_name = "ucantseeme";
  std::vector<std::string> field_values =
      configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_values.size(), 0);
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, getPortFound) {
  bool success = parser.Parse("basic_config", &out_config);
  size_t port = getPortNumber(out_config);
  EXPECT_EQ(8080, port);
  EXPECT_TRUE(success);
}
TEST_F(ServerUtilsTest, getPortNotFound) {
  bool success = parser.Parse("empty_config", &out_config);
  size_t port = getPortNumber(out_config);
  EXPECT_EQ(-1, port);
  EXPECT_TRUE(success);
}

TEST_F(ServerUtilsTest, parseConfigFile) {
  int port;
  std::set<std::string> echo_roots;
  std::map<std::string, std::string> root_to_base_dir;
  bool success =
      parseConfigFile("basic_config", port, echo_roots, root_to_base_dir);
  EXPECT_EQ(port, 8080);
  EXPECT_EQ(echo_roots.size(), 2);
  EXPECT_TRUE(echo_roots.find("/echo") != echo_roots.end());
  EXPECT_TRUE(echo_roots.find("/echo2") != echo_roots.end());
  EXPECT_EQ(root_to_base_dir.size(), 2);
  EXPECT_TRUE(root_to_base_dir.find("/static") != root_to_base_dir.end());
  EXPECT_TRUE(root_to_base_dir.find("/static2") != root_to_base_dir.end());
  EXPECT_TRUE(root_to_base_dir.find("/random") == root_to_base_dir.end());
  EXPECT_TRUE(root_to_base_dir.find("/nope") == root_to_base_dir.end());
  EXPECT_TRUE(success);
}
