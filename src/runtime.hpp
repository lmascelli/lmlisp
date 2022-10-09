#pragma once
#include <string>
#include <functional>

namespace lmlisp {
using std::string, std::function;


struct Runtime_external_functions {
  function<string(void)> reader;
  function<void(string)> printer;
};

class Runtime {
public:
  Runtime(Runtime_external_functions externals);
  void repl();
  string rep(string expr);

private:
  bool running;
  string READ(string input) const;
  string EVAL(string ast);
  string PRINT(string expr) const;


  /****************** INTERFACES ***************/

  /* reader
   * this function abstract the getting of the user input.
   * It should handle the text inserted by user and store it
   * in a returned std::string
   * */
 
  function<string(void)> reader;

  /* printer 
   * same as reader but for displaying text to the user. It
   * accept a string as input and must print it to the screen
   * */
  function<void(string)> printer;
};
} // namespace lmlisp
