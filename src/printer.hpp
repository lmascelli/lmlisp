#pragma once
#include "types.hpp"
#include <functional>
#include <string>

namespace lmlisp {
class Printer {
public:
  Printer(std::function<void(std::string)> printer);
  std::string pr_str(ElementP el, bool print_readably = false);

private:
  std::function<void(std::string)> printer;
};
} // namespace lmlisp
