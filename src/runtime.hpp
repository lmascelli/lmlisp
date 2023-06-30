#pragma once
#include "externals.hpp"
#include "types.hpp"

namespace lmlisp {
  ElementP READ(std::string input);
  ElementP EVAL(ElementP input);
  std::string PRINT(ElementP input);

  std::string rep(std::string input);
}
