#include "reader.hpp"
#include "types.hpp"
#include "externals.hpp"
// #include "debug.hpp"

// #define LM_DEBUG
#ifdef LM_DEBUG
// #define _LM_DEBUG
#define _LM_DEBUG_TK
#include <iostream>
using std::cout, std::endl;

#endif

namespace lmlisp {
Reader::Reader() {
  this->pos = -1;
};

bool Reader::end() const { return pos == static_cast<int>(tokens.size()); }

std::string Reader::next() {
  if (pos > static_cast<int>(tokens.size())) {
    writeln(
        "Going over list of tokens bound. Maybe an unclosed list or string");
    abort();
  }
  std::string tok = tokens[++pos];
  return tok;
}

std::string Reader::peek() { return tokens[pos]; }

ElementP read_str(const std::string &input) {
  Reader r;
  auto tokens_ = r.tokenize(input);
  if (tokens_.index() == 0) {
    r.tokens = std::get<std::vector<std::string>>(tokens_);
    return r.read_form();
  } else {
    // TODO return exception
    return std::get<ExceptionP>(tokens_);
  }
}

std::optional<ExceptionP> Reader::Brackets_Checker::close() {
  return generate_exception_error(close_stack());
}

BRACKETS_BALANCING_ERROR Reader::Brackets_Checker::close_stack() {
  if (not bracket_stack.empty()) {
    switch (bracket_stack[bracket_stack.size() - 1]) {
    case '(':
      return UNBALANCED_CURVE;
    case '[':
      return UNBALANCED_SQUARED;
    case '{':
      return UNBALANCED_BRACKET;
    case '"':
      return UNBALANCED_DOUBLEQUOTE;
    default:
      return NOT_A_BRACKET;
    }
  } else
    return NO_ERROR;
}

std::optional<ExceptionP> Reader::Brackets_Checker::generate_exception_error
(BRACKETS_BALANCING_ERROR error) {
  switch (error) {
  case NOT_A_BRACKET:
    return exc("Somehow not a bracket ends here");
  case UNMATCHED_CURVE:
    return exc("Unmatched )");
  case UNMATCHED_SQUARED:
    return exc("Unmatched ]");
  case UNMATCHED_BRACKET:
    return exc("Unmatched }");
  case UNMATCHED_DOUBLEQUOTE:
    return exc("Unmatched \"");
  case UNBALANCED_CURVE:
    return exc("unbalanced");
  case UNBALANCED_SQUARED:
    return exc("unbalanced");
  case UNBALANCED_BRACKET:
    return exc("unbalanced");
  case UNBALANCED_DOUBLEQUOTE:
    return exc("unbalanced");
  case NO_ERROR:
    return std::nullopt;
  default:
    return exc("Reader - bracket balancing: Unknown error");
  }
}

std::optional<ExceptionP> Reader::Brackets_Checker::push_bracket(char br) {
  return generate_exception_error(check_stack(br));
}

BRACKETS_BALANCING_ERROR Reader::Brackets_Checker::check_stack(char br) {
  switch (br) {
  case '(':
  case '[':
  case '{':
    bracket_stack.push_back(br);
    return NO_ERROR;
  case ')':
    if (bracket_stack.empty())
      return UNBALANCED_CURVE;
    if (bracket_stack[bracket_stack.size() - 1] != '(')
      return UNMATCHED_CURVE;
    bracket_stack.pop_back();
    return NO_ERROR;
  case ']':
    if (bracket_stack.empty())
      return UNBALANCED_SQUARED;
    if (bracket_stack[bracket_stack.size() - 1] != '[')
      return UNMATCHED_SQUARED;
    bracket_stack.pop_back();
    return NO_ERROR;
  case '}':
    if (bracket_stack.empty())
      return UNBALANCED_BRACKET;
    if (bracket_stack[bracket_stack.size() - 1] != '{')
      return UNMATCHED_BRACKET;
    bracket_stack.pop_back();
    return NO_ERROR;
  case '"':
    if (not bracket_stack.empty())
      switch (bracket_stack[bracket_stack.size() - 1]) {
      case '(':
      case '[':
      case '{':
        bracket_stack.push_back(br);
        return NO_ERROR;
      case '"':
        bracket_stack.pop_back();
        return NO_ERROR;
      default:
        return NOT_A_BRACKET;
      }
    else {
      bracket_stack.push_back(br);
      return NO_ERROR;
    }
  default:
    return NOT_A_BRACKET;
  }
}

void Reader::add_token(std::string token) { tokens.push_back(token); }
  
#define PUSH_AND_RETURN(br) {					\
    std::optional<ExceptionP> err = bc.push_bracket(br);	\
    if (err.has_value()) {					\
      return err.value();					\
      writeln("returning");					\
    }								\
  }
  
