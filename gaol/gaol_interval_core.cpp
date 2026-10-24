/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_core.cpp
  \brief  The definitions of the intervals: gaol_core

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

// SIMD (GAOL v5, doc/simd.md): with SSE2 and NEON the two bounds of an interval are the two lanes of one
// vector, and + - * / sqr are vector kernels of gaol_interval_sse.cpp and gaol_interval_neon.cpp (sqrt too
// with NEON); the AVX-512 path of gaol_interval_avx512.cpp carries the rounding direction in the
// instruction, its square root being two scalar roots, and the integer powers have a two-lane FMA kernel
// with SSE2 and AVX-512 where __FMA__ is. What is scalar in this file is the elementary functions, which
// are CORE-MATH calls, one per bound, and the operations made of branches on the bounds. doc/simd.md lists
// what could still gain (sqrt with SSE2, the integer powers with NEON, mid(), the reduction of the argument
// of sin() and cos()) and what the other processors would need

#include <iostream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <cmath>
#include <cfenv>
// Whether pow_up_without_overflow_exception() writes the control register of
// the SSE instructions, MXCSR, which compute the doubles on x86 (not with
// Visual C++ for 32-bit x86, where its Release build left the flag of the
// masked section raised: it takes the functions of <cfenv>, which that C
// library does right)
#if (defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)) \
    && !(defined(_MSC_VER) && !defined(__clang__) && defined(_M_IX86))
#  include <xmmintrin.h>
#  define GAOL_POW_MASKS_MXCSR 1
#else
#  define GAOL_POW_MASKS_MXCSR 0
#endif
#include <string>
#include <cerrno>
#include <locale>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cstdint>
#include <cstring>
#include <sstream>

#if GAOL_USING_SSE2_INSTRUCTIONS
#  include <pmmintrin.h>
#endif

#include "gaol/gaol_config.h"
#include "gaol/gaol_fpu.h"
// The bounds at doubles, *_dn() and *_up(): a header of the sources, not
// installed (GAOL v5)
#include "gaol/gaol_double_op.h"
#include "gaol/gaol_common.h"
#include "gaol/parser/gaol_parser.h"
#include "gaol/gaol_port.h"
#include "gaol/gaol_interval_core.h"
#include "gaol/gaol_exceptions.h"
// x * 2^n rounded once, for the integer powers beyond the doubles (GAOL v5)
#include "gaol/gaol_ldexp.h"

// The FMA instructions on two doubles at once, for the integer powers, where
// the build has them (GAOL v5, see pown_products())
#if GAOL_USING_SSE2_INSTRUCTIONS && (defined(__FMA__) || (defined(_MSC_VER) && defined(__AVX2__)))
#  define GAOL_POWN_PACKED 1
#  include <immintrin.h>
#else
#  define GAOL_POWN_PACKED 0
#endif
// The same kernel rounded upward by the embedded rounding of AVX-512, where
// GAOL_AVX512_ARITHMETIC is (GAOL v5, point ZZ of TODO.md, see
// pown_large_er()): it needs the packed kernel, and runs in functions that
// carry the target of gaol/gaol_interval_avx512.cpp
#if GAOL_POWN_PACKED && (defined(__x86_64__) || defined(_M_X64)) && GAOL_AVX512_ARITHMETIC
#  define GAOL_POWN_ER 1
#else
#  define GAOL_POWN_ER 0
#endif

using std::ostream;
using std::istream;
using std::sqrt;

//debug
using std::cout;
using std::endl;


namespace gaol_core {

  // The names of gaol_detail, and 2^52, from which on the doubles are
  // integers, which gaol_port.h declared for the code using GAOL too (GAOL v5)
  using namespace gaol_detail;
  const double two_power_52 = 4503599627370496.0;

  // fma(a, b, c) rounded once (GAOL v5). __builtin_fma, not std::fma: without
  // optimization std::fma is a call of the fma() of the C library, and that of
  // Cygwin is not correctly rounded, where the builtin is the instruction with
  // -mfma (gaol_config.h refuses Cygwin without it)
  static inline double fma_once(double a, double b, double c)
  {
#if defined(__GNUC__)
    return __builtin_fma(a, b, c);
#else
    return std::fma(a, b, c);
#endif
  }

  /*
    x read through a volatile, for an operation that a test leaves out where
    it would raise an exception flag (GAOL v5): no compiler reads a volatile
    the program does not, so that the operation stays in the branch the test
    takes. Clang for 64-bit ARM computed both sides of such tests and chose
    between them afterwards (fcsel): floor_rel([DBL_MAX], [0, 1]) raised the
    overflow flag with DBL_MAX + 1 (Clang 18 and 21 at -O3, AppleClang),
    atan2_rel_y([0], [3, +oo], [-oo, -0]) the invalid-operation one
    (AppleClang), where the slope 0 leaves out the product +oo * 0, and the
    inverse of the FPU intervals, inverse([-2, 0]), the division-by-zero one
    with -1/0 (AppleClang, GAOL_PRESERVE_ROUNDING) (tests/rounding_direction.cpp,
    tests/reverse.cpp).
  */
  static inline double read_in_branch(double x)
  {
    const volatile double v = x;
    return v;
  }

  /*
    Quotients and products of finite doubles a >= 0 and b > 0 (b >= 0 for a
    product) that return what the operation returns when it overflows,
    without executing it, so that no overflow exception is raised (GAOL v5,
    point 79 of TODO.md): +oo rounded upward, and the largest double for
    -((-a)/b) and -((-a)*b), the same operations rounded downward by the
    upward rounding GAOL computes in. The intermediate values of the
    relational functions, which are then intersected with a finite interval,
    overflow for a finite result: the divisor of div_rel() next to 2^-1074,
    the slope of a cone of atan2_rel_y()... The exponents e of a and f of b
    (frexp(), 2^(e-1) <= a < 2^e) tell: a / b is below 2^(e-f+1), so that it
    is below 2^1023 for e - f <= 1022, and above 2^(e-f-1), so that it is
    above 2^1024 for e - f >= 1025. Between them, for e - f = 1023 or 1024
    (e + f = 1024 or 1025 for a product), a quarter of a gives a result
    between 2^1020 and 2^1023, to compare with a quarter of the largest
    double: the rounding being monotonic, the result is above the largest
    double exactly when its quarter is above that quarter, and the scaling by
    4 is exact (a >= 2^-51 there). The operands go through read_in_branch() where the operation is
    executed, as above. A first test tells the usual operands, for which the
    exponents are not computed.
  */
  static const double two_power_500 = 3.273390607896142e+150, two_power_minus_500 = 3.054936363499605e-151;

  static inline double dbl_max_quarter()
  {
    return 0.25*(std::numeric_limits<double>::max)();
  }

  // The e of frexp() for a finite d: 2^(e-1) <= d < 2^e for d > 0
  static inline int exponent_of(double d)
  {
    int e;
    (void)std::frexp(d, &e);
    return e;
  }

  static inline double quotient_up_sat(double a, double b)
  {
    if (a < two_power_500 && b > two_power_minus_500) { // The quotient is below 2^1000
      return read_in_branch(a)/b;
    }
    const int e = exponent_of(a), f = exponent_of(b);
    if (e - f <= 1022) {
      return read_in_branch(a)/b;
    }
    if (e - f >= 1025) {
      return GAOL_INFINITY;
    }
    const double q = (0.25*a)/read_in_branch(b);
    return (q > dbl_max_quarter()) ? GAOL_INFINITY : 4.0*q;
  }

  // -((-a)/b)
  static inline double quotient_dn_sat(double a, double b)
  {
    if (a < two_power_500 && b > two_power_minus_500) {
      return -((-read_in_branch(a))/b);
    }
    const int e = exponent_of(a), f = exponent_of(b);
    if (e - f <= 1022) {
      return -((-read_in_branch(a))/b);
    }
    if (e - f >= 1025) {
      return (std::numeric_limits<double>::max)();
    }
    const double q = -((-0.25*a)/read_in_branch(b));
    return (q >= dbl_max_quarter()) ? (std::numeric_limits<double>::max)() : 4.0*q;
  }

  static inline double product_up_sat(double a, double b)
  {
    if (a < two_power_500 && b < two_power_500) { // The product is below 2^1000
      return read_in_branch(a)*b;
    }
    const int e = exponent_of(a), f = exponent_of(b);
    if (e + f <= 1023) {
      return read_in_branch(a)*b;
    }
    if (e + f >= 1026) {
      return GAOL_INFINITY;
    }
    const double q = (0.25*a)*read_in_branch(b);
    return (q > dbl_max_quarter()) ? GAOL_INFINITY : 4.0*q;
  }

  // -((-a)*b)
  static inline double product_dn_sat(double a, double b)
  {
    if (a < two_power_500 && b < two_power_500) {
      return -((-read_in_branch(a))*b);
    }
    const int e = exponent_of(a), f = exponent_of(b);
    if (e + f <= 1023) {
      return -((-read_in_branch(a))*b);
    }
    if (e + f >= 1026) {
      return (std::numeric_limits<double>::max)();
    }
    const double q = -((-0.25*a)*read_in_branch(b));
    return (q >= dbl_max_quarter()) ? (std::numeric_limits<double>::max)() : 4.0*q;
  }

  /*
    The quotient a / b of two doubles of any signs, b other than 0, rounded
    upward, like a / b in the upward rounding GAOL computes in, but for the
    overflow: it gives the infinity, or the largest double of the opposite
    sign, that an overflowing quotient gives (see quotient_up_sat()),
    without raising the exception (GAOL v5, point 79 of TODO.md)
  */
  static inline double quotient_sat(double a, double b)
  {
    const double x = std::fabs(a), y = std::fabs(b);
    if (x == 0.0 || (x < two_power_500 && y > two_power_minus_500) || !(x <= (std::numeric_limits<double>::max)())
        || !(y <= (std::numeric_limits<double>::max)())) {
      return read_in_branch(a)/b; // The quotient is below 2^1000, or 0, or has an infinite operand
    }
    return ((a < 0.0) == (b < 0.0)) ? quotient_up_sat(x, y) : -quotient_dn_sat(x, y);
  }

  /*
    div_rel(), mulRev of IEEE 1788-2015, after the check of the rounding
    direction: the hull of the x of I with an x y in K for a y of J, from the
    quotients of the bounds of K by those of J, whichever the signs. The FPU
    and the NEON intervals compute it so, and the SSE2 intervals where a
    quotient of their vectors could overflow (see div_rel() in
    gaol/gaol_interval_sse.cpp). The quotients are those of quotient_sat():
    one that overflows is the infinity it gives, which is intersected with a
    finite I or joined to an infinite bound of the hull, where the division
    raised the overflow exception for a finite result, as in div_rel([0, 1/2],
    [2^-1074, 2^-1022], [0, 1]) (GAOL v5, point 79 of TODO.md). The code is
    that of the FPU and the NEON intervals, which took the stored bounds of
    their operands, the opposite of the lower bound being -K.left().
  */
  static interval div_rel_upward(const interval &K, const interval &J, const interval &I)
  {
    if (K.is_empty() || J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }

    if (K.right() < 0.0) { // [this] N1
      if (J.right() < 0.0) { // [J] N1
	// I & [K.r/J.l, K.l/J.r]
	interval tmp(-(quotient_sat(K.right(), -J.left())),quotient_sat(-K.left(), -J.right()));
	return I & tmp; // N1 N1
      } else { // [J] P or Z
	if (J.right() == 0.0) { // [J] N0 or Z
	  if (J.left() == 0.0) { // [J] Z
	    return interval::emptyset(); // N1 Z
	  }
	  // I & [K.r/J.l,+oo]
	  interval tmp(-(quotient_sat(K.right(), -J.left())),GAOL_INFINITY);
	  return I & tmp; // N1 N0
	} else { // [I] P or M
	  if (J.left() < 0.0) { // [J] M
	    interval tmp(-GAOL_INFINITY,quotient_sat(K.right(), J.right()));
	    // The lower bound rounded downward, the rounding direction being
	    // upward (GAOL v5: K.right()/J.left() was rounded upward, and the
	    // result left out the doubles below it that belong to the hull)
	    interval tmp2(-(quotient_sat(K.right(), -J.left())),GAOL_INFINITY);
	    return (I&tmp) | (I&tmp2); // N1 M
	  } else { // [J] P0 or P1
	    if (J.left() == 0.0) { // [J] P0
	      interval tmp2 = I & interval(-GAOL_INFINITY,quotient_sat(K.right(), J.right())); // N1 P0
	      return tmp2; // N1 P0
	    } else { // [J] P1
	      interval tmp(-(quotient_sat(-K.left(), J.left())),quotient_sat(K.right(), J.right()));
	      return I & tmp; // N1 P1
	    }
	  }
	}
      }
    } else { // [this] N0, Z, M, or P
      if (K.right() == 0.0) { // [this] N0 or Z
	if (K.left() == 0.0) { // [this] Z
	  if (J.straddles_zero()) {
	    return I; // Z (N0, M, P0)
	  } else {
	    return I & interval::zero(); // Z (N1, P1)
	  }
	} else { // [this] N0
	  if (J.right() < 0.0) { // [J] N1
	    interval tmp2 = I & interval(0.0,quotient_sat(K.left(), J.right())); // N0 N1
	    return tmp2;
	  } else { // [J] P or Z
	    if (J.right() == 0.0) { // [J] N0 or Z
	      if (J.left() == 0.0) { // [J] Z
		return I; // N0 Z
	      }
	      return I; // N0 N0
	    } else { // [J] P or M
	      if (J.left() < 0.0) { // [J] M
		return I; // N0 M
	      } else { // [J] P0 or P1
		if (J.left() == 0.0) { // [J] P0
		  return I; // N0 P0
		} else { // [J] P1
		  interval tmp(-(quotient_sat(-K.left(), J.left())),0.0);
		  return I & tmp; // N0 P1
		}
	      }
	    }
	  }
	}
      } else { // [this] M, P0, or P1
	if (K.left() < 0.0) { // [this] M
	  if (J.right() < 0.0) { // [J] N1
	    interval tmp(-(quotient_sat(K.right(), -J.right())),quotient_sat(-K.left(), -J.right()));
	    return I & tmp; // M N1
	  } else { // [J] P or Z
	    if (J.right() == 0.0) { // [J] N0 or Z
	      return I; // M N0
	    } else { // [J] P or M
	      if (J.left() < 0.0) { // [J] M
		return I; // M M
	      } else { // [J] P0 or P1
		if (J.left() == 0.0) { // [J] P0
		  return I; // M P0
		} else { // [J] P1
		  interval tmp(-(quotient_sat(-K.left(), J.left())),quotient_sat(K.right(), J.left()));
		  return I & tmp; // M P1
		}
	      }
	    }
	  }
	} else { // [this] P0 or P1
	  if (K.left() == 0.0) { // [this] P0
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(quotient_sat(K.right(), -J.right())),0.0);
	      return I & tmp; // P0 N1
	    } else { // [J] P or Z
	      if (J.right() == 0.0) { // [J] N0 or Z
		return I; // P0 (Z or N0)
	      } else { // [J] P or M
		if (J.left() < 0.0) { // [J] M
		  return I; // P0 M
		} else { // [J] P0 or P1
		  if (J.left() == 0.0) { // [J] P0
		    return I; // P0 P0
		  } else { // [J] P1
		    interval tmp2 = I & interval(0.0,quotient_sat(K.right(), J.left())); // P0 P1
		    return tmp2;
		  }
		}
	      }
	    }
	  } else { // [this] P1
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(quotient_sat(K.right(), -J.right())),quotient_sat(-K.left(), -J.left()));
	      return I & tmp; // P1 N1
	    } else { // [J] P or Z
	      if (J.right() == 0.0) { // [J] N0 or Z
		if (J.left() == 0.0) { // [J] Z
		  return interval::emptyset(); // P1 Z
		}
		interval tmp2 = I & interval(-GAOL_INFINITY,quotient_sat(-K.left(), -J.left())); // P1 N0
		return tmp2;
	      } else { // [J] P or M
		if (J.left() < 0.0) { // [J] M
		  interval tmp(-GAOL_INFINITY,quotient_sat(-K.left(), -J.left()));
		  interval tmp2 = interval(-(quotient_sat(-K.left(), J.right())),GAOL_INFINITY);
		  return (I&tmp) | (I&tmp2); // P1 M
		} else { // [J] P0 or P1
		  if (J.left() == 0.0) { // [J] P0
		    interval tmp(-(quotient_sat(-K.left(), J.right())),GAOL_INFINITY);
		    return I & tmp; // P1 P0
		  } else { // [J] P1
		    interval tmp(-(quotient_sat(-K.left(), J.right())),quotient_sat(K.right(), J.left()));
		    return I & tmp; // P1 P1
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

#if (defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
  // GAOL_AVX512_ARITHMETIC, decided when GAOL is configured: +, -, *, /, sqr,
  // sqrt and, with FMA, the integer powers beyond 1 and -1 take the AVX-512
  // path of gaol/gaol_interval_avx512.cpp, which sets no rounding direction,
  // and clears the flush-to-zero modes as the other operations do
  // (flush_guard there; GAOL v5, point Q of TODO.md)
#  include <immintrin.h>   // the AVX-512 intrinsics of the path below
#  include "gaol/gaol_interval_avx512.cpp"
#endif
#if GAOL_USING_SSE2_INSTRUCTIONS
#  include "gaol/gaol_interval_sse.cpp"
#elif GAOL_USING_NEON_INSTRUCTIONS
#  include "gaol/gaol_interval_neon.cpp"
#else
#  include "gaol/gaol_interval_fpu.cpp"
#endif // GAOL_USING_SSE2_INSTRUCTIONS

  /*
    x^n for an int n, |n| >= 2 but n = 2 (x^2 is sqr(), which gaol_pown() and
    pown_upward() take before this code, GAOL v5), with the tightest bounds,
    from exact products (GAOL v5, points 8, 72 and 73 of TODO.md). GAOL rounded each
    product of its binary exponentiation outward, and its bounds were up to
    n + 1 doubles beyond the tightest (32 to 39 at n = 30, 1300 to 1500 at
    n = 1000); GAOL v5 then kept each power as a double and a rest, rounded
    downward and upward apart, which cost 41 ns for x^1000 against 9.7 with
    GAOL 4, and took x^-m as 1/x^m, two roundings, one or two doubles beyond
    the tightest bound.

    The bounds come from two corners c0 and c1 (pown_corners()): x^n is c0^n
    at its least and c1^n at its greatest. Both powers are computed at once,
    |c0|^m in lane 0 and |c1|^m in lane 1 of an SSE2 register (of a pair of
    doubles in the other builds, the same operations in the same order), as
    h + l, h being the power rounded and l a rest, the sign of c being given
    back to it at the end for an odd m: the products of c and of -c, rounded
    upward, are not opposite, and the powers of c itself would make
    pow([-c], n) other than -pow([c], n) where c^n is within K of a double,
    and other than the bound pown_point() gives from |c|. From the highest
    bit of m = |n| down, a square
      p = h*h, e = h*h - p, t = 2h + l, l' = l*t + e, h' = p,
    and, for each bit set, a product by |c|
      q = h*|c|, f = h*|c| - q, l' = l*|c| + f, h' = q,
    e and f being exactly the rests of the products (fma(), the products
    being normal), and t and l' rounded once each (fma()). The error of
    h + l is bounded beforehand, relatively: |h + l - |c|^m| <= K h with
    K = (8 m L + 16) 2^-104, L being the number of bits of m (below). A
    square, m = 2, is h + l exactly, and comes here for n = -2 only, whose K
    is that of the inverse (below): x^2 is sqr(), which has no error to
    bound. Each bound is then rounded once, upward, the rounding direction being upward
    throughout, v + w being h + l or -(h + l), of the sign of c^m:
      lower bound = -RU(-v + RU(K h - w)), upper bound = RU(v + RU(K h + w)),
    the lane 0 holding the opposite of the lower bound, as the SSE2 intervals
    store it. The bounds are the tightest, and exact where c^m is a double:
    every product is exact then (c^k is a double for each k <= m), each rest
    e and f is +0, and K is taken as 0 where the rests or-ed together are 0
    (without that, pown([2, 3], 3) was [8 - 2^-50, 27 + 2^-48]). They are one
    double beyond where c^m is within 2K of a double, relatively (K is 2^-98
    at m = 3, 2^-88 at m = 1000, 2^-65 at m = 2^31 - 1): 2^-46 of the bounds
    at m = 3 at random, but a fifth of them where c is within a few doubles
    of a number whose power is a double (c within 8 doubles of 1, m up to
    200), c^m being then that close to a double: pow([1 + 2^-52], 3) is
    [1 + 2^-51, 1 + 2^-50], [1 + 3 2^-52, 1 + 2^-50] being the tightest.
    Deciding these bounds with CORE-MATH's pow took 55 to 270 ns, these
    powers being its hard cases: they are left so (decided on 13 October
    2026, TODO.md B).

    For n < 0, 1/c^m from h + l: q = 1/h rounded upward, R = 1 - q(h + l) =
    (1 - q h) - q l, 1 - q h being exact, r = R rounded once, and
    1/(h + l) = q (1 + R + R^2 + R^3/(1 - R)), which q + q (r + r^2) gives
    to within the error of r, two roundings and q R^3: K = 2 K_m + 64 2^-104
    + 2 m^3 2^-156 (below). The result is exact where 1/c^m is a double, r
    being +0 too. 1/x is inverse() itself, correctly rounded.

    The bound K, with u = 2^-52 and m <= 2^31. A rounding upward is within
    u|z| of a normal z, and within 2^-1074 of any z, which is u^2/2 of
    2^-969, below which no power c^k goes here. After j steps (squares and
    products by c) reaching c^k = X, k u <= 2^-20 and
      |l| <= b |X|, b = (k - 1) u (1 + 2(k - 1) u),
      |h + l - X| <= a |X|, a = 3 k j u^2.
    The first square gives c^2 = h + l exactly, |l| < u|X|. A square takes
    d = h + l - X to d(2X + d) + l(t - 2h - l) + (l' - l t - e), and a
    product by c to c d + (l' - l c - f), with |h| <= (1 + a + b)|X|,
    |e| <= u h^2 and |f| <= u(1 + u)|h c|: the bounds become
      b' = (1 + u)^2 b (2 + 2a + 3b) + (1 + u) u (1 + a + b)^2 + u^2/2,
      a' = a(2 + a) + u b (2 + 2a + 3b) + u b' + u^2/2
    after a square, and
      b' = (1 + u)(b + u (1 + u)(1 + a + b)) + u^2/2,  a' = a + u b' + u^2/2
    after a product, about 2b + u = (2k - 1) u and 2a + 4 k u^2, b + u and
    a + k u^2: below the bounds for 2k and j + 1, for k + 1 and j + 1, by
    (b + u)^2/4 and 2 k u^2 at least for the terms of second order
    (tests/tools/pow/pown_bound.py checks it with rationals). With
    j <= 2(L - 1), |h + l - c^m| <= 6 m (L - 1) u^2 |X|, which is below K|h|,
    |h| being above (1 - 2^-19)|X|. For n < 0, |R| <= u + |q l| <=
    m u (1 + 2^-17); the roundings of r, q r and q r (1 + r) are within
    3 m u^2 (1 + 2^-16)|q| together, q R^3/(1 - R) within 1.0001 m^3 u^3 |q|,
    and 1/(h + l) within 6 m (L - 1) u^2 (1 + 2^-17)|q| of 1/c^m: below
    2 K_m |q| for the first and the third, 2 m^3 u^3 |q| for the second, and
    64 u^2 |q| for the roundings within 2^-1074. A product scaled by 2^-500 in
    pown_point() rounds its rest l within 2^-1074 too, the power being above
    1 there.

    Each power c^k stays within [2^-969, 2^961] where |c| is within
    [pown_least[L], pown_greatest[L]] (c = 0 too for n > 0): the rests are
    doubles, and q = 1/h is normal, for every m of L bits. Any other corner
    takes pown_point(), one bound at a time, c^m being computed there with c
    scaled to [1, 2) and the power kept below 2^501 by powers of two, scaled
    back at the end, rounded once in the direction of the bound: the same
    bounds, the tightest, for the powers beyond the doubles and below 2^-968
    too, and the limits at 0 and at the infinities. No operation computes
    oo - oo or 0 oo: x^n raises no invalid-operation exception, where the
    exponentiation from an infinite bound would.
  */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_POWN_INLINE __attribute__((always_inline)) inline
#  define GAOL_POWN_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define GAOL_POWN_INLINE __forceinline
#  define GAOL_POWN_NOINLINE __declspec(noinline)
#else
#  define GAOL_POWN_INLINE inline
#  define GAOL_POWN_NOINLINE
#endif
// The functions that inline the operations of pown_er_lanes (and the ones
// that call them) carry the target of those: the compiler refuses to inline a
// function of an AVX-512 target into another. The library needs a processor
// with AVX-512 where GAOL_POWN_ER is, as for the other operations of the path
#if GAOL_POWN_ER
#  define GAOL_POWN_TARGET GAOL_AVX512_TARGET
#  define GAOL_POWN_INLINE_T GAOL_AVX512_TARGET GAOL_POWN_INLINE
#else
#  define GAOL_POWN_TARGET
#  define GAOL_POWN_INLINE_T GAOL_POWN_INLINE
#endif

  // 2^(-968/2^L) rounded upward and 2^(960/2^L) rounded downward, for the
  // m of L bits, 2 <= L <= 32 (2^(L-1) <= m < 2^L): |c|^m is then within
  // [2^-968, 2^960] for |c| within [pown_least[L], pown_greatest[L]]
  static const double pown_least[33] = {
    0.0, 0.0, 1.4149498560666738e-73, 3.76158192263132e-37, 6.133173666733497e-19,
    7.83145814439016e-10, 2.7984742529439433e-05, 0.005290060730222238,
    0.0727328036735986, 0.26969019943927997, 0.5193170509806895,
    0.7206365595643129, 0.848903150874299, 0.9213594037476901,
    0.9598746812723472, 0.979731943580665, 0.9898140954647318,
    0.9948940121765392, 0.9974437388527432, 0.9987210515718307,
    0.9993603211914263, 0.999680109430725, 0.9998400419220692,
    0.9999200177624554, 0.9999600080815509, 0.9999800038408523,
    0.9999900018704448, 0.9999950009227271, 0.9999975004582398,
    0.999998750228339, 0.9999993751139743, 0.9999996875569384,
    0.999999843778457
  };
  static const double pown_greatest[33] = {
    0.0, 0.0, 1.7668470647783843e+72, 1.329227995784916e+36, 1.152921504606847e+18,
    1073741824.0, 32768.0, 181.01933598375615, 13.454342644059432,
    3.668016172818685, 1.9152065613971472, 1.3839098819638318,
    1.1763969916502812, 1.0846183622133092, 1.041450124688316,
    1.020514637174948, 1.0102052450739643, 1.0050896701657839,
    1.0025416052043843, 1.0012699961570726, 1.0006347965951776,
    1.0003173479427303, 1.0001586613846476, 1.000079327545894,
    1.0000396629863706, 1.0000198312965451, 1.000009915599113,
    1.0000049577872665, 1.0000024788905608, 1.0000012394445121,
    1.000000619722064, 1.000000309860984, 1.00000015493048
  };

  // The number of bits of m >= 1
  static GAOL_POWN_INLINE unsigned int pown_bit_length(unsigned int m)
  {
#if defined(__GNUC__) || defined(__clang__)
    return 32u - static_cast<unsigned int>(__builtin_clz(m));
#else
    unsigned int length = 1u;
    while (length < 32u && (m >> length) != 0u) {
      ++length;
    }
    return length;
#endif
  }

  // K for c^m and for c^-m (see above), in the upward rounding: 2^-104 is
  // 4.930380657631324e-32 and 2^-155 2.1895288505075267e-47. pown_bound() is
  // for m >= 3: x^2 is sqr(), which gaol_pown() and pown_upward() take before
  // the kernel (c^2 is h + l exactly, and its K was 0, GAOL v5); c^-2 comes
  // to pown_bound_inverse(), whose K covers the roundings of the inverse
  static GAOL_POWN_INLINE double pown_bound(unsigned int m, unsigned int L)
  {
    return (8.0*m*L + 16.0)*4.930380657631324e-32;
  }

  static GAOL_POWN_INLINE double pown_bound_inverse(unsigned int m, unsigned int L)
  {
    const double dm = m;
    return 2.0*(8.0*m*L + 16.0)*4.930380657631324e-32 + 64.0*4.930380657631324e-32
      + dm*dm*dm*2.1895288505075267e-47;
  }

  static GAOL_POWN_INLINE std::uint64_t pown_bits_of(double x)
  {
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    return bits;
  }

  // The two lanes: an SSE2 register where the FMA instructions are, a pair
  // of doubles elsewhere; the rests or-ed together
#if GAOL_POWN_PACKED
  typedef __m128d pown_lanes;
  typedef __m128d pown_rests;
  static GAOL_POWN_INLINE pown_lanes pown_mul(pown_lanes a, pown_lanes b) { return _mm_mul_pd(a, b); }
  static GAOL_POWN_INLINE pown_lanes pown_fms(pown_lanes a, pown_lanes b, pown_lanes c) { return _mm_fmsub_pd(a, b, c); }
  static GAOL_POWN_INLINE pown_lanes pown_fma(pown_lanes a, pown_lanes b, pown_lanes c) { return _mm_fmadd_pd(a, b, c); }
  static GAOL_POWN_INLINE pown_lanes pown_twice_plus(pown_lanes a, pown_lanes b) { return _mm_fmadd_pd(_mm_set1_pd(2.0), a, b); }
  static GAOL_POWN_INLINE pown_rests pown_first_rest(pown_lanes e) { return e; }
  static GAOL_POWN_INLINE void pown_add_rest(pown_rests& s, pown_lanes e) { s = _mm_or_pd(s, e); }
#  if GAOL_POWN_ER
  /*
    The two lanes of the kernel rounded upward by the embedded rounding of
    AVX-512 (_mm512_*_round_pd on the 512 bits of which the upper ones are not
    used, as er_mul2() does) rather than by the direction of MXCSR: the same
    operations in the same order as the ones above, so the same bits, with no
    write of MXCSR, which cost 3 to 6 ns an operation with
    GAOL_PRESERVE_ROUNDING. A struct, so that no operation of a register of
    the other kind is taken for one of this: the compiler refuses it, and
    would otherwise round it by the direction of MXCSR, which is not upward.
  */
  struct pown_er_lanes { __m128d v; };
  static GAOL_POWN_INLINE_T pown_er_lanes pown_mul(pown_er_lanes a, pown_er_lanes b)
  {
    const pown_er_lanes r = { er_mul2(a.v, b.v) };
    return r;
  }
  static GAOL_POWN_INLINE_T pown_er_lanes pown_fms(pown_er_lanes a, pown_er_lanes b, pown_er_lanes c)
  {
    const pown_er_lanes r = { back128(_mm512_fmsub_round_pd(lo512(a.v), lo512(b.v), lo512(c.v), gaol_er)) };
    return r;
  }
  static GAOL_POWN_INLINE_T pown_er_lanes pown_fma(pown_er_lanes a, pown_er_lanes b, pown_er_lanes c)
  {
    const pown_er_lanes r = { back128(_mm512_fmadd_round_pd(lo512(a.v), lo512(b.v), lo512(c.v), gaol_er)) };
    return r;
  }
  static GAOL_POWN_INLINE_T pown_er_lanes pown_fnmadd(pown_er_lanes a, pown_er_lanes b, pown_er_lanes c)
  {
    const pown_er_lanes r = { back128(_mm512_fnmadd_round_pd(lo512(a.v), lo512(b.v), lo512(c.v), gaol_er)) };
    return r;
  }
  static GAOL_POWN_INLINE_T pown_er_lanes pown_twice_plus(pown_er_lanes a, pown_er_lanes b)
  {
    const pown_er_lanes r = { back128(_mm512_fmadd_round_pd(_mm512_set1_pd(2.0), lo512(a.v), lo512(b.v), gaol_er)) };
    return r;
  }
  static GAOL_POWN_INLINE_T pown_er_lanes pown_div(pown_er_lanes a, pown_er_lanes b)
  {
    const pown_er_lanes r = { back128(_mm512_div_round_pd(lo512(a.v), lo512(b.v), gaol_er)) };
    return r;
  }
  static GAOL_POWN_INLINE_T pown_er_lanes pown_add(pown_er_lanes a, pown_er_lanes b)
  {
    const pown_er_lanes r = { back128(_mm512_add_round_pd(lo512(a.v), lo512(b.v), gaol_er)) };
    return r;
  }
  static GAOL_POWN_INLINE pown_rests pown_first_rest(pown_er_lanes e) { return e.v; }
  static GAOL_POWN_INLINE void pown_add_rest(pown_rests& s, pown_er_lanes e) { s = _mm_or_pd(s, e.v); }
#  endif
#else
  struct pown_lanes { double l0, l1; };
  struct pown_rests { std::uint64_t l0, l1; };
  static GAOL_POWN_INLINE pown_lanes pown_mul(pown_lanes a, pown_lanes b)
  {
    const pown_lanes r = { a.l0*b.l0, a.l1*b.l1 };
    return r;
  }
  static GAOL_POWN_INLINE pown_lanes pown_fms(pown_lanes a, pown_lanes b, pown_lanes c)
  {
    const pown_lanes r = { fma_once(a.l0, b.l0, -c.l0), fma_once(a.l1, b.l1, -c.l1) };
    return r;
  }
  static GAOL_POWN_INLINE pown_lanes pown_fma(pown_lanes a, pown_lanes b, pown_lanes c)
  {
    const pown_lanes r = { fma_once(a.l0, b.l0, c.l0), fma_once(a.l1, b.l1, c.l1) };
    return r;
  }
  static GAOL_POWN_INLINE pown_lanes pown_twice_plus(pown_lanes a, pown_lanes b)
  {
    const pown_lanes r = { fma_once(2.0, a.l0, b.l0), fma_once(2.0, a.l1, b.l1) };
    return r;
  }
  static GAOL_POWN_INLINE pown_rests pown_first_rest(pown_lanes e)
  {
    const pown_rests s = { pown_bits_of(e.l0), pown_bits_of(e.l1) };
    return s;
  }
  static GAOL_POWN_INLINE void pown_add_rest(pown_rests& s, pown_lanes e)
  {
    s.l0 |= pown_bits_of(e.l0);
    s.l1 |= pown_bits_of(e.l1);
  }
#endif

  /*
    c^m as h + l in each lane, m >= 2, and the rests or-ed in s (see above).
    Inlined by force, so that each constant m of pown_powers() unrolls it:
    the loop and its tests cost as much as the products for a small m
    (x^3 took 6.1 ns rather than 4.1 with Clang 18 on an i7-1185G7, in a
    prototype).
  */
  template <class Lanes, class Rests>
  static GAOL_POWN_INLINE_T void pown_products(const Lanes& c, unsigned int m, Lanes& h, Lanes& l, Rests& s)
  {
    unsigned int bit = (1u << (pown_bit_length(m) - 1u)) >> 1;
    h = pown_mul(c, c);
    l = pown_fms(c, c, h); // c^2 = h + l, exactly
    s = pown_first_rest(l);
    for (;;) {
      if ((m & bit) != 0u) {
        const Lanes q = pown_mul(h, c);
        const Lanes f = pown_fms(h, c, q);
        pown_add_rest(s, f);
        l = pown_fma(l, c, f);
        h = q;
      }
      bit >>= 1;
      if (bit == 0u) {
        break;
      }
      const Lanes p = pown_mul(h, h);
      const Lanes e = pown_fms(h, h, p);
      pown_add_rest(s, e);
      l = pown_fma(l, pown_twice_plus(h, l), e); // 2h + l rounded once, 2h being exact
      h = p;
    }
  }

#define GAOL_POWN_CASE(k) case k: pown_products(c, k##u, h, l, s); break;
  template <class Lanes, class Rests>
  static GAOL_POWN_INLINE_T void pown_powers(const Lanes& c, unsigned int m, Lanes& h, Lanes& l, Rests& s)
  {
    switch (m) {
      GAOL_POWN_CASE(2) GAOL_POWN_CASE(3) GAOL_POWN_CASE(4) GAOL_POWN_CASE(5)
      GAOL_POWN_CASE(6) GAOL_POWN_CASE(7) GAOL_POWN_CASE(8) GAOL_POWN_CASE(9)
      GAOL_POWN_CASE(10) GAOL_POWN_CASE(11) GAOL_POWN_CASE(12) GAOL_POWN_CASE(13)
      GAOL_POWN_CASE(14) GAOL_POWN_CASE(15) GAOL_POWN_CASE(16) GAOL_POWN_CASE(17)
      GAOL_POWN_CASE(18) GAOL_POWN_CASE(19) GAOL_POWN_CASE(20) GAOL_POWN_CASE(21)
      GAOL_POWN_CASE(22) GAOL_POWN_CASE(23) GAOL_POWN_CASE(24) GAOL_POWN_CASE(25)
      GAOL_POWN_CASE(26) GAOL_POWN_CASE(27) GAOL_POWN_CASE(28) GAOL_POWN_CASE(29)
      GAOL_POWN_CASE(30) GAOL_POWN_CASE(31) GAOL_POWN_CASE(32)
    default:
      pown_products(c, m, h, l, s);
      break;
    }
  }
#undef GAOL_POWN_CASE

  /*
    The bounds of c^n for a double c and an int n, |n| >= 2, the rounding
    direction being upward: the corners pown_large_upward() does not take
    at once (see above). 0^n is 0 for n > 0, and +oo for n < 0, -oo at -0
    for an odd n, the limit of x^n at 0 on its side; +-oo^n is +-oo for n > 0
    and 0 for n < 0. Any other c is |c| = y 2^(e - 1), y in [1, 2), whose
    power is (h + l) 2^z, the product being scaled by 2^-500 above 2^500:
    exactly, but for a rest l below 2^-522, rounded within 2^-1074 of it,
    which the bound K holds. The bounds of y^n, which are within
    [2^-502, 2^502], are then scaled by 2^z, rounded once in their direction
    (gaol_ldexp()).
  */
  static void pown_point(double c, int n, double& lo, double& hi)
  {
    const unsigned int m = (n > 0) ? static_cast<unsigned int>(n) : 0u - static_cast<unsigned int>(n);
    const double g = std::fabs(c);
    if (g == 0.0 || g == GAOL_INFINITY) {
      lo = hi = ((g == 0.0) == (n < 0)) ? GAOL_INFINITY : 0.0;
    } else {
      // 2^500 and 2^-500
      const double big = 3.273390607896142e+150, small = 3.054936363499605e-151;
      const unsigned int L = pown_bit_length(m);
      int ey;
      const double y = 2.0*std::frexp(g, &ey); // exact
      --ey;
      unsigned int bit = (1u << (L - 1u)) >> 1;
      double h = y*y;
      double l = fma_once(y, y, -h);
      std::uint64_t s = pown_bits_of(l);
      long long z = 2LL*ey;
      for (;;) {
        if ((m & bit) != 0u) {
          const double q = h*y;
          const double f = fma_once(h, y, -q);
          s |= pown_bits_of(f);
          l = fma_once(l, y, f);
          h = q;
          z += ey;
        }
        if (h > big) {
          h *= small;
          l *= small;
          z += 500;
        }
        bit >>= 1;
        if (bit == 0u) {
          break;
        }
        const double p = h*h;
        const double e = fma_once(h, h, -p);
        s |= pown_bits_of(e);
        l = fma_once(l, fma_once(2.0, h, l), e);
        h = p;
        z *= 2;
        if (h > big) {
          h *= small;
          l *= small;
          z += 500;
        }
      }
      double K;
      if (n < 0) {
        const double q = 1.0/h;
        const double r = fma_once(-q, l, fma_once(-q, h, 1.0));
        const double qr = q*r;
        l = fma_once(qr, r, qr);
        h = q;
        s |= pown_bits_of(r);
        z = -z;
        K = pown_bound_inverse(m, L);
      } else {
        K = pown_bound(m, L);
      }
      K = (s != 0u) ? K : 0.0;
      // h > 0: the bound above, and the opposite of the bound below, of y^n,
      // then scaled
      const double up = h + fma_once(K, h, l);
      const double down = -h + fma_once(K, h, -l);
      const int w = (z > 2200) ? 2200 : ((z < -2200) ? -2200 : static_cast<int>(z));
      hi = gaol_ldexp(up, w);
      lo = -gaol_ldexp(down, w);
    }
    if (std::signbit(c) && (m & 1u) != 0u) {
      const double t = lo;
      lo = -hi;
      hi = -t;
    }
  }



  // Default format for output
  interval_format::format_t interval::output = interval_format::bounds;

  // Number of digits to print.
  std::streamsize interval::output_precision = 16;

  /* The square root rounded upward, whatever the rounding of ::sqrt. IEEE 754
     requires a square root to be rounded in the rounding direction in effect,
     but the C library of Visual C++ for 32-bit x86 rounds it to nearest in
     every direction. The result of ::sqrt, correctly rounded in some direction
     and thus within one float of the exact root, is compared with the exact
     root through s*s rounded downward, and moved to the next float above when
     it is not a bound yet; where ::sqrt rounds as it should, it is left
     unchanged. The next float is reached by adding the smallest denormal,
     rather than with nextafter(), which IBEX found to crash on ARM64 macOS when
     not rounding to nearest. To be called with the rounding direction set
     upward; the root rounded downward is gaol_minus_sqrt_down() below.

     s*s rounded downward, -((-s)*s) when rounding upward, is below x exactly
     when s*s is, x being a double, and s is then below sqrt(x) (GAOL v5:
     GAOL compared s with x/s, a division, several times slower than a
     product).

     The products of this check and of gaol_minus_sqrt_down() reach 2^1024 for
     an x next to the largest double, which raised the overflow exception for
     a finite root (GAOL v5, point 79 of TODO.md: sqrt([1, DBL_MAX])). From
     huge_root_argument, 2^1000, the root s >= 2^500 is scaled by 2^-512 and x
     by 2^-1024 (x >= 2^-24 is then normal), exactly, and the products are
     those of the check scaled by 2^-1024, rounded to the same bits. */
  static const double huge_root_argument = 1.0715086071862673e+301; // 2^1000
  static const double two_power_minus_512 = 7.458340731200207e-155;

  // s*s rounded downward < x, for s the root of x
  static inline bool square_down_below(double s, double x)
  {
    if (x > huge_root_argument) {
      const double t = read_in_branch(s)*two_power_minus_512;
      return -((-t)*t) < (read_in_branch(x)*two_power_minus_512)*two_power_minus_512;
    }
    return -((-s)*s) < x;
  }

  // u*u rounded upward == x, for u the root of x
  static inline bool square_up_equals(double u, double x)
  {
    if (x > huge_root_argument) {
      const double t = read_in_branch(u)*two_power_minus_512;
      return t*t == (read_in_branch(x)*two_power_minus_512)*two_power_minus_512;
    }
    return u*u == x;
  }

  static double gaol_sqrt_up(double x)
  {
    double s = ::sqrt(x);
    if (square_down_below(s, x)) { // s*s rounded downward < x proves s < sqrt(x)
      s += std::numeric_limits<double>::denorm_min();
    }
    return s;
  }

  /* The square root of x rounded downward, u being the square root rounded
     upward, to be called with the rounding direction set upward: u when it is
     the exact root, the double below u otherwise. u*u rounded upward is x
     exactly when u is the exact root, u*u being above x otherwise. Returned
     negated, as the SSE2 intervals store their lower bound (GAOL v5: GAOL
     computed x/u rounded downward, one double below the tightest bound for half
     of the doubles). sqrt() and sqrt_rel() take their lower bounds from it,
     without setting the rounding direction downward. */
  static double gaol_minus_sqrt_down(double x, double u)
  {
    return square_up_equals(u, x) ? -u : (-u + std::numeric_limits<double>::denorm_min());
  }

  /*
    The bounds of the elementary functions at the arguments where their value
    is a double, or one of the constants whose tightest bounds GAOL knows (fork
    of GAOL): the value of the mathematical library moved outward is not the
    tightest bound there. sin(0) = tan(0) = asin(0) = atan(0) = 0, cos(0) = 1,
    acos(1) = 0, acos(0) = asin(1) = atan(+oo) = pi/2, acos(-1) = pi,
    atan(1) = pi/4, and for the hyperbolic functions, from the libm moved three
    doubles outward: sinh(0) = tanh(0) = asinh(0) = atanh(0) = 0, cosh(0) = 1,
    acosh(1) = 0, cosh(x) and sinh(x) beyond the largest double for x >= 711,
    and tanh(x) above the double below 1 for x >= 20, 2 exp(-2x) being below
    2^-54 there.
    They are called between GAOL_RND_ENTER() and GAOL_RND_LEAVE(), the
    rounding direction being upward, and call the functions of namespace
    upward, which do not check it again (GAOL v5).
  */
  static inline double sin_lo(double x) { return (x == 0.0) ? 0.0 : upward::sin_dn(x); }
  static inline double sin_hi(double x) { return (x == 0.0) ? 0.0 : upward::sin_up(x); }
  static inline double cos_lo(double x) { return (x == 0.0) ? 1.0 : upward::cos_dn(x); }
  static inline double cos_hi(double x) { return (x == 0.0) ? 1.0 : upward::cos_up(x); }
  static inline double tan_lo(double x) { return (x == 0.0) ? 0.0 : upward::tan_dn(x); }
  static inline double tan_hi(double x) { return (x == 0.0) ? 0.0 : upward::tan_up(x); }

  /*
    The signs of sin(x) and cos(x), x being finite (GAOL v5, issue #6).
    Neither is 0 but sin(0): no other double is a multiple of pi/2, and the
    closest one, 6381956970095103 2^797, has a cosine of 4.7e-19, so that the
    value of the mathematical library, moved one double downward, still has
    the sign of the function. Next to 0 the sign is known from x. Rounding
    upward, after the GAOL_RND_ENTER() of the caller.
  */
  static inline int sign_of_sin(double x)
  {
    if (std::fabs(x) < 3.0) {
      return (x > 0.0) - (x < 0.0);
    }
    return (upward::sin_dn(x) > 0.0) ? 1 : -1;
  }

  static inline int sign_of_cos(double x)
  {
    if (std::fabs(x) < 1.5) {
      return 1;
    }
    return (upward::cos_dn(x) > 0.0) ? 1 : -1;
  }

  static double asin_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? gaol_half_pi_dn : ((x == -1.0) ? -gaol_half_pi_up : upward::asin_dn(x)));
  }

  static double asin_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? gaol_half_pi_up : ((x == -1.0) ? -gaol_half_pi_dn : upward::asin_up(x)));
  }

  static double acos_lo(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? gaol_half_pi_dn : ((x == -1.0) ? gaol_pi_dn : upward::acos_dn(x)));
  }

