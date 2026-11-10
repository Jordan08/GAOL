/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_internal.h
  \brief  What the sources of the intervals share: gaol_core (GAOL v5)

  A header of the sources, not installed. The definitions of the intervals
  were all in gaol_interval_core.cpp; they are in one source for each domain
  now (GAOL v5): gaol_interval_core.cpp, the operations and their kernels;
  gaol_interval_pow.cpp, the powers and the roots; gaol_interval_exp_log.cpp,
  the exponentials and the logarithms; gaol_interval_trig.cpp, the
  trigonometric functions; gaol_interval_hyperbolic.cpp, the hyperbolic ones;
  gaol_interval_relational.cpp, the relational functions (*_rel() and
  div_rel()); gaol_interval_io.cpp, the input and the output. Each includes
  this file first: the headers they all included, the macros of the integer
  powers and of the AVX-512 path, the static functions that more than one of
  them call (the saturated products and quotients, the square root rounded
  upward, the bounds of the elementary functions at the points where GAOL
  knows them, pow_lo() and pow_hi(), pown_upward()...), and the declarations
  of the functions that one of them defines for the others.

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#ifndef GAOL_INTERVAL_INTERNAL_H
#define GAOL_INTERVAL_INTERNAL_H

#include <iostream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <cmath>
#include <cfenv>
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
#include "gaol/math/gaol_double_op.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_port.h"
#include "gaol/core/gaol_interval_core.h"

// The FMA instructions on two doubles at once, for the integer powers, where
// the build has them (GAOL v5, see pown_products()): SSE2 with FMA on x86
// (GAOL_FMA_AS_INSTRUCTION, gaol/gaol_fpu.h). The NEON intervals have them all
// (gaol/core/gaol_interval_neon_impl.h)
#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_FMA_AS_INSTRUCTION
#  define GAOL_POWN_PACKED 1
#  include <immintrin.h>
#else
#  define GAOL_POWN_PACKED 0
#endif
// The same kernel rounded upward by the embedded rounding of AVX-512, where
// GAOL_AVX512_ARITHMETIC is (GAOL v5, point ZZ of TODO.md, see
// pown_large_er()): it needs the packed kernel, and runs in functions that
// carry the target of gaol/core/gaol_interval_avx512_impl.h
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

  // The names of gaol_detail (GAOL v5)
  using namespace gaol_detail;

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

#if (defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
/* GCC and Clang accept the AVX-512 instructions in the functions that carry
   the target, and nowhere else. Visual C++ has no such attribute and needs
   none: it takes the intrinsics of any instruction set in any function,
   whatever /arch says, and emits the instructions they stand for (the
   embedded rounding included), with a vzeroupper where the code goes back to
   SSE; clang-cl is Clang and has the attribute (GAOL v5). */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_AVX512_TARGET __attribute__((target("avx512f")))
