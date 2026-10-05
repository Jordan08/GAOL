/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the intervals built from a string.
 *
 * textToInterval("...") lexes the string, parses it into the tree of
 * gaol/gaol_expression.h, and evaluates that tree (gaol_expr_eval.h,
 * gaol_expr_visitor.h). This test goes through every node of the tree and
 * every way the string can be wrong, so that the parts of GAOL the other
 * tests never reach are run too:
 *
 * - the numbers, in every form the lexer takes (decimal, exponent,
 *   hexadecimal, the bounds given apart);
 * - the operators and the functions, alone and nested;
 * - the strings the parser refuses, which have to raise an exception rather
 *   than give an interval, and what the exceptions of GAOL say (what() and
 *   operator<<).
 *
 * The value is compared with the same computation written in C++, which the
 * other tests check against the exact results: here what is tested is the
 * lexer, the parser and the evaluation of the tree, not the operations.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

/* Before gaol/gaol, which gaol_tests.h includes: gaol/gaol_expression.h is
   included here before gaol/gaol_ieee1788.h, and after it in
   tests/ieee1788.cpp, and the functions of gaol_ieee1788 take the overloads
   of the expressions in both orders (GAOL v5) */
#include "gaol/gaol_expr_eval.h"

#include "gaol_tests.h"

#include <cstdio>
#include <type_traits>
#include <utility>

// Commented out: the tests run no thread (GAOL v5)
// // std::thread, which libstdc++ has only when built with a thread model
// #if !defined(__GLIBCXX__) || defined(_GLIBCXX_HAS_GTHREADS)
// #  include <atomic>
// #  include <thread>
// #  include <vector>
// #  define GAOL_TESTS_THREADS 1
// #endif

using namespace gaol;
using namespace gaol_tests;

namespace
{
  static_assert(std::is_same<decltype(gaol_ieee1788::sin(std::declval<const expression&>())), const expression>::value,
                "gaol_ieee1788::sin of an expression, gaol/gaol_expression.h included before gaol/gaol_ieee1788.h");

  /* expression(double) and expression(const interval&) are explicit, as
     interval(double) (GAOL v5): gaol::sin(0.5), which was ambiguous between
     the sin of [0.5] and that of an expression, does not compile, and
     gaol::sin(x) on an interval is the sin of the interval */
  template <typename T, typename = void> struct has_gaol_sin : std::false_type {};
  template <typename T>
  struct has_gaol_sin<T, decltype(void(gaol::sin(std::declval<const T&>())))> : std::true_type {};
  static_assert(!std::is_convertible<double, expression>::value, "a double does not convert implicitly to an expression");
  static_assert(!std::is_convertible<interval, expression>::value, "an interval does not convert implicitly to an expression");
  static_assert(std::is_constructible<expression, double>::value && std::is_constructible<expression, interval>::value,
                "expression(d) and expression(x) build the expressions of [d, d] and x");
  static_assert(!has_gaol_sin<double>::value, "gaol::sin(0.5) does not compile, gaol/gaol_expression.h included");
  // An integer neither, and expression(n) is expression(interval(n)) (GAOL v5, point D.21)
  static_assert(!std::is_convertible<long long, expression>::value && std::is_constructible<expression, long long>::value,
                "expression(n) builds the expression of interval(n)");
  static_assert(std::is_same<decltype(gaol::sin(std::declval<const interval&>())), interval>::value,
                "gaol::sin(x) on an interval is the sin of x, gaol/gaol_expression.h included");

  /* An empty expression of static storage, which refers to the node of the
     empty expression and is destroyed after main() has called
     gaol::cleanup(): cleanup() deleted that node, and the destructor of the
     expression then wrote into freed memory, which the sanitizers report
     (GAOL v5). The node is now freed by the automatic cleanup, after the
     static objects of the program are destroyed. */
  expression static_expression;

  // The interval of a string, and the same interval computed in C++
  /* Written on the standard error as each part starts, and flushed, so that a
     crash says where it happened: the test prints nothing else until its
     summary, and a floating-point exception left no trace at all of where it
     came from (MinGW-w64 for a 32-bit target, GAOL v5). */
  void step(const char* what)
  {
    std::fprintf(stderr, "-- %s\n", what);
    std::fflush(stderr);
  }

  void same(const std::string& text, const interval& expected)
  {
    interval got;
    bool threw = false;
    try {
      got = textToInterval(text);
    } catch (const std::exception& e) {
      threw = true;
      check("expression: no exception on a string that is right", false,
            [&] { return text + ": " + e.what(); });
    }
    if (threw) {
      return;
    }
    check("expression: the same interval as the operations of C++",
          (got.left() == expected.left() && got.right() == expected.right())
            || (got.is_empty() && expected.is_empty()),
          [&] {
            std::ostringstream o;
            o << text << " gave " << got << " rather than " << expected;
            return o.str();
          });
  }

