#pragma once
#include "externals.hpp"
#include "types.hpp"
#include <optional>

namespace lmlisp {
ElementP READ(std::string input);
ElementP eval_ast(ElementP ast, EnvironmentP env);
ElementP EVAL(ElementP input, EnvironmentP env);
std::string PRINT(ElementP input);

ElementP apply(FunctionP f, ListP args,
               std::optional<EnvironmentP> env = std::nullopt);
ElementP cons(ElementP el, ElementP l);
ElementP concat(std::vector<ElementP> args);

class Runtime {
public:
  Runtime(const Runtime &o) = delete;
  Runtime(const Runtime &&o) = delete;

  std::string rep(std::string input);

  void repl();
  friend Runtime &init(std::string filename, std::vector<std::string> argv);
  static Runtime &get_current();
  void quit();

  // EXCEPTIONS
  static ElementP exc_value;
  static bool raised;
  static bool handled;

private:
  Runtime(std::string filename, std::vector<std::string> argv);
  static Runtime *current;

  // STATUS
  bool running;
  EnvironmentP core_runtime;
};
} // namespace lmlisp