#  define GAOL_NOINLINE __attribute__((noinline))
#else
#  define GAOL_AVX512_TARGET
#  define GAOL_NOINLINE __declspec(noinline)
#endif
#endif
// The functions of the integer powers inlined by force, and those kept out of
// line (see pown_products() and pown_checked() in gaol_interval_core.cpp)
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
// The functions that inline the operations of pown_er_ops (and the ones
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

  /*
    The corners of x^n on [a, b], |n| >= 2 (see pown_products() in
    gaol_interval_core.cpp): c0, where x^n is the least, and c1, where it is
    the greatest. x^n increases for an odd n > 0, and decreases on each side
    of 0 for an odd n < 0; for an even n, it is |x|^n, increasing in |x| for
    n > 0 and decreasing for n < 0. A corner at 0 for n < 0 is the limit of
    x^n on the side of 0 x lies on: -oo at -0, +oo at +0 (pown_point(),
    gaol_interval_core.cpp). false where x^n has no value on x, which is [0]
    for n < 0: 0^n has none (IEEE 1788-2015, Table 9.1).
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

  /*
    The functions that one source of the intervals defines and others call
    (GAOL v5: gaol_interval_core.cpp held them all, with the static functions
    of this file). No installed header declares them.
  */
  // gaol_interval_core.cpp: the kernel of the integer powers of the kind of
  // intervals (pown_large_inline()), and gaol_pown() after the test of n = 2
  GAOL_POWN_TARGET interval pown_large_upward(const interval& I, int n);
  GAOL_POWN_TARGET GAOL_POWN_NOINLINE interval pown_checked(const interval& I, int n);
  // gaol_interval_pow.cpp: whether x^y is a double, x^y rounded upward
  // without the overflow exception, and the n-th root after the check of the
  // direction
  bool pow_is_double(double x, double y, double& d);
  double pow_up_without_overflow_exception(double x, double y);
  interval nth_root_upward(const interval& I, unsigned int n);
  // gaol_interval_hyperbolic.cpp: acosh(), asinh() and atanh() after the
  // check of the direction
  interval acosh_upward(const interval& I);
  interval asinh_upward(const interval& I);
  interval atanh_upward(const interval& I, bool by_division = true);


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

  /* The smallest positive double, 2^-1074, written as a literal rather than as
     std::numeric_limits<double>::denorm_min(): that is __DBL_DENORM_MIN__, a
     long double constant converted to double, and GCC does not fold the
     conversion of a long double to a subnormal double with -frounding-math
     ((double) 0x1p-1022L is folded, 0x1p-1023L is not). GCC 15.2 on x86-64
     loaded the constant as an extended number (fldt) and stored it as a double
     (fstpl) at each use, a microcode assist of about 100 ns on an Intel
     i7-1185G7 and 160 ns on an Intel Xeon (sqrt() took 111 ns, against 10 ns
     with Clang, which defines __DBL_DENORM_MIN__ as a double), and GCC 12.2 on
     aarch64 calls __trunctfdf2 (GAOL v5, #224). The literal has the 17
     significant digits that Clang gives __DBL_DENORM_MIN__ (5e-324 is the
     shortest), C++11 having no hexadecimal floating literal */
  static const double double_denorm_min = 4.9406564584124654e-324;

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

  static inline double gaol_sqrt_up(double x)
  {
    double s = ::sqrt(x);
    if (square_down_below(s, x)) { // s*s rounded downward < x proves s < sqrt(x)
      s += double_denorm_min;
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
  static inline double gaol_minus_sqrt_down(double x, double u)
  {
    return square_up_equals(u, x) ? -u : (-u + double_denorm_min);
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
    upward, which do not check it again (GAOL v5). Those that one source
    alone calls are in it: sin_lo(), cos_lo()... in gaol_interval_trig.cpp,
    sinh_lo()... in gaol_interval_hyperbolic.cpp.
  */
  static inline double tan_lo(double x) { return (x == 0.0) ? 0.0 : upward::tan_dn(x); }
  static inline double tan_hi(double x) { return (x == 0.0) ? 0.0 : upward::tan_up(x); }

  static inline double asin_lo(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? gaol_half_pi_dn : ((x == -1.0) ? -gaol_half_pi_up : upward::asin_dn(x)));
  }

  static inline double asin_hi(double x)
  {
    return (x == 0.0) ? 0.0 : ((x == 1.0) ? gaol_half_pi_up : ((x == -1.0) ? -gaol_half_pi_dn : upward::asin_up(x)));
  }

  static inline double acos_lo(double x)
  {
    return (x == 1.0) ? 0.0 : ((x == 0.0) ? gaol_half_pi_dn : ((x == -1.0) ? gaol_pi_dn : upward::acos_dn(x)));
  }

  static inline double acos_hi(double x)
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
  static inline double atan_lo(double x)
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

  static inline double atan_hi(double x)
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
    x^y rounded upward for x >= 0 and y finite or not, the rounding direction
    being upward: CORE-MATH's pow, which is correctly rounded in that
    direction, and for the template argument quiet without the overflow
    exception where the power is above the largest double
    (pow_up_without_overflow_exception()). The one power computed here is the
    tiny one, x^y < 1 for a finite x > 0 other than 1 and |y| >= 2^1013:
    |y log(x)| >= 2^960 there, and x^y rounded upward is 2^-1074 (for an
    infinite y, whose power is 0, it is an upper bound, which is all
    root_of_power_near() asks of it). The is_exact() of CORE-MATH's pow
    multiplies y by the exponent e of a power of two x, a product that
    overflows from |y e| = 2^1024 on (|e| is at most 1074), which raises the
    overflow exception for this finite result (GAOL v5, point 79 of TODO.md;
    pow(2^-1074, 1.5 2^1014) does): it needs y > 0, so x < 1, but the power is
    right for the other sign too, and cheaper than CORE-MATH.
    Until upstream commit c01f08c5 (5 October 2026) CORE-MATH's pow raised two
    more exceptions, and GAOL computed the powers of an exponent of extreme
    magnitude itself: the invalid-operation exception for |y| < 2^-969 and for
    |y| >= 2^1014, where it compared with < the NaN it makes of its first
    approximation of log(x) (pow([1, 2], [1e-300]) killed a program that
    enabled it), and the overflow exception from |y log(x)| = 2^512 on, where
    it squared that product, for a tiny power.
  */
  template <bool quiet = false>
  static inline double pow_rounded_up(double x, double y)
  {
    // 2^1013, written in decimal with the digits that give it exactly: C++11
    // has no hexadecimal floating literal
    if (std::fabs(y) >= 8.777798510069902e+304 && x > 0.0 && x < GAOL_INFINITY && x != 1.0
        && (x > 1.0) != (y > 0.0)) {
      return double_denorm_min;
    }
    return quiet ? pow_up_without_overflow_exception(x, y) : upward::nthroot_up(x, y);
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

  // feven() of an integer i that an operation computed: neither i nor its
  // half is subnormal, so that the modes that flush the subnormals to zero
  // change nothing here
  static inline bool integer_is_even(double i)
  {
    return (std::floor(0.5*i)*2.0 == i);
  }

  // An increasing function, whose range the bounds are brought back into: the
  // value at the right bound, the double below the value at the left bound
  // unless it is a double. The rounding direction is upward, after the check
  // of increasing_cr() or of the caller, made before I is computed (see
  // gaol/gaol_fpu.h)
  static inline interval increasing_cr_upward(const interval& I, double (*f)(double),
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

  static inline interval increasing_cr(const interval& I, double (*f)(double),
                                       bool (*exact)(double), double lowest, double highest)
  {
    GAOL_RND_ENTER();
    interval res = increasing_cr_upward(I, f, exact, lowest, highest);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }




  /*
    inverse_upward() and sqr_upward() of gaol/core/gaol_interval_core.h, which are
    private to interval, for pown_upward() and nth_root(I, q) for q < 0: they
    check the rounding direction once and take the inverse or the square after
    it, without checking it again as inverse() and sqr() do (GAOL v5)
  */
  class upward_access {
  public:
    static interval inverse(const interval& I) { return interval::inverse_upward(I); }
    static interval sqr(const interval& I) { return interval::sqr_upward(I); }
#if GAOL_USING_SSE2_INSTRUCTIONS
    //! The stored bounds <-l, r>, for max() and min() (GAOL v5)
    static __m128d stored_bounds(const interval& I) { return I.get_xmminterval(); }
#elif GAOL_USING_NEON_INSTRUCTIONS
    static float64x2_t stored_bounds(const interval& I) { return I.get_neoninterval(); }
#endif
  };

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
    [+oo] and [-oo] contain no real number: numsToInterval(l, u) of IEEE
    1788-2015 has no value for l = +oo or u = -oo (10.5.8). The constructors
    and the operations make none, but the constructors of the registers
    without a check (gaol_detail::unchecked_bounds) do, as the code of GAOL
    may use them (the ones of the interface give the empty set). pow gives the empty set for one as an exponent, and
    pow_rel() and pow_exponent_rel() for one as the power, or as the exponent
    or the base (GAOL v5: they took it for the limit of x^y). The caller has
    tested for the empty set, whose NaN bounds this would compare with the
    signaling comparisons
  */
  static inline bool holds_no_real(const interval& a)
  {
    return !(a.left() < GAOL_INFINITY && a.right() > -GAOL_INFINITY);
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

} // namespace gaol_core

#endif /* GAOL_INTERVAL_INTERNAL_H */
