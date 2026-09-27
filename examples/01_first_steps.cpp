/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: a first program with intervals.
 *
 * The program builds intervals in every way a program does (from two bounds,
 * from one double, from text, as the hull of two numbers, from the constants
 * of GAOL), computes with them (arithmetic mixed with doubles, elementary
 * functions), reads their numbers (bounds, midpoint, width, radius,
 * mignitude, magnitude), uses them as sets (hull, intersection, membership,
 * subset, interior, disjointness, equality) and compares them with the
 * certainly relations. An interval stands for an unknown real number: every
 * result encloses all the values the computation can take, so that a program
 * can prove things about numbers it never holds exactly, 1/10 or pi. The
 * example shows the few rules of GAOL that surprise a newcomer: a decimal
 * constant is written as a string, "0.1", the double 0.1 not being 1/10; the
 * literal 0 is ambiguous where 0.0 is not; interval(2, 1) is the empty set;
 * x < y means "for every point of x and every point of y", and there is no
 * ==; mid() is an interval where midpoint() is a double. Each line of the
 * output says what it proves, and the program checks it: the enclosures
 * contain the values mpmath computed to 45 digits, and the set relations are
 * the ones stated. It follows IBEX's examples/doc-arithmetic.cpp (the
 * constructors and constants of Interval), the page on the Interval class of
 * Codac's manual (doc/manual/manual/intervals/src.cpp, whose example of sin,
 * exp and intersection it repeats) and the chapter "An overview of GAOL" of
 * GAOL's manual (manual/v5/gaol.tex).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include <gaol/gaol.h>

using namespace gaol;

namespace {

  const double inf = std::numeric_limits<double>::infinity();

  /* Reference values, computed with mpmath at 60 digits and cut at 45. A
     string is read by GAOL into the two doubles around the number it writes;
     each string being within 1e-44 of the real number, it has the same two
     doubles around it, and an enclosure of the real number contains them. */
  const interval pi_ref("3.14159265358979323846264338327950288419716940");
  const interval half_pi_ref("1.57079632679489661923132169163975144209858470");
  const interval e_ref("2.71828182845904523536028747135266249775724709");
  const interval sqrt2_ref("1.41421356237309504880168872420969807856967188");
  const interval exp_half_pi_ref("4.81047738096535165547303566670383312639017087");

  int failures = 0;

  // Every claim the output makes is checked here: one that does not hold is
  // reported, and the program then fails, so that ctest reports it too
  void check(bool holds, const std::string& claim)
  {
    if (!holds) {
      std::cout << "FAILED: " << claim << '\n';
      ++failures;
    }
  }

  // One line of the output: what was computed, its value, and what the line
  // proves, which holds is the check of. An interval is converted to its
  // text first (operator std::string writes what operator<< writes), and the
  // text is padded, to align the columns.
  void show(const std::string& what, const std::string& value, const std::string& claim, bool holds)
  {
    std::cout << "  " << std::left << std::setw(29) << what << std::setw(45) << value << claim << '\n';
    check(holds, what + ": " + claim);
  }

  void show(const std::string& what, const interval& x, const std::string& claim, bool holds)
  {
    show(what, std::string(x), claim, holds);
  }

  void show(const std::string& what, bool b, const std::string& claim, bool holds)
  {
    show(what, std::string(b ? "true" : "false"), claim, holds);
  }

  /* True when the rational p/q (q > 0) is in x, proved with GAOL itself: the
     product q*left() is computed as an interval, whose upper bound is at least
     the exact product, so that "interval <= p" proves q*left() <= p; and the
     same for the right bound. The test of emptiness comes first: the empty
     set satisfies every certainly relation, having no point that could
     contradict it. */
  bool contains_ratio(const interval& x, double p, double q)
  {
    return !x.is_empty() && interval(x.left()) * q <= p && interval(x.right()) * q >= p;
  }

} // namespace

