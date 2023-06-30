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

int main([[maybe_unused]]int argc, [[maybe_unused]]char** argv) {
  std::string input;
  lmlisp::init().repl();
}
