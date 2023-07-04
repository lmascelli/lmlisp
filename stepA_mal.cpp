#include "src/lmlisp.hpp"
#include <iostream>

std::string lmlisp::readln(std::string prompt) {
  std::cout << prompt;
  std::string ret;
  std::getline(std::cin, ret);
  return ret;
}

void lmlisp::writeln(std::string line) {
  std::cout << line << std::endl;
}

int main(int argc, char** argv) {
  std::string filename = argc > 1 ? argv[1] : "";
  lmlisp::init(filename).repl();
}
