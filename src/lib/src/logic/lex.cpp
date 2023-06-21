//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "black/logic/lex.hpp"

#include <map>
#include <algorithm>
#include <istream>
#include <charconv>
#include <limits>

namespace black_internal::lexer_details
{
  using namespace black::logic::fragments::Everything;

  static
  std::string to_string(quantifier::type t) {
    return t.match(
      [](quantifier::type::exists) { return "exists"; },
      [](quantifier::type::forall) { return "forall"; }
    );
  }
  
  static
  std::string to_string(token::equality_t t) {
    return t.first.match(
      [&](equality::type::equal)    { return t.second ? "=" : "equal";  },
      [&](equality::type::distinct) { return t.second ? "!=" : "distinct"; }
    );
  }

  static
  std::string to_string(comparison::type t) {
    return t.match(
      [](comparison::type::less_than)          { return "<";  },
      [](comparison::type::less_than_equal)    { return "<="; },
      [](comparison::type::greater_than)       { return ">="; },
      [](comparison::type::greater_than_equal) { return ">="; }
    );
  }
  
  static
  std::string to_string(arithmetic_sort::type t) {
    return t.match(
      [](arithmetic_sort::type::integer_sort) { return "Int";  },
      [](arithmetic_sort::type::real_sort)    { return "Real"; }
    );
  }
  
  static
  std::string to_string(unary_term::type t) {
    return t.match(
      [](unary_term::type::negative)   { return "-";       }, // LCOV_EXCL_LINE
      [](unary_term::type::to_integer) { return "to_int";  }, // LCOV_EXCL_LINE
      [](unary_term::type::to_real)    { return "to_real"; }, // LCOV_EXCL_LINE
      [](unary_term::type::next)       { return "next";    },
      [](unary_term::type::wnext)      { return "wnext";   },
      [](unary_term::type::prev)       { return "prev";    },
      [](unary_term::type::wprev)      { return "wprev";   }
    );
  }
  
  static
  std::string to_string(binary_term::type t) {
    return t.match(
      [](binary_term::type::addition)       { return "+"; },
      [](binary_term::type::subtraction)    { return "-"; },
      [](binary_term::type::multiplication) { return "*"; },
      [](binary_term::type::division)       { return "/"; },
      [](binary_term::type::int_division)   { return "div"; }
    );
  }
  
  static
  std::string to_string(unary::type t) {
    return t.match(
      [](unary::type::negation)     { return "!";  },
      [](unary::type::tomorrow)     { return "X";  },
      [](unary::type::w_tomorrow)   { return "wX"; },
      [](unary::type::yesterday)    { return "Y";  },
      [](unary::type::w_yesterday)  { return "Z";  },
      [](unary::type::always)       { return "G";  },
      [](unary::type::eventually)   { return "F";  },
      [](unary::type::once)         { return "O";  },
      [](unary::type::historically) { return "H";  }
    );
  }
  
  static
  std::string to_string(binary::type t) {
    return t.match(
      [](binary::type::conjunction) { return "&&";  },
      [](binary::type::disjunction) { return "||";  },
      [](binary::type::implication) { return "->";  },
      [](binary::type::iff)         { return "<->"; },
      [](binary::type::until)       { return "U";   },
      [](binary::type::release)     { return "R";   },
      [](binary::type::w_until)     { return "W";   },
      [](binary::type::s_release)   { return "M";   },
      [](binary::type::since)       { return "S";   },
      [](binary::type::triggered)   { return "T";   }
    );
  }
  
  static
  std::string to_string(token::hs_op_t t) {
    std::string op = t.first.match(
      [](interval_op::type::after) { return "L"; },
      [](interval_op::type::before) { return "!L"; },
      [](interval_op::type::meets) { return "A"; },
      [](interval_op::type::metby) { return "!A"; },
      [](interval_op::type::overlaps) { return "O"; },
      [](interval_op::type::overlappedby) { return "!O"; },
      [](interval_op::type::begins) { return "B"; },
      [](interval_op::type::beganby) { return "!B"; },
      [](interval_op::type::during) { return "D"; },
      [](interval_op::type::contains) { return "!D"; },
      [](interval_op::type::ends) { return "E"; },
      [](interval_op::type::endedby) { return "!E"; }
    );

    if(t.second)
      return "<" + op + ">";
    return "[" + op + "]";
  }
  
  std::string to_string(token::punctuation p) {
    switch(p) {
      case token::punctuation::left_paren:  return "(";
      case token::punctuation::right_paren: return ")";
      case token::punctuation::comma:       return ",";
      case token::punctuation::dot:         return ".";
      case token::punctuation::colon:       return ":";
    }
    black_unreachable(); // LCOV_EXCL_LINE
  }

