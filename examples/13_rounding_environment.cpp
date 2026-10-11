/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: intervals and the rest of the program, the rounding
 * direction.
 *
 * GAOL computes both bounds of its intervals rounded upward, the lower one
 * negated, so that each bound is on the right side of the real result. The
 * rounding direction it sets is the processor's, which the rest of the
 * program shares: unless GAOL is built with GAOL_PRESERVE_ROUNDING, GAOL
 * sets it upward when it initializes itself, before main(), and each of its
 * operations sets it upward again when it is not, until gaol::cleanup().
 * The example shows what this means for a program that also computes with
 * doubles. Its own quotients, std::lrint(), printf(), strtod(), sums, text
 * round trips and error-free transformations follow the direction (section 1,
 * the table of doc/using.md). GAOL's intervals do not depend on the direction the
 * program leaves, which it may therefore change freely (section 2). A small
 * RAII guard, nearest_scope, runs a block of the program's own double code
 * to nearest, with the two cautions it needs (section 3). A std::thread
 * starts with the direction of main() and computes the same intervals
 * (section 4). gaol::cleanup() sets back the direction GAOL found when it
 * initialized itself, once only: a later operation of GAOL sets it upward
 * again (section 5). The program finds out at run time how the GAOL it runs
 * with is built, and checks each claim in both cases: the intervals contain
 * the values mpmath computed to 45 digits, are the same bit for bit
 * (exact_string()) in every direction and in the thread, and the directions
 * are the ones stated. It follows the sections "Initialization and cleanup"
 * and "The rounding direction" of doc/using.md and the section "The
 * rounding direction" of GAOL's manual (manual/v5/gaol.tex). The guard is
 * the scoped rounding object of Boost.Interval
 * (boost/numeric/interval/rounding.hpp, save_state); IBEX switches the
 * direction for its own double code with fpu_round_up(), fpu_round_down()
 * and fpu_round_near() (src/arithmetic/ibex_Interval.h, used in
 * src/arithmetic/ibex_InnerArith.cpp), which are GAOL's round_upward(),
 * round_downward() and round_nearest().
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cfenv>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gaol/gaol.h>

using namespace gaol;

