/*-*-C-*---------------------------------------------------------------------
* gaol -- NOT Just Another Interval Library
*--------------------------------------------------------------------------
* What the sources of CORE-MATH (3rd/math-core) need to compile as a part of
* GAOL, with every compiler GAOL is built with and on every architecture.
*
* The three builds include this header of themselves, before anything else,
* in each source of CORE-MATH they compile (`-include` with GCC and Clang,
* `/FI` with Visual C++): the sources themselves do not name it, so that
* importing a newer CORE-MATH is a plain copy.
*
* It gives them:
*
* - the 128-bit integer their accurate phases compute with
*   (gaol/gaol_u128.h), so that log, sin, cos, tan, atan2, pow, log2p1,
*   log10p1, atan2pi, hypot, rsqrt and asinpi are built with Visual C++ and on
*   the 32-bit targets where the compiler has no 128-bit type of its own;
* - the names gaol_cr_<f>() rather than cr_<f>(), so that GAOL does not clash
*   with a program or a C library holding CORE-MATH's functions too;
* - __builtin_roundeven() where the compiler or the math library has not;
* - on a 32-bit x86 Windows, a fegetexceptflag() and a fesetexceptflag()
*   written on MXCSR, which leave its exception masks alone;
* - on 32-bit x86, the square root of SSE2, rounded in the direction in
*   effect, rather than the one of the C library;
* - on x86, a fegetround() that reads MXCSR, where their doubles are
*   rounded, rather than the x87 unit, which the C library may read;
* - what Visual C++ has not of GCC: the builtins the sources call,
*   __attribute__, and NAN and INFINITY as constants;
* - the ldexp() of gaol/gaol_ldexp.h, rounded in the direction in effect,
*   rather than the one of the C library;
* - silence for the warnings on conversions GAOL's library is compiled with.
*
* Copyright (c) 2026 ENSTA, France
* Modified by Jordan NININ (GAOL v5)
*
* Created 2026-09-20 by Jordan NININ
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
* COPYING file for information.
*--------------------------------------------------------------------------*/

#ifndef GAOL_CORE_MATH_PORT_H
#define GAOL_CORE_MATH_PORT_H

/* GCC 14 for a 32-bit x86 target stopped on an internal compiler error in
asinpi_acc() of asinpi.c ("in extract_bit_field_1, at expmed.cc:1838", at
-O2 and -O3): its SLP vectorizer puts the two 64-bit halves of the 128-bit
integer in a vector it cannot take them out of again
(the continuous integration, MinGW-w64 14.2 x86; reproduced with the GCC
14.2 of Ubuntu and -m32). GCC 12, 13 and 15 compile it. The SLP vectorizer
is turned off for the sources of CORE-MATH with that compiler only, which
compute on scalars. */
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 14 \
&& defined(__i386__) && !defined(__x86_64__)
#pragma GCC optimize ("no-tree-slp-vectorize")
#endif

#include <stdint.h>

/*---------------------------------------------------------------------------
The names of the functions
--------------------------------------------------------------------------*/

#define cr_exp gaol_cr_exp
#define cr_log gaol_cr_log
#define cr_pow gaol_cr_pow
#define cr_sin gaol_cr_sin
#define cr_cos gaol_cr_cos
#define cr_tan gaol_cr_tan
#define cr_asin gaol_cr_asin
#define cr_acos gaol_cr_acos
#define cr_atan gaol_cr_atan
#define cr_atan2 gaol_cr_atan2
#define cr_sinh gaol_cr_sinh
#define cr_cosh gaol_cr_cosh
#define cr_tanh gaol_cr_tanh
#define cr_asinh gaol_cr_asinh
#define cr_acosh gaol_cr_acosh
#define cr_atanh gaol_cr_atanh
#define cr_cbrt gaol_cr_cbrt
#define cr_exp2 gaol_cr_exp2
#define cr_exp10 gaol_cr_exp10
#define cr_log2 gaol_cr_log2
#define cr_log10 gaol_cr_log10
#define cr_expm1 gaol_cr_expm1
#define cr_exp2m1 gaol_cr_exp2m1
#define cr_exp10m1 gaol_cr_exp10m1
#define cr_sinpi gaol_cr_sinpi
#define cr_cospi gaol_cr_cospi
#define cr_tanpi gaol_cr_tanpi
#define cr_acospi gaol_cr_acospi
#define cr_atanpi gaol_cr_atanpi
#define cr_log1p gaol_cr_log1p
#define cr_log2p1 gaol_cr_log2p1
#define cr_log10p1 gaol_cr_log10p1
#define cr_hypot gaol_cr_hypot
#define cr_rsqrt gaol_cr_rsqrt
#define cr_asinpi gaol_cr_asinpi
#define cr_atan2pi gaol_cr_atan2pi
#define cr_erf gaol_cr_erf
#define cr_erfc gaol_cr_erfc

