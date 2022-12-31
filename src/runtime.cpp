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
  switch (ast->type) {
  case LIST: {
    ListP ast_p = ast->to<List>();
    if (ast_p->size() > 0) {
      if (ast_p->at(0)->type == SYMBOL) {
        SymbolP first = ast_p->at(0)->to<Symbol>();
        if (first->value() == "let*") {
          if (ast_p->at_least(3)) {
            EnvironmentP new_env = environment(env)->to<Environment>();
            if (ast_p->check_nth(1, LIST)) {
              ListP let_binds_l = ast_p->at(1)->to<List>();
              if (let_binds_l->size() % 2 == 0) {
                for (int i = 0; i < let_binds_l->size(); i += 2) {
                  if (let_binds_l->check_nth(i, SYMBOL)) {
                    new_env->set(let_binds_l->at(i)->to<Symbol>()->value(),
                                 EVAL(let_binds_l->at(i + 1), new_env));
                  } else {
                    // TODO let* key is not a symbol type
                    return nil();
                  }
                }
              } else {
                // TOTO let* binds not in pairs
                return nil();
              }
            } else if (ast_p->check_nth(1, VEC)) {
              VecP let_binds_v = ast_p->at(1)->to<Vec>();
              if (let_binds_v->size() % 2 == 0) {
                for (int i = 0; i < let_binds_v->size(); i += 2) {
                  if (let_binds_v->check_nth(i, SYMBOL)) {
                    new_env->set(let_binds_v->at(i)->to<Symbol>()->value(),
                                 EVAL(let_binds_v->at(i + 1), new_env));
                  } else {
                    // TODO let* key is not a symbol type
                    return nil();
                  }
                }
              } else {
                // TOTO let* binds not in pairs
                return nil();
              }

            } else {
              // TODO let* not valid type
	      return nil();
            }
	    return EVAL(ast_p->at(2), new_env);
          } else {
            // TODO let* not valid arguments number
            return nil();
          }
        } else if (first->value() == "def!") {
          if (ast_p->at_least(3) and ast_p->check_nth(1, SYMBOL)) {
            ElementP ret = EVAL(ast_p->at(2), env);
            env->set(ast_p->at(1)->to<Symbol>()->value(), ret);
            return ret;
          } else {
            // def! not valid arguments
            return nil();
          }
        } else {
          ListP ast_l = eval_ast(ast, env)->to<List>();
          // APPLY
          FunctionP f = ast_l->at(0)->to<Function>();
          ListP args = list()->to<List>();
          for (int i = 1; i < ast_l->size(); i++)
            args->append(ast_l->at(i));
          return f->apply(args);
        }
      } else
        // SOME KIND OF ERROR
        return nil();
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
  Printer p;
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
