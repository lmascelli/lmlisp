#include "types.hpp"
#include "externals.hpp"
#include <cassert>
#include <memory>
#include <stdlib.h>

namespace lmlisp {

// ELEMENT
Element::Element(TYPES type) { this->type = type; }
ElementP Element::el() {
  return std::static_pointer_cast<Element>(shared_from_this());
}
bool Element::compare(ElementP el) {
  if (this->type == el->type) {
    switch (this->type) {
    case LIST: {
      ListP _arg1 = this->to<List>(), _arg2 = el->to<List>();
      if (_arg1->size() != _arg2->size())
        return false;
      else {
        for (unsigned int i = 0; i < _arg1->size(); i++)
          if (not _arg1->at(i)->compare(_arg2->at(i)))
            return false;
        return true;
      }
    }
    case VEC: {
      VecP _arg1 = this->to<Vec>(), _arg2 = el->to<Vec>();
      if (_arg1->size() != _arg2->size())
        return false;
      else {
        for (unsigned int i = 0; i < _arg1->size(); i++)
          if (not _arg1->at(i)->compare(_arg2->at(i)))
            return false;
        return true;
      }
    }
    case DICT: {
      ListP _arg1 = this->to<Dict>()->keys(), _arg2 = el->to<Dict>()->keys();
      if (_arg1->size() != _arg2->size())
        return false;
      else {
        for (unsigned int i = 0; i < _arg1->size(); i++) {
          ElementP key = _arg1->at(i);
          if (not this->to<Dict>()->get(key)->compare(el->to<Dict>()->get(key)))
            return false;
        }
        return true;
      }
    }
    case NUMBER:
      return this->to<Number>()->value() == el->to<Number>()->value();
    case STRING:
      return this->to<String>()->value() == el->to<String>()->value();
    case KEYWORD:
      return this->to<Keyword>()->value() == el->to<Keyword>()->value();
    case SYMBOL:
      return this->to<Symbol>()->value() == el->to<Symbol>()->value();
    case FUNCTION:
      writeln("= : comparison of two Functions");
      abort();
      return false;
    case NIL:
      return true;
    case BOOLEAN:
      return this->to<Boolean>()->value() == el->to<Boolean>()->value();
    case ENVIRONMENT:
      writeln("= : comparison of two Environments");
      abort();
      return false;
    case EXCEPTION:
      return this->to<Exception>()->value() == el->to<Exception>()->value();
    case ATOM:
      return this->el() == el;
    }
  } else {
    if (this->type == LIST and el->type == VEC) {
      ListP _arg1 = this->to<List>();
      VecP _arg2 = el->to<Vec>();
      if (_arg1->size() != _arg2->size())
        return false;
      else {
        for (unsigned int i = 0; i < _arg1->size(); i++)
          if (not _arg1->at(i)->compare(_arg2->at(i)))
            return false;
        return true;
      }
    } else if (this->type == VEC and el->type == LIST) {
      VecP _arg1 = this->to<Vec>();
      ListP _arg2 = el->to<List>();
      if (_arg1->size() != _arg2->size())
        return false;
      else {
        for (unsigned int i = 0; i < _arg1->size(); i++)
          if (not _arg1->at(i)->compare(_arg2->at(i)))
            return false;
        return true;
      }
    } else {
      return false;
    }
  }
  return false;
}

// NIL
Nil::Nil() : Element(NIL) {}

// FUNCTION
Function::Function(std::function<ElementP(ListP)> f_native)
    : Element(FUNCTION) {
  native = true;
  this->f_native = f_native;
}

Function::Function(EnvironmentP outer, ListP binds, ElementP exprs,
                   bool last_is_variadic)
    : Element(FUNCTION), is_macro(false) {
  native = false;
  this->env = environment(outer);
  this->exprs = exprs;
  this->binds = binds;
  this->last_is_variadic = last_is_variadic;
}

bool Function::is_native() const { return native; }

EnvironmentP Function::create_env([[maybe_unused]]EnvironmentP outer, ListP args) {
  EnvironmentP apply_env = environment(env);
  if (binds->size() > 0) {
    for (unsigned int i = 0; i < binds->size() - 1; i++) {
      apply_env->set(binds->at(i)->to<Symbol>()->value(),
                     args->to<List>()->at(i));
    }
    if (last_is_variadic) {
      ListP varargs = list();
      for (unsigned int i = binds->size() - 1; i < args->to<List>()->size();
           i++) {
        varargs->append(args->to<List>()->at(i));
      }
      apply_env->set(binds->at(binds->size() - 1)->to<Symbol>()->value(),
                     varargs);
    } else {
      apply_env->set(binds->at(binds->size() - 1)->to<Symbol>()->value(),
                     args->to<List>()->at(binds->size() - 1));
    }
  }
  return apply_env;
}

ElementP Function::apply(ListP args) {
  assert(is_native() && "PANIC: function is not native");
  return f_native(args);
}

ElementP Function::get_exprs() { return exprs; }

// ENVIRONMENT
Environment::Environment(ElementP outer) : Element(ENVIRONMENT) {
  if (outer->type == NIL)
    level = 0;
  else
    level = outer->to<Environment>()->level + 1;
  this->outer = outer->to<Environment>();
}

ElementP Environment::find(std::string key) {
  if (env.contains(key))
    return shared_from_this();
  else if (not is_nil(outer))
    return outer->find(key);
  else
    return nil();
}

ElementP Environment::get(std::string key) {
  ElementP found_env = find(key);
  if (not is_nil(found_env)) {
    return found_env->to<Environment>()->env.at(key);
  } else
    return exc("symbol: " + key + " not found");
}

void Environment::set(std::string key, ElementP value) {
  env.insert_or_assign(key, value);
}

int Environment::get_level() const { return level; }

// BOOLEAN
Boolean::Boolean(bool logic_value)
    : Element(BOOLEAN), logic_value(logic_value) {}
bool Boolean::value() const { return logic_value; }

// LIST
List::List() : Element(LIST) {}
void List::append(ElementP el) { elements.push_back(el); }
ElementP List::at(unsigned int i) const { return elements[i]; }
unsigned int List::size() const { return elements.size(); }
bool List::at_least(unsigned int n) const { return size() >= n; }
bool List::check_nth(int n, TYPES t) const {
  if (at_least(n+1) and at(n)->type == t)
    return true;
  else
    return false;
}

// VEC
Vec::Vec() : Element(VEC) {}
void Vec::append(ElementP el) { elements.push_back(el); }
ElementP Vec::at(unsigned int i) const { return elements[i]; }
unsigned int Vec::size() const { return elements.size(); }
bool Vec::at_least(unsigned int n) const { return size() >= n; }
bool Vec::check_nth(int n, TYPES t) const {
  if (at_least(n) and at(n)->type == t)
    return true;
  else
    return false;
}
ListP Vec::listed() {
  ListP ret = list();
  for (unsigned int i = 0; i < size(); i++)
    ret->append(at(i));
  return ret;
}

// DICT
Dict::Dict() : Element(DICT) {}

void Dict::append(ElementP key, ElementP value) {
  switch (key->type) {
  case STRING:
    elements.insert_or_assign(key->to<String>()->value(), value);
    break;
  case KEYWORD:
    elements.insert_or_assign(":" + key->to<Keyword>()->value(), value);
    break;
  default:
    exit(1);
  }
}

ElementP Dict::get(ElementP key) {
  std::string key_string;
  switch (key->type) {
  case STRING:
    key_string = key->to<String>()->value();
    break;
  case KEYWORD:
    key_string = ":" + key->to<Keyword>()->value();
    break;
  default:
    return exc("not a valid key passed")->el();
  }

  if (elements.contains(key_string))
    return this->elements[key_string];
  else
    return nil();
}

