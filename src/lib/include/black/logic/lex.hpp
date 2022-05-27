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

#ifndef BLACK_LEX_H_
#define BLACK_LEX_H_

#include <black/support/common.hpp>
#include <black/logic/formula.hpp>

#include <cassert>
#include <cctype>

#include <optional>
#include <variant>
#include <vector>

namespace black::internal
{
  // Type representing tokens generated from the lexer.
  struct token
  {
    enum class type : uint8_t {
      invalid = 0,
      boolean,
      integer,
      real,
      identifier,
      keyword,
      constructor,
      relation,
      function,
      unary_operator,
      binary_operator,
      punctuation
    };

    // Type of non-logical tokens. Only parens, for now.
    enum class punctuation : uint8_t {
      // non-logical tokens
      left_paren,
      right_paren,
      comma,
      dot
    };

    // we only have one keyword yet
    enum class keyword : uint8_t {
      exists,
      forall
    };

             token()                     : _data{std::monostate{}} { }
    explicit token(bool b)               : _data{b} { }
    explicit token(int64_t c)            : _data{c} { }
    explicit token(double d)             : _data{d} { }
    explicit token(std::string s)        : _data{std::move(s)} { }
    explicit token(keyword k)            : _data{k} { }
    explicit token(constructor::type t)  : _data{t} { }
    explicit token(relation::type t)     : _data{t} { }
    explicit token(function::type t)     : _data{t} { }
    explicit token(unary::type t)        : _data{t} { }
    explicit token(binary::type t)       : _data{t} { }
    explicit token(punctuation s) : _data{s} { }

    template<typename T>
    bool is() const {
      return std::holds_alternative<T>(_data);
    }

    template<typename T>
    std::optional<T> data() const {
      if(auto p = std::get_if<T>(&_data); p)
        return {*p};
      return std::nullopt;
    }

    type token_type() const { return static_cast<type>(_data.index()); }

    friend std::string to_string(token const &tok);

  private:
    // data related to recognized tokens
    std::variant<
      std::monostate,    // invalid tokens
      bool,              // booleans
      int64_t,           // integers
      double,            // reals
      std::string,       // identifiers
      keyword,           // keywords
      constructor::type, // next/prev/wnext/wprev
      relation::type,    // known relations
      function::type,    // known functions
      unary::type,       // unary operator
      binary::type,      // binary operator
      punctuation        // any non-logical token
    > _data;
  };

  inline std::string to_string(constructor::type t)
  {
    constexpr std::string_view toks[] = {
      "next",  // next
      "wnext", // wnext
      "prev",  // prev
      "wprev"  // wprev
    };

    return std::string{
      toks[to_underlying(t) - to_underlying(constructor::type::next)]
    };
  }

  inline std::string to_string(unary::type t)
  {
    constexpr std::string_view toks[] = {
      "!",  // negation
      "X",  // tomorrow
      "wX", // weak tomorrow
      "Y",  // yesterday
      "Z",  // weak yesterday
      "G",  // always
      "F",  // eventually
      "O",  // once
      "H",  // historically
    };

    return std::string{
      toks[to_underlying(t) - to_underlying(unary::type::negation)]
    };
  }

  inline std::string to_string(binary::type t) {
    constexpr std::string_view toks[] = {
      "&",   // conjunction
      "|",   // disjunction
      "->",  // implication
      "<->", // iff
      "U",   // until
      "R",   // release
      "W",   // weak until
      "M",   // strong release
      "S",   // since
      "T",   // triggered
    };

    return std::string{
      toks[to_underlying(t) - to_underlying(binary::type::conjunction)]
    };
  }

  inline std::string to_string(token::keyword k) {
    constexpr std::string_view toks[] = {
      "next",
      "wnext",
      "exists",
      "forall"
    };

    return std::string{toks[to_underlying(k)]};
  }

  inline std::string to_string(relation::type t) {
    constexpr std::string_view toks[] = {
      "=",  // equal
      "!=", // not_equal
      "<",  // less_than
      "<=", // less_than_equal
      ">",  // greater_than
      ">="  // greater_than_equal
    };

    return std::string{toks[to_underlying(t)]};
  }

  inline std::string to_string(function::type t) {
    constexpr std::string_view toks[] = {
      "-",   // negation
      "-",   // subtraction
      "+",   // addition
      "*",   // multiplication
      "/",   // division
    };

    return std::string{toks[to_underlying(t)]};
  }

  inline std::string to_string(token::punctuation p) {
    constexpr std::string_view toks[] = {
      "(", // left_paren
      ")", // right_paren
      ",", // comma
      "."  // dot
    };

    return std::string{toks[to_underlying(p)]};
  }

  inline std::string to_string(token const &tok)
  {
    using namespace std::literals;

    return std::visit( overloaded {
      [](std::monostate)       { return "<invalid>"s; },
      [](bool b)               { return b ? "true"s : "false"s; },
      [](int64_t c)            { return std::to_string(c); },
      [](double d)             { return std::to_string(d); },
      [](std::string s)        { return s; },
      [](token::keyword k)     { return to_string(k); },
      [](constructor::type t)  { return to_string(t); },
      [](relation::type t)     { return to_string(t); },
      [](function::type t)     { return to_string(t); },
      [](unary::type t)        { return to_string(t); },
      [](binary::type t)       { return to_string(t); },
      [](token::punctuation p) { return to_string(p); }
    }, tok._data);
  }

  class BLACK_EXPORT lexer
  {
  public:
    using error_handler = std::function<void(std::string)>;

    explicit lexer(std::istream &stream, error_handler error) 
      : _stream(stream), _error{error} {}  
    
    std::optional<token> get() { return _token = _lex(); }
    std::optional<token> peek() const { return _token; }

    static bool is_identifier_char(int c);
    static bool is_initial_identifier_char(int c);

  private:
    std::optional<token> _lex();
    std::optional<token> _identifier();
    std::optional<token> _raw_identifier();

    std::optional<token> _token = std::nullopt;
    std::istream &_stream;
    error_handler _error;
  };

  std::ostream &operator<<(std::ostream &s, token const &t);
}

#endif // LEX_H_
