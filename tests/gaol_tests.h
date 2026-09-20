/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of this fork of GAOL: what they have in common.
 *
 * The tests check the bounds GAOL computes against the exact results of the
 * operations, independently of GAOL and of the floating-point environment:
 * the exact results are computed with integers, or were computed with 400
 * bits of precision (tests/elementary_values.py). A bound is right when it is
 * on the right side of the exact result, and as tight as it can be when no
 * double between it and the exact result is.
 *
 * They follow the tests Codac (https://github.com/codac-team/codac) makes of
 * the rounding of its intervals, which are GAOL's
 * (tests/core/domains/interval/codac2_tests_Interval_rounding.cpp, Jordan
 * Ninin): the same random doubles, the same checks, and the same list of
 * operations after which the rounding direction has to be upward still.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef __gaol_tests_h__
#define __gaol_tests_h__

#include <cfenv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "gaol/gaol"

namespace gaol_tests
{
  const double inf = std::numeric_limits<double>::infinity();

  //---------------------------------------------------------------------------
  // Reporting
  //---------------------------------------------------------------------------

  struct Tally
  {
    long checks = 0;
    long failures = 0;
  };

  inline std::map<std::string, Tally>& tallies()
  {
    static std::map<std::string, Tally> t;
    return t;
  }

  // The number of failures described for each name: 5, or the value of the
  // environment variable GAOL_TESTS_FAILURES_SHOWN
  inline long failures_shown()
  {
    static const long n = std::getenv("GAOL_TESTS_FAILURES_SHOWN") ? std::atol(std::getenv("GAOL_TESTS_FAILURES_SHOWN")) : 5;
    return n;
  }

  // Counts a check named name, and prints the description describe() gives of
  // the first failures of each name
  template<class Describe>
  bool check(const std::string& name, bool ok, const Describe& describe)
  {
    Tally& t = tallies()[name];
    ++t.checks;
    if (!ok) {
      if (t.failures < failures_shown()) {
        std::printf("FAILED %s: %s\n", name.c_str(), std::string(describe()).c_str());
      }
      ++t.failures;
    }
    return ok;
  }

  inline bool check(const std::string& name, bool ok)
  {
    return check(name, ok, [] { return std::string(); });
  }

  inline std::map<std::string, int>& largest_distances()
  {
    static std::map<std::string, int> d;
    return d;
  }

  // Checks that bounds are no more than limit doubles away from the tightest
  // ones, distance being the number of doubles between them, and records the
  // largest distance of each name
  template<class Describe>
  bool check_distance(const std::string& name, int distance, int limit, const Describe& describe)
  {
    int& largest = largest_distances()[name];
    if (distance > largest) {
      largest = distance;
    }
    return check(name + ": no more than " + std::to_string(limit) + " doubles from the tightest bounds",
                 distance <= limit, describe);
  }

  // f(), or the empty set, with a failure of the check named name, when it
  // throws an exception
  template<class F, class Describe>
  gaol::interval evaluate(const std::string& name, const F& f, const Describe& describe)
  {
    try {
      return f();
    } catch (const std::exception& e) {
      check(name + ": no exception", false, [&] { return std::string(describe()) + ": " + e.what(); });
    } catch (...) {
      check(name + ": no exception", false, describe);
    }
    return gaol::interval::emptyset();
  }

  // Prints the number of checks and failures of each name, and returns the
  // exit status of the test
  inline int summary()
  {
    for (const auto& d : largest_distances()) {
      std::printf("%-60s at most %d doubles from the tightest bounds\n", d.first.c_str(), d.second);
    }
    long checks = 0, failures = 0;
    for (const auto& t : tallies()) {
      std::printf("%-60s %8ld checks, %ld failed\n", t.first.c_str(), t.second.checks, t.second.failures);
      checks += t.second.checks;
      failures += t.second.failures;
    }
    std::printf("%ld checks, %ld failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
  }

  inline std::string hex(double x)
  {
    std::ostringstream s;
    s << std::hexfloat << x;
    return s.str();
  }

  inline std::string hex(const gaol::interval& x)
  {
    return "[" + hex(x.left()) + ", " + hex(x.right()) + "]";
  }

  //---------------------------------------------------------------------------
  // Rounding
  //---------------------------------------------------------------------------

  // Sets rounding to nearest for its lifetime, and restores the rounding
  // direction it found afterwards
  class RoundingToNearest
  {
    public:

      RoundingToNearest()
        : saved_(std::fegetround())
      {
        std::fesetround(FE_TONEAREST);
      }

      ~RoundingToNearest()
      {
        std::fesetround(saved_);
      }

    private:

      const int saved_;
  };

  // The doubles next to x, computed rounding to nearest (nextafter() was found
  // by IBEX to crash on ARM64 macOS when not rounding to nearest)
  inline double next_double(double x)
  {
    RoundingToNearest nearest;
    return std::nextafter(x, inf);
  }

  inline double previous_double(double x)
  {
    RoundingToNearest nearest;
    return std::nextafter(x, -inf);
  }

  //---------------------------------------------------------------------------
  // Exact arithmetic
  //---------------------------------------------------------------------------

  // A natural number, as digits in base 2^32, least significant first, with no
  // leading zero digit (0 has no digit)
  class Natural
  {
    public:

      Natural() {}

      explicit Natural(std::uint64_t n)
      {
        for (; n != 0; n >>= 32) {
          d_.push_back(static_cast<std::uint32_t>(n));
        }
      }

      bool is_zero() const
      {
        return d_.empty();
      }

      friend Natural operator*(const Natural& a, const Natural& b)
      {
        Natural p;
        if (a.is_zero() || b.is_zero()) {
          return p;
        }
        p.d_.assign(a.d_.size() + b.d_.size(), 0);
        for (std::size_t i = 0; i < a.d_.size(); ++i) {
          std::uint64_t carry = 0;
          for (std::size_t j = 0; j < b.d_.size(); ++j) {
            const std::uint64_t t = static_cast<std::uint64_t>(a.d_[i])*b.d_[j] + p.d_[i+j] + carry;
            p.d_[i+j] = static_cast<std::uint32_t>(t);
            carry = t >> 32;
          }
          p.d_[i + b.d_.size()] = static_cast<std::uint32_t>(carry);
        }
        p.trim();
        return p;
      }

      friend Natural operator+(const Natural& a, const Natural& b)
      {
        const Natural& longer = (a.d_.size() >= b.d_.size()) ? a : b;
        const Natural& shorter = (a.d_.size() >= b.d_.size()) ? b : a;
        Natural s;
        s.d_.resize(longer.d_.size());
        std::uint64_t carry = 0;
        for (std::size_t i = 0; i < longer.d_.size(); ++i) {
          const std::uint64_t t = static_cast<std::uint64_t>(longer.d_[i])
            + (i < shorter.d_.size() ? shorter.d_[i] : 0u) + carry;
          s.d_[i] = static_cast<std::uint32_t>(t);
          carry = t >> 32;
        }
        if (carry != 0) {
          s.d_.push_back(static_cast<std::uint32_t>(carry));
        }
        return s;
      }

      // a - b, with a >= b
      friend Natural operator-(const Natural& a, const Natural& b)
      {
        Natural s;
        s.d_.resize(a.d_.size());
        std::int64_t borrow = 0;
        for (std::size_t i = 0; i < a.d_.size(); ++i) {
          std::int64_t t = static_cast<std::int64_t>(a.d_[i])
            - (i < b.d_.size() ? static_cast<std::int64_t>(b.d_[i]) : 0) - borrow;
          borrow = (t < 0) ? 1 : 0;
          if (t < 0) {
            t += static_cast<std::int64_t>(1) << 32;
          }
          s.d_[i] = static_cast<std::uint32_t>(t);
        }
        s.trim();
        return s;
      }

      // n*2^e, with e >= 0
      Natural shifted_left(long e) const
      {
        Natural s(*this);
        if (s.is_zero()) {
          return s;
        }
        const unsigned int bits = static_cast<unsigned int>(e % 32);
        if (bits != 0) {
          std::uint32_t carry = 0;
          for (std::size_t i = 0; i < s.d_.size(); ++i) {
            const std::uint32_t d = s.d_[i];
            s.d_[i] = (d << bits) | carry;
            carry = d >> (32 - bits);
          }
          if (carry != 0) {
            s.d_.push_back(carry);
          }
        }
        s.d_.insert(s.d_.begin(), static_cast<std::size_t>(e / 32), 0u);
        return s;
      }

      // The sign of a - b
      friend int compare(const Natural& a, const Natural& b)
      {
        if (a.d_.size() != b.d_.size()) {
          return (a.d_.size() < b.d_.size()) ? -1 : 1;
        }
        for (std::size_t i = a.d_.size(); i-- > 0; ) {
          if (a.d_[i] != b.d_[i]) {
            return (a.d_[i] < b.d_[i]) ? -1 : 1;
          }
        }
        return 0;
      }

    private:

      void trim()
      {
        while (!d_.empty() && d_.back() == 0) {
          d_.pop_back();
        }
      }

      std::vector<std::uint32_t> d_;
  };

  // A dyadic number, sign*m*2^e, which any finite double is exactly
  struct Dyadic
  {
    int sign = 0; // -1, 0 or 1
    Natural m;
    long e = 0;
  };

  // x, finite, exactly
  inline Dyadic dyadic(double x)
  {
    Dyadic r;
    if (x == 0.0) {
      return r;
    }
    r.sign = (x < 0.0) ? -1 : 1;
    int ex;
    const double f = std::frexp(std::fabs(x), &ex); // |x| = f*2^ex, f of 53 bits at most
    r.m = Natural(static_cast<std::uint64_t>(std::ldexp(f, 53)));
    r.e = static_cast<long>(ex) - 53;
    return r;
  }

  inline Dyadic operator-(Dyadic a)
  {
    a.sign = -a.sign;
    return a;
  }

  inline Dyadic operator*(const Dyadic& a, const Dyadic& b)
  {
    Dyadic p;
    if (a.sign == 0 || b.sign == 0) {
      return p;
    }
    p.sign = a.sign*b.sign;
    p.m = a.m*b.m;
    p.e = a.e + b.e;
    return p;
  }

  inline Dyadic operator+(const Dyadic& a, const Dyadic& b)
  {
    if (a.sign == 0) {
      return b;
    }
    if (b.sign == 0) {
      return a;
    }
    Dyadic s;
    s.e = (a.e < b.e) ? a.e : b.e;
    const Natural ma = a.m.shifted_left(a.e - s.e), mb = b.m.shifted_left(b.e - s.e);
    if (a.sign == b.sign) {
      s.sign = a.sign;
      s.m = ma + mb;
    } else {
      const int c = compare(ma, mb);
      if (c == 0) {
        return Dyadic();
      }
      s.sign = (c > 0) ? a.sign : b.sign;
      s.m = (c > 0) ? ma - mb : mb - ma;
    }
    return s;
  }

  inline Dyadic operator-(const Dyadic& a, const Dyadic& b)
  {
    return a + (-b);
  }

  // The sign of a - b
  inline int compare(const Dyadic& a, const Dyadic& b)
  {
    return (a - b).sign;
  }

  // A rational number, num/den with den > 0: the exact result of an operation
  struct Exact
  {
    Dyadic num;
    Dyadic den;
  };

  inline Exact exact(const Dyadic& d)
  {
    Exact r;
    r.num = d;
    r.den = dyadic(1.0);
    return r;
  }

  inline Exact exact(double x)
  {
    return exact(dyadic(x));
  }

  // n/d, with d != 0
  inline Exact quotient(const Dyadic& n, const Dyadic& d)
  {
    Exact r;
    r.num = (d.sign < 0) ? -n : n;
    r.den = (d.sign < 0) ? -d : d;
    return r;
  }

  inline Exact operator-(const Exact& a)
  {
    Exact r(a);
    r.num = -a.num;
    return r;
  }

  inline Exact operator+(const Exact& a, const Exact& b)
  {
    return quotient(a.num*b.den + b.num*a.den, a.den*b.den);
  }

  inline Exact operator*(const Exact& a, const Exact& b)
  {
    return quotient(a.num*b.num, a.den*b.den);
  }

  // The sign of a - b
  inline int compare(const Exact& a, const Exact& b)
  {
    return compare(a.num*b.den, b.num*a.den);
  }

  // The sign of x - v, x being a double, infinite or not
  inline int compare(double x, const Exact& v)
  {
    if (x == inf) {
      return 1;
    }
    if (x == -inf) {
      return -1;
    }
    return compare(dyadic(x)*v.den, v.num);
  }

  inline const Exact& min(const std::vector<Exact>& values)
  {
    std::size_t k = 0;
    for (std::size_t i = 1; i < values.size(); ++i) {
      if (compare(values[i], values[k]) < 0) {
        k = i;
      }
    }
    return values[k];
  }

  inline const Exact& max(const std::vector<Exact>& values)
  {
    std::size_t k = 0;
    for (std::size_t i = 1; i < values.size(); ++i) {
      if (compare(values[i], values[k]) > 0) {
        k = i;
      }
    }
    return values[k];
  }

  // x^n, x being finite and n >= 0
  inline Exact power(double x, int n)
  {
    Dyadic p = dyadic(1.0);
    const Dyadic d = dyadic(x);
    for (int i = 0; i < n; ++i) {
      p = p*d;
    }
    return exact(p);
  }

  //---------------------------------------------------------------------------
  // Bounds
  //---------------------------------------------------------------------------

  inline bool is_lower_bound(double l, const Exact& v)
  {
    return compare(l, v) <= 0;
  }

  inline bool is_upper_bound(double u, const Exact& v)
  {
    return compare(u, v) >= 0;
  }

  // l <= v, and no double above l is
  inline bool is_tightest_lower_bound(double l, const Exact& v)
  {
    return is_lower_bound(l, v) && !is_lower_bound(next_double(l), v);
  }

  // u >= v, and no double below u is
  inline bool is_tightest_upper_bound(double u, const Exact& v)
  {
    return is_upper_bound(u, v) && !is_upper_bound(previous_double(u), v);
  }

  // Whether [x] is the tightest interval of doubles enclosing [lo, hi]
  inline bool is_tightest_enclosure(const gaol::interval& x, const Exact& lo, const Exact& hi)
  {
    return is_tightest_lower_bound(x.left(), lo) && is_tightest_upper_bound(x.right(), hi);
  }

  inline bool is_tightest_enclosure(const gaol::interval& x, const Exact& v)
  {
    return is_tightest_enclosure(x, v, v);
  }

  inline bool is_enclosure(const gaol::interval& x, const Exact& lo, const Exact& hi)
  {
    return is_lower_bound(x.left(), lo) && is_upper_bound(x.right(), hi);
  }

  inline bool is_enclosure(const gaol::interval& x, const Exact& v)
  {
    return is_enclosure(x, v, v);
  }

  // The number of doubles strictly between the bound and the tightest bound,
  // the bound being a lower bound, respectively an upper bound (limit + 1 when
  // there are more than limit)
  inline int doubles_below_tightest(double l, const Exact& v, int limit)
  {
    int n = 0;
    for (double t = next_double(l); is_lower_bound(t, v) && n <= limit; t = next_double(t)) {
      ++n;
    }
    return n;
  }

  inline int doubles_above_tightest(double u, const Exact& v, int limit)
  {
    int n = 0;
    for (double t = previous_double(u); is_upper_bound(t, v) && n <= limit; t = previous_double(t)) {
      ++n;
    }
    return n;
  }

  //---------------------------------------------------------------------------
  // Random doubles
  //---------------------------------------------------------------------------

  // A deterministic generator (xorshift64)
  class Random
  {
    public:

      std::uint64_t bits()
      {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 7;
        state_ ^= state_ << 17;
        return state_;
      }

      // An integer from lo to hi
      int integer(int lo, int hi)
      {
        return lo + static_cast<int>(bits() % static_cast<std::uint64_t>(hi - lo + 1));
      }

      // A double with a random sign and mantissa, and an exponent from emin to
      // emax
      double operator()(int emin, int emax)
      {
        return make(bits() & 1u, static_cast<std::uint64_t>(1023 + integer(emin, emax)));
      }

      // A positive double with a random mantissa and an exponent from emin to
      // emax
      double positive(int emin, int emax)
      {
        return make(0u, static_cast<std::uint64_t>(1023 + integer(emin, emax)));
      }

      // Any finite double, subnormal ones included, with a random sign,
      // mantissa and exponent
      double any()
      {
        return make(bits() & 1u, bits() % 2047u);
      }

      // A double from lo to hi, about
      double uniform(double lo, double hi)
      {
        return lo + (hi - lo)*std::ldexp(static_cast<double>(bits() >> 11), -53);
      }

    private:

      double make(std::uint64_t sign, std::uint64_t biased_exponent)
      {
        const std::uint64_t b = (sign << 63) | (biased_exponent << 52) | (bits() & 0x000FFFFFFFFFFFFFull);
        double x;
        std::memcpy(&x, &b, sizeof x);
        return x;
      }

      std::uint64_t state_ = 0x9E3779B97F4A7C15ull;
  };

  // The interval of doubles a and b, whatever their order
  inline gaol::interval hull(double a, double b)
  {
    return (a < b) ? gaol::interval(a, b) : gaol::interval(b, a);
  }
}

#endif /* __gaol_tests_h__ */
