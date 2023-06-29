#pragma once
#include <string>

namespace lmlisp {
  extern std::string readln(std::string prompt = "");
  extern void writeln(std::string line);
  
  std::string READ(std::string input);
  std::string EVAL(std::string input);
  std::string PRINT(std::string input);

  std::string rep(std::string input);
}