  static double acos_hi(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? gaol_half_pi_up : ((x == -1.0) ? gaol_pi_up : upward::acos_up(x)));
  }

  /*
    From |x| = 2^54 on, atan(x) = pi/2 - 1/x is above the double below pi/2
    (pi/2 exceeds it by 6.1e-17, and 1/x is at most 5.6e-17) and below pi/2,
    so that its bounds are those of pi/2, as they are at the infinities.
    CORE-MATH returns them from 0x1.d02967c31cdb5p+53 on without computing,
    but after a test that Visual C++ (x64 and arm64, /fp:strict) compiles
    with the sum x + x it returns for a NaN, which overflows from |x| = 2^1023
    on: atan([DBL_MAX]) and atan_rel() raised the overflow exception for a
    finite result (GAOL v5, point 79 of TODO.md; the continuous integration)
  */
  static const double atan_is_half_pi = 1.8014398509481984e+16; // 2^54

  // pi/4 is half of pi/2, exactly
  static double atan_lo(double x)
  {
    if (x == 0.0) {
      return 0.0;
    }
    const double a = std::fabs(x);
    if (a == 1.0 || a >= atan_is_half_pi) {
      const double b = (a == 1.0) ? 0.5 : 1.0;
      return (x > 0.0) ? gaol_half_pi_dn*b : -gaol_half_pi_up*b;
    }
    return upward::atan_dn(x);
  }

  static double atan_hi(double x)
  {
    if (x == 0.0) {
      return 0.0;
    }
    const double a = std::fabs(x);
    if (a == 1.0 || a >= atan_is_half_pi) {
      const double b = (a == 1.0) ? 0.5 : 1.0;
      return (x > 0.0) ? gaol_half_pi_up*b : -gaol_half_pi_dn*b;
    }
    return upward::atan_up(x);
  }

  /*
    Bounds of atan2(y, x) at a point other than (0, 0) (GAOL v5): 0, pi/2,
    pi, their opposites and pi/4 where the angle is one of them, a bound being
    an infinity included, where atan2 has that limit; the value of the
    mathematical library moved outward elsewhere, within [-pi, pi]. The sign of
    a zero does not count: atan2(0, x) is pi for x < 0.
  */
  static double atan2_lo(double y, double x)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      return (x > 0.0) ? 0.0 : gaol_pi_dn;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      return (y > 0.0) ? gaol_half_pi_dn : -gaol_half_pi_up;
    }
    if (x == -GAOL_INFINITY) {
      return (y > 0.0) ? gaol_pi_dn : -gaol_pi_up;
    }
    if (x == std::fabs(y)) {
      return (y > 0.0) ? gaol_half_pi_dn*0.5 : -gaol_half_pi_up*0.5;
    }
    return maximum(upward::atan2_dn(y,x), -gaol_pi_up);
  }

  static double atan2_hi(double y, double x)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      return (x > 0.0) ? 0.0 : gaol_pi_up;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      return (y > 0.0) ? gaol_half_pi_up : -gaol_half_pi_dn;
    }
    if (x == -GAOL_INFINITY) {
      return (y > 0.0) ? gaol_pi_up : -gaol_pi_dn;
    }
    if (x == std::fabs(y)) {
      return (y > 0.0) ? gaol_half_pi_up*0.5 : -gaol_half_pi_dn*0.5;
    }
    return minimum(upward::atan2_up(y,x), gaol_pi_up);
  }

  // x > 0 finite as m 2^e, m an odd integer, read from the bits of x (in
  // pow_is_double() and hypot()): std::frexp(), std::ldexp() and a loop over
  // the trailing zeros cost as much as CORE-MATH's pow. The lowest set bit of
  // the significand is a power of two below 2^53, a double whose exponent is
  // the number of trailing zeros
  static inline void odd_significand(double x, std::uint64_t& m, int& e)
  {
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    const int field = static_cast<int>(bits >> 52); // no sign bit; 0 for a subnormal
    m = bits & ((static_cast<std::uint64_t>(1) << 52) - 1);
    e = -1074;
    if (field != 0) {
      m |= static_cast<std::uint64_t>(1) << 52;
      e = field - 1075;
    }
    const double lowest = static_cast<double>(m & (~m + 1)); // exact
    std::memcpy(&bits, &lowest, sizeof bits);
    const int zeros = static_cast<int>(bits >> 52) - 1023;
    m >>= zeros;
    e += zeros;
  }

  /*
    Whether x^y is a double, and that double d, for a finite x > 0 other than
    1 and a finite y other than 0 (GAOL v5). pow_lo() takes d as the lower
    bound, where the double below CORE-MATH's value rounded upward is one
    double below it: pow([4], 0.5) was [2 - 2^-52, 2]. Calling CORE-MATH
    downward too would tell, at twice the cost of every power: this proves it
    instead, with integers and with operations on doubles that are exact
    whatever the rounding direction, and leaves at the first tests for nearly
    every y. y is a/2^k, a an odd integer and k >= 1, or an integer a (k = 0),
    and x is m 2^e, m an odd integer.
    - m = 1: x^y = 2^(e y), a double where e y is an integer of [-1074, 1023],
      and irrational elsewhere. As |e| <= 1074 < 2^11, e y is an integer only
      where 2^k divides e, k <= 10: 1024 y is an integer j, and e j a multiple
      of 1024. The product e*y of doubles could not tell: for e = 3 and y the
      double nearest 1/3, 3 y = 1 - 2^-54 rounds to 1.
    - m >= 3: x^y is rational only where x is the 2^k-th power of a rational
      z (a and 2^k being coprime), which is c 2^f with c odd, x being dyadic:
      m = c^(2^k), e = f 2^k, and x^y = z^a = c^a 2^(f a). That is no double
      for a < 0 (c >= 3), and a double for a > 0 where c^a < 2^53,
      f a >= -1074 and c^a 2^(f a) < 2^1024. c >= 3 gives a <= 33
      (3^33 < 2^53 < 3^34) and 2^k <= 33 (3^(2^k) <= m < 2^53): y is a multiple
      of 1/32 in (0, 33]. Then 2^k has to divide e, and m to be a 2^k-th
      power: the square root of a double is exact at a perfect square below
      2^53, and r*r is m for no integer r otherwise. c^a is a product of
      doubles, exact below 2^53, and at least 2^53 above it, 2^53 being a
      double.
  */
  static bool pow_is_double(double x, double y, double& d)
  {
    if (!(y >= -1075.0 && y <= 1075.0)) {
      return false; // |e y| > 1074 for |e| >= 1
    }
    const double scaled = 1024.0*y; // exact, unless flush-to-zero makes it 0 (|y| < 2^-1032)
    const int j = static_cast<int>(scaled);
    if (j == 0 || static_cast<double>(j) != scaled) {
      return false; // k > 10, j being 0 for y != 0 only where 1024 y was flushed to 0
    }
    std::uint64_t m;
    int e;
    odd_significand(x, m, e);
    if (m == 1) {
      const int t = e*j; // 1024 e y, below 1.2e9 in magnitude
      if (t % 1024 != 0 || t < -1074*1024 || t > 1023*1024) {
        return false;
      }
      d = std::ldexp(1.0, t/1024);
      return true;
    }
    if (j <= 0 || j > 33*1024 || (j >= 2048 && m >= (static_cast<std::uint64_t>(1) << 27))) {
      return false; // c^a = m^y >= 2^54 for y >= 2 and m >= 2^27, as for most x
    }
    // y = a/2^k: j = a 2^(10-k), whose lowest set bit is 2^(10-k) for k > 0
    const double lowest = static_cast<double>(j & -j); // exact
    std::uint64_t bits;
    std::memcpy(&bits, &lowest, sizeof bits);
    const int zeros = static_cast<int>(bits >> 52) - 1023;
    const int k = (zeros >= 10) ? 0 : 10 - zeros, a = j >> (10 - k);
    if (k > 5 || (static_cast<unsigned int>(e) & ((1u << k) - 1u)) != 0u) {
      return false; // 2^k does not divide e
    }
    for (int i = 0; i < k; ++i) {
      const std::uint64_t r = static_cast<std::uint64_t>(std::sqrt(static_cast<double>(m)));
      if (r*r != m) {
        return false;
      }
      m = r;
    }
    const double c = static_cast<double>(m);
    double p = c; // c^a
    for (int i = 1; i < a; ++i) {
      p *= c;
      if (!(p < 9007199254740992.0)) { // 2^53
        return false;
      }
    }
    std::memcpy(&bits, &p, sizeof bits);
    const int b = static_cast<int>(bits >> 52) - 1022; // 2^(b-1) <= c^a < 2^b
    const int s = e/(1 << k)*a; // f a
    if (s < -1074 || s + b > 1024) {
      return false;
    }
    d = std::ldexp(p, s); // exact
    return true;
  }

  /*
    CORE-MATH's pow(x, y) in the upward rounding, for a finite x > 0 and a y
    of normal magnitude, without the overflow exception where x^y is above
    the largest double (GAOL v5, point 79 of TODO.md). The reverse functions of
    pow look for the bounds of c^(1/y) and log(c)/log(x) by powers d^y of the
    doubles on either side of them, which overflow for a finite bound when c
    is next to the largest double, or when y is huge, where d^y is beyond the
    doubles for a d one double above 1. The answer there is +oo, the value
    CORE-MATH gives, which the tests against a finite bound take as they take
    the exception. The functions pow_rounded_up(), pow_rounded_dn(), pow_lo()
    and pow_hi() call this one for the template argument quiet, which the
    reverse functions give: pow keeps raising the exception for its infinite
    upper bound, as doc/using.md says of the infinite bounds.
    x^y can only overflow where it is above 1, for x > 1 and y > 0 or
    x < 1 and y < 0, and it is the tiny power, 2^-1074 rounded upward, in the
    other cases beyond t = 1100 below, where CORE-MATH squares y log(x) in its
    first phase, which overflows from 2^512 on. The binary logarithm of x^y is
    bounded by |y| |log2(x)|, with |log2(x)| at most |e| + 1, e being the
    exponent of x as frexp() gives it: when that is below 1000, nothing
    overflows. From |y| >= 2^500 on, |y log2(x)| >= 2^447, and x^y is beyond
    the doubles, +oo or 2^-1074. Otherwise
    t = |y| |log(x)|/log(2) is computed from the logarithm of CORE-MATH,
    which is within a few units of 2^-52 of it relatively, so that
    x^y < 2^(1024 - 2^-30) is no overflow for t < 1024 - 10^-9, and x^y is
    beyond the doubles for t > 1024 + 10^-9. Only the powers in between ask
    CORE-MATH to decide: they are computed with the exceptions masked, and
    the environment of the program, its flags and the exceptions it enabled,
    restored.
  */
  static double pow_up_without_overflow_exception(double x, double y)
  {
    if (!(x > 0.0 && x < GAOL_INFINITY) || x == 1.0) {
      return upward::nthroot_up(x, y);
    }
    const bool grows = (x > 1.0) == (y > 0.0);
    const double magnitude = std::fabs(y);
    // |e| from the exponent field of x: e = field - 1022 for a normal x, and
    // 1074 at most for a subnormal one
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    const int field = static_cast<int>((bits >> 52) & 0x7ffu);
    const int e = (field == 0) ? 1074 : std::abs(field - 1022);
    if (magnitude < 1000.0/(e + 1)) {
      return upward::nthroot_up(x, y);
    }
    if (magnitude >= two_power_500) {
      return grows ? GAOL_INFINITY : std::numeric_limits<double>::denorm_min();
    }
    // 1/log(2), rounded to nearest: the margins below are far larger than its error
    const double t = magnitude*std::fabs(upward::log_up(x))*1.4426950408889634;
    if (!grows) {
      // x^y < 1: nothing overflows but the square of y log(x) in the first
      // phase of CORE-MATH, which is 2^1024 for |y log(x)| = 2^512 (Clang):
      // the power is below 2^-1100 from t = 1100 on, and rounds upward to 2^-1074
      return (t >= 1100.0) ? std::numeric_limits<double>::denorm_min() : upward::nthroot_up(x, y);
    }
    if (t < 1023.999999999) {
      return upward::nthroot_up(x, y);
    }
    if (t > 1024.000000001) {
      return GAOL_INFINITY;
    }
#if GAOL_POW_MASKS_MXCSR
    // On x86 the doubles are computed by SSE instructions, whose exceptions
    // the control register MXCSR masks (bits 7 to 12) and whose flags it holds
    // (bits 0 to 5): written back as it was, it gives the program its masks
    // and flags exactly, where the functions of <cfenv> of MinGW-w64 12 to 14
    // left the flags of the masked section raised after fesetenv(), and
    // crashed with fesetexceptflag() (the continuous integration)
    const unsigned int csr = _mm_getcsr();
    _mm_setcsr(csr | 0x1f80u);
    const double power = upward::nthroot_up(x, y);
    _mm_setcsr(csr);
    return power;
#else
    std::fenv_t environment;
    std::feholdexcept(&environment);
    const double power = upward::nthroot_up(x, y);
    std::fesetenv(&environment);
    return power;
#endif
  }

  /*
    x^y rounded upward for x >= 0 and a finite y, the rounding direction
    being upward: CORE-MATH's pow, which is correctly rounded in that
    direction, but for an exponent of extreme magnitude, |y| < 2^-969 or
    |y| >= 2^1014, and a finite x > 0 other than 1, whose power is computed
    here (GAOL v5). For those, the first phase of CORE-MATH's pow makes its
    approximation of log(x) a NaN on purpose (the exponent field of y below
    0x36 or from 0x7f5 on), so that its rounding test fails, and compares the
    NaN product with 2^-1022 with <, which raises the invalid-operation
    exception: pow([1, 2], [1e-300]) and pow([1, 2], [1e300, 1e308]) raised
    it, and killed a program that enabled it. The value is known there,
    |log(x)| being between 2^-54 and 745 for a positive double x other than 1:
    - |y| < 2^-969: 0 < |y log(x)| < 2^-959, and x^y is within 2^-958 of 1,
      above 1 where y log(x) > 0: x^y rounded upward is the double above 1
      there, and 1 otherwise;
    - |y| >= 2^1014: |y log(x)| > 2^960, and x^y is above DBL_MAX where
      y log(x) > 0, rounded upward to +oo, and below 2^-1074 otherwise,
      rounded upward to 2^-1074.
    These are the values CORE-MATH gives, and the double below each the lower
    bound it gives (pow_rounded_dn()). 1^y and x^0 are 1; CORE-MATH takes 0
    and +oo apart before its first phase.
  */
  template <bool quiet = false>
  static inline double pow_rounded_up(double x, double y)
  {
    // 2^-969 and 2^1014, written in decimal with the digits that give them
    // exactly: C++11 has no hexadecimal floating literal
    const double magnitude = std::fabs(y);
    // 2^450 too: from |y| = 2^450 on, a power below 1 is the tiny one, which
    // CORE-MATH, squaring y log(x) in its first phase, raised the overflow
    // exception to find, from |y log(x)| = 2^512 on (Clang), and the quiet
    // powers do not need it either (GAOL v5, point 79 of TODO.md)
    if (magnitude >= 2.0041683600089728e-292 && magnitude < (quiet ? 1.7555597020139804e+305 : 2.9073548971824276e+135)) {
      return quiet ? pow_up_without_overflow_exception(x, y) : upward::nthroot_up(x, y);
    }
    if (!quiet && magnitude >= 2.9073548971824276e+135 && magnitude < 1.7555597020139804e+305) {
      return (x > 0.0 && x < GAOL_INFINITY && x != 1.0 && (x > 1.0) != (y > 0.0))
        ? std::numeric_limits<double>::denorm_min() : upward::nthroot_up(x, y);
    }
    if (x == 1.0 || y == 0.0) {
      return 1.0;
    }
    if (x == 0.0 || x == GAOL_INFINITY) {
      return upward::nthroot_up(x, y);
    }
    const bool above_one = (x > 1.0) == (y > 0.0);
    if (magnitude < 1.0) {
      return above_one ? 1.0 + std::numeric_limits<double>::epsilon() : 1.0;
    }
    return above_one ? GAOL_INFINITY : std::numeric_limits<double>::denorm_min();
  }

  template <bool quiet = false>
  static inline double pow_rounded_dn(double x, double y)
  {
    return previous_float(pow_rounded_up<quiet>(x, y));
  }

  /*
    x^y at a corner of a box, x in [0, +oo] and y in [-oo, +oo], rounded
    downward (pow_lo()) and upward (pow_hi()): 1 for x = 1 or y = 0; the
    limit of x^y where x is 0 or +oo or y infinite, 0 or +oo, which bounds it
    both ways, +oo where x > 1 and y > 0 or x < 1 and y < 0, 0 otherwise
    (GAOL v5: pow(x, y) took exp(y*log(x)) for these boxes, and its bounds
    were one to hundreds of doubles beyond the tightest ones, and
    pow_is_double() does not take them: 0 has no odd significand, +oo is
    read as 2^1024); elsewhere x^y itself where it is a double
    (pow_is_double()), the double below CORE-MATH's value rounded upward
    otherwise, and at least 0, for the lower bound, and CORE-MATH's value for
    the upper one
  */
  static inline bool pow_is_limit(double x, double y)
  {
    return !(x > 0.0 && x < GAOL_INFINITY && std::fabs(y) < GAOL_INFINITY);
  }

  static inline double pow_limit(double x, double y)
  {
    return ((x > 1.0) == (y > 0.0)) ? GAOL_INFINITY : 0.0;
  }

  template <bool quiet = false>
  static inline double pow_lo(double x, double y)
  {
    double d;
    if (x == 1.0 || y == 0.0) {
      return 1.0;
    }
    if (pow_is_limit(x, y)) {
      return pow_limit(x, y);
    }
    return pow_is_double(x, y, d) ? d : maximum(pow_rounded_dn<quiet>(x,y), 0.0);
  }

  template <bool quiet = false>
  static inline double pow_hi(double x, double y)
  {
    if (x == 1.0 || y == 0.0) {
      return 1.0;
    }
    return pow_is_limit(x, y) ? pow_limit(x, y) : pow_rounded_up<quiet>(x,y);
  }

  static inline double sinh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 711.0) ? std::numeric_limits<double>::max() : upward::sinh_dn(x));
  }

  static inline double sinh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -711.0) ? -std::numeric_limits<double>::max() : upward::sinh_up(x));
  }

  // cosh(x), cosh(-x): the lower bound for |x|, the upper bound for 0
  static inline double cosh_lo(double x)
  {
    return (x == 0.0) ? 1.0 : ((std::fabs(x) >= 711.0) ? std::numeric_limits<double>::max() : upward::cosh_dn(x));
  }

  static inline double cosh_hi(double x)
  {
    return (x == 0.0) ? 1.0 : upward::cosh_up(x);
  }

  // 1 - 2^-53, the double below 1, exactly
  static inline double tanh_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x >= 20.0) ? 1.0 - 0.5*std::numeric_limits<double>::epsilon() : upward::tanh_dn(x));
  }

  static inline double tanh_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x <= -20.0) ? -(1.0 - 0.5*std::numeric_limits<double>::epsilon()) : upward::tanh_up(x));
  }

  static inline double asinh_lo(double x) { return (x == 0.0) ? 0.0 : upward::asinh_dn(x); }
  static inline double asinh_hi(double x) { return (x == 0.0) ? 0.0 : upward::asinh_up(x); }
  static inline double acosh_lo(double x) { return (x == 1.0) ? 0.0 : upward::acosh_dn(x); }
  static inline double acosh_hi(double x) { return (x == 1.0) ? 0.0 : upward::acosh_up(x); }
  static inline double atanh_lo(double x) { return (x == 0.0) ? 0.0 : upward::atanh_dn(x); }
  static inline double atanh_hi(double x) { return (x == 0.0) ? 0.0 : upward::atanh_up(x); }

  // feven() of an integer i that an operation computed: neither i nor its
  // half is subnormal, so that the modes that flush the subnormals to zero
  // change nothing here
  static bool integer_is_even(double i)
  {
    return (std::floor(0.5*i)*2.0 == i);
  }

  /*
    \brief test for evenness
    \warning d should not be +/-oo
  */
  bool feven(double d)
  {
    // A subnormal is no integer, which its bits tell whatever the modes that
    // flush the subnormals to zero: under denormals-are-zero, 0.5*d and d
    // were 0 for the comparison, and feven(2^-1074) was true (GAOL v5,
    // point Q of TODO.md)
    if (gaol_detail::bound_is_subnormal(d)) {
      return false;
    }
    return integer_is_even(d);
  }


  /*
    The double above x, from its bits: the next one away from -oo, -0 above
    the least negative subnormal, the least positive subnormal above a zero,
    +oo above DBL_MAX; a NaN and +oo stay as they are. nextafter() of the C
    library computes with doubles, which the modes that flush the subnormals
    to zero change: under denormals-are-zero, the UCRT of Windows and the
    libm of macOS (arm64 and x86_64) did not give the subnormal above 100
    times 2^-1074 (GAOL v5, point Q of TODO.md, continuous integration)
  */
  static double next_double_above(double x)
  {
    if (!(x == x) || x == GAOL_INFINITY) {
      return x;
    }
    std::uint64_t bits = gaol_detail::bound_bits(x);
    if ((bits << 1) == 0) {
      return std::numeric_limits<double>::denorm_min();
    }
    if ((bits >> 63) != 0) {
      --bits; // a smaller magnitude
    } else {
      ++bits;
    }
    double above;
    std::memcpy(&above, &bits, sizeof above);
    return above;
  }

  // The bounds compared as the relations compare them, and the double above
  // the left one taken from its bits, whatever the modes that flush the
  // subnormals to zero: under denormals-are-zero, [2^-1074, 3*2^-1074] was
  // canonical (GAOL v5, point Q of TODO.md)
  bool interval::is_canonical(void) const
  {
#if defined (_MSC_VER)
    return !is_empty() && gaol_detail::bound_greater_equal(next_double_above(left()), right());
#else
    // emptyset handled thanks to unorderedness of NaNs, with a quiet
    // comparison, which raises no invalid-operation exception on them (GAOL v5)
    return gaol_detail::bound_greater_equal(next_double_above(left()),right());
#endif
  }


  void interval::format(interval_format::format_t f)
  {
    output = f;
  }

  interval_format::format_t interval::format(void)
  {
    return output;
  }


  // Sets the failbit of is, without the std::ios_base::failure setstate()
  // throws when the exceptions of is include failbit: the error of a line
  // that is no interval is GAOL's exception. The bit stays set.
  static void set_failbit(istream& is)
  {
    try {
      is.setstate(std::ios_base::failbit);
    } catch (const std::ios_base::failure&) {
      // failbit is set
    }
  }

  /*
    Reads an interval from a line, after the blanks that precede it, line ends
    included, which std::ws skips as the reading of a double does, and under
    std::noskipws too, which keeps a double from skipping them (GAOL v5).
    GAOL read the line where the previous value stopped: over
    "1.5\n[1, 2]\n", "in >> d >> x" read the empty rest of the first line,
    and threw input_format_error, as while (in >> x) did over a file ending
    with an empty line. A blank line is now no line to read, and the intervals
    around it are read, as two numbers with a blank line between them are.

    Where no line is left, std::getline() fails, which sets failbit: I is left
    as it was and nothing is thrown, as for a double, so that while (is >> x)
    ends at the end of the input (GAOL v5). GAOL read the empty text then,
    threw input_format_error and emptied I, so that such a loop always ended
    with an exception. A line that is no interval sets failbit too, I becomes
    the empty set, and the exception of the reader is thrown:
    input_format_error, or invalid_action_error for a function called with an
    argument it does not take. A program reading on calls is.clear() first.

    The std::ws of libstdc++ before GCC 12 constructs no sentry (libc++, the
    library of Visual C++, and libstdc++ from GCC 12 on, construct one): it
    neither flushes the stream tied to is nor looks at the state of is, and
    reads the buffer whatever that state is. Used alone, it would crash there
    on an istream without buffer, consume the blanks of a stream that has
    failed, and leave a prompt written on cout unflushed until the user had
    typed the interval. The sentry of any extraction is therefore constructed
    first, without skipping (std::getline() does the same): it flushes the
    tied stream, and sets failbit where is is not good.
  */
  istream& operator >>(istream& is, interval& I)
  {
    std::string buffer;

    if (!istream::sentry(is,true)) {
      return is;
    }
    if (!std::getline(is >> std::ws,buffer)) {
      return is;
    }

    bool read;
    try {
      read = gaol_parse_string(buffer.data(), buffer.size(), I, parsing_names::gaol);
    } catch (...) { // An error the reader raised as it read the line
      I = interval::emptyset();
      set_failbit(is);
      throw;
    }
    if (!read) {
      std::string err_msg("Syntax error in expression of interval: ");
      err_msg += gaol_detail::text_in_message(buffer);
      I = interval::emptyset();
      set_failbit(is);
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    return is;
  }

  /*
    Moves the last digit of text by one unit, away from zero or toward it,
    until the number it writes is on the side of the double `magnitude` asked
    for: at least it if `away`, at most it otherwise. text is a positive
    number written to nearest in the fixed format, or in the scientific one if
    `scientific`: one digit, the others after a point, and an exponent with
    its sign.
  */
  static void move_to_side(std::string& text, double magnitude, bool away, bool scientific)
  {
    for (int moves = 0; moves < 4; ++moves) {
      const std::size_t last = scientific ? text.find_first_of("eE") : text.size(); // One past the mantissa
      const int sign = gaol_detail::compare_number_with_double(text.c_str(), magnitude);
      if (sign == 0 || (sign > 0) == away) {
        return;
      }
      std::size_t i = last;
      bool carry = true;
      while (carry && i > 0) {
        --i;
        if (text[i] == '.') {
          continue;
        }
        if (away) {
          carry = (text[i] == '9');
          text[i] = carry ? '0' : static_cast<char>(text[i] + 1);
        } else {
          carry = (text[i] == '0');
          text[i] = carry ? '9' : static_cast<char>(text[i] - 1);
        }
      }
      int exponent_move = 0;
      if (away && carry) {
        // 9.99 has become 0.00: 1.00 with the next exponent, or 10.00
        if (scientific) {
          text[0] = '1';
          exponent_move = 1;
        } else {
          text.insert(0, 1, '1');
        }
      } else if (!away && scientific && text[0] == '0') {
        // 1.00 has become 0.99: 9.99 with the previous exponent, which is
        // below the magnitude as well, 1.00 being it rounded to nearest
        text[0] = '9';
        exponent_move = -1;
      } else if (!away && text[0] == '0' && text.size() > 1 && text[1] != '.') {
        text.erase(0, 1); // 0999.99
      }
      if (exponent_move != 0) {
        const long e = std::strtol(text.c_str() + last + 1, NULL, 10) + exponent_move;
        std::ostringstream exponent;
        exponent << ((e < 0) ? '-' : '+') << std::setw(2) << std::setfill('0') << ((e < 0) ? -e : e);
        text.replace(last + 1, std::string::npos, exponent.str());
      }
    }
  }

  /*
    What GAOL takes from a stream to write a number: its flags, its precision
    and its locale, whose facet numpunct gives the decimal point and, if
    `grouped`, the grouping of the digits before it (GAOL v5). The text of an
    interval is made from these in a character string, rather than in
    std::ostringstreams, one for each interval and one for each bound: each
    built a locale and its cached facets, and destroyed them, a third of the
    work of writing an interval.
  */
  namespace {
  struct text_format {
    std::ios_base::fmtflags flags;
    std::streamsize precision;
    std::locale loc;
    const std::numpunct<char>& punct; // The facet of loc, which the copy keeps
    bool grouped;

    text_format(std::ios_base::fmtflags f, std::streamsize p, const std::locale& l, bool g = false)
      : flags(f), precision(p), loc(l), punct(std::use_facet<std::numpunct<char> >(loc)), grouped(g)
    {
    }
  };

  // How the text of a number is rounded: to nearest, as a stream writes a
  // double, or downward or upward for the bound of an interval
  enum text_rounding { text_nearest, text_downward, text_upward };
  } // namespace

  /*
    x written by a stream that has the flags, the precision and the locale of
    fmt, under the rounding direction asked: what number_to_text() leaves to
    the C library and the facets of the locale, whatever they are, as it
    cannot write it as the stream does. That is a NaN, the hexadecimal
    floating-point format, and a conversion of the C library that fails.
  */
  static std::string stream_text(double x, text_rounding rounding, const text_format& fmt)
  {
    std::ostringstream out;
    out.flags(fmt.flags);
    out.precision(fmt.precision);
    out.imbue(fmt.loc);
    // Zero has no digit to round, and the C runtime of Windows, asked to write
    // it in the upward direction, writes 0.1 (the point interval [0, 0] came
    // out as <0.0, 0.1>): it is written to nearest. The other cases keep the
    // direction of the bound, the hexadecimal format being rounded by the C
    // library when the stream limits its digits.
    if (x == 0.0 || rounding == text_nearest) {
      round_nearest();
    } else if (rounding == text_downward) {
      round_downward();
    }
    out << x;
    round_upward();
    return out.str();
  }

  /*
    The magnitude of a double written by the C library with `prec` digits after
    the point, in the fixed format, or in the scientific one, rounded to
    nearest: the same digits as the stream writes. printf follows the decimal
    point of the C locale, a comma under some, where a stream of C++ follows
    its own locale: the point is written as a '.' whatever the locale, from
    its position among the digits, which the precision gives.
  */
  static bool magnitude_text(std::string& text, double magnitude, bool fixed, int prec)
  {
    char buffer[512];
    const char *const format = fixed ? "%.*f" : "%.*e";
    round_nearest();
    int n = std::snprintf(buffer, sizeof buffer, format, prec, magnitude);
    if (n >= 0 && static_cast<std::size_t>(n) < sizeof buffer) {
      text.assign(buffer, static_cast<std::size_t>(n));
    } else if (n >= 0) {
      text.assign(static_cast<std::size_t>(n) + 1, '\0');
      n = std::snprintf(&text[0], text.size(), format, prec, magnitude);
      text.resize((n >= 0) ? static_cast<std::size_t>(n) : 0);
    }
    round_upward();
    if (n < 0 || text.empty()) {
      return false;
    }
    if (prec > 0) {
      // 3.14e+00 or 3.14: the point is where the digits before it end, and it
      // ends where the digits after it, of the precision, begin
      const std::size_t point = fixed ? text.find_first_not_of("0123456789") : 1;
      const std::size_t digits_end = fixed ? text.size() : text.find('e');
      if (point == std::string::npos || digits_end == std::string::npos
          || digits_end < point + static_cast<std::size_t>(prec) + 1) {
        return false;
      }
      const std::size_t length = digits_end - static_cast<std::size_t>(prec) - point;
      if (length != 1 || text[point] != '.') {
        text.replace(point, length, 1, '.');
      }
    }
    return true;
  }

  /*
    The digits text starts with, those before its decimal point, grouped as
    std::num_put groups those of a double: from the point leftward, each group
    as long as its character of grouping() says, the last one repeated, and
    no group past a size of 0, a negative one or CHAR_MAX, which the standard
    takes as unlimited (22.4.3.1.2). libstdc++ differs there for a facet a
    program defines: it repeats the size before a 0 ("\3\0" groups 1234567 as
    1,234,567, where libc++ and GAOL write 1234,567), and takes the sizes 128
    to 254 for negative ones where char is unsigned (ARM, ppc64le, s390x).
  */
  static void group_digits(std::string& text, const std::numpunct<char>& punct)
  {
    const std::string grouping = punct.grouping();
    std::size_t pos = text.find_first_not_of("0123456789");
    if (pos == std::string::npos) {
      pos = text.size();
    }
    std::size_t g = 0;
    while (g < grouping.size()) {
      const char size = grouping[g];
      if (size <= 0 || size == CHAR_MAX || pos <= static_cast<std::size_t>(size)) {
        break;
      }
      pos -= static_cast<std::size_t>(size);
      text.insert(pos, 1, punct.thousands_sep());
      if (g + 1 < grouping.size()) {
        ++g;
      }
    }
  }

  /*
    The text of a number x written as a stream with the flags, the precision
    and the locale of fmt would write it, and, for the bound of an interval,
    rounded downward, or upward, whatever the C library does. GAOL set the
    rounding direction and let the C library write x, but not every C library
    rounds its decimal conversions in the rounding direction: the C runtime of
    Windows rounds the magnitude, so that -2/3 rounded downward was written
    -0.6666, and musl on 64-bit ARM processors rounds to nearest whatever the
    direction (issue #3). The magnitude is now written rounding to nearest, in
    the fixed format or in the scientific one, and compared exactly with x:
    when it is on the wrong side of x, its last digit is moved by one. The
    general format is made from the scientific one, by the rules of printf's
    %g, rather than asked of the C library: glibc 2.31 writes 999999.5 with six
    digits and the zeros kept "1.e+06", whose last digit is not the sixth.
    The text of a zero, or of an infinity, is made the same way, without
    the comparison: "inf" is "INF" with the flag uppercase, but in the fixed
    format, whose conversion the standard has as %f (a stream of libc++, which
    uses %F, wrote "INF" there). With fmt.grouped, the
    digits before the point are grouped as the locale has them, as a stream
    groups those of a double: the midpoint and the radius of the width and
    center formats, both alike (GAOL v5: a stream wrote the midpoint, grouped,
    and GAOL the radius, which was not). The other texts are never grouped.
    Left to a stream, with the rounding direction set (stream_text()): what
    the text cannot be made of.
  */
  static std::string number_to_text(double x, text_rounding rounding, const text_format& fmt)
  {
    const std::ios_base::fmtflags floatfield = fmt.flags & std::ios_base::floatfield;
    // A subnormal under denormals-are-zero, which a program may set, compares
    // equal to 0. operator<< and intervalToText() clear that mode before
    // they write (GAOL v5, point Q of TODO.md), where GAOL can (x86, and ARM
    // with GCC and Clang): only where it cannot (ARM with Visual C++, the
    // program having set FZ itself) does a subnormal come here. The C
    // library may write it 0 under the mode (the snprintf called below did
    // for 5e-324 with MSYS2 CLANG64, gdtoa does), and GAOL wrote it to
    // nearest, below an upper bound: a bound is written as the double on its
    // outer side that no mode changes, the smallest normal double, 2^-1022,
    // away from zero, and 0 toward it, loose but outward (GAOL v5, point Q).
    // A number written to nearest is written by a stream, as before. Tested
    // before the hexadecimal format, which a stream writes too.
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    if (x == 0.0 && (bits << 1) != 0) {
      if (rounding == text_nearest) {
        return stream_text(x, text_nearest, fmt);
      }
      const bool away = (rounding == text_upward) == !std::signbit(x);
      const double smallest_normal = std::numeric_limits<double>::min();
      return number_to_text(away ? (std::signbit(x) ? -smallest_normal : smallest_normal) : 0.0, rounding, fmt);
    }
    if (!(x == x) || floatfield == (std::ios_base::fixed | std::ios_base::scientific)) {
      return stream_text(x, rounding, fmt);
    }

    const bool fixed = (floatfield == std::ios_base::fixed);
    const bool general = (floatfield == 0);
    const bool showpoint = ((fmt.flags & std::ios_base::showpoint) != 0);
    std::string text;
    if (x == GAOL_INFINITY || x == -GAOL_INFINITY) {
      text = ((fmt.flags & std::ios_base::uppercase) && !fixed) ? "INF" : "inf";
    } else {
      // A stream writes 6 digits for a negative precision: so are the numbers
      // written to nearest and the zeros. A bound that is not 0 takes it as 0
      // in the general format, 1 digit, and as 6 in the fixed and scientific
      // ones, as the stream that wrote it did
      const bool as_stream = (rounding == text_nearest || x == 0.0);
      const std::streamsize precision = (as_stream && fmt.precision < 0) ? 6 : fmt.precision;
      // %g takes a precision of 0 as 1, and writes that many significant digits
      const std::streamsize digits = (general && precision <= 0) ? 1 : precision;
      std::streamsize prec = general ? digits - 1 : precision;
      if (prec < 0) {
        prec = 6;
      } else if (prec > std::numeric_limits<int>::max() / 4) {
        prec = std::numeric_limits<int>::max() / 4;
      }
      const double magnitude = std::fabs(x);
      if (!magnitude_text(text, magnitude, fixed, static_cast<int>(prec))) {
        // snprintf failed (a precision its buffer cannot hold), or its point
        // was not found: the digits of a stream of the C locale, to nearest,
        // as GAOL wrote them before, then moved outward as above. Never those
        // of the C library under the directed rounding, which the C runtime of
        // Windows and musl do not honour (issue #3)
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out.setf(fixed ? std::ios_base::fixed : std::ios_base::scientific, std::ios_base::floatfield);
        out.precision(prec);
        round_nearest();
        out << magnitude;
        round_upward();
        text = out.str();
      }
      // The magnitude written has to be at least that of x when rounding away
      // from zero, and at most that of x otherwise
      if (rounding != text_nearest && x != 0.0) {
        move_to_side(text, magnitude, (rounding == text_upward) == (x > 0.0), !fixed);
      }

      if (general) {
        // d.ddde+XX as %g writes it: without an exponent when XX is from -4 to
        // the precision, and without the zeros ending its fractional part,
        // unless showpoint
        const std::size_t e = text.find('e');
        const long exponent = std::strtol(text.c_str() + e + 1, NULL, 10);
        std::string mantissa = text.substr(0, e);
        mantissa.erase(1, (mantissa.size() > 1) ? 1 : 0); // The digits
        std::string tail;
        if (exponent < -4 || exponent >= digits) {
          tail = text.substr(e);
          mantissa.insert(1, 1, '.');
        } else if (exponent >= 0) {
          mantissa.insert(static_cast<std::size_t>(exponent) + 1, 1, '.');
        } else {
          mantissa.insert(0, "0." + std::string(static_cast<std::size_t>(-exponent - 1), '0'));
        }
        if (!showpoint) {
          mantissa.erase(mantissa.find_last_not_of('0') + 1);
          if (mantissa[mantissa.size() - 1] == '.') {
            mantissa.erase(mantissa.size() - 1);
          }
        }
        text = mantissa + tail;
      } else if (precision == 0 && showpoint) {
        text.insert(fixed ? text.size() : text.find('e'), 1, '.');
      }

      const char point = fmt.punct.decimal_point();
      for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '.') {
          text[i] = point;
        } else if (text[i] == 'e' && (fmt.flags & std::ios_base::uppercase)) {
          text[i] = 'E';
        }
      }
      // After the point is in place: the separator of the groups may be '.'
      if (fmt.grouped) {
        group_digits(text, fmt.punct);
      }
    }
    if (std::signbit(x)) { // -0 is written -0, as a stream does
      text.insert(0, 1, '-');
    } else if (fmt.flags & std::ios_base::showpos) {
      text.insert(0, 1, '+');
    }
    return text;
  }

  static std::string bound_to_text(double x, bool upward, const text_format& fmt)
  {
    return number_to_text(x, upward ? text_upward : text_downward, fmt);
  }

  /*
    The interval [l, r] as two bounds rounded outward, [l, r], or as [a], the
    literal of IEEE 1788-2015 for a point (12.11), for a point interval whose
    double the text writes exactly (GAOL v5). The text of the lower bound being
    at most it, and that of the upper bound at least it, two equal texts are
    the double itself, which the reader reads back as the point; the other
    point intervals are written as any other interval, [0.1, 0.1000000000000001]
    for interval(0.1), which is read back as an interval enclosing it. A zero
    is written exactly, and a point interval of zero, [-0, +0], [+0, +0] or
    [-0, -0], is written [0], the text of +0: the three are the set {0}, which
    [0] and [-0] are both read back as, and the text of interval::zero() is
    then the same whatever the build ([-0, +0] with the SSE2 intervals,
    [+0, +0] with the FPU ones), and the hexadecimal format writes it [0x0p+0].
    The bounds are compared by their bits: under denormals-are-zero,
    which a program may set, a subnormal compares equal to 0, and l == r and
    l == 0.0 would have [0, 5e-324] written [0]. (Under that mode,
    bound_to_text() took a subnormal bound for 0 as well, and wrote it
    rounded to nearest rather than outward, so that two equal texts need not
    have been the double: with 1 digit, [22u] (u = 5e-324) was written
    [1e-322], which is read as [20u, 21u]. operator<< and intervalToText()
    clear that mode before they write, where GAOL can, and number_to_text()
    writes such a bound outward where it cannot: GAOL v5, point Q of
    TODO.md.)
    fmt has the C locale, whose decimal point is '.', the reader's: operator<<
    writes the bounds format with it whatever the locale of the stream (GAOL
    v5). Under a locale writing a decimal comma, the reader took the comma of
    [0,5, 2,5] for the one between two bounds and refused the text, and would
    have read [-2,5] for interval(-2.5) as the interval [-2, 5], [12,5] as the
    empty set and [0,] (a zero in the fixed format with the showpoint flag and
    no digit) as [0, +oo]; GAOL wrote that point [-2,5, -2,5], which it
    refused. A point is still written alone between the brackets only when
    the decimal point of fmt is '.'.
    GAOL wrote every point interval <a, b>, the text of its double rounded
    downward and upward, which the reader takes for two numbers that are the
    same double only: <0.1, 0.1000000000000001> for interval(0.1) was refused,
    and textToInterval(intervalToText(interval(0.1))) was the empty set. The
    reader still takes the angles, which are no literal of the standard.
  */
  static void display_bounds(double l, double r, std::string& out, const text_format& fmt)
  {
    // A quiet comparison, which raises no invalid-operation exception on the
    // NaN bounds of the empty set (GAOL v5)
    if (!gaol_detail::quiet_less_equal(l, r)) {
      out += "[empty]";
    } else {
      std::uint64_t lbits, rbits;
      std::memcpy(&lbits, &l, sizeof lbits);
      std::memcpy(&rbits, &r, sizeof rbits);
      const bool zero = ((lbits << 1) == 0 && (rbits << 1) == 0); // +0 or -0, both
      const std::string left = bound_to_text(zero ? 0.0 : l, false, fmt);
      const std::string right = bound_to_text(zero ? 0.0 : r, true, fmt);
      out += '[';
      out += left;
      if (!((zero || (lbits == rbits && left == right)) && fmt.punct.decimal_point() == '.')) {
        out += ", ";
        out += right;
      }
      out += ']';
    }
  }

  /*
    The text of x of gaol_ieee1788::intervalToText() (gaol/parser/gaol_parser.h):
    the bounds of x as display_bounds() writes them, with the flags a new
    stream starts with, the digits of the intervals and the C locale
  */
  std::string literal_text(const interval& x)
  {
    const text_format fmt(std::ios_base::skipws | std::ios_base::dec, interval::precision(), std::locale::classic());
    std::string out;
    // The guard, rather than GAOL_RND_ENTER() and GAOL_RND_LEAVE():
    // display_bounds() allocates, and a failed allocation left the direction
    // upward, which GAOL_PRESERVE_ROUNDING promises to set back (GAOL v5)
    const rounding_guard rnd;
    round_upward_if_needed();
    display_bounds(x.left(), x.right(), out, fmt);
    return out;
  }


  /*
    A bound written in the hexadecimal-significand form of IEEE 1788-2015
    (13.4.1), which textToInterval() reads back exactly (GAOL v5).

    The recovery requirement of 13.4 asks that writing an interval and reading
    it again give the same bounds. GAOL wrote the sixteen hexadecimal digits of
    each double instead, which is no interval literal at all: the parser
    refused them, and the note of 13.4.1 gives that very form as the one that
    fails the readability test. The form is the one printf("%a") of the glibc
    writes: as many digits as the value needs, "0x1.999999999999ap-4",
    "0x0.0000000000001p-1022" for a subnormal, "0x0p+0" for zero, and the
    lexer reads it.
    It is written from the bits of the double rather than by printf("%a"),
    which follows the decimal point of the C locale: under a locale writing a
    decimal comma, it wrote "0x1,999999999999ap-4", which cannot be read
    back (GAOL v5).
  */
  static void write_hexa_bound(double x, char *buf, std::size_t n)
  {
    if (x == GAOL_INFINITY) {
      std::snprintf(buf, n, "inf");
      return;
    }
    if (x == -GAOL_INFINITY) {
      std::snprintf(buf, n, "-inf");
      return;
    }
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    const int biased = static_cast<int>((bits >> 52) & 0x7ff);
    std::uint64_t fraction = bits & ((static_cast<std::uint64_t>(1) << 52) - 1);
    // 1.f 2^(e-1023) for a normal double, 0.f 2^-1022 for a subnormal one, 0
    const int leading = (biased == 0) ? 0 : 1;
    const int exponent = (biased == 0) ? ((fraction == 0) ? 0 : -1022) : biased - 1023;
    // The 13 hexadecimal digits of the fraction, without the zeros ending them
    char digits[14];
    for (int i = 12; i >= 0; --i, fraction >>= 4) {
      digits[i] = "0123456789abcdef"[fraction & 0xf];
    }
    int nb_digits = 13;
    while (nb_digits > 0 && digits[nb_digits - 1] == '0') {
      --nb_digits;
    }
    digits[nb_digits] = '\0';
    std::snprintf(buf, n, "%s0x%d%s%sp%+d", (bits >> 63) ? "-" : "", leading,
                  (nb_digits > 0) ? "." : "", digits, exponent);
  }

  /*
    A point interval is written [a], as in the decimal format, and a point
    interval of zero [0x0p+0], whatever the signs of its bounds, the three
    being the set {0} (GAOL v5: it was written with its two bounds,
    [0x1p+2, 0x1p+2], and [-0x0p+0, 0x0p+0] for interval::zero() with the
    SSE2 intervals). The bounds are compared by their bits, which
    denormals-are-zero does not change.
  */
  std::string exact_string(const interval& I)
  {
    if (I.is_empty()) {
      return "[empty]";
    }
    const double l = I.left(), r = I.right();
    std::uint64_t lbits, rbits;
    std::memcpy(&lbits, &l, sizeof lbits);
    std::memcpy(&rbits, &r, sizeof rbits);
    char lo[64], hi[64];
    if ((lbits << 1) == 0 && (rbits << 1) == 0) {
      return "[0x0p+0]";
    }
    write_hexa_bound(l, lo, sizeof lo);
    if (lbits == rbits) {
      return std::string("[") + lo + "]";
    }
    write_hexa_bound(r, hi, sizeof hi);
    return std::string("[") + lo + ", " + hi + "]";
  }

  /*
    text into os as one item, padded to the width of os with its fill, to the
    left, to the right, or inside (std::internal). os pads a string to the left
    or to the right only: for std::internal, the fill is put where a number of
    os has it (GAOL v5).
  */
  static void write_text(ostream& os, std::string& text)
  {
    // std::internal puts the fill after the sign of a number and its 0x
    // prefix, before its digits. The text of the width and center formats
    // starts with the sign of the midpoint, if it has one, and that of the
    // agreeing digits with the sign of the bounds, and the fill goes after
    // it, as std::internal padded the midpoint alone before the interval was
    // written as a whole. The other texts start with '[', before which the
    // fill goes, as with std::right, and std::complex writes its parentheses.
    const std::streamsize width = os.width();
    if ((os.flags() & std::ios_base::adjustfield) == std::ios_base::internal
        && width > static_cast<std::streamsize>(text.size())) {
      std::size_t at = (!text.empty() && (text[0] == '-' || text[0] == '+')) ? 1 : 0;
      if (text.size() > at + 1 && text[at] == '0' && (text[at + 1] == 'x' || text[at + 1] == 'X')) {
        at += 2;
      }
      if (at > 0) {
        text.insert(at, static_cast<std::size_t>(width) - text.size(), os.fill());
      }
    }
    os << text;
  }

  /*
    The text is made in a character string, from the flags and the locale of
    os and the precision of interval::precision(), then written into os at once
    (GAOL v5): os keeps its precision, and its width (std::setw) and
    adjustment apply to the whole interval. GAOL set the precision of os to
    interval::precision() and left it so, the doubles written afterwards
    getting 16 digits, and wrote the interval piece by piece, std::setw
    padding its '[' only.

    The text is not written in a std::ostringstream of its own, whose locale
    and cached facets, built and destroyed for each interval and each bound,
    took a third of the work of writing an interval. os is only read, and is
    never given another precision or width for the time of the writing: the
    threads of a program writing to std::cout at once would race on them.
  */
  ostream& operator<<(ostream& os, const interval& I)
  {
    //    double l = ((I.left()==0.0) ? 0.0  : I.left()); // Avoids printing -0
    //    double r = ((I.right()==0.0) ? 0.0 : I.right());  // Avoids printing -0
    std::string text;

    // The guard, rather than GAOL_RND_PRESERVE() and GAOL_RND_RESTORE(): the
    // construction of the text may fail (std::bad_alloc), and a program
    // reading the exception found the direction left as this function had
    // set it, to nearest (GAOL v5). The check of the direction, as in
    // intervalToText(), clears the modes that flush the subnormals to zero,
    // which the guard sets back with GAOL_PRESERVE_ROUNDING: under
    // denormals-are-zero, a subnormal bound was 0 for the comparisons and
    // was written to nearest, [22*2^-1074] as [1e-322] with 1 digit, and the
    // C library writes it 0 (gdtoa of FreeBSD and macOS, the Debug runtime
    // of Visual C++, issue #68; GAOL v5, point Q of TODO.md)
    const rounding_guard rnd;
	round_upward_if_needed();

    double l = I.left(), r = I.right();
    const interval_format::format_t format = interval::format();

    switch (format) {
    case interval_format::bounds: { // Display in the form "[ l, r ]"
      // The bounds format, and the agreeing format where it writes bounds,
      // in the C locale, whose decimal point is the reader's (GAOL v5); the
      // width and center formats, which are for the eye, in the locale of os
      const text_format fmt(os.flags(), interval::precision(), std::locale::classic());
      display_bounds(l,r,text,fmt);
      break;
    }
    case interval_format::hexa: // The exact text representation of 13.4
      text = exact_string(I);
      break;
    case interval_format::width:  // Display in the form "c (+/- w)"
    case interval_format::center: // Display in the form "c"
      if (I.is_empty()) {
        text = "[empty]";
      } else {
        /*
          The midpoint c and the radius w of IEEE 1788-2015 (12.12.8),
          midpoint() and rad() (GAOL v5): [c-w, c+w] contains the interval, w
          being the smallest double that makes it so, and +oo for an
          unbounded interval, whose midpoint is 0 or plus or minus the
          largest double. c is written rounded to nearest, and w upward, so
          that a radius that is not 0 is never written 0. GAOL wrote (l+r)/2
          and (r-l)/2, both rounded to nearest: c and w did not contain the
          interval ([1, 1+2^-52] was "1 (+/- 1.11e-16)"), w was 0 for
          [0, 5e-324], l+r overflowed for [1e308, 1.7e308], and the center of
          a point interval was written rounded upward (0.1 was
          0.1000000000000001).
          A point interval has no radius: it is written as its center, which
          tells it from an interval too narrow for the digits of its center.
          These formats are for the eye: c has the digits of the precision, so
          that c (+/- w) written need not contain the interval, where the
          bounds format does. A radius that made the digits of c enclose it
          would show 0.1 (+/- 5.6e-18) for the point interval 0.1, and a
          radius worth the resolution of the digits of c for any interval
          narrower than that: the formats could no longer show that an
          interval is narrower than its digits (the manual, Output format).
        */
        // The center format writes no radius: midpoint() alone
        double c, w = 0.0;
        if (format == interval_format::width) {
          I.mid_rad(c, w);
        } else {
          c = I.midpoint();
        }
        // The decimal point of the locale of os, and its grouping of the
        // digits, for c and w alike; a midpoint 0 is written 0, as the bounds
        // format writes a zero, where midpoint() gives -0 for
        // [-2^-1073, 2^-1074]: its bits, which denormals-are-zero leaves alone
        const text_format shown(os.flags(), interval::precision(), os.getloc(), true);
        std::uint64_t cbits;
        std::memcpy(&cbits, &c, sizeof cbits);
        if ((cbits << 1) == 0) {
          c = 0.0;
        }
        text = number_to_text(c, text_nearest, shown);
        if (format == interval_format::width && w != 0.0) {
          // The radius is never negative, and has no sign under showpos,
          // which is for the midpoint: "+2 (+/- 1)" (GAOL v5: "+2 (+/- +1)")
          text_format radius(shown);
          radius.flags &= ~std::ios_base::showpos;
          text += " (+/- ";
          text += bound_to_text(w, true, radius);
          text += ')';
        }
      }
      break;
    case interval_format::agreeing:
      if (I.is_empty()) {
        text = "[empty]";
      } else {
        // Each bound is written with the digits of the precision and the flag
        // showpoint, in the locale of the program, as a stream of its own,
        // whose flags are the ones a stream starts with, would write it: the
        // flags and the locale of os do not apply here
        const text_format shown(std::ios_base::skipws | std::ios_base::dec | std::ios_base::showpoint,
                                interval::precision(), std::locale());
        const char point = shown.punct.decimal_point();
        const std::string lb = bound_to_text(l, false, shown), rb = bound_to_text(r, true, shown);
        std::size_t i = 0;
        while (i < lb.length() && i < rb.length() && lb[i] == rb[i]) {
          ++i;
        }
        /*
          The digits both bounds start with are written once, before the rest
          of each, only where they line up and say something: two bounds
          finite, not 0 and of the same sign, written with the same number of
          digits before the point and the same exponent, that share their
          first digit that is not 0. The other intervals are written as in
          the bounds format (GAOL v5). GAOL tested r > 10 l, true for every
          interval with a negative bound or 0, which were all written with
          their bounds, and false for [1, 10], written 1~[., 0.], the 1 of 10
          taken for the 1 of 1, or for [1, 2], written ~[1., 2.] with no
          digit shared, and for zeros, written ~[-0., 0.] for
          interval::zero() with the SSE2 intervals where the bounds format
          writes [0].
        */
        std::uint64_t lbits, rbits;
        std::memcpy(&lbits, &l, sizeof lbits);
        std::memcpy(&rbits, &r, sizeof rbits);
        const std::size_t le = lb.find_first_of("eE"), re = rb.find_first_of("eE");
        const bool line_up = (lbits << 1) != 0 && (rbits << 1) != 0
          && l > -GAOL_INFINITY && r < GAOL_INFINITY && std::signbit(l) == std::signbit(r)
          && lb.find(point) == rb.find(point)
          && ((le == std::string::npos) ? std::string() : lb.substr(le))
             == ((re == std::string::npos) ? std::string() : rb.substr(re))
          && lb.find_first_of("123456789") < i;
        if (!line_up) {
          const text_format fmt(os.flags(), interval::precision(), std::locale::classic());
          display_bounds(l,r,text,fmt);
        } else {
          // The characters both bounds start with, then what is left of each,
          // without the zeros ending it (GAOL v5: GAOL dropped from both
          // bounds the characters after the last one of the left bound that is
          // not a zero, and wrote [1.25, 1.2567] "1.25~[, ]"; zeros ending an
          // exponent are not dropped)
          text = lb.substr(0, i);
          if (i < lb.length() || i < rb.length()) {
            const std::string *bounds[2] = { &lb, &rb };
            text += "~[";
            for (int k = 0; k < 2; ++k) {
              const std::string& b = *bounds[k];
              std::size_t end = b.length();
              if (b.find_first_of("eE") == std::string::npos && b.find(point) != std::string::npos) {
                while (end > i && end > b.find(point) + 1 && b[end - 1] == '0') {
                  --end;
                }
              }
              text += (end > i) ? b.substr(i, end - i) : std::string("0");
              text += (k == 0) ? ", " : "]";
            }
          }
        }
      }
    }
    // No double is computed from here on: the text is written before the
    // guard sets the direction back
    write_text(os, text);
    return os;
  }




  /*
    inverse_upward() and sqr_upward() of gaol/gaol_interval_core.h, which are
    private to interval, for pown_upward() and nth_root(I, q) for q < 0: they
    check the rounding direction once and take the inverse or the square after
    it, without checking it again as inverse() and sqr() do (GAOL v5)
  */
  class upward_access {
  public:
    static interval inverse(const interval& I) { return interval::inverse_upward(I); }
    static interval sqr(const interval& I) { return interval::sqr_upward(I); }
  };

  /*
    The corners of x^n on [a, b], |n| >= 2 (see pown_products()): c0, where
    x^n is the least, and c1, where it is the greatest. x^n increases for an
    odd n > 0, and decreases on each side of 0 for an odd n < 0; for an even
    n, it is |x|^n, increasing in |x| for n > 0 and decreasing for n < 0. A
    corner at 0 for n < 0 is the limit of x^n on the side of 0 x lies on:
    -oo at -0, +oo at +0 (pown_point()). false where x^n has no value on x,
    which is [0] for n < 0: 0^n has none (IEEE 1788-2015, Table 9.1).
  */
  static GAOL_POWN_INLINE bool pown_corners(double a, double b, bool negative, bool odd_n, double& c0, double& c1)
  {
    if (!negative) {
      if (odd_n) {
        c0 = a;
        c1 = b;
      } else {
        c0 = (a > 0.0) ? a : ((b < 0.0) ? -b : 0.0);
        c1 = (-a > b) ? -a : b;
      }
    } else if (a > 0.0 || b < 0.0) {
      if (odd_n) {
        c0 = b;
        c1 = a;
      } else {
        c0 = (a > 0.0) ? b : -a;
        c1 = (a > 0.0) ? a : -b;
      }
    } else if (a == 0.0 && b == 0.0) {
      return false;
    } else if (odd_n) {
      c0 = (a < 0.0) ? -0.0 : b;
      c1 = (b > 0.0) ? 0.0 : a;
    } else {
      c0 = (-a > b) ? -a : b;
      c1 = 0.0;
    }
    return true;
  }

  // x^n on [a, b] for |n| >= 2, one bound at a time (pown_point())
  static interval pown_by_corners(double a, double b, int n)
  {
    double c0, c1, lo, hi, other;
    if (!pown_corners(a, b, n < 0, (n & 1) != 0, c0, c1)) {
      return interval::emptyset();
    }
    pown_point(c0, n, lo, other);
    pown_point(c1, n, other, hi);
    return interval(lo, hi);
  }

  /*
    I^n for |n| >= 2 and a nonempty I, the rounding direction being upward:
    both corners at once where they are within the range of pown_least[] and
    pown_greatest[], and pown_by_corners() otherwise (see pown_products()).
    Inlined in pown_checked() alone, the body of gaol_pown() for n != 2: its
    switch over the small exponents is long, and the callers that take
    pown_upward() after their own check call pown_large_upward().
  */
  static GAOL_POWN_INLINE_T interval pown_large_inline(const interval& I, int n)
  {
    const bool negative = n < 0;
    const unsigned int m = negative ? 0u - static_cast<unsigned int>(n) : static_cast<unsigned int>(n);
    const unsigned int L = pown_bit_length(m);
    const double a = I.left(), b = I.right();
    if (!negative || a > 0.0 || b < 0.0) {
#if GAOL_POWN_PACKED
      // (a, b), or (max(a, -b, 0), max(-a, b)) for an even m, and the
      // other way around for n < 0, with the operations of the register:
      // Clang calls std::fmax() under -frounding-math
      const __m128d ab = _mm_set_pd(b, a), nba = _mm_set_pd(-a, -b), zero = _mm_setzero_pd();
      __m128d c = ((m & 1u) != 0u) ? ab : _mm_max_sd(_mm_max_pd(ab, nba), zero);
      if (negative) {
        c = _mm_shuffle_pd(c, c, 1);
      }
      const __m128d minus_zero = _mm_set1_pd(-0.0);
      const __m128d g = _mm_andnot_pd(minus_zero, c);
      __m128d ok = _mm_and_pd(_mm_cmpge_pd(g, _mm_set1_pd(pown_least[L])), _mm_cmple_pd(g, _mm_set1_pd(pown_greatest[L])));
      if (!negative) {
        ok = _mm_or_pd(ok, _mm_cmpeq_pd(c, zero));
      }
      if (_mm_movemask_pd(ok) == 3) {
        __m128d h, l, s, K;
        pown_powers(g, m, h, l, s);
        if (negative) {
          const __m128d one = _mm_set1_pd(1.0);
          const __m128d q = _mm_div_pd(one, h);
          const __m128d r = _mm_fnmadd_pd(q, l, _mm_fnmadd_pd(q, h, one)); // (1 - q h) - q l
          const __m128d qr = _mm_mul_pd(q, r);
          l = _mm_fmadd_pd(qr, r, qr); // q (r + r^2)
          h = q;
          s = _mm_or_pd(s, r);
          K = _mm_set1_pd(pown_bound_inverse(m, L));
        } else {
          K = _mm_set1_pd(pown_bound(m, L));
        }
        // The rests compared as integers: or-ed together, they may make the
        // bits of a signaling NaN, which a comparison of doubles would take
        // for one, raising the invalid-operation exception
        K = _mm_andnot_pd(_mm_castsi128_pd(_mm_cmpeq_epi64(_mm_castpd_si128(s), _mm_setzero_si128())), K);
        // (-v0 + RU(K h - w0), v1 + RU(K h + w1)), v + w being +-(h + l), of
        // the sign of c for an odd m, the lower bound negated
        const __m128d odd = ((m & 1u) != 0u) ? minus_zero : zero;
        const __m128d sign = _mm_xor_pd(_mm_set_pd(0.0, -0.0), _mm_and_pd(c, odd));
        return interval(_mm_add_pd(_mm_xor_pd(h, sign), _mm_fmadd_pd(K, h, _mm_xor_pd(l, sign))));
      }
#else
      double c0, c1;
      (void)pown_corners(a, b, negative, (m & 1u) != 0u, c0, c1);
      const double least = pown_least[L], greatest = pown_greatest[L];
      const double g0 = std::fabs(c0), g1 = std::fabs(c1);
      if (((g0 >= least && g0 <= greatest) || (!negative && g0 == 0.0))
          && ((g1 >= least && g1 <= greatest) || (!negative && g1 == 0.0))) {
        const pown_lanes c = { g0, g1 };
        pown_lanes h, l;
        pown_rests s;
        pown_powers(c, m, h, l, s);
        double K;
        if (negative) {
          const double q0 = 1.0/h.l0, q1 = 1.0/h.l1;
          const double r0 = fma_once(-q0, l.l0, fma_once(-q0, h.l0, 1.0));
          const double r1 = fma_once(-q1, l.l1, fma_once(-q1, h.l1, 1.0));
          const double qr0 = q0*r0, qr1 = q1*r1;
          l.l0 = fma_once(qr0, r0, qr0);
          l.l1 = fma_once(qr1, r1, qr1);
          h.l0 = q0;
          h.l1 = q1;
          s.l0 |= pown_bits_of(r0);
          s.l1 |= pown_bits_of(r1);
          K = pown_bound_inverse(m, L);
        } else {
          K = pown_bound(m, L);
        }
        const double K0 = (s.l0 != 0u) ? K : 0.0, K1 = (s.l1 != 0u) ? K : 0.0;
        // the signs of c0^n and c1^n, those of c0 and c1 for an odd m
        const double t0 = ((m & 1u) != 0u && std::signbit(c0)) ? -1.0 : 1.0;
        const double t1 = ((m & 1u) != 0u && std::signbit(c1)) ? -1.0 : 1.0;
        const double down = -t0*h.l0 + fma_once(K0, h.l0, -t0*l.l0);
        const double up = t1*h.l1 + fma_once(K1, h.l1, t1*l.l1);
        return interval(-down, up);
      }
#endif
    }
    return pown_by_corners(a, b, n);
  }