  // A string whose value is no double: the interval has to hold it
  void encloses(const std::string& text, double a, double b)
  {
    try {
      const interval got = textToInterval(text);
      check("expression: the interval holds the value written",
            got.left() <= a && got.right() >= b,
            [&] {
              std::ostringstream o;
              o << text << " gave " << got;
              return o.str();
            });
    } catch (const std::exception& e) {
      check("expression: no exception on a string that is right", false,
            [&] { return text + ": " + e.what(); });
    }
  }

  // A string the parser has to refuse
  void refused(const std::string& text)
  {
    bool threw = false;
    try {
      const interval got = textToInterval(text);
      (void)got;
    } catch (const std::exception&) {
      threw = true;
    } catch (...) {
      threw = true;
    }
    check("expression: a string that is wrong raises an exception", threw,
          [&] { return text + " gave an interval"; });
  }

  void numbers()
  {
    same("[1,2]", interval(1.0, 2.0));
    same("[1.5,2.5]", interval(1.5, 2.5));
    same("[-2,-1]", interval(-2.0, -1.0));
    same("3", interval(3.0, 3.0));
    same("-3", interval(-3.0, -3.0));
    same("0", interval(0.0, 0.0));
    same("[0,0]", interval(0.0, 0.0));
    same("1e3", interval(1000.0, 1000.0));
    // 1e-3 is no double: the parser has to enclose it, not to round it
    encloses("1e-3", 1e-3, 1e-3);
    encloses("[1e-3,1e3]", 1e-3, 1000.0);
    encloses("0.1", 0.1, 0.1);
    encloses("[0.1,0.2]", 0.1, 0.2);
    encloses("[-0.3,-0.1]", -0.3, -0.1);
    same("[ 1 , 2 ]", interval(1.0, 2.0));
    // the bounds given apart, as tests/numbers.cpp writes them
    check("expression: the bounds given apart",
          textToInterval("[0.1,0.1]", "[0.2,0.2]").left() <= 0.1
            && textToInterval("[0.1,0.1]", "[0.2,0.2]").right() >= 0.2);
  }

  void operators()
  {
    same("1+2", interval(3.0, 3.0));
    same("[1,2]+[3,4]", interval(1.0, 2.0) + interval(3.0, 4.0));
    same("[1,2]-[3,4]", interval(1.0, 2.0) - interval(3.0, 4.0));
    same("[1,2]*[3,4]", interval(1.0, 2.0) * interval(3.0, 4.0));
    same("[1,2]/[3,4]", interval(1.0, 2.0) / interval(3.0, 4.0));
    same("-[1,2]", -interval(1.0, 2.0));
    same("-(-[1,2])", interval(1.0, 2.0));
    same("([1,2]+[3,4])*[0,1]", (interval(1.0, 2.0) + interval(3.0, 4.0)) * interval(0.0, 1.0));
    same("[1,2]+[3,4]*[0,1]", interval(1.0, 2.0) + interval(3.0, 4.0) * interval(0.0, 1.0));
    same("[1,2]-[3,4]-[0,1]", interval(1.0, 2.0) - interval(3.0, 4.0) - interval(0.0, 1.0));
    same("[1,2]/[3,4]/[1,2]", interval(1.0, 2.0) / interval(3.0, 4.0) / interval(1.0, 2.0));
    // a division by an interval holding 0
    same("[1,2]/[-1,1]", interval(1.0, 2.0) / interval(-1.0, 1.0));
    /* Every string is an expression, the intervals among its terms (GAOL v5):
       with two grammars, GAOL refused an interval after a number, and one in
       a bound, but read [1,2]*2 */
    same("1+[1,2]", interval(1.0) + interval(1.0, 2.0));
    same("2*cos([0,1])", 2.0 * cos(interval(0.0, 1.0)));
    same("[1,2]+1+[1,2]", interval(1.0, 2.0) + interval(1.0) + interval(1.0, 2.0));
    same("[1,2]*2", interval(1.0, 2.0) * 2.0);
    same("[cos([0,1]), 2]", interval(cos(interval(0.0, 1.0)).left(), 2.0));
    same("[[1,2], 3]", interval(1.0, 3.0));
    same("3.56?1*2", textToInterval("3.56?1") * 2.0);
    // a newline is a space: flex wrote it on the standard output (GAOL v5)
    same("[1,2]\n+[3,4]", interval(1.0, 2.0) + interval(3.0, 4.0));
    same("1 +\n 2", interval(3.0));
    /* pow(x, n) is gaol::pow for a negative n too: the parser took 1/x^|n|,
       whose x^|n| overflowed, and [0, 5.6e-309] for 2^-1050 (GAOL v5) */
    same("pow(2,-1050)", pow(interval(2.0), -1050));
    same("pow([-4,-1],2)", pow(interval(-4.0, -1.0), 2));
  }

