/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: compoundm1, (1 + x)^y - 1 of IEEE 1788-2015 (Table 10.5).
 *
 * CORE-MATH has no compoundm1: its bounds are computed in
 * gaol/math/gaol_compoundm1.h with 64-bit fixed point in a fast phase, then
 * where that does not settle them a 128-bit significand and a bound of the
 * error that every operation adds to, and have to be the tightest ones. This
 * test checks, from the bottom up:
 *
 *  - the integers: the 256-bit product, the shifts, the square root, and the
 *    rounding of an integer to the doubles below and above it, subnormal
 *    results, overflows and carries into the next binade included;
 *  - the balls: sums and products of random doubles against the exact
 *    results (gaol_tests.h), the ball holding the exact value; the series of
 *    exp(r) - 1 against mpmath; the preconditions (an unknown ball, of an
 *    infinite radius, where a precondition fails); the rounding of a ball to
 *    doubles at its ends, across 0 and past the doubles;
 *  - the proofs of equality, power_equals() and product_equals(), against
 *    fractions, with the near misses: none that does not hold, every one that
 *    does with 128 bits to spare;
 *  - the balls of log(1 + x) and of exp(t) - 1, which have to hold the value
 *    to 256 bits computed by mpmath (compoundm1_values.py), within a width
 *    that is checked too;
 *  - the fast phase: the bound of its error, which the balls' value has to
 *    stay far inside of, its answers, which have to be those of the balls, and
 *    its domain; the series, the reduction by log(2) and the doubles around
 *    an enclosure by themselves; and the exact division of y = -1;
 *  - the bounds at a point: for 5854 pairs (x, y) of every magnitude, the
 *    values that are doubles ((1.5)^3 - 1 = 2.375, 2^m - 1...), the tiny x
 *    whose value is next to the double y x, the thresholds of the reduction
 *    by log(2) and of the cuts at -64 and 2048, the doubles that mpmath gives
 *    around the value, exactly;
 *  - the intervals: 850 boxes whose hull is read from the definition (the
 *    corners and 0, the limits at -1 and +oo and for an infinite y), the
 *    cases that were chosen by hand (the empty set, x = -1, the infinite
 *    bounds), random boxes against the hull of the values at their corners
 *    and at points inside them, the three names of the function, the
 *    reading of the text, the exception flags and the modes that flush
 *    the subnormals to zero.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-11-04 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"
#include "gaol/math/gaol_compoundm1.h"
#include "gaol/gaol_ieee1788.h"
#include "compoundm1_values.h"

#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace gaol;
using namespace gaol_tests;

namespace
{
  namespace cm1 = gaol_cm1;
  typedef std::uint64_t u64;

  std::string show(double x)
  {
    return hex(x);
  }

  //-------------------------------------------------------------------------
  // The integers
  //-------------------------------------------------------------------------

  // A 128-bit integer as the 32-bit digits of a schoolbook product, least significant first
  struct Digits { u64 d[8]; };

  Digits schoolbook(const cm1::U128& a, const cm1::U128& b)
  {
    u64 x[4] = { a.l & 0xffffffffu, a.l >> 32, a.h & 0xffffffffu, a.h >> 32 };
    u64 y[4] = { b.l & 0xffffffffu, b.l >> 32, b.h & 0xffffffffu, b.h >> 32 };
    Digits r = { { 0, 0, 0, 0, 0, 0, 0, 0 } };
    for (int i = 0; i < 4; ++i) {
      u64 carry = 0;
      for (int j = 0; j < 4; ++j) {
        const u64 t = x[i]*y[j] + r.d[i + j] + carry; // below 2^64
        r.d[i + j] = t & 0xffffffffu;
        carry = t >> 32;
      }
      r.d[i + 4] += carry;
    }
    return r;
  }

  std::string text(const cm1::U128& a)
  {
    char b[64];
    std::snprintf(b, sizeof b, "0x%016llx%016llx", static_cast<unsigned long long>(a.h), static_cast<unsigned long long>(a.l));
    return b;
  }

  // The bit i of a 256-bit integer
  bool bit(const cm1::U256& a, int i)
  {
    return i >= 0 && i < 256 && ((a.w[i / 64] >> (i % 64)) & 1u) != 0;
  }

  void integers()
  {
    Random rnd;
    std::vector<u64> words = { 0, 1, 2, 3, 0xffffffffu, 0x100000000ull, ~static_cast<u64>(0), ~static_cast<u64>(0) - 1,
                               static_cast<u64>(1) << 63, (static_cast<u64>(1) << 63) - 1 };
    for (int i = 0; i < 40; ++i) {
      words.push_back(rnd.bits());
      words.push_back(rnd.bits() >> rnd.integer(0, 63));
    }
    for (u64 ah : words) {
      const u64 lows[4] = { words[0], words[4], words[8], ah ^ static_cast<u64>(0x5555555555555555ull) };
      for (u64 al : lows) {
        const u64 highs[4] = { words[1], words[3], words[9], words[20] };
        for (u64 bh : highs) {
          const cm1::U128 a = cm1::mk(ah, al), b = cm1::mk(bh, al ^ bh);
          cm1::U128 hi, lo;
          cm1::mul_full(a, b, hi, lo);
          const Digits s = schoolbook(a, b);
          const bool same = lo.l == (s.d[0] | (s.d[1] << 32)) && lo.h == (s.d[2] | (s.d[3] << 32))
            && hi.l == (s.d[4] | (s.d[5] << 32)) && hi.h == (s.d[6] | (s.d[7] << 32));
          check("the 256-bit product is the schoolbook one", same, [&] { return text(a) + " * " + text(b); });
        }
      }
    }
    // (2^128 - 1)^2 = 2^256 - 2^129 + 1
    {
      const cm1::U128 m = cm1::mk(~static_cast<u64>(0), ~static_cast<u64>(0));
      cm1::U128 hi, lo;
      cm1::mul_full(m, m, hi, lo);
      check("(2^128 - 1)^2", hi.h == ~static_cast<u64>(0) && hi.l == ~static_cast<u64>(0) - 1 && lo.h == 0 && lo.l == 1);
    }
    // The shifts of 256 bits, by every count, against the bits
    for (int k = 0; k < 20; ++k) {
      cm1::U256 a;
      for (int i = 0; i < 4; ++i) {
        a.w[i] = (k % 3 == 0) ? rnd.bits() : ((k % 3 == 1) ? ~static_cast<u64>(0) : (rnd.bits() & rnd.bits() & rnd.bits()));
      }
      for (int n = 0; n < 256; ++n) {
        const cm1::U256 l = cm1::shl(a, n);
        bool lost = false;
        const cm1::U256 r = cm1::shr(a, n, lost);
        bool left = true, right = true, dropped = false;
        for (int i = 0; i < 256; ++i) {
          left = left && bit(l, i) == bit(a, i - n);
          right = right && bit(r, i) == bit(a, i + n);
          if (i < n && bit(a, i)) {
            dropped = true;
          }
        }
        check("the shifts of 256 bits are those of the bits", left && right && lost == dropped,
              [&] { return "by " + std::to_string(n); });
      }
      bool lost = false;
      const cm1::U256 z = cm1::shr(a, 300, lost);
      check("a shift of 256 bits or more leaves 0", cm1::is_zero(z) && lost);
    }
    // The shifts of 128 bits, and the counts of bits
    for (int n = 0; n < 128; ++n) {
      const cm1::U128 a = cm1::mk(0x8000000000000001ull, 0x0123456789abcdefull);
      const cm1::U128 l = cm1::shl(a, n), r = cm1::shr(a, n);
      cm1::U256 wide;
      wide.w[0] = a.l; wide.w[1] = a.h; wide.w[2] = wide.w[3] = 0;
      const cm1::U256 wl = cm1::shl(wide, n);
      bool lost = false;
      const cm1::U256 wr = cm1::shr(wide, n, lost);
      check("the shifts of 128 bits", l.l == wl.w[0] && l.h == wl.w[1] && r.l == wr.w[0] && r.h == wr.w[1],
            [&] { return "by " + std::to_string(n); });
    }
    for (int k = 0; k < 64; ++k) {
      const u64 p = static_cast<u64>(1) << k;
      check("clz64 and ctz64 of a power of two", cm1::clz64(p) == 63 - k && cm1::ctz64(p) == k);
      check("the bits of 128-bit and 256-bit integers", cm1::bitlen(cm1::mk(0, p)) == k + 1 && cm1::bitlen(cm1::mk(p, 0)) == 65 + k);
    }
    // isqrt: r^2 - 1, r^2 and r^2 + 2r
    for (int i = 0; i < 3000; ++i) {
      const u64 r = (i < 200) ? static_cast<u64>(i) : (rnd.bits() >> rnd.integer(0, 63));
      if (r == 0) {
        continue;
      }
      u64 hi, lo;
      cm1::mul64(r, r, hi, lo);
      const cm1::U128 sq = cm1::mk(hi, lo);
      u64 c;
      cm1::U128 below = cm1::sub(sq, cm1::mk(0, 1), c);
      cm1::U128 above = cm1::add(sq, cm1::mk(r >> 63, r << 1), c);
      if (c != 0) {
        above = cm1::mk(~static_cast<u64>(0), ~static_cast<u64>(0));
      }
      check("isqrt of a square", cm1::isqrt(sq) == r, [&] { return std::to_string(r); });
      check("isqrt below a square", cm1::isqrt(below) == r - 1, [&] { return std::to_string(r); });
      if (r != ~static_cast<u64>(0) && c == 0) {
        check("isqrt of r^2 + 2r", cm1::isqrt(above) == r, [&] { return std::to_string(r); });
      }
    }
    check("isqrt(2^128 - 1) = 2^64 - 1", cm1::isqrt(cm1::mk(~static_cast<u64>(0), ~static_cast<u64>(0))) == ~static_cast<u64>(0));
  }

