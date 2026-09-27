/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: generic code, one function for double, interval and
 * Dual.
 *
 * The program writes a physical model once, as a template: the
 * Maxwell-Boltzmann density of the speeds of the molecules of a gas, f(v) =
 * sqrt(2/pi) v^2 exp(-v^2/2) with v in units of sqrt(kT/m). With T = double
 * it computes an approximation, with T = gaol::interval an enclosure of the
 * value, at a point or over a range of speeds, and with T = Dual<1> (dual.h)
 * an enclosure of the derivative as well. Generic statistics (a mean with
 * std::accumulate, a mean energy with std::inner_product) then run on a
 * std::vector<interval> of measured speeds, and the program shows what
 * generic code has to write for intervals, and why: the using-declarations
 * that let argument-dependent lookup find GAOL's functions, pow being in the
 * namespace gaol rather than in the namespace of interval; T(0.0) rather
 * than T{} (the whole line) or T(0) (ambiguous); constants computed in T
 * rather than written as doubles; comparisons that are "certainly"
 * relations, which std::sort, std::set, std::priority_queue and std::max must
 * not use; set_eq for set equality.
 *
 * It follows the generic style of C++ numerical code (the "using std::sqrt;
 * sqrt(x)" idiom that lets a template take any number type, as Boost.Interval
 * and the kv library expect), the probes of the review of GAOL v5 that tried
 * it (generic templates, argument-dependent lookup, standard algorithms), and
 * Codac's AnalyticFunction, which evaluates one expression on several
 * domains. The model is the Maxwell-Boltzmann distribution of any textbook
 * of statistical physics.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <algorithm>
#include <cfenv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include <gaol/gaol.h>

#include "dual.h"

// No "using namespace gaol": generic code has to work without it, as it
// would in the header of a library, where opening a namespace is bad manners
using gaol::interval;
using examples::Dual;

namespace {

  // The Maxwell-Boltzmann density of the speeds v of the molecules of a gas,
  // in units of sqrt(kT/m): f(v) = sqrt(2/pi) v^2 exp(-v^2/2)
  template <class T>
  T maxwell(const T& v)
  {
    // The idiom of generic C++: the functions of std are brought into scope
    // for double, and argument-dependent lookup finds those of the type of v.
    // GAOL's sqrt, exp and atan are in the namespace gaol_core, where interval
    // is defined, and those of dual.h are friends of Dual: both are found.
    using std::atan;
    using std::exp;
    using std::sqrt;
    // pow is not in gaol_core, on purpose: the namespaces gaol and
    // gaol_ieee1788 each have their own, which differ (see main), and a
    // program chooses one by naming it. Without "using gaol::pow", pow(v, 2)
    // does not compile for an interval: "no matching function for call to
    // pow(const gaol_core::interval&, int)". For a double, std::pow is the
    // better match; for a Dual, the friend of dual.h, found by
    // argument-dependent lookup.
    using gaol::pow;
    using std::pow;
    // pi computed in T: 4 atan(1) is pi to rounding for a double, an
    // enclosure of pi for an interval, a constant for a Dual. Written as the
    // double 3.141592653589793, it would not be pi, and T(3.141592653589793)
    // a point interval without pi in it (interval::pi() encloses it too).
    const T pi = 4.0 * atan(T(1.0));
    // pow(v, 2) rather than v * v: for an interval holding negative and
    // positive values, v * v takes the two factors as independent (see main)
    const T v2 = pow(v, 2);
    return sqrt(2.0 / pi) * v2 * exp(-v2 / 2.0);
  }

  // The mean of values. The sum starts at T(0.0): T{} and T() are the whole
  // line [-oo, +oo] for an interval, as in IBEX and Codac ("nothing known"),
  // and T(0) does not compile, the literal 0 being a null pointer too, which
  // interval(const char*) takes as well as interval(double).
  template <class T>
  T mean(const std::vector<T>& values)
  {
    return std::accumulate(values.begin(), values.end(), T(0.0)) / double(values.size());
  }

  // The mean kinetic energy m v^2 / 2, in units of kT: the mean of v^2 / 2
  template <class T>
  T mean_energy(const std::vector<T>& v)
  {
    return std::inner_product(v.begin(), v.end(), v.begin(), T(0.0)) / (2.0 * double(v.size()));
  }

