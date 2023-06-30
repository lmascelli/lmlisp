#pragma once
#include "types.hpp"
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

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
      Reader();
      std::string next();
      std::string peek();
      bool end() const;

      ElementP read_form();
      ElementP read_list();
      ElementP read_vec();
      ElementP read_dict();
      ElementP read_atom();

      friend ElementP read_str(const std::string& input);

    private:
      enum TOKEN_TYPE {
        UNKNOWN,
        STRING,
        SPACES,
        COMMENT,
      };

      void add_token(std::string token);
      std::variant<std::vector<std::string>, ExceptionP>
        tokenize(const std::string input);

      std::vector<std::string> tokens;
      int pos;

      class Brackets_Checker {
        public:
          std::optional<ExceptionP> push_bracket(char br);
          std::optional<ExceptionP> close();

        private:
          BRACKETS_BALANCING_ERROR check_stack(char br);
          BRACKETS_BALANCING_ERROR close_stack();
          std::optional<ExceptionP>
            generate_exception_error(BRACKETS_BALANCING_ERROR error);

          std::vector<char> bracket_stack;
      };
  };

  ElementP read_str(const std::string &input);
}