std::variant<std::vector<std::string>, ExceptionP>
Reader::tokenize(std::string input) {
  Brackets_Checker bc;
  TOKEN_TYPE current_token = UNKNOWN;
  unsigned int index = 0, offset = 0;
  bool last_is_escape = false;
  while (index + offset < input.length()) {
    char ch = input.at(index + offset);
    switch (current_token) {
    case STRING:
      if (ch == '"' and not last_is_escape) {
        last_is_escape = false;
        bc.push_bracket('"');
        current_token = UNKNOWN;
        add_token(input.substr(index, offset + 1));
        index += offset + 1;
        offset = 0;
      } else if (ch == '\\') {
        if (last_is_escape)
          last_is_escape = false;
        else
          last_is_escape = true;
        offset++;
      } else if (last_is_escape) {
        switch (ch) {
	case '"':
        case 'n':
        case 't':
          offset++;
          last_is_escape = false;
        }
      } else
        offset++;
      break;
    case SPACES:
      switch (ch) {
      case ' ':
      case '\n':
      case '\t':
        index++;
        break;
      case '"':
        PUSH_AND_RETURN('"');
        index += offset;
        offset = 1;
        current_token = STRING;
        break;
      default:
        offset = 0;
        current_token = UNKNOWN;
      }
      break;
    case COMMENT:
      if (ch == '\n')
        current_token = UNKNOWN;
      index++;
      break;
    case UNKNOWN:
      switch (ch) {
      case ' ':
        if (offset != 0) {
          add_token(input.substr(index, offset));
          index += offset;
          offset = 0;
        }
        current_token = SPACES;
        break;
      case '"':
        PUSH_AND_RETURN('"');
        current_token = STRING;
        index += offset;
        offset = 1;
        break;
      case '~':
        if (input.at(index + offset + 1) == '@') {
          if (offset != 0)
            add_token(input.substr(index, offset));
          add_token(input.substr(index + offset, 2));
          index += offset + 2;
          offset = 0;
          break;
        } else {
          if (offset != 0)
            add_token(input.substr(index, offset));
          add_token(input.substr(index + offset, 1));
          index += offset + 1;
          offset = 0;
          break;
        }
      case '(':
        PUSH_AND_RETURN('(');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case ')':
        PUSH_AND_RETURN(')');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '[':
        PUSH_AND_RETURN('[');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case ']':
        PUSH_AND_RETURN(']');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '{':
        PUSH_AND_RETURN('{');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '}':
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        PUSH_AND_RETURN('}');
        break;
      case '@':
      case '&':
      case '\'':
      case '`':
      case '^':
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '\n':
      case '\t':
      case ',': // TODO test if ignoring comma is correct
        if (offset > 0)
          add_token(input.substr(index, offset));
        index += offset + 1;
        offset = 0;
        break;
      case ';':
        current_token = COMMENT;
        if (offset > 0)
          add_token(input.substr(index, offset));
        index += offset + 1;
        offset = 0;
        break;
      default:
        offset++;
      }
    }
#ifdef _LM_DEBUG_TK
    writeln(input);
    for (int i = 0; i < index; i++)
      cout << " ";
    cout << "*" << endl;
    for (int i = 0; i < (index + offset); i++)
      cout << " ";
    cout << "^" << endl;
    for (auto el : tokens) {
      cout << el << " ";
    }
    std::cin.get();
#endif
  }
  if (input.length() > 0 and index < input.length()) {
    if (current_token == STRING)
      writeln("residue: " + input.substr(index, offset));
    add_token(input.substr(index, offset));
  }
  std::optional<ExceptionP> err = bc.close();
  if (err.has_value()) return err.value();
  return tokens;
}