  // An order on intervals for std::sort, std::set, std::map and
  // std::priority_queue: by left bound, then by right bound, the empty set
  // (whose bounds are NaN) first. operator< is not one: [1.2, 1.25] < [1.22,
  // 1.3] and [1.22, 1.3] < [1.2, 1.25] are both false, as for equal keys, so
  // that std::set<interval> would keep only one of them; and with empty sets,
  // for which it is true, std::sort reads out of its range.
  struct by_bounds {
    bool operator()(const interval& a, const interval& b) const
    {
      if (a.is_empty() || b.is_empty()) {
        return a.is_empty() && !b.is_empty();
      }
      return a.left() < b.left() || (a.left() == b.left() && a.right() < b.right());
    }
  };

  // The intervals of v, through their conversion to std::string
  std::string text(const std::vector<interval>& v)
  {
    std::string s;
    for (const interval& x : v) {
      s += (s.empty() ? "" : " ") + std::string(x);
    }
    return s;
  }

  bool all_passed = true;

  void check(bool passed, const char* what)
  {
    if (!passed) {
      std::cout << "FAILED: " << what << std::endl;
      all_passed = false;
    }
  }

} // namespace

int main()
{
  interval::precision(12);
  std::cout << std::boolalpha;

  // -------------------------------------------------------------------------
  std::cout << "1. One template, three types: f(v) = sqrt(2/pi) v^2 exp(-v^2/2)\n";
  // The references, from mpmath with 60 digits, read by the text constructor,
  // which encloses a decimal that no double equals
  const interval f15("0.58282918049651277426344801079633671032");      // f(1.5)
  const interval df15("-0.097138196749418795710574668466056118387");   // f'(1.5)
  const interval f_max("0.58705065269495959957725771612621847203");    // f(sqrt 2), the maximum
  const interval f2("0.43192773210550441560451360328570865391");       // f(2)
  const interval pi_ref("3.1415926535897932384626433832795028841972");

  // T = double: no error bound. GAOL leaves the rounding direction upward
  // (unless it is built with GAOL_PRESERVE_ROUNDING), for the doubles of the
  // program too: this one is computed with the direction upward, and its
  // last digits depend on the compiler (GCC and Clang print different ones).
  const double at_15 = maxwell(1.5);
  const bool upward = std::fegetround() == FE_UPWARD;
  const interval F15 = maxwell(interval(1.5));
  const interval F12 = maxwell(interval(1.0, 2.0));
  const Dual<1> D15 = maxwell(Dual<1>::variable(interval(1.5), 0));
  const Dual<1> D12 = maxwell(Dual<1>::variable(interval(1.0, 2.0), 0));
  std::cout << "   interval   f([1.5])   = " << F15 << "  holds f(1.5) (mpmath)\n"
            << "   interval   f([1, 2])  = " << F12 << "  holds f(v) for v in [1, 2] (v^2 occurs twice)\n"
            << "   Dual<1>    f'([1.5])  = " << D15.d[0] << "  holds f'(1.5) (mpmath)\n"
            << "   Dual<1>    f'([1, 2]) = " << D12.d[0] << "  holds 0: the maximum may be inside\n"
            << "   double     f(1.5)     : printed in 5., after gaol::cleanup()\n";
  check(F15.set_contains(f15) && F15.width() < 1e-14, "f([1.5]) holds f(1.5) and is tight");
  check(D15.v.set_contains(f15) && D15.d[0].set_contains(df15), "the Dual holds f(1.5) and f'(1.5)");
  // The range of f over [1, 2]: f increases up to sqrt(2), then decreases
  // (f' = sqrt(2/pi) (2v - v^3) exp(-v^2/2)), from f(sqrt 2) down to f(2)
  check(F12.set_contains(f2 | f_max), "f([1, 2]) holds the range [f(2), f(sqrt 2)]");
  // The speeds 1 + k/1000, each enclosed by an interval a few doubles wide
  bool samples_in = true;
  for (int k = 0; k <= 1000; ++k) {
    samples_in = samples_in && F12.set_contains(maxwell(interval(1.0) + k / interval(1000.0)));
  }
  check(samples_in, "f([1, 2]) holds f at 1001 points of [1, 2]");
  check(D12.d[0].set_contains(0.0), "f'([1, 2]) holds 0");
  check((4.0 * atan(interval(1.0))).set_contains(pi_ref) && !interval(3.141592653589793).set_contains(pi_ref),
        "4 atan([1]) holds pi, the double 3.141592653589793 does not");
  // The double is close to f(1.5), but nothing in the program says how close
  check(std::fabs(at_15 - f15.midpoint()) < 1e-14, "the double f(1.5) is close to mpmath's");

  // -------------------------------------------------------------------------
  std::cout << "2. pow: generic code writes \"using std::pow; using gaol::pow;\"\n";
  // Why pow is not found by argument-dependent lookup: the two namespaces
  // give it two meanings, and a program opens one of them
  const interval neg(-4.0, -1.0);
  const interval p_gaol = gaol::pow(neg, 2), p_std = gaol_ieee1788::pow(neg, 2), p_pown = gaol_ieee1788::pown(neg, 2);
  std::cout << "   gaol::pow([-4, -1], 2)           = " << p_gaol << "  the integer power\n"
            << "   gaol_ieee1788::pow([-4, -1], 2)  = " << p_std << "  pow of IEEE 1788: x^y for x > 0\n"
            << "   gaol_ieee1788::pown([-4, -1], 2) = " << p_pown << "  IEEE 1788's integer power\n";
  check(p_gaol.set_eq(interval(1.0, 16.0)) && p_std.is_empty() && p_pown.set_eq(interval(1.0, 16.0)),
        "the two pow of GAOL differ on [-4, -1]");

  // -------------------------------------------------------------------------
  std::cout << "3. A std::vector<interval> of measured speeds, with <numeric>\n";
  // Measured speeds with their uncertainty, given as text: the text
  // constructor encloses the decimals, which two doubles need not do
  const char* const measures[] = {"[1.20, 1.25]", "[0.85, 0.90]", "[1.95, 2.05]",
                                  "[1.22, 1.30]", "[0.60, 0.66]", "[1.40, 1.42]"};
  std::vector<interval> speeds;
  std::string measures_text;
  for (const char* m : measures) {
    speeds.emplace_back(m);
    measures_text += std::string(measures_text.empty() ? "" : " ") + m;
  }
  // Printing rounds each bound outward, so that the text still holds the
  // interval: a bound that is not a double shows as a slightly wider decimal
  interval::precision(5);
  std::cout << "   measured speeds, as text: " << measures_text << "\n"
            << "   read by interval(const char*), printed back with 5 digits rounded outward:\n"
            << "     " << text(speeds) << "\n"
            << "   interval(1.95, 2.05), from two doubles, misses 2.05: the double 2.05 is below it\n";
  check(!interval(1.95, 2.05).set_contains(interval("[1.95, 2.05]")), "interval(1.95, 2.05) misses 2.05");
  const interval mean_speed = mean(speeds), energy = mean_energy(speeds);
  const interval sum_entire = std::accumulate(speeds.begin(), speeds.end(), interval());
  std::cout << "   mean(speeds)        = " << mean_speed << "  std::accumulate from interval(0.0)\n"
            << "   mean_energy(speeds) = " << energy << "  std::inner_product from interval(0.0)\n"
            << "   std::accumulate from interval() = " << sum_entire << ": interval() is the whole line\n";
  // The exact sums, in decimal: 7.22 and 7.58 for the bounds, 9.7734 and
  // 10.717 for their squares (all bounds are positive)
  check(mean_speed.set_contains(interval("7.22/6", "7.58/6")) && mean_speed.width() < 0.36 / 6 + 1e-12,
        "the mean speed holds [7.22/6, 7.58/6], and no more than rounding beyond it");
  check(energy.set_contains(interval("9.7734/12", "10.717/12")) && energy.width() < 0.9436 / 12 + 1e-12,
        "the mean energy holds [9.7734/12, 10.717/12], and no more than rounding beyond it");
  check(sum_entire.is_entire() && interval{}.is_entire(), "interval() and interval{} are [-oo, +oo]");
  // The same templates on doubles: the midpoints of the measures
  std::vector<double> midpoints;
  for (const interval& v : speeds) {
    midpoints.push_back(v.midpoint());
  }
  const double mean_mid = mean(midpoints), energy_mid = mean_energy(midpoints);
  check(mean_speed.set_contains(mean_mid) && energy.set_contains(energy_mid),
        "the means of the midpoints, in double, lie in the enclosures");

  // -------------------------------------------------------------------------
  std::cout << "4. Comparisons are \"certainly\" relations, and there is no ==\n";
  const interval a("[1.20, 1.25]"), b("[1.40, 1.42]"), c("[1.22, 1.30]");
  std::cout << "   [1.2, 1.25] < [1.4, 1.42] : " << (a < b) << "   each element of one is below each of the other\n"
            << "   [1.2, 1.25] < [1.22, 1.3] : " << (a < c) << ", and >= : " << (a >= c)
            << "   they overlap: false is not \"the opposite\"\n";
  check(a < b && !(a < c) && !(a >= c) && !(a > c) && !(c < a), "the certainly relations on overlapping speeds");
  // Set equality is set_eq; == does not compile, having no single meaning.
  // The lower bound 0 of pow(x, 2) may print as -0 (it does with the SSE2
  // build): the same number as 0, the sign of a zero bound meaning nothing
  // for an interval (IEEE 1788-2015, 14.4).
  const interval x(-1.0, 2.0);
  std::cout << "   x = [-1, 2]: x * x = " << x * x << ", pow(x, 2) = " << gaol::pow(x, 2)
            << ", set_eq: " << (x * x).set_eq(gaol::pow(x, 2)) << "\n";
  check(!(x * x).set_eq(gaol::pow(x, 2)) && gaol::pow(x, 2).set_eq(interval(0.0, 4.0)),
        "x * x is not pow(x, 2) on [-1, 2]");
  // std::max(p, q) returns p unless p < q: with the certainly relation, the
  // first argument of two that overlap. gaol::max is the interval max: the
  // set of max(s, t) for s in p and t in q.
  const interval p(1.0, 3.0), q(2.0, 4.0);
  std::cout << "   std::max([1, 3], [2, 4])  = " << std::max(p, q) << "  no maximum: operator< orders nothing\n"
            << "   gaol::max([1, 3], [2, 4]) = " << gaol::max(p, q) << "  max(s, t) for s in [1, 3], t in [2, 4]\n";
  check(std::max(p, q).set_eq(p) && gaol::max(p, q).set_eq(interval(2.0, 4.0)), "std::max against gaol::max");
  // Sorting, sets and priority queues take the explicit order by_bounds
  std::vector<interval> sorted = speeds;
  std::sort(sorted.begin(), sorted.end(), by_bounds());
  const std::set<interval, by_bounds> kept(speeds.begin(), speeds.end());
  std::priority_queue<interval, std::vector<interval>, by_bounds> queue(speeds.begin(), speeds.end());
  std::cout << "   sorted by std::sort with by_bounds:\n"
            << "     " << text(sorted) << "\n"
            << "   std::set<interval, by_bounds>: " << kept.size() << " speeds, [1.2, 1.25] and [1.22, 1.3] kept apart\n"
            << "   std::priority_queue<interval, std::vector<interval>, by_bounds>: top " << queue.top() << "\n";
  bool increasing = std::is_sorted(sorted.begin(), sorted.end(), by_bounds());
  for (std::size_t i = 1; i < sorted.size(); ++i) {
    increasing = increasing && sorted[i - 1].left() <= sorted[i].left();
  }
  check(increasing, "the speeds sorted by by_bounds are sorted by left bound");
  check(kept.size() == speeds.size() && queue.top().set_eq(interval("[1.95, 2.05]")),
        "std::set keeps the 6 speeds, the queue gives [1.95, 2.05] first");
  // Eigen::Matrix<interval, n, n> does not compile today, even with a
  // NumTraits<interval>: Eigen writes Scalar(0) and Scalar(1), and
  // interval(0) is ambiguous, as T(0) above. Codac wraps gaol::interval in a
  // class of its own for that reason.

  // GAOL is not used below. gaol::cleanup() sets the rounding direction back
  // to the one the program started with, to nearest: the doubles computed
  // from here on are rounded to nearest, and so is their printing.
  gaol::cleanup();

  std::printf("5. The same templates on doubles, printed after gaol::cleanup()\n");
  std::printf("   f(1.5) computed before cleanup, direction %s: %.17f\n", upward ? "upward    " : "to nearest", at_15);
  std::printf("   f(1.5) computed after cleanup,  direction to nearest: %.17f\n", maxwell(1.5));
  std::printf("   mean and mean energy of the midpoints               : %.6g, %.6g\n", mean_mid, energy_mid);
  std::printf("   a double carries no error bound; the intervals above do\n");

  if (!all_passed) {
    return EXIT_FAILURE;
  }
  std::printf("All checks passed.\n");
  return 0;
}
