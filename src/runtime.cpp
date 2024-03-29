#include "runtime.hpp"
#include "core.hpp"
#include "macros.hpp"
#include "printer.hpp"
#include "reader.hpp"
#include "types.hpp"
#include <cstdlib>
#include <stdlib.h>

namespace lmlisp {
Runtime *Runtime::current = nullptr;
ElementP Runtime::exc_value = nil();
bool Runtime::raised = false;
bool Runtime::handled = false;

void error(std::string message) {
  writeln(message);
  abort();
}

//**************************************************************************
//
//                             RUNTIME METHODS
//
//**************************************************************************

Runtime::Runtime(std::string filename, std::vector<std::string> argv) {
  running = false;
  core_runtime = init_core(argv);

  core_runtime->set("eval", func([this](ListP args) {
                      if (args->size() == 1) {
                        return EVAL(args->at(0), this->core_runtime);
                      } else {
                        THROW("eval: accept one argument");
                      }
                    }));

  core_runtime->set("cons", func([](ListP args) {
                      if (args->size() == 2) {
                        return cons(args->at(0), args->at(1));
                      } else
                        THROW("cons: requires two arguments");
                    }));

  core_runtime->set("concat", func([](ListP args) {
                      std::vector<ElementP> nargs;
                      for (unsigned int i = 0; i < args->size(); i++)
                        nargs.push_back(args->at(i));
                      return concat(nargs);
                    }));

  core_runtime->set("vec", func([](ListP args) {
                      if (args->size() > 0) {
                        ElementP el0 = args->at(0);
                        if (el0->type == LIST) {
                          VecP ret = vec();
                          for (unsigned int i = 0; i < el0->to<List>()->size();
                               i++)
                            ret->append(el0->to<List>()->at(i));
                          return ret->el();
                        } else if (el0->type == VEC)
                          return el0;
                        else
                          THROW("vec: accepted values are list or vecs");
                      } else
                        return vec()->el();
                    }));

  post_init(*this, filename);
}

ElementP apply(FunctionP f, ListP args, std::optional<EnvironmentP> env) {
  if (f->is_native()) {
    return f->apply(args);
  } else {
    return EVAL(f->get_exprs(), f->create_env(env.value(), args));
  }
}

void Runtime::quit() {
  writeln("Bye :)");
  running = false;
}

Runtime &Runtime::get_current() { return *Runtime::current; }

void Runtime::repl() {
  running = true;
  while (running) {
    std::string input = readln("user> ");
    writeln(rep(input));
  }
}

//**************************************************************************
//
//                             CHECK FUNCTIONS
//
//**************************************************************************

static bool is_special_form(ElementP el, std::string form) {
  return (el->type == SYMBOL and el->to<Symbol>()->value() == form);
}

//**************************************************************************
//
//                             REPL FUNCTIONS
//
//**************************************************************************

ElementP cons(ElementP el, ElementP l) {
  ListP ret = list();
  ret->append(el);
  if (l->type == LIST) {
    for (unsigned int i = 0; i < l->to<List>()->size(); i++)
      ret->append(l->to<List>()->at(i));
  } else if (l->type == VEC) {
    for (unsigned int i = 0; i < l->to<Vec>()->size(); i++)
      ret->append(l->to<Vec>()->at(i));
  } else
    THROW("cons: second argument must be a list or a vector");
  return ret->el();
}

ElementP concat(std::vector<ElementP> args) {
  bool valid = true;
  ListP ret = list();
  for (ElementP l : args) {
    if (l->type == LIST) {
      ListP ll = l->to<List>();
      for (unsigned int j = 0; j < ll->size(); j++)
        ret->append(ll->at(j));
    } else if (l->type == VEC) {
      VecP vl = l->to<Vec>();
      for (unsigned int j = 0; j < vl->size(); j++)
        ret->append(vl->at(j));
    } else {
      valid = false;
      break;
    }
  }
  if (valid)
    return ret->el();
  else
    THROW("concat: arguments must be lists or vectors");
}

ElementP quasiquote(ElementP ast) {
  switch (ast->type) {
  case LIST: {
    ListP l_ast = ast->to<List>();
    /******************** (unquote EL) ********************/
    if (l_ast->size() > 0 and l_ast->at(0)->type == SYMBOL and
        l_ast->at(0)->to<Symbol>()->value() == "unquote") {
      if (l_ast->size() == 2) {
        return l_ast->at(1);
      } else
        THROW("unquote: requires one argument");
    } else {
      ListP ret = list();
      for (int i = l_ast->size() - 1; i >= 0; i--) {
        ElementP elt = l_ast->at(i);
        ListP new_ret = list();
        /******************** (splice-unquote EL) ********************/
        if (elt->type == LIST and elt->to<List>()->at_least(2) and
            elt->to<List>()->at(0)->type == SYMBOL and
            elt->to<List>()->at(0)->to<Symbol>()->value() == "splice-unquote") {
          new_ret->append(sym("concat"));
          new_ret->append(elt->to<List>()->at(1));
          new_ret->append(ret);
        } else {
          new_ret->append(sym("cons"));
          new_ret->append(quasiquote(elt));
          new_ret->append(ret);
        }
        ret = new_ret;
      }
      return ret;
    }
  }
  case VEC: {
    ListP ret = list();
    VecP v_ast = ast->to<Vec>();
    for (int i = v_ast->size() - 1; i >= 0; i--) {
      ElementP elt = v_ast->at(i);
      ListP new_ret = list();
      /******************** (splice-unquote EL) ********************/
      if (elt->type == LIST and elt->to<List>()->at_least(2) and
          elt->to<List>()->at(0)->type == SYMBOL and
          elt->to<List>()->at(0)->to<Symbol>()->value() == "splice-unquote") {
        new_ret->append(sym("concat"));
        new_ret->append(elt->to<List>()->at(1));
        new_ret->append(ret);
      } else {
        new_ret->append(sym("cons"));
        new_ret->append(quasiquote(elt));
        new_ret->append(ret);
      }
      ret = new_ret;
    }
    ListP new_ret = list();
    new_ret->append(sym("vec"));
    new_ret->append(ret);
    return new_ret->el();
  }
  case DICT:
  case SYMBOL: {
    ListP ret = list();
    ret->append(sym("quote"));
    ret->append(ast);
    return ret;
  }
  default:
    return ast;
  }
}

bool is_macro_call(ElementP ast, EnvironmentP env) {
  if (ast->type != LIST)
    return false;
  if (not ast->to<List>()->check_nth(0, SYMBOL))
    return false;
  ElementP possible_macro =
      env->get(ast->to<List>()->at(0)->to<Symbol>()->value());
  if (possible_macro->type != FUNCTION)
    return false;
  if (not possible_macro->to<Function>()->is_macro)
    return false;
  return true;
}

ElementP macroexpand(ElementP ast, EnvironmentP env) {
  while (is_macro_call(ast, env)) {
    ListP l_ast = ast->to<List>();
    FunctionP macro =
        env->get(l_ast->at(0)->to<Symbol>()->value())->to<Function>();
    ListP args = list();
    for (unsigned int i = 1; i < l_ast->size(); i++) {
      args->append(l_ast->at(i));
    }
    ast = EVAL(macro->get_exprs(), macro->create_env(env, args));
  }
  return ast;
}

ElementP READ(std::string input) { return read_str(input); }

ElementP EVAL(ElementP ast, EnvironmentP env) {
  while (true) {
    // EXCEPTION CHECK
    if (Runtime::raised and not Runtime::handled) {
      return nil();
    }
    // NOT A LIST
    if (ast->type != LIST) {
      return eval_ast(ast, env);
    } else {
      ast = macroexpand(ast, env);
      if (ast->type != LIST) {
        return eval_ast(ast, env);
      } else {
        ListP u_ast = ast->to<List>();
        // EMPTY LIST
        if (u_ast->size() == 0) {
          return ast;
        } else {
          // LIST
          // CHECK SPECIAL FORMS FIRST
          ElementP ast_first = ast->to<List>()->at(0);
          //***************************** let ******************************//
          if (is_special_form(ast_first, "let*")) {
            if (u_ast->at_least(3)) {
              EnvironmentP new_env = environment(env)->to<Environment>();
              if (u_ast->check_nth(1, LIST)) {
                ListP let_binds_l = u_ast->at(1)->to<List>();
                if (let_binds_l->size() % 2 == 0) {
                  for (unsigned int i = 0; i < let_binds_l->size(); i += 2) {
                    if (let_binds_l->check_nth(i, SYMBOL)) {
                      new_env->set(let_binds_l->at(i)->to<Symbol>()->value(),
                                   EVAL(let_binds_l->at(i + 1), new_env));
                    } else {
                      THROW("let*: a key was not a symbol");
                    }
                  }
                } else {
                  THROW("let*: key-value binds not in pairs");
                }
              } else if (u_ast->check_nth(1, VEC)) {
                VecP let_binds_v = u_ast->at(1)->to<Vec>();
                if (let_binds_v->size() % 2 == 0) {
                  for (unsigned int i = 0; i < let_binds_v->size(); i += 2) {
                    if (let_binds_v->check_nth(i, SYMBOL)) {
                      new_env->set(let_binds_v->at(i)->to<Symbol>()->value(),
                                   EVAL(let_binds_v->at(i + 1), new_env));
                    } else {
                      THROW("let* - vector case: a key was not a symbol");
                    }
                  }
                } else {
                  THROW("let* - vector case: key-value binds not in pairs");
                }
              } else {
                THROW("let*: first element must be a list or a vec");
              }
              env = new_env;
              ast = u_ast->at(2);
              continue;
            } else {
              THROW("let*: needed at least 3 arguments");
            }
          }
          //**************************** def! ******************************//
          else if (is_special_form(ast_first, "def!")) {
            if (u_ast->at_least(3) and u_ast->check_nth(1, SYMBOL)) {
              ElementP ret = EVAL(u_ast->at(2), env);
              if (CHECK_EXC) {
                return nil();
              } else {
                env->set(u_ast->at(1)->to<Symbol>()->value(), ret);
                return ret;
              }
            }
          }
          //***************************** do *******************************//
          else if (is_special_form(ast_first, "do")) {
            if (u_ast->size() >= 2) {
              for (unsigned int i = 1; i < u_ast->size() - 1; i++)
                EVAL(u_ast->at(i), env);
              ast = u_ast->at(u_ast->size() - 1);
              continue;
            } else
              THROW("do: requires at least one argument");
          }
          //****************************** if ******************************//
          else if (is_special_form(ast_first, "if")) {
            if (u_ast->size() >= 3) {
              ElementP condition = EVAL(u_ast->at(1), env);
              if (not(condition->type == NIL or
                      (condition->type == BOOLEAN and
                       condition->to<Boolean>()->value() == false))) {
                ast = u_ast->at(2);
                continue;
              } else {
                if (u_ast->size() >= 4) {
                  ast = u_ast->at(3);
                  continue;
                } else
                  return nil();
              }
            } else {
              THROW("if: require at least two arguments");
            }
          }
          //***************************** fn* ******************************//
          else if (is_special_form(ast_first, "fn*")) {
            if (u_ast->size() >= 3) {
              if (u_ast->at(1)->type == LIST or u_ast->at(1)->type == VEC) {
                bool args_all_symbols = true;
                bool last_is_variadic = false;
                ListP args = list();
                ListP u_args = u_ast->at(1)->type == VEC
                                   ? u_ast->at(1)->to<Vec>()->listed()
                                   : u_ast->at(1)->to<List>();
                for (unsigned int i = 0; i < u_args->size(); i++) {
                  if (u_args->at(i)->type == SYMBOL) {
                    if (i == u_args->size() - 2 and
                        u_args->at(i)->to<Symbol>()->value() == "&") {
                      last_is_variadic = true;
                      args->append(u_args->at(i + 1));
                      break;
                    } else
                      args->append(u_args->at(i));
                  } else {
                    args_all_symbols = false;
                    break;
                  }
                }
                if (args_all_symbols) {
                  return func(env, args, u_ast->at(2), last_is_variadic);
                } else {
                  THROW("fn*: binds element must all be symbols");
                }
              } else {
                THROW("fn*: clojure arguments must be a list or a vec");
              }
            } else
              THROW("fn*: require at least two parameters");
          }
          //***************************** quote ****************************//
          else if (is_special_form(ast_first, "quote")) {
            TEST_DO_OR_EXC(
                u_ast->size() == 2, { return u_ast->at(1); },
                "quote: requires one argument");
          }
          //********************** quasiquoteexpand ************************//
          else if (is_special_form(ast_first, "quasiquoteexpand")) {
            if (u_ast->size() == 2) {
              return quasiquote(u_ast->at(1));
            } else
              THROW("quasiquoteexpand: requires one argument");
          }
          //************************* quasiquote ***************************//
          else if (is_special_form(ast_first, "quasiquote")) {
            if (u_ast->size() == 2) {
              ast = quasiquote(u_ast->at(1));
              continue;
            } else
              THROW("quasiquote: requires one argument");
          }
          //************************ macroexpand ***************************//
          else if (is_special_form(ast_first, "macroexpand")) {
            if (u_ast->size() == 2) {
              return macroexpand(u_ast->at(1), env);
            } else
              THROW("macroexpand: requires one argument");
          }
          //**************************** try *******************************//
          else if (is_special_form(ast_first, "try*")) {
            if (u_ast->size() == 3 and u_ast->at(2)->type == LIST and
                u_ast->at(2)->to<List>()->size() == 3 and
                u_ast->at(2)->to<List>()->at(0)->type == SYMBOL and
                u_ast->at(2)->to<List>()->at(0)->to<Symbol>()->value() ==
                    "catch*" and
                u_ast->at(2)->to<List>()->at(1)->type == SYMBOL) {
              Runtime::handled = true;
              ElementP ret = EVAL(u_ast->at(1), env);
              if (Runtime::raised) {
                EnvironmentP catch_env = environment(env);
                catch_env->set(
                    u_ast->at(2)->to<List>()->at(1)->to<Symbol>()->value(),
                    Runtime::exc_value);
                Runtime::raised = false;
                Runtime::handled = false;
                ast = EVAL(u_ast->at(2)->to<List>()->at(2), catch_env);
                continue;
              } else {
                Runtime::handled = false;
                ast = ret;
                continue;
              }
            } else if (u_ast->size() == 2) {
              ast = u_ast->at(1);
              continue;
            } else {
              THROW("try* catch*: must be in the form (try* expr1 (catch* "
                    "exc expr2))");
            }
          }
          //************************** defmacro! ****************************//
          else if (is_special_form(ast_first, "defmacro!")) {
            if (u_ast->at_least(3) and u_ast->check_nth(1, SYMBOL)) {
              ElementP ret = EVAL(u_ast->at(2), env);
              if (ret->type == FUNCTION) {
                ElementP ret_as_m = copy(ret);
                ret_as_m->to<Function>()->is_macro = true;
                env->set(u_ast->at(1)->to<Symbol>()->value(), ret_as_m);
                return ret;
              } else
                THROW("defmacro!: define a function as macro");
            } else
              THROW("defmacro!: wrong arguments passed");
          }
          // *********************** APPLY SECTION **************************//
          else {
            ListP e_ast = eval_ast(ast, env)->to<List>();
            ElementP e_f = e_ast->at(0);
            if (e_f->type == FUNCTION) {
              FunctionP f = e_ast->at(0)->to<Function>();
              ListP args = list()->to<List>();
              for (unsigned int i = 1; i < e_ast->size(); i++) {
                args->append(e_ast->at(i));
              }
              if (f->is_native()) {
                return f->apply(args);
              } else {
                ast = f->get_exprs();
                env = f->create_env(env, args);
                continue;
              }
            } else
              THROW("'" + pr_str(u_ast->at(0)) + "' not found");
          }
        }
      }
    }
  }
}

std::string PRINT(ElementP res) {
  if (Runtime::raised) {
    Runtime::raised = false;
    writeln("Exception: " + pr_str(Runtime::exc_value));
    return pr_str(nil());
  } else
    return pr_str(res, true);
}

std::string Runtime::rep(std::string expr) {
  return PRINT(EVAL(READ(expr), core_runtime));
}

ElementP eval_ast(ElementP ast, EnvironmentP env) {
  switch (ast->type) {
  case SYMBOL: {
    return env->get(ast->to<Symbol>()->value());
  }
  case LIST: {
    ListP ret = list()->to<List>();
    for (unsigned int i = 0; i < ast->to<List>()->size(); i++) {
      ret->append(EVAL(ast->to<List>()->at(i), env));
    }
    return ret;
  }
  case VEC: {
    VecP ret = vec()->to<Vec>();
    for (unsigned int i = 0; i < ast->to<List>()->size(); i++) {
      ret->append(EVAL(ast->to<List>()->at(i), env));
    }
    return ret;
  }
  case DICT: {
    DictP ret = dict()->to<Dict>();
    ListP keys = ast->to<Dict>()->keys()->to<List>();
    for (unsigned int i = 0; i < keys->size(); i++)
      ret->append(keys->at(i), EVAL(ast->to<Dict>()->get(keys->at(i)), env));
    return ret;
  }
  default:
    return ast;
  }
}
} // namespace lmlisp