  void functions()
  {
    same("cos(0)", cos(interval(0.0, 0.0)));
    same("sin(0)", sin(interval(0.0, 0.0)));
    same("tan(0)", tan(interval(0.0, 0.0)));
    same("cos([0,1])", cos(interval(0.0, 1.0)));
    same("sin([0,1])", sin(interval(0.0, 1.0)));
    same("tan([0,1])", tan(interval(0.0, 1.0)));
    same("acos([0,1])", acos(interval(0.0, 1.0)));
    same("asin([0,1])", asin(interval(0.0, 1.0)));
    same("atan([0,1])", atan(interval(0.0, 1.0)));
    same("cosh([0,1])", cosh(interval(0.0, 1.0)));
    same("sinh([0,1])", sinh(interval(0.0, 1.0)));
    same("tanh([0,1])", tanh(interval(0.0, 1.0)));
    same("acosh([1,2])", acosh(interval(1.0, 2.0)));
    same("asinh([0,1])", asinh(interval(0.0, 1.0)));
    same("atanh([0,0.5])", atanh(interval(0.0, 0.5)));
    same("exp([0,1])", exp(interval(0.0, 1.0)));
    same("log([1,2])", log(interval(1.0, 2.0)));
    same("sqrt([4,9])", sqrt(interval(4.0, 9.0)));
    same("atan2([1,2],[3,4])", atan2(interval(1.0, 2.0), interval(3.0, 4.0)));
    same("nth_root([1,8],3)", nth_root(interval(1.0, 8.0), 3));
    /* A negative exponent is the rootn of IEEE 1788-2015 for q < 0,
       1/x^(1/|q|), as nth_root(x, q) computes it in C++: the parser converted
       it to an unsigned int, and nth_root(16, -2) was the 4294967294-th root of
       16, [1.000000000645543, 1.000000000645544] rather than [0.25] (GAOL v5).
       Alone, then in the bounds of a literal. */
    same("nth_root(16,-2)", interval(0.25, 0.25));
    same("nth_root([4,16],-2)", nth_root(interval(4.0, 16.0), -2));
    same("nth_root([-8,27],-3)", nth_root(interval(-8.0, 27.0), -3));
    same("nth_root([0,16],-2)", nth_root(interval(0.0, 16.0), -2));
    same("[nth_root(16,-2)]", interval(0.25, 0.25));
    same("[nth_root(16,-2), nth_root(1,-2)]", interval(0.25, 1.0));
    same("[nth_root(-8,-3)]", interval(-0.5, -0.5));
    same("[nth_root(0,-2), 1]", interval::emptyset());
    /* The names GAOL v5 adds to the reader. cbrt(x) is nth_root(x, 3), as
       sqrt(x) is nth_root(x, 2); the lexer takes the longest name, so exp2 and
       log2 are read as themselves rather than as exp and log followed by 2. */
    same("exp2([1,2])", exp2(interval(1.0, 2.0)));
    same("log2([1,8])", log2(interval(1.0, 8.0)));
    same("cbrt([1,8])", nth_root(interval(1.0, 8.0), 3));
    same("cbrt([-8,-1])", nth_root(interval(-8.0, -1.0), 3));
    same("sign([-2,3])", sign(interval(-2.0, 3.0)));
    same("trunc([-1.5,2.7])", trunc(interval(-1.5, 2.7)));
    // the letters may be in any case, as for every other name
    same("EXP2([1,2])", exp2(interval(1.0, 2.0)));
    same("Trunc([-1.5,2.7])", trunc(interval(-1.5, 2.7)));
    // The same names in the bounds of a literal
    same("[exp2(1), exp2(2)]", interval(2.0, 4.0));
    same("[log2(1), log2(8)]", interval(0.0, 3.0));
    same("[cbrt(1), cbrt(8)]", interval(1.0, 2.0));
    same("[sign(-2), sign(3)]", interval(-1.0, 1.0));
    same("[trunc(-1.5), trunc(2.7)]", interval(-1.0, 2.0));
    same("[cbrt(27)]", interval(3.0, 3.0));
    same("[log2(exp2(5))]", interval(5.0, 5.0));
    same("[cbrt(8)+sign(5), exp2(3)]", interval(3.0, 8.0));
    // nested, and mixed with the operators
    same("exp(log([1,2]))", exp(log(interval(1.0, 2.0))));
    same("sin(cos(tan([0,1])))", sin(cos(tan(interval(0.0, 1.0)))));
    same("cos([0,1])+sin([0,1])*exp([0,1])",
         cos(interval(0.0, 1.0)) + sin(interval(0.0, 1.0)) * exp(interval(0.0, 1.0)));
    same("-exp([0,1])", -exp(interval(0.0, 1.0)));
    same("log(exp([1,2])*exp([1,2]))", log(exp(interval(1.0, 2.0)) * exp(interval(1.0, 2.0))));
    same("log2(exp2([1,2]))", log2(exp2(interval(1.0, 2.0))));
    same("trunc(exp([1,2]))", trunc(exp(interval(1.0, 2.0))));
    same("exp2([1,2])*log2([2,4])", exp2(interval(1.0, 2.0)) * log2(interval(2.0, 4.0)));
    // outside the domain: the empty set rather than an exception
    same("log([-2,-1])", log(interval(-2.0, -1.0)));
    same("sqrt([-2,-1])", sqrt(interval(-2.0, -1.0)));
    same("acos([2,3])", acos(interval(2.0, 3.0)));
    same("log2([-2,-1])", log2(interval(-2.0, -1.0)));
    /* The functions of GAOL the reader did not know, which it now looks up in
       the table of GAOL's names (GAOL v5) */
    const interval u(0.0, 1.0), v(1.0, 2.0), w(3.0, 4.0);
    same("exp10([0,1])", exp10(u));
    same("log10([1,2])", log10(v));
    same("expm1([0,1])", expm1(u));
    same("exp2m1([0,1])", exp2m1(u));
    same("exp10m1([0,1])", exp10m1(u));
    same("log1p([0,1])", log1p(u));
    same("log2p1([0,1])", log2p1(u));
    same("log10p1([0,1])", log10p1(u));
    same("hypot([1,2],[3,4])", hypot(v, w));
    same("rsqrt([1,2])", rsqrt(v));
    same("sinpi([0,1])", sinpi(u));
    same("cospi([0,1])", cospi(u));
    same("tanpi([0,0.25])", tanpi(interval(0.0, 0.25)));
    same("asinpi([0,1])", asinpi(u));
    same("acospi([0,1])", acospi(u));
    same("atanpi([0,1])", atanpi(u));
    same("atan2pi([1,2],[3,4])", atan2pi(v, w));
    same("sqr([-2,3])", sqr(interval(-2.0, 3.0)));
    same("abs([-2,1])", abs(interval(-2.0, 1.0)));
    same("min([1,2],[0,3])", min(v, interval(0.0, 3.0)));
    same("max([1,2],[0,3])", max(v, interval(0.0, 3.0)));
    same("floor([1.5,2.5])", floor(interval(1.5, 2.5)));
    same("ceil([1.5,2.5])", ceil(interval(1.5, 2.5)));
    same("integer([1.5,3.5])", integer(interval(1.5, 3.5)));
    same("round_ties_to_even([0.5,2.5])", round_ties_to_even(interval(0.5, 2.5)));
    same("round_ties_to_away([0.5,2.5])", round_ties_to_away(interval(0.5, 2.5)));
    same("inverse([2,4])", inverse(interval(2.0, 4.0)));
    same("fma([1,2],[3,4],[0,1])", fma(v, w, u));
    same("SinPi([0,1])", sinpi(u));
    same("[exp10(0), hypot(3,4)]", interval(1.0, 5.0));

    // gaol::textToInterval reads the names of GAOL, nth_root among them
    check("gaol::textToInterval(s) reads the names of GAOL",
          textToInterval("[1,2]+nth_root([8,27],3)").set_eq(interval(1.0, 2.0) + nth_root(interval(8.0, 27.0), 3)));
    check("gaol::textToInterval(sl, sr) takes the left bound of sl and the right bound of sr",
          textToInterval("[-5,4]+1", "[4,6]-[2,3]").set_eq(interval(-4.0, 4.0)));
    // Bounds in the wrong order give the empty set, which stays empty: the
    // constructor from two strings kept the bounds [2, 1], whose sum with
    // [0, 1] was [2, 2] (review #6 of examples/examples.md; GAOL v5)
    check("gaol::textToInterval(\"2\", \"1\") is the empty set, which stays empty",
          textToInterval("2", "1").is_empty() && (textToInterval("2", "1") + interval(0.0, 1.0)).is_empty(),
          [] { return hex(textToInterval("2", "1") + interval(0.0, 1.0)); });
    bool threw = false;
    try {
      const interval z = textToInterval("pown([2,5],5)");
      (void)z;
    } catch (const input_format_error&) {
      threw = true;
    }
    check("gaol::textToInterval throws input_format_error for a name of IEEE 1788-2015 alone", threw);
  }

