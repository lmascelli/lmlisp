#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace lmlisp {
enum TYPES {
  NIL,
  SYMBOL,
  FUNCTION,
  ENVIRONMENT,
  KEYWORD,
  BOOLEAN,
  NUMBER,
  STRING,
  LIST,
  VEC,
  DICT,
  ATOM,
  EXCEPTION,
};

class Element;
class Environment;
class Nil;
class Boolean;
class List;
class Vec;
class Dict;
class Number;
class Symbol;
class Keyword;
class String;
class Function;
class Atom;
class Exception;
using ElementP = std::shared_ptr<Element>;
using EnvironmentP = std::shared_ptr<Environment>;
using ListP = std::shared_ptr<List>;
using VecP = std::shared_ptr<Vec>;
using DictP = std::shared_ptr<Dict>;
using NumberP = std::shared_ptr<Number>;
using SymbolP = std::shared_ptr<Symbol>;
using BooleanP = std::shared_ptr<Boolean>;
using KeywordP = std::shared_ptr<Keyword>;
using StringP = std::shared_ptr<String>;
using FunctionP = std::shared_ptr<Function>;
using AtomP = std::shared_ptr<Atom>;
using ExceptionP = std::shared_ptr<Exception>;

// ELEMENT
class Element : public std::enable_shared_from_this<Element> {
public:
  Element(TYPES type);
  TYPES type;
  template <class T> std::shared_ptr<T> to() {
    return std::static_pointer_cast<T>(shared_from_this());
  }
  ElementP el();
  bool compare(ElementP el);
  friend ElementP copy(ElementP el);
};

// NIL
class Nil : public Element {
public:
  Nil();
  friend ElementP copy(ElementP el);
};

// FUNCTION
class Function : public Element {
public:
  Function(std::function<ElementP(ListP)>);
  Function(EnvironmentP outer, ListP binds, ElementP exprs,
           bool last_is_variadic);
  bool is_native() const;
  EnvironmentP create_env(EnvironmentP outer, ListP args);
  ElementP apply(ListP args);
  ElementP get_exprs();
  bool is_macro;
  friend ElementP copy(ElementP el);
  friend ElementP get_meta(ElementP el);
  friend void set_meta(ElementP el, ElementP meta);

private:
  ListP binds;
  ElementP exprs;
  EnvironmentP env;
  bool last_is_variadic;
  std::function<ElementP(ListP)> f_native;
  bool native;
  ElementP meta;
};

// ENVIRONMENT

class Environment : public Element {
public:
  Environment(ElementP outer);
  ElementP get(std::string key);
  ElementP find(std::string key);
  void set(std::string key, ElementP value);
  int get_level() const;
  friend ElementP copy(ElementP el);

private:
  std::unordered_map<std::string, ElementP> env;
  ListP exprs;
  EnvironmentP outer;
  int level;
};

// BOOLEAN
class Boolean : public Element {
public:
  Boolean(bool logic_value = false);
  bool value() const;
  friend ElementP copy(ElementP el);

private:
  bool logic_value;
};

// LIST
class List : public Element {
public:
  List();
  void append(ElementP el);
  ElementP at(unsigned int i) const;
  unsigned int size() const;
  bool at_least(unsigned int n) const;
  bool check_nth(int n, TYPES t) const;
  friend ElementP copy(ElementP el);
  friend ElementP get_meta(ElementP el);
  friend void set_meta(ElementP el, ElementP meta);

private:
  std::vector<ElementP> elements;
  ElementP meta;
};

// VEC
class Vec : public Element {
public:
  Vec();
  void append(ElementP el);
  ElementP at(unsigned int i) const;
  unsigned int size() const;
  bool at_least(unsigned int n) const;
  bool check_nth(int n, TYPES t) const;
  ListP listed();
  friend ElementP copy(ElementP el);
  friend ElementP get_meta(ElementP el);
  friend void set_meta(ElementP el, ElementP meta);

private:
  std::vector<ElementP> elements;
  ElementP meta;
};

// DICT
class Dict : public Element {
public:
  Dict();
  void append(ElementP key, ElementP value);
  ElementP get(ElementP key);
  ElementP contains(ElementP key);
  ListP keys() const;
  friend ElementP copy(ElementP el);
  friend ElementP get_meta(ElementP el);
  friend void set_meta(ElementP el, ElementP meta);

private:
  std::unordered_map<std::string, ElementP> elements;
  ElementP meta;
};

// NUMBER
#ifdef _LM_WITH_FLOAT
class Number : public Element {
public:
  Number(float number);
  float value() const;

  friend ElementP copy(ElementP el);

private:
  float data;
};
#else
class Number : public Element {
public:
  Number(int number);
  int value() const;
  friend ElementP copy(ElementP el);

private:
  int data;
};
#endif

// SYMBOL
class Symbol : public Element {
public:
  Symbol(std::string symbol);
  std::string value() const;

  friend ElementP copy(ElementP el);

private:
  std::string data;
};

// KEYWORD
class Keyword : public Element {
public:
  Keyword(std::string keyword);
  std::string value() const;

  friend ElementP copy(ElementP el);

private:
  std::string data;
};

// STRING
class String : public Element {
public:
  String(std::string string);
  std::string value() const;

  friend ElementP copy(ElementP el);

private:
  std::string data;
};

// ATOM

class Atom : public Element {
public:
  Atom(ElementP el);
  ElementP ref;

  friend ElementP copy(ElementP el);
};

// EXCEPTION

class Exception : public Element {
public:
  Exception(std::string msg);
  std::string value() const;

  friend ElementP copy(ElementP el);

private:
  std::string msg;
};

// POINTER CONSTRUCTORS
ElementP nil();
ListP list();
VecP vec();
DictP dict();
BooleanP boolean(bool value);
NumberP num(float number);
SymbolP sym(std::string symbol);
KeywordP kw(std::string keyword);
StringP str(std::string string);
FunctionP func(std::function<ElementP(ListP)> f);
FunctionP func(EnvironmentP outer, ListP binds, ElementP exprs,
               bool last_is_variadic = false);
EnvironmentP environment(EnvironmentP outer);
AtomP atom(ElementP ref);
ExceptionP exc(std::string msg);

// UTILITY FUNCTIONS

ElementP copy(ElementP el);
inline bool is_nil(ElementP el);
ElementP get_meta(ElementP el);
void set_meta(ElementP el, ElementP meta);
} // namespace lmlisp
