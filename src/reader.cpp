#include "reader.hpp"
#include "types.hpp"
// #include <cctype>
// #include <cstdlib>

// #define LM_DEBUG
#ifdef LM_DEBUG
// #define _LM_DEBUG
#define _LM_DEBUG_TK
#include <iostream>
using std::cout, std::endl;

#endif

namespace lmlisp {
Reader::Reader(std::function<void(std::string)> printer) : printer{printer} {
  this->pos = -1;
};

bool Reader::end() const { return pos == tokens.size(); }

std::string Reader::next() {
  if (pos > static_cast<int>(tokens.size())) {
    printer(
        "Going over list of tokens bound. Maybe an unclosed list or string");
    abort();
  }
  std::string tok = tokens[++pos];
  return tok;
}

std::string Reader::peek() { return tokens[pos]; }

void Reader::read_str(const std::string &input) { tokens = tokenize(input); }

Reader::Brackets_Checker::Brackets_Checker(
    std::function<void(std::string)> printer)
    : printer{printer} {}

void Reader::Brackets_Checker::close() { react_error(close_stack()); }

int Reader::Brackets_Checker::close_stack() {
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

void Reader::Brackets_Checker::react_error(int error) {
  switch (error) {
  case NOT_A_BRACKET:
    printer("Somehow not a bracket ends here");
    break;
  case UNMATCHED_CURVE:
    printer("Unmatched )");
    break;
  case UNMATCHED_SQUARED:
    printer("Unmatched ]");
    break;
  case UNMATCHED_BRACKET:
    printer("Unmatched }");
    break;
  case UNMATCHED_DOUBLEQUOTE:
    printer("Unmatched \"");
    break;
  case UNBALANCED_CURVE:
    printer("Unbalanced (");
    break;
  case UNBALANCED_SQUARED:
    printer("Unbalanced [");
    break;
  case UNBALANCED_BRACKET:
    printer("Unbalanced {");
    break;
  case UNBALANCED_DOUBLEQUOTE:
    printer("Unbalanced \"");
    break;
  case NO_ERROR:
    return;
  }
  exit(1);
}

void Reader::Brackets_Checker::push_bracket(char br) {
  react_error(check_stack(br));
}

int Reader::Brackets_Checker::check_stack(char br) {
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

std::vector<std::string> Reader::tokenize(std::string input) {
  Brackets_Checker bc(printer);
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
        bc.push_bracket('"');
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
        bc.push_bracket('"');
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
        bc.push_bracket('(');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case ')':
        bc.push_bracket(')');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '[':
        bc.push_bracket('[');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case ']':
        bc.push_bracket(']');
        if (offset != 0)
          add_token(input.substr(index, offset));
        add_token(input.substr(index + offset, 1));
        index += offset + 1;
        offset = 0;
        break;
      case '{':
        bc.push_bracket('{');
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
        bc.push_bracket('}');
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
    printer(input);
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
      printer("residue: " + input.substr(index, offset));
    add_token(input.substr(index, offset));
  }
  bc.close();
  return tokens;
}

ElementP Reader::read_form() {
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
  printer("starting list");
#endif
  while (not end()) {
    ElementP el = read_form();
    if (el->type == SYMBOL and el->to<Symbol>()->value() == ")") {
#ifdef _LM_DEBUG
      printer("ending list");
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
  printer("starting vec");
#endif
  ElementP ret_vec = vec();
  while (not end()) {
    ElementP el = read_form();
    if (el->type == SYMBOL and el->to<Symbol>()->value() == "]") {
#ifdef _LM_DEBUG
      printer("ending vec");
#endif
      break;
    }
    ret_vec->to<Vec>()->append(el);
  }
  return ret_vec;
}

ElementP Reader::read_dict() {
#ifdef _LM_DEBUG
  printer("starting dict");
#endif
  ElementP ret_dict = dict();
  while (not end()) {
    ElementP key = read_form();
    if (key->type == SYMBOL and key->to<Symbol>()->value() == "}") {
#ifdef _LM_DEBUG
      printer("ending dict");
#endif

      break;
    } else {
      ElementP value = read_form();
      if (value->type == SYMBOL and value->to<Symbol>()->value() == "}") {
        printer("hashmap must be a set of pair key-value");
        exit(1);
      }
      if (key->type == TYPES::STRING) {
        ret_dict->to<Dict>()->append(key->to<String>(), value);
      } else if (key->type == TYPES::KEYWORD) {
        ret_dict->to<Dict>()->append(key->to<Keyword>(), value);
      } else {
        printer("keys of an hashmap can be only strings and keywords");
        exit(1);
      }
    }
  }
  return ret_dict;
}

ElementP Reader::read_atom() {
#ifdef _LM_DEBUG
  printer("atom " + peek());
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
      printer("number " + peek());
#endif
      return num(stoi(peek()));
      //      return num(stof(peek()));  // TODO maybe replace with stof() for float;
    } else {
      // -----------------------------------------------------------------------
      //                       NOT NUMBER BUT SYMBOL
      // -----------------------------------------------------------------------
#ifdef _LM_DEBUG
      printer("symbol " + peek());
#endif
      return sym(peek());
    }
  } else if (c == '"') {
    // -------------------------------------------------------------------------
    //                             STRING
    // -------------------------------------------------------------------------

#ifdef _LM_DEBUG
    printer("string " + peek());
#endif
    return str(peek().substr(1, peek().length() - 2));
  } else if (c == ':') {
    // -------------------------------------------------------------------------
    //                             KEYWORD
    // -------------------------------------------------------------------------
    return kw(peek().substr(1));
#ifdef _LM_DEBUG
    printer("keyword " + peek());
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
      printer("deref " + peek());
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
      printer("with-meta" + pr_str(ret_with_meta));
#endif
      return ret_with_meta;
    }
      // quote
    case '\'': {
      ElementP ret_quote = list();
      ret_quote->to<List>()->append(sym("quote"));
      ret_quote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
      printer("quote " + peek());
#endif
      return ret_quote;
    }
      // quasiquote
    case '`': {
      ElementP ret_quasiquote = list();
      ret_quasiquote->to<List>()->append(sym("quasiquote"));
      ret_quasiquote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
      printer("quasiquote " + peek());
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
        printer("unquote " + peek());
#endif
        return ret_unquote;
      } else if (peek().at(1) == '@') {
        // splice-unquote
        ElementP ret_splice_unquote = list();
        ret_splice_unquote->to<List>()->append(sym("splice-unquote"));
        ret_splice_unquote->to<List>()->append(read_form());
#ifdef _LM_DEBUG
        printer("splice-unquote " + peek());
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
      printer("symbol " + peek());
#endif
      return sym(peek());
    }
  }
}

} // namespace lmlisp
// namespace lmlisp
