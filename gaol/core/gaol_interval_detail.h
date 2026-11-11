/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-10)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_detail.h
  \brief  The helpers of the intervals that read no interval, in gaol_detail: the tag of the
          unchecked constructors, the traits of the integers, the unaligned SSE2 storage.

  \author NININ Jordan
  \date   2026-11-10
*/


#ifndef GAOL_INTERVAL_DETAIL_H
#define GAOL_INTERVAL_DETAIL_H

#include <limits>
#include <string>
#include <type_traits>
#include "gaol/gaol_config.h"
#include "gaol/gaol_port.h"

/*
  What gaol/core/gaol_interval_core.h and the headers it includes in the
  middle of its code (gaol_interval_sse.h, gaol_interval_neon.h and
  gaol_interval_fpu.h) use, and that need no interval: the tag of the
  constructors that make no check, the traits of the integers that GAOL takes
  as they are, the storage of the SSE2 bounds that is not aligned,
  GAOL_FPU_SCALAR() and keep_ordered(), which keep GCC from compiling the
  comparisons of the bounds of an empty interval into signaling ones. The
  helpers are in gaol_detail, with the other helpers of the library, and not
  part of the interface. This file is what gaol_interval_core.h held before
  its class and after it (GAOL v5).
*/

// __m128d and _mm_storeu_pd() of unaligned_xmm: gaol_port.h includes
// <emmintrin.h> for Visual C++ alone, and this header compiles alone (GAOL v5)
#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_SSE2_UNALIGNED_STORAGE
#  include <emmintrin.h>
#endif

/* A loop of comparisons of bounds that GCC vectorizes raises the
   invalid-operation exception on the NaN bounds of an empty interval, with
   the FPU intervals, wherever the vector comparisons signal or where it
   computes for every element a comparison that follows a test of the empty
   set: GCC 9.4 and 13 at -O3 with -mfma (which turns AVX on, and which the
   three builds give where the processor has FMA), and GCC 13 to 15 for 32-bit
   x86 with SSE2 alone (MinGW-w64), make the quiet comparisons signaling
   predicates (vcmpnlepd, cmpnlepd...); GCC 14 for POWER8 uses xvcmpgedp and
   xvcmpgtdp, which signal on a quiet NaN; GCC 11 to 15 for 64-bit ARM keep
   the quiet ones quiet, but compute the second and third comparisons of the
   constructor (fcmgt) for an empty element too. Loops of is_empty(), of the
   relations, of constructions and of floor() raised it there
   (tests/rounding_direction.cpp, in the continuous integration). With GCC and
   the FPU intervals, the bounds is_empty() compares, those of the relations
   that do not call it, and the doubles given to the constructors therefore go
   through an empty asm statement, which leaves them in their register at no
   cost but keeps GCC from vectorizing these loops (GAOL v5), with a register
   of the floating-point unit of each processor, memory elsewhere; not the
   constants, which GCC still folds. The scalar code then makes the first,
   quiet comparison before the others. Clang 18 keeps such loops scalar, and
   the SSE2 intervals were not concerned. */
#if !GAOL_USING_SSE2_INSTRUCTIONS && defined(__GNUC__) && !defined(__clang__)
#  if (defined(__x86_64__) || defined(__i386__)) && defined(__SSE2_MATH__)
#    define GAOL_FPU_SCALAR_ASM(x) __asm__ ("" : "+x" (x))
#  elif defined(__aarch64__) || (defined(__arm__) && defined(__ARM_FP) && (__ARM_FP & 8))
#    define GAOL_FPU_SCALAR_ASM(x) __asm__ ("" : "+w" (x)) // a register that holds a double
#  elif (defined(__powerpc__) || defined(__PPC__)) && !defined(_SOFT_FLOAT) && !defined(__NO_FPRS__)
#    define GAOL_FPU_SCALAR_ASM(x) __asm__ ("" : "+d" (x))
#  elif defined(__s390__) || defined(__loongarch_double_float) \
      || (defined(__riscv) && defined(__riscv_flen) && __riscv_flen >= 64) \
      || (defined(__mips__) && defined(__mips_hard_float))