  //-------------------------------------------------------------------------
  // The doubles around an integer
  //-------------------------------------------------------------------------

  cm1::U256 integer(u64 w1, u64 w0)
  {
    cm1::U256 a;
    a.w[0] = w0; a.w[1] = w1; a.w[2] = a.w[3] = 0;
    return a;
  }

  void rounding_of_integers()
  {
    struct Case { const char* what; cm1::U256 mag; int q; double down, up; bool exact; };
    const double max = std::numeric_limits<double>::max(), min_sub = 0x1p-1074;
    const double infinity = std::numeric_limits<double>::infinity();
    cm1::U256 wide = integer(0, 1);
    wide.w[2] = 1; // 2^128 + 1
    const Case cases[] = {
      { "1 * 2^-1074", integer(0, 1), -1074, 0x1p-1074, 0x1p-1074, true },
      { "3 * 2^-1075", integer(0, 3), -1075, 0x1p-1074, 0x1p-1073, false },
      { "1 * 2^-1075", integer(0, 1), -1075, 0.0, min_sub, false },
      { "1 * 2^-1200", integer(0, 1), -1200, 0.0, min_sub, false },
      { "a huge shift of 255 bits", integer(1ull << 63, 0), -1500, 0.0, min_sub, false },
      { "2^52 - 1 subnormal units", integer(0, (1ull << 52) - 1), -1074, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, true },
      { "2^53 - 1 at 2^-1075", integer(0, (1ull << 53) - 1), -1075, 0x0.fffffffffffffp-1022, 0x1p-1022, false },
      { "2^52 * 2^-1074", integer(0, 1ull << 52), -1074, 0x1p-1022, 0x1p-1022, true },
      { "1", integer(0, 1), 0, 1.0, 1.0, true },
      { "2^53 + 1", integer(0, (1ull << 53) + 1), 0, 0x1p+53, 0x1.0000000000001p+53, false },
      { "2^54 - 1: the carry into the next binade", integer(0, (1ull << 54) - 1), 0, 0x1.fffffffffffffp+53, 0x1p+54, false },
      { "2^54 - 2", integer(0, (1ull << 54) - 2), 0, 0x1.fffffffffffffp+53, 0x1.fffffffffffffp+53, true },
      { "2^128 + 1: a number of 129 bits", wide, 0, 0x1p+128, 0x1.0000000000001p+128, false },
      { "the largest double", integer(0, (1ull << 53) - 1), 971, max, max, true },
      { "(2^53 + 1) 2^971: past the largest double", integer(0, (1ull << 53) + 1), 971, max, infinity, false },
      { "2^1024", integer(0, 1), 1024, max, infinity, false },
      { "2^54 * 2^970", integer(0, 1ull << 54), 970, max, infinity, false },
    };
    for (const Case& c : cases) {
      bool exact_down = false, exact_up = false;
      const double down = cm1::dbl_round(false, c.mag, c.q, false, exact_down);
      const double up = cm1::dbl_round(false, c.mag, c.q, true, exact_up);
      const double ndown = cm1::dbl_round(true, c.mag, c.q, false, exact_down);
      const double nup = cm1::dbl_round(true, c.mag, c.q, true, exact_up);
      check("the doubles around an integer, rounded down and up", down == c.down && up == c.up && exact_up == c.exact,
            [&] { return std::string(c.what) + ": " + show(down) + ", " + show(up); });
      // up and down are those of the magnitude: -down is the double below -mag... above it, toward 0
      check("the doubles around a negative integer: the magnitudes rounded the same way",
            ndown == -c.down && nup == -c.up, [&] { return std::string(c.what) + ": " + show(ndown) + ", " + show(nup); });
    }
    bool ignored;
    check("a zero is +0", cm1::dbl_round(true, integer(0, 0), 5, true, ignored) == 0.0
          && !std::signbit(cm1::dbl_round(true, integer(0, 0), 5, true, ignored))
          && !std::signbit(cm1::dbl_round(true, integer(0, 1), -1200, false, ignored)));
  }

  //-------------------------------------------------------------------------
  // Balls against exact results
  //-------------------------------------------------------------------------

  void balls_against_exact_results()
  {
    Random rnd;
    for (int i = 0; i < 20000; ++i) {
      const int spread = (i % 4 == 0) ? 20 : ((i % 4 == 1) ? 120 : ((i % 4 == 2) ? 600 : 3));
      const double a = rnd(-spread, spread), b = rnd(-spread, spread), c = rnd(-spread, spread);
      const cm1::Num product = cm1::num_mul(cm1::num_of_double(a), cm1::num_of_double(b));
      const cm1::Num sum = cm1::num_add(product, cm1::num_of_double(c));
      const cm1::Num difference = cm1::num_sub(cm1::num_of_double(a), cm1::num_of_double(c));
      struct Ball { const char* name; const cm1::Num* ball; Exact value; };
      const Ball balls[] = {
        { "a * b", &product, exact(a)*exact(b) },
        { "a * b + c", &sum, exact(a)*exact(b) + exact(c) },
        { "a - c", &difference, exact(a) + (-exact(c)) },
      };
      for (const Ball& v : balls) {
        double lo, hi;
        bool lo_exact, hi_exact;
        cm1::round_ball(*v.ball, lo, hi, lo_exact, hi_exact);
        if (std::isinf(lo) || std::isinf(hi) || std::fabs(lo) > 1e300 || std::fabs(hi) > 1e300) {
          continue; // an overflow: the case above
        }
        const bool exact_ball = cm1::mag_le(v.ball->r, cm1::mag_zero());
        const std::string what = std::string(v.name) + " for " + show(a) + ", " + show(b) + ", " + show(c);
        check("the ball of a sum or a product holds the exact value",
              is_lower_bound(lo, v.value) && is_upper_bound(hi, v.value), [&] { return what + ": " + show(lo) + ", " + show(hi); });
        if (exact_ball) {
          check("an exact ball gives the tightest doubles",
                is_tightest_lower_bound(lo, v.value) && is_tightest_upper_bound(hi, v.value),
                [&] { return what + ": " + show(lo) + ", " + show(hi); });
        } else {
          check("a ball of a width of 2^-120 and less is within a double of the tightest doubles",
                doubles_below_tightest(lo, v.value, 2) <= 1 && doubles_above_tightest(hi, v.value, 2) <= 1,
                [&] { return what + ": " + show(lo) + ", " + show(hi); });
        }
      }
    }
  }

  //-------------------------------------------------------------------------
  // The balls of log(1 + x) and exp(t) - 1
  //-------------------------------------------------------------------------