  void wrong_strings()
  {
    refused("");
    refused("[");
    refused("]");
    refused("[1,");
    refused("[1 2]");
    refused("1+");
    refused("+");
    refused("*[1,2]");
    refused("([1,2]");
    refused("[1,2])");
    refused("cos");
    refused("cos(");
    refused("cos()");
    refused("unknown([1,2])");
    refused("[1,2] [3,4]");
    refused("$");
    refused("[1,2]^3");        // the parser has no power operator
    refused("atan2([1,2])");   // atan2 takes two arguments
    refused("sin(1,2)");
    refused("pow(2)");
    refused("fma(1,2)");
    refused("nth_root(8)");
    /* The names of IEEE 1788-2015 alone, which gaol_ieee1788::textToInterval
       reads: gaol::textToInterval reads those of GAOL */
    refused("pown([2,5],5)");
    refused("rootn(8,3)");
    refused("recip([2,4])");
    refused("logp1(0)");
    refused("roundTiesToEven(1)");
    refused("add(1,2)");
    /* An error stops the reading: each literal set the flag of success again,
       and this string gave [-oo, +oo] (GAOL v5) */
    refused("[nth_root(8,1.5)]+[1,2]");
    refused("[1,2]+[nth_root(8,1.5)]");
  }

  /* What an exception of GAOL says as a std::exception (GAOL v5). Its what()
     was the one of the standard class, "std::exception" with libstdc++ and
     libc++ whatever went wrong: the text that a handler of std::exception
     printed, and the one that a program ends with when nothing catches the
     exception ("what():  std::exception"), where explanation() had the
     explanation. what() is now the explanation, or "gaol_exception" where
     there is none, and operator<< writes the explanation once, where it
     wrote what() and the explanation. The exceptions are built here with a
     known explanation, so that what they say does not depend on the text of
     the reader. */
  template<class Exception>
  void says(const char* name)
  {
    const unsigned line = __LINE__;
    const Exception with_text(__FILE__, line, "boom");
    const Exception with_string(__FILE__, line, std::string("boom"));
    const Exception without(__FILE__, line);

    // Through the base class, as a handler of std::exception reads them
    const std::exception& a = with_text;
    const std::exception& b = with_string;
    const std::exception& c = without;
    check("exception: what() is the explanation, given as a text",
          std::string(a.what()) == "boom", [&] { return std::string(name) + ": \"" + a.what() + "\""; });
    check("exception: what() is the explanation, given as a string",
          std::string(b.what()) == "boom", [&] { return std::string(name) + ": \"" + b.what() + "\""; });
    check("exception: what() says gaol_exception where there is no explanation",
          std::string(c.what()) == "gaol_exception", [&] { return std::string(name) + ": \"" + c.what() + "\""; });

    const std::string where = std::string(__FILE__) + ", line " + std::to_string(line) + ": exception thrown";
    std::ostringstream shown, shown_without;
    shown << with_text;
    shown_without << without;
    check("exception: operator<< writes the file, the line and the explanation, once",
          shown.str() == where + ": boom", [&] { return std::string(name) + ": " + shown.str(); });
    check("exception: operator<< writes no explanation where there is none",
          shown_without.str() == where, [&] { return std::string(name) + ": " + shown_without.str(); });
  }

