#include "runtime.hpp"
#include "reader.hpp"
#include "printer.hpp"

namespace lmlisp {
  ElementP READ(std::string input) {
    return read_str(input);
  }
  ElementP EVAL(ElementP input) {
    return input;
  }
  std::string PRINT(ElementP input) {
    return pr_str(input, true);
  }

  std::string rep(std::string input) {
    std::string ret = PRINT(EVAL(READ(input)));
    writeln(ret);
    return ret;
  }
}
