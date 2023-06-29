#include "runtime.hpp"

namespace lmlisp {
  std::string READ(std::string input) {
    return input;
  }
  std::string EVAL(std::string input) {
    return input;
  }
  std::string PRINT(std::string input) {
    return input;
  }

  std::string rep(std::string input) {
    std::string ret = PRINT(EVAL(READ(input)));
    writeln(ret);
    return ret;
  }
}
