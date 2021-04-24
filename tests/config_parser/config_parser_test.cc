#include "config_parser.h"
#include "gtest/gtest.h"
#include <vector>

class NginxConfigParserTest : public testing::Test {
protected:
  NginxConfigParser parser;
  NginxConfig out_config;
};

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
TEST_F(NginxConfigParserTest, NonEscapedConfig) {
  bool success = parser.Parse("not_escaped_config", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(NginxConfigParserTest, ExtraQuote) {
  bool success = parser.Parse("extra_quote_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(NginxConfigParserTest, NoSemiColonOrEndBlockBeforeEOF) {
  bool success = parser.Parse("missing_end_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(NginxConfigParserTest, InvalidBeforeStmtEnd) {
  bool success = parser.Parse("invalid_before_stmt_end_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(NginxConfigParserTest, InvalidBeforeStmtStart) {
  bool success = parser.Parse("invalid_before_stmt_start_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(NginxConfigParserTest, BracketBfStmtEnd) {
  bool success = parser.Parse("bracket_bf_stmt_end_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(NginxConfigParserTest, NormalBfEndBracket) {
  bool success = parser.Parse("normal_bf_end_bracket_config", &out_config);
  EXPECT_FALSE(success);
}
