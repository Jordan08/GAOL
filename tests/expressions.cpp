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
 *   than give an interval;
 * - the long and the deeply nested expressions: sums and products of 200000
 *   terms, differences and quotients of 20000, read from a string and built in
 *   C++, and chains of 200000 minus signs and 20000 sines, which have to be
 *   evaluated and deleted whatever their depth, on no more stack than a tree
 *   of two nodes takes, and the nesting the reader refuses.
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

#include "gaol_tests.h"

#include <algorithm>
#include <cstdio>
#include <random>
#include "gaol/gaol_expr_eval.h"

#if defined(_MSC_VER)
#  include <intrin.h>
#endif

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
    // What the checks of the last part printed, which a crash in this one
    // would take with the buffer of a pipe (GAOL v5)
    std::fflush(stdout);
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
    const unsigned int before = the_null_expr->references();
    unsigned int during = 0;
    {
      expression e, f = e;
      expression g;
      g = f;
      g += expression(1.0);
      expression h;
      h /= expression(2.0);
      during = the_null_expr->references();
    }
    const unsigned int after = the_null_expr->references();
    check("expression: the empty expressions do not count their references to the_null_expr",
          before == during && during == after,
          [&] { return std::to_string(before) + ", " + std::to_string(during) + ", " + std::to_string(after); });
  }

  /* The long and the deeply nested expressions (GAOL v5)

     A sum of 100000 terms read from a string, and an expression that long
     built in C++, were trees 100000 nodes deep, whose evaluation and
     deletion made one call within another per node: the evaluation
     overflowed the stack of the program, 8 MB with Linux and macOS, and
     crashed it (the deletion, whose frames are smaller, needs a longer
     chain). The parser now computes an operation when it reads it, the
     evaluation and the deletion of a tree take stacks of their own in the
     heap, and the reader keeps what it reads in a stack of the heap too, of
     10000 entries: a string nested deeper, with parentheses, signs or calls,
     is refused. */

  // The operation of the operator op, one of + - * /
  interval operate(char op, const interval& a, const interval& b)
  {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    default:  return a / b;
    }
  }

  // The expression a op b
  expression combine(char op, const expression& a, const expression& b)
  {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    default:  return a / b;
    }
  }

  /* "a op b op a op b ...", n terms, the operators in turn those of ops: the
     operators of one cycle have the same precedence, and group from the left */
  std::string repeated(int n, const std::string& ops, const std::string& a, const std::string& b)
  {
    std::string text;
    text.reserve(static_cast<std::size_t>(n) * (std::max(a.size(), b.size()) + 1));
    for (int i = 0; i < n; ++i) {
      if (i > 0) {
        text += ops[static_cast<std::size_t>(i - 1) % ops.size()];
      }
      text += (i % 2 == 0) ? a : b;
    }
    return text;
  }

  // The same bounds to the bit, the signs of the zeros and the empty set included
  bool same_bits(const interval& a, const interval& b)
  {
    return hex(a) == hex(b);
  }

  /* Sums and products have to be 200000 terms long to overflow the stack of
     8 MB of Linux and macOS with a recursion of about 100 bytes for each
     node; the differences and the quotients, which do not commute, only have
     to show that the operands are taken in the right order, in fewer */
  const int long_terms = 200000;
  const int short_terms = 20000;

  int terms_of(char op)
  {
    return (op == '+' || op == '*') ? long_terms : short_terms;
  }

  // e op= x
  void update(char op, expression& e, const expression& x)
  {
    switch (op) {
    case '+': e += x; break;
    case '-': e -= x; break;
    case '*': e *= x; break;
    default:  e /= x; break;
    }
  }

  /* Chains of operations read from a string: the value is the one of the
     same operations in C++, grouped from the left as the reader groups them.
     The long chains are of pi, which the lexer reads in no time: it takes
     tens of microseconds to read a number when the sanitizers slow it down,
     as it does the decimals of the short chains, whose operations round. */
  void long_strings()
  {
    struct Chain { const char* ops; const char* a; const char* b; int terms; };
    const Chain chains[] = {
      {"+", "pi", "pi", long_terms}, {"*/", "pi", "pi", long_terms}, {"-+", "pi", "pi", short_terms},
      {"+", "0.1", "0.2", 2000}, {"-+", "0.1", "0.2", 2000}, {"*/", "1.0000001", "0.9999999", 2000},
    };
    for (const Chain& c : chains) {
      const interval ta = textToInterval(c.a), tb = textToInterval(c.b);
      const std::string ops = c.ops;
      interval expected = ta;
      for (int i = 1; i < c.terms; ++i) {
        expected = operate(ops[static_cast<std::size_t>(i - 1) % ops.size()], expected, (i % 2 == 0) ? ta : tb);
      }
      const std::string what = std::string("\"") + c.a + c.ops[0] + c.b + "...\", " + std::to_string(c.terms) + " terms";
      try {
        const interval got = textToInterval(repeated(c.terms, ops, c.a, c.b));
        check("long expression: a long string is read as its operators group them",
              same_bits(got, expected),
              [&] { return what + " gave " + hex(got) + " rather than " + hex(expected); });
      } catch (const std::exception& e) {
        check("long expression: no exception on a long string", false,
              [&] { return what + ": " + e.what(); });
      }
    }
  }

  /* The chains built in C++, on the left with the operators that change an
     expression in place, on the right with the others, then chains of unary
     operations: the value is the one of the same operations in C++, in the
     same order. The trees are deleted when they go out of scope, which is a
     part of the test too. */
  void long_built()
  {
    for (const char op : {'+', '-', '*', '/'}) {
      const int terms = terms_of(op);
      // Terms whose sums and products round, and neither overflow nor underflow in 200000 of them
      const interval t = (op == '+' || op == '-') ? interval(0.1, 0.2) : interval(0.9999999, 1.0000001);

      expression left(t), right(t);
      interval expected_left = t, expected_right = t;
      for (int i = 1; i < terms; ++i) {
        update(op, left, expression(t));
        expected_left = operate(op, expected_left, t);
        right = combine(op, expression(t), right);
        expected_right = operate(op, t, expected_right);
      }
      interval got_left, got_right;
      const bool evaluated_left = evaluate_expr(left, got_left);
      const bool evaluated_right = evaluate_expr(right, got_right);
      check("long expression: a chain built on the left has the value of the operations in C++",
            evaluated_left && same_bits(got_left, expected_left),
            [&] { return std::string(1, op) + " gave " + hex(got_left) + " rather than " + hex(expected_left); });
      check("long expression: a chain built on the right has the value of the operations in C++",
            evaluated_right && same_bits(got_right, expected_right),
            [&] { return std::string(1, op) + " gave " + hex(got_right) + " rather than " + hex(expected_right); });
    }

    // Chains of unary operations: minus signs, and sines
    {
      const interval t(0.1, 0.2);
      expression signs(t), sines(t);
      interval expected_sines = t;
      for (int i = 0; i < long_terms; ++i) {
        signs = -signs;
      }
      for (int i = 0; i < short_terms; ++i) {
        sines = sin(sines);
        expected_sines = sin(expected_sines);
      }
      interval got_signs, got_sines;
      bool evaluated = evaluate_expr(signs, got_signs);
      check("long expression: 200000 minus signs, an even number, are no sign",
            evaluated && same_bits(got_signs, t), [&] { return hex(got_signs); });
      signs = -signs;
      evaluated = evaluate_expr(signs, got_signs);
      check("long expression: 200001 minus signs are one",
            evaluated && same_bits(got_signs, -t), [&] { return hex(got_signs); });
      evaluated = evaluate_expr(sines, got_sines);
      check("long expression: a sine of a sine of 20000 sines",
            evaluated && same_bits(got_sines, expected_sines),
            [&] { return hex(got_sines) + " rather than " + hex(expected_sines); });
    }
  }

  /* What the reader does with a string nested deeper than its stack of 10000
     entries: it refuses it, as a syntax error, where it read a string 9000
     deep. The nesting of parentheses, of brackets, of signs and of calls of
     functions: none is more than a few thousand nodes deep once read, which
     the evaluation and the deletion of the tree take without recursion. */
  void deep_strings()
  {
    const auto nested = [](int depth, const char* open, const char* inner, const char* close) {
      std::string text;
      for (int i = 0; i < depth; ++i) {
        text += open;
      }
      text += inner;
      for (int i = 0; i < depth; ++i) {
        text += close;
      }
      return text;
    };

    // Accepted: 9000 levels of parentheses, of brackets and of signs, and thousands of calls
    same(nested(9000, "(", "1", ")"), interval(1.0));
    same(nested(9000, "[", "1", "]"), interval(1.0));
    same(nested(4000, "sin(", "0", ")"), interval(0.0));
    same(nested(2000, "pow(1,", "1", ")"), interval(1.0));
    same(nested(9000, "-", "3", ""), interval(3.0));
    same(nested(8999, "-", "3", ""), interval(-3.0));
    same(nested(9000, "-", "inf", ""), textToInterval("inf"));
    same(nested(8999, "-", "inf", ""), textToInterval("-inf"));
    same("[" + nested(9000, "-", "1", "") + ", 2]", interval(1.0, 2.0));
    same("[" + nested(9000, "-", "inf", "") + ", 2]", interval::emptyset());
    same("[" + nested(8999, "-", "inf", "") + ", 2]", interval(-inf, 2.0));
    same("1+" + nested(9000, "-", "3", "") + "*2", interval(7.0));
    same(nested(4000, "(", "1", "+1)"), interval(4001.0));
    // An uncertain number behind thousands of signs is still no bound
    refused("[" + nested(9000, "-", "3.56?1", "") + ", 5]");
    refused("[1, " + nested(9001, "-", "3.56?1", "") + "]");

    // Refused as a syntax error, an input_format_error, whatever the depth
    const struct { const char* name; std::string text; } refusals[] = {
      {"parentheses", nested(100000, "(", "1", ")")},
      {"brackets", nested(100000, "[", "1", "]")},
      {"signs", nested(100000, "-", "1", "")},
      {"calls", nested(100000, "sin(", "1", ")")},
      {"nested sums", nested(100000, "(", "1", "+1)")},
    };
    for (const auto& r : refusals) {
      bool syntax_error = false, other = false;
      try {
        const interval x = textToInterval(r.text);
        (void)x;
      } catch (const input_format_error&) {
        syntax_error = true;
      } catch (...) {
        other = true;
      }
      check("deep string: 100000 levels are refused with an input_format_error", syntax_error && !other,
            [&] { return std::string(r.name) + (other ? ": another exception" : ": read"); });
    }
  }