  // Whether the ball holds the value to 256 bits (the error of the 256 bits, below 2^(e - 256), counting)
  bool holds_value(const cm1::Num& ball, const BallValue& ref)
  {
    cm1::Num center = ball;
    center.r = cm1::mag_zero();
    const cm1::Num ref_hi = cm1::num_of_u128(ref.negative != 0, cm1::mk(ref.m3, ref.m2), ref.e - 128);
    const cm1::Num ref_lo = cm1::num_of_u128(ref.negative != 0, cm1::mk(ref.m1, ref.m0), ref.e - 256);
    const cm1::Num difference = cm1::num_sub(cm1::num_sub(center, ref_hi), ref_lo);
    const cm1::Mag error = cm1::mag_add(cm1::mag_add(cm1::num_mag(difference), difference.r), cm1::mag_pow2(ref.e - 255));
    return cm1::mag_le(error, ball.r);
  }

  // Whether the ball holds the value to 256 bits, and is not wider than 2^-bound relatively
  void check_ball(const char* name, const cm1::Num& ball, const BallValue& ref, int bound, double x)
  {
    const std::string what = show(x);
    check(std::string("the ball of ") + name + " holds the value", holds_value(ball, ref),
          [&] { return what + ": the error is more than the radius"; });
    // |value| >= 2^(e - 1)
    check(std::string("the ball of ") + name + " is narrow", cm1::mag_le(ball.r, cm1::mag_pow2(ref.e - 1 - bound)),
          [&] { return what + ": wider than 2^-" + std::to_string(bound); });
    check(std::string("the ball of ") + name + " has the sign of the value", ball.neg == (ref.negative != 0),
          [&] { return what; });
  }

  void balls_of_log1p_and_expm1()
  {
    for (const BallValue& v : log1p_values) {
      const cm1::Num ball = cm1::log1p_ball(v.x, gaol_cr_log1p(v.x));
      check_ball("log(1 + x)", ball, v, 118, v.x);
    }
    for (const BallValue& v : expm1_values) {
      const cm1::Num ball = cm1::expm1_ball(cm1::num_of_double(v.x));
      check_ball("exp(t) - 1", ball, v, 118, v.x);
    }
    // A ball for t, whose width is multiplied by exp(t) / (exp(t) - 1) at most, and by 2 + |t| from 1
    for (const BallValue& v : expm1_values) {
      if (std::fabs(v.x) > 700.0 || std::fabs(v.x) < 1e-300) {
        continue;
      }
      cm1::Num t = cm1::num_of_double(v.x);
      t.r = cm1::mag_scale(cm1::num_mag(t), -100); // a relative width of 2^-100
      const cm1::Num ball = cm1::expm1_ball(t);
      check("the ball of exp(t) - 1 holds the value for a ball of t", holds_value(ball, v), [&] { return show(v.x); });
    }
    /*
      The ball of a t that holds two consecutive doubles, t0 and t1, holds
      exp(t) - 1 at both: the balls of the two, taken for the values, are
      within the radius of the ball of their mean, whose half-width is the
      distance of the two. That is the derivative exp(t) of the function, 3/2
      of the one of the series at most, which the ball counts.
    */
    for (const BallValue& v : expm1_values) {
      if (std::fabs(v.x) > 700.0 || std::fabs(v.x) < 1e-300) {
        continue;
      }
      const double t0 = v.x, t1 = std::nextafter(t0, (t0 > 0.0) ? 1e300 : -1e300);
      const cm1::Num n0 = cm1::num_of_double(t0), n1 = cm1::num_of_double(t1);
      cm1::Num mean = cm1::num_scale(cm1::num_add(n0, n1), -1);
      mean.r = cm1::num_mag(cm1::num_scale(cm1::num_sub(n1, n0), -1)); // the half of the distance, above
      const cm1::Num wide = cm1::expm1_ball(mean);
      cm1::Num center = wide;
      center.r = cm1::mag_zero();
      for (const cm1::Num& end : { n0, n1 }) {
        const cm1::Num difference = cm1::num_sub(cm1::expm1_ball(end), center);
        const cm1::Mag error = cm1::mag_add(cm1::num_mag(difference), difference.r);
        check("the ball of exp(t) - 1 for the mean of two doubles holds the values at both", cm1::mag_le(error, wide.r),
              [&] { return show(t0); });
      }
    }
  }

  //-------------------------------------------------------------------------
  // The series, the preconditions, the rounding of a ball, the equalities
  //-------------------------------------------------------------------------

  u64 low_word(const cm1::U128& a) { return a.l; }

  // The series (exp(r) - 1) / r on 127 bits for |r| <= 0.36: the error of G is below 4 units of 2^-127, the
  // fourth of the radius 2^-123 the ball of G is given, which takes the error of the representation of r too
  void series_of_the_balls()
  {
    for (const GValue& v : g_values) {
      const cm1::U128 R = cm1::mk(v.r1, v.r0), G = cm1::g_fixed(R, v.negative != 0), ref = cm1::mk(v.g1, v.g0);
      u64 borrow;
      const cm1::U128 difference = cm1::lt(G, ref) ? cm1::sub(ref, G, borrow) : cm1::sub(G, ref, borrow);
      check("the series of exp(r) - 1 is within 4 units of 2^-127 of the value", difference.h == 0 && difference.l <= 4,
            [&] { return "R = " + text(R) + ": the error is " + std::to_string(low_word(difference)) + " units"; });
    }
  }

  bool unknown(const cm1::Num& a)
  {
    return cm1::mag_is_inf(a.r);
  }

  // A ball that its precondition does not allow is unknown (an infinite radius), whatever it is given
  void preconditions()
  {
    check("exp(r) - 1 of |r| >= 1/2 is unknown", unknown(cm1::expm1_small(cm1::num_of_double(0.75)))
          && unknown(cm1::expm1_small(cm1::num_of_double(-0.5))) && unknown(cm1::expm1_small(cm1::num_of_double(3.0))));
    check("exp(r) - 1 of |r| next to 1/2 is known", !unknown(cm1::expm1_small(cm1::num_of_double(0.49))));
    cm1::Num wide = cm1::num_of_double(0.25);
    wide.r = cm1::mag_pow2(-5);
    check("exp(r) - 1 of a ball of a radius 2^-5 is unknown", unknown(cm1::expm1_small(wide)));
    wide.r = cm1::mag_pow2(-12);
    check("exp(r) - 1 of a ball of a radius 2^-12 is known, and holds exp(r) - 1 at both ends",
          !unknown(cm1::expm1_small(wide)) && cm1::mag_le(cm1::expm1_small(wide).r, cm1::mag_pow2(-10)));
    cm1::Num zero = cm1::num_zero(cm1::mag_pow2(-20));
    const cm1::Num around_zero = cm1::expm1_small(zero);
    check("exp(t) - 1 of 0 +- 2^-20 holds exp(2^-20) - 1, a little more than 2^-20",
          cm1::mag_le(cm1::mag_add(cm1::mag_pow2(-20), cm1::mag_pow2(-41)), around_zero.r) && !unknown(around_zero));
    zero.r = cm1::mag_pow2(-2);
    check("exp(t) - 1 of 0 +- 1/4 is unknown", unknown(cm1::expm1_small(zero)));
    check("exp(t) - 1 of |t| >= 2^11 is unknown", unknown(cm1::expm1_ball(cm1::num_of_double(4096.0)))
          && unknown(cm1::expm1_ball(cm1::num_of_double(-3000.0))));
    check("exp(t) - 1 of 2047 is known", !unknown(cm1::expm1_ball(cm1::num_of_double(2047.0))));
    // log(1 + x) from a y0 that is not next to it: d = (1 + x) exp(-y0) - 1 is above 2^-8
    check("log(1 + x) from a y0 far from it is unknown",
          unknown(cm1::log1p_ball(1.0, 2.0*gaol_cr_log1p(1.0))) && unknown(cm1::log1p_ball(3.0, 1.0)));
    check("log(1 + x) from a y0 within 2^-9 of it is known", !unknown(cm1::log1p_ball(1.0, gaol_cr_log1p(1.0)*(1.0 + 0x1p-9))));
  }

