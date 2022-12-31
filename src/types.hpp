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
using ElementP = std::shared_ptr<Element>;
using EnvironmentP = std::shared_ptr<Environment>;
using ListP = std::shared_ptr<List>;
using VecP = std::shared_ptr<Vec>;
using DictP = std::shared_ptr<Dict>;
using NumberP = std::shared_ptr<Number>;
using SymbolP = std::shared_ptr<Symbol>;
using KeywordP = std::shared_ptr<Keyword>;
using StringP = std::shared_ptr<String>;
using FunctionP = std::shared_ptr<Function>;

// ELEMENT
class Element : public std::enable_shared_from_this<Element> {
public:
  Element(TYPES type);
  TYPES type;
  template <class T> std::shared_ptr<T> to() {
    return std::static_pointer_cast<T>(shared_from_this());
  }
};

// NIL
class Nil : public Element {
public:
  Nil();
};

// FUNCTION
class Function : public Element {
public:
  Function(std::function<ElementP(ElementP)>);
  bool is_native() const;
  ElementP apply(ElementP args) const;

private:
  std::function<ElementP(ElementP)> f_native;
  bool native;
};

// ENVIRONMENT

  class Environment : public Element {
public:
  Environment(ElementP outer);
  ElementP get(std::string key);
  ElementP find(std::string key);
  void set(std::string key, ElementP value);

private:
  std::unordered_map<std::string, ElementP> env;
  EnvironmentP outer;
};

// BOOLEAN
class Boolean : public Element {
public:
  Boolean(bool logic_value = false);
  bool value() const;

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
  bool at_least(int n) const;
  bool check_nth(int n, TYPES t) const;

private:
  std::vector<ElementP> elements;
};

// VEC
class Vec : public Element {
public:
  Vec();
  void append(ElementP el);
  ElementP at(unsigned int i) const;
  unsigned int size() const;
  bool at_least(int n) const;
  bool check_nth(int n, TYPES t) const;

private:
  std::vector<ElementP> elements;
};

// DICT
class Dict : public Element {
public:
  Dict();
  void append(ElementP key, ElementP value);
  ElementP get(ElementP key);
  ElementP keys() const;

private:
  // TODO make an hashmap that distinguish between string and keywords
  std::unordered_map<std::string, ElementP> elements;
};

// NUMBER
#ifdef _LM_WITH_FLOAT
class Number : public Element {
public:
  Number(float number);
  float value() const;

private:
  float data;
};
#else
class Number : public Element {
public:
  Number(int number);
  int value() const;

private:
  int data;
};
#endif

// SYMBOL
class Symbol : public Element {
public:
  Symbol(std::string symbol);
  std::string value() const;

private:
  std::string data;
};

// KEYWORD
class Keyword : public Element {
public:
  Keyword(std::string keyword);
  std::string value() const;

private:
  std::string data;
};

// STRING
class String : public Element {
public:
  String(std::string string);
  std::string value() const;

private:
  std::string data;
};

// POINTER CONSTRUCTORS
ElementP nil();
ElementP list();
ElementP vec();
ElementP dict();
ElementP boolean(bool value);
ElementP num(float number);
ElementP sym(std::string symbol);
ElementP kw(std::string keyword);
ElementP str(std::string string);
ElementP func(std::function<ElementP(ElementP)> f);
ElementP environment(EnvironmentP outer);

// UTILITY FUNCTIONS

inline bool is_nil(ElementP el);

} // namespace lmlisp
