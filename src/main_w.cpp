#include "linenoise.hpp"
#include "lmlisp.hpp"
#include <iostream>
#include <winnt.h>


static lmlisp::Runtime_external_functions externals {
  [](std::string prompt) {
  return linenoise::Readline(prompt.c_str());
  },
  [](std::string output) {
  std::cout << output << std::endl;
  }
};

int main(int argc, char **argv) {
  lmlisp::Runtime runtime = lmlisp::lm_init(externals);
  switch (argc) {
  case 1:
    std::cout << "LMLisp REPL" << std::endl;
    runtime.repl();
    break;
  case 2:
    if (std::string(argv[1]) == "-f") {
      // TODO load file
      std::cout << "file" << std::endl;
      break;
    }
  default:
    std::string expr;
    for (unsigned int i = 1; i < argc; i++) {
      expr += argv[i];
      expr += " ";
    }
    std::cout << runtime.rep(expr) << std::endl;
  }
}
