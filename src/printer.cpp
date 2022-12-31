#include "printer.hpp"

namespace lmlisp {

inline void replace_str(std::string &s, const std::string &old_s,
                        const std::string &new_s) {
  int index = s.find(old_s, 0);
  while (index != std::string::npos) {
    s.replace(index, old_s.length(), new_s);
    index = s.find(old_s, index + new_s.length());
  }
}

std::string Printer::pr_str(ElementP el, bool print_readably) {
  switch (el->type) {
  case NIL:
    return "nil";
  case BOOLEAN:
    return std::to_string(el->to<Boolean>()->value());
  case NUMBER:
    return std::to_string(el->to<Number>()->value());
  case STRING:
    if (print_readably) {
      return "\"" + el->to<String>()->value() + "\"";
    } else {
      std::string ret = el->to<String>()->value();
      replace_str(ret, "\\n", "\n");
      replace_str(ret, "\\\"", "\"");
      replace_str(ret, "\\\\", "\\");
      return "\"" + ret + "\"";
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
    for (int i = 0; i < keys->size(); ++i) {
      ret += pr_str(keys->at(i), print_readably) + " " +
             pr_str(d->get(keys->at(i)), print_readably);
      if (i < (keys->size() - 1))
        ret += " ";
    }
    ret += "}";
    return ret;
  }
  default:
    return std::string("printer ERROR: something else");
  }
}
} // namespace lmlisp
