#include "runtime.hpp"
#include <iostream>

namespace lmlisp {
Runtime::Runtime(Runtime_external_functions externals) { 
  reader = externals.reader;
  printer = externals.printer;
  running = false; 
}

void Runtime::repl() {
  running = true;
  while (running) {
    string input = reader();
    printer(PRINT(EVAL(READ(input))));
  }
}

string Runtime::READ(string input) const { return input; }

string Runtime::EVAL(string ast) { return ast; }

string Runtime::PRINT(string expr) const { return expr; }

string Runtime::rep(string expr) {
  return PRINT(EVAL(READ(expr)));
}
} // namespace lmlisp