#    define GAOL_FPU_SCALAR_ASM(x) __asm__ ("" : "+f" (x))
#  else
#    define GAOL_FPU_SCALAR_ASM(x) __asm__ ("" : "+m" (x))
#  endif
#  define GAOL_FPU_SCALAR(x) do { if (!__builtin_constant_p(x)) { GAOL_FPU_SCALAR_ASM(x); } } while (0)
#else
#  define GAOL_FPU_SCALAR(x) ((void)0)
#endif

namespace gaol_detail {

#if GAOL_USING_SSE2_INSTRUCTIONS && GAOL_SSE2_UNALIGNED_STORAGE
  /*
    The two bounds of an SSE2 interval as two doubles in memory, alignment 8,
    for the targets whose memory of new is aligned on 8 bytes and whose compiler
    cannot be told to ask for more (Visual C++ and clang-cl on 32-bit Windows;
    see GAOL_SSE2_UNALIGNED_STORAGE of gaol/gaol_config.h): a __m128d member
    would put 16 on every object that holds an interval, and a std::vector of
    them would stop on its first movaps. The register is loaded and stored with
    movupd, which costs no more than movapd on the processors since Nehalem
    where the data does not cross a cache line (nothing measurable on an Intel
    i7-1185G7). It converts to and from a __m128d, so that the code reads and
    writes it as it does the register (GAOL v5)
  */
  struct unaligned_xmm {
    double lanes[2];
    unaligned_xmm() {}
    unaligned_xmm(const __m128d& v) { _mm_storeu_pd(lanes, v); }
    operator __m128d() const { return _mm_loadu_pd(lanes); }
    unaligned_xmm& operator=(const __m128d& v) { _mm_storeu_pd(lanes, v); return *this; }
  };
#endif

  /*
    The tag of the constructors of an interval from the two lanes of a
    register that make no check (GAOL v5, point AC of TODO.md): the code of
    GAOL gives them the lanes it computed, which are right, where the
    constructor of the interface refuses the lanes of [+oo] and [-oo]
  */
  struct unchecked_bounds_t {};
  const unchecked_bounds_t unchecked_bounds = unchecked_bounds_t();

  /*
    The integers GAOL takes as they are (GAOL v5, point D.21 of TODO.md):
    the integer types but bool. An integer given where GAOL takes a double
    was converted to a double, the nearest one or the one of the rounding
    direction, which an integer beyond 2^53 is not: interval(2^53 + 1) was
    [2^53, 2^53] or [2^53 + 2, 2^53 + 2], neither of which contains it, and
    [1, 2] - (2^53 + 1) did not contain [1 - n, 2 - n] in any rounding
    direction. The constructors, =, &=, |=, the arithmetic operators, the
    relations and set_contains() therefore have overloads for them, which
    take the tightest interval containing the integer.
  */
  template <class T>
  struct is_integer
    : std::integral_constant<bool, std::is_integral<T>::value
                                   && !std::is_same<typename std::remove_cv<T>::type, bool>::value> {};

  template <class T>
  using if_integer = typename std::enable_if<is_integer<T>::value, int>::type;

  // Whether every value of the integer type T is a double: the types of 53
  // bits or less (int, short, char...)
  template <class T>
  struct integer_type_is_double
    : std::integral_constant<bool, std::numeric_limits<T>::digits <= std::numeric_limits<double>::digits> {};

  // n < 0, without comparing an unsigned n with 0
  template <class T>
  GAOL_INLINE bool integer_is_negative(T n, std::true_type) { return n < 0; }
  template <class T>
  GAOL_INLINE bool integer_is_negative(T, std::false_type) { return false; }
  template <class T>
  GAOL_INLINE bool integer_is_negative(T n) { return integer_is_negative(n, std::is_signed<T>()); }