ElementP Reader::read_form() {
  if (tokens.size() == 0) return nil();
  next();
  switch (peek().at(0)) {
  case '(':
    return read_list();
  case '[':
    return read_vec();
  case '{':
    return read_dict();
  default:
    return read_atom();
  }
}

ElementP Reader::read_list() {
  ElementP ret_list = list();
#ifdef _LM_DEBUG
  writeln("starting list");
#endif
  while (not end()) {
    ElementP el = read_form();
    if (el->type == SYMBOL and el->to<Symbol>()->value() == ")") {
#ifdef _LM_DEBUG
      writeln("ending list");
#endif
      break;
    }
#ifdef _LM_DEBUG
#endif
    ret_list->to<List>()->append(el);
  }
  return ret_list;
}

ElementP Reader::read_vec() {
#ifdef _LM_DEBUG
  writeln("starting vec");
#endif
  ElementP ret_vec = vec();
  while (not end()) {
    ElementP el = read_form();
    if (el->type == SYMBOL and el->to<Symbol>()->value() == "]") {
#ifdef _LM_DEBUG
      writeln("ending vec");
#endif
      break;
    }
    ret_vec->to<Vec>()->append(el);
  }
  return ret_vec;
}

ElementP Reader::read_dict() {
#ifdef _LM_DEBUG
  writeln("starting dict");
#endif
  ElementP ret_dict = dict();
  while (not end()) {
    ElementP key = read_form();
    if (key->type == SYMBOL and key->to<Symbol>()->value() == "}") {
#ifdef _LM_DEBUG
      writeln("ending dict");
#endif

      break;
    } else {
      ElementP value = read_form();
      if (value->type == SYMBOL and value->to<Symbol>()->value() == "}") {
        writeln("hashmap must be a set of pair key-value");
        exit(1);
      }
      if (key->type == TYPES::STRING) {
        ret_dict->to<Dict>()->append(key->to<String>(), value);
      } else if (key->type == TYPES::KEYWORD) {
        ret_dict->to<Dict>()->append(key->to<Keyword>(), value);
      } else {
        writeln("keys of an hashmap can be only strings and keywords");
        exit(1);
      }
    }
  }
  return ret_dict;
}

