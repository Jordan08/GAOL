/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: intervals as text, and the names of IEEE 1788-2015.
 *
 * Part 1 reads intervals from text and writes them back. GAOL's reader takes
 * the literals of IEEE 1788-2015 (decimal and hexadecimal numbers, [a, b],
 * [a,] and [,], [entire], [empty], the uncertain form 3.56?1 of Table 12.1)
 * and whole expressions (1/3, sqrt(2), 2*pi), and encloses the real number a
 * text writes: this is how a program brings in a constant or a measurement
 * that no double holds. A malformed text throws gaol::input_format_error,
 * where gaol_ieee1788::textToInterval() returns the empty set. A file of
 * intervals is read line by line, each line in a try block, so that one bad
 * line is reported and skipped. Where GAOL is built without exceptions
 * (configure --disable-exceptions, meson -Denable-exception=false), its
 * reader ends the program on such a text, and sections 1.2 and 1.3 read with
 * gaol_ieee1788::textToInterval(). Written in decimal, the bounds are rounded
 * outward, so that the text still encloses the interval; the hexadecimal
 * text of exact_string() reads back bit for bit. Part 2 writes one step of a
 * contractor (the HC4-revise of 2 x^2 in c over x, as IBEX's and Codac's
 * contractors do it) twice: under GAOL's names, and in a function that opens
 * gaol_ieee1788, under the names of the standard (numsToInterval, pown,
 * mulRev, sqrRev, intersection, isMember, subset, interior, disjoint,
 * precedes, intervalToExact...); the two give the same intervals. It shows
 * where the two sets of names differ: the pow of the standard, exp(y log x),
 * is not defined for x < 0, so that pow([-4, -1], 2) is empty there and
 * [1, 16] in gaol, the integer power being pown; mulRev(b, c, x) is
 * div_rel(c, b, x); inf and sup of the empty set are +oo and -oo. A program
 * opens one of the two namespaces, never both, which would make pow
 * ambiguous, or silently GAOL's for an int exponent. Every claim is
 * checked: the texts read contain the values mpmath computed, the round
 * trips are exact, and the results of the two parts are those stated. It
 * follows Clauses 9 to 13 of IEEE 1788-2015, the chapter "Input/output" of
 * GAOL's manual (manual/v5/gaol.tex) and "The names of IEEE 1788-2015" in
 * doc/using.md.
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
#include <sstream>
#include <string>

#include <gaol/gaol.h>

/* No using-directive here, at the level of the file: each function opens the
   namespace whose names it uses, gaol for Part 1 and gaol_ieee1788 for Part 2.
   With both open, pow(x, y) and pow(x, 2.0) would be ambiguous, pow(x, 2)
   would silently be GAOL's (its overload for an int matches better), and
   inf(x) would be ambiguous with a constant inf of the program's own. */

namespace {

  /* Reference values, computed with mpmath at 60 digits and cut at 45. A
     string is read into the two doubles around the number it writes; each
     string being within 1e-44 of the real number, it has the same two doubles
     around it, and an enclosure of the real number contains them. */
  const gaol::interval sqrt2_ref = gaol::textToInterval("1.41421356237309504880168872420969807856967188");
  const gaol::interval two_pi_ref = gaol::textToInterval("6.28318530717958647692528676655900576839433880");

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
  // proves, which holds is the check of
  void show(const std::string& what, const std::string& value, const std::string& claim, bool holds)
  {
    std::cout << "  " << std::left << std::setw(29) << what << std::setw(48) << value << claim << '\n';
    check(holds, what + ": " + claim);
  }

  // An interval is converted to its text first (operator std::string writes
  // what operator<< writes), and the text is padded, to align the columns
  void show(const std::string& what, const gaol::interval& x, const std::string& claim, bool holds)
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
     same for the right bound. The empty set satisfies every certainly
     relation, having no point to contradict it, hence the test first. */
  bool contains_ratio(const gaol::interval& x, double p, double q)
  {
    return !x.is_empty() && gaol::interval(x.left()) * q <= p && gaol::interval(x.right()) * q >= p;
  }

