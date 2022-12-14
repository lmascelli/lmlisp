#include "types.hpp"
#include <memory>

namespace lmlisp {

// ELEMENT
Element::Element(TYPES type) { this->type = type; }

// NIL
Nil::Nil() : Element(NIL) {}

// FUNCTION
Function::Function(std::function<ElementP(ElementP)> f_native)
    : Element(FUNCTION) {
  native = true;
  this->f_native = f_native;
}

bool Function::is_native() const { return native; }

ElementP Function::apply(ElementP args) const { return f_native(args); }

// ENVIRONMENT
Environment::Environment(ElementP outer) : Element(ENVIRONMENT) {
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
    return nil();
}
void Environment::set(std::string key, ElementP value) {
  env.insert_or_assign(key, value);
}

// BOOLEAN
Boolean::Boolean(bool logic_value)
    : Element(BOOLEAN), logic_value(logic_value) {}
bool Boolean::value() const { return logic_value; }

// LIST
List::List() : Element(LIST) {}
void List::append(ElementP el) { elements.push_back(el); }
ElementP List::at(unsigned int i) const { return elements[i]; }
unsigned int List::size() const { return elements.size(); }
bool List::at_least(int n) const { return size() >= n; }
bool List::check_nth(int n, TYPES t) const {
  if (at_least(n) and at(n)->type == t)
    return true;
  else
    return false;
}

// VEC
Vec::Vec() : Element(VEC) {}
void Vec::append(ElementP el) { elements.push_back(el); }
ElementP Vec::at(unsigned int i) const { return elements[i]; }
unsigned int Vec::size() const { return elements.size(); }
bool Vec::at_least(int n) const { return size() >= n; }
bool Vec::check_nth(int n, TYPES t) const {
  if (at_least(n) and at(n)->type == t)
    return true;
  else
    return false;
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
    exit(1);
  }

  if (elements.contains(key_string))
    return this->elements[key_string];
  else
    return nil();
}

ElementP Dict::keys() const {
  ElementP ret = list();
  std::vector<std::string> keys_strings;
  for (std::pair<std::string, ElementP> el : elements) {
    keys_strings.push_back(el.first);
  }
  for (int i = keys_strings.size() - 1; i >= 0; i--) {
    if (keys_strings[i].starts_with(":")) {
      ret->to<List>()->append(kw(keys_strings[i].substr(1)));
    } else {
      ret->to<List>()->append(str(keys_strings[i]));
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

// POINTER CONSTRUCTORS
ElementP nil() { return std::make_shared<Nil>(); }
ElementP list() { return std::make_shared<List>(); }
ElementP vec() { return std::make_shared<Vec>(); }
ElementP dict() { return std::make_shared<Dict>(); }
ElementP boolean(bool value) { return std::make_shared<Boolean>(value); }
ElementP num(float number) { return std::make_shared<Number>(number); }
ElementP sym(std::string symbol) { return std::make_shared<Symbol>(symbol); }
ElementP kw(std::string keyword) { return std::make_shared<Keyword>(keyword); }
ElementP str(std::string string) { return std::make_shared<String>(string); }
ElementP func(std::function<ElementP(ElementP)> f) {
  return std::make_shared<Function>(f);
}
ElementP environment(EnvironmentP outer) {
  return std::make_shared<Environment>(outer);
}
// UTILITY FUNCTIONS

inline bool is_nil(ElementP el) { return el->type == NIL; }

} // namespace lmlisp
