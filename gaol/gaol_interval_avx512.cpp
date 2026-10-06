/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------*
 * The AVX-512 path of the SSE2 intervals (GAOL v5).
 *
 * This file is not meant to be compiled. It is only to be included
 *        by gaol/gaol_interval.cpp, before gaol/gaol_interval_sse.cpp: the
 *        operations of the SSE2 fragments call the functions of this one.
 *
 * The AVX-512 instructions can carry the rounding direction of a
 * floating-point operation in themselves, in the imm8 the EVEX encoding
 * gives them (the embedded rounding): an operation that uses it sets
 * neither the rounding direction of the processor nor the exception
 * flags, the architecture requiring the exception suppression with the
 * embedded rounding. Their bounds are those of the SSE2 path, bit for
 * bit, the rounding rule being the same: checked against it on 500 000
 * intervals of random bounds plus the empty set, on the zero corners of
 * the products and of the quotients, the signs of their bounds included,
 * and on every value the tests of tests/ take (GAOL v5).
 *
 * The embedded rounding honours the modes that flush the subnormal
 * numbers to zero, as the other operations do: flush-to-zero, which
 * flushes a subnormal result (measured on an Intel i7-1185G7, GCC 9.4 and
 * Clang 18: with the mode set, the product of 1e-300 by 1e-20 and the
 * difference of 3e-308 and 2.9e-308 are 0), and denormals-are-zero, which
 * reads a subnormal operand as 0 (measured: the product of 2^-1060 by 1e10
 * is 0 with the mode set). It was thought to ignore flush-to-zero, from a
 * measure the compiler had reordered. The operations of the path clear
 * the modes before they read a bound, as the SSE2 operations clear them at
 * their entry, and compute their results before the modes are set back
 * with GAOL_PRESERVE_ROUNDING (flush_guard below; GAOL v5, point Q of
 * TODO.md).
 *
 * The functions of this file are compiled into the library built for any
 * processor of the architecture, as the SSE2 operations are: only they
 * carry __attribute__((target("avx512f"))), which makes the compiler accept
 * the AVX-512 instructions in them alone, and the code outside them is
 * compiled as before. gaol::init() then takes the AVX-512 path when the
 * program asks for it, with GAOL_PREFER_AVX512, and the processor has the
 * instructions, which __builtin_cpu_supports() tells: +, -, *, / and sqrt
 * never touch the rounding direction nor the flush-to-zero modes on that
 * path, whatever the program left. The divisions and the square roots take
 * it too, the embedded rounding of their 512-bit form being slower only
 * on the processors that halve it (the i7-1185G7 measures 6.0 ns against
 * 1.8 for a division, 8.0 against 2.0 for a square root, where an
 * addition takes 0.9 against 1.8 and a product 0.8 against 1.9). The
 * operations the path does not take are unchanged: the integer powers, the
 * relational divisions, the reverse functions, the reading of a number,
 * which set the direction as they always did.
 *
 * The dispatch of the products and of the divisions below compares the
 * bounds with 0. It reads the sign and the magnitude of their bits, not
 * their values: a subnormal bound, which the denormals-are-zero mode reads
 * as 0, keeps its sign and its magnitude, and the classification of an
 * operand is the same whatever the modes (see operator*=(double) of
 * gaol/gaol_interval_sse.cpp for the comparison that needed the check of
 * the modes). The max of the straddling product, and the zero of a bound
 * by an infinite one, are the two places where the SSE2 path compares
 * values that the mode reads wrong: the former compares the bits of its
 * two candidates, which orders them as their values, both positive in the
 * stored form, and the latter reads the zero of the magnitude and the
 * infinity of the exponent (GAOL v5).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-05 by Jordan NININ
 *--------------------------------------------------------------------------*
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#define GAOL_AVX512_TARGET __attribute__((target("avx512f")))

  // The sign and the zero of a double, from its bits: the operations of the
  // AVX-512 path read them where the SSE2 path compares values, which the
  // denormals-are-zero mode reads wrong for a subnormal bound (a subnormal
  // keeps its sign and its magnitude bits, and 0 is the only value whose
  // magnitude bits are all 0, the zeros of both signs included). The
  // operations with a double operand test them too, where no check clears
  // the mode (GAOL v5).
  GAOL_INLINE std::uint64_t bits_of(double b)
  {
    std::uint64_t bits;
    std::memcpy(&bits, &b, sizeof bits);
    return bits;
  }
  GAOL_INLINE bool bit_negative(double b) // b < 0, false for 0 of both signs
  {
    return (bits_of(b) >> 63) != 0 && (bits_of(b) << 1) != 0;
  }
  GAOL_INLINE bool bit_zero(double b) // b == 0, the zeros of both signs
  {
    return (bits_of(b) << 1) == 0;
  }
  GAOL_INLINE bool bit_positive(double b) // b > 0, false for 0 of both signs
  {
    return (bits_of(b) >> 63) == 0 && (bits_of(b) << 1) != 0;
  }

  namespace {

  // The rounding of the embedded forms used below: upward, with the
  // exception suppression the architecture requires
  enum { gaol_er = _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC };
  // ... and downward, for the lower bound of a square root
  enum { gaol_er_down = _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC };

  // The 512-bit form of the two doubles of the SSE2 intervals: the upper
  // half is undefined, and only the lower half of the result is used
  // Carrying the target too: they wrap the always-inline intrinsics of
  // AVX-512, which the compiler refuses to inline into a function of
  // another target
  GAOL_AVX512_TARGET GAOL_INLINE __m512d lo512(__m128d x) { return _mm512_castpd128_pd512(x); }
  GAOL_AVX512_TARGET GAOL_INLINE __m128d back128(__m512d x) { return _mm512_castpd512_pd128(x); }

  // The two bounds of a stored pair, extracted with the SSE2 instructions
  // rather than subscripted: the compilers do not take the subscript of a
  // vector everywhere (GCC 9 refuses it in a source without a flag)
  GAOL_INLINE double low_of(__m128d x) { return _mm_cvtsd_f64(x); }
  GAOL_INLINE double high_of(__m128d x) { return _mm_cvtsd_f64(_mm_unpackhi_pd(x, x)); }

  /*
    Whether the modes that flush the subnormal numbers to zero are set: the
    sum of a subnormal with +0 is exact in every rounding direction, and 0
    only through denormals-are-zero, which reads the operand as 0, or
    flush-to-zero, which flushes its subnormal result. The embedded
    rounding honours both (see the head of this file): the operations of
    the path clear the modes before they compute, as the SSE2 path does at
    its entry, and the barrier keeps their place after the clearing (see
    GAOL_RND_BARRIER() of gaol/gaol_fpu.h). The modes clear in the common case, one comparison
    and one branch: 0.3 ns measured against the 0.9 of an addition.
  */
  GAOL_INLINE bool flush_modes_set()
  {
    static const volatile double subnormal = 8.0947715414629834e-320;
    return (subnormal + 0.0) == 0.0;
  }
  GAOL_INLINE void clear_flush_modes_if_set()
  {
    if (flush_modes_set()) {
      clear_flush_to_zero();
      GAOL_RND_BARRIER();
    }
  }

  /*
    The flush-to-zero modes of an operation of the path, cleared for its
    computation and left so, as the other operations clear them, or given
    back as they were with GAOL_PRESERVE_ROUNDING, as the SSE2 operations
    give them back (GAOL_RND_LEAVE_SSE of gaol/gaol_fpu.h). The embedded
    rounding honours both modes (see the head of this file), and no compiler
    models them: the operands go through in() after the constructor, and
    the result through out() before the destructor, two empty asm
    statements, which keep their places with respect to the writes of the
    control register and leave the doubles in their registers, at no cost.
    Clang 18 computed the difference of fast_sub() after the destructor had
    set flush-to-zero back, with GAOL_PRESERVE_ROUNDING, and
    [3e-308] - [2.9e-308] was [0, 0] (GAOL v5, point Q of TODO.md,
    tests/rounding_direction.cpp).
  */
  struct flush_guard
  {
    static __m128d in(__m128d x)
    {
      __asm__ __volatile__ ("" : "+x" (x));
      return x;
    }
    static double in(double x)
    {
      __asm__ __volatile__ ("" : "+x" (x));
      return x;
    }
    static __m128d out(__m128d r)
    {
      __asm__ __volatile__ ("" : "+x" (r));
      return r;
    }

#if GAOL_PRESERVE_ROUNDING
    unsigned int saved;
#endif
    flush_guard()
#if GAOL_PRESERVE_ROUNDING
      : saved(0u)
    {
      if (flush_modes_set()) {
        saved = get_flush_modes();
        clear_flush_to_zero();
        GAOL_RND_BARRIER();
      }
    }
    ~flush_guard()
    {
      if (saved != 0u) {
        set_flush_modes(saved);
      }
    }
#else
    {
      clear_flush_modes_if_set();
    }
#endif
  };

  // The infinity of a bound: the exponent all ones, the magnitude null (no
  // bound of a nonempty interval is a NaN, told before), from the bits as
  // the other tests below
  GAOL_INLINE bool bit_infinite(double b)
  {
    return (bits_of(b) & 0x7fffffffffffffffULL) == 0x7ff0000000000000ULL;
  }

  /*
    The signs and the zeros of the two stored bounds of a nonempty interval
    x = <-l, r> (the low half holds -left, the high half right), which the
    dispatch of the divisions below reads where the SSE2 path compares
    left() and right() with 0.
  */
  // left() < 0: the low half -left positive and not 0
  GAOL_INLINE bool stored_left_below_zero(__m128d x)
  {
    return bit_positive(low_of(x));
  }
  // right() < 0, false for 0 of both signs
  GAOL_INLINE bool stored_right_below_zero(__m128d x)
  {
    return bit_negative(high_of(x));
  }

  // The product of two doubles by the embedded rounding
  GAOL_AVX512_TARGET GAOL_INLINE double er_mul(double a, double b)
  {
    return low_of(back128(_mm512_mul_round_pd(_mm512_set1_pd(a), _mm512_set1_pd(b), gaol_er)));
  }

  /*
    The max of the two candidates of the product of two operands that
    strictly straddle zero, which the SSE2 path makes with _mm_max_pd: the
    candidates of each half are positive in the stored form (the low half
    holds max(left*left', right*right'), the high one max(-left*right',
    -left'*right), products of bounds of the same sign), and the bits of
    two positive doubles order them as their values, which the
    denormals-are-zero mode does not change: the max of the integers is the
    max of the doubles (SSE has no max of 64-bit integers before SSE4.1,
    which the library is not compiled with).
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d max_of_positive(__m128d a, __m128d b)
  {
    const double a0 = low_of(a), a1 = high_of(a);
    const double b0 = low_of(b), b1 = high_of(b);
    return _mm_set_pd(bits_of(a1) > bits_of(b1) ? a1 : b1,
                      bits_of(a0) > bits_of(b0) ? a0 : b0);
  }

  /*
    The products of the stored bounds u and v, one of their lanes holding a
    zero bound and the other an infinite one: that lane gives +0, the
    product of such bounds in interval arithmetic, as the SSE2 path
    computes it (products_with_infinities() of gaol/gaol_interval_sse.cpp),
    the zero and the infinity read from the bits, not from the values the
    denormals-are-zero mode reads wrong. The other lanes are the plain
    products.
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d er_products_with_infinities(__m128d u, __m128d v)
  {
    const double ul = low_of(u), uh = high_of(u);
    const double vl = low_of(v), vh = high_of(v);
    return _mm_set_pd((bit_zero(uh) && bit_infinite(vh)) || (bit_zero(vh) && bit_infinite(uh))
                        ? 0.0 : er_mul(uh, vh),
                      (bit_zero(ul) && bit_infinite(vl)) || (bit_zero(vl) && bit_infinite(ul))
                        ? 0.0 : er_mul(ul, vl));
  }

  // The square root of b rounded upward, and the one of a rounded downward,
  // by the embedded rounding: the square root instruction is correctly
  // rounded in every direction, as the mathematical library GAOL calls on
  // the SSE2 path is
  GAOL_AVX512_TARGET GAOL_INLINE double er_sqrt_up(double b)
  {
    return low_of(back128(_mm512_sqrt_round_pd(_mm512_set1_pd(b), gaol_er)));
  }
  GAOL_AVX512_TARGET GAOL_INLINE double er_sqrt_down(double b)
  {
    return low_of(back128(_mm512_sqrt_round_pd(_mm512_set1_pd(b), gaol_er_down)));
  }

}

  /*
    The AVX-512 path of the operations of the intervals, taken when
    gaol::init() has set avx512_arithmetic (GAOL_PREFER_AVX512 and a
    processor that has the instructions). Each of them rounds every
    floating-point operation in itself: no rounding direction to set, no
    flush-to-zero mode to clear, no exception flag raised. Their results
    are those of the SSE2 path, bit for bit (see the head of this file).
  */

  // x + y of the stored bounds, both bounds at once
  GAOL_AVX512_TARGET GAOL_INLINE __m128d fast_add(__m128d x, __m128d y)
  {
    const flush_guard flush;
    return flush_guard::out(back128(_mm512_add_round_pd(lo512(flush_guard::in(x)), lo512(flush_guard::in(y)), gaol_er)));
  }

  // x - y of the stored bounds: the bounds of y exchanged, as the SSE2 path
  GAOL_AVX512_TARGET GAOL_INLINE __m128d fast_sub(__m128d x, __m128d y)
  {
    const flush_guard flush;
    const __m128d a = flush_guard::in(x), b = flush_guard::in(y);
    return flush_guard::out(back128(_mm512_add_round_pd(lo512(a), lo512(_mm_shuffle_pd(b, b, 1)), gaol_er)));
  }

  /*
    x / d of the stored bounds, d a positive double: the bounds divided by
    d lane by lane, as operator/=(double) divides them, whose zero bounds
    keep the sign of their quotient where the dispatch of the quotient by
    the interval [d, d] writes its own. The caller exchanges the bounds and
    negates d for a negative one, as that operator does.
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d fast_div_by(__m128d x, double d)
  {
    const flush_guard flush;
    return flush_guard::out(back128(_mm512_div_round_pd(lo512(flush_guard::in(x)), _mm512_set1_pd(flush_guard::in(d)), gaol_er)));
  }

  // The plain products of two stored pairs
  GAOL_AVX512_TARGET GAOL_INLINE __m128d er_products(__m128d u, __m128d v)
  {
    return back128(_mm512_mul_round_pd(lo512(u), lo512(v), gaol_er));
  }

  /*
    x * y of the stored bounds of two nonempty operands, neither of them
    empty: the dispatch and the products of product_of_bounds() of
    gaol/gaol_interval_sse.cpp, the products by the embedded rounding, the
    classification by the sign bits as that dispatch reads them, and the
    max of the straddling case by the bits (max_of_positive() above). The
    products go through the zero of a bound by an infinite one only where
    there is one, as the SSE2 path does with products_with_infinities(),
    which product_of_bounds() takes for them. A zero operand goes through
    the dispatch as any other, the bounds it gives being those of the SSE2
    path, the signs of its zeros included.
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d mul_unguarded(__m128d x, __m128d y)
  {
    // No shortcut for a zero operand: the dispatch of product_of_bounds()
    // computes the products of the lanes even there, and the signs of the
    // zeros of its results are those of the SSE2 path, which this path
    // keeps bit for bit
    const bool infinities = bit_infinite(low_of(x)) || bit_infinite(high_of(x))
                         || bit_infinite(low_of(y)) || bit_infinite(high_of(y));
    const auto product = [=](__m128d a, __m128d b) -> __m128d {
      return infinities ? er_products_with_infinities(a, b) : er_products(a, b);
    };
    __m128d res = interval::m128_zero;
    const int signs = (_mm_movemask_pd(y) << 2) | _mm_movemask_pd(x);
    switch (signs) {
    case 3:
    case 7:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      // [0, 0]
      res = interval::m128_zero;
      break;
    case 0:
      // (5) [min(ad,bc), max(ac, bd)]: the max of two candidates positive
      // in the stored form
      {
        const __m128d r2 = product(_mm_unpacklo_pd(x, x), _mm_shuffle_pd(y, y, 1));
        const __m128d r4 = product(_mm_unpackhi_pd(x, x), y);
        res = max_of_positive(r2, r4);
      }
      break;
    case 1:
      // (8) [bc, bd]
      res = product(_mm_unpackhi_pd(x, x), y);
      break;
    case 2:
      // (2) [ad, ac]
      res = product(_mm_unpacklo_pd(x, x), _mm_shuffle_pd(y, y, 1));
      break;
    case 4:
      // (6) [ad, bd]
      res = product(x, _mm_unpackhi_pd(y, y));
      break;
    case 5:
      // (9) [ac, bd]
      res = product(_mm_xor_pd(x, interval::lbsignmask), y);
      break;
    case 6:
      // (3) [ad, bc]
      {
        const __m128d r0 = _mm_xor_pd(y, interval::lbsignmask);
        res = product(x, _mm_shuffle_pd(r0, r0, 1));
      }
      break;
    case 8:
      // (4) [bc, ac]
      res = product(_mm_shuffle_pd(x, x, 1), _mm_unpacklo_pd(y, y));
      break;
    case 9:
      // (7) [bc, ad]
      {
        const __m128d r0 = _mm_xor_pd(x, interval::lbsignmask);
        res = product(_mm_shuffle_pd(r0, r0, 1), y);
      }
      break;
    case 10:
      // (1) [bd, ac]
      {
        const __m128d r0 = _mm_shuffle_pd(x, x, 1);
        const __m128d r1 = _mm_xor_pd(r0, interval::lbsignmask);
        res = product(r1, _mm_shuffle_pd(y, y, 1));
      }
      break;
    }
    return res;
  }

  /*
    x / y of the stored bounds of two nonempty operands: the dispatch of
    operator/=() of gaol/gaol_interval_sse.cpp, its comparisons with 0 read
    from the sign and the magnitude bits (stored_left_below_zero() and
    stored_right_below_zero() above), its divisions by the embedded
    rounding. The cases the dispatch does not divide give the stored bounds
    of the intervals it returns: the empty set, the universe,
    [0, +oo] and [-oo, 0].
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d div_unguarded(__m128d x, __m128d y)
  {
    if (stored_right_below_zero(x)) { // [x] N1
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r = _mm_xor_pd(x, interval::lbsignmask);
        return back128(_mm512_div_round_pd(lo512(_mm_shuffle_pd(r, r, 1)), lo512(y), gaol_er)); // N1 N1
      }
      if (bit_zero(high_of(y))) { // [y] N0 or Z
        if (bit_zero(low_of(y))) { // [y] Z
          return interval::m128_nan; // N1 Z
        }
        return _mm_move_sd(interval::m128_infinf,
                           back128(_mm512_div_round_pd(lo512(_mm_shuffle_pd(x, x, 1)), lo512(y), gaol_er))); // N1 N0
      }
      if (stored_left_below_zero(y)) { // [y] M
        return interval::m128_infinf; // N1 M: the universe
      }
      if (bit_zero(low_of(y))) { // [y] P0
        return _mm_move_sd(back128(_mm512_div_round_pd(lo512(x), lo512(y), gaol_er)),
                           interval::m128_infinf); // N1 P0
      }
      // [y] P1
      return back128(_mm512_div_round_pd(lo512(x), lo512(_mm_xor_pd(y, interval::lbsignmask)), gaol_er)); // N1 P1
    }
    if (bit_zero(high_of(x))) { // [x] N0 or Z
      if (bit_zero(low_of(x))) { // [x] Z
        if (bit_zero(low_of(y)) && bit_zero(high_of(y))) { // [y] Z
          return interval::m128_nan; // Z Z
        }
        return interval::m128_zero; // Z (N,M,P)
      }
      // [x] N0
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r1 = _mm_xor_pd(x, interval::lbsignmask);
        return _mm_move_sd(back128(_mm512_div_round_pd(lo512(_mm_shuffle_pd(r1, r1, 1)), lo512(y), gaol_er)),
                           interval::m128_zero); // N0 N1
      }
      if (bit_zero(high_of(y))) { // [y] N0 or Z
        if (bit_zero(low_of(y))) { // [y] Z
          return interval::m128_nan; // N0 Z
        }
        return _mm_set_pd(GAOL_INFINITY, -0.0); // N0 N0: [0, +oo]
      }
      if (stored_left_below_zero(y)) { // [y] M
        return interval::m128_infinf; // N0 M
      }
      if (bit_zero(low_of(y))) { // [y] P0
        return _mm_set_pd(0.0, GAOL_INFINITY); // N0 P0: [-oo, 0]
      }
      // [y] P1
      return _mm_move_sd(interval::m128_zero,
                         back128(_mm512_div_round_pd(lo512(x), lo512(_mm_xor_pd(y, interval::lbsignmask)), gaol_er))); // N0 P1
    }
    if (stored_left_below_zero(x)) { // [x] M
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r1 = _mm_xor_pd(x, interval::lbrbsignmask);
        return back128(_mm512_div_round_pd(lo512(_mm_shuffle_pd(r1, r1, 1)),
                                           lo512(_mm_unpackhi_pd(y, y)), gaol_er)); // M N1
      }
      if (bit_zero(high_of(y))) { // [y] N0 or Z
        if (bit_zero(low_of(y))) { // [y] Z
          return interval::m128_nan; // M Z
        }
        return interval::m128_infinf; // M N0
      }
      if (stored_left_below_zero(y)) { // [y] M
        return interval::m128_infinf; // M M
      }
      if (bit_zero(low_of(y))) { // [y] P0
        return interval::m128_infinf; // M P0
      }
      // [y] P1
      {
        const __m128d r1 = _mm_xor_pd(y, interval::lbsignmask);
        return back128(_mm512_div_round_pd(lo512(x), lo512(_mm_unpacklo_pd(r1, r1)), gaol_er)); // M P1
      }
    }
    if (bit_zero(low_of(x))) { // [x] P0
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(x, x, 1), interval::lbsignmask);
        return _mm_move_sd(interval::m128_zero,
                           back128(_mm512_div_round_pd(lo512(r1), lo512(_mm_shuffle_pd(y, y, 1)), gaol_er))); // P0 N1
      }
      if (bit_zero(high_of(y))) { // [y] N0 or Z
        if (bit_zero(low_of(y))) { // [y] Z
          return interval::m128_nan; // P0 Z
        }
        return _mm_set_pd(0.0, GAOL_INFINITY); // P0 N0: [-oo, 0]
      }
      if (stored_left_below_zero(y)) { // [y] M
        return interval::m128_infinf; // P0 M
      }
      if (bit_zero(low_of(y))) { // [y] P0
        return _mm_set_pd(GAOL_INFINITY, -0.0); // P0 P0: [0, +oo]
      }
      // [y] P1
      {
        const __m128d r1 = _mm_xor_pd(y, interval::lbsignmask);
        return _mm_move_sd(back128(_mm512_div_round_pd(lo512(x), lo512(_mm_shuffle_pd(r1, r1, 1)), gaol_er)),
                          interval::m128_zero); // P0 P1
      }
    }
    // [x] P1
    if (stored_right_below_zero(y)) { // [y] N1
      const __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(x, x, 1), interval::lbrbsignmask);
      const __m128d r2 = _mm_xor_pd(y, interval::lbsignmask);
      return back128(_mm512_div_round_pd(lo512(r1), lo512(_mm_shuffle_pd(r2, r2, 1)), gaol_er)); // P1 N1
    }
    if (bit_zero(high_of(y))) { // [y] N0 or Z
      if (bit_zero(low_of(y))) { // [y] Z
        return interval::m128_nan; // P1 Z
      }
      return _mm_move_sd(back128(_mm512_div_round_pd(lo512(_mm_shuffle_pd(x, x, 1)),
                                                    lo512(_mm_shuffle_pd(y, y, 1)), gaol_er)),
                         interval::m128_infinf); // P1 N0
    }
    if (stored_left_below_zero(y)) { // [y] M
      return interval::m128_infinf; // P1 M
    }
    if (bit_zero(low_of(y))) { // [y] P0
      return _mm_move_sd(interval::m128_infinf,
                         back128(_mm512_div_round_pd(lo512(x), lo512(_mm_shuffle_pd(y, y, 1)), gaol_er))); // P1 P0
    }
    // [y] P1
    {
      const __m128d r1 = _mm_xor_pd(y, interval::lbsignmask);
      return back128(_mm512_div_round_pd(lo512(x), lo512(_mm_shuffle_pd(r1, r1, 1)), gaol_er)); // P1 P1
    }
  }

  /*
    The square root of a nonempty interval, as sqrt() of
    gaol/gaol_interval.cpp computes it on the SSE2 path: the part of the
    interval in [0, +oo] (the lower bound clamped to 0, a -0 kept as the
    root takes it for 0), the root of the lower bound rounded downward and
    the one of the upper bound rounded upward, by the embedded rounding as
    the square root instruction rounds correctly in every direction. The
    emptiness of the intersection is read from the bits: the upper bound
    negative and not a zero, or the clamped lower bound of a magnitude
    above the upper one (both nonnegative, their bits order them as their
    values).
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d sqrt_unguarded(__m128d x)
  {
    const double left = -low_of(x); // the low half holds -left
    const double right = high_of(x);
    const double lpos = bit_negative(left) ? 0.0 : left;
    if (bit_negative(right)) {
      return interval::m128_nan; // the upper bound below 0, and not -0
    }
    if (!bit_zero(lpos) && (bits_of(lpos) << 1) > (bits_of(right) << 1)) {
      return interval::m128_nan; // the lower bound above the upper one
    }
    const double lo = bit_zero(lpos) ? 0.0 : er_sqrt_down(lpos);
    const double hi = er_sqrt_up(right);
    return _mm_set_pd(hi, -lo);
  }

  // x * y, x / y and the square root of x, with the modes that flush the
  // subnormals to zero cleared around their computation (flush_guard above)
  GAOL_AVX512_TARGET __m128d fast_mul(__m128d x, __m128d y)
  {
    const flush_guard flush;
    return flush_guard::out(mul_unguarded(flush_guard::in(x), flush_guard::in(y)));
  }

  GAOL_AVX512_TARGET __m128d fast_div(__m128d x, __m128d y)
  {
    const flush_guard flush;
    return flush_guard::out(div_unguarded(flush_guard::in(x), flush_guard::in(y)));
  }

  GAOL_AVX512_TARGET __m128d fast_sqrt(__m128d x)
  {
    const flush_guard flush;
    return flush_guard::out(sqrt_unguarded(flush_guard::in(x)));
  }