namespace {

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
    std::cout << "  " << std::left << std::setw(32) << what << std::setw(44) << value << claim << '\n';
    check(holds, what + ": " + claim);
  }

  std::string direction_name(int direction)
  {
    switch (direction) {
    case FE_TONEAREST:
      return "to nearest";
    case FE_UPWARD:
      return "upward";
    case FE_DOWNWARD:
      return "downward";
    case FE_TOWARDZERO:
      return "toward zero";
    default:
      return "unknown";
    }
  }

  // The exact value of a double: %a writes its bits in hexadecimal, which no
  // rounding direction changes. Written in decimal with %g or <<, it would be
  // rounded, upward while GAOL is initialized with the GNU C library.
  std::string hex(double d)
  {
    char text[32];
    std::snprintf(text, sizeof text, "%a", d);
    return text;
  }

  /* The data of the program, in volatile memory: the compiler reads them when
     the program runs, where the source code reads them. It can then neither
     compute 1.0 / 3.0 when compiling, which it would round to nearest, nor
     reuse a quotient computed before a change of rounding direction. */
  volatile double one = 1.0;
  volatile double three = 3.0;
  volatile double two_point_three = 2.3;
  volatile double price = 2.675;   // the double 2.67499999999999982236431605997...
  volatile double pi_digits = 3.14159265358979;
  volatile double one_tenth = 0.1;
  volatile double addend_big = 1e20;
  volatile double addend_tiny = 1e-20;

  /* x, through volatile memory: x is computed where the source code computes
     it, before the change of rounding direction that follows. GCC does not
     see that std::fesetround() changes the result of a division (GCC bug
     34678, even with -frounding-math, which gaol.pc passes): it may compute
     after a change of direction a value that the source code computes before
     it, and reuse after the change a value computed before it. A write to
     volatile memory is made where the source code makes it, and what it
     writes is computed before. */
  template<class T>
  T keep(T x)
  {
    volatile T kept = x;
    return kept;
  }

  /*
    Rounds the doubles of the program to nearest in a block, and sets back
    the direction it found when the block ends, however it ends: the RAII
    idiom of std::lock_guard, and of the rounding objects of Boost.Interval.
    GAOL needs no such guard, each of its operations setting the direction
    upward itself when it is not. The program needs it for its own double
    code written for rounding to nearest: integer rounding with lrint(),
    decimal input and output, compensated sums, a library it calls.

    Two cautions, which section 3 shows:
    - An operation of GAOL in the block sets the direction upward again
      (unless GAOL is built with GAOL_PRESERVE_ROUNDING): the block holds the
      program's double code, not intervals.
    - The compiler may compute the block's doubles outside it (see keep()):
      the block reads its data from volatile memory, or gets them from calls,
      and writes its results through keep() or to volatile memory.

    std::fesetround() sets the direction of both units of an x86 processor,
    x87 and SSE, as GAOL does; with the GNU C library, std::fegetround()
    reads the x87 one only, which is enough as long as the program changes
    the direction with these functions or with GAOL's.
  */
  struct nearest_scope {
    const int saved = std::fegetround();

    nearest_scope() { std::fesetround(FE_TONEAREST); }
    ~nearest_scope() { std::fesetround(saved); }
    nearest_scope(const nearest_scope&) = delete;
    nearest_scope& operator=(const nearest_scope&) = delete;
  };

  /*
    A double written with 17 digits, which read back to nearest gives the
    same double. Written in a nearest_scope: with the GNU C library, the
    digits are rounded in the direction in effect.
  */
  std::string decimal(double d)
  {
    nearest_scope nearest;
    char text[40];
    std::snprintf(text, sizeof text, "%.17g", d);
    return text;
  }

  // Knuth's TwoSum: s = a + b, and the error e of the sum, with s + e = a + b
  // exactly when rounding to nearest
  void two_sum(double a, double b, double& s, double& e)
  {
    s = a + b;
    const double bb = s - a;
    e = (a - (s - bb)) + (b - bb);
  }

  /*
    Whether s + e is exactly a + b. To nearest, TwoSum gives the exact sum of
    two doubles as a pair, the same pair for the same sum: s + e is a + b
    when TwoSum of s and e gives the pair TwoSum of a and b does.
  */
  bool is_exact_sum(double a, double b, double s, double e)
  {
    bool same;
    {
      nearest_scope nearest;
      double s1, e1, s2, e2;
      two_sum(keep(a), keep(b), s1, e1);
      two_sum(keep(s), keep(e), s2, e2);
      same = keep(s1 == s2 && e1 == e2);
    }
    return same;
  }

  // Dekker's TwoProd: p = a * b, and the error e of the product, with p + e =
  // a * b exactly when rounding to nearest. From the split of the operands
  // (Veltkamp), which the directed roundings break as they break TwoSum
  void two_prod_dekker(double a, double b, double& p, double& e)
  {
    const double split = 134217729.0;   // 2^27 + 1
    const double ca = split*a - (split*a - a);
    const double cb = split*b - (split*b - b);
    p = a * b;
    e = ca*cb - p + ca*(b - cb) + (a - ca)*cb + (a - ca)*(b - cb);
  }

  /*
    Whether p + e is exactly a * b, in every rounding direction: the error of
    a rounded product is a double, which std::fma computes exactly however
    it rounds, so p + e is the product exactly when std::fma(a, b, -p) gives
    e back. The check runs to nearest, whatever the direction of the pair.
  */
  bool is_exact_product(double a, double b, double p, double e)
  {
    bool same;
    {
      nearest_scope nearest;
      same = keep(e == std::fma(keep(a), keep(b), -keep(p)));
    }
    return same;
  }

  /*
    The other rows of the table of doc/using.md: what the direction does to a
    stream, a sum, a text round trip and an error-free transformation. They
    are computed once as main() runs, and once in a nearest_scope, from data
    in memory and through keep(), so that the compiler cannot take a result
    of one computation for the other's.
  */
  struct Consequences {
    std::string stream;               // std::cout << std::setprecision(3) << pi
    double sum;                       // 0.1 added ten million times
    std::size_t changed;              // doubles that %.17g, then strtod, give back different
    bool exact_sum;                   // s + e is a + b for TwoSum(1e20, 1e-20)
    std::size_t inexact_pairs;        // pairs of doubles whose TwoSum is not exact
    std::size_t inexact_products;     // pairs whose TwoProd, from Dekker's splitting, is not exact
    std::size_t inexact_fma_products; // pairs whose TwoProd, from std::fma, is not exact
  };

  Consequences consequences(const std::vector<double>& values, const std::vector<std::pair<double, double>>& pairs,
                             const std::vector<std::pair<double, double>>& product_pairs)
  {
    Consequences c;
    std::ostringstream stream;
    stream << std::setprecision(3) << pi_digits;
    c.stream = stream.str();

    const double tenth = one_tenth;
    double sum = 0.0;
    for (long i = 0; i < 10000000; ++i) {
      sum += tenth;
    }
    c.sum = keep(sum);

    c.changed = 0;
    for (double x : values) {
      char text[40];
      std::snprintf(text, sizeof text, "%.17g", x);
      c.changed += std::strtod(text, nullptr) != x;
    }

    double s, e;
    two_sum(addend_big, addend_tiny, s, e);
    s = keep(s);
    e = keep(e);
    c.exact_sum = is_exact_sum(addend_big, addend_tiny, s, e);

    c.inexact_pairs = 0;
    for (const std::pair<double, double>& ab : pairs) {
      two_sum(ab.first, ab.second, s, e);
      s = keep(s);
      e = keep(e);
      c.inexact_pairs += !is_exact_sum(ab.first, ab.second, s, e);
    }

    /* TwoProd, the error of a product: Dekker's, from the split operands,
       which TwoSum's fate awaits (it is exact to nearest only), and the one
       of std::fma, exact in every direction: the error of a rounded product
       is a double, which std::fma computes however it rounds. That e is the
       error is what is_exact_product() checks, from std::fma to nearest. */
    c.inexact_products = 0;
    c.inexact_fma_products = 0;
    for (const std::pair<double, double>& ab : product_pairs) {
      double p, err;
      two_prod_dekker(ab.first, ab.second, p, err);
      p = keep(p);
      err = keep(err);
      c.inexact_products += !is_exact_product(ab.first, ab.second, p, err);
      p = ab.first * ab.second;
      err = std::fma(ab.first, ab.second, -p);
      p = keep(p);
      err = keep(err);
      c.inexact_fma_products += !is_exact_product(ab.first, ab.second, p, err);
    }
    return c;
  }

  /*
    Does this GAOL leave the rounding direction upward after its operations,
    as by default, or restore the one it found, as when built with
    GAOL_PRESERVE_ROUNDING? The program asks the library it runs with: one
    operation from the direction to nearest, and a look at the direction
    afterwards.
  */
  bool gaol_leaves_upward()
  {
    nearest_scope nearest;
    static_cast<void>(interval(1.0) / three);
    return std::fegetround() == FE_UPWARD;
  }

  /*
    A few intervals, each with what it must contain: the real number, as an
    interval of two decimals mpmath wrote at 45 digits around it, which GAOL
    reads rounded outward. Only exact doubles (1.0, 3.0, 1e22...) and
    decimal strings go in: a double the program computed, 1.0 / 3.0, would
    carry the direction it was computed in.
  */
  struct Computation {
    const char* what;
    interval (*compute)();
    const char* truth;
    const char* claim;
  };

  const Computation computations[] = {
    {"interval(1.0) / 3.0", [] { return interval(1.0) / 3.0; },
     "[0.333333333333333333333333333333333333333333333, 0.333333333333333333333333333333333333333333334]",
     "contains 1/3"},
    {"textToInterval(\"0.3\") * 10.0", [] { return textToInterval("0.3") * 10.0; }, "3",
     "contains 3: \"0.3\" encloses 3/10"},
    {"sqrt(interval(2.0))", [] { return sqrt(interval(2.0)); },
     "[1.414213562373095048801688724209698078569671875, 1.414213562373095048801688724209698078569671876]",
     "contains sqrt(2)"},
    {"exp(interval(1.0))", [] { return exp(interval(1.0)); },
     "[2.718281828459045235360287471352662497757247093, 2.718281828459045235360287471352662497757247094]",
     "contains e"},
    {"log(interval(10.0))", [] { return log(interval(10.0)); },
     "[2.302585092994045684017991454684364207601101488, 2.302585092994045684017991454684364207601101489]",
     "contains ln(10)"},
    {"sin(interval(1e22))", [] { return sin(interval(1e22)); },
     "[-0.852200849767188801772705893753029368261762151, -0.852200849767188801772705893753029368261762150]",
     "contains sin(10^22)"},
    // pow(x, n) of intervals is GAOL's, which "using namespace gaol" names
    {"pow(interval(-2.0, 1.0), 3)", [] { return pow(interval(-2.0, 1.0), 3); }, "[-8, 1]",
     "contains the range of x^3 on [-2, 1]"},
  };

  const std::size_t computation_count = sizeof computations / sizeof computations[0];

  std::vector<interval> compute_all()
  {
    std::vector<interval> results;
    for (const Computation& c : computations) {
      results.push_back(c.compute());
    }
    return results;
  }

  // The exact texts of intervals, which tell them apart bit for bit
  std::vector<std::string> exact_texts(const std::vector<interval>& xs)
  {
    std::vector<std::string> texts;
    for (const interval& x : xs) {
      texts.push_back(exact_string(x));
    }
    return texts;
  }

  // True when d is one of the two bounds of x
  bool is_bound_of(double d, const interval& x)
  {
    return d == x.left() || d == x.right();
  }

} // namespace

