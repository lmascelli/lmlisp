#include "lmlisp.hpp"
#include <iostream>
#include <readline/readline.h>

int main(int argc, char **argv) {
  lmlisp::Runtime runtime = lmlisp::lm_init({
      []() {
        char *input = readline("user> ");
        std::string ret = input;
        free(input);
        return ret;
      },
      [](std::string output) { std::cout << output << std::endl; },
  });
  switch (argc) {
  case 1:
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
