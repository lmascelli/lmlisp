#pragma once
#include <memory>
#include <vector>

namespace lmlisp {
enum TYPES {
  SYMBOL,
  NUMBER,
  STRING,
  LIST,
};

class Element;
class List;
class Number;
class Symbol;
class String;
using ElementP = std::shared_ptr<Element>;
using ListP = std::shared_ptr<List>;
using NumberP = std::shared_ptr<Number>;
using SymbolP = std::shared_ptr<Symbol>;
using StringP = std::shared_ptr<String>;

// ELEMENT
class Element : public std::enable_shared_from_this<Element> {
public:
  Element(TYPES type);
  TYPES type;
  template <class T> std::shared_ptr<T> to() {
    return std::static_pointer_cast<T>(shared_from_this());
  }
};

// LIST
class List : public Element {
public:
  List();
  void append(ElementP el);
  ElementP &at(unsigned int i);
  unsigned int size() const;

private:
  std::vector<ElementP> elements;
};

// NUMBER
class Number : public Element {
public:
  Number(float number);
  float value() const;

private:
  float data;
};

// SYMBOL
class Symbol : public Element {
public:
  Symbol(std::string symbol);
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
ElementP list();
ElementP num(float number);
ElementP sym(std::string symbol_);
ElementP str(std::string string_);
} // namespace lmlisp