  std::string to_string(token const &tok)
  {
    using namespace std::literals;

    std::string stok = std::visit( overloaded { // LCOV_EXCL_LINE
      [](std::monostate)          { return "<invalid>"s; },
      [](bool b)                  { return b ? "True"s : "False"s; },
      [](int64_t c)               { return std::to_string(c); },
      [](double d)                { return std::to_string(d); },
      [](std::string s)           { return s; },
      [](quantifier::type k)      { return to_string(k); },
      [](token::equality_t t)     { return to_string(t); },
      [](comparison::type t)      { return to_string(t); },
      [](arithmetic_sort::type t) { return to_string(t); },
      [](unary_term::type t)      { return to_string(t); },
      [](binary_term::type t)     { return to_string(t); },
      [](unary::type t)           { return to_string(t); },
      [](binary::type t)          { return to_string(t); },
      [](token::hs_op_t t)        { return to_string(t); },
      [](token::punctuation p)    { return to_string(p); }
    }, tok._data);

    return stok;
  }

  namespace {

    std::optional<token> digits(std::istream &s, lexer::error_handler error)
    {
      std::string integer;

      // integer part
      while(isdigit(s.peek())) {
        integer += char(s.peek());
        s.get();
      }
      if(integer.empty())
        return {};

      if(s.peek() != '.') {
        int64_t val = 0;
        auto [ptr, err] = 
          std::from_chars(integer.data(), integer.data() + integer.size(), val);

        if(err == std::errc())
          return token{val};

        error(
          "Integer constant '" + integer + "' is too big. " +
          "Admitted range: [" + 
          std::to_string(std::numeric_limits<int64_t>::min()) + ", " +
          std::to_string(std::numeric_limits<int64_t>::max()) + "]"
        );
        return token{};
      }

      // fractional part
      std::string fractional;
      
      s.get(); // consume the dot
      while(s.good() && isdigit(s.peek())) {
        fractional += char(s.peek());
        s.get();
      }
      if(fractional.empty()) {
        error("Incomplete fractional constant: '" + integer + ".'");
        return token{};
      }
      
      std::string number = integer + '.' + fractional;
      try {
        return token{stod(number)};
      } catch (std::out_of_range const&) {
        error(
          "Fractional constant '" + number + "' cannot be represented as a "
          "64-bit floating-point value"
        );
        return token{};
      }
    }

    std::optional<token> symbol(std::istream &s)
    {
      char ch = char(s.peek());

      bool existential = false;
      bool equal = false;
      switch (ch) {
        case '(':
          s.get();
          return token{token::punctuation::left_paren};
        case ')':
          s.get();
          return token{token::punctuation::right_paren};
        case ',':
          s.get();
          return token{token::punctuation::comma};
        case '.':
          s.get();
          return token{token::punctuation::dot};
        case ':':
          s.get();
          return token{token::punctuation::colon};
        case '!':
          s.get();
          if(s.peek() == '=') {
            s.get();
            return token{{equality::type::distinct{}, true}};
          }
          return token{unary::type::negation{}};
        case '~':
          s.get();
          return token{unary::type::negation{}};
        // '&' or '&&'
        case '&':
          s.get();
          if (s.peek() == '&')
            s.get();
          return token{binary::type::conjunction{}};

        // '|' or '||'
        case '|':
          s.get();
          if (s.peek() == '|')
            s.get();
          return token{binary::type::disjunction{}};

        // '->' and '=>'
        case '-':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{binary::type::implication{}};
          }
          return token{binary_term::type::subtraction{}};
        case '=':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{binary::type::implication{}};
          }
          return token{{equality::type::equal{}, true}};
        
        case '>':
          s.get();
          if(s.peek() == '=') {
            s.get();
            return token{comparison::type::greater_than_equal{}};
          }
          return token{comparison::type::greater_than{}};

        // '<->' or '<=>' or '<>' or HS operator
        case '<': 
          existential = true;
          s.get();
          if (s.peek() == '-' || s.peek() == '=') {
            if(s.peek() == '=')
              equal = true;
            s.get();
            if (s.peek() == '>') {
              s.get();
              return token{binary::type::iff{}};
            }
          }
                   
          [[fallthrough]];
        
        case '[': {
          if(!existential)
            s.get();
          bool negated = false;
          if(s.peek() == '!') {
            negated = true;
            s.get();
          }
          
          std::optional<interval_op::type> op;
          switch(s.peek()) {
            case 'L':
              if(!negated)
                op = interval_op::type::after{};
              else
                op = interval_op::type::before{};
              break;
            case 'A':
              if(!negated)
                op = interval_op::type::meets{};
              else
                op = interval_op::type::metby{};
              break;
            case 'O':
              if(!negated)
                op = interval_op::type::overlaps{};
              else
                op = interval_op::type::overlappedby{};
              break;
            case 'B':
              if(!negated)
                op = interval_op::type::begins{};
              else
                op = interval_op::type::beganby{};
              break;
            case 'D':
              if(!negated)
                op = interval_op::type::during{};
              else
                op = interval_op::type::contains{};
              break;
            case 'E':
              if(!negated)
                op = interval_op::type::ends{};
              else
                op = interval_op::type::endedby{};
              break;
            default:
              if(!negated) {
                if(equal)
                  return token{comparison::type::less_than_equal{}};
                return token{comparison::type::less_than{}};
              }
              return std::nullopt;
          }

          s.get();
          if((existential && s.peek() != '>') || 
             (!existential && s.peek() != ']'))
            return std::nullopt;
          s.get();
          return token(std::pair(*op, existential));
        }