#if GAOL_POWN_ER
  /*
    The packed kernel of pown_large_inline(), rounded upward by the embedded
    rounding (GAOL v5, point ZZ of TODO.md): the same operations in the same
    order, on pown_er_lanes, so that the bounds are the same bit for bit, and
    no direction of MXCSR is read or written. With GAOL_PRESERVE_ROUNDING,
    gaol_pown() saved the direction, set it upward and restored it around the
    kernel: pown(x, 3) took 19.0 ns, the direction being read and written
    through the C library and the control word of the x87 unit as well, 8.4
    ns with the writes of MXCSR alone (see gaol/gaol_fpu.h), and takes 6.2
    with this kernel, against the 5.1 of the direction left upward (an
    i7-1185G7, Clang 21): the two writes of MXCSR cost 3 to 6 ns, each one
    waiting for the operations in flight. The function gives false, res left
    alone, where the corners are not both in the range of pown_least[] and
    pown_greatest[] (pown_by_corners(), which rounds by the direction, is
    called then), and for an n < 0 with 0 in I. The bounds a and b of I are
    read after the clearing of the modes that flush the subnormals to zero
    (flush_guard), which the embedded rounding honours. res is the stored
    pair, <-lower, upper>. It repeats the block of pown_large_inline() for
    the packed kernel, on the other lanes: keep them alike. They give the
    same bounds on 8.8 million powers (tests/tools/pow/mutate.py has
    mutants of both).
  */
  static GAOL_POWN_INLINE_T bool pown_large_er(double a, double b, int n, __m128d& res)
  {
    const bool negative = n < 0;
    const unsigned int m = negative ? 0u - static_cast<unsigned int>(n) : static_cast<unsigned int>(n);
    const unsigned int L = pown_bit_length(m);
    if (negative && !(a > 0.0 || b < 0.0)) {
      return false;
    }
    const __m128d ab = _mm_set_pd(b, a), nba = _mm_set_pd(-a, -b), zero = _mm_setzero_pd();
    __m128d c = ((m & 1u) != 0u) ? ab : _mm_max_sd(_mm_max_pd(ab, nba), zero);
    if (negative) {
      c = _mm_shuffle_pd(c, c, 1);
    }
    const __m128d minus_zero = _mm_set1_pd(-0.0);
    const __m128d g = _mm_andnot_pd(minus_zero, c);
    __m128d ok = _mm_and_pd(_mm_cmpge_pd(g, _mm_set1_pd(pown_least[L])), _mm_cmple_pd(g, _mm_set1_pd(pown_greatest[L])));
    if (!negative) {
      ok = _mm_or_pd(ok, _mm_cmpeq_pd(c, zero));
    }
    if (_mm_movemask_pd(ok) != 3) {
      return false;
    }
    pown_er_lanes h, l;
    __m128d s, K;
    const pown_er_lanes gl = { g };
    pown_powers(gl, m, h, l, s);
    if (negative) {
      const pown_er_lanes one = { _mm_set1_pd(1.0) };
      const pown_er_lanes q = pown_div(one, h);
      const pown_er_lanes r = pown_fnmadd(q, l, pown_fnmadd(q, h, one)); // (1 - q h) - q l
      const pown_er_lanes qr = pown_mul(q, r);
      l = pown_fma(qr, r, qr); // q (r + r^2)
      h = q;
      pown_add_rest(s, r);
      K = _mm_set1_pd(pown_bound_inverse(m, L));
    } else {
      K = _mm_set1_pd(pown_bound(m, L));
    }
    K = _mm_andnot_pd(_mm_castsi128_pd(_mm_cmpeq_epi64(_mm_castpd_si128(s), _mm_setzero_si128())), K);
    const __m128d odd = ((m & 1u) != 0u) ? minus_zero : zero;
    const __m128d sign = _mm_xor_pd(_mm_set_pd(0.0, -0.0), _mm_and_pd(c, odd));
    const pown_er_lanes vs = { _mm_xor_pd(h.v, sign) }, ws = { _mm_xor_pd(l.v, sign) }, Kl = { K };
    res = pown_add(vs, pown_fma(Kl, h, ws)).v;
    return true;
  }