  void rounding_of_balls()
  {
    struct Case { const char* what; double center; bool radius; int exponent; double lo, hi; };
    const double max = std::numeric_limits<double>::max(), infinity = gaol_tests::inf;
    const Case cases[] = {
      { "0 +- 0", 0.0, false, 0, 0.0, 0.0 },
      { "0 +- 2^-3", 0.0, true, -3, -0.125, 0.125 },
      { "1 +- 1/2: exact ends", 1.0, true, -1, 0.5, 1.5 },
      { "-1 +- 1/2: exact ends", -1.0, true, -1, -1.5, -0.5 },
      { "1 +- 1: as wide as its center: the whole line", 1.0, true, 0, -infinity, infinity },
      { "-1 +- 2: wider than its center: the whole line", -1.0, true, 1, -infinity, infinity },
      { "1 +- 2^-60: the doubles around 1", 1.0, true, -60, 1.0 - 0x1p-53, 1.0 + 0x1p-52 },
      { "-1 +- 2^-60", -1.0, true, -60, -1.0 - 0x1p-52, -1.0 + 0x1p-53 },
      { "3 +- 2^-1: exact ends", 3.0, true, -1, 2.5, 3.5 },
      { "2^-1070 +- 2^-1074: subnormal exact ends", 0x1p-1070, true, -1074, 0x1.ep-1071, 0x1.1p-1070 },
      { "2^-1074 +- 2^-1076: the lower end rounds to 0", 0x1p-1074, true, -1076, 0.0, 0x1p-1073 },
      { "-2^-1074 +- 2^-1076: the upper end rounds to 0", -0x1p-1074, true, -1076, -0x1p-1073, 0.0 },
      { "the largest double +- 2^900: the upper end is past it", max, true, 900, std::nextafter(max, 0.0), infinity },
      { "-(the largest double) +- 2^900", -max, true, 900, -infinity, -std::nextafter(max, 0.0) },
      { "1 +- 2^100: a ball too wide for its grid", 1.0, true, 100, -infinity, infinity },
    };
    for (const Case& c : cases) {
      cm1::Num ball = cm1::num_of_double(c.center);
      ball.r = c.radius ? cm1::mag_pow2(c.exponent) : cm1::mag_zero();
      double lo, hi;
      bool lo_exact, hi_exact;
      cm1::round_ball(ball, lo, hi, lo_exact, hi_exact);
      check("the doubles around a ball: the greatest at most its lower end, the least at least its upper end",
            lo == c.lo && hi == c.hi, [&] { return std::string(c.what) + ": [" + show(lo) + ", " + show(hi) + "] rather than [" + show(c.lo) + ", " + show(c.hi) + "]"; });
    }
    cm1::Num unknown_ball = cm1::num_unknown();
    double lo, hi;
    bool lo_exact, hi_exact;
    cm1::round_ball(unknown_ball, lo, hi, lo_exact, hi_exact);
    check("the doubles around an unknown ball: the whole line", lo == -gaol_tests::inf && hi == gaol_tests::inf);
    // The ends that are doubles are told: 1 +- 2^-3
    cm1::Num exact_ends = cm1::num_of_double(1.0);
    exact_ends.r = cm1::mag_pow2(-3);
    cm1::round_ball(exact_ends, lo, hi, lo_exact, hi_exact);
    check("the ends of 1 +- 2^-3 are doubles", lo_exact && hi_exact && lo == 0.875 && hi == 1.125);
    exact_ends.r = cm1::mag_pow2(-60);
    cm1::round_ball(exact_ends, lo, hi, lo_exact, hi_exact);
    check("the ends of 1 +- 2^-60 are no doubles", !lo_exact && !hi_exact);
  }

  // power_equals() and product_equals() against fractions: no equality is proved that does not hold, and every one
  // that holds with room in 128 bits is. A false equality gives the bounds [d, d], which do not enclose the value.
  void exact_equalities()
  {
    for (const PowerTriple& t : power_triples) {
      cm1::Dyadic w;
      bool known;
      if (t.value == 0) {
        known = t.d > -1.0 && cm1::one_plus(t.d, w);
      } else {
        known = t.d > 0.0;
        if (known) {
          w = cm1::dyadic_of(t.d);
        }
      }
      if (!known) {
        continue; // 1 + d takes more than 127 bits: nothing is proved
      }
      const bool got = cm1::power_equals(t.x, t.y, w);
      const std::string what = std::string("(1 + ") + show(t.x) + ")^" + show(t.y) + (t.value == 0 ? " = 1 + " : " = ") + show(t.d);
      check("power_equals proves no equality that does not hold", !got || t.expected != 0, [&] { return what + " is not exact"; });
      if (t.expected != 0 && t.complete != 0) {
        check("power_equals proves the equalities that hold with 128 bits to spare", got, [&] { return what + " is exact"; });
      }
    }
    for (const ProductTriple& t : product_triples) {
      const bool got = cm1::product_equals(t.x, t.y, t.d);
      check("product_equals is x y = d, exactly", got == (t.expected != 0),
            [&] { return show(t.x) + " * " + show(t.y) + (t.expected != 0 ? " is " : " is not ") + show(t.d); });
    }
  }

  //-------------------------------------------------------------------------
  // The reciprocal and the fast phase
  //-------------------------------------------------------------------------

  // clz64() is the instruction of the compiler where there is one, clz64_soft() the bisection: the same count
  void leading_zeros()
  {
    Random rnd;
    std::vector<u64> values = { 1, 2, 3, 0x7fffffffffffffffull, 0x8000000000000000ull, ~static_cast<u64>(0), 0x100000000ull, 0xffffffffull };
    for (int i = 0; i < 2000; ++i) {
      values.push_back((rnd.bits() | 1u) >> rnd.integer(0, 63));
    }
    for (u64 v : values) {
      if (v == 0) { continue; }
      int n = 0;
      while (((v << n) >> 63) == 0) { ++n; }
      check("clz64() counts the leading zeros", cm1::clz64(v) == n && cm1::clz64_soft(v) == n,
            [&] { return std::to_string(v) + ": " + std::to_string(cm1::clz64(v)) + ", " + std::to_string(cm1::clz64_soft(v)) + " rather than " + std::to_string(n); });
    }
  }

  // g64(R) is (exp(r) - 1) / r 2^63 within 5 units, against the values of mpmath (to 127 bits, rounded down: one unit more)
  void series_of_the_fast_phase()
  {
    for (const GValue& v : g_values) {
      const u64 R = v.r1; // r = +-R / 2^64, within 2^-64 of the r of the table
      const u64 got = cm1::g64(R, v.negative != 0);
      const u64 distance = got > v.g1 ? got - v.g1 : v.g1 - got;
      check("g64() is within 5 units of 2^-63 of (exp(r) - 1) / r, 6 for the reference rounded down and R cut",
            distance <= 6, [&] { return "R = " + std::to_string(R) + ": " + std::to_string(got) + " rather than " + std::to_string(v.g1); });
    }
    // And against the series of the balls, to 127 bits, for every size of R up to 2^63 (r = 1/2), 5 units exactly
    Random rnd;
    std::vector<u64> rs = { 0, 1, 2, 3, static_cast<u64>(1) << 62, (static_cast<u64>(1) << 63) - 1, static_cast<u64>(1) << 63 };
    for (int i = 0; i < 4000; ++i) {
      rs.push_back((rnd.bits() >> 1) >> rnd.integer(0, 62));
    }
    for (u64 R : rs) {
      for (int neg = 0; neg < 2; ++neg) {
        const cm1::U128 ref = cm1::g_fixed(cm1::mk(R, 0), neg != 0); // G 2^127, within 2^-123
        const u64 got = cm1::g64(R, neg != 0);
        // |got 2^64 - ref| <= 5 2^64, with the 2^-123 of the reference (2^-60 of a unit of 2^-63) as a margin
        u64 borrow;
        const cm1::U128 g128 = cm1::mk(got, 0);
        const cm1::U128 diff = cm1::lt(g128, ref) ? cm1::sub(ref, g128, borrow) : cm1::sub(g128, ref, borrow);
        check("g64() is within 5 units of 2^-63 of the series of the balls", diff.h < 5 || (diff.h == 5 && diff.l <= 16),
              [&] { return "R = " + std::to_string(R) + (neg != 0 ? " (negative): " : ": ") + text(diff); });
      }
    }
  }