  /* f, which has to throw an exception of GAOL, caught as a std::exception
     as a program that knows nothing of GAOL does: its what() has to be the
     explanation that the exception gives */
  template<class F>
  void thrown_says(const std::string& done, const F& f)
  {
    bool caught = false, from_gaol = false;
    std::string what, explanation;
    try {
      f();
    } catch (const std::exception& e) {
      caught = true;
      what = e.what();
      if (const gaol_exception* g = dynamic_cast<const gaol_exception*>(&e)) {
        from_gaol = true;
        explanation = g->explanation();
      }
    }
    check("exception: GAOL throws a gaol_exception, which a handler of std::exception catches",
          caught && from_gaol,
          [&] { return done + (caught ? " threw an exception that is no gaol_exception" : " threw nothing"); });
    check("exception: what() is the explanation of what GAOL threw",
          from_gaol && !explanation.empty() && what == explanation,
          [&] { return done + ": what() is \"" + what + "\", explanation() \"" + explanation + "\""; });
  }

  void exception_messages()
  {
    says<gaol_exception>("gaol_exception");
    says<input_format_error>("input_format_error");
    says<unavailable_feature_error>("unavailable_feature_error");
    says<invalid_action_error>("invalid_action_error");

    // What GAOL throws: a string the reader refuses, as it reads it or at its
    // end, a function called with an argument it does not take, and operator>>
    thrown_says("textToInterval(\"sin(1)+\")", [] { const interval x = textToInterval("sin(1)+"); (void)x; });
    thrown_says("textToInterval(\"[1, 2\")", [] { const interval x = textToInterval("[1, 2"); (void)x; });
    thrown_says("textToInterval(\"nth_root(8,1.5)\")", [] { const interval x = textToInterval("nth_root(8,1.5)"); (void)x; });
    thrown_says("nb_fp_numbers(NaN, 1)", [] { (void)nb_fp_numbers(std::numeric_limits<double>::quiet_NaN(), 1.0); });
    thrown_says("operator>> of \"[1, 2\"", [] { std::istringstream in("[1, 2"); interval x; in >> x; });
  }

