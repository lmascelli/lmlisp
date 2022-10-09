#pragma once
#include <vector>
#include <string>

namespace lmlisp {
  using std::string;
class Reader {
  public:
    Reader(std::vector<string>& tokens);
    string& next();
    string& peek();

  private:
    std::vector<string> tokens;
    unsigned int pos;
};

void read_str(string input);
Reader tokenize(string input);
}