  // fast_reduce(): the multiple of log(2) that is nearest, and what is left of it
  void reduction_of_the_fast_phase()
  {
    Random rnd;
    const cm1::U128 M = cm1::ln2_120();
    for (int i = 0; i < 6000; ++i) {
      // T = |v| 2^120 < 2^123, from 2^100 on
      cm1::U128 T = cm1::shr(cm1::mk(rnd.bits(), rnd.bits()), 5 + rnd.integer(0, 25));
      if (i % 7 == 0) {
        // next to (2 n + 1) log(2) / 2, n = 0 to 11: the nearest multiple changes there
        const int n = rnd.integer(0, 11);
        u64 carry;
        T = cm1::shr(M, 1);
        for (int j = 0; j < n; ++j) { T = cm1::add(T, M, carry); }
        const cm1::U128 one_or_few = cm1::mk(0, static_cast<u64>(1 + rnd.integer(0, 3)));
        T = (i % 2 == 0) ? T : cm1::sub(T, one_or_few, carry);
        T = (i % 3 == 0) ? cm1::add(T, one_or_few, carry) : T;
      }
      int k;
      bool sneg;
      u64 R;
      cm1::fast_reduce(T, k, sneg, R);
      // |T - k M| <= M / 2 and the nearest: T = k M +- |s|
      u64 c;
      cm1::U128 kM = cm1::mk(0, 0);
      for (int j = 0; j < k; ++j) { kM = cm1::add(kM, M, c); }
      cm1::U128 s;
      u64 borrow;
      if (sneg) {
        s = cm1::sub(kM, T, borrow);
        check("fast_reduce: T below k log(2) when s is negative", borrow == 0);
      } else {
        s = cm1::sub(T, kM, borrow);
        check("fast_reduce: T at least k log(2) when s is not negative", borrow == 0);
      }
      const cm1::U128 twice = cm1::shl(s, 1);
      check("fast_reduce: |s| is at most log(2) / 2", !cm1::lt(M, twice), [&] { return text(T); });
      check("fast_reduce: R is |s| 2^-56", R == cm1::shr(s, 56).l && cm1::shr(s, 56).h == 0, [&] { return text(T); });
      check("fast_reduce: k is at most 12", k >= 0 && k <= 12, [&] { return text(T) + ": " + std::to_string(k); });
    }
  }

  // norm64() and top64(), and the constant log(2) 2^120 of mpmath
  void helpers_of_the_fast_phase()
  {
    Random rnd;
    check("log(2) 2^120 is the one of mpmath", cm1::ln2_120().h == ln2_120_high && cm1::ln2_120().l == ln2_120_low,
          [&] { return text(cm1::ln2_120()); });
    std::vector<double> values = { 1.0, 0.5, 3.0, 0x1p-1074, 0x1.fffffffffffffp-1023, 0x1p-1022, 1e300, 0x1.8p-1070, 7.0e-310, 0.1 };
    for (int i = 0; i < 400; ++i) {
      values.push_back(rnd(-1074, 1023) * (rnd.integer(0, 1) != 0 ? 1.0 : -1.0));
    }
    for (double v : values) {
      if (v == 0.0 || !std::isfinite(v)) { continue; }
      bool neg;
      u64 m;
      int e;
      cm1::norm64(v, neg, m, e);
      const double back = std::ldexp(static_cast<double>(m >> 11) * 2048.0 + static_cast<double>(m & 2047u), e) * (neg ? -1.0 : 1.0);
      check("norm64(): a 64-bit significand with its top bit set, and the number", (m >> 63) == 1 && neg == (v < 0.0) && (m & 2047u) == 0 && back == v,
            [&] { return show(v) + ": " + text(cm1::mk(m, 0)) + " 2^" + std::to_string(e); });
    }
    for (int i = 0; i < 2000; ++i) {
      const u64 hi = (i % 5 == 0) ? 0 : (rnd.bits() >> rnd.integer(0, 63)), lo = rnd.bits() | 1u;
      u64 m;
      int z;
      cm1::top64(hi, lo, m, z);
      // The 128 bits shifted left by z have m for their 64 high bits, the top bit set
      cm1::U128 shifted = cm1::shl(cm1::mk(hi, lo), z);
      check("top64(): the 64 high bits of a number of 128 bits, shifted to the top", (m >> 63) == 1 && shifted.h == m && z >= 0 && z < 128,
            [&] { return text(cm1::mk(hi, lo)) + ": " + std::to_string(z); });
    }
  }

  // fast_doubles(): the enclosure [m - err, m + err] 2^fe holds no double (m has 11 more bits than a double)
  void doubles_of_the_fast_phase()
  {
    const u64 top = static_cast<u64>(1) << 63;
    struct Case { const char* what; bool neg; u64 m; int fe; u64 rho; bool answers; double lo, hi; };
    // err = floor(rho m / 2^63) + 2 units, 2^11 units to a double: for rho = 1 and m = 1.5 2^63, err = 3 units
    const Case cases[] = {
      { "inside of the doubles around 1.5", false, top + (top >> 1) + 0x400, -63, 1, true, 1.5, 1.5 + 0x1p-52 },
      { "negative: the bounds are opposite and swapped", true, top + (top >> 1) + 0x400, -63, 1, true, -(1.5 + 0x1p-52), -1.5 },
      { "the lower end of the enclosure is a double", false, top + (top >> 1) + 3, -63, 1, false, 0.0, 0.0 },
      { "the lower end is just above a double", false, top + (top >> 1) + 4, -63, 1, true, 1.5, 1.5 + 0x1p-52 },
      { "the upper end is the next double", false, top + (top >> 1) + 0x800 - 3, -63, 1, false, 0.0, 0.0 },
      { "the upper end is just below the next double", false, top + (top >> 1) + 0x800 - 4, -63, 1, true, 1.5, 1.5 + 0x1p-52 },
      { "the enclosure goes below 2^63 (a power of two)", false, top + 2, -63, 1, false, 0.0, 0.0 },
      { "the enclosure goes past 2^64 (a power of two)", false, ~static_cast<u64>(0) - 2, -63, 1, false, 0.0, 0.0 },
      { "a larger rho widens it: the enclosure goes over a double", false, top + (top >> 1) + 0x400, -63, 800, false, 0.0, 0.0 },
      { "a larger rho that still fits", false, top + (top >> 1) + 0x400, -63, 300, true, 1.5, 1.5 + 0x1p-52 },
      { "the smallest normal double", false, top + 0x400, -1085, 1, true, 0x1p-1022, 0x1p-1022 + 0x1p-1074 },
      { "a subnormal double is not told", false, top + 0x400, -1086, 1, false, 0.0, 0.0 },
      { "the largest doubles: the least double above is the infinity", false, ~static_cast<u64>(0) - 0x400, 960, 1, true, 0x1.fffffffffffffp+1023, gaol_tests::inf },
      { "past the largest double", false, top + 0x400, 961, 1, false, 0.0, 0.0 },
    };
    for (const Case& c : cases) {
      double lo = 0.0, hi = 0.0;
      const bool got = cm1::fast_doubles(c.neg, c.m, c.fe, c.rho, lo, hi);
      check("fast_doubles(): the doubles around the enclosure, or no answer where it holds one", got == c.answers && (!got || (lo == c.lo && hi == c.hi)),
            [&] { return std::string(c.what) + ": " + (got ? "[" + show(lo) + ", " + show(hi) + "]" : std::string("no answer")); });
    }
  }

  // The bounds of the error of the fast phase, as the analysis of the header gives them (27 to 200 units of 2^-63)
  void bounds_of_the_fast_phase()
  {
    struct Case { int k; int tube; cm1::u64 base; cm1::u64 rho; };
    const Case cases[] = {
      { 0, 0, 14, 27 }, { 0, 0, 20, 35 }, { 1, 0, 14, 48 }, { 1, 0, 20, 60 }, { 1, 1, 14, 62 }, { 2, 1, 20, 80 },
      { 3, 2, 14, 90 }, { 4, 2, 20, 120 }, { 5, 3, 14, 146 }, { 12, 3, 20, 200 },
    };
    for (const Case& c : cases) {
      check("fast_rho(): the bound of the analysis", cm1::fast_rho(c.k, c.tube, c.base) == c.rho,
            [&] { return "k = " + std::to_string(c.k) + ", |t| < 2^" + std::to_string(c.tube) + ", base " + std::to_string(c.base) + ": " + std::to_string(cm1::fast_rho(c.k, c.tube, c.base)); });
    }
    // The classes of the pairs: log1p(x) below 1/2 or above (base 14 or 20), t below log(2)/2 or past it, the sign of t
    const struct { const char* what; double x, y; cm1::u64 rho; } pairs[] = {
      { "log1p(x) = 0.095, t = 0.14", 0.1, 1.5, 27 },
      { "log1p(x) = 1.1, t = 0.11", 2.0, 0.1, 35 },
      { "log1p(x) = 0.095, t = 0.57: k = 1, |t| < 1", 0.1, 6.0, 48 },
      { "log1p(x) = 1.1, t = 0.77: k = 1, |t| < 1", 2.0, 0.7, 60 },
      { "log1p(x) = 0.095, t = -0.57: k = -1", 0.1, -6.0, 48 },
      { "log1p(x) = 0.095, t = 1.9: k = 3, |t| < 2", 0.1, 20.0, 62 },
      { "log1p(x) = 1.1, t = 3.3: k = 5, |t| < 4", 2.0, 3.0, 120 },
      { "log1p(x) = 1.1, t = 6.6: k = 10, |t| < 8", 2.0, 6.0, 200 },
    };
    for (const auto& p : pairs) {
      bool fneg;
      cm1::u64 fm, rho;
      int fe;
      const bool got = cm1::fast_centre(p.x, p.y, fneg, fm, fe, rho);
      check("fast_centre(): the class of the pair gives its bound", got && rho == p.rho,
            [&] { return std::string(p.what) + ": " + (got ? std::to_string(rho) : std::string("no centre")); });
    }
  }

