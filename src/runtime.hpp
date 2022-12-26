#pragma once
#include "types.hpp"
#include <functional>
#include <string>

namespace lmlisp {

struct Runtime_external_functions {
  std::function<std::string(std::string)> reader;
  std::function<void(std::string)> printer;
};

class Runtime {
public:
  Runtime(Runtime_external_functions &externals);
  void repl();
  std::string rep(std::string expr);

private:
  bool running;
  EnvironmentP core_runtime;
  ElementP READ(std::string input) const;
  ElementP EVAL(ElementP ast, EnvironmentP env);
  std::string PRINT(ElementP expr) const;
  ElementP eval_ast(ElementP ast, EnvironmentP env);
  
  /****************** INTERFACES ***************/

  /* reader
   * this function abstract the getting of the user input.
   * It should handle the text inserted by user and store it
   * in a returned std::string
   * */

  std::function<std::string(std::string)> &reader;

  /* printer
   * same as reader but for displaying text to the user. It
   * accept a string as input and must print it to the screen
   * */
  std::function<void(std::string)> &printer;
};
} // namespace lmlisp
