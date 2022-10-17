#include "types.hpp"
#include <iostream>
#include <memory>

namespace lmlisp {

// ELEMENT
Element::Element(TYPES type) { this->type = type; }

// LIST
List::List() : Element(LIST) {}
void List::append(ElementP el) { elements.push_back(el); }
ElementP &List::at(unsigned int i) { return elements[i]; }
unsigned int List::size() const { return elements.size(); }

// NUMBER
Number::Number(float number) : Element(NUMBER) { this->data = number; }
float Number::value() const { return data; }

// SYMBOL
Symbol::Symbol(std::string symbol) : Element(SYMBOL) { this->data = symbol; }
std::string Symbol::value() const { return data; }

// STRING
String::String(std::string string) : Element(STRING) { this->data = string; }
std::string String::value() const { return data; }

// POINTER CONSTRUCTORS
ElementP list() { return std::make_shared<List>(); }
ElementP num(float number) { return std::make_shared<Number>(number); }
ElementP sym(std::string symbol) { return std::make_shared<Symbol>(symbol); }
ElementP str(std::string string) { return std::make_shared<String>(string); }
} // namespace lmlisp