  // Whether the integer n is a value of the integer type R
  template <class R, class T>
  GAOL_INLINE bool integer_fits(T n)
  {
    const R r = static_cast<R>(n);
    return static_cast<T>(r) == n && integer_is_negative(r) == integer_is_negative(n);
  }

  // Whether n is a double for certain: every value of a type of 53 bits or
  // less, |n| <= 2^53 for a wider one. A wider n may be one too (2^60),
  // where interval(n) is a point as well
  template <class T>
  GAOL_INLINE bool integer_is_double(T, std::true_type) { return true; }
  template <class T>
  GAOL_INLINE bool integer_is_double(T n, std::false_type)
  {
    return integer_fits<long long>(n) && static_cast<long long>(n) <= 9007199254740992LL
      && static_cast<long long>(n) >= -9007199254740992LL;
  }
  template <class T>
  GAOL_INLINE bool integer_is_double(T n) { return integer_is_double(n, integer_type_is_double<T>()); }

  /*
    The doubles around the integer n of a type of more than 53 bits,
    computed on integers alone, whatever the rounding direction and the
    precision the conversions are made in: |n|, unsigned, keeps its 53
    highest bits, t, which is a double, and the double above |n| is
    t + 2^s, s being the number of bits dropped, which the addition of the
    two doubles t and 2^s gives exactly, being a double (2^digits for the
    largest value of the type). Not static_cast<double>(n) converted back to
    the type of n and compared with n: GCC 12 and 13 for 32-bit x86 converted
    it on the x87 unit, in extended precision, without rounding it to a
    double in between, and found n again, taking 2^53 + 1 for a double
    (Debian 12 i386, GAOL v5).
  */
  template <class T>
  GAOL_INLINE void integer_neighbours(T n, double& below, double& above)
  {
    typedef typename std::make_unsigned<T>::type U;
    const bool negative = integer_is_negative(n);
    const U u = negative ? static_cast<U>(U(0) - static_cast<U>(n)) : static_cast<U>(n);
    double low, high;
    if (u <= static_cast<U>(9007199254740992ULL)) {
      low = high = static_cast<double>(u);
    } else {
      int length = 0;
      for (U v = u; v != 0; v = static_cast<U>(v >> 1)) {
        ++length;
      }
      const int s = length - std::numeric_limits<double>::digits;
      const U t = static_cast<U>(static_cast<U>(u >> s) << s);
      low = static_cast<double>(t);
      high = t == u ? low : low + static_cast<double>(static_cast<U>(U(1) << s));
    }
    below = negative ? -high : low;
    above = negative ? -low : high;
  }

  /*
    The largest double at most n and the smallest one at least n, whatever
    the rounding direction: n itself where it is a double, the two doubles
    around it otherwise, as interval(n) takes them
  */
  template <class T>
  GAOL_INLINE double double_below(T n, std::true_type) { return static_cast<double>(n); }
  template <class T>
  GAOL_INLINE double double_below(T n, std::false_type)
  {
    double below, above;
    integer_neighbours(n, below, above);
    return below;
  }
  template <class T, if_integer<T> = 0>
  GAOL_INLINE double double_below(T n) { return double_below(n, integer_type_is_double<T>()); }

  template <class T>
  GAOL_INLINE double double_above(T n, std::true_type) { return static_cast<double>(n); }
  template <class T>
  GAOL_INLINE double double_above(T n, std::false_type)
  {
    double below, above;
    integer_neighbours(n, below, above);
    return above;
  }
  template <class T, if_integer<T> = 0>
  GAOL_INLINE double double_above(T n) { return double_above(n, integer_type_is_double<T>()); }

  // A bound given as a double (or a float) is that double
  GAOL_INLINE double double_below(double d) { return d; }
  GAOL_INLINE double double_above(double d) { return d; }

  // The bounds of interval(a, b) with an integer bound: integers, and
  // doubles or floats
  template <class T>
  struct is_bound
    : std::integral_constant<bool, is_integer<T>::value || std::is_same<T, double>::value
                                   || std::is_same<T, float>::value> {};

