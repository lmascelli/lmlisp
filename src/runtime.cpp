#include "runtime.hpp"
#include "printer.hpp"
#include "reader.hpp"

namespace lmlisp {
Runtime::Runtime(Runtime_external_functions externals) {
  reader = externals.reader;
  printer = externals.printer;
  running = false;
}

void Runtime::repl() {
  running = true;
  while (running) {
    std::string input = reader();
    printer(PRINT(EVAL(READ(input))));
  }
}

ElementP Runtime::READ(std::string input) const {
  Reader r(printer);
  r.read_str(input);
  return r.read_form();
}

ElementP Runtime::EVAL(ElementP ast) {
  return ast;
}

std::string Runtime::PRINT(ElementP res) const {
  Printer p(printer);
  return p.pr_str(res);
}

std::string Runtime::rep(std::string expr) { return PRINT(EVAL(READ(expr))); }
} // namespace lmlisp