  // The same bounds, bit for bit: the same doubles, with the same signs (a
  // bound 0 may be -0)
  bool same_bits(const gaol::interval& a, const gaol::interval& b)
  {
    if (a.is_empty() || b.is_empty()) {
      return a.is_empty() && b.is_empty();
    }
    return a.set_eq(b) && std::signbit(a.left()) == std::signbit(b.left())
           && std::signbit(a.right()) == std::signbit(b.right());
  }

  // ==========================================================================
  // Part 1: reading and writing intervals, under GAOL's names
  // ==========================================================================

  // Reads text as a program does, and shows what the interval read proves
  template <class Holds>
  void read_text(const char* text, const std::string& claim, Holds holds)
  {
    const gaol::interval x = gaol::textToInterval(text);
    show(std::string("\"") + text + "\"", x, claim, holds(x));
  }

  void reading()
  {
    using namespace gaol;
    const double oo = std::numeric_limits<double>::infinity();

    std::cout << "1.1 gaol::textToInterval(\"...\") encloses the number the text writes\n";
    read_text("0.1", "encloses 1/10", [](const interval& x) { return contains_ratio(x, 1, 10); });
    read_text("[1, 2]", "the bounds 1 and 2", [](const interval& x) { return x.set_eq(interval(1, 2)); });
    // The text is an expression, computed with intervals
    read_text("1/3", "encloses 1/3", [](const interval& x) { return contains_ratio(x, 1, 3); });
    read_text("sqrt(2)", "contains sqrt(2)", [](const interval& x) { return x.set_contains(sqrt2_ref); });
    read_text("2*pi", "contains 2 pi", [](const interval& x) { return x.set_contains(two_pi_ref); });
    // pi here is the real number, enclosed: sin(pi) contains 0, where
    // std::sin(M_PI) is about 1.2e-16, M_PI being a double near pi
    read_text("sin(pi)", "contains sin(pi) = 0", [](const interval& x) { return x.set_contains(0.0); });
    // The uncertain form (IEEE 1788, 12.11 and Table 12.1): a radius in units
    // of the last digit, the way measurements are written; an exponent may
    // follow
    read_text("3.56?1", "3.56 +/- 0.01: encloses [3.55, 3.57]", [](const interval& x) {
      return contains_ratio(x, 355, 100) && contains_ratio(x, 357, 100) && x.left() > 3.54 && x.right() < 3.58;
    });
    read_text("3.56?1e3", "(3.56 +/- 0.01) 10^3: [3550, 3570]",
              [](const interval& x) { return x.set_eq(interval(3550, 3570)); });
    // A bound left out is infinite
    read_text("[1,]", "[1, +oo)", [oo](const interval& x) { return x.set_eq(interval(1, oo)); });
    read_text("[,]", "the whole line", [](const interval& x) { return x.is_entire(); });
    read_text("[entire]", "the whole line", [](const interval& x) { return x.is_entire(); });
    read_text("[empty]", "the empty set", [](const interval& x) { return x.is_empty(); });
    // Hexadecimal: 0x1.8 is 1.5, times 2^1: a double, read exactly
    read_text("0x1.8p1", "exactly 3", [](const interval& x) { return x.set_eq(interval(3.0)); });

    /* A text that is no interval throws input_format_error, which derives
       from std::exception; its explanation() says what was wrong. The
       reader of gaol_ieee1788 returns the empty set instead, as a constructor
       of the standard does for an invalid input (12.1.3). The standard also
       signals UndefinedOperation then, and GAOL has neither this signal nor
       the decorations: a malformed text cannot be told from "[empty]". */
    std::cout << "1.2 A malformed text\n";
#if GAOL_EXCEPTIONS_ENABLED
    bool thrown = false;
    try {
      const interval x = gaol::textToInterval("[1, 2");
      std::cout << "  \"[1, 2\" read as " << x << '\n';
    } catch (const input_format_error& e) {
      thrown = true;
      std::cout << "  \"[1, 2\": input_format_error: " << e.explanation() << '\n';
    }
    check(thrown, "\"[1, 2\" throws input_format_error");
#else
    // Without exceptions, gaol::textToInterval("[1, 2") ends the program
    std::cout << "  GAOL built without exceptions: gaol::textToInterval would end the program\n";
#endif
    show("textToInterval(\"[1, 2\")", gaol_ieee1788::textToInterval("[1, 2"), "of gaol_ieee1788: nothing thrown",
         gaol_ieee1788::textToInterval("[1, 2").is_empty());
  }

