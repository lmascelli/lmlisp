#include "core.hpp"
#include "externals.hpp"
#include "macros.hpp"
#include "printer.hpp"
#include "reader.hpp"
#include "runtime.hpp"
#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>

namespace lmlisp {
StringP type(ElementP el) {
  switch (el->type) {
  case NIL:
    return str("nil");
  case LIST:
    return str("list");
  case STRING:
    return str("string");
  case NUMBER:
    return str("number");
  case BOOLEAN:
    return str("boolean");
  case DICT:
    return str("dict");
  case FUNCTION:
    return str("function");
  case ENVIRONMENT:
    return str("environment");
  case KEYWORD:
    return str("keyword");
  case SYMBOL:
    return str("symbol");
  case VEC:
    return str("vector");
  case EXCEPTION:
    return str("excepion");
  case ATOM:
    return str("atom");
  default:
    return str("type unknown");
  }
}

unsigned int timeMillisec() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now()
      .time_since_epoch()).count();
}

EnvironmentP init_core(std::vector<std::string> argv) {
  EnvironmentP core = std::make_shared<Environment>(nil());

  // *************************** RUNTIME **********************************

  core->set("true", boolean(true));

  core->set("false", boolean(false));

  core->set("nil", nil());

  core->set("quit", func([]([[maybe_unused]] ListP args) {
              Runtime::get_current().quit();
              return nil();
            }));

  ListP l_argv = list();
  for (std::string arg : argv) {
    l_argv->append(str(arg));
  }
  core->set("*ARGV*", l_argv);

  core->set("*host-language*", str("cpp"));

  core->set("apply", func([core](ListP args) {
              if (args->at_least(2) and args->at(0)->type == FUNCTION and
                  (args->at(args->size() - 1)->type == LIST or
                   args->at(args->size() - 1)->type == VEC)) {
                FunctionP f = args->at(0)->to<Function>();
                ListP f_args = list();
                for (unsigned int i = 1; i < args->size() - 1; i++) {
                  f_args->append(args->at(i));
                }
                ElementP el_arg = args->at(args->size() - 1);
                if (el_arg->type == LIST) {
                  ListP l_arg = el_arg->to<List>();
                  for (unsigned int i = 0; i < l_arg->size(); i++)
                    f_args->append(l_arg->at(i));
                } else {
                  VecP v_arg = el_arg->to<Vec>();
                  for (unsigned int i = 0; i < v_arg->size(); i++)
                    f_args->append(v_arg->at(i));
                }
                if (f->is_native()) {
                  return f->apply(f_args);
                } else {
                  ElementP ast = f->get_exprs();
                  EnvironmentP env = f->create_env(core, f_args);
                  return EVAL(ast, env);
                }
              } else
                return exc("apply: arguments are a function and its arguments, "
                           "last of whom must be a list or a vector whose "
                           "other will be concatenated")
                    ->el();
            }));

  core->set(
      "map", func([core](ListP args) {
        if (args->at_least(2) and args->at(0)->type == FUNCTION and
            (args->at(1)->type == LIST or args->at(1)->type == VEC)) {
          ListP ret = list();
          FunctionP f = args->at(0)->to<Function>();
          if (args->at(1)->type == LIST) {
            ListP f_args = args->at(1)->to<List>();
            for (unsigned int i = 0; i < f_args->size(); i++) {
              ListP if_args = list();
              if_args->append(eval_ast(f_args->at(i), core));
              ret->append(apply(f, if_args, core));
            }
          } else if (args->at(1)->type == VEC) {
            VecP f_args = args->at(1)->to<Vec>();
            for (unsigned int i = 0; i < f_args->size(); i++) {
              ListP if_args = list();
              if_args->append(f_args->at(i));
              ret->append(apply(f, if_args, core));
            }
          }
          if (ret->size() == 1)
            return ret->at(0);
          else
            return ret->el();
        } else
          return exc("map: arguments are a function and a list or a vector")
              ->el();
      }));

  core->set("time-ms", func([]([[maybe_unused]] ListP args) {
              return num(timeMillisec());
            }));

  // ****************************** IO ************************************

  core->set("prn", func([](ListP args) {
              std::string ret;
              if (args->size() > 1)
                for (unsigned int i = 0; i < args->size() - 1; i++)
                  ret += pr_str(args->at(i), true) + " ";
              if (args->size() > 0)
                ret += pr_str(args->at(args->size() - 1), true);
              writeln(ret);
              return nil();
            }));

  core->set("println", func([](ListP args) {
              std::string ret;
              if (args->size() > 1)
                for (unsigned int i = 0; i < args->size() - 1; i++)
                  ret += pr_str(args->at(i), false) + " ";
              if (args->size() > 0)
                ret += pr_str(args->at(args->size() - 1), false);
              writeln(ret);
              return nil();
            }));

  core->set("read-string", func([core](ListP args) {
              if (args->at_least(1) and args->check_nth(0, STRING)) {
                return read_str(pr_str(args->at(0), false))->el();
              } else {
                THROW("read-string: argument must be a string");
              }
            }));

  core->set("slurp", func([](ListP args) {
              if (args->at_least(1) and args->check_nth(0, STRING)) {
                std::ifstream ifs(args->at(0)->to<String>()->value());
                if (ifs.is_open()) {
                  std::ostringstream ss;
                  ss << ifs.rdbuf();
                  return str(ss.str())->el();
                } else {
                  return exc("slurp: error opening file")->el();
                }
              } else
                return exc("slurp: wrong argument")->el();
            }));

  core->set("readline", func([](ListP args) {
              if (args->size() == 0) {
                return str(readln(""))->el();
              } else if (args->at(0)->type == STRING) {
                return str(readln(args->at(0)->to<String>()->value()))->el();
              } else
                THROW("readline needs a string as argument");
            }));

  // **************************** TYPES **********************************

  core->set("nil?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == NIL)->el();
              } else
                return exc("nil?: requires one argument")->el();
            }));

  core->set("type", func([](ListP args) {
              if (args->size() == 0) {
                return nil()->el();
              } else if (args->size() == 1) {
                return type(args->at(0))->el();
              } else {
                ListP ret = list();
                for (unsigned int i = 0; i < args->size(); i++)
                  ret->append(type(args->at(i)));
                return ret->el();
              }
            }));

  core->set("true?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == BOOLEAN and
                               args->at(0)->to<Boolean>()->value())
                    ->el();
              } else
                return exc("true?: requires one argument")->el();
            }));

  core->set("false?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == BOOLEAN and
                               not args->at(0)->to<Boolean>()->value())
                    ->el();
              } else
                return exc("false?: requires one argument")->el();
            }));

  core->set("symbol?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == SYMBOL)->el();
              } else
                return exc("symbol?: requires one argument")->el();
            }));

  core->set("string?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == STRING)->el();
              } else
                return exc("string?: requires one argument")->el();
            }));

  core->set("number?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == NUMBER)->el();
              } else
                return exc("number?: requires one argument")->el();
            }));

  core->set("keyword?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == KEYWORD)->el();
              } else
                return exc("keyword?: requires one argument")->el();
            }));

  core->set("vector?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == VEC)->el();
              } else
                return exc("vector?: requires one argument")->el();
            }));

  core->set("sequential?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == VEC or
                               args->at(0)->type == LIST)
                    ->el();
              } else
                return exc("vector?: requires one argument")->el();
            }));

  core->set("map?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == DICT)->el();
              } else
                return exc("map?: requires one argument")->el();
            }));

  core->set("fn?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == FUNCTION and
                               not args->at(0)->to<Function>()->is_macro)
                    ->el();
              } else
                return exc("fn?: requires one argument")->el();
            }));

  core->set("macro?", func([](ListP args) {
              if (args->size() == 1) {
                return boolean(args->at(0)->type == FUNCTION and
                               args->at(0)->to<Function>()->is_macro)
                    ->el();
              } else
                return exc("fn?: requires one argument")->el();
            }));

  // ****************************** LIST ***********************************

  core->set("list", func([](ListP args) {
              // maybe just return args
              ListP ret = list();
              for (unsigned int i = 0; i < args->size(); i++) {
                ret->append(args->at(i));
              }
              return ret;
            }));

  core->set("list?", func([](ListP args) {
              if (args->size() == 1)
                return boolean(args->at(0)->type == LIST)->el();
              else if (args->size() > 1) {
                ListP ret = list();
                for (unsigned int i = 0; i < args->size(); i++) {
                  ret->append(boolean(args->at(i)->type == LIST));
                }
                return ret->el();
              } else
                return nil()->el();
            }));

  core->set("empty?", func([](ListP args) {
              if (args->size() >= 1 and args->at(0)->type == LIST) {
                return boolean(args->at(0)->to<List>()->size() == 0)->el();
              } else if (args->size() >= 1 and args->at(0)->type == VEC) {
                return boolean(args->at(0)->to<Vec>()->size() == 0)->el();
              } else {
                return nil()->el();
              }
            }));

  core->set("count", func([](ListP args) {
              if (args->size() == 1 and args->at(0)->type == LIST) {
                return num(args->at(0)->to<List>()->size())->el();
              } else if (args->size() == 1 and args->at(0)->type == VEC) {
                return num(args->at(0)->to<Vec>()->size())->el();
              } else if (args->size() == 1 and args->at(0)->type == NIL) {
                return num(0)->el();
              } else {
                return num(args->size())->el();
              }
            }));

  core->set("nth", func([](ListP args) {
              if (args->size() == 2 and args->at(0)->type == LIST and
                  args->at(1)->type == NUMBER) {
                ListP l = args->at(0)->to<List>();
#ifdef _LM_WITH_FLOAT
                int index = floor(args->at(1)->to<Number>()->value());
#else
                int index = args->at(1)->to<Number>()->value();
#endif
                if (index >= 0 and index < static_cast<int>(l->size())) {
                  return l->at(index);
                } else
                  THROW("nth: index out of bounds");
              } else if (args->size() == 2 and args->at(0)->type == VEC and
                         args->at(1)->type == NUMBER) {
                VecP v = args->at(0)->to<Vec>();
#ifdef _LM_WITH_FLOAT
                int index = floor(args->at(1)->to<Number>()->value());
#else
                int index = args->at(1)->to<Number>()->value();
#endif
                if (index >= 0 and index < static_cast<int>(v->size())) {
                  return v->at(index);
                } else
                  THROW("nth: index out of bounds");
              } else
                THROW("nth: arguments are a list or a vector and an index");
            }));

  core->set("first", func([](ListP args) {
              if (args->check_nth(0, LIST)) {
                if (args->at(0)->to<List>()->size() == 0)
                  return nil()->el();
                else
                  return args->at(0)->to<List>()->at(0);
              } else if (args->check_nth(0, VEC)) {
                if (args->at(0)->to<Vec>()->size() == 0)
                  return nil()->el();
                else
                  return args->at(0)->to<Vec>()->at(0);
              } else if (args->check_nth(0, NIL))
                return nil()->el();
              else
                return exc("first: argument is a list or a vector")->el();
            }));

  core->set("rest", func([](ListP args) {
              if (args->check_nth(0, LIST)) {
                ListP l = args->at(0)->to<List>();
                if (l->size() == 0)
                  return list()->el();
                else {
                  ListP ret = list();
                  for (unsigned int i = 1; i < l->size(); i++)
                    ret->append(l->at(i));
                  return ret->el();
                }
              } else if (args->check_nth(0, VEC)) {
                VecP v = args->at(0)->to<Vec>();
                if (v->size() == 0)
                  return list()->el();
                else {
                  ListP ret = list();
                  for (unsigned int i = 1; i < v->size(); i++)
                    ret->append(v->at(i));
                  return ret->el();
                }
              } else if (args->check_nth(0, NIL))
                return list()->el();

              else
                return exc("rest: argument is a list or a vector")->el();
            }));

  core->set("conj", func([](ListP args) {
              if (args->at_least(2)) {
                switch (args->at(0)->type) {
                case LIST: {
                  ListP ret = list();
                  for (unsigned int i = args->size() - 1; i >= 1; --i) {
                    ret->append(args->at(i));
                  }
                  if (args->at(0)->to<List>()->size() > 0) {
                    for (unsigned int i = 0;
                         i < args->at(0)->to<List>()->size(); ++i) {
                      ret->append(args->at(0)->to<List>()->at(i));
                    }
                  }
                  return ret->el();
                }
                case VEC: {
                  VecP ret = vec();
                  if (args->at(0)->to<Vec>()->size() > 0) {
                    for (unsigned int i = 0; i < args->at(0)->to<Vec>()->size();
                         ++i) {
                      ret->append(args->at(0)->to<Vec>()->at(i));
                    }
                  }
                  for (unsigned int i = 1; i <= args->size() - 1; ++i) {
                    ret->append(args->at(i));
                  }
                  return ret->el();
                }
                default:
                  THROW("conj: first argument must be a list or a vector");
                }
              } else {
                THROW("conj: pass at least two arguments");
              }
            }));

  // ***************************** STRING **********************************

  core->set("pr-str", func([](ListP args) {
              std::string ret = "";
              if (args->size() > 0) {
                if (args->size() > 1) {
                  for (unsigned int i = 0; i < args->size() - 1; i++)
                    ret += pr_str(args->at(i), true) + " ";
                }
                ret += pr_str(args->at(args->size() - 1), true);
              }
              return str(ret);
            }));

  core->set("str", func([](ListP args) {
              std::string ret = "";
              for (unsigned int i = 0; i < args->size(); i++)
                ret += pr_str(args->at(i), false);
              return str(ret)->el();
            }));

  core->set("seq", func([](ListP args) {
              if (args->at_least(1)) {
                switch (args->at(0)->type) {
                case NIL:
                  return nil()->el();
                case STRING: {
                  std::string content = args->at(0)->to<String>()->value();
                  if (content.empty())
                    return nil()->el();
                  else {
                    ListP ret = list();
                    for (unsigned int i=0; i < content.length(); ++i) {
                      ret->append(str(content.substr(i, 1)));
                    }
                    return ret->el();
                  }
                } break;
                case LIST: {
                if (args->at(0)->to<List>()->size() == 0) return nil()->el();
                else return args->at(0);
                }
                case VEC:
                if (args->at(0)->to<Vec>()->size() == 0) return nil()->el();
                else {
                  ListP ret = list();
                  for (unsigned int i=0; i<args->at(0)->to<Vec>()->size(); ++i)
                    ret->append(args->at(0)->to<Vec>()->at(i));
                  return ret->el();
                }
                default:
                  THROW("seq: argument must be a list, vector, string or nil");
                }
              } else {
                THROW("seq: pass at least one argument");
              }
            }));

  // ***************************** SYMBOL **********************************

  core->set("symbol", func([](ListP args) {
              if (args->size() == 1 and args->at(0)->type == STRING) {
                return sym(args->at(0)->to<String>()->value())->el();
              } else
                return exc("symbol: expects a string as argument")->el();
            }));

  // ***************************** KEYWORD *********************************

  core->set("keyword", func([](ListP args) {
              if (args->size() == 1 and args->at(0)->type == STRING) {
                return kw(args->at(0)->to<String>()->value())->el();
              } else if (args->size() == 1 and args->at(0)->type == KEYWORD) {
                return args->at(0);
              } else
                return exc("keyword: expects a string as argument")->el();
            }));

  // ***************************** VECTOR **********************************

  core->set("vector", func([](ListP args) {
              VecP ret = vec();
              for (unsigned int i = 0; i < args->size(); i++)
                ret->append(args->at(i));
              return ret;
            }));

  // ****************************** DICT ***********************************

  core->set(
      "hash-map", func([](ListP args) {
        if (args->size() % 2 == 0) {
          DictP ret = dict();
          for (unsigned int i = 0; i < args->size(); i += 2) {
            ret->append(args->at(i)->to<Keyword>(), args->at(i + 1));
          }
          return ret->el();
        } else
          return exc("hash-map: keys and values must came in pairs")->el();
      }));

  core->set("assoc", func([](ListP args) {
              if (args->size() % 2 == 1 and args->at(0)->type == DICT) {
                DictP ret = dict(), orig = args->at(0)->to<Dict>();
                ListP keys = orig->keys();
                for (unsigned int i = 0; i < keys->size(); i++) {
                  ret->append(keys->at(i), orig->get(keys->at(i)));
                }
                for (unsigned int i = 1; i < args->size(); i += 2) {
                  ret->append(args->at(i)->to<Keyword>(), args->at(i + 1));
                }
                return ret->el();
              } else
                return exc("assoc: arguments are an hash-map"
                           "and pairs of key and value")
                    ->el();
            }));

  core->set(
      "dissoc", func([](ListP args) {
        if (args->at_least(1) and args->at(0)->type == DICT) {
          // check once all arguments are valid keywords
          DictP ret = dict(), orig = args->at(0)->to<Dict>();
          ListP keys = orig->keys();
          for (unsigned int i = 0; i < keys->size(); i++) {
            bool to_skip = false;
            for (unsigned int j = 1; j < args->size(); j++) {
              if (keys->at(i)->compare(args->at(j))) {
                to_skip = true;
                break;
              }
            }
            if (not to_skip) {
              ret->append(keys->at(i), orig->get(keys->at(i)));
            }
          }
          return ret->el();
        } else
          return exc("dissoc: requires an hash-map as first arguments followed"
                     " by the keys to remove")
              ->el();
      }));

  core->set("get", func([](ListP args) {
              if (args->at_least(2) and args->at(0)->type == DICT) {
                return args->at(0)->to<Dict>()->get(args->at(1));
              } else
                THROW("get: requires a dict and a key");
            }));

  core->set("contains?", func([](ListP args) {
              if (args->at_least(2) and args->at(0)->type == DICT) {
                return args->at(0)->to<Dict>()->contains(args->at(1))->el();
              } else
                return exc("contains: requires a dict and a key")->el();
            }));

  core->set("keys", func([](ListP args) {
              if (args->at_least(1) and args->at(0)->type == DICT) {
                // ListP ret = list(), keys = args->at(0)->to<Dict>()->keys();
                // for (unsigned int i =0; i<keys->size(); i++){
                // 	if (keys->at(i)->type == KEYWORD)
                // ret->append(keys->at(i)); 	else
                // ret->append(kw(keys->at(i)->to<String>()->value()));
                // }
                // return ret->el();
                return args->at(0)->to<Dict>()->keys()->el();
              } else
                return exc("keys: requires a dict")->el();
            }));

  core->set("vals", func([](ListP args) {
              if (args->at_least(1) and args->at(0)->type == DICT) {
                DictP orig = args->at(0)->to<Dict>();
                ListP ret = list(), keys = orig->keys();
                for (unsigned int i = 0; i < keys->size(); i++) {
                  ret->append(orig->get(keys->at(i)));
                }
                return ret->el();
              } else
                return exc("vals: requires a dict")->el();
            }));

  // ***************************** COMPARE *********************************

  core->set("=", func([](ListP args) {
              if (args->at_least(2)) {
                return boolean(args->at(0)->compare(args->at(1)))->el();
              } else {
                THROW("= : pass two arguments to compare");
                abort();
              }
            }));

  core->set(">", func([](ListP args) {
              if (args->size() == 2 and args->at(0)->type == NUMBER and
                  args->at(1)->type == NUMBER) {
                return boolean(args->at(0)->to<Number>()->value() >
                               args->at(1)->to<Number>()->value())
                    ->el();
              } else {
                THROW("> : pass two numbers to compare");
              }
            }));

  core->set("<", func([](ListP args) {
              if (args->size() == 2 and args->at(0)->type == NUMBER and
                  args->at(1)->type == NUMBER) {
                return boolean(args->at(0)->to<Number>()->value() <
                               args->at(1)->to<Number>()->value())
                    ->el();
              } else {
                THROW("< : pass two numbers to compare");
              }
            }));

  core->set(">=", func([](ListP args) {
              if (args->size() == 2 and args->at(0)->type == NUMBER and
                  args->at(1)->type == NUMBER) {
                return boolean(args->at(0)->to<Number>()->value() >=
                               args->at(1)->to<Number>()->value())
                    ->el();
              } else {
                THROW(">= : pass two numbers to compare");
              }
            }));

  core->set("<=", func([](ListP args) {
              if (args->size() == 2 and args->at(0)->type == NUMBER and
                  args->at(1)->type == NUMBER) {
                return boolean(args->at(0)->to<Number>()->value() <=
                               args->at(1)->to<Number>()->value())
                    ->el();
              } else {
                THROW("<= : pass two numbers to compare");
              }
            }));

  // ****************************** MATH ***********************************

  core->set("+", func([](ListP args) {
#ifdef _LM_WITH_FLOAT
              float tot = 0;
#else
              int tot = 0;
#endif
              if (args->size() > 0) {
                for (unsigned int i = 0; i < args->size(); i++) {
                  if (args->at(i)->type == NUMBER) {
                    tot += args->at(i)->to<Number>()->value();
                  } else
                    return exc("+: arguments must be numbers")->el();
                }
                return num(tot)->el();
              } else
                return num(0)->el();
            }));

  core->set("-", func([](ListP args) {
#ifdef _LM_WITH_FLOAT
              float tot = 0;
#else
              int tot = 0;
#endif
              if (args->size() > 0) {
                if (args->at(0)->type == NUMBER) {
                  tot += args->at(0)->to<Number>()->value();
                  if (args->size() > 1) {
                    for (unsigned int i = 1; i < args->size(); i++) {
                      if (args->at(i)->type == NUMBER) {
                        tot -= args->at(i)->to<Number>()->value();
                      } else
                        return exc("-: arguments must be numbers")->el();
                    }
                  }
                  return num(tot)->el();
                } else {
                  return nil();
                }
              } else
                return num(0)->el();
            }));

  core->set("*", func([](ListP args) {
#ifdef _LM_WITH_FLOAT
              float tot = 1;
#else
              int tot = 1;
#endif
              if (args->size() > 0) {
                for (unsigned int i = 0; i < args->size(); i++) {
                  if (args->at(i)->type == NUMBER) {
                    tot *= args->at(i)->to<Number>()->value();
                  } else
                    return exc("*: arguments must be numbers")->el();
                }
                return num(tot)->el();
              } else
                return num(0)->el();
            }));

  core->set("/", func([](ListP args) {
#ifdef _LM_WITH_FLOAT
              float tot = 1;
#else
              int tot = 1;
#endif
              if (args->size() > 0) {
                if (args->at(0)->type == NUMBER) {
                  tot *= args->at(0)->to<Number>()->value();
                  if (args->size() > 1) {
                    for (unsigned int i = 1; i < args->size(); i++) {
                      if (args->at(i)->type == NUMBER) {
                        if (args->at(i)->to<Number>()->value() != 0)
                          tot /= args->at(i)->to<Number>()->value();
                        else {
                          // DIVISION BY ZERO
                          return exc("/: division by zero")->el();
                        }
                      } else
                        return exc("/: arguments must be numbers")->el();
                    }
                  }
                  return num(tot)->to<Element>();
                } else {
                  return nil();
                }
              } else
                return num(0)->to<Element>();
            }));

  // ***************************** ATOMS **********************************

  core->set("atom", func([](ListP args) {
              TEST_DO_OR_EXC(
                  args->size() == 1, { return atom(args->at(0))->el(); },
                  "atom: takes one argument");
            }));

  core->set("atom?", func([](ListP args) {
              TEST_DO_OR_EXC(
                  args->size() == 1,
                  {
                    if (args->at(0)->type == ATOM)
                      return boolean(true)->el();
                    else
                      return boolean(false)->el();
                  },
                  "atom?: takes one argument");
            }));

  core->set("deref", func([](ListP args) {
              TEST_DO_OR_EXC(
                  args->size() == 1, { return args->at(0)->to<Atom>()->ref; },
                  "deref: takes one argument");
            }));

  core->set(
      "reset!", func([](ListP args) {
        TEST_DO_OR_EXC(
            args->size() == 2 and args->at(0)->type == ATOM,
            {
              args->at(0)->to<Atom>()->ref = args->at(1);
              return args->at(1);
            },
            "reset!: takes an atom as first argument and a value as second");
      }));

  core->set(
      "swap!", func([core](ListP args) {
        if (args->at_least(2) and args->at(0)->type == ATOM and
            args->at(1)->type == FUNCTION) {
          FunctionP f = args->at(1)->to<Function>();
          AtomP a = args->at(0)->to<Atom>();
          ListP f_args = list();
          f_args->append(a->ref);
          for (unsigned int i = 2; i < args->size(); ++i) {
            f_args->append(eval_ast(args->at(i), core));
          }
          a->ref = apply(f, f_args, core);
          return a->el();
        } else {
          THROW("swap!: takes an atom as first argument, a function as second"
                "and others function parameters as rest");
        }
      }));

  // ************************** EXCEPTIONS ********************************

  core->set("throw", func([](ListP args) {
              if (args->at(0)->type == STRING) {
                THROW(args->at(0)->to<String>()->value());
              } else if (args->at(0)->type == DICT and
                         args->at(0)->to<Dict>()->keys()->size() > 0) {
                DictP e_dict = args->at(0)->to<Dict>();
                ElementP key = e_dict->keys()->at(0);
                ElementP value = e_dict->get(key);
                THROW(pr_str(key) + ":" + pr_str(value));
              } else {
                // THROW("\\" + pr_str(args->at(0)) + "\\");
                Runtime::raised = true;
                Runtime::exc_value = args->at(0);
                return nil();
              }
            }));

  // *************************** META-DATA ********************************

  core->set("meta", func([](ListP args) {
              if (args->at_least(1)) {
                return get_meta(args->at(0));
              } else {
                THROW("meta: pass at least one argument");
              }
            }));

  core->set(
      "with-meta", func([](ListP args) {
        if (args->at_least(2)) {
          ElementP ret;
          switch (args->at(0)->type) {
          case FUNCTION: {
            ret = copy(args->at(0)->to<Function>());
            set_meta(ret, args->at(1));
          } break;
          case LIST: {
            ret = copy(args->at(0)->to<List>());
            set_meta(ret, args->at(1));
          } break;
          case VEC: {
            ret = copy(args->at(0)->to<Vec>());
            set_meta(ret, args->at(1));
          } break;
          case DICT: {
            ret = copy(args->at(0)->to<Dict>());
            set_meta(ret, args->at(1));
          } break;
          default:
            THROW(
                "with-meta: first argument must be a function, list, vector or"
                "hash-map");
          }
          return ret;
        } else {
          THROW("with-meta: pass two arguments");
        }
      }));

  return core;
}

void post_init(Runtime &r, std::string filename) {
  r.rep("(def! not (fn* (x) (if x false true)))");
  r.rep("(def! load-file (fn* (f) (eval (read-string"
        " (str \"(do \" (slurp f) \"\nnil)\")))))");
  r.rep("(defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) "
        "(if (> (count xs) 1) (nth xs 1) (throw \"odd number of forms to "
        "cond\")) (cons 'cond (rest (rest xs)))))))");

  if (not filename.empty()) {
    r.rep("(load-file \"" + filename + "\")");
  }
}
} // namespace lmlisp
