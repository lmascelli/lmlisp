#include "runtime.hpp"
#include "core.hpp"
#include "printer.hpp"
#include "reader.hpp"

namespace lmlisp {
Runtime::Runtime(Runtime_external_functions &externals)
    : reader(externals.reader), printer(externals.printer) {
  running = false;
  core_runtime = init_core(externals);
}

void Runtime::repl() {
  running = true;
  while (running) {
    std::string input = reader("> ");
    rep(input);
  }
}

ElementP Runtime::READ(std::string input) const {
  Reader r(printer);
  r.read_str(input);
  return r.read_form();
}

ElementP Runtime::EVAL(ElementP ast, EnvironmentP env) {
  Printer p(printer);
  switch (ast->type) {
  case LIST: {
    ListP ast_l = eval_ast(ast, env)->to<List>();
    if (ast_l->size() > 0) {
      // APPLY
      FunctionP f = ast_l->at(0)->to<Function>();
      ListP args = list()->to<List>();
      for (int i = 1; i < ast_l->size(); i++)
        args->append(ast_l->at(i));
      return f->apply(args);
    } else {
      // VOID LIST
      return ast;
    }
  }
    // NO LIST
  default:
    return eval_ast(ast, env);
  }
}

std::string Runtime::PRINT(ElementP res) const {
  Printer p(printer);
  return p.pr_str(res, true);
}

std::string Runtime::rep(std::string expr) {
  return PRINT(EVAL(READ(expr), core_runtime));
}

ElementP Runtime::eval_ast(ElementP ast, EnvironmentP env) {
  switch (ast->type) {
  case SYMBOL:
    return env->get(ast->to<Symbol>()->value());
  case LIST: {
    ListP ret = list()->to<List>();
    for (int i = 0; i < ast->to<List>()->size(); i++) {
      ret->append(EVAL(ast->to<List>()->at(i), env));
    }
    return ret;
  }
  case VEC: {
    VecP ret = vec()->to<Vec>();
    for (int i = 0; i < ast->to<List>()->size(); i++) {
      ret->append(EVAL(ast->to<List>()->at(i), env));
    }
    return ret;
  }
  case DICT: {
    DictP ret = dict()->to<Dict>();
    ListP keys = ast->to<Dict>()->keys()->to<List>();
    for (int i = 0; i < keys->size(); i++)
      ret->append(keys->at(i), EVAL(ast->to<Dict>()->get(keys->at(i)), env));
    return ret;
  }
  default:
    return ast;
  }
}
} // namespace lmlisp