  void reading_a_file()
  {
    using namespace gaol;

    /* Four measurements of the same resistance R, in ohms, one per line, in
       the forms above, with a comment, a blank line and a line with an error.
       The robust way to read them: std::getline, then
       gaol::textToInterval(line) in a try block of its own, so that a bad
       line is reported with its number and skipped. The handler takes
       gaol_exception, the base of GAOL's exceptions: a text can also be
       refused with an invalid_action_error, "nth_root(8, 1.5)" for one. A
       std::ifstream is read the same way as this stream. */
    std::istringstream file("# R in ohms, measured four times\n"
                            "[99.5, 100.5]\n"
                            "100.2?5\n"
                            "\n"
                            "[99.8, 100.9\n"
                            "99.9 + [-0.4, 0.4]\n");
    std::cout << "1.3 A file of measurements, read line by line\n";
    interval r;  // [-oo, +oo]: nothing known yet
    int number = 0, measurements = 0, errors = 0;
    std::string line;
    while (std::getline(file, line)) {
      ++number;
      if (line.empty() || line[0] == '#') {
        continue;
      }
      const std::string where = "line " + std::to_string(number) + ": " + line;
#if GAOL_EXCEPTIONS_ENABLED
      try {
        const interval m = gaol::textToInterval(line);
        // Each measurement holds R: so does their intersection
        r &= m;
        ++measurements;
        show(where, m, "read: a bounded interval", m.is_common_interval());
      } catch (const gaol_exception& e) {
        ++errors;
        std::cout << "  " << std::left << std::setw(29) << where << "skipped: " << e.explanation() << '\n';
      }
#else
      /* Without exceptions, gaol::textToInterval would end the program on
         the bad line: the reader of gaol_ieee1788 returns the empty set for
         a text that is no interval. It still ends the program on a call it
         refuses, as pown([2, 5], 2.5) (issue #98). */
      const interval m = gaol_ieee1788::textToInterval(line);
      if (m.is_empty()) {
        ++errors;
        std::cout << "  " << std::left << std::setw(29) << where << "skipped: not an interval\n";
      } else {
        r &= m;
        ++measurements;
        show(where, m, "read: a bounded interval", m.is_common_interval());
      }
#endif
    }
    check(measurements == 3 && errors == 1, "three measurements read, one line skipped");
    // [99.5, 100.5] & [99.7, 100.7] & [99.5, 100.3]: R is in [99.7, 100.3]
    show("R, in all three", r, "[99.7, 100.3], rounded outward",
         contains_ratio(r, 997, 10) && contains_ratio(r, 1003, 10) && r.left() > 99.69 && r.right() < 100.31);
  }