ElementP Reader::read_atom() {
#ifdef _LM_DEBUG
  writeln("atom " + peek());
#endif
  char c = peek().at(0);
  // ---------------------------------------------------------------------------
  //                             NUMBER
  // ---------------------------------------------------------------------------
  if ((c >= '0' and c <= '9') or c == '-' or c == '+') {
    int digit_number = 0;
    bool period = false, comma = false, is_number = true,
         minus_mantissa = false,
         at_least_one_digit = false; // maybe implement e
    for (char d : peek()) {
      if (std::isdigit(d)) {
        at_least_one_digit = true;
        continue;
      } else if (d == '.' and not period) {
        period = true;
        continue;
      } else if (d == ',' and not comma) {
        comma = true;
        continue;
      } else if (d == '-' and not minus_mantissa and digit_number == 0) {
        minus_mantissa = true;
        continue;
      } else {
        is_number = false;
        break;
      }
      digit_number++;
    }
    if (is_number and at_least_one_digit) {
#ifdef _LM_DEBUG
      writeln("number " + peek());
#endif
      return num(stoi(peek()));
      //      return num(stof(peek()));  // TODO maybe replace with stof() for float;
    } else {
      // -----------------------------------------------------------------------
      //                       NOT NUMBER BUT SYMBOL
      // -----------------------------------------------------------------------
#ifdef _LM_DEBUG
      writeln("symbol " + peek());
#endif
      return sym(peek());
    }
  } else if (c == '"') {
    // -------------------------------------------------------------------------
    //                             STRING
    // -------------------------------------------------------------------------

#ifdef _LM_DEBUG
    writeln("string " + peek());
#endif
    // replace escaped char with their corresponding value
    std::string ret = "", orig = peek().substr(1, peek().length() - 2);
    bool prev_escape = false;
    for (char c: orig) {
      if (c == '\\' and not prev_escape) {
	prev_escape = true;
	continue;
      }
      if (prev_escape) {
	prev_escape = false;
	switch (c) {
	case '\\':
	  break;
	case '"':
	  break;
	case 'n':
	  ret += '\n';
	  continue;
	case 't':
	  ret += '\t';
	  continue;
	default:
	  return exc("unkwown escape char: \\" + c);
	}
      }
      ret += c;
    }
    return str(ret);
  } else if (c == ':') {
    // -------------------------------------------------------------------------
    //                             KEYWORD
    // -------------------------------------------------------------------------
    return kw(peek().substr(1));
#ifdef _LM_DEBUG
    writeln("keyword " + peek());
#endif
  } else {
    // -------------------------------------------------------------------------
    //                             MACRO EXPANDS
    // -------------------------------------------------------------------------
    switch (c) {
    // deref
    case '@': {
      ElementP ret_defer = list();
      ret_defer->to<List>()->append(sym("deref"));
      ret_defer->to<List>()->append(read_form());
#ifdef _LM_DEBUG
      writeln("deref " + peek());
#endif
      return ret_defer;
    }
      // with-meta
    case '^': {
      ElementP ret_with_meta = list();
      ret_with_meta->to<List>()->append(sym("with-meta"));
      ElementP meta_dict = read_form();
      ret_with_meta->to<List>()->append(read_form());
      ret_with_meta->to<List>()->append(meta_dict);
#ifdef _LM_DEBUG
      writeln("with-meta" + pr_str(ret_with_meta));
#endif
      return ret_with_meta;
    }
      // quote
    case '\'': {
      ElementP ret_quote = list();
      ret_quote->to<List>()->append(sym("quote"));
      ret_quote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
      writeln("quote " + peek());
#endif
      return ret_quote;
    }
      // quasiquote
    case '`': {
      ElementP ret_quasiquote = list();
      ret_quasiquote->to<List>()->append(sym("quasiquote"));
      ret_quasiquote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
      writeln("quasiquote " + peek());
#endif
      return ret_quasiquote;
    }
    case '~': {
      if (peek().length() == 1) {
        // unquote
        ElementP ret_unquote = list();
        ret_unquote->to<List>()->append(sym("unquote"));
        ret_unquote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
        writeln("unquote " + peek());
#endif
        return ret_unquote;
      } else if (peek().at(1) == '@') {
        // splice-unquote
        ElementP ret_splice_unquote = list();
        ret_splice_unquote->to<List>()->append(sym("splice-unquote"));
        ret_splice_unquote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
        writeln("splice-unquote " + peek());
#endif
        return ret_splice_unquote;
      } else {
        exit(1);
      }
    }
      // normal symbol. no macro
    default:
      // -----------------------------------------------------------------------
      //                             SYMBOL
      // -----------------------------------------------------------------------
#ifdef _LM_DEBUG
      writeln("symbol " + peek());
#endif
      if (peek() == "nil") return nil();
      else return sym(peek());
    }
  }
}

} // namespace lmlisp
// namespace lmlisp