  // The relative error of the centre of the fast phase against the balls, in units of rho: 0 to 1, or -1 where it does not answer
  double fast_error(double x, double y)
  {
    bool fneg;
    cm1::u64 fm, rho;
    int fe;
    if (!cm1::fast_centre(x, y, fneg, fm, fe, rho)) { return -1.0; }
    const double y0 = gaol_cr_log1p(x);
    const cm1::Num T = cm1::num_mul(cm1::num_of_double(y), cm1::log1p_ball(x, y0));
    const cm1::Num F = cm1::expm1_ball(T);
    const cm1::Num centre = cm1::num_of_u128(fneg, cm1::mk(0, fm), fe);
    const cm1::Num D = cm1::num_sub(F, centre);
    if (cm1::num_is_zero(D)) { return 0.0; }
    if (cm1::num_is_zero(F) || cm1::mag_is_inf(F.r) || cm1::mag_is_inf(D.r)) { return 1e9; }
    const double dd = std::ldexp(static_cast<double>(D.m.h), D.e - 64), ff = std::ldexp(static_cast<double>(F.m.h), F.e - 64);
    return dd / ff * 9223372036854775808.0 / static_cast<double>(rho);
  }

  // The fast phase: the bound of its error holds with a margin of three (the worst error found is 0.22 of it), its answers are those of the balls, it settles
  // most of the pairs of its domain and none outside of it
  void fast_phase()
  {
    Random rnd;
    struct Domain { const char* what; double xlo, xhi, ylo, yhi; double rate; };
    const Domain domains[] = {
      { "x in [-0.3, 0.5], y in [-3, 3]", -0.3, 0.5, -3.0, 3.0, 0.92 },
      { "x in [-0.05, 0.05], y in [-20, 20]", -0.05, 0.05, -20.0, 20.0, 0.92 },
      { "x in [0, 0.1], y in [1, 30]", 0.0, 0.1, 1.0, 30.0, 0.90 },
      { "x in [-0.9, 5], y in [0.1, 3] (log1p past 1/2)", -0.9, 5.0, 0.1, 3.0, 0.84 },
      { "x in [-0.7, -0.4], y in [-1, 1] (the log of 0.3 to 0.6)", -0.7, -0.4, -1.0, 1.0, 0.89 },
      { "x in [5, 2900], y in [-0.5, 0.5] (log1p up to 8)", 5.0, 2900.0, -0.5, 0.5, 0.84 },
      { "x in [-0.9996, -0.92], y in [-1, 1] (log1p from -2.5 to -7.8: 4 to 11 multiples of log(2))", -0.9996, -0.92, -1.0, 1.0, 0.83 },
      { "x in [2000, 2900], y in [-1, 1] (log1p near 8: 11 multiples of log(2))", 2000.0, 2900.0, -1.0, 1.0, 0.73 },
      { "x in [2890, 2979], y in [-1, 1] (log1p up to 8: 12 multiples of log(2))", 2890.0, 2979.0, -1.0, 1.0, 0.74 },
    };
    for (const Domain& d : domains) {
      int answered = 0, total = 0;
      for (int i = 0; i < 2500; ++i) {
        const double x = rnd.uniform(d.xlo, d.xhi), y = rnd.uniform(d.ylo, d.yhi);
        if (x == 0.0 || y == 0.0 || y == 1.0 || y == -1.0) { continue; }
        ++total;
        const double error = fast_error(x, y);
        check("the error of the fast phase is at most 3/10 of its bound", error <= 0.3,
              [&] { return std::string(d.what) + ": (" + show(x) + ", " + show(y) + "): " + std::to_string(error) + " of the bound"; });
        double lo, hi, blo, bhi;
        if (cm1::compoundm1_fast(x, y, lo, hi)) {
          ++answered;
          cm1::compoundm1_balls(x, y, blo, bhi);
          check("the bounds of the fast phase are those of the balls", lo == blo && hi == bhi,
                [&] { return std::string(d.what) + ": (" + show(x) + ", " + show(y) + "): [" + show(lo) + ", " + show(hi) + "] and the balls' [" + show(blo) + ", " + show(bhi) + "]"; });
        }
      }
      check("the fast phase settles most of the pairs of its domain", answered >= d.rate * total,
            [&] { return std::string(d.what) + ": " + std::to_string(answered) + " of " + std::to_string(total); });
    }
    // The domain: |log1p(x)| <= 8, |y log1p(x)| < 8, a result of the normal doubles; and no answer where the value is a double
    const struct { const char* what; double x, y; bool answers; } edges[] = {
      { "log1p(x) = 8.007", 3000.0, 0.001, false },
      { "log1p(x) = -8.11", -0.9997, 0.5, false },
      { "y log1p(x) = 16", 0.5, 40.0, false },
      { "y log1p(x) = -16", 0.5, -40.0, false },
      { "a subnormal value", 0x1p-1060, 0.5, false },
      { "x subnormal and y 1.5", 0x1p-1070, 1.5, false },
      { "(0.5)^3 - 1 = 2.375 is a double", 0.5, 3.0, false },
      { "(2)^0.5 - 1 is not", 2.0, 0.5, true },
      { "log1p(x) = 0.405, y = 2.5", 0.5, 2.5, true },
      { "log1p(x) = -0.693, y = 3", -0.5, 3.0, false }, // (0.5)^3 - 1 = -0.875 is a double
      { "x = 0.7 and y = 11 (the value is 1.4 10^2, not a double)", 0.7, 11.0, true },
      { "x = -0.7 and y = 2.5 (|t| = 3)", -0.7, 2.5, true },
      { "y log1p(x) = 12.2", 0.5, 30.0, false },
      { "y log1p(x) = -12.2", 0.5, -30.0, false },
      { "y log1p(x) = 9.7", -0.7, -8.1, false },
    };
    for (const auto& e : edges) {
      double lo, hi;
      const bool got = cm1::compoundm1_fast(e.x, e.y, lo, hi);
      check("the fast phase answers inside of its domain, and nowhere else", got == e.answers,
            [&] { return std::string(e.what) + ": " + (got ? "answers [" + show(lo) + ", " + show(hi) + "]" : std::string("does not answer")); });
    }
  }

  // reciprocal_m1() is the exact division: the tightest bounds of the points of y = -1 (the contrived values among them),
  // wherever 1 + x takes 127 bits or less
  void reciprocals()
  {
    int divided = 0;
    for (const CompoundPoint& p : compound_points) {
      if (p.y != -1.0) { continue; }
      cm1::Dyadic s;
      const bool representable = cm1::one_plus(p.x, s);
      double lo, hi;
      const bool done = cm1::reciprocal_m1(p.x, lo, hi);
      check("reciprocal_m1() divides where 1 + x takes 127 bits or less, and nowhere else", done == representable,
            [&] { return show(p.x) + (done ? " is divided" : " is not divided"); });
      if (done) {
        ++divided;
        check("reciprocal_m1(): the tightest bounds of 1/(1 + x) - 1", lo == p.below && hi == p.above,
              [&] { return show(p.x) + ": [" + show(lo) + ", " + show(hi) + "] rather than [" + show(p.below) + ", " + show(p.above) + "]"; });
      }
    }
    check("the points of y = -1 are divided, most of them", divided > 600, [&] { return std::to_string(divided); });
  }

  //-------------------------------------------------------------------------
  // Points
  //-------------------------------------------------------------------------

