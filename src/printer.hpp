#pragma once
#include "types.hpp"
#include <functional>
#include <string>

namespace lmlisp {
class Printer {
public:
  std::string pr_str(ElementP el, bool print_readably = false);
};
} // namespace lmlisp