  void writing()
  {
    using namespace gaol;

    std::cout << "1.4 Writing: decimal rounded outward, or exact\n";
    const interval third = gaol::textToInterval("1/3");
    show("1/3 (16 digits by default)", third, "read back, still encloses 1/3",
         contains_ratio(gaol::textToInterval(std::string(third)), 1, 3));
    // interval::precision(n) chooses the number of digits, for every output
    // of intervals, and returns the previous one to set it back. The text
    // still encloses the interval: read back, it contains 1/3.
    const std::streamsize digits = interval::precision(5);
    const std::string five = third;
    interval::precision(digits);
    show("1/3 with 5 digits", five, "read back, still encloses 1/3",
         contains_ratio(gaol::textToInterval(five), 1, 3));

    // The hexadecimal format is exact. The format is global, as the
    // precision: set it back after use. exact_string() gives the same text
    // without touching the format.
    interval::format(interval_format::hexa);
    const std::string hexa = third;
    interval::format(interval_format::bounds);
    show("1/3 in interval_format::hexa", hexa, "the same as exact_string()", hexa == exact_string(third));

    // exact_string() reads back as the same doubles: the way to save an
    // interval and read it again, in a file or between programs
    const interval saved[] = { third, interval(0.1), interval::pi(), interval(-0.0, 1.0),
                               interval(1, std::numeric_limits<double>::infinity()), interval::emptyset() };
    bool exact = true;
    for (const interval& x : saved) {
      exact = exact && same_bits(gaol::textToInterval(exact_string(x)), x);
    }
    show("exact_string(pi)", exact_string(interval::pi()), "reads back as the same doubles",
         same_bits(gaol::textToInterval(exact_string(interval::pi())), interval::pi()));
    show("the same round trip for", std::string("1/3, 0.1, [-0, 1], [1, +oo), [empty]"), "bit for bit", exact);
    // A point is written [a], as in decimal, and a zero [0x0p+0] whatever the
    // signs of its bounds: [-0, 0] reads back as the same set, {0}
    const interval zero(-0.0, 0.0);
    show("exact_string([-0, 0])", exact_string(zero), "the set {0}, read back as [0, 0]",
         gaol::textToInterval(exact_string(zero)).set_eq(zero));
  }

  // ==========================================================================
  // Part 2: the same computation under GAOL's names and the standard's
  // ==========================================================================

  /* One step of a contractor: the constraint 2 x^2 in c narrows the interval
     x, as IBEX's and Codac's HC4-revise does for every constraint. The
     expression is evaluated forward, t = x^2 and u = 2 t, u is intersected
     with c, and the backward pass goes up the expression again with the
     reverse functions: the t of t with 2 t in u, then the x of x with x^2 in
     t. The points removed from x cannot satisfy the constraint, and an empty
     x proves that no point of x does. */
  struct Revise {
    gaol::interval power;  // pow(x, 2), which differs between the two namespaces
    gaol::interval t;      // x^2 once narrowed
    gaol::interval x;      // x narrowed
  };

  // Under GAOL's names
  Revise revise_gaol(double lo, double hi, const char* c_text)
  {
    using namespace gaol;
    const interval x0(lo, hi), c = gaol::textToInterval(c_text);
    const interval power = pow(x0, 2);  // GAOL's pow: the integer power, x < 0 too
    interval t = power;
    interval u = 2.0 * t;
    u &= c;
    t = div_rel(u, interval(2.0), t);  // the t in t with 2 t in u: div_rel(c, b, x)
    return { power, t, sqrt_rel(t, x0) };
  }