#include "gaol/gaol_core_math.h"

/*---------------------------------------------------------------------------
The 128-bit unsigned integer

The accurate phases of log, sin, cos, tan, atan2, pow, log2p1, log10p1,
atan2pi, hypot, rsqrt and asinpi compute with a 128-bit unsigned integer,
which their sources name u128, and asinpi with a signed one too, i128, held
in a u128 in two's complement. GCC and Clang have one on 64-bit targets, and
_BitInt(128) on 32-bit targets too: Clang from 14 on, and GCC from 14 on
where the ABI of the target defines it, as for x86, which
__BITINT_MAXWIDTH__ tells. Visual C++ has none, on no architecture, and
neither has GCC for the 32-bit targets whose ABI defines no _BitInt(128).
There the sources take the structure of two 64-bit halves of
gaol/gaol_u128.h, which tests/u128.cpp checks against the native type.

Each of these sources keeps its own definitions of uint128_t, addu_128,
subu_128, cmp and cmpu, whose signatures are not the same in all of them.
The lines that choose the type name gaol_u128 instead, and the
extended-arithmetic functions call the gaol_u128_*() functions rather than
the operators of the language: changes made in the sources themselves,
marked GAOL (3rd/README.md, "What differs from upstream", item 1).
--------------------------------------------------------------------------*/

#include "gaol/gaol_u128.h"

/*---------------------------------------------------------------------------
roundeven(), which the math library of Windows has not

The sources call __builtin_roundeven(). GCC and Clang turn it into one
instruction where the processor has it (roundsd of SSE4.1, frintn on ARM),
and into a call to roundeven() of C23 otherwise: the math library of glibc
has that function, those of mingw-w64 and of Visual C++ have not, and a
program linking GAOL there stopped on "undefined reference to roundeven"
(the continuous integration, with MinGW-w64 15 and MSYS2, where GAOL is
compiled without the AVX instructions).

So on Windows the builtin is replaced by the function of
gaol/gaol_roundeven.h, which reads the bits: it is neither round() (halfway
values away from zero) nor nearbyint() (the rounding direction in effect,
upward in GAOL), and it does not depend on that direction.

It is given as well to the compilers that have no such builtin: GCC before
10 and the Clang whose __has_builtin() says so. The other sources of
CORE-MATH take __builtin_roundeven() only from GCC 10 and Clang 17 and
round by themselves before, but sin.c called it unguarded from its rewrite
(upstream commit 6b844573) to commit 19e85418 (5 October 2026), which GCC 9.4
took for an undeclared function and did not link.
--------------------------------------------------------------------------*/

#include "gaol/gaol_roundeven.h"

#if defined(_WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
#define __builtin_roundeven(x) gaol_roundeven(x)
#elif defined(__clang__)
#  if defined(__has_builtin)
#    if !__has_builtin(__builtin_roundeven)
#      define __builtin_roundeven(x) gaol_roundeven(x)
#    endif
#  endif
#elif defined(__GNUC__) && __GNUC__ < 10
#define __builtin_roundeven(x) gaol_roundeven(x)
#endif /* Windows, or no __builtin_roundeven() */

/*---------------------------------------------------------------------------
fesetexceptflag() on a 32-bit Windows, which unmasks the exceptions

cbrt, pow, atan2 and hypot keep the exception flags around their work, with
_mm_getcsr() and _mm_setcsr() on x86-64 (cbrt, pow and hypot; atan2 calls
fegetexceptflag() and fesetexceptflag() there too) and with
fegetexceptflag() and fesetexceptflag() everywhere else. The
fesetexceptflag() of mingw-w64 for a 32-bit target does not write the flags
alone: it clears the mask bits of MXCSR with them, so that the invalid, the
divide-by-zero and the overflow exceptions become unmasked (the register went
from 0x5fb2 to 0x5932 in the continuous integration). An exception then traps
rather than raise a flag, and GAOL died on the first comparison of the bounds
of an empty interval, which are NaN and which is_empty() compares with <=, an
operation that signals invalid.

So on a 32-bit x86 Windows the two are written here, on MXCSR, where GAOL's
doubles are computed (gaol/gaol_config.h refuses an x86 target whose doubles
are not), and the mask bits are left exactly as they are. Each FE_* flag is
mapped to its bit of MXCSR: the values of mingw-w64 up to 12 are those bits,
as glibc's for x86, but those of the UCRT, which Visual C++ and clang-cl
take, and of mingw-w64 from 13 on, which took them, are not (FE_INEXACT 0x01
to FE_INVALID 0x10). Taken for the bits, they had pow save and restore the
invalid flag for the inexact one, atan2 the denormal flag for the underflow
one, and cbrt and hypot never the inexact one, which then stayed raised
after an exact result such as hypot(3, 4) (simulated on Linux with the
values of the UCRT; tests/core_math.cpp checks exact results raise none).
--------------------------------------------------------------------------*/