  ElementP Dict::contains(ElementP key){
  std::string key_string;
  switch (key->type) {
  case STRING:
    key_string = key->to<String>()->value();
    break;
  case KEYWORD:
    key_string = ":" + key->to<Keyword>()->value();
    break;
  default:
    return exc("not a valid key passed")->el();
  }

  return boolean(elements.contains(key_string));
  }

ListP Dict::keys() const {
  ListP ret = list();
  std::vector<std::string> keys_strings;
  for (std::pair<std::string, ElementP> el : elements) {
    keys_strings.push_back(el.first);
  }
  for (int i = keys_strings.size() - 1; i >= 0; i--) {
    if (keys_strings[i].starts_with(":")) {
      ret->append(kw(keys_strings[i].substr(1)));
    } else {
      ret->append(str(keys_strings[i]));
    }
  }
  return ret;
}

// NUMBER
#ifdef _LM_WITH_FLOAT
Number::Number(float number) : Element(NUMBER) { this->data = number; }
float Number::value() const { return data; }
#else
Number::Number(int number) : Element(NUMBER) { this->data = number; }
int Number::value() const { return data; }
#endif

// SYMBOL
Symbol::Symbol(std::string symbol) : Element(SYMBOL) { this->data = symbol; }
std::string Symbol::value() const { return data; }

// KEYWORD
Keyword::Keyword(std::string keyword) : Element(KEYWORD) {
  this->data = keyword;
}
std::string Keyword::value() const { return data; }

// STRING
String::String(std::string string) : Element(STRING) { this->data = string; }
std::string String::value() const { return data; }

// ATOM

Atom::Atom(ElementP ref) : Element(ATOM) { this->ref = ref; }

// EXCEPTION

Exception::Exception(std::string msg) : Element(EXCEPTION) { this->msg = msg; }
std::string Exception::value() const { return msg; }

// POINTER CONSTRUCTORS
ElementP nil() { return std::make_shared<Nil>(); }
ListP list() { return std::make_shared<List>(); }
VecP vec() { return std::make_shared<Vec>(); }
DictP dict() { return std::make_shared<Dict>(); }
BooleanP boolean(bool value) { return std::make_shared<Boolean>(value); }
NumberP num(float number) { return std::make_shared<Number>(number); }
SymbolP sym(std::string symbol) { return std::make_shared<Symbol>(symbol); }
KeywordP kw(std::string keyword) { return std::make_shared<Keyword>(keyword); }
StringP str(std::string string) { return std::make_shared<String>(string); }
FunctionP func(std::function<ElementP(ListP)> f) {
  return std::make_shared<Function>(f);
}
FunctionP func(EnvironmentP outer, ListP binds, ElementP exprs,
               bool last_is_variadic) {
  return std::make_shared<Function>(outer, binds, exprs, last_is_variadic);
}
EnvironmentP environment(EnvironmentP outer) {
  return std::make_shared<Environment>(outer);
}
AtomP atom(ElementP ref) { return std::make_shared<Atom>(ref); }
ExceptionP exc(std::string msg) { return std::make_shared<Exception>(msg); }

// UTILITY FUNCTIONS

inline bool is_nil(ElementP el) { return el->type == NIL; }
} // namespace lmlisp