int main()
{
  std::cout << "A first program with intervals (GAOL v5)\n";

  // ------------------------------------------------------------------------
  std::cout << "1. Building intervals\n";

  // From two bounds, from one double (a point interval) and from text. The
  // literal 0 is both a number and a null pointer, which the constructor
  // from a string takes too: interval(0), interval(0, 0), x |= 0 and x < 0
  // do not compile, being ambiguous; interval(0.0), x |= 0.0 and x < 0.0 do.
  show("interval(1, 2)", interval(1, 2), "the real numbers from 1 to 2",
       interval(1, 2).left() == 1.0 && interval(1, 2).right() == 2.0);
  show("interval(2.0)", interval(2.0), "a point interval: one double", interval(2.0).is_a_double());
  show("interval(\"[1, 2]\")", interval("[1, 2]"), "read from text", interval("[1, 2]").set_eq(interval(1, 2)));

  /* The double 0.1 is not 1/10, which has no finite binary expansion: the
     compiler rounded it to the nearest double before GAOL saw it, and
     interval(0.1) holds that double only. The string "0.1" is read by GAOL,
     which encloses the decimal number it writes between two doubles. The
     hexadecimal text of exact_string() shows the bounds exactly, where 16
     decimal digits would print both intervals alike. */
  const interval tenth_double(0.1), tenth("0.1");
  // interval(0.1) is one double d; 10*d is not a double (the interval product
  // is not a point), so it is not 1, and d is not 1/10
  show("interval(0.1)", exact_string(tenth_double), "one double: misses 1/10",
       tenth_double.is_a_double() && !(tenth_double * 10.0).is_a_double());
  show("interval(\"0.1\")", exact_string(tenth), "two doubles: encloses 1/10", contains_ratio(tenth, 1, 10));

  // Bounds in the wrong order give the empty set, as numsToInterval does in
  // IEEE 1788: the hull of two points is the interval between two numbers
  // whose order the program does not know
  const double a = 3.5, b = -1.25;
  show("interval(a, b)", interval(a, b), "a = 3.5 > b = -1.25: empty", interval(a, b).is_empty());
  show("interval(a) | interval(b)", interval(a) | interval(b), "the hull: from b to a",
       (interval(a) | interval(b)).set_eq(interval(b, a)));

  // The constants of GAOL enclose the real numbers they are named after
  show("interval::pi()", interval::pi(), "contains pi", interval::pi().set_contains(pi_ref));
  show("interval::emptyset()", interval::emptyset(), "no real number", interval::emptyset().is_empty());
  show("interval()", interval(), "every real number", interval().is_entire());
  show("interval(0.0)", interval(0.0), "zero; interval(0) would not compile",
       interval(0.0).set_eq(interval::zero()));

  // ------------------------------------------------------------------------
  std::cout << "2. Arithmetic, doubles mixed in: x = [1, 2], y = [-1, 2]\n";

  const interval x(1, 2), y(-1, 2);
  // A double operand is a point interval: 2.0 and 3.0 are exact, and the
  // result is rounded outward, so that it holds x/3 for every x of [1, 2]
  show("2.0 * x", 2.0 * x, "exact: [2, 4]", (2.0 * x).set_eq(interval(2, 4)));
  show("x / 3.0", x / 3.0, "contains 1/3 and 2/3", contains_ratio(x / 3.0, 1, 3) && contains_ratio(x / 3.0, 2, 3));
  // Dividing by an interval holding 0: 1/y takes the values of (-oo, -1]
  // and of [1/2, +oo), whose hull is the whole line. It is no error.
  show("1.0 / y", 1.0 / y, "0 in y: every real number", (1.0 / y).is_entire());
  show("1.0 / interval(0.0, 2.0)", 1.0 / interval(0.0, 2.0), "1/x for x in (0, 2]: [1/2, +oo)",
       (1.0 / interval(0.0, 2.0)).set_eq(interval(0.5, inf)));
  // Each occurrence of x varies on its own: x - x is {a - b : a, b in x},
  // not {0}. This is the dependency problem of interval arithmetic.
  show("x - x", x - x, "not [0, 0]: the dependency problem", (x - x).set_eq(interval(-1, 1)));

  // ------------------------------------------------------------------------
  std::cout << "3. Elementary functions\n";

  show("sqrt(interval(2.0))", sqrt(interval(2.0)), "contains sqrt(2)", sqrt(interval(2.0)).set_contains(sqrt2_ref));
  show("exp(interval(1.0))", exp(interval(1.0)), "contains e", exp(interval(1.0)).set_contains(e_ref));
  // Only the points of the argument in the domain count (IEEE 1788): sqrt
  // drops [-1, 0), log(0) is the bound -oo, and a set outside is empty
  show("sqrt(interval(-1.0, 4.0))", sqrt(interval(-1.0, 4.0)), "sqrt of [0, 4] only",
       sqrt(interval(-1.0, 4.0)).set_eq(interval(0.0, 2.0)));
  show("log(interval(0.0, 1.0))", log(interval(0.0, 1.0)), "log(0) is the bound -oo",
       log(interval(0.0, 1.0)).set_eq(interval(-inf, 0.0)));
  show("sqrt(interval(-2.0, -1.0))", sqrt(interval(-2.0, -1.0)), "no point in the domain",
       sqrt(interval(-2.0, -1.0)).is_empty());

  // Codac's example: the hull of [pi/2] and 0 (Codac writes x |= 0, which
  // is ambiguous here, 0 being a null pointer as well: write 0.0)
  interval h = interval::half_pi();
  h |= 0.0;
  show("h = half_pi() | 0.0", h, "contains 0 and pi/2", h.set_contains(0.0) && h.set_contains(half_pi_ref));
  show("sin(h)", sin(h), "the maximum 1 at pi/2 is kept", sin(h).set_eq(interval(0.0, 1.0)));
  show("exp(h)", exp(h), "contains 1 and e^(pi/2)", exp(h).set_contains(1.0) && exp(h).set_contains(exp_half_pi_ref));
  show("sin(h) & exp(h)", sin(h) & exp(h), "their only common value: 1", (sin(h) & exp(h)).set_eq(interval(1.0)));

  // ------------------------------------------------------------------------
  std::cout << "4. The numbers of an interval, z = [-3, 2]\n";

  /* These are doubles. The program computes its own doubles rounded upward
     until gaol::cleanup() (GAOL sets the rounding direction for the whole
     program), and printing a double rounds it too: the values printed here
     are exact doubles with few digits, which print the same in any rounding
     direction. */
  const interval z(-3, 2);
  std::cout << "  left() = " << z.left() << ", right() = " << z.right() << ", width() = " << z.width()
            << ", rad() = " << z.rad() << ", mig() = " << z.mig() << ", mag() = " << z.mag() << '\n';
  check(z.left() == -3.0 && z.right() == 2.0 && z.width() == 5.0 && z.rad() == 2.5,
        "the bounds, width and radius of z");
  check(z.mig() == 0.0 && z.mag() == 3.0, "mig(z) = 0 (0 is in z) and mag(z) = 3");
  // midpoint() is a double; mid() is an interval, the enclosure of the exact
  // midpoint, which is what a proof evaluates a function at. Here they agree.
  std::cout << "  midpoint() = " << z.midpoint() << " (a double), mid() = " << z.mid() << " (an interval)\n";
  check(z.midpoint() == -0.5 && z.mid().set_eq(interval(-0.5)), "the midpoint of z is -0.5");
  // Between two consecutive doubles, the exact midpoint is no double:
  // midpoint() rounds it, mid() encloses it
  const interval t(1.0, std::nextafter(1.0, 2.0));
  show("t = [1, next double]", exact_string(t), "the smallest interval above 1", t.is_canonical() && !t.is_a_double());
  std::cout << "  t.midpoint() = " << t.midpoint() << ", a double of t; t.mid() = t: 1 + 2^-53 is no double\n";
  check(t.set_contains(t.midpoint()) && t.mid().set_eq(t), "midpoint() is in t and mid() is t");

  // ------------------------------------------------------------------------
  std::cout << "5. Sets: u = [1, 3], v = [2, 4], w = [5, 6], r = [1, 2]\n";

  const interval u(1, 3), v(2, 4), w(5, 6), r(1, 2);
  show("u | v", u | v, "the hull", (u | v).set_eq(interval(1, 4)));
  show("u | w", u | w, "the hull adds (3, 5): not the union", (u | w).set_eq(interval(1, 6)));
  show("u & v", u & v, "the intersection", (u & v).set_eq(interval(2, 3)));
  show("u & w", u & w, "empty: u and w are disjoint", (u & w).is_empty());
  show("u.set_contains(2.5)", u.set_contains(2.5), "2.5 is in u", u.set_contains(2.5));
  show("u.set_strictly_contains(2.0)", u.set_strictly_contains(2.0), "2 is in the interior (1, 3)",
       u.set_strictly_contains(2.0));
  show("r.set_leq(u)", r.set_leq(u), "r is a subset of u", r.set_leq(u));
  show("u.set_strictly_contains(r)", u.set_strictly_contains(r), "r touches the bound 1 of u",
       !u.set_strictly_contains(r));
  show("u.set_disjoint(w)", u.set_disjoint(w), "no common point", u.set_disjoint(w));
  // There is no operator ==, whose meaning would be unclear (the same sets?
  // the same unknown number?): the equality of two sets is set_eq()
  show("(u & v).set_eq(r + 1.0)", (u & v).set_eq(r + 1.0), "the same set; there is no ==", (u & v).set_eq(r + 1.0));

  // ------------------------------------------------------------------------
  std::cout << "6. Certainly relations: true when true for every point\n";

  /* x < y is true when a < b for every a of x and every b of y, which a
     program can then rely on whatever the unknown numbers are. When it is
     false, nothing follows: !(u < v) does not make u >= v. And the empty
     set, having no point, satisfies them all: test is_empty() first. */
  show("[1, 2] < [3, 4]", interval(1, 2) < interval(3, 4), "each a below each b", interval(1, 2) < interval(3, 4));
  show("[1, 2] < [2, 3]", interval(1, 2) < interval(2, 3), "a = b = 2 is a counterexample",
       !(interval(1, 2) < interval(2, 3)));
  show("[1, 2] <= [2, 3]", interval(1, 2) <= interval(2, 3), "a <= b for each a and b",
       interval(1, 2) <= interval(2, 3));
  show("u < v, u >= v", std::string(u < v ? "true" : "false") + ", " + (u >= v ? "true" : "false"),
       "overlapping: neither holds", !(u < v) && !(u >= v));
  const interval empty = interval::emptyset();
  show("empty < 0.0, empty > 0.0",
       std::string(empty < 0.0 ? "true" : "false") + ", " + (empty > 0.0 ? "true" : "false"),
       "no point to contradict them", empty < 0.0 && empty > 0.0);

  // ------------------------------------------------------------------------
  std::cout << "7. Output: interval::precision(n) digits, rounded outward\n";

  // interval::precision(n) sets the number of digits of the bounds, for all
  // the output of intervals, and returns the previous one, to set it back.
  // The bounds are rounded outward, so that the text printed still encloses
  // the interval: read back, it contains pi.
  const std::streamsize digits = interval::precision(5);
  const std::string pi_text = interval::pi();
  show("pi with 5 digits", pi_text, "read back, still contains pi", interval(pi_text.c_str()).set_contains(pi_ref));
  interval::precision(digits);

  // The last use of GAOL: cleanup() sets back the rounding direction the
  // program started with, so that the doubles computed from here on are
  // rounded to nearest again
  gaol::cleanup();

  if (failures != 0) {
    return EXIT_FAILURE;
  }
  std::cout << "Every claim above was checked.\n";
  return 0;
}
