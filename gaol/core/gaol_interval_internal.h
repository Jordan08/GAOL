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
  \brief  What the sources of gaol/core/ share, not installed: the common includes and macros,
          the static helpers, the inline kernels and the declarations between sources.

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

/*
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
  them call (the saturated quotients, with the saturated products that the
  relational functions alone call, the square root rounded upward, the
  bounds of the elementary functions at the points where GAOL knows them,
  mig_of() and mag_of(), acosh_upward() and its twins, pow_lo(), pow_hi()
  and pow_up_without_overflow_exception(), pown_upward(), sqrt_checked()...),
  the kernels of the kind of intervals that they take inline (the header of
  the kernels it includes, gaol_interval_{sse,neon,fpu}_kernels.h), and the
  declarations of the functions that one of them defines for the others. A
  function that gaol_interval_core.cpp took inline when every definition was
  there is inline here, and the function of the library is a call of it, so
  that the other sources take it inline too (GAOL v5). Two are called:
  pown_large_upward(), whose kernel is the long switch of pown_large_inline(),
  and midpoint(), which the output alone calls (through mid_rad(), inline in
  gaol_interval_core.h), where the writing of the digits costs far more.
*/
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
    tests/reverse.cpp). inline and not static: the inline kernels of
    gaol_interval_neon_kernels.h and gaol_interval_fpu_kernels.h, which every
    source defines, call it, and have to call the same function in all of
    them (the one-definition rule; GAOL v5)
  */
  inline double read_in_branch(double x)
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

#if (defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
  // The square root of the AVX-512 path (gaol_interval_avx512_impl.h), which
  // sqrt_checked() below calls (GAOL v5)
  GAOL_AVX512_TARGET __m128d fast_sqrt(__m128d x);
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
  // gaol_interval_pow.cpp: whether x^y is a double, and the n-th root after
  // the check of the direction
  bool pow_is_double(double x, double y, double& d);
  interval nth_root_upward(const interval& I, unsigned int n);


  /*
    The mignitude and the magnitude of I: the bodies of interval::mig() and
    interval::mag() (gaol_interval_core.cpp), which call them, inline for
    hypot() too (GAOL v5)
  */
  static inline double mig_of(const interval& I)
  {
    if (I.is_empty()) {
      return GAOL_NAN;
    } else {
      if (I.set_contains(0)) {
	return 0.0;
      }
      // Compared as in the relations (gaol_port.h): under denormals-are-zero,
      // mig([-3, -2]*2^-1074) was 0 (GAOL v5, point Q of TODO.md)
      if (gaol_detail::bound_less(I.right(), 0.0)) {
	return -I.right();
      } else { // left() > 0.0
	return I.left();
      }
    }
  }

  static inline double mag_of(const interval& I)
  {
    return maximum(std::fabs(I.left()),std::fabs(I.right()));
  }

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

  static inline double asinh_lo(double x) { return (x == 0.0) ? 0.0 : upward::asinh_dn(x); }
  static inline double asinh_hi(double x) { return (x == 0.0) ? 0.0 : upward::asinh_up(x); }
  static inline double acosh_lo(double x) { return (x == 1.0) ? 0.0 : upward::acosh_dn(x); }
  static inline double acosh_hi(double x) { return (x == 1.0) ? 0.0 : upward::acosh_up(x); }
  static inline double atanh_lo(double x) { return (x == 0.0) ? 0.0 : upward::atanh_dn(x); }
  static inline double atanh_hi(double x) { return (x == 0.0) ? 0.0 : upward::atanh_up(x); }

  /*
    The bodies of acosh(), asinh() and atanh() after their check, for these
    functions and for acosh_rel(), asinh_rel() and atanh_rel(), which check
    the direction before them (GAOL v5: the reverse functions called acosh(),
    asinh() and atanh(), which checked it a second time)
  */
  static inline interval acosh_upward(const interval& I)
  {
    const interval J = I &  interval::one_plus_infinity();

    if (J.is_empty()) {
      return J;
    }
    const double l = acosh_lo(J.left()), r = acosh_hi(J.right());
    return interval(l,r);
  }

  static inline interval asinh_upward(const interval& I)
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
  static inline interval atanh_upward(const interval& I, bool by_division = true)
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
    x < 1 and y < 0: CORE-MATH's pow goes its way in the other cases, where
    pow_rounded_up() has taken the tiny powers of |y| >= 2^1013 apart (before
    upstream commit c01f08c5, 5 October 2026, CORE-MATH squared y log(x) in
    its first phase, which overflowed from 2^512 on, for a power that is tiny).
    The binary logarithm of x^y is
    bounded by |y| |log2(x)|, with |log2(x)| at most |e| + 1, e being the
    exponent of x as frexp() gives it: when that is below 1000, nothing
    overflows. From |y| >= 2^500 on, |y log2(x)| >= 2^447, and x^y is beyond
    the doubles, +oo. Otherwise
    t = |y| |log(x)|/log(2) is computed from the logarithm of CORE-MATH,
    which is within a few units of 2^-52 of it relatively, so that
    x^y < 2^(1024 - 2^-30) is no overflow for t < 1024 - 10^-9, and x^y is
    beyond the doubles for t > 1024 + 10^-9. Only the powers in between ask
    CORE-MATH to decide: they are computed with the exceptions masked, and
    the environment of the program, its flags and the exceptions it enabled,
    restored.
  */
  static inline double pow_up_without_overflow_exception(double x, double y)
  {
    if (!(x > 0.0 && x < GAOL_INFINITY) || x == 1.0) {
      return upward::nthroot_up(x, y);
    }
    const bool grows = (x > 1.0) == (y > 0.0);
    if (!grows) {
      return upward::nthroot_up(x, y);
    }
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
      return GAOL_INFINITY;
    }
    // 1/log(2), rounded to nearest: the margins below are far larger than its error
    const double t = magnitude*std::fabs(upward::log_up(x))*1.4426950408889634;
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
    The inverse and the square of a non-empty interval, the rounding direction
    being upward already, for inverse(), pown_upward() and nth_root(I, q) for
    q < 0, which check the direction once and take the inverse or the square
    after it, without checking it again as inverse() and sqr() do; and
    checked_sqr() and checked_inverse(), sqr() and inverse() with their check,
    for x^2 in gaol_pown() and 1/sqrt(x) in nth_root(x, -2) (GAOL v5). A friend of interval (gaol/core/gaol_interval_core.h), for its stored
    bounds. Its functions are inline in the header of the kernels of each kind
    of intervals, included below, and the square and the inverse of the
    library call checked_sqr() and checked_inverse(): inverse() and sqr() were inverse_upward() and sqr_upward(), private
    functions of interval, which gaol_interval_core.cpp alone defined
  */
  class upward_access {
  public:
    static interval inverse(const interval& I);
    static interval sqr(const interval& I);
    static interval checked_sqr(const interval& I);
    static interval checked_inverse(const interval& I);
#if GAOL_USING_SSE2_INSTRUCTIONS
    //! The stored bounds <-l, r>, for max() and min() (GAOL v5)
    static __m128d stored_bounds(const interval& I) { return I.get_xmminterval(); }
#elif GAOL_USING_NEON_INSTRUCTIONS
    static float64x2_t stored_bounds(const interval& I) { return I.get_neoninterval(); }
#endif
  };

} // namespace gaol_core