#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_TESTS_FRAME_POSITION() reinterpret_cast<std::uintptr_t>(__builtin_frame_address(0))
#elif defined(_MSC_VER)
#  define GAOL_TESTS_FRAME_POSITION() reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress())
#endif

#ifdef GAOL_TESTS_FRAME_POSITION
  /* A leaf that notes where the stack of the program is when it is visited
     by an evaluation and when it is deleted. Under a chain of nodes, the
     recursion of an evaluation or of a deletion has put a frame on the stack
     for each node above the leaf: the further the leaf is from where the
     evaluation or the deletion started, the deeper the recursion.
     The position is the address of the frame, and not the one of a local
     variable, which the sanitizers move to a stack of their own. */
  class probe_leaf : public expr_node
  {
    public:
      explicit probe_leaf(double v) : value_(v) {}
      ~probe_leaf() override { deleted_at = GAOL_TESTS_FRAME_POSITION(); }
      expr_node* clone() const override { return new probe_leaf(value_.get_val()); }
      void accept(expr_visitor& visitor) override
      {
        visited_at = GAOL_TESTS_FRAME_POSITION();
        visitor.visit(&value_);
      }
      unsigned int get_precedence() const override { return value_.get_precedence(); }
      std::ostream& display(std::ostream& os) const override { return value_.display(os); }

      static std::uintptr_t visited_at, deleted_at;

    private:
      double_node value_;
  };

  std::uintptr_t probe_leaf::visited_at = 0;
  std::uintptr_t probe_leaf::deleted_at = 0;

  // The bytes of stack between two positions, whichever way the stack grows
  std::uintptr_t stack_between(std::uintptr_t a, std::uintptr_t b)
  {
    return a > b ? a - b : b - a;
  }