  /* Many decimal intervals, whose bounds are no doubles: the interval read has
     to hold them, and to be no wider than the two doubles around them. */
  void decimals()
  {
    const char* texts[] = {
      "[0.1,0.2]", "[1.23456789012345,1.23456789012346]", "[-1e-300,1e-300]",
      "[1e300,1.0001e300]", "[3.141592653589793,3.141592653589794]",
      "[0.333333333333333,0.666666666666667]", "[-2.5e-10,2.5e-10]",
      "[1e-320,1e-310]", "[0.9999999999999999,1.0000000000000002]",
    };
    for (const char* t : texts) {
      interval got;
      try {
        got = textToInterval(t);
      } catch (const std::exception& e) {
        check("expression: a decimal interval is read", false,
              [&] { return std::string(t) + ": " + e.what(); });
        continue;
      }
      double a = 0.0, b = 0.0;
      if (std::sscanf(t, "[%lf,%lf]", &a, &b) != 2) {
        continue;
      }
      check("expression: a decimal interval holds its bounds",
            got.left() <= a && got.right() >= b,
            [&] {
              std::ostringstream o;
              o << t << " gave " << got;
              return o.str();
            });
      check("expression: a decimal interval is no wider than two doubles",
            got.left() >= previous_float(a) && got.right() <= next_float(b),
            [&] {
              std::ostringstream o;
              o << t << " gave " << got;
              return o.str();
            });
    }
  }