  void points()
  {
    for (const CompoundPoint& p : compound_points) {
      const interval r = compoundm1(interval(p.x), interval(p.y));
      // A bound that is 0 is +0 (the reader of the result of the interval would not care, but the function of points does)
      double plo, phi;
      cm1::compoundm1_point(p.x, p.y, plo, phi);
      check("a bound of compoundm1_point() that is 0 is +0", !(plo == 0.0 && std::signbit(plo)) && !(phi == 0.0 && std::signbit(phi)),
            [&] { return "(" + show(p.x) + ", " + show(p.y) + "): " + show(plo) + ", " + show(phi); });
      const std::string what = "compoundm1([" + show(p.x) + "], [" + show(p.y) + "]) = " + hex(r) + " rather than ["
        + show(p.below) + ", " + show(p.above) + "]";
      check("compoundm1 at a point: the tightest bounds", r.left() == p.below && r.right() == p.above, [&] { return what; });
      double lo, hi;
      cm1::compoundm1_point(p.x, p.y, lo, hi);
      check("compoundm1_point(): the tightest bounds", lo == p.below && hi == p.above,
            [&] { return what + " (the point function)"; });
    }
  }

  //-------------------------------------------------------------------------
  // Boxes
  //-------------------------------------------------------------------------

  void boxes()
  {
    for (const CompoundBox& b : compound_boxes) {
      const interval x(b.xl, b.xu), y(b.yl, b.yu);
      const interval r = compoundm1(x, y);
      const std::string what = "compoundm1(" + hex(x) + ", " + hex(y) + ") = " + hex(r);
      if (b.empty != 0) {
        check("compoundm1 on a box outside the domain: the empty set", r.is_empty(), [&] { return what; });
      } else {
        check("compoundm1 on a box: the hull of the corners and of 0", !r.is_empty() && r.left() == b.below && r.right() == b.above,
              [&] { return what + " rather than [" + show(b.below) + ", " + show(b.above) + "]"; });
      }
    }
  }

  void chosen_boxes()
  {
    struct Case { const char* what; interval got; double lo, hi; bool empty; };
    const double inf = gaol_tests::inf;
    const Case cases[] = {
      { "an empty x", compoundm1(interval::emptyset(), interval(1.0)), 0.0, 0.0, true },
      { "an empty y", compoundm1(interval(1.0), interval::emptyset()), 0.0, 0.0, true },
      { "x < -1", compoundm1(interval(-3.0, -1.5), interval(1.0, 2.0)), 0.0, 0.0, true },
      { "x = -1 and y > 0", compoundm1(interval(-1.0), interval(0.5, 3.0)), -1.0, -1.0, false },
      { "x = -1 and y = 0 (0^0 has no value)", compoundm1(interval(-1.0), interval(0.0)), 0.0, 0.0, true },
      { "x = -1 and y < 0", compoundm1(interval(-1.0), interval(-2.0, -0.5)), 0.0, 0.0, true },
      { "x = -1 and y holding 0 and 1: only y > 0 counts", compoundm1(interval(-1.0), interval(-1.0, 1.0)), -1.0, -1.0, false },
      { "x = [-1, 0] and y = 1: -1 is the least value", compoundm1(interval(-1.0, 0.0), interval(1.0)), -1.0, 0.0, false },
      { "x = [-1, 0] and y = -1: the limit +oo at x = -1", compoundm1(interval(-1.0, 0.0), interval(-1.0)), 0.0, inf, false },
      { "x = [-1, 3] and y = [-2, 2]: [-1, +oo]", compoundm1(interval(-1.0, 3.0), interval(-2.0, 2.0)), -1.0, inf, false },
      { "x = [-3, 3] and y = 2: x < -1 left out", compoundm1(interval(-3.0, 3.0), interval(2.0)), -1.0, 15.0, false },
      { "x = 0, whatever y", compoundm1(interval(0.0), interval(-1e300, 1e300)), 0.0, 0.0, false },
      { "y = 0, whatever x", compoundm1(interval(-0.5, 1e300), interval(0.0)), 0.0, 0.0, false },
      { "the whole line: [-1, +oo]", compoundm1(interval::universe(), interval::universe()), -1.0, inf, false },
      { "x = [0, +oo] and y = 1: x - 0", compoundm1(interval(0.0, inf), interval(1.0)), 0.0, inf, false },
      { "x = [1, +oo] and y = [-oo, 0]: the limit -1 at +oo, and 0", compoundm1(interval(1.0, inf), interval(-inf, -1.0)), -1.0, -0.5, false },
      { "x = 1 and y = [1, +oo]: the limit +oo", compoundm1(interval(1.0), interval(1.0, inf)), 1.0, inf, false },
      { "x = 1 and y = [-oo, -1]: the limit -1", compoundm1(interval(1.0), interval(-inf, -1.0)), -1.0, -0.5, false },
      { "x = -0.5 and y = [1, +oo]: the limit -1", compoundm1(interval(-0.5), interval(1.0, inf)), -1.0, -0.5, false },
      { "x = -0.5 and y = [-oo, -1]: the limit +oo", compoundm1(interval(-0.5), interval(-inf, -1.0)), 1.0, inf, false },
      { "x = 1 and y = 3: 7", compoundm1(interval(1.0), interval(3.0)), 7.0, 7.0, false },
      { "x = 0.5 and y = 2: 1.25", compoundm1(interval(0.5), interval(2.0)), 1.25, 1.25, false },
      { "x = 8 and y = 0.5: 2", compoundm1(interval(8.0), interval(0.5)), 2.0, 2.0, false },
      { "x = 3 and y = -1: -0.75", compoundm1(interval(3.0), interval(-1.0)), -0.75, -0.75, false },
      { "x = -0.75 and y = -0.5: 1", compoundm1(interval(-0.75), interval(-0.5)), 1.0, 1.0, false },
      { "x = 1.25 and y = 0.5: 0.5", compoundm1(interval(1.25), interval(0.5)), 0.5, 0.5, false },
      { "x = 2^-1074 and y = 1: the same", compoundm1(interval(0x1p-1074), interval(1.0)), 0x1p-1074, 0x1p-1074, false },
      { "x = 15 and y = 25: 2^100 - 1 is between two doubles", compoundm1(interval(15.0), interval(25.0)), 0x1.fffffffffffffp+99, 0x1p+100, false },
      { "x = [3, 7] and y = [1, 2]: [3, 63]", compoundm1(interval(3.0, 7.0), interval(1.0, 2.0)), 3.0, 63.0, false },
      { "x = 1 and y = 2000: 2^2000 - 1 is past the doubles", compoundm1(interval(1.0), interval(2000.0)), std::numeric_limits<double>::max(), inf, false },
      { "x = 1 and y = -2000: -1 + 2^-2000", compoundm1(interval(1.0), interval(-2000.0)), -1.0, -0x1.fffffffffffffp-1, false },
    };
    for (const Case& c : cases) {
      if (c.empty) {
        check("compoundm1 on chosen boxes: the empty set", c.got.is_empty(), [&] { return std::string(c.what) + ": " + hex(c.got); });
      } else {
        check("compoundm1 on chosen boxes", !c.got.is_empty() && c.got.left() == c.lo && c.got.right() == c.hi,
              [&] { return std::string(c.what) + ": " + hex(c.got) + " rather than [" + show(c.lo) + ", " + show(c.hi) + "]"; });
      }
    }
  }

  // The contrived values the balls cannot tell apart, y = -1 and x = 2^e + 2^(2e-1), whose 1/(1 + x) has no cubic term and is
  // within 2^-155 of a double (the tightest bounds of x = 2^-51 + 2^-103 are [-0x1.fffffffffffffp-52, -0x1.ffffffffffffep-52],
  // and the ball holds the double -0x1.ffffffffffffep-52, which it cannot tell from the value): reciprocal_m1() divides the
  // integers and gives the tightest bounds, which is checked against the balls' enclosure too (compoundm1_balls() is one
  // double wider on one side, no more, and holds them) for every e where x is a double
  void contrived_value()
  {
    const interval r = compoundm1(interval(0x1.0000000000001p-51), interval(-1.0));
    check("compoundm1(2^-51 + 2^-103, -1) gives the tightest bounds",
          r.left() == -0x1.fffffffffffffp-52 && r.right() == -0x1.ffffffffffffep-52, [&] { return hex(r); });
    int wider = 0;
    for (int e = -51; e <= -30; ++e) {
      for (int sign = -1; sign <= 1; sign += 2) {
        const double x = sign * (std::ldexp(1.0, e) + std::ldexp(1.0, 2*e - 1));
        double lo, hi, blo, bhi;
        cm1::compoundm1_point(x, -1.0, lo, hi);
        cm1::compoundm1_balls(x, -1.0, blo, bhi);
        check("the contrived x = 2^e + 2^(2e-1), y = -1: the balls hold the tightest bounds, within a double of them",
              blo <= lo && hi <= bhi && std::nextafter(lo, -gaol_tests::inf) <= blo && bhi <= std::nextafter(hi, gaol_tests::inf),
              [&] { return show(x) + ": [" + show(lo) + ", " + show(hi) + "] and the balls' [" + show(blo) + ", " + show(bhi) + "]"; });
        if (blo != lo || bhi != hi) { ++wider; }
      }
    }
    // The balls are wider for some of them (that is why there is a division), and the division is tight for all
    check("the balls alone are one double wider for the contrived values of y = -1", wider > 0);
  }

