#include "reader.hpp"
#include "types.hpp"
#include <cctype>
#include <cstdlib>
#include <regex>

namespace lmlisp {
Reader::Reader(std::function<void(std::string)> printer) {
  this->pos = -1;
  this->printer = printer;
};

std::string Reader::next() {
  if (pos > static_cast<int>(tokens.size())) {
    printer(
        "Going over list of tokens bound. Maybe an unclosed list or string");
    abort();
  }
  return tokens[++pos];
}

std::string Reader::peek() { return tokens[pos]; }

void Reader::read_str(const std::string &input) {
  tokens = tokenize(input);
}

std::vector<std::string> tokenize(std::string input) {
  std::vector<std::string> tokens;

  enum TOKEN_TYPE {
    UNKNOWN,
    STRING,
    SPACES,
    COMMENT,
  };

  TOKEN_TYPE current_token = UNKNOWN;
  auto add_token = [&tokens](std::string token) { tokens.push_back(token); };
  unsigned int index = 0, offset = 0;

  while (index + offset < input.length()) {
    char ch = input.at(index + offset);
    switch (current_token) {
    case STRING:
      if (ch == '"' and index > 0 and input.at(index + offset - 1) != '\\') {
        current_token = UNKNOWN;
        add_token(input.substr(index, offset + 1));
        index += offset + 1;
        offset = 0;
      } else
        offset++;
      break;
    case SPACES:
      switch (ch) {
      case ' ':
      case '\n':
      case '\t':
        index++;
        break;
      case '"':
        index += offset;
        offset = 1;
        current_token = STRING;
        break;
      default:
        offset = 0;
        current_token = UNKNOWN;
      }
      break;
    case COMMENT:
      if (ch == '\n')
        current_token = UNKNOWN;
      index++;
      break;
    case UNKNOWN:
      switch (ch) {
      case ' ':
        if (offset != 0) {
          add_token(input.substr(index, offset));
          index += offset;
          offset = 0;
        }
        current_token = SPACES;
        break;
      case '"':
        current_token = STRING;
        index += offset;
        offset++;
        break;
      case '~':
        if (input.at(index + offset + 1) == '@') {
          if (offset != 0)
            add_token(input.substr(index, offset));
          add_token(input.substr(index + offset, 2));
          index += offset + 2;
          offset = 0;
          break;
        }
      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case '@':
      case '&':
      case '\'':
      case '`':
      case '^':
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '\n':
      case '\t':
        if (offset > 0)
          add_token(input.substr(index, offset));
        index += offset + 1;
        offset = 0;
        break;
      case ';':
        current_token = COMMENT;
        if (offset > 0)
          add_token(input.substr(index, offset));
        index += offset + 1;
        offset = 0;
        break;
      default:
        offset++;
      }
    }
  }
  if (input.length() > 0 and index < input.length()) {
    add_token(input.substr(index, offset));
  }
  return tokens;
}

ElementP Reader::read_form() {
  printer(next());
  switch (peek().at(0)) {
  case '(':
    return read_list();
  default:
    return read_atom();
  }
}

ElementP Reader::read_list() {
  printer("making list");
  ElementP ret_list = list();
  while (peek().at(0) != ')') {
    ElementP el = read_form();
    if (el->type == SYMBOL and el->to<Symbol>()->value() == ")")
      break;
    ret_list->to<List>()->append(el);
    printer("appending " + peek());
  }
  printer("returning list");
  return ret_list;
}

ElementP Reader::read_atom() {
  char c = peek().at(0);
  if (std::isdigit(c) or c == '-' or c == '+') {
    bool period = false, comma = false, is_number = true; // maybe implement e
    for (char d : peek()) {
      if (std::isdigit(d)) {
        if (d == '.' and not period)
          period = true;
        else {
          is_number = false;
          break;
        }
        if (d == ',' and not comma)
          comma = true;
        else {
          is_number = false;
          break;
        }
      } else {
        is_number = false;
        break;
      }
    }
    if (is_number) {
      printer("making number");
      return num(stof(peek()));
    } else {
      printer("making symbol");
      return sym(peek());
    }
  } else if (c == '"') {
    printer("making string");
    return str(peek().substr(1, peek().length() - 2));
  } else {
    printer("making symbol");
    return sym(peek());
  }
}

} // namespace lmlisp
// namespace lmlisp