/*
  The kernels of each kind of intervals that more than one source takes: the
  inverse and the square above, and what they take, inline (GAOL v5). They
  were functions of gaol_interval_{sse,neon,fpu}_impl.h, which
  gaol_interval_core.cpp alone includes, and the compilers took them inline
  into the powers and the roots while every definition was in that file:
  called from gaol_interval_pow.cpp, x^2 took 1.93 ns rather than 1.60 and
  nth_root(x, -2) 4.11 ns rather than 3.59 (Apple M2, NEON intervals, Apple
  clang 21). The functions of the fragments are now calls of them
*/
#if GAOL_USING_SSE2_INSTRUCTIONS
#  include "gaol/core/gaol_interval_sse_kernels.h"
#elif GAOL_USING_NEON_INSTRUCTIONS
#  include "gaol/core/gaol_interval_neon_kernels.h"
#else
#  include "gaol/core/gaol_interval_fpu_kernels.h"
#endif

namespace gaol_core {

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

  /*
    sqrt() of I, with its check: its body, which sqrt() (gaol_interval_core.cpp)
    calls and nth_root(I, q) for q = -2 takes inline, as it did when both
    were in gaol_interval_core.cpp (GAOL v5). fast_sqrt() is the AVX-512 root
    of gaol_interval_avx512_impl.h, and neon_detail::sqrt_bounds() the NEON
    one of gaol_interval_neon_kernels.h
  */
  static inline interval sqrt_checked(const interval& I)
  {
#if (defined(__x86_64__) || defined(_M_X64)) && GAOL_USING_SSE2_INSTRUCTIONS && GAOL_AVX512_ARITHMETIC
    /* The AVX-512 path: the roots by the embedded rounding, the part of I
       in [0, +oo] and its emptiness by the sign and the magnitude bits,
       which the denormals-are-zero mode does not change (the SSE2 path
       checks the modes for the intersection, a lower bound -1e-310 having
       been compared equal to 0) */
    if (!I.is_empty()) {
      return interval(fast_sqrt(I.get_xmminterval()), gaol_detail::unchecked_bounds);
    }
#elif GAOL_USING_NEON_INSTRUCTIONS
    // The roots of both bounds by one vector instruction (GAOL v5, see
    // gaol/core/gaol_interval_neon_kernels.h), unless the upper bound is above
    // huge_root_argument: the vector product of its proof of exactness
    // overflows, and the code below scales it (GAOL v5, point 79 of TODO.md).
    // The empty set, whose bounds are NaN, is not above it
    if (!gaol_detail::quiet_greater(I.right(), huge_root_argument)) {
      return interval(neon_detail::sqrt_bounds(I.get_neoninterval()), gaol_detail::unchecked_bounds);
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

} // namespace gaol_core

#endif /* GAOL_INTERVAL_INTERNAL_H */