  // Random boxes with finite bounds, whose result is the hull of the values at the corners and at 0 where
  // the box crosses x = 0 or y = 0, and holds the values at points inside
  void random_boxes()
  {
    Random rnd;
    for (int i = 0; i < 400; ++i) {
      double xs[2], ys[2];
      for (int k = 0; k < 2; ++k) {
        xs[k] = (i % 5 == 0) ? rnd.uniform(-0.999, 0.001) : ((i % 5 == 1) ? rnd.uniform(-0.5, 4.0) : ((i % 5 == 2) ? rnd.uniform(-1e-6, 1e-6)
                : ((i % 5 == 3) ? rnd.uniform(0.0, 100.0) : rnd.uniform(-0.9, 0.0))));
        ys[k] = (i % 3 == 0) ? rnd.uniform(-8.0, 8.0) : ((i % 3 == 1) ? rnd.uniform(0.0, 3.0) : rnd.uniform(-3.0, 0.0));
      }
      const interval x = hull(xs[0], xs[1]), y = hull(ys[0], ys[1]);
      const interval r = compoundm1(x, y);
      // The corners
      double low = inf, high = -inf;
      for (double cx : { x.left(), x.right() }) {
        for (double cy : { y.left(), y.right() }) {
          const interval p = compoundm1(interval(cx), interval(cy));
          low = std::fmin(low, p.left());
          high = std::fmax(high, p.right());
        }
      }
      if ((x.left() <= 0.0 && x.right() >= 0.0) || (y.left() <= 0.0 && y.right() >= 0.0)) {
        low = std::fmin(low, 0.0);
        high = std::fmax(high, 0.0);
      }
      check("compoundm1 on a random box: the hull of the corners and of 0", r.left() == low && r.right() == high,
            [&] { return hex(x) + ", " + hex(y) + ": " + hex(r) + " rather than [" + show(low) + ", " + show(high) + "]"; });
      // Points inside
      for (int k = 0; k < 8; ++k) {
        const double px = std::fmin(std::fmax(rnd.uniform(x.left(), x.right()), x.left()), x.right());
        const double py = std::fmin(std::fmax(rnd.uniform(y.left(), y.right()), y.left()), y.right());
        const interval p = compoundm1(interval(px), interval(py));
        check("compoundm1 on a random box holds the values at the points inside", r.left() <= p.left() && p.right() <= r.right(),
              [&] { return hex(r) + " does not hold " + hex(p) + " at (" + show(px) + ", " + show(py) + ")"; });
      }
    }
  }

  //-------------------------------------------------------------------------
  // Names, text, flags, flush modes
  //-------------------------------------------------------------------------

  void names_and_text()
  {
    const interval x(0.5, 2.0), y(-1.0, 3.0);
    const interval a = gaol_core::compoundm1(x, y), b = gaol::compoundm1(x, y), c = gaol_ieee1788::compoundm1(x, y);
    check("gaol_core::compoundm1, gaol::compoundm1 and gaol_ieee1788::compoundm1 are one function",
          a.set_eq(b) && a.set_eq(c), [&] { return hex(a) + hex(b) + hex(c); });
    check("compoundm1([0.5, 2], [-1, 3]) has 26 for its upper bound, at (2, 3)", !a.is_empty() && a.right() == 26.0 && a.left() < 0.0,
          [&] { return hex(a); });
    const interval t1 = gaol::textToInterval("compoundm1(1, 3)"), t2 = gaol::textToInterval("COMPOUNDM1([0.5, 2], [-1, 3])");
    check("the text compoundm1(1, 3) is [7]", t1.set_eq(interval(7.0)), [&] { return hex(t1); });
    check("the text COMPOUNDM1(x, y) reads the function, whatever the case", t2.set_eq(a), [&] { return hex(t2); });
    check("the text compoundm1 of an expression: compoundm1(2 - 1, 1 + 2)", gaol::textToInterval("compoundm1(2 - 1, 1 + 2)").set_eq(interval(7.0)));
  }

  void exception_flags()
  {
    const double inf = gaol_tests::inf;
    const interval boxes[][2] = {
      { interval(1.0), interval(3.0) }, { interval(-0.5, 2.0), interval(-2.0, 5.0) }, { interval(-1.0, 1.0), interval(0.0, 1.0) },
      { interval(1.0, inf), interval(-inf, 2.0) }, { interval::universe(), interval::universe() },
      { interval(0.5), interval(1e300) }, { interval(1e300), interval(-1e300) }, { interval::emptyset(), interval(1.0) },
      { interval(2.0), interval::emptyset() }, { interval(-1.0), interval(1.0) }, { interval(-3.0), interval(1.0) },
      { interval(-0.999999), interval(1e10) }, { interval(1e-300), interval(1e-300) }, { interval(1.0), interval(2000.0) },
    };
    const int flags = FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW;
    for (const auto& b : boxes) {
      std::feclearexcept(FE_ALL_EXCEPT);
      const interval r = compoundm1(b[0], b[1]);
      const int raised = std::fetestexcept(flags);
      check("compoundm1 raises no invalid-operation, division-by-zero or overflow flag", raised == 0,
            [&] { return hex(b[0]) + ", " + hex(b[1]) + " = " + hex(r) + ": flags " + std::to_string(raised); });
    }
  }

#if GAOL_TESTS_SET_FLUSH
  // The modes that flush the subnormals to zero, set by the program, change nothing
  void flush_modes_change_nothing()
  {
    std::vector<std::pair<double, double>> pairs;
    for (const CompoundPoint& p : compound_points) {
      if (std::fabs(p.x) < 1e-290 || std::fabs(p.below) < 1e-290 || std::fabs(p.above) < 1e-290 || std::fabs(p.y) < 1e-290) {
        pairs.push_back(std::make_pair(p.x, p.y));
      }
    }
    std::vector<interval> reference;
    for (const auto& p : pairs) {
      reference.push_back(compoundm1(interval(p.first), interval(p.second)));
    }
    for (const FlushMode& m : flush_modes()) {
      const unsigned int saved = flush_bits();
      set_flush_bits(m.bits);
      std::vector<interval> got;
      for (const auto& p : pairs) {
        got.push_back(compoundm1(interval(p.first), interval(p.second)));
      }
      const unsigned int kept = flush_bits(); // GAOL's own work does not matter, the results do
      set_flush_bits(saved);
      (void)kept;
      for (std::size_t i = 0; i < pairs.size(); ++i) {
        check("compoundm1 with " + m.name + ": the same bounds", got[i].left() == reference[i].left() && got[i].right() == reference[i].right(),
              [&] { return "(" + show(pairs[i].first) + ", " + show(pairs[i].second) + "): " + hex(got[i]) + " rather than " + hex(reference[i]); });
      }
    }
  }
#endif
}

int main()
{
  gaol::init();
  integers();
  leading_zeros();
  series_of_the_balls();
  series_of_the_fast_phase();
  reduction_of_the_fast_phase();
  helpers_of_the_fast_phase();
  doubles_of_the_fast_phase();
  bounds_of_the_fast_phase();
  preconditions();
  rounding_of_balls();
  exact_equalities();
  rounding_of_integers();
  balls_against_exact_results();
  balls_of_log1p_and_expm1();
  reciprocals();
  fast_phase();
  points();
  boxes();
  chosen_boxes();
  contrived_value();
  random_boxes();
  names_and_text();
  exception_flags();
#if GAOL_TESTS_SET_FLUSH
  flush_modes_change_nothing();
#endif
  const int status = summary();
  gaol::cleanup();
  return status;
}
