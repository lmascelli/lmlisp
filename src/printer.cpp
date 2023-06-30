#include "printer.hpp"
#include "externals.hpp"

namespace lmlisp {

  inline void replace_str(std::string &s, const std::string &old_s,
			  const std::string &new_s) {
    size_t index = s.find(old_s, 0);
    while (index != std::string::npos) {
      s.replace(index, old_s.length(), new_s);
      index = s.find(old_s, index + new_s.length());
    }
  }

std::string pr_str(ElementP el, bool print_readably) {
    switch (el->type) {
    case NIL:
      return "nil";
    case BOOLEAN:
      return el->to<Boolean>()->value() ? "true" : "false";
    case NUMBER:
      return std::to_string(el->to<Number>()->value());
    case STRING:
      {
	std::string ret = el->to<String>()->value();
	if (print_readably) {
	  replace_str(ret, "\\", "\\\\");
	  replace_str(ret, "\"", "\\\"");
	  replace_str(ret, "\n", "\\n");
	  ret = "\"" + ret + "\"";	  
	} 
	return ret;
      }
    case SYMBOL:
      return el->to<Symbol>()->value();
    case KEYWORD:
      return ":" + el->to<Keyword>()->value();
    case LIST: {
      ListP l = el->to<List>();
      std::string ret = "(";
      for (unsigned int i = 0; i < l->size(); i++) {
	ret += pr_str(l->at(i), print_readably);
	if (i < (l->size() - 1))
	  ret += " ";
      }
      ret += ")";
      return ret;
    }
    case VEC: {
      VecP l = el->to<Vec>();
      std::string ret = "[";
      for (unsigned int i = 0; i < l->size(); i++) {
	ret += pr_str(l->at(i), print_readably);
	if (i < (l->size() - 1))
	  ret += " ";
      }
      ret += "]";
      return ret;
    }
    case DICT: {
      std::string ret = "{";
      DictP d = el->to<Dict>();
      ListP keys = d->keys()->to<List>();
      for (unsigned int i = 0; i < keys->size(); ++i) {
	ret += pr_str(keys->at(i), print_readably) + " " +
	  pr_str(d->get(keys->at(i)), print_readably);
	if (i < (keys->size() - 1))
	  ret += " ";
      }
      ret += "}";
      return ret;
    }
    case FUNCTION: {
      FunctionP f_el = el->to<Function>();
      if (f_el->is_macro) {
	return std::string("Macro");
      } else
	return std::string("Function");
    }
    case ATOM:
      return std::string("(atom " + pr_str(el->to<Atom>()->ref) + ")");
    case EXCEPTION:
      return el->to<Exception>()->value();
      break;
    default:
      return std::string("printer ERROR: something else");
    }
  }
} // namespace lmlisp