  void ieee1788()
  {
    using namespace gaol_ieee1788;

    std::cout << "2. The names of IEEE 1788-2015: using namespace gaol_ieee1788\n";
    std::cout << "   x in [-4, -1], contracted by 2 x^2 in c = [2, 4] (one HC4-revise)\n";
    // numsToInterval, the constructor from two numbers of the standard, takes
    // doubles, as interval(l, r) does.
    const interval x0 = numsToInterval(-4, -1);
    const interval c = numsToInterval(2, 4);
    const interval two = numsToInterval(2, 2);

    interval t = pown(x0, 2);  // the integer power of the standard
    const interval u = intersection(mul(two, t), c);
    t = mulRev(two, u, t);     // mulRev(b, c, x): the x in x with b x in c
    const interval x = sqrRev(t, x0);
    const Revise g = revise_gaol(-4, -1, "[2, 4]");

    // pow(x, y) of the standard is exp(y log x): not defined for x < 0
    show("pown(x, 2)", pown(x0, 2), "x^2 for an integer exponent", pown(x0, 2).set_eq(numsToInterval(1, 16)));
    show("pow(x, 2)", pow(x0, 2), "the standard's pow: none for x < 0", pow(x0, 2).is_empty());
    show("gaol::pow(x, 2)", g.power, "GAOL's pow: pown for an integer", g.power.set_eq(numsToInterval(1, 16)));
    show("t = mulRev(2, u, pown(x, 2))", t, "same as gaol's div_rel(u, 2, t)",
         t.set_eq(numsToInterval(1, 2)) && equal(t, g.t));
    show("x = sqrRev(t, x)", x, "[-sqrt(2), -1], as gaol's sqrt_rel",
         equal(x, g.x) && isMember(-1.0, x) && subset(-sqrt2_ref, x) && x.left() > -1.5);

    // The boolean functions, on the x found, in the standard's names
    show("isMember(-1.2, x)", isMember(-1.2, x), "-1.2 is in x", isMember(-1.2, x));
    show("isMember(-1.5, x)", isMember(-1.5, x), "-1.5 < -sqrt(2) is not", !isMember(-1.5, x));
    show("subset(x, [-4, -1])", subset(x, x0), "x only narrowed", subset(x, x0));
    show("interior(x, [-4, -1])", interior(x, x0), "both have the bound -1", !interior(x, x0));
    show("disjoint(x, [0, 1])", disjoint(x, numsToInterval(0, 1)), "no common point",
         disjoint(x, numsToInterval(0, 1)));
    // precedes is <= for every point, strictPrecedes < (GAOL's <= and <)
    show("precedes(x, [-1, 0])", precedes(x, numsToInterval(-1, 0)), "each point of x <= each of [-1, 0]",
         precedes(x, numsToInterval(-1, 0)));
    show("strictPrecedes(x, [-1, 0])", strictPrecedes(x, numsToInterval(-1, 0)), "-1 is in both",
         !strictPrecedes(x, numsToInterval(-1, 0)));
    // The exact text of the standard (13.4): GAOL's exact_string()
    const std::string text = intervalToExact(x);
    show("intervalToExact(x)", text, "read back bit for bit", same_bits(exactToInterval(text), x));

    // With c = [40, 50], 2 x^2 <= 32 < 40 on x: the constraint empties x
    const interval c2 = numsToInterval(40, 50);
    interval t2 = pown(x0, 2);
    t2 = mulRev(two, intersection(mul(two, t2), c2), t2);
    const interval x2 = sqrRev(t2, x0);
    std::cout << "   the same with c = [40, 50]\n";
    show("x = sqrRev(t, x)", x2, "proof: no x in [-4, -1] fits",
         isEmpty(x2) && revise_gaol(-4, -1, "[40, 50]").x.is_empty());
    // inf and sup of the empty set are +oo and -oo (Table 10.2), where
    // GAOL's left() and right() are NaN
    show("inf(x), sup(x)", std::to_string(inf(x2)) + ", " + std::to_string(sup(x2)), "+oo and -oo; left() is NaN",
         std::isinf(inf(x2)) && inf(x2) > 0 && std::isinf(sup(x2)) && sup(x2) < 0 && std::isnan(x2.left()));
  }

} // namespace

int main()
{
  std::cout << "Intervals as text, and the names of IEEE 1788-2015 (GAOL v5)\n";
  reading();
  reading_a_file();
  writing();
  ieee1788();

  // The last use of GAOL: cleanup() sets back the rounding direction the
  // program started with
  gaol::cleanup();

  if (failures != 0) {
    return EXIT_FAILURE;
  }
  std::cout << "Every claim above was checked.\n";
  return 0;
}