#if defined(_WIN32) && (defined(__i386__) || defined(_M_IX86)) && !defined(__x86_64__)

#include <fenv.h>
#include <xmmintrin.h>

/* The flag bits of MXCSR of the FE_* flags in excepts: IE, ZE, OE, UE and PE,
   bits 0, 2, 3, 4 and 5 (DE, bit 1, is no flag of IEEE 754). A fexcept_t of
   the two functions below holds these bits, which only they read. */
static inline unsigned int gaol_mxcsr_flags(int excepts)
{
  return ((excepts & FE_INVALID) ? 0x01u : 0u) | ((excepts & FE_DIVBYZERO) ? 0x04u : 0u)
       | ((excepts & FE_OVERFLOW) ? 0x08u : 0u) | ((excepts & FE_UNDERFLOW) ? 0x10u : 0u)
       | ((excepts & FE_INEXACT) ? 0x20u : 0u);
}

static inline void gaol_fegetexceptflag(fexcept_t *flagp, int excepts)
{
  *flagp = (fexcept_t)(_mm_getcsr() & gaol_mxcsr_flags(excepts));
}

static inline void gaol_fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  const unsigned int keep = gaol_mxcsr_flags(excepts);
  const unsigned int want = (unsigned int)(*flagp) & keep;
  _mm_setcsr((_mm_getcsr() & ~keep) | want);
}

#define fegetexceptflag(f, e) gaol_fegetexceptflag((f), (e))
#define fesetexceptflag(f, e) gaol_fesetexceptflag((f), (e))

#endif /* a 32-bit x86 Windows */

/*---------------------------------------------------------------------------
The square root on 32-bit x86, which the C libraries of Windows round to
nearest

The sources take their square roots with __builtin_sqrt(): pow.c for
y = 0.5 where its first phase cannot round, and acos, asin, acosh, asinh,
atanh, hypot, rsqrt, asinpi and acospi. The square root of the C libraries
of Windows for 32-bit x86 is rounded to nearest in every rounding direction
(see gaol_sqrt_up() in gaol/core/gaol_interval_core.cpp). Visual C++ calls it for the
builtin, and so does GCC for mingw-w64 where it does not optimize (-O0,
the Debug builds) and in the accurate phases of acos, asin and asinpi,
which the sources mark cold, at -O2 too (GCC 13); elsewhere it writes
sqrtsd, and calls the C library for a negative argument only (acosh,
acospi). pow(x, 0.5) was then below the root rounding upward, and above it
rounding downward, at subnormal x, and GAOL's upper bound of
pow([0x0.0000100020002p-1022], 0.5) did not enclose the root
(tests/core_math.cpp, Visual C++ x86, and MinGW-w64 15 x86 Debug). The
square root of SSE2 is taken instead on every 32-bit x86, with every
compiler, every C library and at every optimization level, which rounds in
the direction of MXCSR, as every other operation on doubles does there
(gaol/gaol_config.h refuses an x86 target that does not compute its doubles
with SSE2, and the three builds give these sources -msse2 -mfpmath=sse).
Elsewhere than on Windows, the square root of the C library, fsqrt on the
x87 unit with glibc and musl, is right as long as the x87 unit rounds as
MXCSR, which round_upward_if_needed() of gaol/gaol_fpu.h sees to before
CORE-MATH runs; with SSE2 everywhere, the square roots of CORE-MATH depend
neither on the C library nor on the x87 unit (GAOL v5, point A.3 of
TODO.md). On x64 Windows, Visual C++ and clang-cl call the sqrt() of the
UCRT for the builtin as well (/fp:strict): it rounds in the direction of
MXCSR there, which tests/core_math.cpp checks in their jobs of the
continuous integration (pow(x, 0.5) in pow_exact_at_corners(), pow.c
returning that root for y = 0.5 where its first phase cannot round; rsqrt
and asinpi correct the root they take, and do not show its rounding).
--------------------------------------------------------------------------*/

#if (defined(__i386__) || defined(_M_IX86)) && !defined(__x86_64__)

#include <emmintrin.h>

static inline double gaol_sqrt(double x)
{
  return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_set_sd(x), _mm_set_sd(x)));
}

