#pragma once
#include <vector>
#include <string>
#include <functional>
#include "types.hpp"

namespace lmlisp {
class Reader {
  public:
    Reader(std::function<void(std::string)> printer);
    void read_str(const std::string& input);
    std::string next();
    std::string peek();

    ElementP read_form();
    ElementP read_list();
    ElementP read_atom();

  private:
    std::vector<std::string> tokens;
    int pos;
    std::function<void(std::string)> printer;
};


std::vector<std::string> tokenize(const std::string input);
}