        case '+':
          s.get();
          return token{binary_term::type::addition{}};
        case '*':
          s.get();
          return token{binary_term::type::multiplication{}};
        case '/':
          s.get();
          return token{binary_term::type::division{}};
      }

      // TODO: garantire che se restituiamo nullopt, lo stream non è avanzato
      return std::nullopt;
    }
  }  // namespace

  bool lexer::is_identifier_char(int c) {
    return isalnum(c) || c == '_';
  }

  bool lexer::is_initial_identifier_char(int c) {
    return isalpha(c) || c == '_' || c == '{';
  }

  std::pair<std::string_view, token> lexer::_keywords[35] = {
    {"True",     token{true}},
    {"False",    token{false}},
    {"Int",      token{arithmetic_sort::type::integer_sort{}}},
    {"Real",     token{arithmetic_sort::type::real_sort{}}},
    {"to_int",   token{unary_term::type::to_integer{}}},
    {"to_real",  token{unary_term::type::to_real{}}},
    {"next",     token{unary_term::type::next{}}},
    {"wnext",    token{unary_term::type::wnext{}}},
    {"prev",     token{unary_term::type::prev{}}},
    {"wprev",    token{unary_term::type::wprev{}}},
    {"div",      token{binary_term::type::int_division{}}},
    {"exists",   token{quantifier::type::exists{}}},
    {"forall",   token{quantifier::type::forall{}}},
    {"equal",    token{{equality::type::equal{}, false}}},
    {"distinct", token{{equality::type::distinct{}, false}}},
    {"NOT",      token{unary::type::negation{}}},
    {"X",        token{unary::type::tomorrow{}}},
    {"wX",       token{unary::type::w_tomorrow{}}},
    {"Y",        token{unary::type::yesterday{}}},
    {"Z",        token{unary::type::w_yesterday{}}},
    {"F",        token{unary::type::eventually{}}},
    {"G",        token{unary::type::always{}}},
    {"O",        token{unary::type::once{}}},
    {"H",        token{unary::type::historically{}}},
    {"AND",      token{binary::type::conjunction{}}},
    {"OR",       token{binary::type::disjunction{}}},
    {"THEN",     token{binary::type::implication{}}},
    {"IFF",      token{binary::type::iff{}}},
    {"U",        token{binary::type::until{}}},
    {"R",        token{binary::type::release{}}},
    {"V",        token{binary::type::release{}}},
    {"W",        token{binary::type::w_until{}}},
    {"M",        token{binary::type::s_release{}}},
    {"S",        token{binary::type::since{}}},
    {"T",        token{binary::type::triggered{}}}
  };

  bool lexer::is_keyword(std::string_view s) {
    return
      std::find_if(std::begin(_keywords), std::end(_keywords), [&](auto p) {
        return p.first == s;
      }) != std::end(_keywords);
  }

  std::optional<token> lexer::_identifier()
  {
    if (!_stream.good() || !is_initial_identifier_char(_stream.peek())) {
      _error(
        std::string{"Unrecognized input character: '"} + 
        (char)_stream.peek() + "'"
      );
      return token{};
    }

    if((char)_stream.peek() == '{')
      return _raw_identifier();

    std::string id;
    while (_stream.good() && is_identifier_char(_stream.peek())) 
    {
      id += char(_stream.peek());
      _stream.get();
    }
    black_assert(!id.empty());

    auto it = 
      std::find_if(std::begin(_keywords), std::end(_keywords), [&](auto p) {
        return p.first == id;
      });

    if(it != std::end(_keywords))
      return {it->second};

    return token{std::move(id)};
  }

  std::optional<token> lexer::_raw_identifier()
  {
    using namespace std::literals;

    black_assert(_stream.peek() == '{');

    _stream.get();
    std::string id;
    
    while(_stream.peek() != '}') {
      char c = (char)_stream.peek();
      
      if(c == '\\') {
        _stream.get();
        char c2 = (char)_stream.peek(); 
        if(c2 == '}') {
          c = '}';
        } else if(c2 == '\\') {
          c = '\\';
        } else {
          _error(
            "Unknown escape sequence '\\"s + c2 + "' in raw identifier"
          );
          return {};
        }
      }      
      
      _stream.get();
      id += c;
    }
    _stream.get();

    return token{std::move(id)};
  }

  std::optional<token> lexer::_lex()
  {
    char c = (char)_stream.peek();
    while (_stream.good() && isspace(c)) {
      _stream.get();
      c = (char)_stream.peek();
    }

    if (!_stream.good())
      return std::nullopt;

    if(std::optional<token> t = digits(_stream, _error); t)
      return t;

    if(std::optional<token> t = symbol(_stream); t)
      return t;

    return _identifier();
  }

}  // namespace black_internal