#define __builtin_sqrt(x) gaol_sqrt(x)

#endif /* 32-bit x86 */

/*---------------------------------------------------------------------------
fegetround() on x86: the direction of the SSE instructions

pow rounds a subnormal result itself, in the direction fegetround() gives,
where every other double it computes is rounded by the SSE instructions, in
the direction of MXCSR (tan has the same code, in an accurate phase that never
gives a subnormal, and cos has none). On x86-64 the two are the same only if
nothing left them differing: the fegetround() of glibc reads the control word
of the x87 unit alone, and GAOL finds the direction upward when 1 + 2^-60, a
sum of the SSE instructions, is above 1, without looking at the x87 unit. A
program that leaves the x87 unit to nearest and MXCSR upward, as the
exactinit() of the predicates of Shewchuk and of Triangle does, then had pow()
round its subnormal results to nearest: the upper bound was below the exact
value for about half of the arguments (tests/rounding_direction.cpp).

So the sources of CORE-MATH read the direction from MXCSR here, since the
processor computes their doubles there, as the get_rounding_mode() of
cbrt.c, rsqrt.c and asinpi.c does with GCC and Clang, which define
__x86_64__, whatever values the C library gives FE_* (item 6 of
3rd/README.md); Visual C++ for x64 defines no __x86_64__, and these call
fegetround() there too, this one. The rounding field of MXCSR is the same on
every x86 processor, but the values of FE_UPWARD and FE_DOWNWARD are the
ones of the C library (those of Windows are not those of glibc, and changed
once), which the switch gives back whichever they are.

The standard lets fegetround() be a macro, and it is a function in the C
libraries known here: what the sources call is renamed by a macro of ours,
after <fenv.h> has declared it, and only for the sources of CORE-MATH, the
only ones this header is included in. ARM64EC, which no build of GAOL
targets, defines _M_X64 too, and would read the MXCSR its emulation keeps.

On a 32-bit x86 processor too (GAOL v5, point AI.108 of TODO_DONE.md). The
operations that call CORE-MATH have read both units there and set the one that
is not upward (round_upward_if_needed() of gaol/gaol_fpu.h), so that the two
agree when it runs and the answer has been the same; but fegetround() is a
call into the C library, about 80 ns with Visual C++ for x86 (which gives -1
when the units differ), and cbrt and asinpi call it on every call.
MXCSR is read by one instruction, stmxcsr, and is the register of the doubles
of CORE-MATH there as on x86-64 (gaol/gaol_config.h refuses an x86 target
that computes its doubles elsewhere). tests/core_math.cpp calls pow and cbrt
with the x87 unit and MXCSR differing, which only this reading gets right.
--------------------------------------------------------------------------*/

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

#include <fenv.h>
#include <xmmintrin.h>

static inline int gaol_fegetround(void)
{
  switch (_mm_getcsr() & _MM_ROUND_MASK) {
    case _MM_ROUND_NEAREST: return FE_TONEAREST;
    case _MM_ROUND_DOWN: return FE_DOWNWARD;
    case _MM_ROUND_UP: return FE_UPWARD;
    default: return FE_TOWARDZERO;
  }
}

#undef fegetround
#define fegetround gaol_fegetround

#endif /* x86 */

/*---------------------------------------------------------------------------
What Visual C++ has not of GCC and Clang
--------------------------------------------------------------------------*/

#if defined(_MSC_VER) && !defined(__clang__)

#include <math.h>
#include <string.h>
#include <intrin.h>

#pragma warning(disable: 4244 4267 4146 4305 4723)

#define __builtin_expect(x, y) (x)
#define __builtin_fma(x, y, z) fma(x, y, z)
#define __builtin_fabs(x) fabs(x)
#define __builtin_copysign(x, y) copysign(x, y)
#define __builtin_floor(x) floor(x)
#define __builtin_round(x) round(x)
#define __builtin_trunc(x) trunc(x)
#define __builtin_isnan(x) isnan(x)
#define __builtin_isinf(x) isinf(x)
#define __builtin_fmin(x, y) fmin(x, y)
#define __builtin_fmax(x, y) fmax(x, y)
#define __builtin_nan(s) nan(s)
#define __builtin_inf() ((double)INFINITY)
#define __attribute__(x)

/* On 32-bit x86, the square root of SSE2 (see above) */
#if !defined(_M_IX86)
#define __builtin_sqrt(x) sqrt(x)
#endif

