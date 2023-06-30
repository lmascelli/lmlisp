#pragma once
#include "externals.hpp"
#include "types.hpp"

namespace lmlisp {
  ElementP READ(std::string input);
  ElementP eval_ast(ElementP ast, EnvironmentP env);
  ElementP EVAL(ElementP input, EnvironmentP env);
  std::string PRINT(ElementP input);

  ElementP cons(ElementP el, ElementP l);
  ElementP concat(std::vector<ElementP> args);

  class Runtime {
    public:
      Runtime(const Runtime& o) = delete;
      Runtime(const Runtime&& o) = delete;
      std::string rep(std::string input);

      void repl();
      friend Runtime& init(
          std::string filename,
          std::vector<std::string> argv);
      static Runtime& get_current();
      void quit();

    private:
      Runtime(std::string filename, std::vector<std::string> argv);
      static Runtime* current;

      // STATUS
      bool running;
      EnvironmentP core_runtime;

      // EXCEPTIONS
      ElementP raised;
      bool handled;
  };  
}
