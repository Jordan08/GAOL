/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: bounded-error parameter estimation by set inversion.
 *
 * Part 1 finds every pair of parameters (p1, p2) in [0, 1]^2 for which the
 * model y(t) = 20 exp(-p1 t) - 8 exp(-p2 t) goes through ten interval
 * measurements [y_i] taken at t = 1, ..., 10: the set S of the parameters
 * whose image lies in the box of the measurements, the inverse image of that
 * box. SIVIA (Set Inversion Via Interval Analysis, L. Jaulin and E. Walter,
 * Automatica 29(4), 1993) proves a box inside S when the enclosure of every
 * model output is inside its measurement, outside S when one of them is
 * disjoint from its measurement, and bisects the other boxes down to a
 * width epsilon. The inner and boundary boxes enclose S, from which follow
 * its hull and guaranteed bounds on its area. Nothing is assumed about the
 * errors but their bounds: this is the bounded-error, or set-membership,
 * approach to estimation. Part 1 follows IBEX's lab5
 * (ibex-lib/examples/lab5.cpp and doc/lab.rst, "Parameter Estimation"),
 * whose data it uses.
 *
 * Part 2 locates a robot from its distances to three beacons, each known
 * within an interval (Codac's examples/13_qinter, from L. Jaulin and
 * B. Desrochers, "Robust localisation using separators"; Codac allows one
 * of the three distances to be wrong, SepQInter with q = 1, where all three
 * hold here): the same paving, with a forward-backward contractor (IBEX's
 * CtcFwdBwd, Codac's CtcInverse) that removes from each box parts proved
 * outside before bisecting it. It shows the relational function
 * sqrt_rel(), the reverse of sqr. 07_contractors.cpp explains these
 * contractors, and 08_sivia.cpp the trap of a function that is not defined
 * everywhere.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

#include <gaol/gaol.h>

#include "box.h"

using namespace gaol;
using examples::Box;

namespace {

  bool ok = true;

  void check(bool condition, const char* what)
  {
    if (!condition) {
      std::cout << "FAILED: " << what << "\n";
      ok = false;
    }
  }

  enum class Verdict { inside, outside, undecided };

  // The result of a paving: the boxes proved inside the set, the boxes left
  // undecided at the width epsilon, and how many boxes were tested and
  // proved outside
  struct Paving {
    std::vector<Box> inside, boundary;
    long tested = 0, outside = 0;

    // Bounds on the area of the set: the inner boxes are in it, and it is in
    // the inner and boundary boxes, the others being proved outside. Box's
    // volume() is an interval, so that the sum is rounded outward.
    interval area() const
    {
      interval in(0.0), bd(0.0);
      for (const Box& b : inside) {
        in += b.volume();
      }
      for (const Box& b : boundary) {
        bd += b.volume();
      }
      return interval(in.left(), (in + bd).right());
    }

    // The hull of the set: it contains every point of the set
    Box hull() const
    {
      Box h(2, interval::emptyset());
      for (const Box& b : inside) {
        h |= b;
      }
      for (const Box& b : boundary) {
        h |= b;
      }
      return h;
    }

    // True if one of the boxes meets p (a point, enclosed in a tiny box)
    static bool meets(const std::vector<Box>& boxes, const Box& p)
    {
      for (const Box& b : boxes) {
        if (!b[0].set_disjoint(p[0]) && !b[1].set_disjoint(p[1])) {
          return true;
        }
      }
      return false;
    }
  };

  // SIVIA: a box is first contracted (contract() returns false when nothing
  // is left of it), then classified; an undecided box is bisected until it
  // is narrower than eps. A stack gives a depth-first search.
  template <class Contract, class Classify>
  Paving sivia(const Box& init, double eps, const Contract& contract, const Classify& classify)
  {
    Paving s;
    std::vector<Box> stack{ init };
    while (!stack.empty()) {
      Box p = stack.back();
      stack.pop_back();
      ++s.tested;
      if (!contract(p)) {
        ++s.outside;
        continue;
      }
      const Verdict v = classify(p);
      if (v == Verdict::outside) {
        ++s.outside;
      } else if (v == Verdict::inside) {
        s.inside.push_back(p);
      } else {
        if (p.max_width() <= eps) {
          s.boundary.push_back(p);
        } else {
          const auto halves = p.bisect();
          stack.push_back(halves.first);
          stack.push_back(halves.second);
        }
      }
    }
    return s;
  }

  //------------------------------------------------------------------------
  // Part 1: IBEX lab5

  // The output of the model at time t for the parameters p = (p1, p2). Each
  // parameter occurs once: the natural extension is then the exact range of
  // the output over the box, up to the outward rounding, with no
  // overestimation from the dependency problem.
  interval model(const Box& p, double t)
  {
    return 20.0 * exp(-p[0] * t) - 8.0 * exp(-p[1] * t);
  }

  struct Measurement {
    double t;
    interval y;
  };

  // Is every parameter of p consistent with the data (inside), none
  // (outside), or can the enclosures not tell (undecided)?
  Verdict classify(const Box& p, const std::vector<Measurement>& data)
  {
    bool inside = true;
    for (const Measurement& m : data) {
      const interval y = model(p, m.t);
      // GAOL has no decorations: an operation returns the image of the part
      // of its domain where it is defined, and an empty set where it is
      // defined nowhere. The empty set is a subset of every measurement, so
      // is_empty() must be tested before set_contains(), or a box where the
      // model is undefined would be proved inside. And where the model is
      // defined on a part of the box only (sqrt, log), "inside" is not
      // proved at all: IEEE 1788 decorations would tell, GAOL cannot. exp is
      // defined everywhere, so neither happens here.
      if (y.is_empty() || y.set_disjoint(m.y)) {
        return Verdict::outside;
      }
      if (!m.y.set_contains(y)) {
        inside = false;
      }
    }
    return inside ? Verdict::inside : Verdict::undecided;
  }

  // True when the point p fits every measurement, for sure
  bool fits(const Box& p, const std::vector<Measurement>& data)
  {
    return classify(p, data) == Verdict::inside;
  }

  void part1()
  {
    // The measurements of lab5.cpp. They are decimal numbers, read from
    // their text: interval("[0.67, 4.6]") is rounded outward and encloses
    // them, whereas interval(0.67, 4.6) would take the nearest doubles and
    // lose both ends of the interval: the double 0.67 is above 0.67, and the
    // double 4.6 below 4.6.
    const char* const y_text[10] = { "[4.5, 7.5]",   "[0.67, 4.6]", "[-1, 2.8]",   "[-1.7, 1.7]",
                                     "[-1.9, 0.93]", "[-1.8, 0.5]", "[-1.6, 0.24]", "[-1.4, 0.09]",
                                     "[-1.2, 0.0089]", "[-1, -0.031]" };
    std::vector<Measurement> data;
    for (int i = 0; i < 10; ++i) {
      data.push_back({ i + 1.0, interval(y_text[i]) });
    }
    std::cout << "Part 1. Bounded-error parameter estimation (IBEX lab5)\n"
              << "  y(t) = 20 exp(-p1 t) - 8 exp(-p2 t) measured at t = 1, ..., 10:\n"
              << "  y(1) in [4.5, 7.5], y(2) in [0.67, 4.6], ..., y(10) in [-1, -0.031]\n"
              << "  y(2) read from text as " << data[1].y << "\n";

    const Paving s = sivia(
      Box{ interval(0.0, 1.0), interval(0.0, 1.0) }, 0.001, [](Box&) { return true; },
      [&](const Box& p) { return classify(p, data); });
    std::cout << "  set inversion of ([0, 1] ; [0, 1]) down to a width of 0.001: " << s.tested << " boxes tested\n"
              << "    inside   " << std::setw(6) << s.inside.size() << " boxes: every point fits the 10 measurements\n"
              << "    outside  " << std::setw(6) << s.outside << " boxes: no point fits them all\n"
              << "    boundary " << std::setw(6) << s.boundary.size() << " boxes, undecided at that width\n";

    // The hull and the area of S computed apart with mpmath, from the exact
    // slices of S: y increases with p2, so that for a given p1 each bound of
    // a measurement bounds p2 by a logarithm, -log((20 exp(-p1 t) - y) / 8) / t;
    // the area is the integral over p1 of the width of the slice (mpmath's
    // quad, between the points where the active bounds change)
    const Box hull = s.hull();
    const Box hull_ref{ interval("[0.33098635095139862492168319503200028, 0.77609861402355031216830196139317631]"),
                        interval("[0.15310755595639399872521438977822748, 0.53103716068658547974283981201728409]") };
    const interval area_ref("0.06611365919069719801467444514175237576689");
    const interval area = s.area();
    std::cout << "  hull of S (inside and boundary boxes) " << hull << "\n"
              << "  holds the hull of S computed apart    " << hull_ref << "\n"
              << "  area of S in " << area << ", which holds " << area_ref << "\n";
    check(hull.contains(hull_ref), "the hull of the paving misses a part of the hull of S");
    check(area.set_contains(area_ref), "the bounds on the area miss the area of S");

    // Every inner box is in S: its midpoint fits every measurement
    bool midpoints_fit = true;
    for (const Box& b : s.inside) {
      midpoints_fit = midpoints_fit && fits(b.mid(), data);
    }
    check(midpoints_fit, "the midpoint of an inner box does not fit the data");

    // The code at the end of lab5.cpp makes the data from the corners of
    // [0.4, 0.6] x [0.2, 0.3], keeping two digits. Three corners, and two
    // points near the ends of S that mpmath found, fit the data: they have
    // to be in the paving. The corner (0.6, 0.2) does not: the published
    // lower bounds are above its outputs, and no inner box can hold it.
    const char* const fitting[5][2] = { { "0.4", "0.2" },
                                        { "0.4", "0.3" },
                                        { "0.6", "0.3" },
                                        { "0.33108635", "0.15323329" },
                                        { "0.77599861", "0.53092855" } };
    bool all_fit = true, all_covered = true;
    for (const auto& p : fitting) {
      const Box point{ interval(p[0]), interval(p[1]) };
      all_fit = all_fit && fits(point, data);
      all_covered = all_covered && (Paving::meets(s.inside, point) || Paving::meets(s.boundary, point));
    }
    std::cout << "  (0.4, 0.2), (0.4, 0.3), (0.6, 0.3), (0.33108635, 0.15323329), (0.77599861, 0.53092855):\n"
              << "    proved to fit every measurement, and in the paving\n";
    check(all_fit, "a point of S does not fit the data");
    check(all_covered, "a point of S is not in the paving");

    const Box corner{ interval("0.6"), interval("0.2") };
    const interval y1 = model(corner, 1.0);
    std::cout << "  (0.6, 0.2): y(1) in " << y1 << ", proved outside [4.5, 7.5]\n";
    check(y1.set_disjoint(data[0].y) && !Paving::meets(s.inside, corner),
          "(0.6, 0.2) should be proved outside S");
  }

  //------------------------------------------------------------------------
  // Part 2: range-only localisation, Codac's example 13_qinter

  // The robot is at a distance in d from the beacon (bx, by)
  struct Beacon {
    double bx, by;
    interval d;
  };

  // Forward-backward contractor of (x - bx)^2 + (y - by)^2 in d^2 (IBEX's
  // HC4Revise). The forward pass evaluates the expression node by node; the
  // root is intersected with the values the constraint allows; the backward
  // pass then intersects each node with what its parent allows, down to the
  // variables. No point of the box that satisfies the constraint is lost.
  bool contract_ring(const Beacon& b, Box& p)
  {
    interval u = p[0] - b.bx, v = p[1] - b.by;
    interval u2 = sqr(u), v2 = sqr(v);
    interval s = u2 + v2;
    s &= sqr(b.d);
    if (s.is_empty()) {
      return false;
    }
    u2 &= s - v2;
    v2 &= s - u2;
    // sqrt_rel(c, u) is the hull of the u in u whose square is in c (sqrRev
    // of IEEE 1788): both signs, where sqrt(c) would keep only the positive
    // square root and lose the points left of the beacon
    u = sqrt_rel(u2, u);
    v = sqrt_rel(v2, v);
    p[0] &= u + b.bx;
    p[1] &= v + b.by;
    return !p.is_empty();
  }

  Verdict classify(const Box& p, const std::vector<Beacon>& beacons)
  {
    bool inside = true;
    for (const Beacon& b : beacons) {
      const interval s = sqr(p[0] - b.bx) + sqr(p[1] - b.by);
      const interval d2 = sqr(b.d);
      if (s.set_disjoint(d2)) {
        return Verdict::outside;
      }
      if (!d2.set_contains(s)) {
        inside = false;
      }
    }
    return inside ? Verdict::inside : Verdict::undecided;
  }

  void part2()
  {
    const std::vector<Beacon> beacons = { { 1.0, 3.0, interval(1.0, 2.0) },
                                          { 3.0, 1.0, interval(2.0, 3.0) },
                                          { -1.0, -1.0, interval(3.0, 4.0) } };
    const Box init{ interval(-6.0, 6.0), interval(-6.0, 6.0) };
    const double eps = 0.01;
    std::cout << "\nPart 2. Range-only localisation from three beacons (Codac example 13_qinter)\n"
              << "  distance in [1, 2] from (1, 3), [2, 3] from (3, 1), [3, 4] from (-1, -1)\n";

    // The three contractors applied in turn until a pass shrinks no side of
    // the box by more than 10%: a fixpoint, up to that ratio
    const auto contract_all = [&](Box& p) {
      while (true) {
        const Box before = p;
        for (const Beacon& b : beacons) {
          if (!contract_ring(b, p)) {
            return false;
          }
        }
        if (!(p[0].width() < 0.9 * before[0].width() || p[1].width() < 0.9 * before[1].width())) {
          return true;
        }
      }
    };
    // GAOL writes -0 for the lower bound 0 that -1 + 1 gives here: -0 and 0
    // are the same number, and the sign of a zero bound means nothing
    Box contracted = init;
    contract_all(contracted);
    std::cout << "  the contractor alone reduces " << init << " to\n"
              << "    " << contracted << "\n"
              << "  pavings down to a width of 0.01    tested  inside  boundary  area of the set in\n";

    const Paving plain = sivia(
      init, eps, [](Box&) { return true; }, [&](const Box& p) { return classify(p, beacons); });
    const Paving ctc = sivia(init, eps, contract_all, [&](const Box& p) { return classify(p, beacons); });
    const interval area_ref("0.569341846214634982520589826093");
    const Paving* const pavings[2] = { &plain, &ctc };
    const char* const names[2] = { "bisection only", "contractor, then bisection" };
    for (int k = 0; k < 2; ++k) {
      const Paving& s = *pavings[k];
      std::cout << "    " << std::left << std::setw(29) << names[k] << std::right << std::setw(8) << s.tested
                << std::setw(8) << s.inside.size() << std::setw(10) << s.boundary.size() << "  " << s.area() << "\n";
      check(s.area().set_contains(area_ref), "the bounds on the area miss the area of the set");
    }
    std::cout << "  both hold the area computed apart, " << area_ref << "\n";
    check(ctc.tested < plain.tested, "the contractor should save boxes");
    check(ctc.area().width() < plain.area().width(), "the contractor should tighten the area");

    // A point of the set found apart, with a margin of 0.27 on each ring
    const Box point{ interval("0.81"), interval("1.74") };
    const bool in_set = classify(point, beacons) == Verdict::inside;
    const bool covered = (Paving::meets(ctc.inside, point) || Paving::meets(ctc.boundary, point)) &&
                         (Paving::meets(plain.inside, point) || Paving::meets(plain.boundary, point));
    std::cout << "  the point (0.81, 1.74) is proved in the set, and in both pavings\n";
    check(in_set && covered, "the point (0.81, 1.74) should be in the set and in the pavings");
  }

} // namespace

int main()
{
  // 7 digits are enough to read the results; interval::precision() sets how
  // many digits GAOL writes, 16 by default
  interval::precision(7);
  part1();
  part2();
  gaol::cleanup();
  return ok ? 0 : EXIT_FAILURE;
}
