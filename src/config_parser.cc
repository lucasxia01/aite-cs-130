// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c
#include "config_parser.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto &statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}
std::string NginxConfigStatement::ToString(int depth) {
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i) {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr) {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i) {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  } else {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}
const char *NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
  case TOKEN_TYPE_START:
    return "TOKEN_TYPE_START";
  case TOKEN_TYPE_NORMAL:
    return "TOKEN_TYPE_NORMAL";
  case TOKEN_TYPE_START_BLOCK:
    return "TOKEN_TYPE_START_BLOCK";
  case TOKEN_TYPE_END_BLOCK:
    return "TOKEN_TYPE_END_BLOCK";
  case TOKEN_TYPE_COMMENT:
    return "TOKEN_TYPE_COMMENT";
  case TOKEN_TYPE_STATEMENT_END:
    return "TOKEN_TYPE_STATEMENT_END";
  case TOKEN_TYPE_EOF:
    return "TOKEN_TYPE_EOF";
  case TOKEN_TYPE_ERROR:
    return "TOKEN_TYPE_ERROR";
  case TOKEN_TYPE_QUOTED_STRING:
    return "TOKEN_TYPE_QUOTED_STRING";
  default:
    return "Unknown token type";
  }
}

std::string
NginxConfigParser::configLookup(NginxConfig &config,
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
        std::string field_value = "";
        for (unsigned int i = 1; i < tokens.size(); ++i) {
          field_value.append(tokens[i]);
          if (i != 1) {
            field_value.append(" ");
          }
        }
        return field_value;
      }
    }
    return "";
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
  return "";
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream *input,
                                                           std::string *value) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) {
      break;
    }
    switch (state) {
    case TOKEN_STATE_INITIAL_WHITESPACE:
      switch (c) {
      case '{':
        *value = c;
        return TOKEN_TYPE_START_BLOCK;
      case '}':
        *value = c;
        return TOKEN_TYPE_END_BLOCK;
      case '#':
        *value = c;
        state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
        continue;
      case '"':
        *value = c;
        state = TOKEN_STATE_DOUBLE_QUOTE;
        continue;
      case '\'':
        *value = c;
        state = TOKEN_STATE_SINGLE_QUOTE;
        continue;
      case ';':
        *value = c;
        return TOKEN_TYPE_STATEMENT_END;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        continue;
      default:
        *value += c;
        state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
        continue;
      }
    case TOKEN_STATE_SINGLE_QUOTE:
      // RESOLVED: the end of a quoted token should be followed by whitespace.
      // RESOLVED: Maybe also define a QUOTED_STRING token type.
      // RESOLVED: Allow for backslash-escaping within strings.
      *value += c;
      if (c == '\\') {
        const char next_c = input->get();
        if (!input->good()) {
          break;
        }
        switch (next_c) {
        case 't':
          *value += '\t';
          continue;
        case 'n':
          *value += '\n';
          continue;
        case 'r':
          *value += '\r';
          continue;
        case '\'':
          *value += '\'';
          continue;
        }
        *value += c;
        input->unget();
      } else {
        *value += c;
      }
      if (c == '\'') {
        return TOKEN_TYPE_QUOTED_STRING;
      }
      continue;
    case TOKEN_STATE_DOUBLE_QUOTE:
      if (c == '\\') {
        const char next_c = input->get();
        if (!input->good()) {
          break;
        }
        switch (next_c) {
        case 't':
          *value += '\t';
          continue;
        case 'n':
          *value += '\n';
          continue;
        case 'r':
          *value += '\r';
          continue;
        case '"':
          *value += '"';
          continue;
        }
        *value += c;
        input->unget();
      } else {
        *value += c;
      }
      if (c == '"') {
        return TOKEN_TYPE_QUOTED_STRING;
      }
      continue;
    case TOKEN_STATE_TOKEN_TYPE_COMMENT:
      if (c == '\n' || c == '\r') {
        return TOKEN_TYPE_COMMENT;
      }
      *value += c;
      continue;
    case TOKEN_STATE_TOKEN_TYPE_NORMAL:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\t' || c == ';' ||
          c == '{' || c == '}') {
        input->unget();
        return TOKEN_TYPE_NORMAL;
      }
      *value += c;
      continue;
    }
  }
  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE || state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }
  return TOKEN_TYPE_EOF;
}
bool NginxConfigParser::Parse(std::istream *config_file, NginxConfig *config) {
  std::stack<NginxConfig *> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int bracketCtr = 0;
  while (true) {
    if (bracketCtr < 0) {
      break;
    }
    std::string token;
    token_type = ParseToken(config_file, &token);
    printf("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }
    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }
    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL) {
        if (last_token_type != TOKEN_TYPE_NORMAL) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(token);
      } else {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_QUOTED_STRING) {
      const char next_char = config_file->get();
      if (!config_file->good()) {
        break;
      }
      if (next_char != ' ' && next_char != '\t' && next_char != '\n' &&
          next_char != '\r' && next_char != ';' && next_char != '{') {
        break;
      }
      config_stack.top()->statements_.back().get()->tokens_.push_back(token);
      config_file->unget();
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL &&
          last_token_type != TOKEN_TYPE_QUOTED_STRING) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      bracketCtr++;
      if (last_token_type != TOKEN_TYPE_NORMAL &&
          last_token_type != TOKEN_TYPE_QUOTED_STRING) {
        // Error.
        break;
      }
      NginxConfig *const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      bracketCtr--;
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START_BLOCK) {
        // Error.
        break;
      }
      config_stack.pop();
    } else if (token_type == TOKEN_TYPE_EOF) {
      if (bracketCtr != 0) {
        break;
      }
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START) {
        // Error.
        break;
      }
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }
  printf("Bad transition from %s to %s\n", TokenTypeAsString(last_token_type),
         TokenTypeAsString(token_type));
  return false;
}
bool NginxConfigParser::Parse(const char *file_name, NginxConfig *config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf("Failed to open config file: %s\n", file_name);
    return false;
  }
  const bool return_value =
      Parse(dynamic_cast<std::istream *>(&config_file), config);
  config_file.close();
  return return_value;
}