  template <class A, class B>
  using if_integer_bounds = typename std::enable_if<(is_integer<A>::value || is_integer<B>::value)
                                                     && is_bound<A>::value && is_bound<B>::value, int>::type;

  /*
    Whether a <= b, the bounds of interval(a, b) with an integer among them,
    compared as the numbers they are: interval(a, b) takes the double below
    a and the double above b, which are in order where a > b lie between
    the same two doubles (interval(2^53 + 1, 2^53) would be [2^53, 2^53]).
    An integer a is at most a double b if and only if the double above a is,
    and a double a at most an integer b if and only if a is at most the
    double below b; a double is compared quietly, a NaN raising nothing.
    Two integers are compared as integers, in the unsigned type of their
    common type where neither is negative.
  */
  template <class A, class B>
  GAOL_INLINE bool bounds_in_order(A a, B b, std::true_type, std::false_type)
  {
    return quiet_less_equal(double_above(a), static_cast<double>(b));
  }
  template <class A, class B>
  GAOL_INLINE bool bounds_in_order(A a, B b, std::false_type, std::true_type)
  {
    return quiet_less_equal(static_cast<double>(a), double_below(b));
  }
  template <class A, class B>
  GAOL_INLINE bool bounds_in_order(A a, B b, std::true_type, std::true_type)
  {
    if (integer_is_negative(a) != integer_is_negative(b)) {
      return integer_is_negative(a);
    }
    typedef typename std::common_type<A, B>::type C;
    typedef typename std::make_unsigned<C>::type U;
    return integer_is_negative(a) ? static_cast<C>(a) <= static_cast<C>(b) : static_cast<U>(a) <= static_cast<U>(b);
  }
  // Bounds that are all doubles (a double, a float or an integer of 53 bits
  // or less, every int) need no comparison here: interval(double, double)
  // compares them as they are
  template <class T>
  struct bound_is_double
    : std::integral_constant<bool, !is_integer<T>::value || integer_type_is_double<T>::value> {};
  template <class A, class B>
  GAOL_INLINE bool bounds_in_order(A, B, std::false_type) { return true; }
  template <class A, class B>
  GAOL_INLINE bool bounds_in_order(A a, B b, std::true_type) { return bounds_in_order(a, b, is_integer<A>(), is_integer<B>()); }
  template <class A, class B>
  GAOL_INLINE bool bounds_in_order(A a, B b)
  {
    return bounds_in_order(a, b, std::integral_constant<bool, !(bound_is_double<A>::value && bound_is_double<B>::value)>());
  }

  /*
    The string s in an error message, its NUL characters written \0: what()
    is a C string, which stopped at the first, so that the message of
    "1\0junk" read as a refusal of 1 (GAOL v5)
  */
  inline std::string text_in_message(const std::string& s)
  {
    std::string text;
    for (const char c : s) {
      if (c == '\0') {
        text += "\\0";
      } else {
        text += c;
      }
    }
    return text;
  }

#if (defined(__arm__) && !defined(__aarch64__)) || defined(_ARCH_PWR9)
  /*
    On 32-bit ARM and on POWER9, the two doubles go through an empty asm
    statement, which leaves them in their register at no cost, once a
    std::isunordered() of them told they are no NaN, in is_empty() and in
    the constructors of the FPU intervals (GAOL v5): GCC can then not merge
    that test and the comparison after it back into a single "unordered or
    greater", which it reverses into a signaling comparison there (see
    is_empty()).
  */
  GAOL_INLINE void keep_ordered(double& l, double& r)
  {
#  if defined(__arm__) && defined(__ARM_FP) && (__ARM_FP & 8)
    __asm__ ("" : "+w" (l), "+w" (r)); // a VFP register that holds a double
#  elif defined(__arm__)
    __asm__ ("" : "+r" (l), "+r" (r));
#  else
    __asm__ ("" : "+d" (l), "+d" (r));
#  endif
  }
#endif
} // namespace gaol_detail

#endif // GAOL_INTERVAL_DETAIL_H
