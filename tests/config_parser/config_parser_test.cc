#include "config_parser.h"
#include "gtest/gtest.h"
#include <vector>

class NginxConfigParserTest : public testing::Test {
protected:
  NginxConfigParser parser;
  NginxConfig out_config;
};

TEST_F(NginxConfigParserTest, OneBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server"};
  std::string field_name = "port";
  std::string field_value =
      parser.configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_value, "80");
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, TwoBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server", "stupid"};
  std::string field_name = "port";
  std::string field_value =
      parser.configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_value, "8080");
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, ThreeBlockDeepPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server", "garbage", "deepergarbage"};
  std::string field_name = "port";
  std::string field_value =
      parser.configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_value, "420");
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, SecondBlockPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"wtf"};
  std::string field_name = "port";
  std::string field_value =
      parser.configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_value, "8000");
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, WrongBlockPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"thisdoesntexist"};
  std::string field_name = "port";
  std::string field_value =
      parser.configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_value, "");
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, WrongFieldPortLookup) {
  bool success = parser.Parse("basic_config", &out_config);
  std::vector<std::string> block_names = {"server"};
  std::string field_name = "ucantseeme";
  std::string field_value =
      parser.configLookup(out_config, block_names, field_name);
  EXPECT_EQ(field_value, "");
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, SimpleConfig) {
  bool success = parser.Parse("example_config", &out_config);
  EXPECT_TRUE(success);
}
// config with an open brace but no closing brace
TEST_F(NginxConfigParserTest, MismatchedConfig1) {
  bool success = parser.Parse("mismatched_config_1", &out_config);
  EXPECT_FALSE(success);
}
// config with a closing brace before any open braces
TEST_F(NginxConfigParserTest, MismatchedConfig2) {
  bool success = parser.Parse("mismatched_config_2", &out_config);
  EXPECT_FALSE(success);
}
// config with nested blocks and consecutive closing braces
TEST_F(NginxConfigParserTest, NestedConfig) {
  bool success = parser.Parse("nested_config", &out_config);
  EXPECT_TRUE(success);
}
// config with comments that make sure comments are ignored
TEST_F(NginxConfigParserTest, CommentsConfig) {
  bool success = parser.Parse("comments_config", &out_config);
  EXPECT_TRUE(success);
}
// config with empty content in braces
TEST_F(NginxConfigParserTest, EmptyBracesConfig) {
  bool success = parser.Parse("empty_braces_config", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(NginxConfigParserTest, EmptyConfig) {
  bool success = parser.Parse("empty_config", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(NginxConfigParserTest, QuoteNoSpaceConfigFail) {
  bool success = parser.Parse("quoted_no_space_config_fail", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(NginxConfigParserTest, QuoteNoSpaceConfigPass) {
  bool success = parser.Parse("quoted_no_space_config_pass", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(NginxConfigParserTest, BackslashConfig) {
  bool success = parser.Parse("backslash_config", &out_config);
  EXPECT_TRUE(success);
}