#endif

  /* The stack an evaluation and a deletion use does not depend on the depth of
     the tree: the leaf at the bottom of a chain of 20000 nodes is visited and
     deleted less than 64 KiB of stack away from the function that starts
     them, where a recursion needs a frame, 16 bytes at the very least, for
     each node: 320 KB. Deterministic, whatever the stack of the program and
     the size of the frames (the sanitizers make them larger), and it fails
     without crashing, where the chains of 200000 nodes above overflow the
     stack of 8 MB only. */
  void stack_use()
  {
#ifdef GAOL_TESTS_FRAME_POSITION
    const int terms = 20000;
    const std::uintptr_t limit = 64 * 1024;
    const char* const shapes[] = {"on the left", "on the right", "of minus signs"};
    const std::uintptr_t here = GAOL_TESTS_FRAME_POSITION();
    for (int shape = 0; shape < 3; ++shape) {
      probe_leaf::visited_at = probe_leaf::deleted_at = 0;
      interval value;
      bool evaluated = false;
      {
        // The leaf is at the bottom: leftmost, rightmost, or under the signs
        expression e(*new probe_leaf(1.0));
        for (int i = 1; i < terms; ++i) {
          if (shape == 0) {
            e += expression(1.0);
          } else if (shape == 1) {
            e = expression(1.0) + e;
          } else {
            e = -e;
          }
        }
        evaluated = evaluate_expr(e, value);
      }
      const interval expected = (shape < 2) ? interval(static_cast<double>(terms)) : -interval(1.0);
      check("stack use: the value of a chain of 20000 nodes", evaluated && same_bits(value, expected),
            [&] { return std::string(shapes[shape]) + ": " + hex(value); });
      const std::uintptr_t evaluation = stack_between(here, probe_leaf::visited_at);
      const std::uintptr_t deletion = stack_between(here, probe_leaf::deleted_at);
      check("stack use: the evaluation of a chain of 20000 nodes takes less than 64 KiB of stack",
            probe_leaf::visited_at != 0 && evaluation < limit,
            [&] { return std::string(shapes[shape]) + ": the leaf was visited " + std::to_string(evaluation) + " bytes down"; });
      check("stack use: the deletion of a chain of 20000 nodes takes less than 64 KiB of stack",
            probe_leaf::deleted_at != 0 && deletion < limit,
            [&] { return std::string(shapes[shape]) + ": the leaf was deleted " + std::to_string(deletion) + " bytes down"; });
    }
#else
    std::printf("stack use: no frame address on this compiler: skipped\n");
#endif
  }

  // The value of a random expression, computed as the expression is built
  struct Modeled
  {
    expression e;
    interval v;
  };

  Modeled random_leaf(std::mt19937& rng)
  {
    const double doubles[] = {-2.5, -1.0, -0.5, 0.0, 0.25, 0.5, 1.0, 1.5, 2.0, 3.0};
    const interval intervals[] = {interval(-1.0, 1.0), interval(0.5, 2.0), interval(-3.0, -1.0),
                                  interval(0.0, 0.5), interval(1.0, 4.0), interval(-0.5, 0.25)};
    if (rng() % 2 == 0) {
      const double d = doubles[rng() % 10];
      return Modeled{expression(d), interval(d)};
    }
    const interval& i = intervals[rng() % 6];
    return Modeled{expression(i), i};
  }

  Modeled random_tree(std::mt19937& rng, int depth)
  {
    if (depth == 0 || rng() % 6 == 0) {
      return random_leaf(rng);
    }
    const unsigned int kind = static_cast<unsigned int>(rng() % 27);
    const Modeled a = random_tree(rng, depth - 1);
    if (kind < 21) {
      switch (kind) {
      case 0:  return Modeled{-a.e, -a.v};
      case 1:  return Modeled{pow(a.e, 3), pow(a.v, 3)};
      case 2:  return Modeled{nth_root(a.e, 3), nth_root(a.v, 3)};
      case 3:  return Modeled{cos(a.e), cos(a.v)};
      case 4:  return Modeled{sin(a.e), sin(a.v)};
      case 5:  return Modeled{tan(a.e), tan(a.v)};
      case 6:  return Modeled{acos(a.e), acos(a.v)};
      case 7:  return Modeled{asin(a.e), asin(a.v)};
      case 8:  return Modeled{atan(a.e), atan(a.v)};
      case 9:  return Modeled{cosh(a.e), cosh(a.v)};
      case 10: return Modeled{sinh(a.e), sinh(a.v)};
      case 11: return Modeled{tanh(a.e), tanh(a.v)};
      case 12: return Modeled{acosh(a.e), acosh(a.v)};
      case 13: return Modeled{asinh(a.e), asinh(a.v)};
      case 14: return Modeled{atanh(a.e), atanh(a.v)};
      case 15: return Modeled{exp(a.e), exp(a.v)};
      case 16: return Modeled{log(a.e), log(a.v)};
      case 17: return Modeled{exp2(a.e), exp2(a.v)};
      case 18: return Modeled{log2(a.e), log2(a.v)};
      case 19: return Modeled{sign(a.e), sign(a.v)};
      default: return Modeled{trunc(a.e), trunc(a.v)};
      }
    }
    // A quarter of the operations have the same operand twice: a node that two nodes share
    const Modeled b = (rng() % 4 == 0) ? a : random_tree(rng, depth - 1);
    switch (kind) {
    case 21: return Modeled{a.e + b.e, a.v + b.v};
    case 22: return Modeled{a.e - b.e, a.v - b.v};
    case 23: return Modeled{a.e * b.e, a.v * b.v};
    case 24: return Modeled{a.e / b.e, a.v / b.v};
    case 25: return Modeled{pow(a.e, b.e), pow(a.v, b.v)};
    default: return Modeled{atan2(a.e, b.e), atan2(a.v, b.v)};
    }
  }

  /* The evaluation of a tree takes the operands of each node in the order of
     the operations, and gives the bounds of the operations in C++: random
     trees of every node, with shared nodes, whose value is computed as they
     are built. */
  void random_trees()
  {
    std::mt19937 rng(20260929);
    for (int i = 0; i < 3000; ++i) {
      const Modeled m = random_tree(rng, 2 + i % 5);
      interval got;
      const bool evaluated = evaluate_expr(m.e, got);
      check("random expression: the same bounds as the operations of C++", evaluated && same_bits(got, m.v),
            [&] {
              std::ostringstream o;
              o << m.e << " gave " << hex(got) << " rather than " << hex(m.v);
              return o.str();
            });
    }
  }

  /* An empty expression, at the bottom or beside other operands, is an error
     that leaves the operands of the other nodes where they are */
  void null_operands()
  {
    const expression none, one(1.0), two(2.0);
    interval x;
    check("null operand: the empty expression cannot be evaluated", !evaluate_expr(none, x));
    check("null operand: nor as the left operand of +", !evaluate_expr(none + one, x));
    check("null operand: nor as the right operand of *", !evaluate_expr(one * none, x));
    check("null operand: nor as the operand of a minus sign", !evaluate_expr(-none, x));
    check("null operand: nor as an argument of a function", !evaluate_expr(atan2(one, none), x));
    check("null operand: nor deep in a chain", !evaluate_expr(none + one + two + one * two - one / two, x));
    expression chain = none;
    for (int i = 0; i < 2000; ++i) {
      chain = chain + one;
    }
    check("null operand: nor at the bottom of a chain of 2000 nodes", !evaluate_expr(chain, x));

    // The same evaluator, after an error, evaluates the next expression
    expr_eval ev;
    none.get_root()->accept(ev);
    check("null operand: the evaluator sets its error", ev.error_occurred());
    (void)ev.result();
    ev.reset();
    (one + two).get_root()->accept(ev);
    const interval three = ev.result();
    check("null operand: and evaluates the next expression once reset",
          !ev.error_occurred() && same_bits(three, interval(3.0)), [&] { return hex(three); });
  }

  /* The stack of values of an evaluation is a stack that grows: 10000 values
     pushed on one that holds 4 come out in reverse */
  void eval_stack_growth()
  {
    gaol_core::eval_stack<int, 4> stack;
    for (int i = 0; i < 10000; ++i) {
      stack.push(i);
    }
    bool in_reverse = true;
    for (int i = 9999; i >= 0; --i) {
      in_reverse = (stack.pop() == i) && in_reverse;
    }
    check("eval_stack: 10000 values pushed on a stack of 4 come out in reverse", in_reverse);
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
  step("decimals");          decimals();
  step("built_expressions"); built_expressions();
  step("null_node_not_counted"); null_node_not_counted();
  step("eval_stack");        eval_stack_growth();
  step("null_operands");     null_operands();
  step("random_trees");      random_trees();
  step("deep_strings");      deep_strings();
  // The graceful checks of the stack come before the chains of 200000 nodes,
  // which crash the program when the stack is used as deep as they are
  step("stack_use");         stack_use();
  step("long_strings");      long_strings();
  step("long_built");        long_built();
// Commented out: the tests run no thread (GAOL v5)
// #if GAOL_TESTS_THREADS
//   step("reading_in_threads"); reading_in_threads();
// #endif
  {
    std::ostringstream text;
    text << static_expression;
    check("a static empty expression", static_expression.get_root() == the_null_expr && text.str() == ":null:",
          [&] { return text.str(); });
  }
  step("summary");
  const int status = summary();
  gaol::cleanup();
  return status;
}