  /* The expressions built in C++ rather than read from a string
     (gaol/gaol_expression.h): every node is built, printed, copied and
     evaluated, which the strings alone do not reach. */
  void built_expressions()
  {
    const expression x = expression(interval(1.0, 2.0));
    const expression y = expression(interval(3.0, 4.0));
    const expression d = expression(2.5);
    // 2^53 + 1, which expression(double) made the double 2^53: expression(n)
    // is the interval of the two doubles around it (GAOL v5, point D.21)
    const double two53 = std::ldexp(1.0, 53);
    const expression n = expression(9007199254740993LL);

    struct Case { expression e; interval value; const char* name; };
    const Case cases[] = {
      {x + y, interval(1.0, 2.0) + interval(3.0, 4.0), "x+y"},
      {x - y, interval(1.0, 2.0) - interval(3.0, 4.0), "x-y"},
      {x * y, interval(1.0, 2.0) * interval(3.0, 4.0), "x*y"},
      {x / y, interval(1.0, 2.0) / interval(3.0, 4.0), "x/y"},
      {-x, -interval(1.0, 2.0), "-x"},
      {x + d, interval(1.0, 2.0) + interval(2.5), "x+d"},
      {pow(x, y), pow(interval(1.0, 2.0), interval(3.0, 4.0)), "x^y"},
      // pow(e, n) with an int n, which did not link: the library defined it
      // with an unsigned int
      {pow(x, 3), pow(interval(1.0, 2.0), 3), "x^3"},
      {nth_root(y, 2), nth_root(interval(3.0, 4.0), 2), "nth_root(y,2)"},
      {cos(x), cos(interval(1.0, 2.0)), "cos(x)"},
      {sin(x), sin(interval(1.0, 2.0)), "sin(x)"},
      {tan(expression(interval(0.0, 1.0))), tan(interval(0.0, 1.0)), "tan"},
      {acos(expression(interval(0.0, 1.0))), acos(interval(0.0, 1.0)), "acos"},
      {asin(expression(interval(0.0, 1.0))), asin(interval(0.0, 1.0)), "asin"},
      {atan(x), atan(interval(1.0, 2.0)), "atan"},
      {cosh(x), cosh(interval(1.0, 2.0)), "cosh"},
      {sinh(x), sinh(interval(1.0, 2.0)), "sinh"},
      {tanh(x), tanh(interval(1.0, 2.0)), "tanh"},
      {acosh(x), acosh(interval(1.0, 2.0)), "acosh"},
      {asinh(x), asinh(interval(1.0, 2.0)), "asinh"},
      {atanh(expression(interval(0.0, 0.5))), atanh(interval(0.0, 0.5)), "atanh"},
      {exp(x), exp(interval(1.0, 2.0)), "exp"},
      {log(x), log(interval(1.0, 2.0)), "log"},
      {exp2(x), exp2(interval(1.0, 2.0)), "exp2"},
      {log2(x), log2(interval(1.0, 2.0)), "log2"},
      {sign(x), sign(interval(1.0, 2.0)), "sign"},
      {trunc(x), trunc(interval(1.0, 2.0)), "trunc"},
      {nth_root(x, 3), nth_root(interval(1.0, 2.0), 3), "nth_root(x,3)"},
      {n, interval(two53, two53 + 2), "expression(2^53+1)"},
      {x + n, interval(1.0, 2.0) + interval(two53, two53 + 2), "x+expression(2^53+1)"},
      {expression(7), interval(7.0), "expression(7)"},
      // pow(e, n) and nth_root(e, n) for an integer of another type (GAOL v5):
      // an unsigned beyond the ints became a negative int, [0, 1] for
      // [1, 2]^3000000000u, nth_root(e, -2) took the order 4294967294, and a
      // long beyond the unsigned ints was reduced modulo 2^32. Beyond them,
      // the roots of an expression are those of the intervals but where e
      // contains 0 (see roots_of_expressions())
      {pow(x, 3L), pow(interval(1.0, 2.0), 3), "x^3L"},
      {pow(x, 3000000000u), interval::universe(), "x^3000000000u"},
      {pow(x, 5000000000LL), interval::universe(), "x^5e9"},
      {nth_root(x, 3L), nth_root(interval(1.0, 2.0), 3), "nth_root(x,3L)"},
      {nth_root(y, -2), interval(1.0) / nth_root(interval(3.0, 4.0), 2u), "nth_root(y,-2)"},
      {nth_root(x, 5000000000LL), nth_root(interval(1.0, 2.0), 5000000000LL), "nth_root(x,5e9)"},
      {nth_root(x, 5000000001LL), nth_root(interval(1.0, 2.0), 5000000001LL), "nth_root(x,5e9+1)"},
      {nth_root(-x, 5000000001LL), nth_root(interval(-2.0, -1.0), 5000000001LL), "nth_root(-x,5e9+1)"},
      {nth_root(x, -3000000000LL), nth_root(interval(1.0, 2.0), -3000000000LL), "nth_root(x,-3e9)"},
      {nth_root(-x, -5000000001LL), nth_root(interval(-2.0, -1.0), -5000000001LL), "nth_root(-x,-5e9-1)"},
      {cos(x) + sin(y) * exp(x), cos(interval(1.0, 2.0)) + sin(interval(3.0, 4.0)) * exp(interval(1.0, 2.0)), "nested"},
    };

    for (const Case& c : cases) {
      // evaluated
      expr_eval ev;
      c.e.get_root()->accept(ev);
      const interval got = ev.result();
      check("built expression: the same interval as the operations",
            got.left() == c.value.left() && got.right() == c.value.right(),
            [&] {
              std::ostringstream o;
              o << c.name << " gave " << got << " rather than " << c.value;
              return o.str();
            });
      // printed
      std::ostringstream o;
      o << c.e;
      check("built expression: printed as something", !o.str().empty(),
            [&] { return std::string(c.name) + " printed nothing"; });
      // copied, assigned, and evaluated again
      expression copy(c.e);
      expression assigned;
      assigned = c.e;
      expr_eval ev2, ev3;
      copy.get_root()->accept(ev2);
      assigned.get_root()->accept(ev3);
      const interval a = ev2.result(), b = ev3.result();
      check("built expression: the copy and the assignment give the same",
            a.left() == got.left() && a.right() == got.right()
              && b.left() == got.left() && b.right() == got.right(),
            [&] { return std::string(c.name); });
      // cloned node by node
      expr_node* clone = c.e.get_root()->clone();
      expr_eval ev4;
      clone->accept(ev4);
      const interval cl = ev4.result();
      check("built expression: the clone gives the same",
            cl.left() == got.left() && cl.right() == got.right(),
            [&] { return std::string(c.name); });
      delete clone;
    }

    // expression(n) for an integer that is a double is expression(double),
    // written as the double, and a visitor sees a double_node, as before the
    // overload for the integers (GAOL v5)
    {
      std::ostringstream a, b;
      a << expression(7) << ' ' << pow(expression(-2), 2);
      b << expression(7.0) << ' ' << pow(expression(-2.0), 2);
      check("expression(7) is expression(7.0)", a.str() == b.str()
              && dynamic_cast<double_node*>(expression(7).get_root()) != nullptr,
            [&] { return a.str() + " rather than " + b.str(); });
    }

    // The roots of an expression of order beyond the unsigned ints, where e
    // contains 0: an enclosure of those of the intervals, sign(e) being
    // [-1, 1] there (GAOL v5)
    {
      const interval z(-3.0, 2.0);
      expr_eval ev;
      nth_root(expression(z), 5000000001LL).get_root()->accept(ev);
      const interval got = ev.result(), roots = nth_root(z, 5000000001LL);
      check("nth_root(e, 5e9 + 1) of an expression containing 0 encloses the roots", got.set_contains(roots),
            [&] {
              std::ostringstream o;
              o << got << " does not contain " << roots;
              return o.str();
            });
    }

    // the operators that change the expression in place; /= was declared and
    // not defined, and a program using it did not link (GAOL v5)
    expression acc = expression(interval(1.0, 1.0));
    acc += x;
    acc -= d;
    acc *= y;
    acc /= x;
    expr_eval ev;
    acc.get_root()->accept(ev);
    const interval got = ev.result();
    const interval expected =
      (((interval(1.0) + interval(1.0, 2.0)) - interval(2.5)) * interval(3.0, 4.0)) / interval(1.0, 2.0);
    check("built expression: +=, -=, *= and /=",
          got.left() == expected.left() && got.right() == expected.right(),
          [&] {
            std::ostringstream o;
            o << got << " rather than " << expected;
            return o.str();
          });

    /* Printed as written (GAOL v5): the division was printed with '*', and a
       negative number without the parentheses a power of it needs */
    {
      const expression z = expression(interval(5.0, 6.0));
      std::ostringstream quotient, power;
      quotient << x / (y * z);
      power << pow(expression(-2.0), 2);
      check("built expression: printed as written, x/(y*z)",
            quotient.str() == "[1, 2]/([3, 4]*[5, 6])", [&] { return quotient.str(); });
      check("built expression: printed as written, (-2)^2",
            power.str() == "(-2)^2", [&] { return power.str(); });
    }

    // the empty expression, whose evaluation is an error
    expression empty;
    std::ostringstream o;
    o << empty;
    check("built expression: the empty one is printed", true);
  }