#endif

  static GAOL_POWN_TARGET interval pown_large_upward(const interval& I, int n)
  {
    return pown_large_inline(I, n);
  }

  /*
    I^n for an int n, the rounding direction being upward already: the body
    of gaol_pown() and gaol_uipow(), which pow_standard(), gaol_pow_hybrid()
    and gaol_pow_real() call after their own check (GAOL v5: they called
    gaol_pown(), which checked the direction a second time). The empty set
    is given back as it is; x^0 is 1, x^1 x, and x^-1 the inverse, correctly
    rounded, which takes x^-1 at 0 as IEEE 1788-2015 does (recip, Table 9.1)
    and as x^n for n < 0 does: [1/b, +oo] for [0, b], [-oo, +oo] for an x
    holding 0 inside, the empty set for [0]. x^2 is sqr(), without its
    check: the same bounds, and the same signs of the zero bounds, in every
    form of the integer power (GAOL v5: it made its square from exact
    products, which has the same bounds, and gave a zero bound of the other
    sign for part of the intervals holding 0, [0, 2] with the NEON intervals,
    [-1, 2] with the FPU ones). Any other n takes pown_large_upward(), with
    the tightest bounds (GAOL v5, see pown_products(): GAOL computed x^-m as
    1/x^m, whose x^m overflowed for some |x| > 1 before the inversion:
    pow([10],-400) was [0,5.6e-309] rather than [0,2^-1074], and
    pow([2],-1050) [0,2^-1024] rather than [2^-1050]).
  */
  static inline interval pown_upward(const interval& I, int n)
  {
    if (I.is_empty()) {
      return I;
    }
    switch (n) {
    case 0:
      return interval(1.0);
    case 1:
      return I;
    case -1:
      return upward_access::inverse(I);
    case 2:
      return upward_access::sqr(I);
    default:
      return pown_large_upward(I, n);
    }
  }

  /*
    x^n for an integer n beyond the ints (GAOL v5, point 74 of TODO.md):
    the bounds of c^n at the corners of pown_corners(), from CORE-MATH's pow
    (pow_lo(), pow_hi()), the tightest, where n is a double, |n| <= 2^53 and
    beyond as a double; the sign from the parity of n. An integer n of more
    than 53 bits that is no double is nh + nl, nh being n with the bits of
    |n| below its 53 highest cleared, a double, and nl the rest, below 2^11:
    c^n is c^nh c^nl, rounded outward, within three doubles of the tightest
    bounds.
    gaol::pow(x, n) gave [-oo, +oo] beyond the ints and the unsigned ints,
    whereas x^n is finite and not trivial for x near 1: pow([1 + 2^-30],
    2^33), about e^8, is [2980.9579759367944, 2980.9579759367948]. Beyond
    2^53, only a base within 2^-43 of 1 or -1 has a power that is neither
    below the least subnormal nor above the largest double, the others being
    rounded to [0, 2^-1074] or [DBL_MAX, +oo], with their signs.
  */
  static void pown_point_beyond(double c, double nh, double nl, bool odd_n, double& lo, double& hi)
  {
    const double g = std::fabs(c);
    if (g == 0.0 || g == GAOL_INFINITY) {
      lo = hi = ((g == 0.0) == (nh < 0.0)) ? GAOL_INFINITY : 0.0;
    } else {
      lo = pow_lo(g, nh);
      hi = pow_hi(g, nh);
      if (nl != 0.0) {
        // c^nh and c^nl are on the same side of 1: the product is no 0 oo
        lo = -((-lo)*pow_lo(g, nl));
        hi = hi*pow_hi(g, nl);
      }
    }
    if (std::signbit(c) && odd_n) {
      const double t = lo;
      lo = -hi;
      hi = -t;
    }
  }

  static interval pown_beyond_upward(const interval& I, double nh, double nl, bool odd_n)
  {
    double c0, c1, lo, hi, other;
    if (I.is_empty()) {
      return I;
    }
    if (!pown_corners(I.left(), I.right(), nh < 0.0, odd_n, c0, c1)) {
      return interval::emptyset();
    }
    pown_point_beyond(c0, nh, nl, odd_n, lo, other);
    pown_point_beyond(c1, nh, nl, odd_n, other, hi);
    return interval(lo, hi);
  }

  // A degenerate exponent [d], d an integer beyond the ints: odd only below
  // 2^53, where the doubles are all even
  static interval pown_double_upward(const interval& I, double d)
  {
    const bool odd_d = std::fabs(d) < 9007199254740992.0 && std::fmod(d, 2.0) != 0.0;
    return pown_beyond_upward(I, d, 0.0, odd_d);
  }

  /*
    One check of the rounding direction for each call (GAOL v5). The body of
    pown_large_upward() is inlined here, the function of gaol::pow(x, n) and
    of gaol_ieee1788::pown(x, n) for n != 2 (see gaol_pown() below): its call
    took 0.4 to 0.6 ns more for x^3 to x^1000 (5.9 ns rather than 5.5 for x^3,
    with Clang 21 on an i7-1185G7).
  */
  static GAOL_POWN_TARGET GAOL_POWN_NOINLINE interval pown_checked(const interval& I, int n)
  {
#if GAOL_POWN_ER
    // The embedded rounding sets no direction: the modes that flush the
    // subnormals to zero are cleared for the kernel (flush_guard), the bounds
    // read after, and the result made before the modes are set back
    if ((n < -1 || n > 1) && !I.is_empty()) {
      const flush_guard flush;
      __m128d stored;
      if (pown_large_er(flush_guard::in(I.left()), flush_guard::in(I.right()), n, stored)) {
        return interval(flush_guard::out(stored));
      }
    }
#endif
    GAOL_RND_ENTER();
    interval res = (I.is_empty() || (n >= -1 && n <= 1)) ? pown_upward(I, n) : pown_large_inline(I, n);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    I^n for an int n. n = 2 is sqr(), which makes its own check of the
    rounding direction, and is taken before any other (GAOL v5): the integer
    power took four times the time of sqr() for the same bounds (doc/simd.md),
    and the sign of a zero bound is now that of sqr(). The other forms of the
    integer power take pown_upward() after their own check, which has
    sqr_upward() for n = 2. The rest is a function of its own, which is not
    inlined here: with it, the frame that its kernel needs was set up before
    the test of n = 2, which took 3.6 ns for x^2 against 1.7 (an Apple M2,
    NEON intervals).
  */
  interval gaol_pown(const interval& I, int n)
  {
    if (n == 2) {
      return sqr(I);
    }
    return pown_checked(I, n);
  }

  // gaol_pown() within the ints, CORE-MATH's pow beyond them (GAOL v5)
  interval gaol_uipow(const interval& I, unsigned int e)
  {
    if (e <= static_cast<unsigned int>(INT_MAX)) {
      return gaol_pown(I, static_cast<int>(e));
    }
    GAOL_RND_ENTER();
    interval res = pown_beyond_upward(I, static_cast<double>(e), 0.0, (e & 1u) != 0u);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval gaol_pown_ll(const interval& I, long long n)
  {
    // The empty set returns before the check, as sqr() does (GAOL v5): the
    // powers of an n beyond the unsigned ints come here, those of the others
    // go to gaol_pown() and gaol_uipow()
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res;
    if (n >= INT_MIN && n <= INT_MAX) {
      res = pown_upward(I, static_cast<int>(n));
    } else {
      // |n| without a signed negation, which overflows at LLONG_MIN
      const unsigned long long magnitude = (n < 0) ? 0ull - static_cast<unsigned long long>(n) : static_cast<unsigned long long>(n);
      // high: |n| with the bits below its 53 highest cleared, a double, and
      // nl = 0 where n is one, 2^53 + 2 included
      int dropped = 0;
      for (unsigned long long v = magnitude >> 53; v != 0u; v >>= 1) {
        ++dropped;
      }
      const unsigned long long high = (magnitude >> dropped) << dropped;
      const double nh = static_cast<double>(high), nl = static_cast<double>(magnitude - high); // exact
      res = pown_beyond_upward(I, (n < 0) ? -nh : nh, (n < 0) ? -nl : nl, (magnitude & 1u) != 0u);
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    [+oo] and [-oo] contain no real number: numsToInterval(l, u) of IEEE
    1788-2015 has no value for l = +oo or u = -oo (10.5.8). The constructors
    and the operations make none, but interval(const __m128d&) and its NEON
    counterpart do. pow gives the empty set for one as an exponent, and
    pow_rel() and pow_exponent_rel() for one as the power, or as the exponent
    or the base (GAOL v5: they took it for the limit of x^y). The caller has
    tested for the empty set, whose NaN bounds this would compare with the
    signaling comparisons
  */
  static inline bool holds_no_real(const interval& a)
  {
    return !(a.left() < GAOL_INFINITY && a.right() > -GAOL_INFINITY);
  }

  /*
    The pow of IEEE 1788-2015 (Table 9.1) for an interval exponent, the
    rounding direction being upward (GAOL v5): x^y for the x of x in [0, +oo]
    and the y of y, 0^y being defined for y > 0 only. The base is cut to
    [0, +oo]; [0] gives [0] or the empty set; a degenerate integer exponent
    within the ints takes the integer power, x^n on x >= 0, where pow and
    pown agree; any other box takes the powers at its corners.
    gaol_ieee1788::pow(x, y) is this function after the check of
    pow_standard(), and gaol_pow_hybrid() calls it for every exponent but a
    degenerate integer, for which it takes pown on the whole of x.
  */
  static interval pow_standard_upward(const interval& x, const interval& y)
  {
    // An exponent [+oo] or [-oo] contains no real number, and gives the empty
    // set, as the empty one, as in pow_hybrid_upward() (GAOL v5:
    // gaol_ieee1788::pow([0.5], [+oo]) was [0])
    if (x.is_empty() || y.is_empty() || holds_no_real(y)) {
      return interval::emptyset();
    }

    /*
      x^y is only real for a negative x when y is an integer (GAOL v5,
      ported from the fix of Codac, commit 74086ccb, Jordan Ninin). GAOL
      computed the powers of the negative part of I on its magnitude, and
      pow([-4,-1],[0.5,0.5]) returned [-1,2] where sqrt([-4,-1]) is empty.
    */
    const interval base = x & interval::positive();
    if (base.is_empty()) {
      return interval::emptyset();
    }
    // pow(0,y) is 0 for y > 0, and has no value for y <= 0 (Table 9.1, footnote
    // c), which the limits at the corners do not give. interval(0.0) rather
    // than interval::zero(), whose lower bound is -0 with SSE2 intervals
    if (base.right() == 0.0) {
      return y.right() > 0.0 ? interval(0.0) : interval::emptyset();
    }

    /*
      A degenerate integer exponent [n] within the ints: x^n on x >= 0, where
      pow and pown agree, pown giving the powers that are doubles exactly, as
      pow_is_double() does at the corners below. Beyond the ints, n >= 2^31 or
      n < -2^31, [n] takes the corners below, as pown does there, one
      for each bound, x^n being monotone in x (GAOL v5: a block of its own
      took them, with the bounds the corners give since they take the
      limits): 1 at x = 1, the limit 0 or +oo at x = 0 (+0 or -0) and +oo,
      with no call of CORE-MATH, whose pow(+0, n) raised the divide-by-zero
      exception for n < 0 and pow(-0, n) is -oo for an odd n < 0. x^n is a
      double at 0 and 1 only: for another x it is beyond the doubles (2^(k n),
      for a power of two 2^k, k != 0) or not dyadic (1/m^|n| for n < 0) or
      above 2^53 (m^n, m >= 3 odd), so that the double below CORE-MATH's
      value is the tightest lower bound (pow_is_double() tells no such power
      a double). gaol_pow_hybrid() takes [n] before it comes here: pown on the
      whole of x (pown_double_upward() beyond the ints).
    */
    if (y.is_an_int()) {
      return pown_upward(base, static_cast<int>(y.left()));
    }

    /*
      The corners of the box (GAOL v5, issue #8): x^y increases with y for
      x > 1 and decreases for x < 1, increases with x for y > 0 and decreases
      for y < 0, so that its extrema over I x J are at corners, which the
      places of the bounds about 1 and 0 give: one corner for each bound, two
      where I holds 1 inside and J holds 0 inside. At a finite corner with
      x > 0, CORE-MATH's pow is correctly rounded in the upward rounding GAOL
      computes in, which gives the upper bound, and the double below it the
      lower one, unless the power is a double, which pow_is_double() proves
      and which is then the lower bound itself: the bounds are the tightest
      ones, where exp(J*log(I)) multiplied the relative width of log(I) by
      |y log(x)|: pow([2], [1023.5]) was 1425 doubles below and 748 above,
      and pow([4], 0.5) [2 - 2^-52, 2]. At a corner where x is 0 or +oo or y infinite, the
      bound is the limit of x^y there, 0 or +oo, or 1 where x = 1 or y = 0
      (pow_lo(), pow_hi()):
      pow([4, +oo], 0.5) is [2, +oo] and pow([0, 2^-1000], [-oo, -1])
      [2^1000, +oo], where exp(J*log(I)), which these boxes took, gave
      [2 - 2^-52, +oo] and a lower bound 617 doubles below 2^1000. The limits
      at x = 0 are those of the x > 0 of the domain where y <= 0, and 0^y = 0
      itself where y > 0.
    */
    const double xl = base.left(), xu = base.right(), yl = y.left(), yu = y.right();
    double l, r;
    if (yl == yu) {
      // A degenerate exponent, which [0] and the other integers within the
      // ints are not here: x^y increases with x for y > 0 and decreases for
      // y < 0, one corner for each bound whatever the base (GAOL v5:
      // gaol::pow(x, 0.75) took 109 ns rather than 102, and
      // gaol_ieee1788::pow(x, [1e10]) 72 rather than 68, the place of the
      // base about 1 being tested, with Clang 18 on an i7-1185G7)
      l = pow_lo((yl > 0.0) ? xl : xu, yl);
      r = pow_hi((yl > 0.0) ? xu : xl, yl);
    } else if (xl == 0.0 && yl > 0.0) {
      // A base from 0 and exponents above 0: 0^y = 0 is the least power, and
      // the greatest is at xu, with yu for xu >= 1 and yl below, one call of
      // CORE-MATH (GAOL v5: through the corners of a base holding 1, 48 ns
      // rather than 39 for gaol::pow([0, b], J), b from 1 to 10 and J within
      // [0.5, 2.5], with Clang 18 on an i7-1185G7)
      l = 0.0;
      r = pow_hi(xu, (xu >= 1.0) ? yu : yl);
    } else if (xl >= 1.0) {
      l = pow_lo((yl >= 0.0) ? xl : xu, yl);
      r = pow_hi((yu >= 0.0) ? xu : xl, yu);
    } else if (xu <= 1.0) {
      l = pow_lo((yu >= 0.0) ? xl : xu, yu);
      r = pow_hi((yl >= 0.0) ? xu : xl, yl);
    } else {
      // I holds 1 inside, where x^y is 1: x^y is at most 1 at (xl, y >= 0)
      // and (xu, y <= 0), the least at (xl, yu) or (xu, yl), and at least 1
      // at (xl, y <= 0) and (xu, y >= 0), the greatest at (xl, yl) or
      // (xu, yu). An exponent of the other sign is taken as 0, for which
      // pow_lo() and pow_hi() give 1 with no call of CORE-MATH: one call for
      // each bound where J has one sign, as for the other bases, two where J
      // holds 0 inside (GAOL v5: four; gaol::pow([1/b, b], J), b from 1 to 10
      // and J within [0.5, 2.5], took 134 ns, 137 with GAOL 4, and takes 84,
      // with Clang 18 on an i7-1185G7). Where J holds 0 inside and one of the
      // two corners is a limit, the other is not computed either: the limit
      // is 0 at (xl, yu) and (xu, yl), where x^y is below 1, which bounds the
      // box below, and +oo at (xl, yl) and (xu, yu), which bounds it above,
      // and pow([0, 2], [-1, 1]) takes no call (tested where J holds 0 inside
      // only: 90 ns rather than 84 for the boxes above, 81 without the test)
      const double yu_or_0 = (yu > 0.0) ? yu : 0.0, yl_or_0 = (yl < 0.0) ? yl : 0.0;
      const bool around_0 = yl < 0.0 && yu > 0.0;
      l = (around_0 && (pow_is_limit(xl, yu) || pow_is_limit(xu, yl)))
          ? 0.0 : minimum(pow_lo(xl, yu_or_0), pow_lo(xu, yl_or_0));
      r = (around_0 && (pow_is_limit(xl, yl) || pow_is_limit(xu, yu)))
          ? GAOL_INFINITY : maximum(pow_hi(xl, yl_or_0), pow_hi(xu, yu_or_0));
    }
    return interval(l,r);
  }

  /*
    pow_standard_upward() after one check of the rounding direction, made
    before the bounds are compared: with denormals-are-zero, the lower bound
    -5 2^-1074 of x stayed in x & [0, +oo], and pow([-5 2^-1074, 1], [0.5])
    was empty once the check of the corners had cleared the mode (GAOL v5, see
    gaol/gaol_fpu.h). Its integer powers and its corners do not check the
    direction again (pown_upward(), pow_lo(), pow_hi()).
  */
  interval pow_standard(const interval& x, const interval& y)
  {
    // An empty operand returns before the check, as sqr() does (GAOL v5)
    if (x.is_empty() || y.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = pow_standard_upward(x, y);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // gaol_pow_hybrid() after its check, made before the bounds of J are
  // compared: with denormals-are-zero, [2^-1074, 2^-1073] was the degenerate
  // integer exponent [0] (GAOL v5, see gaol/gaol_fpu.h)
  static interval pow_hybrid_upward(const interval &I, const interval &J)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    // [+oo] and [-oo] contain no real number: numsToInterval(l,u) of IEEE 1788
    // has no value for l = +oo or u = -oo (10.5.8). The constructors and the
    // operations make none, but interval(const __m128d&) and get_xmminterval()
    // of the SSE2 intervals do, as in tan() (GAOL v5: kept, TODO.md B.1 having
    // taken the test for one that is never true)
    if (J.left() == J.right() && !(std::fabs(J.left()) <= (std::numeric_limits<double>::max)())) {
      return interval::emptyset();
    }

    /*
      Hybrid semantics (GAOL v5): a degenerate integer exponent always
      takes the integer power, pown of IEEE 1788, which is defined for a
      negative base too, is 1 at p = 0 for any x, 0 included, and has no value
      at x = 0 for p < 0 (Table 9.1, footnote b), and any other exponent the
      pow of IEEE 1788, defined for x > 0, and for x = 0 when y > 0.
    */
    if (J.left() == J.right() && std::floor(J.left()) == J.left()) {
      if (J.is_an_int()) {
        return pown_upward(I,int(J.left()));
      }
      // An integer beyond the ints: the pown of CORE-MATH's pow (GAOL v5,
      // point 74 of TODO.md: [-oo, +oo] before)
      return pown_double_upward(I, J.left());
    }
    return pow_standard_upward(I, J);
  }

  interval gaol_pow_hybrid(const interval &I, const interval &J)
  {
    // An empty operand returns before the check, as sqr() does (GAOL v5)
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = pow_hybrid_upward(I, J);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*!
    \brief I^p for a floating-point p, gaol::pow(I, p) (GAOL v5)

    Without it, pow(I,2.5) called pow(const interval&, int), converting a
    double to an int being a standard conversion and converting it to an
    interval a user-defined one: the exponent was truncated, and pow([4],0.5)
    returned [1]. An integer p within the ints is computed by gaol_pown(),
    which is defined for negative bases too, any other p by
    gaol_pow_hybrid(), and an infinite or NaN p gives the empty set. Ported
    from the fix of Codac (commit 74086ccb, Jordan Ninin).
  */
  interval  gaol_pow_real(const interval& I, double p)
  {
    // Infinite or NaN, told by a quiet comparison: <= raised the
    // invalid-operation exception for a NaN p (GAOL v5). The empty set
    // returns before the check too, as sqr() does (GAOL v5)
    if (I.is_empty() || !gaol_detail::quiet_less_equal(std::fabs(p), (std::numeric_limits<double>::max)())) {
      return interval::emptyset();
    }
    // p compared after the check: with denormals-are-zero, the floor of a
    // subnormal p was p, and pow(I, p) the power [1] (GAOL v5, see
    // gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    p = rnd_reread(p);
    interval res;
    if (std::floor(p) == p && p >= (std::numeric_limits<int>::min)() && p <= (std::numeric_limits<int>::max)()) {
      res = pown_upward(I, static_cast<int>(p));
    } else {
      res = pow_hybrid_upward(I, interval(p));
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The reverse functions of pow (GAOL v5): pow_rel(K, J, I), the hull of the
    x of I with an x^y in K for a y of J, and pow_exponent_rel(K, H, I), the
    hull of the y of I with an x^y in K for an x of H, which are powRev1(J, K,
    I) and powRev2(H, K, I) of IEEE 1788-2015 (10.5.4, Table 10.1).

    They reverse the pow of the standard (Table 9.1), that of
    gaol_ieee1788::pow and pow_standard(): x^y = e^(y log x) for x > 0, 0^y = 0
    for y > 0, and no value elsewhere. gaol::pow takes the integer power for a
    degenerate integer exponent [n], which has negative bases too: an
    interval function that is not monotone in its exponent
    (gaol_pow_hybrid()), and has no set of solutions to reverse; the reverse
    of x^n is nth_root_rel(K, n, I).

    The solutions are those of x = 0 (0^y = 0 in K for y > 0) and those of
    x > 0, where x^y is in K, that is in K & ]0, +oo], exactly when y log(x) is
    in log(K & ]0, +oo]): the reverse of a product, of y and log(x). Where 1
    is in K and the other argument holds the value that makes the product 0
    (y = 0, x^0 = 1; x = 1, 1^y = 1), every x > 0 of I, or every y of I, is a
    solution. Otherwise, each sign of the other argument (y > 0 and y < 0;
    x > 1 and x < 1) gives a piece over which x^y is monotone in both x and
    y: the solutions of each piece are an interval, whose ends are reached
    at the bounds of the other argument and of K, or are limits there, and
    pieces of each sign are intersected with I, then joined.

    The ends of the pieces are c^(1/y) for pow_rel and log(c)/log(x) for
    pow_exponent_rel, c a bound of K and x or y a bound of the other argument;
    1 for pow_rel and 0 for pow_exponent_rel where c is 1 (exactly reached);
    0 and the infinities; and 1 (x^y -> 1 for y -> +oo or -oo) for pow_rel and
    0 (log(c)/log(x) -> 0 for x -> 0 or +oo) for pow_exponent_rel, as limits:
    a piece that ends at such a limit does not hold it, and a bound of I equal
    to it leaves that piece out (pow_rel([2, 3], [1, +oo], [0, 1]) is empty:
    x^y >= 2 for a y >= 1 asks x > 1).

    c^(1/y) and log(c)/log(x) are irrational but where they are doubles, and
    their bounds are proved with the power they reverse: d <= c^(1/y) exactly
    when d^y <= c (y > 0), which holds exactly when d^y rounded upward,
    CORE-MATH's pow in the upward rounding GAOL computes in, is at most c, c
    being a double; d^y >= c exactly when d^y rounded downward is at least c,
    which pow_lo() gives, taking the power itself where it is a double
    (pow_is_double()). y < 0, which makes d^y decrease, swaps the two, and
    log(c)/log(x) is bounded the same way by the powers x^e of a double e. The
    lower bound is the largest double proved to be below the end, the upper
    bound the smallest one proved to be above it: they are looked for by
    steps that double then by bisection, from an approximation, on the
    doubles taken in their order (double_key()), each step costing one pow.
    The bounds are the tightest: a power rounded upward is CORE-MATH's,
    correctly rounded, and pow_is_double() tells every power that is a double,
    which only makes the bounds tighter, its answer that a power is no double
    never being used to leave a point out.

    A piece is left out only where it is proved to miss I: its lower end
    proved above the upper bound u of I, or its upper end proved below the
    lower bound. Where the bound of the end is u itself, c^(1/y) > u is
    proved by u^y < c (y > 0), u^y rounded upward being below c, which holds
    unless u^y is within one double below c: there, telling u^y < c from
    u^y = c asks whether u^y is a double, which only pow_is_double() tells,
    and the point u is kept rather than left out on that answer:
    pow_rel([1 + 2^-52], [0.5], [0, 1 + 2^-51]) is [1 + 2^-51] rather than
    empty, (1 + 2^-52)^2 being 1 + 2^-51 + 2^-104. The result is then the
    tightest one for I widened by one double, which accurate (12.10.1)
    allows.
  */

  /*
    The doubles in their order, as unsigned integers: the key of d >= 0 is
    2^63 plus the bits of d, and that of d < 0 is 2^63 minus the bits of -d,
    so that the keys of -oo, -DBL_MAX, ..., -0 = +0, ..., DBL_MAX, +oo
    increase with the doubles, consecutive doubles having consecutive keys,
    from 2^52 to 2^64 - 2^52. The searches below step and bisect on the keys,
    exactly, without nextafter() nor arithmetic on the doubles.
  */
  static inline std::uint64_t double_key(double d)
  {
    const double a = std::fabs(d);
    std::uint64_t bits;
    std::memcpy(&bits, &a, sizeof bits);
    const std::uint64_t half = static_cast<std::uint64_t>(1) << 63;
    return (d < 0.0) ? half - bits : half + bits;
  }

  static inline double key_double(std::uint64_t k)
  {
    const std::uint64_t half = static_cast<std::uint64_t>(1) << 63;
    const std::uint64_t bits = (k >= half) ? k - half : half - k;
    double a;
    std::memcpy(&a, &bits, sizeof a);
    return (k >= half) ? a : -a;
  }

  /*
    The largest key of [a, b] at which holds() is true, holds() being true at
    a, where it is not called, and false from some key on: from the key k of
    an approximation, by steps that double, up while it holds and down while
    it does not, then by bisection between the last key where it holds and the
    first one where it does not. At most about 2 x 64 calls of holds(), and two
    from a k next to the result.
  */
  template <class Holds>
  static std::uint64_t last_key_holding(const Holds& holds, std::uint64_t a, std::uint64_t b, std::uint64_t k)
  {
    const std::uint64_t largest_step = static_cast<std::uint64_t>(1) << 62;
    k = (k < a) ? a : ((k > b) ? b : k);
    // holds(yes), and !holds(no) once failed is true
    std::uint64_t yes = a, no = b;
    bool failed = false;
    if (k != a) {
      if (holds(k)) {
        yes = k;
      } else {
        no = k;
        failed = true;
      }
    }
    if (!failed) {
      for (std::uint64_t step = 1;; step = (step < largest_step) ? 2*step : step) {
        if (b - yes <= step) {
          if (yes == b || holds(b)) {
            return b;
          }
          no = b;
          break;
        }
        if (holds(yes + step)) {
          yes += step;
        } else {
          no = yes + step;
          break;
        }
      }
    } else {
      for (std::uint64_t step = 1; no - a > step; step = (step < largest_step) ? 2*step : step) {
        if (holds(no - step)) {
          yes = no - step;
          break;
        }
        no -= step;
      }
    }
    while (no - yes > 1) {
      const std::uint64_t m = yes + (no - yes)/2;
      if (holds(m)) {
        yes = m;
      } else {
        no = m;
      }
    }
    return yes;
  }

  // The smallest key of [a, b] at which holds() is true, holds() being true at
  // b, where it is not called, and false up to some key: the search above on
  // the keys taken backward
  template <class Holds>
  static std::uint64_t first_key_holding(const Holds& holds, std::uint64_t a, std::uint64_t b, std::uint64_t k)
  {
    // static, so that the lambda needs no capture of it: Clang warned of the
    // capture as not required (-Wunused-lambda-capture), and Visual C++
    // refused the constant uncaptured (C3493)
    static constexpr std::uint64_t last = ~static_cast<std::uint64_t>(0);
    return last - last_key_holding([&holds](std::uint64_t j) { return holds(last - j); }, last - b, last - a, last - k);
  }

  /*
    An approximation of c^(1/y), c > 0 finite and other than 1 and y finite
    and other than 0, which the bounds below are looked for from: CORE-MATH's
    pow with the exponent 1/y rounded, |log(c^(1/y))| 2^-53 away relatively,
    after a step of Newton's method on r^y = c, r - r (r^y - c)/(y r^y),
    which brings it within about 2^-52/|y| relatively. pow_rounded_up() takes
    the exponents of extreme magnitude apart, 1/y included (+oo for a
    subnormal y), on which CORE-MATH's pow would raise the invalid-operation
    exception. Rounding upward
  */
  static double root_of_power_near(double c, double y)
  {
    const double largest = std::numeric_limits<double>::max();
    // 1/y overflows for a y next to 2^-1074, to +oo (-DBL_MAX for y < 0): the
    // quotient_sat() that raises no exception (GAOL v5, point 79 of TODO.md)
    const double r = pow_rounded_up<true>(c, quotient_sat(1.0, y));
    // The step is for |y| up to 1024 and c up to 2^1000: r is then within
    // about 2^-43 relatively of the root, so that r^y is within 2^-33 of c, and
    // r^y cannot overflow. For a greater |y| the rounded 1/y moves r by less
    // than 745/|y| 2^-53 relatively, below one double, and a step would take
    // r^y, which overflows for |y| next to 2^52 (GAOL v5, point 79 of
    // TODO.md: pow_rel([3], [2^512], I) raised the overflow exception)
    if (r > 0.0 && r <= largest && std::fabs(y) <= 1024.0 && c <= huge_root_argument) {
      const double p = pow_rounded_up<true>(r, y);
      if (p != c && p >= std::numeric_limits<double>::min() && p <= largest) {
        // r (p - c)/(y p), as (p - c)/p then /y, whose quotients saturate,
        // where y p and the quotient of (p - c) by it overflowed (GAOL v5,
        // point 79 of TODO.md), and which is left out where it is 1 or more
        // in magnitude, the power being then no approximation of c
        const double t = quotient_sat(quotient_sat(p - c, p), y);
        if (std::fabs(t) < 1.0) {
          const double s = r - r*t;
          if (s > 0.0 && s <= largest) {
            return s;
          }
        }
      }
    }
    return r;
  }

  /*
    The tightest bounds of c^(1/y), the x > 0 with x^y = c, for c > 0 finite
    and other than 1, and y finite and other than 0: d <= c^(1/y) is proved
    by d^y <= c for y > 0 and by d^y >= c for y < 0, and d >= c^(1/y) the
    other way round. 0 is below c^(1/y), where pow() is not called (CORE-MATH
    gives +oo for 0^y, y < 0, with the divide-by-zero exception), and +oo
    above it: [DBL_MAX, +oo] for a c^(1/y) beyond the doubles, and
    [0, 2^-1074] for one below them. Rounding upward
  */
  static double root_of_power_dn(double c, double y, double near)
  {
    const auto below = [c, y](std::uint64_t k) {
      const double d = key_double(k);
      return (y > 0.0) ? pow_hi<true>(d, y) <= c : pow_lo<true>(d, y) >= c;
    };
    return key_double(last_key_holding(below, double_key(0.0), double_key(std::numeric_limits<double>::max()),
                                       double_key(near)));
  }

  static double root_of_power_up(double c, double y, double near)
  {
    const auto above = [c, y](std::uint64_t k) {
      const double d = key_double(k);
      return (y > 0.0) ? pow_lo<true>(d, y) >= c : pow_hi<true>(d, y) <= c;
    };
    return key_double(first_key_holding(above, double_key(std::numeric_limits<double>::denorm_min()),
                                        double_key(GAOL_INFINITY), double_key(near)));
  }

  /*
    The tightest bounds of log(c)/log(x), the y with x^y = c, for c > 0
    finite and other than 1, and x > 0 finite and other than 1: e <=
    log(c)/log(x) is proved by x^e <= c for x > 1 and by x^e >= c for x < 1,
    and e >= log(c)/log(x) the other way round. The value is finite, below
    2^63 in magnitude (|log(c)| < 745 and |log(x)| > 2^-53), and the
    approximation log(c)/log(x) of CORE-MATH's logarithms is within a few
    doubles of it. Rounding upward
  */
  static double log_ratio_near(double c, double x)
  {
    return upward::log_up(c)/upward::log_up(x);
  }

  static double log_ratio_dn(double c, double x)
  {
    const auto below = [c, x](std::uint64_t k) {
      const double e = key_double(k);
      return (x > 1.0) ? pow_hi<true>(x, e) <= c : pow_lo<true>(x, e) >= c;
    };
    return key_double(last_key_holding(below, double_key(-GAOL_INFINITY),
                                       double_key(std::numeric_limits<double>::max()),
                                       double_key(log_ratio_near(c, x))));
  }

  static double log_ratio_up(double c, double x)
  {
    const auto above = [c, x](std::uint64_t k) {
      const double e = key_double(k);
      return (x > 1.0) ? pow_lo<true>(x, e) >= c : pow_hi<true>(x, e) <= c;
    };
    return key_double(first_key_holding(above, double_key(-std::numeric_limits<double>::max()),
                                        double_key(GAOL_INFINITY), double_key(log_ratio_near(c, x))));
  }

  /*
    Whether c^(1/y) > d (above) or c^(1/y) < d (below) is proved, for a
    double d > 0: d^y < c, or d^y > c, for y > 0, the other way round for
    y < 0. A power rounded upward below c, or one rounded downward above it,
    proves it without telling whether a power is a double. Rounding upward
  */
  static bool root_of_power_beyond(double c, double y, double d, bool above)
  {
    return (above == (y > 0.0)) ? pow_hi<true>(d, y) < c : pow_lo<true>(d, y) > c;
  }

  // The same for log(c)/log(x) and a double e: x^e < c, or x^e > c, for x > 1
  static bool log_ratio_beyond(double c, double x, double e, bool above)
  {
    return (above == (x > 1.0)) ? pow_hi<true>(x, e) < c : pow_lo<true>(x, e) > c;
  }

  /*
    An end of a piece of the solutions: for a lower end, a double at most the
    end, and for an upper end, a double at least it; whether the piece only
    comes as close to the end as one likes, without holding it (the limits
    0, 1 and the infinities); and the end, c^(1/w) or log(c)/log(w), where
    its bound is computed
  */
  enum pow_rel_value { pow_rel_exact, pow_rel_root, pow_rel_log_ratio };

  struct pow_rel_end {
    double bound;
    bool limit;
    pow_rel_value value;
    double c, w;
  };

  static inline pow_rel_end reached(double d)
  {
    pow_rel_end e = { d, false, pow_rel_exact, 0.0, 0.0 };
    return e;
  }

  static inline pow_rel_end approached(double d)
  {
    pow_rel_end e = { d, true, pow_rel_exact, 0.0, 0.0 };
    return e;
  }

  // c^(1/y), below it for a lower end, above it for an upper one
  static pow_rel_end root_end(double c, double y, bool lower)
  {
    const double near = root_of_power_near(c, y);
    pow_rel_end e = { lower ? root_of_power_dn(c, y, near) : root_of_power_up(c, y, near), false, pow_rel_root, c, y };
    return e;
  }

  // log(c)/log(x), below it for a lower end, above it for an upper one
  static pow_rel_end log_ratio_end(double c, double x, bool lower)
  {
    pow_rel_end e = { lower ? log_ratio_dn(c, x) : log_ratio_up(c, x), false, pow_rel_log_ratio, c, x };
    return e;
  }

  // Whether the end is proved to be above d (above) or below it
  static bool pow_rel_beyond(const pow_rel_end& e, double d, bool above)
  {
    switch (e.value) {
    case pow_rel_root:
      return root_of_power_beyond(e.c, e.w, d, above);
    case pow_rel_log_ratio:
      return log_ratio_beyond(e.c, e.w, d, above);
    default:
      return false;
    }
  }

  /*
    Adds to the hull [lo, hi] the part of [l, u] in the piece from the lower
    end to the upper end, unless the piece is proved to miss [l, u]: an end
    beyond l or u, or a limit at it, or a computed end whose bound is l or u
    and which is proved to be beyond it all the same. Where the power of l or
    u is within one double of the bound of K, that proof would need to know
    whether the power is a double, and the point is kept
  */
  static void add_pow_rel_piece(const pow_rel_end& lower, const pow_rel_end& upper, double l, double u,
                                double& lo, double& hi)
  {
    if (lower.bound > u || (lower.limit && lower.bound >= u) || (lower.bound == u && pow_rel_beyond(lower, u, true))
        || upper.bound < l || (upper.limit && upper.bound <= l) || (upper.bound == l && pow_rel_beyond(upper, l, false))) {
      return;
    }
    lo = minimum(lo, maximum(lower.bound, l));
    hi = maximum(hi, minimum(upper.bound, u));
  }

  /*
    pow_rel() after its check, made before the bounds are compared (GAOL v5,
    see gaol/gaol_fpu.h)
  */
  static interval pow_rel_upward(const interval& K, const interval& J, const interval& I)
  {
    // A power or an exponent [+oo] or [-oo] holds no real number: no y and no
    // x (GAOL v5: pow_rel([2], [+oo], I) was [1, 1 + 2^-52] and
    // pow_rel([+oo], [2], I) [DBL_MAX, +oo], limits of x^y). The x to find, I,
    // needs no test: interval(lo, hi) of the result is empty for lo = +oo or
    // hi = -oo
    if (K.is_empty() || J.is_empty() || I.is_empty() || holds_no_real(K) || holds_no_real(J)) {
      return interval::emptyset();
    }
    const double xu = I.right(), cu = K.right();
    if (xu < 0.0 || cu < 0.0) {
      return interval::emptyset(); // No x >= 0, or no power
    }
    const double xl = (I.left() > 0.0) ? I.left() : 0.0, cl = K.left(), yl = J.left(), yu = J.right();
    double lo = GAOL_INFINITY, hi = -GAOL_INFINITY;
    if (xl == 0.0 && cl <= 0.0 && yu > 0.0) {
      lo = hi = 0.0; // 0^y = 0 for y > 0
    }
    if (xu > 0.0 && cu > 0.0) {
      if (cl <= 1.0 && cu >= 1.0 && yl <= 0.0 && yu >= 0.0) {
        // x^0 = 1 for every x > 0
        lo = minimum(lo, xl);
        hi = xu;
      } else {
        const pow_rel_end zero = approached(0.0), one = approached(1.0), plus_infinity = approached(GAOL_INFINITY);
        pow_rel_end lower, upper;
        if (yu > 0.0) {
          /*
            y in [p1, p2], p1 > 0 or p1 = 0, which is then left out (x^0 = 1
            is not in K): x in [cl^(1/y), cu^(1/y)], whose lower end
            decreases with y for cl > 1 and increases for cl < 1, whose upper
            end does the same with cu, and which tends to 1 for y -> +oo and
            to 0 or +oo for y -> 0
          */
          const double p1 = (yl > 0.0) ? yl : 0.0, p2 = yu;
          if (cl <= 0.0) {
            lower = zero;
          } else if (cl == 1.0) {
            lower = reached(1.0);
          } else if (cl > 1.0) {
            lower = (p2 == GAOL_INFINITY) ? one : root_end(cl, p2, true);
          } else {
            lower = (p1 == 0.0) ? zero : root_end(cl, p1, true);
          }
          if (cu == GAOL_INFINITY) {
            upper = plus_infinity;
          } else if (cu == 1.0) {
            upper = reached(1.0);
          } else if (cu > 1.0) {
            upper = (p1 == 0.0) ? plus_infinity : root_end(cu, p1, false);
          } else {
            upper = (p2 == GAOL_INFINITY) ? one : root_end(cu, p2, false);
          }
          add_pow_rel_piece(lower, upper, xl, xu, lo, hi);
        }
        if (yl < 0.0) {
          /*
            y in [n1, n2], n2 < 0 or n2 = 0, which is then left out: x^y
            decreases with x, and x in [cu^(1/y), cl^(1/y)], whose lower end
            decreases with y for cu > 1 and increases for cu < 1, whose upper
            end does the same with cl, and which tends to 1 for y -> -oo and
            to 0 or +oo for y -> 0
          */
          const double n1 = yl, n2 = (yu < 0.0) ? yu : 0.0;
          if (cu == GAOL_INFINITY) {
            lower = zero;
          } else if (cu == 1.0) {
            lower = reached(1.0);
          } else if (cu > 1.0) {
            lower = (n2 == 0.0) ? zero : root_end(cu, n2, true);
          } else {
            lower = (n1 == -GAOL_INFINITY) ? one : root_end(cu, n1, true);
          }
          if (cl <= 0.0) {
            upper = plus_infinity;
          } else if (cl == 1.0) {
            upper = reached(1.0);
          } else if (cl > 1.0) {
            upper = (n1 == -GAOL_INFINITY) ? one : root_end(cl, n1, false);
          } else {
            upper = (n2 == 0.0) ? plus_infinity : root_end(cl, n2, false);
          }
          add_pow_rel_piece(lower, upper, xl, xu, lo, hi);
        }
      }
    }
    // Empty where nothing was added, lo being +oo and hi -oo
    return interval(lo, hi);
  }

  interval pow_rel(const interval& K, const interval& J, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = pow_rel_upward(K, J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    pow_exponent_rel() after its check, made before the bounds are compared
  */
  static interval pow_exponent_rel_upward(const interval& K, const interval& H, const interval& I)
  {
    // A power or a base [+oo] or [-oo] holds no real number, as in pow_rel_upward()
    if (K.is_empty() || H.is_empty() || I.is_empty() || holds_no_real(K) || holds_no_real(H)) {
      return interval::emptyset();
    }
    const double xu = H.right(), cu = K.right();
    if (xu < 0.0 || cu < 0.0) {
      return interval::emptyset(); // No base x >= 0, or no power
    }
    const double xl = (H.left() > 0.0) ? H.left() : 0.0, cl = K.left(), yl = I.left(), yu = I.right();
    double lo = GAOL_INFINITY, hi = -GAOL_INFINITY;
    if (xl == 0.0 && cl <= 0.0 && yu > 0.0) {
      // 0^y = 0 for y > 0: the y > 0 of I
      lo = (yl > 0.0) ? yl : 0.0;
      hi = yu;
    }
    if (xu > 0.0 && cu > 0.0) {
      if (cl <= 1.0 && cu >= 1.0 && xl <= 1.0 && xu >= 1.0) {
        // 1^y = 1 for every y
        lo = yl;
        hi = yu;
      } else {
        const pow_rel_end zero = approached(0.0), plus_infinity = approached(GAOL_INFINITY),
          minus_infinity = approached(-GAOL_INFINITY);
        pow_rel_end lower, upper;
        if (xu > 1.0) {
          /*
            x in [a1, a2], a1 > 1 or a1 = 1, which is then left out (1^y = 1
            is not in K): y in [log(cl)/log(x), log(cu)/log(x)], whose lower
            end decreases with x for cl > 1 and increases for cl < 1, whose
            upper end does the same with cu, and which tends to 0 for
            x -> +oo and to -oo or +oo for x -> 1
          */
          const double a1 = (xl > 1.0) ? xl : 1.0, a2 = xu;
          if (cl <= 0.0) {
            lower = minus_infinity;
          } else if (cl == 1.0) {
            lower = reached(0.0);
          } else if (cl > 1.0) {
            lower = (a2 == GAOL_INFINITY) ? zero : log_ratio_end(cl, a2, true);
          } else {
            lower = (a1 == 1.0) ? minus_infinity : log_ratio_end(cl, a1, true);
          }
          if (cu == GAOL_INFINITY) {
            upper = plus_infinity;
          } else if (cu == 1.0) {
            upper = reached(0.0);
          } else if (cu > 1.0) {
            upper = (a1 == 1.0) ? plus_infinity : log_ratio_end(cu, a1, false);
          } else {
            upper = (a2 == GAOL_INFINITY) ? zero : log_ratio_end(cu, a2, false);
          }
          add_pow_rel_piece(lower, upper, yl, yu, lo, hi);
        }
        if (xl < 1.0) {
          /*
            x in [b1, b2], b1 > 0 or b1 = 0, b2 < 1 or b2 = 1, both then left
            out: x^y decreases with y, and y in [log(cu)/log(x),
            log(cl)/log(x)], whose lower end decreases with x for cu > 1 and
            increases for cu < 1, whose upper end does the same with cl, and
            which tends to 0 for x -> 0 and to -oo or +oo for x -> 1
          */
          const double b1 = xl, b2 = (xu < 1.0) ? xu : 1.0;
          if (cu == GAOL_INFINITY) {
            lower = minus_infinity;
          } else if (cu == 1.0) {
            lower = reached(0.0);
          } else if (cu > 1.0) {
            lower = (b2 == 1.0) ? minus_infinity : log_ratio_end(cu, b2, true);
          } else {
            lower = (b1 == 0.0) ? zero : log_ratio_end(cu, b1, true);
          }
          if (cl <= 0.0) {
            upper = plus_infinity;
          } else if (cl == 1.0) {
            upper = reached(0.0);
          } else if (cl > 1.0) {
            upper = (b1 == 0.0) ? zero : log_ratio_end(cl, b1, false);
          } else {
            upper = (b2 == 1.0) ? plus_infinity : log_ratio_end(cl, b2, false);
          }
          add_pow_rel_piece(lower, upper, yl, yu, lo, hi);
        }
      }
    }
    return interval(lo, hi);
  }

  interval pow_exponent_rel(const interval& K, const interval& H, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = pow_exponent_rel_upward(K, H, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // The bodies of nth_root(), sqrt() and sqrt_rel(), defined below, which do
  // not check the rounding direction again (GAOL v5)
  static inline interval nth_root_upward(const interval& I, unsigned int n);
  static inline interval sqrt_upward(const interval& I);
  static interval sqrt_rel_upward(const interval& J, const interval& I);

  /*
    Code inspired by ia_math code by Timothy Hickey. The rounding direction
    is upward, after the check of nth_root_rel(), whose intersections and
    hulls are taken before the modes are restored with
    GAOL_PRESERVE_ROUNDING (GAOL v5, see gaol/gaol_fpu.h); the roots are
    those of the bodies of sqrt_rel() and nth_root(), which do not check the
    direction again (GAOL v5: they checked it a second time)
  */
  static interval nth_root_rel_upward(const interval& J, unsigned int n, const interval& I)
  {
    switch (n) {
    case 0:
      if (J.set_contains(1.0)) {
	return I;
      } else {
	return interval::emptyset();
      }
    case 1:
      return J & I;
    case 2:
      return sqrt_rel_upward(J,I);
    default:
      break;
    }
    // The roots of nth_root(), proved to be bounds (GAOL v5: GAOL took the
    // powers of the mathematical library with the exponent 1/n here too)
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const interval tmp = nth_root_upward(J,n);
    if (odd(n) || tmp.is_empty()) {
      return (tmp & I);
    }
    // n is even: the roots of either sign
    if (I.certainly_positive()) {
      return (tmp & I);
    }
    if (I.certainly_negative()) {
      return (-tmp & I);
    }
    return ((tmp & I) | ((-tmp) & I));
  }

  interval nth_root_rel(const interval& J, unsigned int n, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = nth_root_rel_upward(J, n, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    x^n rounded upward and downward, x >= 0 and n > 0, by binary exponentiation,
    the rounding direction being upward. x^n is a multiple of each square
    computed, so that ipow_hi() only overflows where x^n does; ipow_lo() is
    rounded downward through -((-a)*b) rounded upward, its product being kept
    negated.
  */
  static double ipow_hi(double x, unsigned int n)
  {
    double y = 1.0, z = x;
    for (;;) {
      if (odd(n)) {
	y *= z;
      }
      n >>= 1;
      if (n == 0) {
	return y;
      }
      z *= z;
    }
  }

  static double ipow_lo(double x, unsigned int n)
  {
    double y = -1.0, z = x;
    for (;;) {
      if (odd(n)) {
	y *= z;
      }
      n >>= 1;
      if (n == 0) {
	return -y;
      }
      z = -((-z)*z);
    }
  }

  /*
    ipow_hi() and ipow_lo() for the proofs of the roots of an x below
    tiny_root_argument (GAOL v5, point AC of TODO.md). Next to the root of
    x < 1, the powers l^k of the binary exponentiation (k <= n) are no smaller
    than x, which is below 2^-1022 for a subnormal x, and a subnormal has
    fewer bits: l^32 is about 2^-1041.5 for x = 2^-1074 and n = 33, which has
    33. Each rounded product there moved the proof away from the root by up to
    2^-32 relatively, and the root by that over n: nth_root([2^-1074], 33) was
    1983 doubles below its tightest lower bound and 28785 above its upper one,
    and n = 65 gave 3.5e8 and 6.9e8. The bounds still enclosed the root, each
    product being rounded in the direction of its proof. From
    tiny_root_argument, 4 times the smallest normal number, the powers next to
    the root (where l^n is within a relative 2^-17 of x) stay above 2^-1022,
    and ipow_hi() and ipow_lo() are used as they were.

    Here l = c 2^e with c in [1, 2) (exact), and the products are those of
    ipow_hi() and ipow_lo() on c, in the same order: l^n = h 2^s, s being an
    integer and h in [1, 2^500], scaled by 2^-500 above 2^500 (s is a long
    long: |e| n is below 2^43). Scaling by a power of two is exact and does
    not change what a product rounds to, so that h is the double of ipow_hi()
    or ipow_lo() wherever that one stays normal, and no product is rounded as
    a subnormal. Rounding upward, l > 0, n > 0
  */
  const double tiny_root_argument = 4.0*std::numeric_limits<double>::min();

  static double ipow_hi_scaled(double l, unsigned int n, long long& s)
  {
    const double big = 3.273390607896142e+150, small = 3.054936363499605e-151; // 2^500, 2^-500
    int e;
    double y = 1.0, z = 2.0*std::frexp(l, &e);
    long long ys = 0, zs = e - 1;
    for (;;) {
      if (odd(n)) {
	y *= z;
	ys += zs;
	if (y > big) {
	  y *= small;
	  ys += 500;
	}
      }
      n >>= 1;
      if (n == 0) {
	s = ys;
	return y;
      }
      z *= z;
      zs *= 2;
      if (z > big) {
	z *= small;
	zs += 500;
      }
    }
  }

  static double ipow_lo_scaled(double l, unsigned int n, long long& s)
  {
    const double big = 3.273390607896142e+150, small = 3.054936363499605e-151; // 2^500, 2^-500
    int e;
    double y = -1.0, z = 2.0*std::frexp(l, &e);
    long long ys = 0, zs = e - 1;
    for (;;) {
      if (odd(n)) {
	y *= z;
	ys += zs;
	if (y < -big) {
	  y *= small;
	  ys += 500;
	}
      }
      n >>= 1;
      if (n == 0) {
	s = ys;
	return -y;
      }
      z = -((-z)*z);
      zs *= 2;
      if (z > big) {
	z *= small;
	zs += 500;
      }
    }
  }

  // The sign of h 2^s - x, for h in [1, 2^500], s an integer and x > 0 finite:
  // h 2^s - x = (h 2^d - m) 2^(f - 1) with x = m 2^(f - 1), m in [1, 2)
  static int compare_scaled(double h, long long s, double x)
  {
    int f;
    const double m = 2.0*std::frexp(x, &f);
    const long long d = s - (f - 1);
    if (d > 0) { // h 2^d >= 2
      return 1;
    }
    if (d < -510) { // h 2^d <= 2^-11
      return -1;
    }
    const double t = gaol_ldexp(h, static_cast<int>(d)); // Exact: in [2^-510, 2^500]
    return (t < m) ? -1 : ((t > m) ? 1 : 0);
  }

  /*
    The n-th root of x rounded downward and upward, n > 2 and 0 < x < +oo, from
    an approximation r of it, the rounding direction being upward (fork of
    GAOL). GAOL took the power x^(1/n) of the mathematical library moved one
    double outward, 1/n being rounded: the root is then |log(x)|/n 2^-53 away
    relatively, up to 234 doubles for a cube root over all the doubles, and 8
    between 2^-30 and 2^30.

    A double l is below the root when l^n rounded upward is at most x, and u
    above it when u^n rounded downward is at least x: both tests grow with
    their argument. r is first brought next to the root by a step of Newton's
    method on r^n = x, r - r (r^n - x)/(n r^n), where r^n is a normal double,
    its rounding errors being relative then. The lower bound is the largest
    double proved to be below the root, and the upper bound the smallest one
    proved to be above it, looked for from r: outward or inward by steps that
    double, until a double that is proved and one that is not are found, then
    by bisection between them. From a double next to the root, that is two
    powers; from any other, as the math library of the system may give, the
    search still ends, and the bounds are proved.

    l^n, of n - 1 rounded products, is within (n - 1) 2^-52 of its value
    relatively (a product rounded in a direction is within 2^-52), which moves
    the root by less than 2^-52 relatively: the bounds are the tightest or up
    to 2 doubles beyond, and the root of a double that is an n-th power is that
    double. (GAOL v5: for an x below tiny_root_argument, the proofs take the
    scaled powers of ipow_hi_scaled() and ipow_lo_scaled(), whose products are
    never subnormal. This function does not: for a subnormal x its power is
    subnormal, its step is left out, and the search below starts from the
    value of CORE-MATH.)
  */
  static double newton_root(double x, double r, unsigned int n)
  {
    if (!(r > 0.0)) { // Negative or NaN
      return 0.0;
    }
    if (r > std::numeric_limits<double>::max()) {
      return std::numeric_limits<double>::max();
    }
    // The power of an r next to the root of an x above huge_root_argument may
    // overflow, which raised the overflow exception (GAOL v5, point 79 of
    // TODO.md): the step is left out, as for a subnormal x, and the search
    // starts from the value of CORE-MATH
    if (x > huge_root_argument) {
      return r;
    }
    const double p = ipow_hi(r,n);
    if (p >= std::numeric_limits<double>::min() && p <= std::numeric_limits<double>::max()) {
      // (p - x)/(n p) as ((p - x)/p)/n: n p overflowed for an n up to 2^32 and
      // a p above 2^992 (GAOL v5, point 79 of TODO.md). The step is left out
      // where it is 1 or more in magnitude, p being then no approximation of x
      const double t = quotient_sat(p - x, p)/double(n);
      if (std::fabs(t) < 1.0) {
	const double s = r - r*t;
	if (s > 0.0 && s <= std::numeric_limits<double>::max()) {
	  return s;
	}
      }
    }
    return r;
  }

  /*
    The two tests of the proofs, by the powers they take: ipow_hi() and
    ipow_lo(), or the scaled ones for an x below tiny_root_argument. The
    searches below are made for one or the other, chosen once (GAOL v5, point
    AC of TODO.md): a test of x inside them cost 3 to 30 ns of the 80 of the
    root of a normal x, according to how the compiler inlined them (n = 33,
    AppleClang 21, arm64, two builds of the same code)
  */
  struct plain_proofs
  {
    static inline bool below(double l, double x, unsigned int n)
    {
      return ipow_hi(l,n) <= x;
    }

    static inline bool above(double u, double x, unsigned int n)
    {
      return ipow_lo(u,n) >= x;
    }
  };

  struct scaled_proofs
  {
    static inline bool below(double l, double x, unsigned int n)
    {
      if (!(l > 0.0 && l < GAOL_INFINITY)) {
	return l == 0.0;
      }
      long long s;
      const double h = ipow_hi_scaled(l,n,s);
      return compare_scaled(h,s,x) <= 0;
    }

    static inline bool above(double u, double x, unsigned int n)
    {
      if (!(u > 0.0 && u < GAOL_INFINITY)) {
	return u == GAOL_INFINITY;
      }
      long long s;
      const double h = ipow_lo_scaled(u,n,s);
      return compare_scaled(h,s,x) >= 0;
    }
  };

  // A step of about one double at r, which doubles
  static inline double first_step(double r)
  {
    return maximum(r*std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::denorm_min());
  }

  /*
    A double between l and h, l < h, their middle where it is one: the double
    above l otherwise, which is h when l and h are consecutive. The neighbours
    are reached with nextafter() (next_float(), previous_float()), whatever
    the rounding direction and the treatment of subnormals.
  */
  static inline double double_between(double l, double h)
  {
    const double m = l + 0.5*(h - l);
    return (l < m && m < h) ? m : next_float(l);
  }

  // The steps of the searches below end within a few thousand steps (the
  // doubling ones reach 0 or the largest double within 2100), and are bounded
  // in case a platform would not move: the bound found so far is returned
  const int root_search_limit = 4096;

  template<class Proofs>
  static double proved_root_dn_by(double x, double r, unsigned int n)
  {
    // l is proved to be below the root, and h is not: 0 is, and the largest
    // double is not
    const double largest = std::numeric_limits<double>::max();
    double l = newton_root(x,r,n), h = l, step = first_step(l);
    int steps = 0;
    if (Proofs::below(l,x,n)) {
      for (h = minimum(l + step, largest); Proofs::below(h,x,n) && h < largest; h = minimum(l + step, largest)) {
	l = h;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return l;
	}
      }
      if (h >= largest && Proofs::below(h,x,n)) { // Not for n > 1 and a finite x
	return h;
      }
    } else {
      for (l = maximum(-(step - h), 0.0); !Proofs::below(l,x,n); l = maximum(-(step - h), 0.0)) {
	h = l;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return 0.0;
	}
      }
    }
    for (;;) {
      const double m = double_between(l,h);
      if (!(m < h) || ++steps > root_search_limit) {
	return l;
      }
      if (Proofs::below(m,x,n)) {
	l = m;
      } else {
	h = m;
      }
    }
  }

  template<class Proofs>
  static double proved_root_up_by(double x, double r, unsigned int n)
  {
    // u is proved to be above the root, and l is not: the largest double is,
    // and 0 is not
    const double largest = std::numeric_limits<double>::max();
    double u = newton_root(x,r,n), l = u, step = first_step(u);
    int steps = 0;
    if (Proofs::above(u,x,n)) {
      for (l = maximum(-(step - u), 0.0); Proofs::above(l,x,n); l = maximum(-(step - u), 0.0)) {
	u = l;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return u;
	}
      }
    } else {
      for (u = minimum(l + step, largest); !Proofs::above(u,x,n); u = minimum(l + step, largest)) {
	l = u;
	step *= 2.0;
	if (++steps > root_search_limit) {
	  return GAOL_INFINITY;
	}
      }
    }
    for (;;) {
      const double m = double_between(l,u);
      if (!(m < u) || ++steps > root_search_limit) {
	return u;
      }
      if (Proofs::above(m,x,n)) {
	u = m;
      } else {
	l = m;
      }
    }
  }

  // The scaled proofs also for an x above huge_root_argument, 2^1000: the
  // powers of the doubles next to the root, up to a relative 2^-17 beyond it
  // as far as the search goes, stay below 2^1024 up to there, and the plain
  // ones overflowed above the largest double for a finite root (GAOL v5, point
  // 79 of TODO.md: nth_root([DBL_MAX], 5))
  static double proved_root_dn(double x, double r, unsigned int n)
  {
    return (x < tiny_root_argument || x > huge_root_argument) ? proved_root_dn_by<scaled_proofs>(x,r,n)
                                                              : proved_root_dn_by<plain_proofs>(x,r,n);
  }

  static double proved_root_up(double x, double r, unsigned int n)
  {
    return (x < tiny_root_argument || x > huge_root_argument) ? proved_root_up_by<scaled_proofs>(x,r,n)
                                                              : proved_root_up_by<plain_proofs>(x,r,n);
  }

  /*
    The power d^e of CORE-MATH, e being 1/n rounded, for the approximation the
    n-th root of d starts from. The roots of 0, 1 and +oo are themselves.
    Rounding upward
  */
  static inline bool root_is_itself(double d)
  {
    return d == 0.0 || d == 1.0 || d == GAOL_INFINITY;
  }

  static inline double root_near(double d, double e)
  {
    return root_is_itself(d) ? d : upward::nthroot_up(d,e);
  }

  // Rounding upward
  static inline double root_dn(double d, double near_root, unsigned int n)
  {
    return root_is_itself(d) ? d : proved_root_dn(d,near_root,n);
  }

  static inline double root_up(double d, double near_root, unsigned int n)
  {
    return root_is_itself(d) ? d : proved_root_up(d,near_root,n);
  }

  /*
    The approximations the n-th roots of a >= 0 and b >= 0 start from. Rounding
    upward, as nth_root_upward(), its only caller, takes it after the check of
    its callers: the direction is not checked again here (GAOL v5)
  */
  static void near_roots(double a, double b, unsigned int n, double& near_a, double& near_b)
  {
    const double e = 1.0/double(n);
    near_a = root_near(a,e);
    near_b = (b == a) ? near_a : root_near(b,e);
  }

  /*
    Code inspired from ia_math by Timothy Hickey

    rootn of IEEE 1788-2015 (Table 10.5, GAOL v5): defined on R for an odd
    n, the root of x < 0 being -(-x)^(1/n), and on [0,+oo] for an even n. GAOL
    took the roots of the part of I in [0,+oo] for every n: nth_root([-8,27],3)
    was [0,3] rather than [-2,3]. rootn(x,0) is not defined, and gives the
    empty set.
  */
/*
  The cube root, with CORE-MATH's cbrt (GAOL v5)

  cbrt is correctly rounded in the rounding direction in effect and increasing
  on the whole line. Computed in the upward rounding GAOL keeps, cbrt(x) of a
  positive x is therefore the tightest double at or above the cube root, and
  the double below it the tightest one at or below, unless the cube root is a
  double: u being the value rounded upward, u^3 >= x, and the product of three
  positive numbers rounded upward is at least u^3, so it equals x only when
  u^3 = x exactly. That test is only made on positive numbers, where the
  roundings compose; the root of a negative number is the opposite of the root
  of its magnitude, as the general nth_root() takes it.

  The general nth_root() looked for the bounds from pow(x, 1/n) by Newton's
  method then by bisection, which took 179 ns for a cube root against 67 ns
  here, and gave bounds up to 2 doubles from the tightest ones.
*/
/*
  u*u*u rounded upward == x, for u the cube root of x rounded upward. The
  products reach 2^1024 for an x next to the largest double, which raised the
  overflow exception for a finite root (GAOL v5, point 79 of TODO.md): from
  huge_root_argument, the root u >= 2^333 is scaled by 2^-342 and x by 2^-1026,
  exactly (x >= 2^-26 is then normal), as the square root is (see
  gaol_sqrt_up()), and the products scaled with them are rounded to the same
  bits
*/
static const double two_power_minus_342 = 1.1161986242990967e-103, two_power_minus_513 = 3.7291703656001034e-155;

static inline bool cube_up_equals(double u, double x)
{
  if (x > huge_root_argument) {
    const double t = read_in_branch(u)*two_power_minus_342;
    return t*t*t == (read_in_branch(x)*two_power_minus_513)*two_power_minus_513;
  }
  return u*u*u == x;
}

// Inlined by force with GCC and Clang, as cube_root_upward() and cube_root()
// below
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline double cube_root_up(double x) // x >= 0
{
  return gaol_cr_cbrt(x);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline double cube_root_dn(double x) // x >= 0
{
  const double u = gaol_cr_cbrt(x);
  return cube_up_equals(u, x) ? u : previous_float(u);
}

// The rounding direction being upward already, after the check of cube_root()
// or of the callers of nth_root_upward() (GAOL v5). Inlined by force with GCC
// and Clang, as cube_root(), cube_root_up() and cube_root_dn(): Clang 18
// called cube_root_dn() from nth_root() otherwise, and the cube root of an
// interval took 40 ns rather than 34 (Intel i7-1185G7)
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline interval cube_root_upward(const interval& I)
{
  if (I.is_empty()) {
    return interval::emptyset();
  }
  const double a = I.left(), b = I.right();
  double lo, hi;
  if (a == -GAOL_INFINITY) {
    lo = -GAOL_INFINITY;
  } else {
    lo = (a >= 0.0) ? cube_root_dn(a) : -cube_root_up(-a);
  }
  if (b == GAOL_INFINITY) {
    hi = GAOL_INFINITY;
  } else {
    hi = (b >= 0.0) ? cube_root_up(b) : -cube_root_dn(-b);
  }
  return interval(lo, hi);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
static inline interval cube_root(const interval& I)
{
  if (I.is_empty()) {
    return interval::emptyset();
  }
  GAOL_RND_ENTER();
  interval res = cube_root_upward(I);
  GAOL_RND_KEEP(res);
  GAOL_RND_LEAVE();
  return res;
}

/*
  The n-th root of I, the rounding direction being upward already: the body
  of nth_root(), which nth_root(I, q) for q < 0 and nth_root_rel() call after
  their own check (GAOL v5: they called nth_root(), sqrt() or inverse(), which
  checked the direction again)
*/
static inline interval nth_root_upward(const interval& I, unsigned int n)
{
	switch (n) {
	case 0:
		return interval::emptyset();
	case 1:
		return I;
	case 2:
		return sqrt_upward(I);
	case 3:
		return cube_root_upward(I);
	default:
		break;
	}
	const interval J = odd(n) ? I : (I & interval::positive());
	if (J.is_empty()) {
		return interval::emptyset();
	}
	// The roots of the magnitudes of the bounds, the root of x < 0 being
	// -(-x)^(1/n)
	const double a = std::fabs(J.left()), b = std::fabs(J.right());
	double near_a, near_b;
	near_roots(a,b,n,near_a,near_b);
	const double l = (J.left() >= 0.0) ? root_dn(a,near_a,n) : -root_up(a,near_a,n);
	const double r = (J.right() >= 0.0) ? root_up(b,near_b,n) : -root_dn(b,near_b,n);
	return interval(l,r);
}

interval nth_root(const interval& I, unsigned int n)
{
	switch (n) {
	case 0:
		return interval::emptyset();
	case 1:
		return I;
	case 2:
		return sqrt(I);
	case 3:
		return cube_root(I);
	default:
		break;
	}
	// The check before the intersection, as in sqrt()
	GAOL_RND_ENTER();
	interval res = nth_root_upward(I, n);
	GAOL_RND_KEEP(res);
	GAOL_RND_LEAVE();
	return res;
}

/*
  rootn(x, q) with an integer q, which may be negative (GAOL v5)

  IEEE 1788-2015 recommends rootn(x, q) for every q of Z\{0} (Table 10.5),
  where GAOL only took a positive one. For q < 0, x^(1/q) = 1/x^(1/|q|), whose
  domain is the one the table gives: R\{0} for an odd q, (0, +oo) for an even
  one. Taking the inverse gives exactly that, the inverse of an interval
  holding 0 being the hull of the values away from it, so the result is the
  natural interval extension: rootn([-1, 1], -3) is the hull of
  (-oo, -1] u [1, +oo), that is the whole line, and rootn([0], -3) is empty.
*/
interval nth_root(const interval& I, int q)
{
	if (q > 0) {
		return nth_root(I, static_cast<unsigned int>(q));
	}
	if (q == 0) {
		// As rootn(x, 0), which the table leaves out: no value
		return interval::emptyset();
	}
	// |q| without a signed negation: -q overflows an int for INT_MIN, and so
	// did -static_cast<long>(q) where a long has 32 bits too, with Visual C++,
	// MinGW-w64 and on 32-bit Linux (GAOL v5)
	const unsigned int n = 0u - static_cast<unsigned int>(q);
	// One check, before the root and its inverse, which do not check the
	// direction again (GAOL v5: the root and inverse() checked it each)
	GAOL_RND_ENTER();
	const interval root = nth_root_upward(I, n);
	interval res = root.is_empty() ? root : upward_access::inverse(root);
	GAOL_RND_KEEP(res);
	GAOL_RND_LEAVE();
	return res;
}

  /*
    The root of order q of I for a q beyond the ints and the unsigned ints,
    after one check of the rounding direction (GAOL v5: the template
    gaol_detail::huge_root() of gaol/gaol_interval_core.h computed it, where 1/|q|
    and the two powers checked it once each): order is the tightest interval
    holding |q|, negative and odd tell the sign and the parity of q, and the
    inverse of order has positive bounds. Declared in no header, and not
    exported from a shared library, which is built with hidden visibility.
  */
  interval root_of_large_order(const interval& I, const interval& order, bool negative, bool odd)
  {
    GAOL_RND_ENTER();
    const interval inverse = upward_access::inverse(order);
    const interval exponent = negative ? -inverse : inverse;
    interval res = pow_hybrid_upward(I & interval::positive(), exponent);
    if (odd) {
      res = res | -pow_hybrid_upward(-(I & interval::negative()), exponent);
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval nth_root(const interval& I, long long q)
  {
    if (q >= INT_MIN && q <= INT_MAX) {
      return nth_root(I, static_cast<int>(q));
    }
    if (q > 0 && q <= static_cast<long long>(UINT_MAX)) {
      return nth_root(I, static_cast<unsigned int>(q));
    }
    return root_of_large_order(I, (q < 0) ? -interval(q) : interval(q), q < 0, q % 2 != 0);
  }

  interval nth_root(const interval& I, unsigned long long q)
  {
    if (q <= UINT_MAX) {
      return nth_root(I, static_cast<unsigned int>(q));
    }
    return root_of_large_order(I, interval(q), false, q % 2 != 0);
  }

  unsigned long long nb_fp_numbers(double a, double b)
  {
    // a and b compared as bounds, whatever the modes that flush the
    // subnormals to zero (gaol_port.h): under denormals-are-zero,
    // nb_fp_numbers(2^-1074, 2*2^-1074) was 1 (GAOL v5, point Q of TODO.md)
    if (!is_finite(a) || !is_finite(b) || gaol_detail::bound_greater(a, b)) {
      // Either a or b is a NaN or +/-oo, or [a,b] is empty? gaol_ERROR
      // throws, or aborts where the exceptions are disabled: there is no
      // value to return, whose line Visual C++ found unreachable (C4702)
      gaol_ERROR(invalid_action_error,"invalid argument(s) in call to nb_fp_numbers()");
    }

    if (gaol_detail::bound_equal(a, b)) {
      return 1;
    }

    /*
      The doubles are numbered by the bits of their absolute values, which
      grow with the doubles, -0 and +0 being one number. With the bits of a
      and b themselves, the sign bit of a lower bound -0 (which [1, 2] - 1 has,
      and -0 >= 0), or of a negative a with b = +0, made the difference wrap
      around: GAOL 4 returned 13830554455654793217 for nb_fp_numbers(-0.0, 1.0).
    */
    ullidouble ai, bi;
    ai.d = std::fabs(a);
    bi.d = std::fabs(b);
    if (gaol_detail::bound_greater_equal(a, 0.0)) {
      return (bi.i-ai.i)+1;
    }
    if (gaol_detail::bound_less_equal(b, 0.0)) {
      return (ai.i-bi.i)+1;
    }
    // a < 0 < b: the doubles from a to -0 and from +0 to b, zero being counted once
    return bi.i+ai.i+1;
  }

  /*
    The exponentials and the logarithms in base 2 and 10 (GAOL v5)

    IEEE 1788-2015 requires them among the forward elementary functions
    (Table 9.1: "exp, exp2, exp10(x) = b^x" on R with range (0, +oo), and
    "log, log2, log10(x) = log_b(x)" on (0, +oo), note d saying b = e, 2 or
    10), and GAOL did not provide them. CORE-MATH computes them correctly
    rounded in the rounding direction in effect, so, in the upward rounding
    GAOL keeps, the value at the right bound is the upper bound and the double
    below the value at the left bound the lower one -- unless that value is
    itself a double, and the tests below tell when it is. The bounds are then
    the tightest ones, as they are for exp and log.
  */

  // 2^x is a double exactly when x is an integer of [-1074, 1023]: 2^-1074 is
  // the smallest subnormal and 2^1024 overflows
  static inline bool exp2_is_exact(double x)
  {
    return x == std::floor(x) && x >= -1074.0 && x <= 1023.0;
  }

  // 10^x is a double exactly when x is an integer of [0, 22]: 10^23 is not a
  // double, and no negative power of ten is (0.1 is not one)
  static inline bool exp10_is_exact(double x)
  {
    return x == std::floor(x) && x >= 0.0 && x <= 22.0;
  }

  // log2(x) is a double exactly when x is a power of two, which frexp tells:
  // it writes x as m*2^e with m in [1/2, 1), so m is 1/2 for a power of two
  // and nothing else (and 0 for zero, inf for an infinity)
  static inline bool log2_is_exact(double x)
  {
    int e;
    return std::frexp(x, &e) == 0.5;
  }

  // log10(x) is a double exactly when x is a power of ten with an exponent in
  // [0, 22], the powers of ten that are doubles. The value rounded upward is
  // an integer of that range for those x, and might be for others, which
  // raising ten to it settles: 10^n is exact there.
  static inline bool log10_is_exact(double x)
  {
    if (!(x > 0.0) || !is_finite(x)) {
      return false;
    }
    const double n = gaol_cr_log10(x);
    if (n != std::floor(n) || n < 0.0 || n > 22.0) {
      return false;
    }
    return gaol_cr_exp10(n) == x;
  }

  interval exp2(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, and the maximum taken before
    // GAOL_RND_LEAVE(), as exp() does (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    const double w = gaol_cr_exp2(l);
    const double u = exp2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp2(r);
    // Within [0, +oo], as exp: 2^x is positive
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval exp10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    const double w = gaol_cr_exp10(l);
    const double u = exp10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_exp10(r);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval log2(const interval& I)
  {
    // Defined on (0, +oo), as log: I holding no positive number gives the
    // empty set (IEEE 1788-2015, Table 9.1), the bounds being compared after
    // the check, as in log()
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    const double w = gaol_cr_log2(l);
    const double u = log2_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log2(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log10(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(0.0, I.left()), r = I.right();
    const double w = gaol_cr_log10(l);
    const double u = log10_is_exact(l) ? w : previous_float(w);
    const double v = gaol_cr_log10(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  /*
    The forward functions IEEE 1788-2015 recommends (Table 10.5) (GAOL v5)

    expm1, exp2m1 and exp10m1 compute b^x - 1 without the cancellation of
    subtracting 1 from b^x near 0, and atanpi, acospi, sinpi, cospi and tanpi a
    function of pi*x, or one divided by pi, without the loss of accuracy of pi
    being irrational (notes c and e of the table). CORE-MATH computes each of
    them correctly rounded in the rounding direction in effect, as it does
    exp2, so the bounds are got the same way: in the upward rounding GAOL keeps,
    the value at a bound rounded upward, and the double below it for a lower
    bound unless the value is itself a double, which the tests below tell.
  */

  // b^x - 1 is a double exactly at 0, and for b = 2 at the integers of
  // [-53, 53], for b = 10 at those of [0, 15]: 2^n - 1 has n significant bits,
  // 1 - 2^-n as many, and 10^n - 1 is below 2^53 up to n = 15
  static inline bool expm1_is_exact(double x)
  {
    return x == 0.0;
  }

  static inline bool exp2m1_is_exact(double x)
  {
    return x == std::floor(x) && x >= -53.0 && x <= 53.0;
  }

  static inline bool exp10m1_is_exact(double x)
  {
    return x == std::floor(x) && x >= 0.0 && x <= 15.0;
  }

  // atan(x)/pi is rational at a rational x only for x in {0, +-1}, by Niven's
  // theorem, where it is 0 and +-1/4; and it is +-1/2 at the infinities
  static inline bool atanpi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0 || !is_finite(x);
  }

  // acos(x)/pi is a double at -1, 0 and 1 only, where it is 1, 1/2 and 0: the
  // other rational values, 1/3 and 2/3 at +-1/2, are no doubles
  static inline bool acospi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0;
  }

  // An increasing function, whose range the bounds are brought back into: the
  // value at the right bound, the double below the value at the left bound
  // unless it is a double. The rounding direction is upward, after the check
  // of increasing_cr() or of the caller, made before I is computed (see
  // gaol/gaol_fpu.h)
  static interval increasing_cr_upward(const interval& I, double (*f)(double),
                                       bool (*exact)(double), double lowest, double highest)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    const double w = f(l);
    const double u = exact(l) ? w : previous_float(w);
    const double v = f(r);
    return interval(maximum(lowest, u), minimum(highest, v));
  }

  static interval increasing_cr(const interval& I, double (*f)(double),
                                bool (*exact)(double), double lowest, double highest)
  {
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I, f, exact, lowest, highest);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // Within [-1, +oo]: b^x - 1 exceeds -1, which it nears as x goes to -oo.
  // +oo is read after the check: GAOL_INFINITY was the HUGE_VAL of the UCRT,
  // which clang-cl computes when the program runs, and it was FLT_MAX in the
  // caller's downward rounding, which made expm1([1e10]) empty (GAOL v5)
  static interval increasing_cr_from_minus_one(const interval& I, double (*f)(double),
                                               bool (*exact)(double))
  {
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I, f, exact, -1.0, GAOL_INFINITY);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval expm1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_expm1, expm1_is_exact);
  }

  interval exp2m1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_exp2m1, exp2m1_is_exact);
  }

  interval exp10m1(const interval& I)
  {
    return increasing_cr_from_minus_one(I, gaol_cr_exp10m1, exp10m1_is_exact);
  }

  interval atanpi(const interval& I)
  {
    // within [-1/2, 1/2], the image of atan divided by pi
    return increasing_cr(I, gaol_cr_atanpi, atanpi_is_exact, -0.5, 0.5);
  }

  interval acospi(const interval& I)
  {
    // Defined on [-1, 1], as acos (IEEE 1788-2015, Table 10.5): the part of I
    // outside is left out, and I holding no point of it gives the empty set
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (I.right() < -1.0 || I.left() > 1.0) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    const double l = maximum(-1.0, I.left()), r = minimum(1.0, I.right());
    // decreasing: the upper bound at the left bound, the lower one at the right
    const double v = gaol_cr_acospi(l);
    const double w = gaol_cr_acospi(r);
    const double u = acospi_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), minimum(1.0, v));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    sinpi, cospi and tanpi: the points where these functions change direction,
    or have a pole, are the multiples of 1/2, which are doubles, so the
    analysis of an interval reduces exactly to the integers t = 2x. Doubling a
    double is exact, and so are its ceiling and its floor; below 2^61 these
    integers are held exactly in 64 bits, and two distinct doubles beyond 2^61
    are at least 2^9 apart, whole periods. sin(pi*x) and cos(pi*x) have their
    extrema at the odd, respectively even, t, and tan(pi*x) its poles at the
    odd t, increasing between them.
  */
  static const double gaol_two_61 = 2305843009213693952.0;

  // Whether an integer congruent to r modulo m lies in [lo, hi]
  static inline bool holds_residue(std::int64_t lo, std::int64_t hi,
                                   std::int64_t r, std::int64_t m)
  {
    if (lo > hi) {
      return false;
    }
    std::int64_t lm = lo % m;
    if (lm < 0) {
      lm += m;
    }
    std::int64_t d = (r - lm) % m;
    if (d < 0) {
      d += m;
    }
    return lo + d <= hi;
  }

  // Whether k*x is an integer: sin(pi*x) and cos(pi*x) are doubles exactly at
  // the multiples of 1/2 (k = 2), and tan(pi*x) at those of 1/4 (k = 4). By
  // Niven's theorem sin and cos take at a rational x the rational values 0,
  // +-1/2 and +-1 only, +-1/2 at points that are not doubles, and tan the
  // values 0 and +-1 only. Beyond 2^61 every double is an even integer.
  static inline bool is_multiple_of_inverse(double x, double k)
  {
    if (std::fabs(x) >= gaol_two_61) {
      return true;
    }
    const double t = k * x;
    return t == std::floor(t);
  }

  // sin(pi*x) or cos(pi*x) over I: the maximum 1 where a t congruent to up
  // modulo 4 lies within, the minimum -1 where one congruent to down does, and
  // the values at the bounds otherwise. The rounding direction is upward, after
  // the check of sin_or_cos_pi(), made before the bounds are compared: 2 l,
  // whose ceiling gives ka, is 0 for a subnormal l with the modes that flush
  // them to zero (see gaol/gaol_fpu.h)
  static interval sin_or_cos_pi_upward(const interval& I, double (*f)(double),
                                       std::int64_t up, std::int64_t down)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    if (!is_finite(l) || !is_finite(r)) {
      return interval(-1.0, 1.0);
    }
    const double far = maximum(std::fabs(l), std::fabs(r));
    if (l != r && far >= gaol_two_61) {
      return interval(-1.0, 1.0);
    }
    bool has_max = false, has_min = false;
    if (far < gaol_two_61) {
      const std::int64_t ka = static_cast<std::int64_t>(std::ceil(2.0 * l));
      const std::int64_t kb = static_cast<std::int64_t>(std::floor(2.0 * r));
      if (kb - ka >= 3) {
        return interval(-1.0, 1.0);  // every residue modulo 4
      }
      has_max = holds_residue(ka, kb, up, 4);
      has_min = holds_residue(ka, kb, down, 4);
    }
    const double fl = f(l), fr = f(r);
    const double dl = is_multiple_of_inverse(l, 2.0) ? fl : previous_float(fl);
    const double dr = is_multiple_of_inverse(r, 2.0) ? fr : previous_float(fr);
    const double lower = has_min ? -1.0 : maximum(-1.0, minimum(dl, dr));
    const double upper = has_max ? 1.0 : minimum(1.0, maximum(fl, fr));
    return interval(lower, upper);
  }

  /* The minimum and the maximum taken before GAOL_RND_LEAVE(): with
     GAOL_PRESERVE_ROUNDING, the denormals-are-zero mode it restores made two
     subnormal values equal, and sinpi([100, 1000] 2^-1074) was empty (GAOL v5,
     review of point 4) */
  static interval sin_or_cos_pi(const interval& I, double (*f)(double),
                                std::int64_t up, std::int64_t down)
  {
    GAOL_RND_ENTER();
    interval res = sin_or_cos_pi_upward(I, f, up, down);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval sinpi(const interval& I)
  {
    // sin(pi*x) = 1 at x = 1/2 modulo 2, t = 1 modulo 4; -1 at t = 3
    return sin_or_cos_pi(I, gaol_cr_sinpi, 1, 3);
  }

  interval cospi(const interval& I)
  {
    // cos(pi*x) = 1 at the even x, t = 0 modulo 4; -1 at the odd x, t = 2
    return sin_or_cos_pi(I, gaol_cr_cospi, 0, 2);
  }

  // tanpi() after its check, made before the bounds are compared, as in
  // sin_or_cos_pi()
  static interval tanpi_upward(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const double l = I.left(), r = I.right();
    if (!is_finite(l) || !is_finite(r)) {
      return interval::universe();
    }
    const double far = maximum(std::fabs(l), std::fabs(r));
    if (l != r && far >= gaol_two_61) {
      return interval::universe();
    }
    bool pole_inside = false, pole_at_l = false, pole_at_r = false;
    if (far < gaol_two_61) {
      const double tl = 2.0 * l, tr = 2.0 * r;
      // the integers strictly between tl and tr, and the poles among them
      const std::int64_t lo = static_cast<std::int64_t>(std::floor(tl)) + 1;
      const std::int64_t hi = static_cast<std::int64_t>(std::ceil(tr)) - 1;
      pole_inside = holds_residue(lo, hi, 1, 2);
      pole_at_l = tl == std::floor(tl) && std::fmod(tl, 2.0) != 0.0;
      pole_at_r = tr == std::floor(tr) && std::fmod(tr, 2.0) != 0.0;
    }
    // tan(pi*x) has no value at a pole (IEEE 1788-2015, Table 10.5), and is
    // increasing between two poles, from -oo to +oo
    if (l == r && pole_at_l) {
      return interval::emptyset();
    }
    if (pole_inside || (pole_at_l && pole_at_r)) {
      return interval::universe();
    }
    // a pole at a bound is the limit -oo from its right, +oo from its left
    double lower = -GAOL_INFINITY, upper = GAOL_INFINITY;
    if (!pole_at_l) {
      const double fl = gaol_cr_tanpi(l);
      lower = is_multiple_of_inverse(l, 4.0) ? fl : previous_float(fl);
    }
    if (!pole_at_r) {
      upper = gaol_cr_tanpi(r);
    }
    return interval(lower, upper);
  }

  interval tanpi(const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = tanpi_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    log1p, log2p1 and log10p1, the logp1, log2p1 and log10p1 of IEEE 1788-2015
    (Table 10.5): log_b(1 + x) on (-1, +oo), without the cancellation of
    adding 1 to x near 0. They are increasing and bounded as expm1 is; the
    part of I at or below -1 is left out, and the value at -1 is the limit -oo.
  */

  // log(1 + x) is rational at a rational x only for x = 0, where it is 0: the
  // logarithm of a rational other than 1 is transcendental (Lindemann)
  static inline bool log1p_is_exact(double x)
  {
    return x == 0.0 || x == -1.0 || x == GAOL_INFINITY;
  }

  // log2(1 + x) is a double exactly where 1 + x is a power of two 2^k: then
  // x = 2^k - 1, a double for k in [0, 53], the integers below 2^53 whose
  // successor is a power of two, and for k in [-53, -1], in (-1, -1/2] where
  // 1 + x is computed exactly (Sterbenz). A logarithm of a rational that is
  // not a power of two is irrational.
  static inline bool log2p1_is_exact(double x)
  {
    if (x == -1.0 || x == GAOL_INFINITY) {
      return true;
    }
    if (x >= 0.0 && x < 9007199254740992.0 && x == std::floor(x)) { // 2^53
      const std::uint64_t n = static_cast<std::uint64_t>(x);
      return (n & (n + 1)) == 0;
    }
    if (x > -1.0 && x <= -0.5) {
      int e;
      return std::frexp(1.0 + x, &e) == 0.5;
    }
    return false;
  }

  // log10(1 + x) is a double exactly where 1 + x is a power of ten 10^k with
  // k >= 0, the others being no doubles: x = 10^k - 1 for k in [0, 15],
  // 10^16 - 1 being above 2^53
  static inline bool log10p1_is_exact(double x)
  {
    if (x == -1.0 || x == GAOL_INFINITY) {
      return true;
    }
    if (!(x >= 0.0 && x < 1e15 && x == std::floor(x))) {
      return false;
    }
    double p = 1.0; // 10^k, exact up to 10^15
    for (int k = 0; k <= 15; ++k, p *= 10.0) {
      if (x == p - 1.0) {
        return true;
      }
    }
    return false;
  }

  // The check before the interval cut at -1 is made (see increasing_cr())
  static interval log_p1(const interval& I, double (*f)(double), bool (*exact)(double))
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    interval res = !(I.right() > -1.0) ? interval::emptyset()
      : increasing_cr_upward(interval(maximum(-1.0, I.left()), I.right()), f, exact,
                             -GAOL_INFINITY, GAOL_INFINITY);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval log1p(const interval& I)
  {
    return log_p1(I, gaol_cr_log1p, log1p_is_exact);
  }

  interval log2p1(const interval& I)
  {
    return log_p1(I, gaol_cr_log2p1, log2p1_is_exact);
  }

  interval log10p1(const interval& I)
  {
    return log_p1(I, gaol_cr_log10p1, log10p1_is_exact);
  }

  /*
    rsqrt, the rSqrt of IEEE 1788-2015 (Table 10.5): 1/sqrt(x) on (0, +oo),
    decreasing, +oo the limit at 0.

    1/sqrt(x) is a double exactly at the powers of two of even exponent,
    2^(2k), where it is 2^-k: were 1/sqrt(x) = m 2^e with m an odd integer
    above 1, x = 2^(-2e)/m^2 would be no double. And it is 0 at +oo.
  */
  static inline bool rsqrt_is_exact(double x)
  {
    if (x == GAOL_INFINITY) {
      return true;
    }
    int e;
    return std::frexp(x, &e) == 0.5 && (e - 1) % 2 == 0;
  }

  interval rsqrt(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    // +0 rather than -0 for a left bound at or below 0: 1/sqrt(-0) is -oo
    const double l = (I.left() > 0.0) ? I.left() : 0.0, r = I.right();
    // decreasing: the upper bound at the left bound, the lower one at the right
    const double v = gaol_cr_rsqrt(l);
    const double w = gaol_cr_rsqrt(r);
    const double u = rsqrt_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    asinpi, the asinPi of IEEE 1788-2015 (Table 10.5): asin(x)/pi on [-1, 1],
    increasing, within [-1/2, 1/2].

    asin(x)/pi is rational at a rational x only for x in {0, +-1/2, +-1}, by
    Niven's theorem, where it is 0, +-1/6 and +-1/2: a double at 0 and +-1.
  */
  static inline bool asinpi_is_exact(double x)
  {
    return x == 0.0 || x == 1.0 || x == -1.0;
  }

  interval asinpi(const interval& I)
  {
    // the part of I outside [-1, 1] is left out, as with acospi, after the
    // check: GCC 13 compared the bounds of the intersection, held in
    // registers, with 0 before it, and asinpi([100 2^-1074]) was
    // [32 2^-1074] with the denormals-are-zero mode set (GAOL v5)
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I & interval::minus_one_plus_one(), gaol_cr_asinpi,
                                        asinpi_is_exact, -0.5, 0.5);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    hypot of IEEE 1788-2015 (Table 10.5): sqrt(x^2 + y^2) on the plane. It
    increases with |x| and with |y|: its least value on X x Y is at the point
    nearest to the origin, (mig X, mig Y), and its greatest at the farthest,
    (mag X, mag Y).
  */

  // Whether h^2 - b^2 = a^2 4^d, for odd integers h, b, a below 2^53: both
  // sides are integers below 2^107, which a double with the residual of its
  // rounding holds exactly, std::fma computing the residual. The rounding of
  // an exact value is unique, whatever its direction: the two sides are equal
  // exactly when the roundings and the residuals are.
  static inline bool difference_of_squares(std::uint64_t h, std::uint64_t b,
                                           std::uint64_t a, int d)
  {
    if (h <= b || d > 53) { // a^2 4^d >= 2^(2d) > (h - b)(h + b) for d > 53
      return false;
    }
    const double s = static_cast<double>(h - b), t = static_cast<double>(h + b); // both exact
    const double m = static_cast<double>(a);
    const double p = s * t, q = m * m;
    return p == std::ldexp(q, 2 * d)
        && fma_once(s, t, -p) == std::ldexp(fma_once(m, m, -q), 2 * d);
  }

  /* Whether h, sqrt(a^2 + b^2) rounded upward, is the exact value, for
     a, b >= 0. It is at a = 0 or b = 0, where it is the other one, and at an
     infinity. Otherwise, with a = A 2^ea, b = B 2^eb and h = H 2^eh, A, B and H
     odd integers: dividing A^2 4^ea + B^2 4^eb = H^2 4^eh by the least power of
     4 among them leaves one, two or three odd squares, each 1 modulo 8, the
     other terms being multiples of 4. One odd square alone is not 0 modulo 4,
     A^2 + B^2 is 2 modulo 8 whether H^2 is left or not: the equality holds
     only where eh is the least exponent and equal to eb, H^2 - B^2 being
     A^2 4^(ea - eb), or equal to ea, the same with a and b exchanged. */
  static bool hypot_is_exact(double a, double b, double h)
  {
    if (a == 0.0 || b == 0.0 || !is_finite(a) || !is_finite(b)) {
      return true;
    }
    if (!is_finite(h)) {
      return false; // beyond the largest double
    }
    std::uint64_t A, B, H;
    int ea, eb, eh;
    odd_significand(a, A, ea);
    odd_significand(b, B, eb);
    odd_significand(h, H, eh);
    if (eh == eb && ea > eb) {
      return difference_of_squares(H, B, A, ea - eb);
    }
    if (eh == ea && eb > ea) {
      return difference_of_squares(H, A, B, eb - ea);
    }
    return false;
  }

  interval hypot(const interval& X, const interval& Y)
  {
    if (X.is_empty() || Y.is_empty()) {
      return interval::emptyset();
    }
    // mig and mag compare the bounds: after the check (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double al = X.mig(), bl = Y.mig(), ar = X.mag(), br = Y.mag();
    const double w = gaol_cr_hypot(al, bl);
    const double u = hypot_is_exact(al, bl, w) ? w : previous_float(w);
    const double v = gaol_cr_hypot(ar, br);
    interval res(maximum(0.0, u), v);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    atan2pi, the atan2Pi of IEEE 1788-2015 (Table 10.5): atan2(y, x)/pi on the
    plane but (0, 0), with values in (-1, 1]. The analysis of the box is that
    of atan2 above, the angles divided by pi. The angle over pi is a double
    where it is a multiple of 1/4, which the tests below find: 0, +-1/4, +-1/2,
    +-3/4 and 1, and -1 as a limit. Elsewhere it is irrational: tan(pi r) is
    rational for a rational r only where it is 0 or +-1 (Niven), and y/x is
    rational.
  */
  static inline bool atan2pi_special(double y, double x, double& v)
  {
    if (y == 0.0 || x == GAOL_INFINITY) {
      v = (x > 0.0) ? 0.0 : 1.0;
      return true;
    }
    if (x == 0.0 || std::fabs(y) == GAOL_INFINITY) {
      v = (y > 0.0) ? 0.5 : -0.5;
      return true;
    }
    if (x == -GAOL_INFINITY) {
      v = (y > 0.0) ? 1.0 : -1.0;
      return true;
    }
    if (std::fabs(x) == std::fabs(y)) {
      const double q = (x > 0.0) ? 0.25 : 0.75;
      v = (y > 0.0) ? q : -q;
      return true;
    }
    return false;
  }

  static double atan2pi_lo(double y, double x)
  {
    double v;
    return atan2pi_special(y, x, v) ? v : maximum(previous_float(gaol_cr_atan2pi(y, x)), -1.0);
  }

  static double atan2pi_hi(double y, double x)
  {
    double v;
    return atan2pi_special(y, x, v) ? v : minimum(gaol_cr_atan2pi(y, x), 1.0);
  }

  // atan2pi() after its check, made before the bounds are compared, as in
  // atan2()
  static interval atan2pi_upward(const interval& Y, const interval& X)
  {
    if (Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    if (yl == 0.0 && yu == 0.0 && xl == 0.0 && xu == 0.0) {
      return interval::emptyset();
    }
    if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
      return interval(-1.0, 1.0);
    }
    double l, r;
    if (yl >= 0.0) { // Upper half-plane: the angle decreases with x
      l = atan2pi_lo((xu > 0.0) ? yl : yu, xu);
      r = (xl == 0.0 && yu == 0.0) ? 0.0 : atan2pi_hi((xl >= 0.0) ? yu : yl, xl);
    } else if (yu < 0.0) { // Lower half-plane, y = 0 left out: the angle increases with x
      l = atan2pi_lo((xl >= 0.0) ? yl : yu, xl);
      r = atan2pi_hi((xu > 0.0) ? yu : yl, xu);
    } else { // yl < 0 <= yu, in the right half-plane
      l = atan2pi_lo(yl, xl);
      r = (yu == 0.0) ? ((xu == 0.0) ? -0.5 : 0.0) : atan2pi_hi(yu, xl);
    }
    return interval(l, r);
  }

  interval atan2pi(const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = atan2pi_upward(Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    fma(X, Y, Z) of IEEE 1788-2015 (Table 9.1), whose tightness is required
    (12.10.2) (GAOL v5)

    x*y ranges over the hull of the products of the bounds of X and Y, x*y
    being bilinear, so x*y + z ranges from the least of those products plus
    inf Z to the greatest plus sup Z. Each corner is rounded once, by std::fma,
    upward for the upper bound and downward for the lower one as
    -fma(-a, b, -c): the bounds are the tightest. A bound 0 times an infinite
    one counts as 0, as in the product of intervals, where std::fma would give
    NaN; and an infinite bound of Z gives the bound of the result it is on.
  */
  static inline double fma_up(double a, double b, double c)
  {
    return (a == 0.0 || b == 0.0) ? c : fma_once(a, b, c);
  }

  /*
    The result of std::fma goes through rnd_keep() before it is negated: GCC
    folds -fma(-a, b, -c) into fma(a, b, c), a single vfmadd rounded upward
    whatever -frounding-math says, which gave the lower bound rounded the wrong
    way -- a bound one double above the exact one, the interval no longer
    enclosing it. The negations inside may be folded: -a*b - c is rounded
    upward all the same.
  */
  static inline double fma_down(double a, double b, double c)
  {
    return (a == 0.0 || b == 0.0) ? c : -gaol_core::rnd_keep(fma_once(-a, b, -c));
  }

  interval fma(const interval& X, const interval& Y, const interval& Z)
  {
    if (X.is_empty() || Y.is_empty() || Z.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, fma_up() and fma_down() comparing
    // them with 0 (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double xl = X.left(), xr = X.right(), yl = Y.left(), yr = Y.right();
    const double zl = Z.left(), zr = Z.right();
    double lower = -GAOL_INFINITY, upper = GAOL_INFINITY;
    if (zl != -GAOL_INFINITY) {
      lower = minimum(minimum(fma_down(xl, yl, zl), fma_down(xl, yr, zl)),
                      minimum(fma_down(xr, yl, zl), fma_down(xr, yr, zl)));
    }
    if (zr != GAOL_INFINITY) {
      upper = maximum(maximum(fma_up(xl, yl, zr), fma_up(xl, yr, zr)),
                      maximum(fma_up(xr, yl, zr), fma_up(xr, yr, zr)));
    }
    GAOL_RND_KEEP(lower); GAOL_RND_KEEP(upper);
    GAOL_RND_LEAVE();
    return interval(lower, upper);
  }

  /*
    cancelMinus and cancelPlus of IEEE 1788-2015 (10.5.6, 12.12.5) (GAOL v5)

    Whether the result exists depends on X being at least as wide as Y, that
    is sup X - sup Y >= inf X - inf Y, which rounding the two differences
    cannot decide when they are within a double of each other (note of
    12.12.5). Each difference is therefore computed exactly, as the sum of its
    value rounded to nearest and of its error, by the Fast2Sum of Dekker with
    the operand of the larger magnitude first, which is exact in the rounding
    to nearest: the rounded values compare as the exact ones unless they are
    equal, and the errors decide then.

    The bounds are finite, so an infinite sum is an overflow, and its error is
    not read: difference_at_least() decides by s alone, or halves the bounds.
    The TwoSum of Knuth, ap = s - b, bp = s - ap, e = (a - ap) + (b - bp),
    was not used: for an infinite s, ap is infinite and bp is inf - inf, and
    for a finite s too, where a = +-DBL_MAX and b of the other sign make a sum
    halfway between two doubles that rounds away from the exact one, by 2^970:
    s - b is then DBL_MAX + 2^970, which is infinite to nearest. The invalid-
    operation exception was raised (it stops a program that enabled it), and e
    was NaN, so that cancel_minus([DBL_MAX - 2^971, DBL_MAX], [2^970, 3 2^970]),
    whose bounds have the same width, read X as narrower than Y. s - a is
    exact, and so cannot overflow, when |b| <= |a| (GAOL v5)
  */
  static inline void two_sum(double a, double b, double& s, double& e)
  {
    s = a + b;
    if (std::isinf(s)) {
      e = 0.0;
      return;
    }
    if (std::fabs(a) < std::fabs(b)) {
      const double t = a;
      a = b;
      b = t;
    }
    const double z = s - a;
    e = b - z;
  }

  // Whether a1 - b1 >= a2 - b2, exactly
  static bool difference_at_least(double a1, double b1, double a2, double b2)
  {
    double s1, e1, s2, e2;
    GAOL_RND_NEAREST_ENTER();
    two_sum(a1, -b1, s1, e1);
    two_sum(a2, -b2, s2, e2);
    /* The four doubles go through rnd_keep() before the direction changes
       back: GCC computed them after GAOL_RND_NEAREST_LEAVE(), upward, where
       TwoSum is no longer exact, and cancel_minus([0.5], [4.9e-324, 1e-300]),
       whose exact differences are within a double of each other, took X to be
       wider than Y; the terms of the error of cancel_minus([DBL_MAX], [0.5])
       then read +oo and inf - inf, and raised FE_INVALID
       (GAOL v5) */
    s1 = gaol_core::rnd_keep(s1);
    e1 = gaol_core::rnd_keep(e1);
    s2 = gaol_core::rnd_keep(s2);
    e2 = gaol_core::rnd_keep(e2);
    GAOL_RND_NEAREST_LEAVE();
    const bool inf1 = std::isinf(s1), inf2 = std::isinf(s2);
    if (inf1 || inf2) {
      // A difference beyond the largest double decides the comparison unless
      // both are, with the same sign; the four doubles are then 2^970 or more
      // in magnitude (a difference beyond DBL_MAX of two doubles of at most
      // DBL_MAX leaves each of them 2^970 or more), where halving them is exact
      if (inf1 && inf2 && (s1 > 0.0) == (s2 > 0.0)) {
        return difference_at_least(0.5 * a1, 0.5 * b1, 0.5 * a2, 0.5 * b2);
      }
      return s1 >= s2;
    }
    if (s1 != s2) {
      return s1 > s2;
    }
    return e1 >= e2;
  }

  // cancel_minus() after its check, made before the differences of the bounds
  // are compared (see gaol/gaol_fpu.h)
  static interval cancel_minus_upward(const interval& X, const interval& Y)
  {
    const bool x_empty = X.is_empty(), y_empty = Y.is_empty();
    // no value at Level 1, which 12.12.5 has return [-oo, +oo]: an unbounded X
    // or Y, a nonempty X with an empty Y, or X narrower than Y
    if ((!x_empty && !X.is_finite()) || (!y_empty && !Y.is_finite())) {
      return interval::universe();
    }
    if (x_empty) {
      return interval::emptyset();
    }
    if (y_empty) {
      return interval::universe();
    }
    const double xl = X.left(), xr = X.right(), yl = Y.left(), yr = Y.right();
    if (!difference_at_least(xr, yr, xl, yl)) {
      return interval::universe();
    }
    // [xl - yl, xr - yr] rounded outward: xl - yl downward as -(yl - xl)
    const double lower = -(yl - xl);
    const double upper = xr - yr;
    return interval(lower, upper);
  }

  interval cancel_minus(const interval& X, const interval& Y)
  {
    GAOL_RND_ENTER();
    interval res = cancel_minus_upward(X, Y);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval cancel_plus(const interval& X, const interval& Y)
  {
    return cancel_minus(X, -Y);
  }

  interval exp(const interval& I)
  {
    // The empty set tested first: its NaN bounds would give the empty set
    // too, through the quiet comparison of the constructor, but after the
    // check of the rounding direction and two calls of CORE-MATH (GAOL v5)
    if (I.is_empty()) {
      return interval::emptyset();
    }
	/* We intersect the result with [0, +oo] to ensure that the result is strictly positive
 		Otherwise, we might have: exp([-oo, -MAX] = [-v, +v] with v very small.
	*/
    // exp(0) = 1 exactly, where the value of the mathematical library moved
    // outward gave exp([0]) a width, and pow([1], [-oo, +oo]) = [0, +oo]
    // (GAOL v5). The bounds read after the check (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    // The maximum taken before GAOL_RND_LEAVE(): with GAOL_PRESERVE_ROUNDING,
    // the denormals-are-zero mode it restores made the subnormal lower bound of
    // exp([-740]) equal to 0, and 0 the bound (GAOL v5)
    double u = (l == 0.0) ? 1.0 : maximum(0.0,upward::exp_dn(l));
    double v = (r == 0.0) ? 1.0 : upward::exp_up(r);
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }

  interval log(const interval& I)
  {
    // log is defined on (0,+oo) (IEEE 1788-2015, Table 9.1, GAOL v5): I
    // holding no positive number, as [-4,0] and [0], gives the empty set, where
    // GAOL kept its part in [0,+oo] and gave [-oo,-MAX]. The bounds compared
    // after the check: with denormals-are-zero, [1e-310, 1e-309] held no
    // positive number (GAOL v5)
    if (I.is_empty()) {
      return interval::emptyset();
    }
    GAOL_RND_ENTER();
    if (!(I.right() > 0.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    // log(1) = 0 exactly: log([1]) was [-2^-1074, 2^-1074] (GAOL v5)
    const double l = maximum(0.0,I.left()), r = I.right();
    // CORE-MATH's log, correctly rounded in the rounding direction in effect
    // (GAOL v5): in the upward rounding GAOL computes in, RU(log r) is the
    // right bound, and RD(log l) = pred(RU(log l)) the left one, log(l) being
    // no double for l other than 1. The tightest bounds, without switching the
    // rounding direction.
    const double u = (l == 1.0) ? 0.0 : previous_float(gaol_cr_log(l));
    const double v = (r == 1.0) ? 0.0 : gaol_cr_log(r);
    GAOL_RND_LEAVE();
    return interval(u, v);
  }


  /*
    The bounds of x/[pi] for a finite x, [pi] being [gaol_pi_dn, gaol_pi_up], rounded
    upward, for modulo_k_pi(): computed on doubles, as tan() computes those of
    [x + pi/2]/[pi] (GAOL v5). They are the doubles interval(x) /
    interval::pi() gave, the stored bounds -x and x being divided by gaol_pi_dn
    above 0 and by gaol_pi_up below. A bound 0 of I gives the quotient 0 with its
    own sign, where interval(x), then the division, gave one each build chose:
    floor() keeps it, and the comparisons of modulo_k_pi() do not tell -0
    from +0.
  */
  static inline double lower_of_x_over_pi(double x)
  {
    const double nl = -x;
    return -(nl / ((nl > 0.0) ? gaol_pi_dn : gaol_pi_up));
  }

  static inline double upper_of_x_over_pi(double x)
  {
    return x / ((x > 0.0) ? gaol_pi_dn : gaol_pi_up);
  }

  /*!
    \brief Angle modulo pi (slower version with integer output)

    Computes k_left and k_right for I=[a,b] such that
    * a = k_left*\pi + a'  with |a'|<\pi
    * b = k_right*\pi + b' with |b'|<\pi
    \return 0 if neither l_left nor l_right could be computed accurately,
    1 if l_right could be computed, 2 if l_left could be computed and
    3 if both could be computed.

    \note There might be more than one integer in
    I.left()/interval::pi() (resp. I.right()/interval::pi()) when one of the bounds of I
    is close to a multiple of \pi
  */
  unsigned short int modulo_k_pi(const interval &I, double &k_left, double &k_right)
  {
    // The empty set, whose bounds are NaN, gives NaN and 0, tested first
    // (GAOL v5): the floor of its NaN bounds divided by pi would be the empty
    // set too, but k_left its lower bound, a NaN of the other sign
    if (I.is_empty()) {
      k_left = k_right = GAOL_NAN;
      return 0;
    }
    // The only check of the rounding direction (GAOL v5): the quotients are
    // computed on doubles, where I.left()/interval::pi() and
    // I.right()/interval::pi() checked it once each. An infinite bound gives
    // NaN and no flag, as interval(+-oo), which is empty, gave through them
    GAOL_RND_ENTER();
    const double a = I.left(), b = I.right();
    const double dmax = (std::numeric_limits<double>::max)();
    const bool finite_a = std::fabs(a) <= dmax, finite_b = std::fabs(b) <= dmax;
    double a_lo = finite_a ? bound_floor(lower_of_x_over_pi(a)) : GAOL_NAN;
    double a_hi = finite_a ? bound_floor(upper_of_x_over_pi(a)) : GAOL_NAN;
    double b_lo = finite_b ? bound_floor(lower_of_x_over_pi(b)) : GAOL_NAN;
    double b_hi = finite_b ? bound_floor(upper_of_x_over_pi(b)) : GAOL_NAN;
    GAOL_RND_KEEP(a_lo); GAOL_RND_KEEP(a_hi); GAOL_RND_KEEP(b_lo); GAOL_RND_KEEP(b_hi);
    GAOL_RND_LEAVE();
    k_left  = a_lo;
    k_right = b_hi;
    return static_cast<unsigned short int>(((finite_a && a_lo == a_hi) ? 2 : 0) + ((finite_b && b_lo == b_hi) ? 1 : 0));
  }


  /*
    The bounds of [x + pi/2]/[pi] for a finite x, [pi] being [gaol_pi_dn, gaol_pi_up],
    rounded upward, as tan() needs them: computed on doubles rather than with
    the operations of intervals, which checked the rounding direction once more
    each, after the check of tan() (GAOL v5). They are the doubles
    interval(x) + interval::half_pi() and operator/=(interval) give, in the two
    builds: the stored bounds of the sum are nl = (-x) + (-gaol_half_pi_dn) and
    nr = x + gaol_half_pi_up, and the division by an interval above 0 divides a
    stored bound above 0 by gaol_pi_dn and one below 0 by gaol_pi_up. A stored bound 0
    stays 0, with a sign that may differ from the one of the operations of
    intervals (+0 or -0 for the lower bound, whose stored value the builds set
    each their own way); tan() only takes floor() and ceil() of these bounds
    and compares them, for which -0 and +0 are the same.
  */
  static inline double lower_of_x_plus_half_pi_over_pi(double x)
  {
    const double nl = (-x) + (-gaol_half_pi_dn);
    return -(nl / ((nl > 0.0) ? gaol_pi_dn : gaol_pi_up));
  }

  static inline double upper_of_x_plus_half_pi_over_pi(double x)
  {
    // x + gaol_half_pi_up rounded upward is next to x or above it, +oo for the
    // largest double only, which is not computed: the sum raised the overflow
    // exception for a finite tangent (GAOL v5, point 79 of TODO.md:
    // tan([DBL_MAX]))
    const double nr = (x == (std::numeric_limits<double>::max)()) ? GAOL_INFINITY : read_in_branch(x) + gaol_half_pi_up;
    return nr / ((nr > 0.0) ? gaol_pi_dn : gaol_pi_up);
  }

  interval tan(const interval& I)
  {
    // Empty set?
    if (I.is_empty()) {
      return interval::emptyset();
    }

    // The only check of the rounding direction (GAOL v5): the width and the
    // quotients below are computed on doubles, where tan() called width(), +
    // and / on intervals, which checked it five times more
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    /*
      The width of I, rounded upward as width() does: the exact width is at
      most w, and w <= gaol_pi_dn, the double below pi, proves it below pi, so that
      I holds at most one pole (GAOL v5, review #8 of examples/examples.md: the
      test was !(w < gaol_pi_up) in GAOL 4, and [-M_PI_2, M_PI_2], of width gaol_pi_dn, gave
      [-oo, +oo], as did the intervals whose exact width lies between the
      double below gaol_pi_dn and gaol_pi_dn, which round up to it).
      Above gaol_pi_dn, I may hold two poles, the cosine having the same sign
      at both bounds: [-oo, +oo], the tightest bound but for the intervals
      holding no pole whose exact width is below pi. Also for a NaN width, from
      [+oo, +oo] or [-oo, -oo] built from SSE2 registers, which the
      constructors refuse: GAOL gave [-oo, +oo] for them too
    */
    const double w = r - l;
    if (!(w <= gaol_pi_dn)) {
      GAOL_RND_LEAVE();
      return interval::universe();
    }

    /*
      The poles of tan are the x for which (x + pi/2)/pi is an integer, which
      no double is. A and B enclose that quotient at the bounds of I: there is
      no pole within I when the lower bound of A and the upper bound of B have
      the same integer part, and there is one when an integer lies between the
      upper bound of A and the lower bound of B. When the quotients cannot
      tell, a bound of I being within about |x| 2^-52 of a pole, or beyond
      2^52, the signs of the cosine at the bounds do (GAOL v5, issue #6,
      see cos_or_sin()): I being narrower than pi, as w <= gaol_pi_dn shows, there
      is a pole within I exactly when they differ. GAOL gave [-oo, +oo] then,
      as for the double below pi/2, whose tangent is 0x1.9153d9443ed0bp+51, and
      for every interval beyond 2^52.
    */
    // l and r are finite, w being at most gaol_pi_dn
    const double A_left = lower_of_x_plus_half_pi_over_pi(l), A_right = upper_of_x_plus_half_pi_over_pi(l),
      B_left = lower_of_x_plus_half_pi_over_pi(r), B_right = upper_of_x_plus_half_pi_over_pi(r);
    bool no_pole = (std::floor(A_left) == std::floor(B_right));
    const bool told = no_pole || (std::ceil(A_right) <= std::floor(B_left));
    if (told && !no_pole) { // A pole within I for sure
      GAOL_RND_LEAVE();
      return interval::universe();
    }
    // The rounding direction is upward already, set at the top of tan()
    if (!told) {
      no_pole = (sign_of_cos(l) == sign_of_cos(r));
    }
    double u = -GAOL_INFINITY, v = GAOL_INFINITY;
    if (no_pole) {
      u = tan_lo(l);
      v = tan_hi(r);
    }
    GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
    GAOL_RND_LEAVE();
    return interval(u,v);
  }


  interval acos(const interval& I)
  {
    // The check before the intersection, whose bounds, held in registers,
    // acos_lo() and acos_hi() compare (see asinpi())
    GAOL_RND_ENTER();
    interval J = I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }

    double l = acos_lo(J.right()), r = acos_hi(J.left());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval asin(const interval& I)
  {
    GAOL_RND_ENTER();
    interval J= I & interval::minus_one_plus_one();
    // J <- I \cap [-1,1]

    if (J.is_empty()) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    double l = asin_lo(J.left()), r = asin_hi(J.right());
    GAOL_RND_KEEP(l); GAOL_RND_KEEP(r);
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval atan(const interval& I)
  {
	if (I.is_empty()) {
	  return I;
	}
    GAOL_RND_ENTER();
    const double l = atan_lo(I.left()), r = atan_hi(I.right());
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  /*
    atan2 of IEEE 1788-2015 (Table 9.1), defined on the plane but (0, 0), with
    values in (-pi, pi] (GAOL v5; GAOL raised unavailable_feature_error).
    The angle of a point of the box Y x X is monotonic in y and in x in each
    quadrant, and its extrema are at corners of the box, which the signs of the
    bounds give. It jumps from pi to -pi across the half-line y = 0, x < 0: a
    box with points on that half-line and points below it has angles next to
    -pi and the angle pi, and [-pi, pi] is the hull of its angles.
  */
  // atan2() after its check, made before the bounds are compared: with
  // denormals-are-zero, atan2([100 2^-1074], [100 2^-1074]) was the empty
  // set, the point taken for (0, 0) (GAOL v5, see gaol/gaol_fpu.h)
  static interval atan2_upward(const interval& Y, const interval& X)
  {
    if (Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    if (yl == 0.0 && yu == 0.0 && xl == 0.0 && xu == 0.0) {
      return interval::emptyset();
    }
    if (yl < 0.0 && yu >= 0.0 && xl < 0.0) {
      return interval(-gaol_pi_up, gaol_pi_up);
    }
    double l, r;
    if (yl >= 0.0) { // Upper half-plane: the angle decreases with x
      // The least angle is at the right of the box, at the bottom if x > 0
      // there and at the top otherwise, and the greatest at the left. The box
      // {0} x [xl, 0] has no other corner there than (0, 0): its angle is pi,
      // and that of {0} x [0, xu] is 0
      l = atan2_lo((xu > 0.0) ? yl : yu, xu);
      r = (xl == 0.0 && yu == 0.0) ? 0.0 : atan2_hi((xl >= 0.0) ? yu : yl, xl);
    } else if (yu < 0.0) { // Lower half-plane, y = 0 left out: the angle increases with x
      l = atan2_lo((xl >= 0.0) ? yl : yu, xl);
      r = atan2_hi((xu > 0.0) ? yu : yl, xu);
    } else { // yl < 0 <= yu, in the right half-plane
      l = atan2_lo(yl, xl);
      r = (yu == 0.0) ? ((xu == 0.0) ? -gaol_half_pi_dn : 0.0) : atan2_hi(yu, xl);
    }
    return interval(l,r);
  }

  interval atan2(const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = atan2_upward(Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The relational atan2, atan2Rev1 and atan2Rev2 of IEEE 1788-2015 (GAOL v5).
    The points of the box Y x X whose angle is in C, (0, 0) left out, are
    taken quadrant by quadrant: the angles of C within (-pi, pi] are cut at
    the axes into the pieces [0, pi/2], [pi/2, pi], (-pi, -pi/2] and
    [-pi/2, 0], each the angles of a convex cone of a closed quadrant, whose
    intersection with the box is convex. A symmetry takes each quadrant to
    the first one, (u, v) being (x, y), (-x, y), (-x, -y) or (x, -y), and the
    angle theta to phi = theta, pi - theta, theta + pi or -theta in
    [0, pi/2]. The cone of a piece [phi_lo, phi_hi] is then
      { (u, v) | u, v >= 0, u tan(phi_lo) <= v <= u tan(phi_hi) },
    whose slopes are, up to their signs, tan(theta) at the bounds theta of C,
    computed by CORE-MATH from theta itself, correctly rounded, without
    rounding pi - theta nor theta + pi first; the ends of the pieces at the
    axes have the slopes 0 and +oo exactly. Rounding the lower slope downward
    and the upper one upward widens the cone, which keeps every point of it.
    The bounds of C are compared with the doubles around pi/2 and pi, which
    are no doubles: theta <= pi/2 is theta <= gaol_half_pi_dn.
  */

  // [a, b], or [-b, -a] when negated, a zero bound being +0 (the opposite of
  // a zero bound, and the upper bound -0 of X or Y, gave -0)
  static inline interval signed_interval(bool negated, double a, double b)
  {
    const double l = negated ? -b : a, u = negated ? -a : b;
    return interval((l == 0.0) ? 0.0 : l, (u == 0.0) ? 0.0 : u);
  }

  /*
    The points (u, v) of [ul, uu] x [vl, vu], in the first quadrant, with
    t_lo u <= v <= t_hi u, t_lo being 0 or positive, and finite, t_hi being
    0, positive or +oo; their u, with x = u or -u (x_negated), are added to
    the hull Xr, and their v, with y = v or -v, to Yr. By elimination of v,
    their u are those with vl / t_hi <= u <= vu / t_lo, and by elimination of
    u, their v those with ul t_lo <= v <= uu t_hi, the products and quotients
    rounded outward. At the slopes 0 and +oo these are the limits the cone
    has, the slope +oo leaving the v of u = 0 free, and the slope 0 the u of
    v = 0. The point (0, 0) is left out: a piece holding no other point adds
    nothing, nor does a piece with no point off v = 0 when without_v_zero (the
    half-line y = 0, x < 0, whose angle is pi, out of the third quadrant).
    Neither projection is added when either is empty: the box and the cone do
    not meet.
    t_hi_above tells that t_hi is a subnormal angle theta itself, its
    tangent being above it by less than theta^2/2 < 2^-2044 of it (see
    upper_slope()): a product or a quotient with theta is then the tightest
    bound with the tangent where it is inexact, its rounding moving it by more
    than that, and is moved one double outward where it is exact.
  */
  static void atan2_rel_piece(double ul, double uu, double vl, double vu, double t_lo, double t_hi, bool t_hi_above,
                              bool without_v_zero, bool x_negated, bool y_negated, interval& Xr, interval& Yr)
  {
    if (t_hi == 0.0 && vl > 0.0) { // The half-line v = 0 alone, which the v of V do not reach
      return;
    }
    // The slopes read in the branches that divide or multiply by them: 0/0,
    // +oo/+oo, +oo * 0 and v/0 are left out (see read_in_branch())
    // The quotients and the products of finite bounds, which are intersected
    // with the box below, are those of the saturating operations: a slope next
    // to 2^-1074 or to 2^54, with a bound next to the largest double, overflows
    // for a finite result (GAOL v5, point 79 of TODO.md)
    double u_lo = (t_hi == GAOL_INFINITY || vl == 0.0) ? 0.0 : quotient_dn_sat(vl, t_hi);
    const double u_hi = (t_lo == 0.0) ? GAOL_INFINITY
      : ((vu == 0.0) ? 0.0 : ((vu == GAOL_INFINITY) ? GAOL_INFINITY : quotient_up_sat(vu, t_lo)));
    const double v_lo = (ul == 0.0 || t_lo == 0.0) ? 0.0 : product_dn_sat(ul, t_lo);
    double v_hi = (t_hi == GAOL_INFINITY) ? GAOL_INFINITY
      : ((uu == 0.0 || t_hi == 0.0) ? 0.0 : ((uu == GAOL_INFINITY) ? GAOL_INFINITY : product_up_sat(uu, t_hi)));
    if (t_hi_above) {
      if (u_lo > 0.0 && fma_once(u_lo, read_in_branch(t_hi), -vl) == 0.0) {
        u_lo = previous_float(u_lo);
      }
      if (v_hi > 0.0 && v_hi < GAOL_INFINITY && fma_once(uu, read_in_branch(t_hi), -v_hi) == 0.0) {
        v_hi = next_float(v_hi);
      }
    }
    const double a = maximum(ul, u_lo), b = minimum(uu, u_hi), c = maximum(vl, v_lo), d = minimum(vu, v_hi);
    if (a > b || c > d || (b == 0.0 && d == 0.0) || (without_v_zero && d == 0.0)) {
      return;
    }
    Xr |= signed_interval(x_negated, a, b);
    Yr |= signed_interval(y_negated, c, d);
  }

  /*
    The tangent of theta in [0, pi/2) rounded upward, the upper slope of a
    cone, or theta itself for a subnormal theta, above being set: the
    tangent rounded upward is then the next double, which can be twice theta,
    when tan(theta) is above theta by theta^3/3 only (see atan2_rel_piece())
  */
  static inline double upper_slope(double theta, bool& above)
  {
    above = theta > 0.0 && theta < std::numeric_limits<double>::min();
    return above ? theta : tan_hi(theta);
  }

  // The hulls Yr and Xr of the ordinates and of the abscissas, computed after
  // the check of the rounding direction, before which no bound is compared
  // (see atan2())
  static void atan2_rel_upward(const interval& C, const interval& Y, const interval& X, interval& Yr, interval& Xr)
  {
    Yr = Xr = interval::emptyset();
    if (C.is_empty() || Y.is_empty() || X.is_empty()) {
      return;
    }
    const double cl = C.left(), cu = C.right(), yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    // The parts of X and Y on either side of the axes, as magnitudes: X & [0, +oo]
    // is [xp_l, xu] and -X & [0, +oo] is [xn_l, -xl] when not empty
    const double xp_l = (xl > 0.0) ? xl : 0.0, xn_l = (xu < 0.0) ? -xu : 0.0;
    const double yp_l = (yl > 0.0) ? yl : 0.0, yn_l = (yu < 0.0) ? -yu : 0.0;
    // [0, pi/2]: (u, v) = (x, y), phi = theta
    if (xu >= 0.0 && yu >= 0.0 && cu >= 0.0 && cl <= gaol_half_pi_dn) {
      bool above = false;
      const double t_lo = (cl <= 0.0) ? 0.0 : tan_lo(cl);
      const double t_hi = (cu >= gaol_half_pi_up) ? GAOL_INFINITY : upper_slope(cu, above);
      atan2_rel_piece(xp_l, xu, yp_l, yu, t_lo, t_hi, above, false, false, false, Xr, Yr);
    }
    // [pi/2, pi]: (u, v) = (-x, y), phi = pi - theta, whose slope is -tan(theta)
    if (xl <= 0.0 && yu >= 0.0 && cu >= gaol_half_pi_up && cl <= gaol_pi_dn) {
      const double t_lo = (cu >= gaol_pi_up) ? 0.0 : -tan_hi(cu);
      const double t_hi = (cl <= gaol_half_pi_dn) ? GAOL_INFINITY : -tan_lo(cl);
      atan2_rel_piece(xn_l, -xl, yp_l, yu, t_lo, t_hi, false, false, true, false, Xr, Yr);
    }
    // (-pi, -pi/2]: (u, v) = (-x, -y), phi = theta + pi, whose slope is
    // tan(theta); -pi is no angle, the half-line y = 0, x < 0 having the angle pi
    if (xl <= 0.0 && yl <= 0.0 && cu >= -gaol_pi_dn && cl <= -gaol_half_pi_up) {
      const double t_lo = (cl <= -gaol_pi_up) ? 0.0 : tan_lo(cl);
      const double t_hi = (cu >= -gaol_half_pi_dn) ? GAOL_INFINITY : tan_hi(cu);
      atan2_rel_piece(xn_l, -xl, yn_l, -yl, t_lo, t_hi, false, true, true, true, Xr, Yr);
    }
    // [-pi/2, 0]: (u, v) = (x, -y), phi = -theta, whose slope is -tan(theta),
    // tan(-theta)
    if (xu >= 0.0 && yl <= 0.0 && cu >= -gaol_half_pi_dn && cl <= 0.0) {
      bool above = false;
      const double t_lo = (cu >= 0.0) ? 0.0 : -tan_hi(cu);
      const double t_hi = (cl <= -gaol_half_pi_up) ? GAOL_INFINITY : upper_slope(-cl, above);
      atan2_rel_piece(xp_l, xu, yn_l, -yl, t_lo, t_hi, above, false, false, true, Xr, Yr);
    }
  }

  interval atan2_rel_y(const interval& C, const interval& X, const interval& Y)
  {
    GAOL_RND_ENTER();
    interval Yr, Xr;
    atan2_rel_upward(C, Y, X, Yr, Xr);
    GAOL_RND_KEEP(Yr);
    GAOL_RND_LEAVE();
    return Yr;
  }

  interval atan2_rel_x(const interval& C, const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval Yr, Xr;
    atan2_rel_upward(C, Y, X, Yr, Xr);
    GAOL_RND_KEEP(Xr);
    GAOL_RND_LEAVE();
    return Xr;
  }

  interval cosh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
    double l, r;
    GAOL_RND_ENTER();
    if (I.right() < 0) {
      l = cosh_lo(I.right());
      r = cosh_hi(I.left());
    } else if (I.left() > 0) {
      l = cosh_lo(I.left());
      r = cosh_hi(I.right());
    } else { // 0 \in I
      const double abs_l = -I.left();
      l = 1.0;
      r = cosh_hi((abs_l >= I.right()) ? abs_l : I.right());
    }
    GAOL_RND_LEAVE();
    return interval(l,r);
  }

  interval sinh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	GAOL_RND_ENTER();
	const double l = sinh_lo(I.left()), r = sinh_hi(I.right());
	GAOL_RND_LEAVE();
	return interval(l,r);
  }

  interval tanh(const interval& I)
  {
	if (I.is_empty()) {
		return I;
	}
	GAOL_RND_ENTER();
	const double l = tanh_lo(I.left()), r = tanh_hi(I.right());
	GAOL_RND_LEAVE();
	return interval(l,r) & interval::minus_one_plus_one();
  }


  /*
    The bodies of acosh(), asinh() and atanh() after their check, for these
    functions and for acosh_rel(), asinh_rel() and atanh_rel(), which check
    the direction before them (GAOL v5: the reverse functions called acosh(),
    asinh() and atanh(), which checked it a second time)
  */
  static interval acosh_upward(const interval& I)
  {
    const interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      return J;
    }
    const double l = acosh_lo(J.left()), r = acosh_hi(J.right());
    return interval(l,r);
  }

  static interval asinh_upward(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }
    const double l = asinh_lo(I.left()), r = asinh_hi(I.right());
    return interval(l,r);
  }

  // by_division: the infinite bounds at -1 and 1 are those of CORE-MATH, which
  // raises the division-by-zero exception for them; atanh_rel() takes them
  // without a division, its result being a finite interval where I is (GAOL
  // v5, point 79 of TODO.md: atanh_rel([0, 1], [0, 1]) raised it)
  static interval atanh_upward(const interval& I, bool by_division = true)
  {
    const interval J = I & interval::minus_one_plus_one();
    // atanh is defined on (-1, 1) (IEEE 1788-2015, Table 9.1, GAOL v5): an I
    // meeting [-1, 1] at 1 alone, or at -1 alone, as [1] and [-5,-1], holds
    // no point of it, and gives the empty set. The value of CORE-MATH at 1 is
    // +oo, and the lower bound the double below the value: [1] was [MAX,+oo]
    // ([-1] was empty only because interval(-oo,-oo) is). The limits -oo at
    // -1 and +oo at 1 remain the bounds when I holds other points of (-1, 1)
    if (J.is_empty() || J.left() == 1.0 || J.right() == -1.0) {
      return interval::emptyset();
    }
    const double l = (!by_division && J.left() == -1.0) ? -GAOL_INFINITY : atanh_lo(J.left());
    const double r = (!by_division && J.right() == 1.0) ? GAOL_INFINITY : atanh_hi(J.right());
    return interval(l,r);
  }

  interval acosh(const interval& I)
  {
    // The check before the intersection, as in acos()
    GAOL_RND_ENTER();
    interval res = acosh_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }



  interval asinh(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    GAOL_RND_ENTER();
    interval res = asinh_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;

  }

  interval atanh(const interval& I)
  {
    // The check before the intersection, as in acos()
    GAOL_RND_ENTER();
    interval res = atanh_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The error function and its complement (GAOL v5)

    erf(x) = 2/sqrt(pi) int_0^x exp(-t^2) dt and erfc(x) = 1 - erf(x), the
    erf and erfc of C, which IEEE 1788-2015 does not name, defined on R: erf
    is increasing, from its limit -1 at -oo to its limit 1 at +oo, and erfc
    decreasing, from 2 to 0. CORE-MATH computes both correctly rounded in the
    rounding direction in effect, so they are bounded as exp2 is: in the
    upward rounding GAOL keeps, the value at one bound is a bound, and the
    double below the value at the other bound the other one, unless that
    value is itself a double. erf(0) = 0 and erfc(0) = 1 are: without the
    test below, the lower bound of erf([0, 1]) would be -2^-1074, and that of
    erfc([-1, 0]) the double below 1. So are the limits at an infinite bound,
    erf(-oo) = -1, erf(+oo) = 1, erfc(-oo) = 2 and erfc(+oo) = 0, which
    CORE-MATH returns exactly, and which the range of each function would
    bring the lower bound back to as well. No other double is known where erf
    or erfc takes a value that is a double, and CORE-MATH's sources treat no
    such case: the bounds are the tightest ones.
  */
  // The points where erf and erfc take a value that is a double
  static inline bool erf_is_exact(double x)
  {
    return x == 0.0 || !is_finite(x);
  }

  interval erf(const interval& I)
  {
    // Within [-1, 1]: |erf(x)| < 1 at every finite x, and above 1 - 2^-53,
    // the double below 1, from |x| = 5.87 on, where CORE-MATH rounds erf(x)
    // upward to 1 and the lower bound is 1 - 2^-53
    return increasing_cr(I, gaol_cr_erf, erf_is_exact, -1.0, 1.0);
  }

  interval erfc(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are read, and the result made before
    // GAOL_RND_LEAVE(), as acospi() does (see gaol/gaol_fpu.h): erfc(x) is
    // subnormal from x = 26.55 on, and with GAOL_PRESERVE_ROUNDING the
    // denormals-are-zero mode it restores would compare such bounds as 0
    GAOL_RND_ENTER();
    const double l = I.left(), r = I.right();
    // Decreasing: the upper bound at the left bound, the lower one at the
    // right, within [0, 2]. erfc(x) is below 2^-1074 from x = 27.22 on, where
    // CORE-MATH rounds it upward to 2^-1074 and the lower bound is 0
    const double v = gaol_cr_erfc(l);
    const double w = gaol_cr_erfc(r);
    const double u = erf_is_exact(r) ? w : previous_float(w);
    interval res(maximum(0.0, u), minimum(2.0, v));
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    k pi + X, k being an integer double and X a bounded interval, enclosed
    within about one double (GAOL v5, issue #6): pi = pi_hi + pi_lo,
    pi_hi being the double below pi, and pi_lo lying between two consecutive
    doubles; k pi_hi is p + e exactly, p being the product rounded and e its
    rest, from fma(); k pi_lo is bounded by the products with the two
    doubles, rounded outward. The bounds of X are added to e + k pi_lo, and
    the sums to p, rounded once at the magnitude of the result: adding X to
    k pi rounded would round twice, and the bounds could be two doubles from
    the tightest. GAOL computed k [gaol_pi_dn, gaol_pi_up] + X, whose width, k 2^-51,
    the relational functions took on: their bounds were up to
    2^-49 max(1, |x|) from the values they had to keep. The rounding
    direction is upward.
  */
  static interval k_pi_plus(double k, const interval& X)
  {
    // 0x1.1a62633145c06p-53 and 0x1.1a62633145c07p-53, the doubles around
    // pi - gaol_pi_dn = 1.2246467991473531772e-16
    static const double pi_lo_dn = std::ldexp(4967757600021510.0,-105);
    static const double pi_lo_up = std::ldexp(4967757600021511.0,-105);
    const double p = k*gaol_pi_dn;
    const double e = fma_once(k,gaol_pi_dn,-p);
    // The products of k with the doubles around pi_lo, rounded outward
    const double below = (k >= 0.0) ? pi_lo_dn : pi_lo_up, above = (k >= 0.0) ? pi_lo_up : pi_lo_dn;
    const double lo_lo = -((-k)*below), lo_hi = k*above;
    const double hi = p + ((e + lo_hi) + X.right());
    const double lo = -((-p) + (((-e) - lo_lo) - X.left()));
    return interval(lo,hi);
  }

  /*
    The bounds of interval(x)/interval::pi() + shift, for a finite x of
    magnitude at most 2^52, rounded upward: the doubles operator/=(interval)
    and operator+=(double) give, in the two builds, computed on doubles. The
    division of interval(x), whose stored bounds are -x and x, by
    [gaol_pi_dn, gaol_pi_up] divides a stored bound above 0 by gaol_pi_dn and one below 0 by
    gaol_pi_up, and gives interval::zero() for x = 0. The stored bounds of
    interval::zero() are -0 and +0 in the FPU build, +0 and +0 in the SSE2
    build, and the sign of the zero reaches the bounds of acos_rel() through
    k_pi_plus(): they are taken from interval::zero(), -left() being the
    stored lower bound, rather than written 0.
  */
  static inline double lower_of_x_over_pi_plus(double x, double shift)
  {
    const double nl = (x == 0.0) ? -interval::zero().left() : (-x) / ((x < 0.0) ? gaol_pi_dn : gaol_pi_up);
    return -(nl - shift);
  }

  static inline double upper_of_x_over_pi_plus(double x, double shift)
  {
    const double nr = (x == 0.0) ? interval::zero().right() : x / ((x > 0.0) ? gaol_pi_dn : gaol_pi_up);
    return nr + shift;
  }

  /*
    The hull of the x of I whose image by a periodic function is in J (the
    relational acos_rel(), asin_rel() and atan_rel()), the preimage of J
    being the union of the pieces piece(i), the piece i lying on
    [(i - shift) pi, (i + 1 - shift) pi], shift being 0 for the cosine and
    1/2 for the sine and the tangent (GAOL v5, issue #6: GAOL had three
    copies of this, with k [pi]).

    A bound of I lies on the piece floor(x/pi + shift), or on the next one
    when the quotient, rounded outward, is off by one: the leftmost point of
    the preimage within I is on the piece of the left bound or the next one,
    the union of the pieces being symmetric about every multiple of pi (plus
    shift pi), and the rightmost point on the piece of the right bound or the
    previous one. Beyond 2^52 the quotient may be off by more, and I is kept
    as it is on that side; an I of a single double is decided by the image of
    the function, at every magnitude.

    The rounding direction is checked once (GAOL v5): the inverse image of J
    (inverse(), with the bounds of acos_lo(), asin_lo()... and no check of
    their own) and the quotients x/pi + shift are computed after the check
    here, where the relational functions called acos(J), asin(J) or atan(J),
    and four operations of intervals, which checked it once more each.

    For acos_rel() and asin_rel(), whose function has its image in [-1, 1]
    (bounded), a J outside [-1, 1] has no preimage, and a J containing it has
    the whole line: these are decided after the check too, though the modes
    flushing the subnormals to zero do not change how J compares with -1 and
    1, so that the check, which clears the modes, is made on every path of a
    non-empty J and I (GAOL v5, second review of point 4).
  */
  template<class Inverse, class Piece, class Image>
  static interval periodic_rel(const interval& J, const interval& I, double shift, bool bounded, Inverse inverse,
                               Piece piece, Image image)
  {
    if (J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are compared (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    if (bounded && (J.left() > 1.0 || J.right() < -1.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    if (bounded && J.set_contains(interval::minus_one_plus_one())) {
      GAOL_RND_LEAVE();
      return I;
    }
    if (I.left() == I.right()) {
      interval K = image(I) & J;
      GAOL_RND_KEEP(K);
      GAOL_RND_LEAVE();
      return K.is_empty() ? interval::emptyset() : I;
    }
    const interval Jinv = inverse(J);
    interval Ileft, Iright;
    if (std::fabs(I.left()) > two_power_52) {
      Ileft = I;
    } else {
      const double kl = std::floor(lower_of_x_over_pi_plus(I.left(), shift));
      Ileft = piece(kl, Jinv) & I;
      if (Ileft.is_empty()) {
	Ileft = piece(kl + 1.0, Jinv) & I;
      }
    }
    if (std::fabs(I.right()) > two_power_52) {
      Iright = I;
    } else {
      const double kr = std::floor(upper_of_x_over_pi_plus(I.right(), shift));
      Iright = piece(kr, Jinv) & I;
      if (Iright.is_empty()) {
	Iright = piece(kr - 1.0, Jinv) & I;
      }
    }
    GAOL_RND_KEEP(Ileft); GAOL_RND_KEEP(Iright);
    GAOL_RND_LEAVE();
    if (Ileft.is_empty() || Iright.is_empty()) {
      return interval::emptyset();
    }
    return interval(Ileft.left(),Iright.right());
  }

  interval acos_rel(const interval& J, const interval &I)
  {
    // The preimage of J: i pi + acos(J) for an even i, (i + 1) pi - acos(J)
    // for an odd i. acos(J) as acos() computes it, J & [-1, 1] being non-empty
    return periodic_rel(J, I, 0.0, true,
			[](const interval& X) {
			  const interval K = X & interval::minus_one_plus_one();
			  return interval(acos_lo(K.right()), acos_hi(K.left()));
			},
			[](double i, const interval& Jacos) { return integer_is_even(i) ? k_pi_plus(i, Jacos) : k_pi_plus(i + 1.0, -Jacos); },
			[](const interval& X) { return cos(X); });
  }

  interval asin_rel(const interval& J, const interval &I)
  {
    // The preimage of J: i pi + asin(J) for an even i, i pi - asin(J) for an
    // odd i (GAOL v5: GAOL computed pi/2 + acos_rel(J, I - pi/2), two
    // additions of an enclosure of pi/2 more). asin(J) as asin() computes it,
    // J & [-1, 1] being non-empty
    return periodic_rel(J, I, 0.5, true,
			[](const interval& X) {
			  const interval K = X & interval::minus_one_plus_one();
			  return interval(asin_lo(K.left()), asin_hi(K.right()));
			},
			[](double i, const interval& Jasin) { return k_pi_plus(i, integer_is_even(i) ? Jasin : -Jasin); },
			[](const interval& X) { return sin(X); });
  }

  interval atan_rel(const interval& J, const interval& I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    // The preimage of J: i pi + atan(J), atan(J) as atan() computes it
    return periodic_rel(J, I, 0.5, false,
			[](const interval& X) { return interval(atan_lo(X.left()), atan_hi(X.right())); },
			[](double i, const interval& Jatan) { return k_pi_plus(i, Jatan); },
			[](const interval& X) { return tan(X); });
  }

  /*
    acosh_rel(), asinh_rel() and atanh_rel() check the rounding direction
    before they compare the bounds of the inverse image with those of I, which
    acosh(), asinh() and atanh() returned with the modes that flush the
    subnormals to zero restored, with GAOL_PRESERVE_ROUNDING (GAOL v5, see
    gaol/gaol_fpu.h). The inverse images are those of the bodies of these
    functions, which do not check the direction again (GAOL v5)
  */
  static interval acosh_rel_upward(const interval &J, const interval &I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }

    interval tmp = acosh_upward(J);

    if (I.certainly_positive()) {
      return I & tmp;
    }
    if (I.certainly_negative()) {
      return I & (-tmp);
    }
    return (I & -tmp) | (I & tmp);
  }

  interval acosh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = acosh_rel_upward(J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval asinh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = asinh_upward(J) & I;
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval atanh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = atanh_upward(J, false) & I;
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
   * abs
   */
  interval abs(const interval& I)
  {
    if (I.is_empty()) {
      return I;
    }

    if (I.certainly_positive()) {
      return I;
    }
    if (I.certainly_negative()) {
      return interval(-I.right(),-I.left());
    }
    return interval(0.0,maximum(-I.left(),I.right()));
  }

  /*
   * invabs_rel: the hull of the x of I whose magnitude is in J, absRev of
   * IEEE 1788, the negative part of J holding no magnitude (GAOL v5:
   * GAOL took it as one, invabs_rel([-2, -1], I) being [-2, 2] & I instead of
   * the empty set, and invabs_rel([-2, 0], I) [-2, 2] & I instead of [0])
   */
  interval invabs_rel(const interval &Jall, const interval &I)
  {
    const interval J = Jall & interval::positive();
    if (J.is_empty()) {
      return interval::emptyset();
    }
    if (I.certainly_geq(interval::zero())) {
      return (J & I);
    }

    if (I.certainly_leq(interval::zero())) {
      return ((-J) & I);
    }
    return (J&I) | ((-J)&I);
  }


  double chi(const interval &I)
  {
    // NaN for the empty set, as width() and mig() give (GAOL v5): the
    // quotient of its NaN bounds had the sign the generated code left it,
    // -nan with GCC on x86 and nan with Clang
    if (I.is_empty()) {
      return GAOL_NAN;
    }
    if (I.is_zero()) {
      return -1.0;
    } else {
      if (!I.is_finite()) {
        if (I.set_eq(interval::universe())) {
          return 1.0;
        } else {
          return 0.0;
        }
      } else {
	double res;
	// The check first, which clears the modes that flush the subnormals to
	// zero, as in midpoint(): under denormals-are-zero, the quotient of two
	// subnormal bounds is 0/0 (GAOL v5, point Q of TODO.md). Before,
	// is_zero() took them for zeros, and chi([2, 4]*2^-1074) was -1
	GAOL_RND_PRESERVE();
	round_upward_if_needed();
	round_nearest();
	// A quiet comparison: the empty set, whose bounds are NaN, gives NaN
	// without the invalid-operation exception (GAOL v5)
	if (gaol_detail::quiet_less_equal(std::fabs(I.left()), std::fabs(I.right()))) {
	  res = I.left() / I.right();
	} else {
	  res = I.right() / I.left();
	}
	// Computed rounding to nearest: kept before the direction changes (see gaol_fpu.h)
	res = gaol_core::rnd_keep(res);
	GAOL_RND_RESTORE();
	return res;
      }
    }
  }

  bool interval::is_finite(void) const
  {
    return !std::isinf(left()) && !std::isinf(right());
  }

  /*
    maximum() and minimum() give NaN bounds for an empty I or J, which give
    the empty set: the constructor tells them with a quiet comparison (GAOL
    v5). They were told first, with one more comparison, when the constructor
    compared them with <=, which raises the invalid-operation exception on a
    NaN; testing I and J first made max() 5 to 10% slower (Clang 18).
  */
  interval  max(const interval &I, const interval &J)
  {
    return interval(maximum(I.left(),J.left()), maximum(I.right(),J.right()));
  }

  interval  min(const interval &I, const interval &J)
  {
    return interval(minimum(I.left(),J.left()), minimum(I.right(),J.right()));
  }

  /*
    The reverse functions of max, min, sign and floor (GAOL v5, point P.25 of
    TODO.md), which IBEX wrote itself: each returns the hull of the x of X
    that the function, given the other operand Y for max and min, sends into
    Z, the natural reverse extension of IEEE 1788-2015 (10.5.4, (5) and (6)),
    which names none of them. The sets are intervals, or unions of pieces,
    whose ends are bounds of X and Z, 0 and integers: no bound is rounded but
    the end floor(sup Z) + 1 of floor_rel(), and each result is the tightest
    enclosure of its set.

    Each makes the check of the rounding direction before it compares a bound,
    as the relational functions above, though only floor_rel() computes: the
    check clears the modes that flush the subnormal numbers to zero, under
    which a subnormal bound compares as a zero, and
    sign_rel([1], [-2^-1074, 2^-1074]), whose upper bound is then no longer
    above 0, would be empty rather than [0, 2^-1074] (see gaol/gaol_fpu.h).
    The bounds are compared with the quiet comparisons, as in the relations,
    and only once the empty arguments, whose bounds are NaN, are put aside.
  */

  // max_rel() after its check. max(x, y) is in Z if and only if x and y are
  // at most sup Z and one of them is at least inf Z.
  static interval max_rel_upward(const interval& Z, const interval& Y, const interval& X)
  {
    if (Z.is_empty() || Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    // Every y above Z: max(x, y) >= y > sup Z
    if (gaol_detail::quiet_greater(Y.left(), Z.right())) {
      return interval::emptyset();
    }
    // A y of Y in Z: max(x, y) is y for x <= y and x for y < x <= sup Z, in Z
    // both, so that every x up to sup Z is kept
    if (gaol_detail::quiet_greater_equal(Y.right(), Z.left())) {
      return X & interval(-GAOL_INFINITY, Z.right());
    }
    // Every y below Z: max(x, y) has to be x, in Z
    return X & Z;
  }

  interval max_rel(const interval& Z, const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = max_rel_upward(Z, Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // min_rel() after its check, max_rel() turned upside down: min(x, y) is in Z
  // if and only if x and y are at least inf Z and one of them is at most sup Z
  static interval min_rel_upward(const interval& Z, const interval& Y, const interval& X)
  {
    if (Z.is_empty() || Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    // Every y below Z
    if (gaol_detail::quiet_less(Y.right(), Z.left())) {
      return interval::emptyset();
    }
    // A y of Y in Z: every x from inf Z on is kept
    if (gaol_detail::quiet_less_equal(Y.left(), Z.right())) {
      return X & interval(Z.left(), GAOL_INFINITY);
    }
    // Every y above Z: min(x, y) has to be x, in Z
    return X & Z;
  }

  interval min_rel(const interval& Z, const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = min_rel_upward(Z, Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    sign_rel() after its check. The x of X whose sign is in Z are the union of
    the negative x of X if Z holds -1, of 0 if Z holds 0 and X does, and of the
    positive x of X if Z holds 1; Z holding none of them gives the empty set
    (Z = [0.5, 0.7]). The negative x of X, where X has some, are
    [inf X, min(sup X, 0)), open at 0 when X goes on to 0 or beyond: their
    tightest closed enclosure ends at 0 all the same, every negative double
    next to 0 being in the set (sign_rel([-1], [-2, 3]) is [-2, 0], and
    sign_rel([-1], [0, 3]) is empty, sign(0) being 0). The positive x are
    their mirror image. Z cannot hold -1 and 1 without 0, being an interval.
  */
  static interval sign_rel_upward(const interval& Z, const interval& X)
  {
    if (Z.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double zl = Z.left(), zu = Z.right(), xl = X.left(), xu = X.right();
    const bool negative = gaol_detail::quiet_less_equal(zl, -1.0) && gaol_detail::quiet_less_equal(-1.0, zu)
      && gaol_detail::quiet_less(xl, 0.0);
    const bool zero = gaol_detail::quiet_less_equal(zl, 0.0) && gaol_detail::quiet_less_equal(0.0, zu)
      && gaol_detail::quiet_less_equal(xl, 0.0) && gaol_detail::quiet_less_equal(0.0, xu);
    const bool positive = gaol_detail::quiet_less_equal(zl, 1.0) && gaol_detail::quiet_less_equal(1.0, zu)
      && gaol_detail::quiet_greater(xu, 0.0);
    if (!negative && !zero && !positive) {
      return interval::emptyset();
    }
    // The lower bound of the first piece, the upper bound of the last one
    const double l = negative ? xl : ((zero || gaol_detail::quiet_less_equal(xl, 0.0)) ? 0.0 : xl);
    const double u = positive ? xu : ((zero || gaol_detail::quiet_greater_equal(xu, 0.0)) ? 0.0 : xu);
    return interval(l, u);
  }

  interval sign_rel(const interval& Z, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = sign_rel_upward(Z, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    floor_rel() after its check. floor(x) is the integer n for x in [n, n + 1):
    the x whose floor is in Z make [ceil(inf Z), floor(sup Z) + 1), and Z
    holding no integer (ceil(inf Z) > floor(sup Z), as for Z = [0.5, 0.7])
    gives the empty set. That set is open at its upper end e = floor(sup Z) + 1:
    a closed interval containing it contains e as well, the reals just below e
    being in it, so that the tightest closed enclosure of its part in X ends at
    min(sup X, e), and an X that starts at e or above holds none of it
    (floor_rel([0], [1, 2]) is empty, where [0, 1] & [1, 2] would be [1]).
    e is rounded upward. Where it is no double, beyond 2^53 in magnitude,
    every double is an integer and none lies between floor(sup Z) and e: the
    rounded e is the double above floor(sup Z), the smallest double at least
    e, the tightest bound, and a double is below e if and only if it is below
    the rounded e, so that inf X < e and min(sup X, e) are decided on it
    exactly. The rounded e is +oo for sup Z = DBL_MAX, the tightest bound of
    [DBL_MAX, DBL_MAX + 1), and for sup Z = +oo: it is taken as +oo there
    rather than computed, DBL_MAX + 1 raising the overflow exception even where
    the result has no infinite bound (floor_rel([DBL_MAX], [0, 1]) is empty),
    and the overflow flag GAOL raises stands for an infinite bound of its
    result only (doc/using.md). The sum reads floor(sup Z) in the branch that
    tests it (read_in_branch()), so that no compiler adds 1 before the test:
    Clang 18 and 21 for 64-bit ARM, at -O3 with -frounding-math, and
    AppleClang computed floor(sup Z) + 1 for every Z, then chose between it
    and +oo with fcsel, and DBL_MAX + 1 raised the overflow flag (GAOL v5,
    tests/rounding_direction.cpp).
  */
  static interval floor_rel_upward(const interval& Z, const interval& X)
  {
    if (Z.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double first = std::ceil(Z.left()), last = std::floor(Z.right());
    if (gaol_detail::quiet_greater(first, last)) {
      return interval::emptyset();
    }
    double end = GAOL_INFINITY;
    if (gaol_detail::quiet_less(last, (std::numeric_limits<double>::max)())) {
      end = read_in_branch(last) + 1.0; // rounded upward
    }
    const double xl = X.left(), xu = X.right();
    if (gaol_detail::quiet_greater(first, xu) || gaol_detail::quiet_greater_equal(xl, end)) {
      return interval::emptyset();
    }
    return interval(gaol_detail::quiet_greater(first, xl) ? first : xl,
                    gaol_detail::quiet_less(end, xu) ? end : xu);
  }

  interval floor_rel(const interval& Z, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = floor_rel_upward(Z, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  double interval::smig(void) const
  {
    if (is_empty()) {
      return GAOL_NAN;
    } else {
      if (set_contains(0)) {
	return 0.0;
      }
      // Compared as in the relations (gaol_port.h), whatever the modes that
      // flush the subnormals to zero (GAOL v5, point Q of TODO.md)
      if (gaol_detail::bound_less(right(), 0.0)) {
	return right();
      } else { // left() > 0.0
	return left();
      }
    }
  }

  double interval::mig(void) const
  {
    if (is_empty()) {
      return GAOL_NAN;
    } else {
      if (set_contains(0)) {
	return 0.0;
      }
      // Compared as in the relations (gaol_port.h): under denormals-are-zero,
      // mig([-3, -2]*2^-1074) was 0 (GAOL v5, point Q of TODO.md)
      if (gaol_detail::bound_less(right(), 0.0)) {
	return -right();
      } else { // left() > 0.0
	return left();
      }
    }
  }

  double interval::mag(void) const
  {
    return maximum(std::fabs(left()),std::fabs(right()));
  }


  interval::operator std::string() const
  {
    // Not named output, the format of the intervals written, which Visual C++
    // warned that it hid (C4458, GAOL v5)
    std::ostringstream text;
    text.precision(interval::precision());
    text << *this;
    return text.str();
  }

  std::streamsize interval::precision(void)
  {
    return output_precision;
  }

  std::streamsize interval::precision(std::streamsize n)
  {
    std::streamsize old = output_precision;
    output_precision = n;
    return old;
  }




  double interval::midpoint() const
  {
    if (is_empty()) {
      return NAN;
    }
	 if (is_symmetric()) {
		return 0.0;
	 }
    if (left() == -GAOL_INFINITY) {
      return -std::numeric_limits<double>::max();
    }
    if (right() == GAOL_INFINITY) {
      return std::numeric_limits<double>::max();
    }

    /*
      The half of the sum of the bounds, rounded to nearest, or the sum of
      their halves where the sum may overflow, that is where a bound is 2^1023
      or more in magnitude, which is tested first (GAOL v5): the sum was
      computed first, and its overflow told the other case, so that
      midpoint([DBL_MAX]) raised the overflow exception, though its midpoint
      is DBL_MAX. The sum of the halves gives the same midpoint there: a half
      is exact, unless the bound is below 2^-1021, where its rounding changes
      no sum with a bound of 2^1023 or more, and a sum of halves is the half
      of the sum, rounded, where no half is rounded.
      The check of the rounding direction first, which clears the modes that
      flush the subnormals to zero, and which GAOL_RND_RESTORE() sets back
      with GAOL_PRESERVE_ROUNDING: under denormals-are-zero, the midpoint of
      [2, 4]*2^-1074 was 0, outside the interval, and split() made an empty
      half of it (GAOL v5, point Q of TODO.md). The comparisons above are
      those of the relations, which these modes do not change.
    */
    GAOL_RND_PRESERVE();
    round_upward_if_needed();
    round_nearest();
    const double l = left(), r = right();
    double middle;
    const double big = 8.9884656743115795e+307; // 2^1023 (C++11 has no hexadecimal floating literal)
    if (std::fabs(l) >= big || std::fabs(r) >= big) {
      middle = 0.5*l + 0.5*r;
    } else {
      middle = 0.5*(l + r);
    }
    // Computed rounding to nearest: kept before the direction changes (see gaol_fpu.h)
    middle = gaol_core::rnd_keep(middle);
    GAOL_RND_RESTORE();
    return middle;
  }

  /*
    The cut point c of bisect(ratio) (GAOL v5). Any double of [l, r] gives two
    parts that cover [l, r] exactly, sharing c: c is computed in GAOL's
    rounding direction and put back within [l, r], where the two rounded
    products may have taken it; l (1 - ratio) + r ratio cannot overflow where
    l + ratio (r - l) does, as for [-MAX, MAX]. GAOL_RND_ENTER() clears the
    flush-to-zero and denormals-are-zero modes, so that the cut between two
    subnormal bounds is not read as 0. When a double lies strictly between l
    and r, c is moved to the nearest one where it fell on a bound, so that
    both parts are smaller than [l, r]. ratio = 0.5 and the unbounded
    intervals are cut at midpoint(), as split() cuts them, midpoint() being
    strictly between the bounds of a bisectable interval: rounded to nearest,
    it would be l only for an exact midpoint at most half the distance from
    l to the next double. The ratio is compared quietly, a NaN raising no
    invalid-operation exception before it is refused, by the comparisons of
    gaol_detail, ucomisd in line with Visual C++, where std::isgreater() and
    std::isless() are calls of _dpcomp() (GAOL v5, point Z of TODO.md).
  */
  std::pair<interval, interval> interval::bisect(double ratio) const
  {
    if (!(gaol_detail::quiet_greater(ratio, 0.0) && gaol_detail::quiet_less(ratio, 1.0))) {
      gaol_ERROR(invalid_action_error, "bisect(): the ratio is not in (0, 1)");
    }
    if (is_empty()) {
      return std::make_pair(*this, *this);
    }
    const double l = left(), r = right();
    double c;
    if (ratio == 0.5 || std::isinf(l) || std::isinf(r)) {
      c = midpoint();
    } else {
      GAOL_RND_ENTER();
      // l, r and ratio are registers: they go through rnd_reread() after the check,
      // or GCC computes c before it writes the control register, in the
      // program's direction (GAOL v5)
      c = rnd_reread(l)*(1.0 - rnd_reread(ratio)) + rnd_reread(r)*rnd_reread(ratio);
      GAOL_RND_KEEP(c);
      GAOL_RND_LEAVE();
      if (c < l) {
        c = l;
      } else if (c > r) {
        c = r;
      }
      if (is_bisectable()) {
        if (c == l) {
          c = std::nextafter(l, r);
        } else if (c == r) {
          c = std::nextafter(r, l);
        }
      }
    }
    return std::make_pair(interval(l, c), interval(c, r));
  }


  // The body of sqrt() after its check, for nth_root(I, -2), which checks the
  // direction before it (GAOL v5). Inlined by force with GCC
  // and Clang, so that sqrt() keeps the code it had when it held this body:
  // Clang 18 called it otherwise
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((always_inline))
#endif
  static inline interval sqrt_upward(const interval& I)
  {
    const interval Ipos = I & interval::positive();

    if (Ipos.is_empty()) {
      return interval::emptyset();
    }

    const double l = (Ipos.left() == 0.0) ? 0.0 : -gaol_minus_sqrt_down(Ipos.left(), gaol_sqrt_up(Ipos.left()));
    const double r = gaol_sqrt_up(Ipos.right());
    return interval(l,r);
  }

  interval sqrt(const interval& I)
  {
#if (defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
    /* The AVX-512 path: the roots by the embedded rounding, the part of I
       in [0, +oo] and its emptiness by the sign and the magnitude bits,
       which the denormals-are-zero mode does not change (the SSE2 path
       checks the modes for the intersection, a lower bound -1e-310 having
       been compared equal to 0) */
    if (!I.is_empty()) {
      return interval(fast_sqrt(I.get_xmminterval()));
    }
#elif GAOL_USING_NEON_INSTRUCTIONS
    // The roots of both bounds by one vector instruction (GAOL v5, see
    // gaol/gaol_interval_neon.cpp), unless the upper bound is above
    // huge_root_argument: the vector product of its proof of exactness
    // overflows, and the code below scales it (GAOL v5, point 79 of TODO.md).
    // The empty set, whose bounds are NaN, is not above it
    if (!gaol_detail::quiet_greater(I.right(), huge_root_argument)) {
      return interval(neon_detail::sqrt_bounds(I.get_neoninterval()));
    }
#endif
    // The part of I in [0, +oo], as nth_root() takes it: the intersection
    // compares the bounds with quiet comparisons, and keeps an empty I as it
    // is, where the constructor, given its NaN bounds, raised the
    // invalid-operation exception (GAOL v5). A lower bound -0 is taken as 0
    // below. The check before the intersection: with denormals-are-zero, a
    // lower bound -1e-310 compared equal to 0 and stayed, and its root, once
    // the check had cleared the mode, made the result empty (GAOL v5, see
    // gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    interval res = sqrt_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  // sqrt_rel() after its check, made before the bounds are compared, as in
  // sqrt()
  static interval sqrt_rel_upward(const interval& J, const interval& I)
  {
    // The part of J in [0, +oo], computed as in sqrt() (GAOL v5)
    const interval Jpos = J & interval::positive();

    if (Jpos.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }

	double l, r;

    // The lower bound as sqrt() computes it, in the upward rounding: GAOL set
    // the direction downward for it, then upward again, two changes of
    // direction per call whichever way GAOL is built (GAOL v5)
    if (Jpos.left() == 0.0) {
      l = 0.0;
      r = gaol_sqrt_up(Jpos.right());
    } else {
      const double x = Jpos.left();
      l = -gaol_minus_sqrt_down(x, gaol_sqrt_up(x));
      r = gaol_sqrt_up(Jpos.right());
    }

	interval Res(l,r);

    if (I.certainly_positive()) {
      return Res & I;
    }
    if (I.certainly_negative()) {
      return (-Res) & I;
    }
    return (I & Res) | (I & (-Res));
  }

  interval sqrt_rel(const interval& J, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = sqrt_rel_upward(J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  /*
    cos(I), and sin(I) as cos(I - pi/2) (template argument sine).

    The bounds of I divided by an enclosure of pi, minus 1/2 for the sine,
    rounded outward, give the pieces [m pi, n pi] on which I lies: on one
    piece (n - m < 2), the function is monotonic, and its bounds are the values
    of the mathematical library at the bounds of I. sin is computed from the
    sine of the mathematical library (GAOL v5): GAOL computed
    cos(I - [pi/2]), whose subtraction widened I by about 2^-52 max(2, |x|), and
    sin([1e-10]) was 4.4e-16 wide.

    Otherwise (GAOL v5, issue #6), I is on several pieces for sure when
    the same divisions rounded inward give the same m and n: an extremum is
    within I when n - m is 2, and both extrema are beyond. When they do not,
    a bound of I being within about |x| 2^-52 of an extremum, or beyond 2^52,
    where the quotients are integers, the divisions cannot tell, and GAOL took
    the extremum as reached: cos([2^60]) was [-1, 1]. The signs of the
    derivative at the bounds of I then tell, exactly, the mathematical library
    being correctly rounded: for I narrower than 2 pi, the derivative has at
    most two zeros within I; signs that differ at the bounds show one, a
    minimum or a maximum according to their order; the same signs show none
    for I narrower than pi, and none or two beyond, which the sign at the
    middle of I tells, each half being narrower than pi. The bounds are then
    within one double of the tightest ones at every magnitude.

    That the test w < gaol_pi_dn below gives the tightest bounds was measured,
    not proved: sin and cos gave the tightest bounds over 29 400 intervals
    of width about pi whose bounds are next to extrema of sin (GAOL v5, old
    point 9 of TODO.md, decided on 3 October).
  */
template<bool sine>
static inline interval cos_or_sin(double Il, double Ir)
{
  // I is not empty, Il being the opposite of its left bound, as the SSE2
  // intervals store it, and Ir its right bound
  // The only check of the rounding direction: nothing below changes it, and
  // the bounds are computed with the functions of namespace upward (GAOL v5).
  // Il and Ir, read before it, are compared after it (see gaol/gaol_fpu.h)
  GAOL_RND_ENTER();
  Il = rnd_reread(Il);
  Ir = rnd_reread(Ir);
  double a,b;
  double Ileft = -Il;
  double Iright = Ir;
  
  // a <= Ileft/pi, b >= Iright/pi
  if (Ileft >= 0) {
    a = -(Il/gaol_pi_up);
    b = Ir/gaol_pi_dn;
  } else {
    a = -(Il/gaol_pi_dn);
    if (Iright > 0) {
      b = Ir/gaol_pi_dn;
    } else {
      b = Ir/gaol_pi_up;
    }
  }
  if (sine) {
    // a <= (Ileft - pi/2)/pi, b >= (Iright - pi/2)/pi, rounded upward
    a = -((-a) + 0.5);
    b = b - 0.5;
  }
  
  double m=std::floor(a),
  n=std::ceil(b),
  u = -1.0, v = 1.0;
  
  double nm = (n-m); // Must be rounded towards +oo
  
  if (nm < 2.0) {
    // even(m)? No conversion to int in order to avoid overflow
    const bool even_m = integer_is_even(m);
    if (even_m) { // Decreasing, as cos on [m pi, (m+1) pi]; use of cos(x)=cos(-x)
      u = sine ? sin_lo(Iright) : cos_lo(Iright);
      v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
    } else { // Increasing
      u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
      v = sine ? sin_hi(Iright) : cos_hi(Iright);
    }
  } else {
    // The half of the width of I, rounded upward: both extrema from 2 pi on,
    // or for infinite bounds. The width itself, Ir + Il, overflowed for a
    // finite result, [-1, 1], when a bound was next to the largest double
    // (GAOL v5, point 79 of TODO.md: cos([-1, DBL_MAX])); the halves of the
    // bounds are exact, and their sum is the half of the rounded sum
    const double half_width = 0.5*Ir + 0.5*Il;
    if (!(half_width < gaol_pi_dn)) {
      GAOL_RND_LEAVE();
      return interval::minus_one_plus_one();
    }
    // a_in >= Ileft/pi and b_in <= Iright/pi, minus 1/2 for the sine
    double a_in, b_in;
    if (Ileft >= 0) {
      a_in = Ileft/gaol_pi_dn;
      b_in = -((-Ir)/gaol_pi_up);
    } else {
      a_in = Ileft/gaol_pi_up;
      b_in = (Iright > 0) ? -((-Ir)/gaol_pi_up) : -((-Ir)/gaol_pi_dn);
    }
    if (sine) {
      a_in = a_in - 0.5;
      b_in = -((-b_in) + 0.5);
    }
    // The halves of I, for the sign at its middle
    const double middle = 0.5*Ileft + 0.5*Iright;
    const bool narrow_halves = (middle + Il < gaol_pi_dn) && (Ir - middle < gaol_pi_dn);
    const bool sure = (std::floor(a_in) == m && std::ceil(b_in) == n);
    if (sure && nm != 2.0) { // Both extrema
      GAOL_RND_LEAVE();
      return interval::minus_one_plus_one();
    }
    const bool even_m = integer_is_even(m);
    
    // -1: a minimum within I; 1: a maximum; 0: none; 2: both
    int extremum;
    if (sure) {
        extremum = even_m ? -1 : 1;
    } else {
        // The derivatives are -sin and cos: s is the sign of the slope, taken
        // on the side of I at a bound that is 0
        int sl = sine ? sign_of_cos(Ileft) : -sign_of_sin(Ileft);
        int sr = sine ? sign_of_cos(Iright) : -sign_of_sin(Iright);
        if (sl == 0) {
          sl = -1; // cos decreases on the right of 0
        }
        if (sr == 0) {
          sr = 1; // and increases on its left
        }
        if (sl != sr) {
            extremum = sl; // Increasing then decreasing: a maximum
        } else if (half_width < 0.5*gaol_pi_dn || (narrow_halves && (sine ? sign_of_cos(middle) : -sign_of_sin(middle)) == sl)) {
            extremum = 0;
        } else {
            extremum = 2;
        }
        if (extremum == 0) { // Monotonic
            if (sl < 0) {
              u = sine ? sin_lo(Iright) : cos_lo(Iright);
              v = sine ? sin_hi(Ileft) : cos_hi(Ileft);
            } else {
              u = sine ? sin_lo(Ileft) : cos_lo(Ileft);
              v = sine ? sin_hi(Iright) : cos_hi(Iright);
            }
          }
      }
      if (extremum == -1) {
          const double
          u1 = sine ? sin_hi(Ileft) : cos_hi(Ileft),
          u2 = sine ? sin_hi(Iright) : cos_hi(Iright);
          u = -1.0;
          v = ((u1 > u2) ? u1 : u2);
      } else if (extremum == 1) {
          const double
          u1 = sine ? sin_lo(Ileft) : cos_lo(Ileft),
          u2 = sine ? sin_lo(Iright) : cos_lo(Iright);
          u = ((u1 < u2) ? u1 : u2);
          v = 1.0;
      } else if (extremum == 2) {
          u = -1.0;
          v = 1.0;
      }
  }
  // The values of the mathematical library moved outward may leave [-1,1]
  // (GAOL v5)
  if (u < -1.0) {
    u = -1.0;
  }
  if (v > 1.0) {
    v = 1.0;
  }
  GAOL_RND_KEEP(u); GAOL_RND_KEEP(v);
  GAOL_RND_LEAVE();
  return interval(u,v);
}

  interval cos(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return cos_or_sin<false>(I.left_internal(),I.right_internal());
  }

  interval sin(const interval& I)
  {
    if (I.is_empty()) {
      return interval::emptyset();
    }
    return cos_or_sin<true>(I.left_internal(),I.right_internal());
  }


} // namespace gaol_core

/*
  The intervals of floats, gaol::intervalf and gaol::interval2f: unfinished,
  and compiled only where a developer of GAOL defines GAOL_FLOAT_INTERVALS
  (see gaol/gaol_config.h); none of the three builds has an option for them
  (GAOL v5)
*/
#ifdef GAOL_FLOAT_INTERVALS
#  include "gaol/float/gaol_intervalf.cpp"
#  if GAOL_USING_SSE3_INSTRUCTIONS
#    include "gaol/float/gaol_interval2f.cpp"
#  endif
#endif // GAOL_FLOAT_INTERVALS
