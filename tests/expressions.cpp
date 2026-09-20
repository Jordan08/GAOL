/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: the intervals built from a string.
 *
 * interval("...") lexes the string, parses it into the tree of
 * gaol/gaol_expression.h, and evaluates that tree (gaol_expr_eval.h,
 * gaol_expr_visitor.h). This test goes through every node of the tree and
 * every way the string can be wrong, so that the parts of GAOL the other
 * tests never reach are run too:
 *
 * - the numbers, in every form the lexer takes (decimal, exponent,
 *   hexadecimal, the bounds given apart);
 * - the operators and the functions, alone and nested;
 * - the strings the parser refuses, which have to raise an exception rather
 *   than give an interval.
 *
 * The value is compared with the same computation written in C++, which the
 * other tests check against the exact results: here what is tested is the
 * lexer, the parser and the evaluation of the tree, not the operations.
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/gaol_expr_eval.h"

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // The interval of a string, and the same interval computed in C++
  void same(const std::string& text, const interval& expected)
  {
    interval got;
    bool threw = false;
    try {
      got = interval(text.c_str());
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
      const interval got = interval(text.c_str());
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
      const interval got = interval(text.c_str());
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
          interval("[0.1,0.1]", "[0.2,0.2]").left() <= 0.1
            && interval("[0.1,0.1]", "[0.2,0.2]").right() >= 0.2);
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
    // nested, and mixed with the operators
    same("exp(log([1,2]))", exp(log(interval(1.0, 2.0))));
    same("sin(cos(tan([0,1])))", sin(cos(tan(interval(0.0, 1.0)))));
    same("cos([0,1])+sin([0,1])*exp([0,1])",
         cos(interval(0.0, 1.0)) + sin(interval(0.0, 1.0)) * exp(interval(0.0, 1.0)));
    same("-exp([0,1])", -exp(interval(0.0, 1.0)));
    same("log(exp([1,2])*exp([1,2]))", log(exp(interval(1.0, 2.0)) * exp(interval(1.0, 2.0))));
    // outside the domain: the empty set rather than an exception
    same("log([-2,-1])", log(interval(-2.0, -1.0)));
    same("sqrt([-2,-1])", sqrt(interval(-2.0, -1.0)));
    same("acos([2,3])", acos(interval(2.0, 3.0)));
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
    refused("abs([-2,1])");    // nor abs
    refused("atan2([1,2])");   // atan2 takes two arguments
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
        got = interval(t);
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

    // the operators that change the expression in place
    expression acc = expression(interval(1.0, 1.0));
    acc += x;
    acc -= d;
    acc *= y;
    expr_eval ev;
    acc.get_root()->accept(ev);
    const interval got = ev.result();
    const interval expected =
      ((interval(1.0) + interval(1.0, 2.0)) - interval(2.5)) * interval(3.0, 4.0);
    check("built expression: +=, -= and *=",
          got.left() == expected.left() && got.right() == expected.right(),
          [&] {
            std::ostringstream o;
            o << got << " rather than " << expected;
            return o.str();
          });

    // the empty expression, whose evaluation is an error
    expression empty;
    std::ostringstream o;
    o << empty;
    check("built expression: the empty one is printed", true);
  }

}

int main()
{
  gaol::init();
  numbers();
  operators();
  functions();
  wrong_strings();
  decimals();
  built_expressions();
  const int status = summary();
  gaol::cleanup();
  return status;
}
