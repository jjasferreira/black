//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <catch.hpp>

#include <black/logic/logic.hpp>

#include <string>
#include <type_traits>
#include <ranges>

using namespace std::literals;
using namespace black::logic;
using black_internal::identifier;

static_assert(black::logic::hierarchy<formula<LTL>>);
static_assert(black::logic::hierarchy<proposition>);
static_assert(black::logic::hierarchy<unary<LTL>>);
static_assert(black::logic::hierarchy<conjunction<LTL>>);
static_assert(black::logic::hierarchy<equal<FO>>);
static_assert(black::logic::storage_kind<proposition>);
static_assert(black::logic::storage_kind<unary<LTL>>);
static_assert(black::logic::storage_kind<conjunction<LTL>>);
static_assert(black::logic::hierarchy_element<conjunction<LTL>>);
static_assert(black::logic::hierarchy_element<equal<FO>>);

static_assert(
  std::is_same_v<black::logic::make_combined_fragment_t<LTL>, LTL>
);


TEST_CASE("New API") {

  black::logic::alphabet sigma;

  SECTION("Formula deduplication") {
    REQUIRE(sigma.boolean(true) == sigma.boolean(true));

    REQUIRE(sigma.proposition("p") == sigma.proposition("p"));

    formula<LTL> c = conjunction<LTL>(sigma.boolean(true), sigma.boolean(true));
    formula<LTL> d = conjunction<LTL>(sigma.boolean(true), sigma.boolean(true));

    REQUIRE(c == d);
  }

  SECTION("Leaf storage kinds") {
    static_assert(!std::is_constructible_v<formula<LTL>, variable>);
    static_assert(!std::is_assignable_v<formula<LTL>, variable>);

    boolean b = sigma.boolean(true);
    REQUIRE(b.value() == true);

    proposition p = sigma.proposition("p");
    REQUIRE(p.name() == "p");

    variable x = sigma.variable("x");
    REQUIRE(x.name() == "x");

    formula<LTL> f = p;

    REQUIRE(f.is<proposition>());
    REQUIRE(!f.is<boolean>());

    f = b;

    REQUIRE(f.is<boolean>());
    REQUIRE(!f.is<proposition>());
  }

  SECTION("Storage kinds") {
    boolean b = sigma.boolean(true);

    unary<LTL> u = unary<LTL>(unary<LTL>::type::eventually{}, b);

    REQUIRE(u.argument() == b);

    REQUIRE(u.argument().is<boolean>());
    REQUIRE(u.argument().to<boolean>()->value() == true);

    REQUIRE(u.is<eventually<LTL>>());
    REQUIRE(u.to<eventually<LTL>>()->argument() == b);
    REQUIRE(u.to<eventually<LTL>>()->argument().is<boolean>());
    REQUIRE(u.to<eventually<LTL>>()->argument().to<boolean>()->value() == true);
  }

  SECTION("Hierarchy elements") {
    static_assert(!std::is_constructible_v<unary<LTL>, conjunction<LTL>>);
    static_assert(!std::is_assignable_v<unary<LTL>, conjunction<LTL>>);

    boolean b = sigma.boolean(true);

    negation<LTL> n = negation<LTL>(b);
    always<LTL> a = always<LTL>(b);

    REQUIRE(n.argument() == b);
    REQUIRE(n.argument().is<boolean>());
    REQUIRE(n.argument().to<boolean>()->value() == true);

    REQUIRE(a.argument() == b);
    REQUIRE(a.argument().is<boolean>());
    REQUIRE(a.argument().to<boolean>()->value() == true);

    unary<LTL> u = n;
    REQUIRE(u == n);

    u = a;
    REQUIRE(u == a);    
  }

  SECTION("Use of fragment type enums values") {
    unary<LTL>::type t1 = unary<LTL>::type::always{};
    unary<LTL>::type t2 = unary<LTL>::type::negation{};

    REQUIRE(t1 != t2);

    std::vector<unary<LTL>::type> v = {
      unary<LTL>::type::always{}, unary<LTL>::type::negation{}, 
      unary<LTL>::type::eventually{}
    };

    REQUIRE(v[0] == t1);
  }

  SECTION("Conversions between different syntaxes") {
    static_assert(!std::is_constructible_v<unary<FO>, until<LTL>>);
    static_assert(!std::is_assignable_v<unary<FO>, until<LTL>>);
    static_assert(!std::is_constructible_v<unary<LTL>, conjunction<LTLP>>);
    static_assert(!std::is_assignable_v<unary<LTL>, conjunction<LTLP>>);

    boolean b = sigma.boolean(true);
    
    SECTION("Storage kinds") {
      unary<LTL> n = unary<LTL>(unary<LTL>::type::negation{}, b);
      unary<LTL> a = unary<LTL>(unary<LTL>::type::always{}, b);
    
      formula<LTLP> f = n;
      REQUIRE(f == n);

      f = a;
      REQUIRE(f == a);
    }

    SECTION("Hierarchy elements") {
      negation<LTL> n = negation<LTL>(b);
      always<LTLP> a = always<LTL>(b);

      formula<LTLP> f = a;
      REQUIRE(f == a);

      f = n;
      REQUIRE(f == n);
    }

    SECTION("Storage kinds and hierarchy elements") {
      negation<LTL> n = negation<propositional>(b);
      always<LTLP> a = always<LTL>(b);

      unary<LTLP> u = n;
      REQUIRE(u == n);

      u = a;
      REQUIRE(u == a);
    }

    SECTION("is<>, to<> and from<>") {
      negation<LTL> n = negation<LTL>(sigma.boolean(true));

      REQUIRE(n.is<negation<LTL>>());
      REQUIRE(n.is<negation<LTLP>>());
      REQUIRE(n.is<negation<propositional>>());
      REQUIRE(!n.is<unary_term<FO>>());

      proposition p = sigma.proposition("p");
      formula<LTLP> u1 = G(p);
      formula<LTLP> u2 = Y(sigma.proposition("p"));
      formula<LTLP> u3 = G(Y(sigma.proposition("p")));

      auto x = sigma.variable("x");
      auto y = sigma.variable("y");
      auto z = sigma.variable("z");

      std::vector<variable> vars = {x, y, z};
      std::vector<term<FO>> sums = {x + x, y + y, z + z};

      auto e1 = equal(vars);
      auto e2 = equal(sums);

      using vars_fragment = decltype(e1)::syntax;

      REQUIRE(u1.to<formula<LTL>>().has_value());
      REQUIRE(!u2.to<formula<LTL>>().has_value());
      REQUIRE(!u3.to<formula<LTL>>().has_value());
      
      REQUIRE(!e2.to<equal<vars_fragment>>().has_value());

      REQUIRE(u1.node_type().to<formula<LTL>::type>().has_value());
      REQUIRE(!u2.node_type().to<formula<LTL>::type>().has_value()); 
    }
  }

  SECTION("Tuple-like access") {
    auto p = sigma.proposition("p");
    auto q = sigma.proposition("q");

    auto c = p && q;

    auto [l, r] = c;

    REQUIRE(l == p);
    REQUIRE(r == q);

    REQUIRE(get<0>(c) == p);
    REQUIRE(get<1>(c) == q);

    STATIC_REQUIRE(std::tuple_size_v<decltype(c)> == 2);
  }

  SECTION("Atoms and applications") {
    function f = sigma.function("f");

    REQUIRE(f.is<function>());

    variable x = sigma.variable("x");
    variable y = sigma.variable("y");
    std::vector<term<FO>> variables = {x,y};

    application<FO> app = application<FO>(f, variables);
    REQUIRE(app.func() == f);
    REQUIRE(variables == app.terms());

    application<FO> app2 = f(x,y);

    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term<FO>>{x,y});

    application<FO> app3 = f(std::vector{x, y});

    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term<FO>>{x,y});

    sort s = sigma.integer_sort();
    application<FO> app4 = f(x[s], y[s]);

    std::vector<var_decl> decls = {x[s], y[s]};
    application<FO> app5 = f(decls);

    REQUIRE(bool(app == app2));
    REQUIRE(bool(app == app3));
    REQUIRE(bool(app == app4));
    REQUIRE(bool(app == app5));

    application<FO> app6 = f(1, 2, 3);

    REQUIRE(app6.terms()[0] == 1);
    REQUIRE(app6.terms()[1] == 2);
    REQUIRE(app6.terms()[2] == 3);
  }

  SECTION("Quantifiers") {
    variable x = sigma.variable("x");
    sort s = sigma.integer_sort();
    comparison<FO> e = 
      comparison<FO>(comparison<FO>::type::greater_than{}, x, x);
    quantifier<FO> f = 
      quantifier<FO>(quantifier<FO>::type::forall{}, {x[s]}, e);

    REQUIRE(e.left() == x);
    REQUIRE(e.right() == x);

    REQUIRE(bool(f.variables() == std::vector{x[s]}));
    REQUIRE(f.matrix() == e);
  }
  
  SECTION("Deduction guides") {
    formula b = sigma.boolean(true);
    formula p = sigma.proposition("p");
    variable x = sigma.variable("x");
    unary u = unary<propositional>(unary<propositional>::type::negation{}, b);
    conjunction c = conjunction(p, b);
    binary c2 = conjunction(u, b);
    equal eq = equal(std::vector<term<FO>>{x, x});

    static_assert(std::is_same_v<decltype(c2), binary<propositional>>);

    REQUIRE(b.is<boolean>());
    REQUIRE(p.is<proposition>());
    REQUIRE(u.is<negation<propositional>>());
    REQUIRE(c.is<conjunction<propositional>>());
    REQUIRE(c2.is<conjunction<propositional>>());
    REQUIRE(eq.is<equal<FO>>());
  }

  SECTION("Sugar for formulas") {
    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");

    negation n = !b;
    tomorrow x = X(b);
    w_tomorrow wx = w_tomorrow(b);
    yesterday y = Y(b);
    w_yesterday z = Z(b);
    always g = G(b);
    eventually f = F(b);
    once o = O(b);
    historically h = H(b);

    REQUIRE(n.is<negation<LTLP>>());
    REQUIRE(x.is<tomorrow<LTLP>>());
    REQUIRE(wx.is<w_tomorrow<LTLP>>());
    REQUIRE(y.is<yesterday<LTLP>>());
    REQUIRE(z.is<w_yesterday<LTLP>>());
    REQUIRE(g.is<always<LTLP>>());
    REQUIRE(f.is<eventually<LTLP>>());
    REQUIRE(o.is<once<LTLP>>());
    REQUIRE(h.is<historically<LTLP>>());

    conjunction c = b && p;
    disjunction d = b || p;
    implication i = implies(b, p);
    until u = U(b, p);
    release r = R(b, p);
    w_until wu = W(b, p);
    s_release sr = M(b, p);
    since s = S(b, p);
    triggered t = T(b, p);

    REQUIRE(c.is<conjunction<LTLP>>());
    REQUIRE(d.is<disjunction<LTLP>>());
    REQUIRE(i.is<implication<LTLP>>());
    REQUIRE(u.is<until<LTLP>>());
    REQUIRE(r.is<release<LTLP>>());
    REQUIRE(wu.is<w_until<LTLP>>());
    REQUIRE(sr.is<s_release<LTLP>>());
    REQUIRE(s.is<since<LTLP>>());
    REQUIRE(t.is<triggered<LTLP>>());
  }

  SECTION("Sugar for terms") {
    variable x = sigma.variable("x");
    constant c = constant{sigma.integer(42)};

    comparison lt = x < c;
    comparison le = x <= c;
    comparison gt = x > c;
    comparison ge = x >= c;
    equality eq = x == c;
    equality ne = x != c;
    formula feq = x == c;
    formula fne = x != c;
    comparison lt0 = x < 0;
    comparison le0 = x <= 0;
    comparison gt0 = x > 0;
    comparison ge0 = x >= 0;
    equality eq0 = x == 0;
    equality ne0 = x != 0;
    formula feq0 = x == 0;
    formula fne0 = x != 0;
    comparison lt0p = 0 < x;
    comparison le0p = 0 <= x;
    comparison gt0p = 0 > x;
    comparison ge0p = 0 >= x;
    equality eq0p = 0 == x;
    equality ne0p = 0 != x;
    formula feq0p = 0 == x;
    formula fne0p = 0 != x;
    comparison lt0f = x < 1.0;
    comparison le0f = x <= 1.0;
    comparison gt0f = x > 1.0;
    comparison ge0f = x >= 1.0;
    equality eq0f = x == 1.0;
    equality ne0f = x != 1.0;
    formula feq0f = x == 0.0;
    formula fne0f = x != 0.0;
    comparison lt0fp = 0.0 < x;
    comparison le0fp = 0.0 <= x;
    comparison gt0fp = 0.0 > x;
    comparison ge0fp = 0.0 >= x;
    equality eq0fp = 0.0 == x;
    equality ne0fp = 0.0 != x;
    formula feq0fp = 0.0 == x;
    formula fne0fp = 0.0 != x;

    REQUIRE(lt.is<comparison<FO>>());
    REQUIRE(le.is<comparison<FO>>());
    REQUIRE(gt.is<comparison<FO>>());
    REQUIRE(ge.is<comparison<FO>>());
    REQUIRE(eq.is<equality<FO>>());
    REQUIRE(ne.is<equality<FO>>());
    REQUIRE(feq.is<equality<FO>>());
    REQUIRE(fne.is<equality<FO>>());
    REQUIRE(lt0.is<comparison<FO>>());
    REQUIRE(le0.is<comparison<FO>>());
    REQUIRE(gt0.is<comparison<FO>>());
    REQUIRE(ge0.is<comparison<FO>>());
    REQUIRE(eq0.is<equality<FO>>());
    REQUIRE(ne0.is<equality<FO>>());
    REQUIRE(feq0.is<equality<FO>>());
    REQUIRE(fne0.is<equality<FO>>());
    REQUIRE(lt0p.is<comparison<FO>>());
    REQUIRE(le0p.is<comparison<FO>>());
    REQUIRE(gt0p.is<comparison<FO>>());
    REQUIRE(ge0p.is<comparison<FO>>());
    REQUIRE(eq0p.is<equality<FO>>());
    REQUIRE(ne0p.is<equality<FO>>());
    REQUIRE(feq0p.is<equality<FO>>());
    REQUIRE(fne0p.is<equality<FO>>());
    REQUIRE(lt0f.is<comparison<FO>>());
    REQUIRE(le0f.is<comparison<FO>>());
    REQUIRE(gt0f.is<comparison<FO>>());
    REQUIRE(ge0f.is<comparison<FO>>());
    REQUIRE(eq0f.is<equality<FO>>());
    REQUIRE(ne0f.is<equality<FO>>());
    REQUIRE(feq0f.is<equality<FO>>());
    REQUIRE(fne0f.is<equality<FO>>());
    REQUIRE(lt0fp.is<comparison<FO>>());
    REQUIRE(le0fp.is<comparison<FO>>());
    REQUIRE(gt0fp.is<comparison<FO>>());
    REQUIRE(ge0fp.is<comparison<FO>>());
    REQUIRE(eq0fp.is<equality<FO>>());
    REQUIRE(ne0fp.is<equality<FO>>());
    REQUIRE(feq0fp.is<equality<FO>>());
    REQUIRE(fne0fp.is<equality<FO>>());

    REQUIRE((x == x && true) == true);
    REQUIRE((true && x == x) == true);
    REQUIRE((x == x || true) == true);
    REQUIRE((true || x == x) == true);


    binary_term plus = x + c;
    binary_term minus = x - c;
    unary_term neg = -x;
    binary_term mult = x * c;
    binary_term div = x / c;
    binary_term plus0 = x + 0;
    binary_term minus0 = x - 0;
    binary_term mult0 = x * 0;
    binary_term div0 = x / 0;
    binary_term plus0p = 0 + x;
    binary_term minus0p = 0 - x;
    binary_term mult0p = 0 * x;
    binary_term div0p = 0 / x;
    binary_term plus0f = x + 1.0;
    binary_term minus0f = x - 1.0;
    binary_term mult0f = x * 1.0;
    binary_term div0f = x / 1.0;
    binary_term plus0fp = 0.0 + x;
    binary_term minus0fp = 0.0 - x;
    binary_term mult0fp = 0.0 * x;
    binary_term div0fp = 0.0 / x;

    REQUIRE(plus.is<binary_term<FO>>());
    REQUIRE(minus.is<binary_term<FO>>());
    REQUIRE(neg.is<unary_term<FO>>());
    REQUIRE(mult.is<binary_term<FO>>());
    REQUIRE(div.is<binary_term<FO>>());
    REQUIRE(plus0.is<binary_term<FO>>());
    REQUIRE(minus0.is<binary_term<FO>>());
    REQUIRE(mult0.is<binary_term<FO>>());
    REQUIRE(div0.is<binary_term<FO>>());
    REQUIRE(plus0p.is<binary_term<FO>>());
    REQUIRE(minus0p.is<binary_term<FO>>());
    REQUIRE(mult0p.is<binary_term<FO>>());
    REQUIRE(div0p.is<binary_term<FO>>());
    REQUIRE(plus0f.is<binary_term<FO>>());
    REQUIRE(minus0f.is<binary_term<FO>>());
    REQUIRE(mult0f.is<binary_term<FO>>());
    REQUIRE(div0f.is<binary_term<FO>>());
    REQUIRE(plus0fp.is<binary_term<FO>>());
    REQUIRE(minus0fp.is<binary_term<FO>>());
    REQUIRE(mult0fp.is<binary_term<FO>>());
    REQUIRE(div0fp.is<binary_term<FO>>());
    
  }

  SECTION("Complex formula") {
    variable x = sigma.variable("x");
    function f = sigma.function("f");

    formula complex = (x == 0 && G(wnext(x) == f(x) + 1) && F(x == 42));

    REQUIRE(complex.is<conjunction<LTLFO>>());
  }
  
  SECTION("Iteration over associative operators") {
    using namespace black_internal;

    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");
    variable x = sigma.variable("x");
    variable y = sigma.variable("y");

    conjunction<LTL> c = b && ((p && (b && p)) && b);
    addition<FO> sum = x + ((y + (x + y)) + x);

    using view_t = decltype(c.operands());
    STATIC_REQUIRE(std::input_or_output_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::input_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::forward_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::ranges::range<view_t>);
    STATIC_REQUIRE(std::ranges::view<view_t>);
    STATIC_REQUIRE(std::ranges::viewable_range<view_t>);

    std::vector<formula<LTL>> v1 = {b, p, b, p, b};
    std::vector<formula<LTL>> v2;

    for(auto f : c.operands())
      v2.push_back(f);

    REQUIRE(v1 == v2);
    
    std::vector<term<FO>> tv1 = {x, y, x, y, x};
    std::vector<term<FO>> tv2;

    for(auto f : sum.operands())
      tv2.push_back(f);

    REQUIRE(tv1 == tv2);
  }

  SECTION("has_any_element_of()")
  {
    using namespace black::logic;

    proposition p = sigma.proposition("p");
    variable x = sigma.variable("x");
    boolean top = sigma.top();

    sort s = sigma.integer_sort();

    REQUIRE(has_any_element_of(
      p && !p && x > x && exists({x[s]}, x > x) && F(F(top)),
      syntax_element::boolean, quantifier<FO>::type::forall{}
    ));
  }

  SECTION("big_and, big_or, etc...") {
    using namespace black::logic;

    auto p1 = sigma.proposition("p1");
    auto p2 = sigma.proposition("p2");
    auto p3 = sigma.proposition("p3");
    auto p4 = sigma.proposition("p4");

    std::vector<proposition> v = {p1, p2, p3, p4};
    auto t = big_and(sigma, v, [&](auto p) {
      return !p;
    });

    REQUIRE(t == (((!p1 && !p2) && !p3) && !p4));
  }
}