/* NAN and INFINITY, which pow.c sets and returns, are no constants in a C
source compiled by Visual C++: the UCRT writes them
((float)(_HUGE_ENUF * _HUGE_ENUF)) and
(-(float)(((float)(_HUGE_ENUF * _HUGE_ENUF)) * 0.0F)), _HUGE_ENUF being
1e+300, unless the compiler is compiling C++ or __has_builtin() names
__builtin_nanf, which Visual C++ has not. Under /fp:strict these products
and conversions are computed when the program runs, in the rounding
direction in effect, and give FLT_MAX and -0 downward and toward zero. The
NAN exp_1() of pow.c sets to send a power between 2^-1075 and about
2^-947 to the accurate phase was then -0, and so was the power: CORE-MATH's
pow rounded downward was -0 at (2^-1074)^1 (tests/core_math.cpp, with every
Visual C++ job). GAOL calls CORE-MATH upward, where both are right. They are
read here from their bits, which a load gives in every direction. */
static const union { uint64_t u; double f; } gaol_infinity_bits = { 0x7ff0000000000000ull },
gaol_nan_bits = { 0x7ff8000000000000ull };
#undef INFINITY
#define INFINITY (gaol_infinity_bits.f)
#undef NAN
#define NAN (gaol_nan_bits.f)

/* The number of leading and of trailing zero bits of a 64-bit integer,
undefined at 0 as the builtins of GCC are. _BitScanReverse64 and
_BitScanForward64 are for the 64-bit architectures only (ARM64EC defines
_M_X64): on 32-bit Windows the halves are scanned. __builtin_clzl() and
__builtin_ctzl(), which none of the sources calls, are not given: long has 32
bits on Windows. */
static __forceinline int gaol_clzll(uint64_t x)
{
  #if defined(_M_X64) || defined(_M_ARM64)
  unsigned long i;
  _BitScanReverse64(&i, x);
  return 63 - (int)i;
  #else
  unsigned long i;
  if (_BitScanReverse(&i, (unsigned long)(x >> 32))) return 31 - (int)i;
  _BitScanReverse(&i, (unsigned long)x);
  return 63 - (int)i;
  #endif
}

static __forceinline int gaol_ctzll(uint64_t x)
{
  #if defined(_M_X64) || defined(_M_ARM64)
  unsigned long i;
  _BitScanForward64(&i, x);
  return (int)i;
  #else
  unsigned long i;
  if (_BitScanForward(&i, (unsigned long)x)) return (int)i;
  _BitScanForward(&i, (unsigned long)(x >> 32));
  return 32 + (int)i;
  #endif
}

#define __builtin_clzll(x) gaol_clzll(x)
#define __builtin_ctzll(x) gaol_ctzll(x)

/* The sources use __builtin_mul_overflow on 64-bit operands only, which this
function multiplies as unsigned. In pow.c, the product of the uint64_t m and
the int64_t k is put in k, where the builtin reports an overflow from 2^63
on, and this function from 2^64 on: a k of [2^63, 2^64) is then negative,
and pow.c returns 0 for it all the same, at the test k >> 54 or at the next
product. */
static __forceinline int gaol_mul_overflow_u64(uint64_t a, uint64_t b, uint64_t *r)
{
  const gaol_u128 p = gaol_u128_mul64(a, b);
  *r = gaol_u128_lo(p);
  return gaol_u128_hi(p) != 0;
}

#define __builtin_mul_overflow(a, b, r) gaol_mul_overflow_u64((uint64_t)(a), (uint64_t)(b), (uint64_t *)(r))

#endif /* _MSC_VER */

/*---------------------------------------------------------------------------
ldexp(), with every compiler

erfc.c rounds its subnormal results with __builtin_ldexp(), which compilers
turn into a call to the ldexp() of the C library, and is correctly rounded
only if that ldexp() rounds in the direction in effect. Not all do: the UCRT
of Windows, which Visual C++ and clang-cl call, rounds subnormal results
otherwise, and erfc(x) was the double below its value rounding upward from
x = 26.54 on; the C library of macOS gave 0 for 2^-1127 rounded upward, and
missed 5 (macOS 14 and 15) to 22 (macOS 26) of the checks of
tests/core_math.cpp (GAOL v5). The function of gaol/gaol_ldexp.h is given
instead, everywhere, so that erfc() depends on no C library; the other
sources call __builtin_ldexp() with exact results only (powers of two).
--------------------------------------------------------------------------*/

#include "gaol/gaol_ldexp.h"
#undef __builtin_ldexp
#define __builtin_ldexp(x, n) gaol_ldexp(x, n)

/*---------------------------------------------------------------------------
The warnings GAOL's library is compiled with, which the sources of
CORE-MATH are not written for
--------------------------------------------------------------------------*/

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#endif /* GAOL_CORE_MATH_PORT_H */
