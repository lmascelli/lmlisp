#pragma once
#include "types.hpp"
#include <string>

namespace lmlisp {
namespace check {
template <int _ = 0>
static bool is_special_form(ElementP el, std::string form) {
  return (el->type == SYMBOL and el->to<Symbol>()->value() == form);
}

} // namespace check
} // namespace lmlisp
