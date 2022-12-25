#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

namespace lmlisp {
  enum TYPES {
    NIL,
    SYMBOL,
    KEYWORD,
    BOOLEAN,
    NUMBER,
    STRING,
    LIST,
    VEC,
    DICT,
  };

  class Element;
  class Nil;
  class Boolean;
  class List;
  class Vec;
  class Dict;
  class Number;
  class Symbol;
  class Keyword;
  class String;
  using ElementP = std::shared_ptr<Element>;
  using ListP = std::shared_ptr<List>;
  using VecP = std::shared_ptr<Vec>;
  using DictP = std::shared_ptr<Dict>;
  using NumberP = std::shared_ptr<Number>;
  using SymbolP = std::shared_ptr<Symbol>;
  using KeywordP = std::shared_ptr<Keyword>;
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

  // NIL
  class Nil : public Element {
  public:
    Nil();
  };

  // BOOL
  class Boolean : public Element {
  public:
    Boolean(bool logic_value=false);
    bool value() const;
  private:
    bool logic_value;
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

  // VEC
  class Vec : public Element {
  public:
    Vec();
    void append(ElementP el);
    ElementP &at(unsigned int i);
    unsigned int size() const;

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

  // KEYWORD
  class Keyword : public Element
  {
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
} // namespace lmlisp
