#include "printer.hpp"

namespace lmlisp {
Printer::Printer(std::function<void(std::string)> printer) {
  this->printer = printer;
}

std::string Printer::pr_str(ElementP el) {
  switch (el->type) {
  case NUMBER:
    return std::to_string(el->to<Number>()->value());
  case STRING:
    return el->to<String>()->value();
  case SYMBOL:
    return el->to<Symbol>()->value();
  case LIST:
    ListP l = el->to<List>();
    std::string ret = "( ";
    for (unsigned int i = 0; i < l->size(); i++)
      ret += pr_str(l->at(i)) + " ";
    ret += ")";
    return ret;
  }
}
} // namespace lmlisp
