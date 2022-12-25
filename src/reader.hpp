#pragma once
#include <vector>
#include <string>
#include <functional>
#include "types.hpp"

namespace lmlisp {
  enum BRACKETS_BALANCING_ERROR {
      NO_ERROR = 0,
      UNMATCHED_CURVE,
      UNMATCHED_SQUARED,
      UNMATCHED_BRACKET,
      UNMATCHED_DOUBLEQUOTE,
      UNBALANCED_CURVE,
      UNBALANCED_SQUARED,
      UNBALANCED_BRACKET,
      UNBALANCED_DOUBLEQUOTE,
      NOT_A_BRACKET,
  };
  
  class Reader {
  public:
    Reader(std::function<void(std::string)> printer);
    void read_str(const std::string& input);
    std::string next();
    std::string peek();
    bool end() const;

    ElementP read_form();
    ElementP read_list();
    ElementP read_vec();
    ElementP read_dict();
    ElementP read_atom();

  private:
    enum TOKEN_TYPE {
      UNKNOWN,
      STRING,
      SPACES,
      COMMENT,
    };

    void add_token(std::string token);
    std::vector<std::string> tokenize(const std::string input);
    std::function<void(std::string)> &printer;
    
    std::vector<std::string> tokens;
    int pos;


    class Brackets_Checker {
    public:
      Brackets_Checker(std::function<void(std::string)> printer);
      void push_bracket(char br);
      void close();
      
    private:
      int check_stack(char br);
      int close_stack();
      void react_error(int error);
      
      std::function<void(std::string)> &printer;
      std::vector<char> bracket_stack;
    };
  };

}