int main()
{
  // No gaol::init(): GAOL initialized itself before main(). The direction is
  // read first, before any operation of the program.
  const int at_start = std::fegetround();

  std::cout << "Intervals and the rest of the program: the rounding direction (GAOL v5)\n";

  const bool leaves_upward = gaol_leaves_upward();
#ifdef GAOL_PRESERVE_ROUNDING
  const bool built_to_preserve = true;
#else
  const bool built_to_preserve = false;
#endif
  /* The AVX-512 path of +, -, *, / and sqrt (GAOL_PREFER_AVX512, on a
     processor that has the instructions): it sets no direction, and the
     probe's division leaves the one the program set. The other operations
     leave the direction upward, as in the default build */
#if defined(GAOL_USING_SSE2_INSTRUCTIONS) && GAOL_USING_SSE2_INSTRUCTIONS \
    && defined(GAOL_HAVE_AVX512_TARGET) && GAOL_HAVE_AVX512_TARGET && GAOL_PREFER_AVX512
  const bool embedded_arithmetic = __builtin_cpu_supports("avx512f") != 0;
#else
  const bool embedded_arithmetic = false;
#endif
  std::cout << (leaves_upward ? "This GAOL leaves the direction upward after its operations (the default build)\n"
                              : embedded_arithmetic
                                  ? "This GAOL's +, -, *, / and sqrt set no rounding direction (GAOL_PREFER_AVX512);\n"
                                    "the other operations leave it upward\n"
                                  : "This GAOL restores the direction after each operation (GAOL_PRESERVE_ROUNDING)\n");
  check(leaves_upward == (!built_to_preserve && !embedded_arithmetic),
        "the behaviour agrees with GAOL_PRESERVE_ROUNDING and GAOL_PREFER_AVX512 in gaol_configuration.h");

  // ------------------------------------------------------------------------
  std::cout << "1. The program's own doubles in main(): as it runs, then in a nearest_scope\n";

  /* Each computation is made twice: in the direction main() starts with,
     then in a nearest_scope, as without GAOL. The values are pinned with
     keep(), so that each is computed in its own direction. */
  const double third = keep(one / three);
  const long rounded = keep(std::lrint(two_point_three));
  char printed[16];
  std::snprintf(printed, sizeof printed, "%.2f", price);
  const double read = std::strtod("0.3", nullptr);

  double third_nearest = 0.0;
  long rounded_nearest = 0;
  char printed_nearest[16];
  double read_nearest = 0.0;
  {
    nearest_scope nearest;
    third_nearest = keep(one / three);
    rounded_nearest = keep(std::lrint(two_point_three));
    std::snprintf(printed_nearest, sizeof printed_nearest, "%.2f", price);
    read_nearest = keep(std::strtod("0.3", nullptr));
  }

  /* The other rows of the table of doc/using.md, computed the same way, from
     doubles drawn with integer arithmetic and exact scalings, which do not
     depend on the direction. */
  std::mt19937_64 generator(1788);
  // A significand in [-1, 1), and an exponent in [-spread, spread], drawn
  // one after the other: the compilers evaluate arguments in another order
  const auto random_double = [&generator](int spread) {
    const double significand = std::ldexp(static_cast<double>(generator() >> 11), -52) - 1.0;
    const int exponent = static_cast<int>(generator() % (2 * spread + 1)) - spread;
    return std::ldexp(significand, exponent);
  };
  std::vector<double> values;
  for (int i = 0; i < 20000; ++i) {
    values.push_back(random_double(13));
  }
  std::vector<std::pair<double, double>> pairs;
  for (int i = 0; i < 50000; ++i) {
    const double a = random_double(60);
    const double b = random_double(60);
    pairs.emplace_back(a, b);
  }
  /* Pairs of doubles of [1, 2), full significands, for TwoProd: the pairs
     above, of exponents spread over 120, meet no case where the directed
     roundings break Dekker's product (the error of the split parts they
     drop is then below the error of the rounded product), where about one
     pair in 26 of [1, 2) does. */
  std::vector<std::pair<double, double>> product_pairs;
  for (int i = 0; i < 50000; ++i) {
    product_pairs.emplace_back(1.0 + std::fabs(random_double(0)), 1.0 + std::fabs(random_double(0)));
  }
  const Consequences as_it_runs = consequences(values, pairs, product_pairs);
  Consequences to_nearest;
  {
    nearest_scope nearest;
    to_nearest = consequences(values, pairs, product_pairs);
  }

  // 1/3 lies between two doubles, a third of the gap above the lower one
  // (mpmath): the lower one is the nearest, the upper one the rounding up
  const interval third_enclosure = interval(1.0) / 3.0;
  const interval tenths = textToInterval("0.3");
  // The exact sum of ten million doubles 0.1 lies in this enclosure
  const interval tenths_sum = interval(0.1) * 1e7;
  const bool upward = at_start == FE_UPWARD;

  show("std::fegetround()", direction_name(at_start),
       at_start == FE_UPWARD ? "set by GAOL's initialization, before main()" : "GAOL did not change it",
       !leaves_upward || at_start == FE_UPWARD);
  show("1.0 / 3.0", hex(third) + ", " + hex(third_nearest),
       third == third_nearest ? "the double nearest to 1/3, both times" : "the double above 1/3, then the nearest",
       third == (at_start == FE_UPWARD ? third_enclosure.right() : third_enclosure.left())
           && third_nearest == third_enclosure.left());
  show("std::lrint(2.3)", std::to_string(rounded) + ", " + std::to_string(rounded_nearest),
       "lrint() rounds in the current direction", rounded == (at_start == FE_UPWARD ? 3 : 2) && rounded_nearest == 2);
  /* Annex F of the C standard asks printf() and strtod() to round in the
     current direction, which the GNU C library does; not every C library
     does, so the lines below state what they see. The literal 0.3 is
     converted by the compiler, to nearest. */
  show("printf(\"%.2f\", 2.675)", std::string(printed) + ", " + printed_nearest,
       std::string(printed) != printed_nearest ? "the C library rounds its decimal output too"
                                               : "the same text in both directions",
       true);   // what the C library does, reported rather than checked
  show("strtod(\"0.3\")", hex(read) + ", " + hex(read_nearest),
       read != read_nearest ? "and its decimal input: strtod(\"0.3\") != 0.3" : "the same double in both directions",
       is_bound_of(read, tenths) && is_bound_of(read_nearest, tenths) && (read == read_nearest || read != 0.3));
  show("cout << setprecision(3) << pi", as_it_runs.stream + ", " + to_nearest.stream,
       as_it_runs.stream != to_nearest.stream ? "and so do the C++ streams" : "the same text in both directions",
       true);   // what the C++ library does, reported rather than checked
  // Every addition rounded upward ends above the exact sum, and to nearest,
  // this one ends below it
  show("0.1 added 10^7 times", decimal(as_it_runs.sum) + ", " + decimal(to_nearest.sum),
       upward ? "above the exact sum, then below it" : "below the exact sum, both times",
       (upward ? as_it_runs.sum > tenths_sum.right() : as_it_runs.sum < tenths_sum.left())
           && to_nearest.sum < tenths_sum.left());
  // A text written with 17 digits and read back has to give the same double:
  // it does to nearest, and no longer when both roundings go upward
#ifdef __GLIBC__
  const bool text_follows_direction = true;
#else
  const bool text_follows_direction = false;   // reported, not checked
#endif
  show("%.17g, then strtod", std::to_string(as_it_runs.changed) + ", " + std::to_string(to_nearest.changed),
       "of " + std::to_string(values.size()) + " doubles come back different",
       !text_follows_direction
           || (as_it_runs.changed == (upward ? values.size() : 0) && to_nearest.changed == 0));
  // TwoSum gives the error of a sum exactly to nearest, and not always upward
  show("TwoSum(1e20, 1e-20)",
       std::string(as_it_runs.exact_sum ? "exact" : "not exact") + ", " + (to_nearest.exact_sum ? "exact" : "not exact"),
       "s + e is a + b only to nearest", to_nearest.exact_sum && as_it_runs.exact_sum == !upward);
  show("TwoSum, pairs of doubles", std::to_string(as_it_runs.inexact_pairs) + ", " + std::to_string(to_nearest.inexact_pairs),
       "of " + std::to_string(pairs.size()) + " with exponents within 60: not exact",
       to_nearest.inexact_pairs == 0 && (upward ? as_it_runs.inexact_pairs > 0 : as_it_runs.inexact_pairs == 0));
  // TwoProd, the error of a product: Dekker's splitting breaks as TwoSum does,
  // and the error of std::fma is exact in every rounding direction
  show("TwoProd, Dekker's splitting", std::to_string(as_it_runs.inexact_products) + ", " + std::to_string(to_nearest.inexact_products),
       "of " + std::to_string(product_pairs.size()) + " of [1, 2): not exact",
       to_nearest.inexact_products == 0 && (upward ? as_it_runs.inexact_products > 0 : as_it_runs.inexact_products == 0));
  show("TwoProd, std::fma", std::to_string(as_it_runs.inexact_fma_products) + ", " + std::to_string(to_nearest.inexact_fma_products),
       "the error of a rounded product, exact in every direction",
       as_it_runs.inexact_fma_products == 0 && to_nearest.inexact_fma_products == 0);

  // ------------------------------------------------------------------------
  std::cout << "2. GAOL's intervals do not depend on the direction the program leaves\n";

  /* The references are computed upward, then everything again after the
     program set each other direction: each operation of GAOL sets the
     direction upward itself when it is not, and its bounds are then the same
     whatever the direction the program left. */
  std::fesetround(FE_UPWARD);
  const std::vector<interval> reference = compute_all();
  const std::vector<std::string> reference_texts = exact_texts(reference);
  for (std::size_t i = 0; i < computation_count; ++i) {
    show(computations[i].what, std::string(reference[i]), computations[i].claim,
         reference[i].set_contains(textToInterval(computations[i].truth)));
  }

  const struct {
    int direction;
    const char* name;
  } others[] = {{FE_TONEAREST, "FE_TONEAREST"}, {FE_DOWNWARD, "FE_DOWNWARD"}, {FE_TOWARDZERO, "FE_TOWARDZERO"}};
  for (const auto& other : others) {
    std::fesetround(other.direction);
    const std::vector<std::string> texts = exact_texts(compute_all());
    const int afterwards = std::fegetround();
    show(std::string("after fesetround(") + other.name + ")",
         texts == reference_texts ? "the same 7 intervals, bit for bit" : "OTHER intervals",
         built_to_preserve ? "and the direction is restored"
                           : "and the last operation leaves the direction upward",
         texts == reference_texts && afterwards == (built_to_preserve ? other.direction : FE_UPWARD));
  }

  /* exact_string() writes the bounds in hexadecimal, which textToInterval()
     reads back bit for bit, and %a the same for a double, which strtod()
     reads back: exact, in any direction. A double written with 17 decimal
     digits and read back while rounding upward can move up by one double at
     each round trip, with the GNU C library. */
  std::fesetround(FE_DOWNWARD);
  bool read_back = keep(std::strtod(hex(third).c_str(), nullptr)) == third;
  for (const std::string& text : reference_texts) {
    read_back = read_back && exact_string(textToInterval(text)) == text;
  }
  show("exact_string(), %a, read back", read_back ? "the same bits, after FE_DOWNWARD" : "OTHER bits",
       "textToInterval(\"[0x...]\"), strtod(\"0x...\") are exact", read_back);

  // The program's doubles do follow the direction: interval(1.0 / 3.0) would
  // be one double or the other, and enclose nothing
  std::fesetround(FE_UPWARD);
  const double up = keep(one / three);
  std::fesetround(FE_DOWNWARD);
  const double down = keep(one / three);
  show("1.0 / 3.0 upward, downward", hex(up) + ", " + hex(down), "the program's doubles vary, enclose nothing",
       up == third_enclosure.right() && down == third_enclosure.left());
  std::fesetround(at_start);

  // ------------------------------------------------------------------------
  std::cout << "3. A block of the program's own double code in a nearest_scope\n";

  /* Measurements put into unit bins with std::lrint(), as the program runs
     and in the block. std::lround() rounds to the nearest integer in every
     direction (halfway cases away from zero, of which there are none here):
     it gives the bins wanted. */
  static volatile double measurements[] = {0.2, 1.4, 2.3, 2.8, 3.6, 4.1, 5.45, 6.7};
  const std::size_t count = sizeof measurements / sizeof measurements[0];
  volatile long outside[count];
  volatile long inside[count];
  const int entry = std::fegetround();
  for (std::size_t i = 0; i < count; ++i) {
    outside[i] = std::lrint(measurements[i]);
  }

  /* Values already in registers, and their quotient computed before the
     block: written again in the block, the quotient may be the one computed
     before, rounded upward. GCC 9 reuses it (bug 34678), Clang 18 computes
     it again: a program cannot rely on either. 1.0 / 3.0 of the volatile
     data is computed in the block, whatever the compiler. */
  const double a = one;
  const double b = three;
  const double before = keep(a / b);
  double again = 0.0;
  double fresh = 0.0;
  int during = 0;
  {
    nearest_scope nearest;
    for (std::size_t i = 0; i < count; ++i) {
      inside[i] = std::lrint(measurements[i]);
    }
    again = keep(a / b);
    fresh = keep(one / three);
    // An operation of GAOL, last, as it sets the direction upward again
    static_cast<void>(interval(a) / b);
    during = std::fegetround();
  }
  const int after_block = std::fegetround();

  std::string bins_outside;
  std::string bins_inside;
  int moved_outside = 0;
  int moved_inside = 0;
  bool outside_ceil = true;
  for (std::size_t i = 0; i < count; ++i) {
    bins_outside += std::to_string(outside[i]) + " ";
    bins_inside += std::to_string(inside[i]) + " ";
    moved_outside += outside[i] != std::lround(measurements[i]);
    moved_inside += inside[i] != std::lround(measurements[i]);
    outside_ceil = outside_ceil && outside[i] == static_cast<long>(std::ceil(measurements[i]));
  }
  show("std::lrint(x), outside", bins_outside,
       moved_outside > 0 ? std::to_string(moved_outside) + " of " + std::to_string(count) + " rounded up, not to nearest"
                         : "each x to its nearest integer",
       entry == FE_UPWARD ? moved_outside > 0 && outside_ceil : moved_outside == 0);
  show("std::lrint(x), in the block", bins_inside, "each x to its nearest integer", moved_inside == 0);
  show("1.0 / 3.0 of volatile data", hex(fresh), "computed in the block: to nearest", fresh == third_enclosure.left());
  // The quotient written again in the block: the compiler decides
  std::string again_claim = "to nearest: computed again in the block";
  if (entry != FE_UPWARD) {
    again_claim = "to nearest, as before the block";
  } else if (again == before) {
    again_claim = "rounded upward: reused (GCC bug 34678)";
  }
  show("a / b again, a, b in registers", hex(again), again_claim,
       again == before || again == third_enclosure.left());
  show("an interval operation in it", direction_name(during),
       leaves_upward ? "GAOL set upward again: no intervals here" : "GAOL restored the block's direction",
       during == (leaves_upward ? FE_UPWARD : FE_TONEAREST));
  show("after the block", direction_name(after_block), "the guard set back the direction it found",
       after_block == entry);

  // ------------------------------------------------------------------------
  std::cout << "4. A std::thread started by main()\n";

  /* C++ gives a new thread the floating-point environment of the thread
     constructing it ([cfenv.syn]): a thread started once GAOL is initialized
     rounds its doubles upward too, until it sets another direction itself.
     Its intervals are the same, each operation of GAOL setting the direction
     it needs. gaol::cleanup() sets back the direction of the thread calling
     it only: a thread that goes on with its own double code after intervals
     sets FE_TONEAREST itself. */
  const int main_direction = std::fegetround();
  int thread_start = 0;
  double thread_third = 0.0;
  std::vector<std::string> thread_texts;
  std::thread worker([&] {
    thread_start = std::fegetround();
    thread_third = keep(one / three);
    thread_texts = exact_texts(compute_all());
  });
  worker.join();
  show("its direction when it starts", direction_name(thread_start),
       thread_start == main_direction ? "main()'s: a thread inherits it" : "not main()'s", true);
  show("its 1.0 / 3.0", hex(thread_third),
       thread_start == FE_UPWARD ? "rounded upward, as main()'s doubles" : "to nearest",
       thread_third == (thread_start == FE_UPWARD ? third_enclosure.right() : third_enclosure.left()));
  show("its 7 intervals", thread_texts == reference_texts ? "the same as main()'s, bit for bit" : "OTHER intervals",
       "compared with exact_string()", thread_texts == reference_texts);

  // ------------------------------------------------------------------------
  std::cout << "5. gaol::cleanup(), and GAOL used again after it\n";

  // What the lines below compare with, taken before cleanup()
  const double third_down = third_enclosure.left();

  /* gaol::cleanup(), which a program calls right after its last use of
     GAOL, sets back the rounding direction GAOL found when it initialized
     itself: to nearest, as a program starts (built with
     GAOL_PRESERVE_ROUNDING, GAOL never left it). It does so once only: an
     operation of GAOL after it sets the direction upward again, which this
     section does on purpose, and a second cleanup() does nothing. A program
     that uses GAOL in phases, with its own double code between them, calls
     gaol::restore_rounding() after each phase, as many times as it needs
     (GAOL v5), and cleanup() after the last one. */
  const bool first = gaol::cleanup();
  const int after_cleanup = std::fegetround();
  const double third_after = keep(one / three);
  static_cast<void>(interval(1.0) / three);   // GAOL used again after cleanup()
  const int after_late = std::fegetround();
  // The last use of GAOL: cleanup() again, which now does nothing
  const bool second = gaol::cleanup();
  const int after_second = std::fegetround();
  gaol::restore_rounding();
  gaol::restore_rounding();
  const int final_direction = std::fegetround();

  show("gaol::cleanup()", std::string(first ? "true" : "false") + ", " + direction_name(after_cleanup),
       leaves_upward ? "sets back the direction before GAOL's init" : "nothing to set back",
       first && after_cleanup == FE_TONEAREST);
  show("1.0 / 3.0", hex(third_after),
       leaves_upward ? "the program's doubles, to nearest again" : "to nearest, as all along",
       third_after == third_down);
  show("an interval operation after it", direction_name(after_late),
       leaves_upward ? "GAOL sets upward again, for the program too" : "GAOL restores the direction",
       after_late == (leaves_upward ? FE_UPWARD : FE_TONEAREST));
  show("gaol::cleanup() again", std::string(second ? "true" : "false") + ", " + direction_name(after_second),
       leaves_upward ? "cleanup() sets the direction back once only" : "cleanup() has nothing left to do",
       !second && after_second == after_late);
  show("gaol::restore_rounding() twice", direction_name(final_direction),
       leaves_upward ? "what the program calls after each phase, as many times as it needs"
                     : "GAOL restores the direction itself: restore_rounding() has nothing to do",
       final_direction == FE_TONEAREST);

  std::cout << "What to do\n"
            << "  read data with textToInterval(\"0.3\"), which encloses 3/10 in every direction, not with strtod()\n"
            << "  write data with exact_string() or %a, read back to the same bits in every direction\n"
            << "  call gaol::cleanup() right after the last use of GAOL, and gaol::restore_rounding() after each phase\n";

  if (failures != 0) {
    return EXIT_FAILURE;
  }
  std::cout << "Every claim above was checked.\n";
  return 0;
}
