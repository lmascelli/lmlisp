#pragma once
#include "types.hpp"
#include <functional>
#include <string>

namespace lmlisp {
class Printer {
public:
  Printer(std::function<void(std::string)> printer);
  std::string pr_str(ElementP el);

private:
  std::function<void(std::string)> printer;
};
} // namespace lmlisp