  /* The empty expressions, of every thread, point to one node, the_null_expr,
     whose references are not counted (GAOL v5): its count, a plain unsigned
     int that expressions created and destroyed in two threads changed at
     once, lost updates and came down to 0, and the node was deleted twice.
     The tests run no thread: the count is checked not to change as empty
     expressions are built, copied, assigned, extended and divided. */
  void null_node_not_counted()
  {
    const unsigned int before = gaol_core::the_null_expr->references();
    unsigned int during = 0;
    {
      expression e, f = e;
      expression g;
      g = f;
      g += expression(1.0);
      expression h;
      h /= expression(2.0);
      during = gaol_core::the_null_expr->references();
    }
    const unsigned int after = gaol_core::the_null_expr->references();
    check("expression: the empty expressions do not count their references to the_null_expr",
          before == during && during == after,
          [&] { return std::to_string(before) + ", " + std::to_string(during) + ", " + std::to_string(after); });
  }

// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   /* Strings read by four threads at once, with the names of GAOL and with
//      those of IEEE 1788-2015 (GAOL v5). The lexer of flex, the parser of bison
//      and the state of GAOL's reader were globals: reading two strings at once
//      crashed, "fatal flex scanner internal error" or a segmentation fault,
//      before the lexer became reentrant and the parser pure. */
//   void reading_in_threads()
//   {
//     struct Reading { const char *text; bool standard; };
//     const Reading readings[] = {
//       {"[1,2]+sin([0,1])*3", false}, {"pow([-4,-1],2)", false},
//       {"pow([-4,-1],2)", true}, {"[0.1, rootn(27,3)]*hypot(3,4)", true},
//     };
//     const auto read = [](const Reading& r) {
//       return r.standard ? gaol_ieee1788::textToInterval(r.text) : gaol::textToInterval(r.text);
//     };
//     interval expected[4];
//     for (int t = 0; t < 4; ++t) {
//       expected[t] = read(readings[t]);
//     }
//     std::atomic<long> wrong(0);
//     std::vector<std::thread> threads;
//     for (int t = 0; t < 4; ++t) {
//       threads.emplace_back([&, t] {
//         for (int i = 0; i < 5000; ++i) {
//           try {
//             if (!read(readings[t]).set_eq(expected[t])) {
//               ++wrong;
//             }
//           } catch (...) {
//             ++wrong;
//           }
//         }
//       });
//     }
//     for (std::thread& t : threads) {
//       t.join();
//     }
//     check("expression: strings read by four threads at once", wrong == 0,
//           [&] { return std::to_string(wrong.load()) + " wrong of 20000"; });
//     check("expression: pow([-4,-1],2) is [1, 16] with the names of GAOL, empty with those of the standard",
//           expected[1].set_eq(interval(1.0, 16.0)) && expected[2].is_empty());
//   }
// #endif
}

int main()
{
  gaol::init();
  step("numbers");           numbers();
  step("operators");         operators();
  step("functions");         functions();
  step("wrong_strings");     wrong_strings();
  step("exception_messages"); exception_messages();
  step("decimals");          decimals();
  step("built_expressions"); built_expressions();
  step("null_node_not_counted"); null_node_not_counted();
// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   step("reading_in_threads"); reading_in_threads();
// #endif
  {
    std::ostringstream text;
    text << static_expression;
    check("a static empty expression", static_expression.get_root() == gaol_core::the_null_expr && text.str() == ":null:",
          [&] { return text.str(); });
  }
  step("summary");
  const int status = summary();
  gaol::cleanup();
  return status;
}
