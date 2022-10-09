#include "reader.hpp"
#include <regex>

namespace lmlisp {
Reader::Reader(std::vector<string> &tokens) {
  this->tokens = std::move(tokens);
  this->pos = 0;
};

string &Reader::next() { return tokens[pos++]; }

string &Reader::peek() { return tokens[pos]; }

void read_str(string input) {}
Reader tokenize(string input) {
  std::vector<string> tokens;

  const char regex_pattern[] = "[\\s,]*(~@|[\\[\\]{}()'`~^@]|\"(?:\\\\.|[^"
                               "\\\\\"])*\"?|;.*|[^\\s\\[\\]{}('\"`,;)]*)";
  // char ch;
  // for (unsigned int index = 0, offset = 0; index < input.length(); index++) {
  // }

  return std::move(Reader(tokens));
}
} // namespace lmlisp
