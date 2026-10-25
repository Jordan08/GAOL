/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------*
 * The AVX-512 path of the SSE2 intervals (GAOL v5).
 *
 * This file is not meant to be compiled. It is only to be included
 *        by gaol/gaol_interval_core.cpp, before gaol/gaol_interval_sse.cpp: the
 *        operations of the SSE2 fragments call the functions of this one.
 *
 * The AVX-512 instructions can carry the rounding direction of a
 * floating-point operation in themselves, in the imm8 the EVEX encoding
 * gives them (the embedded rounding): an operation that uses it sets
 * neither the rounding direction of the processor nor the exception
 * flags, the architecture requiring the exception suppression with the
 * embedded rounding. Their bounds are those of the SSE2 path, bit for
 * bit, the rounding rule being the same, but for the sign of a zero bound
 * in one case: x * d and x *= d of an interval x whose zero bounds have
 * different signs, [+0, -0], which are [-0, +0] here and [+0, -0] with the
 * SSE2 path, +0 and -0 being the same number. Checked against the SSE2
 * path on 500 000 intervals of random bounds plus the empty set, on the
 * zero corners of the products and of the quotients, the signs of their
 * bounds included, and on every value the tests of tests/ take, and on
 * every result of the operations of intervals and with a double on the
 * intervals of two bounds among 0 of either sign, +-1, +-2.5, +-3,
 * +-1e-310, +-1e300 and +-oo (GAOL v5).
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
 * The functions of this file are compiled into the library where
 * GAOL_AVX512_ARITHMETIC is (gaol_configuration.h): GAOL_PREFER_AVX512, a
 * compiler that takes them, and a machine building that runs them. Only
 * they, and the functions of the integer powers that call theirs
 * (GAOL_POWN_TARGET), carry __attribute__((target("avx512f"))), which makes the compiler
 * accept the AVX-512 instructions in them alone, and the code outside them
 * is compiled as before; the library then needs a processor with the
 * instructions. +, -, *, / and sqrt take this path, and so do sqr() and the
 * integer powers of gaol_pown() (pown_large_er() of this file; GAOL v5, point
 * ZZ of TODO.md), with no flag to
 * test as they run (gaol::init() set one, which every operation tested, until point
 * AA of TODO.md): they never touch the rounding direction, whatever the
 * program left, and clear the modes that flush the subnormals to zero only
 * where they are set, as the other operations do. The divisions and the square
 * roots take it as scalar instructions, a division of the two bounds being two
 * of them (er_div2() below), the embedded rounding of their 512-bit form being
 * slower on the processors that halve it (the i7-1185G7 measures 6.0 ns against
 * 1.8 for a division, 8.0 against 2.0 for a square root, where an addition takes
 * 0.9 against 1.8 and a product 0.8 against 1.9). The operations the path does
 * not take are unchanged: the integer powers beyond the range of their exact
 * products, and the exponents 0, 1 and -1, the relational divisions, the
 * reverse functions, the reading of a number, which set the direction as they
 * always did.
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

/* The target of the functions (GAOL_AVX512_TARGET) and GAOL_NOINLINE are defined
   in gaol/gaol_interval_core.cpp, before the generic part of the integer powers
   (the templates pown_products() and pown_powers(), whose functions carry the
   target too), which this file needs before its pown_er_ops. */

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
    GAOL_RND_BARRIER() of gaol/gaol_fpu.h). The modes clear in the common
    case, one comparison and one branch: 0.3 ns measured against the 0.9 of
    an addition.
  */
  GAOL_INLINE bool flush_modes_set()
  {
    static const volatile double subnormal = 8.0947715414629834e-320;
    return (subnormal + 0.0) == 0.0;
  }
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_CLEAR_FLUSH() clear_flush_to_zero()
#  define GAOL_SAVE_AND_CLEAR_FLUSH(saved) do { (saved) = get_flush_modes(); clear_flush_to_zero(); } while (0)
#  define GAOL_SET_FLUSH(modes) set_flush_modes(modes)
#else
#  ifdef _ISO_VOLATILE
#    error "The AVX-512 path of Visual C++ orders its operations by volatile accesses, which /volatile:iso (_ISO_VOLATILE) does not order with the other memory accesses and the calls: compile with /volatile:ms, the default for x86 and x64, or with -DGAOL_PREFER_AVX512=OFF (see the head of flush_guard)"
#  endif
  // The reads and writes of the control register in functions that are not
  // inlined, for Visual C++ (see flush_guard below); the modes the guard gives
  // back are the value the call returns, which no move of the read past the
  // call can change
  GAOL_NOINLINE void clear_flush_to_zero_call() { clear_flush_to_zero(); }
  GAOL_NOINLINE unsigned int save_and_clear_flush_call()
  {
    const unsigned int modes = get_flush_modes();
    clear_flush_to_zero();
    return modes;
  }
#  if GAOL_PRESERVE_ROUNDING
  GAOL_NOINLINE void set_flush_modes_call(unsigned int modes) { set_flush_modes(modes); }
#  endif
#  define GAOL_CLEAR_FLUSH() clear_flush_to_zero_call()
#  define GAOL_SAVE_AND_CLEAR_FLUSH(saved) ((saved) = save_and_clear_flush_call())
#  define GAOL_SET_FLUSH(modes) set_flush_modes_call(modes)
#endif
  GAOL_INLINE void clear_flush_modes_if_set()
  {
    if (flush_modes_set()) {
      GAOL_CLEAR_FLUSH();
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

    Visual C++ has no asm statement on x64, and its optimizer takes an
    intrinsic with an embedded rounding for a computation of its operands
    alone, which it moves past a call that writes the control register:
    with GAOL_PRESERVE_ROUNDING and /arch:AVX2, add_bounds() called the
    destructor, which sets the modes back, before the vaddpd (Compiler
    Explorer, the latest Visual C++ of October 2026). The standard orders
    nothing there, the control register not being observable behavior: what
    orders the operations is the data they depend on and the volatile
    accesses, which Visual C++ keeps in place with respect to the other
    memory accesses and to the calls (/volatile:ms, its default for x86 and
    x64), and which the assembler of its versions 19.40 to 19.51 shows in
    the order wanted (the calls, then the volatile read, the operation, the
    volatile write, the calls of the destructor). The operands go through
    in(), an exclusive or with a zero read from volatile memory, which leaves
    their bits as they are: the read stays after the constructor, whose reads
    and writes of the control register are in functions that are not inlined
    (the modes found are the value one returns), and the operation after the
    read. The result goes through out() before the destructor, written to
    volatile memory and read again as GAOL_RND_KEEP() does for the SSE2
    operations, where the modes are given back; where they are not, there is
    nothing to order. A volatile access costs a store and a load, a few
    cycles against the dozens of ns of the writes of MXCSR that the path
    spares (GAOL v5).
  */
  struct flush_guard
  {
#if defined(__GNUC__) || defined(__clang__)
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
#else
    // 0 read from volatile memory, a read the compiler keeps after the calls
    // before it
    static __m128d zero_read()
    {
      static const volatile double zero = 0.0;
      return _mm_set1_pd(zero);
    }
    static __m128d in(__m128d x)
    {
      return _mm_xor_pd(x, zero_read());
    }
    static double in(double x)
    {
      return _mm_cvtsd_f64(_mm_xor_pd(_mm_set_sd(x), zero_read()));
    }
    static __m128d out(__m128d r)
    {
#  if GAOL_PRESERVE_ROUNDING
      return rnd_keep(r);
#  else
      return r;
#  endif
    }
#endif

#if GAOL_PRESERVE_ROUNDING
    unsigned int saved;
#endif
    flush_guard()
#if GAOL_PRESERVE_ROUNDING
      : saved(0u)
    {
      if (flush_modes_set()) {
        GAOL_SAVE_AND_CLEAR_FLUSH(saved);
        GAOL_RND_BARRIER();
      }
    }
    ~flush_guard()
    {
      if (saved != 0u) {
        GAOL_SET_FLUSH(saved);
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

  // The products of the two doubles of a and the two of b, by the embedded
  // rounding (the upper half of the 512 bits is not used)
  GAOL_AVX512_TARGET GAOL_INLINE __m128d er_mul2(__m128d a, __m128d b)
  {
    return back128(_mm512_mul_round_pd(lo512(a), lo512(b), gaol_er));
  }

  // The product of two doubles by the embedded rounding
  GAOL_AVX512_TARGET GAOL_INLINE double er_mul(double a, double b)
  {
    return low_of(back128(_mm512_mul_round_pd(_mm512_set1_pd(a), _mm512_set1_pd(b), gaol_er)));
  }

  /*
    The quotients of the lanes of a by those of b, by scalar divisions with the
    embedded rounding, as div_bounds() of gaol/gaol_interval_sse.cpp has them
    (GAOL v5): the packed division takes the embedded rounding on its 512-bit
    form only, which is a division of eight doubles (16 cycles of throughput
    on Ice Lake and Tiger Lake, uops.info), where two scalar ones take 2 x 4.
    The quotient of a lane is the same bits whichever form divides it.
    er_div_low() divides the low lanes, the high lane of its result being the
    one of a; er_div_high() divides the high ones, its result in the low lane:
    the cases of div_unguarded() that keep one lane of the quotients divide
    that one. On an Intel i7-1185G7 (Clang 21, GAOL_PRESERVE_ROUNDING, medians
    of 7 runs alternated with the 512-bit form) x / d takes 2.8 ns rather than
    3.7 for d > 0 (3.0 rather than 3.7 for d < 0), and x /= d the same; the
    divisors with a zero of div_unguarded() (5.5 ns) and pow(x, -3) (9.1 ns)
    stay within the noise.
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d er_div_low(__m128d a, __m128d b)
  {
    return _mm_div_round_sd(a, b, gaol_er);
  }
  GAOL_AVX512_TARGET GAOL_INLINE __m128d er_div_high(__m128d a, __m128d b)
  {
    return _mm_div_round_sd(_mm_unpackhi_pd(a, a), _mm_unpackhi_pd(b, b), gaol_er);
  }
  GAOL_AVX512_TARGET GAOL_INLINE __m128d er_div2(__m128d a, __m128d b)
  {
    return _mm_unpacklo_pd(er_div_low(a, b), er_div_high(a, b));
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
    const __m128d v = _mm_set_sd(b);
    return low_of(_mm_sqrt_round_sd(v, v, gaol_er));
  }
  GAOL_AVX512_TARGET GAOL_INLINE double er_sqrt_down(double b)
  {
    const __m128d v = _mm_set_sd(b);
    return low_of(_mm_sqrt_round_sd(v, v, gaol_er_down));
  }

}

  /*
    The AVX-512 path of the operations of the intervals, taken where
    GAOL_AVX512_ARITHMETIC is (GAOL_PREFER_AVX512, and a machine building
    that runs the instructions). Each of them rounds every
    floating-point operation in itself: no rounding direction to set, and
    no exception flag raised by the instructions; the modes that flush the
    subnormals to zero, which the embedded rounding honours, are cleared
    where they are set (flush_guard above), whose check raises the flags of
    the check of the other operations (doc/using.md). Their results
    are those of the SSE2 path, bit for bit (see the head of this file).
  */

  // x + y of the stored bounds, both bounds at once
  GAOL_AVX512_TARGET GAOL_INLINE __m128d fast_add(__m128d x, __m128d y)
  {
    const flush_guard flush;
    const __m128d a = flush_guard::in(x), b = flush_guard::in(y);
    return flush_guard::out(back128(_mm512_add_round_pd(lo512(a), lo512(b), gaol_er)));
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
    const __m128d a = flush_guard::in(x);
    const double divisor = flush_guard::in(d);
    return flush_guard::out(er_div2(a, _mm_set1_pd(divisor)));
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
    divide_general() of gaol/gaol_interval_sse.cpp, its comparisons with 0 read
    from the sign and the magnitude bits (stored_left_below_zero() and
    stored_right_below_zero() above), its divisions by the embedded
    rounding. The cases the dispatch does not divide give the stored bounds
    of the intervals it returns: the empty set, the universe,
    [0, +oo] and [-oo, 0].
  */
  GAOL_AVX512_TARGET GAOL_INLINE __m128d div_unguarded(__m128d x, __m128d y)
  {
    // fast_div() calls it, and div_bounds() calls fast_div() for a divisor with a
    // zero only: the cases of a divisor without one (N1 N1, P1 P1...) are not
    // reached through the interface, and keep the table of divide_general()
    if (stored_right_below_zero(x)) { // [x] N1
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r = _mm_xor_pd(x, interval::lbsignmask);
        return er_div2(_mm_shuffle_pd(r, r, 1), y); // N1 N1
      }
      if (bit_zero(high_of(y))) { // [y] N0 or Z
        if (bit_zero(low_of(y))) { // [y] Z
          return interval::m128_nan; // N1 Z
        }
        return _mm_move_sd(interval::m128_infinf, er_div_low(_mm_shuffle_pd(x, x, 1), y)); // N1 N0
      }
      if (stored_left_below_zero(y)) { // [y] M
        return interval::m128_infinf; // N1 M: the universe
      }
      if (bit_zero(low_of(y))) { // [y] P0
        return _mm_unpacklo_pd(interval::m128_infinf, er_div_high(x, y)); // N1 P0
      }
      // [y] P1
      return er_div2(x, _mm_xor_pd(y, interval::lbsignmask)); // N1 P1
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
        return _mm_unpacklo_pd(interval::m128_zero, er_div_high(_mm_shuffle_pd(r1, r1, 1), y)); // N0 N1
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
      return _mm_move_sd(interval::m128_zero, er_div_low(x, _mm_xor_pd(y, interval::lbsignmask))); // N0 P1
    }
    if (stored_left_below_zero(x)) { // [x] M
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r1 = _mm_xor_pd(x, interval::lbrbsignmask);
        return er_div2(_mm_shuffle_pd(r1, r1, 1), _mm_unpackhi_pd(y, y)); // M N1
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
        return er_div2(x, _mm_unpacklo_pd(r1, r1)); // M P1
      }
    }
    if (bit_zero(low_of(x))) { // [x] P0
      if (stored_right_below_zero(y)) { // [y] N1
        const __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(x, x, 1), interval::lbsignmask);
        return _mm_move_sd(interval::m128_zero, er_div_low(r1, _mm_shuffle_pd(y, y, 1))); // P0 N1
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
        return _mm_unpacklo_pd(interval::m128_zero, er_div_high(x, _mm_shuffle_pd(r1, r1, 1))); // P0 P1
      }
    }
    // [x] P1
    if (stored_right_below_zero(y)) { // [y] N1
      const __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(x, x, 1), interval::lbrbsignmask);
      const __m128d r2 = _mm_xor_pd(y, interval::lbsignmask);
      return er_div2(r1, _mm_shuffle_pd(r2, r2, 1)); // P1 N1
    }
    if (bit_zero(high_of(y))) { // [y] N0 or Z
      if (bit_zero(low_of(y))) { // [y] Z
        return interval::m128_nan; // P1 Z
      }
      return _mm_unpacklo_pd(interval::m128_infinf,
                             er_div_high(_mm_shuffle_pd(x, x, 1), _mm_shuffle_pd(y, y, 1))); // P1 N0
    }
    if (stored_left_below_zero(y)) { // [y] M
      return interval::m128_infinf; // P1 M
    }
    if (bit_zero(low_of(y))) { // [y] P0
      return _mm_move_sd(interval::m128_infinf, er_div_low(x, _mm_shuffle_pd(y, y, 1))); // P1 P0
    }
    // [y] P1
    {
      const __m128d r1 = _mm_xor_pd(y, interval::lbsignmask);
      return er_div2(x, _mm_shuffle_pd(r1, r1, 1)); // P1 P1
    }
  }

  /*
    The square root of a nonempty interval, as sqrt() of
    gaol/gaol_interval_core.cpp computes it on the SSE2 path: the part of the
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

  /*
    x op= d on the AVX-512 path, d a double: the four operators with a double
    (gaol/gaol_interval_sse.cpp) call them last, as tail calls: past the tests of
    their operand, each operator is a jump to its function, which saves no
    register (Clang 18 and GCC 13). Called there and followed by the store of
    their result, fast_add() and the others made Clang 18 save a register and
    set up a stack frame in these operators, which the SSE2 path paid too when
    gaol::init() chose the path as the program ran, and x + 4 took 3.0 ns
    rather than 2.5 there (Clang 18, Intel Xeon E3-1220 v6, the library and
    the program built with -mbranches-within-32B-boundaries; GAOL v5, point
    AA of TODO.md). The operations between two intervals are the functions
    add_bounds() and the others, which the header calls with the bounds in a
    register (point ZZ of TODO.md).

    They take the double by value, which is passed in a register by the
    calling conventions of x86-64 Linux, macOS and Windows, where a __m128d
    is passed by a hidden reference to a copy in the frame of the caller on
    Windows (MinGW-w64, MSYS2), and no call taking one can be a tail call.
    Their result goes through opaque(), an empty asm
    statement, which the compiler does not see through, at no cost: seeing
    them return their first argument, Clang 18 had the operator return that
    argument, kept across the call in a saved register, rather than the
    result of the call, which was then no tail call.
  */
  GAOL_INLINE interval& opaque(interval& x)
  {
    interval* p = &x;
#if defined(__GNUC__) || defined(__clang__)
    __asm__ ("" : "+r" (p));
#endif
    return *p;
  }

  // x + [d, d], d finite: <-l, r> + <-d, d>. x - d is x + (-d)
  GAOL_AVX512_TARGET GAOL_NOINLINE interval& fast_add_assign(interval& x, double d)
  {
    x.get_xmminterval() = fast_add(x.get_xmminterval(), _mm_set_pd(d, -d));
    return opaque(x);
  }

  // x * [d, d], d finite and nonzero, through the dispatch of fast_mul()
  GAOL_AVX512_TARGET GAOL_NOINLINE interval& fast_mul_assign(interval& x, double d)
  {
    x.get_xmminterval() = fast_mul(x.get_xmminterval(), _mm_set_pd(d, -d));
    return opaque(x);
  }

  // x / d, d finite and nonzero: the bounds exchanged and d negated for a
  // negative one, as operator/=(double) does
  GAOL_AVX512_TARGET GAOL_NOINLINE interval& fast_div_assign(interval& x, double d)
  {
    const __m128d b = x.get_xmminterval();
    if (bit_negative(d)) {
      x.get_xmminterval() = fast_div_by(_mm_shuffle_pd(b, b, 1), -d); // <r/(-d), -l/(-d)>
    } else {
      x.get_xmminterval() = fast_div_by(b, d); // <-l/d, r/d>
    }
    return opaque(x);
  }

#if GAOL_POWN_ER
  /*
    The two lanes of the kernel of pown_products() rounded upward by the
    embedded rounding of AVX-512 (_mm512_*_round_pd on the 512 bits of which
    the upper ones are not used, as er_mul2() does; two scalar divisions for
    div(), see er_div2()) rather than by the
    direction of MXCSR: the same operations in the same order as the ones of
    the SSE2 register, so the same bits, with no write of MXCSR, which cost 3
    to 6 ns an operation with GAOL_PRESERVE_ROUNDING (GAOL v5). A struct, so
    that no operation of a register of the other kind is taken for one of
    this: the compiler refuses it, and would otherwise round it by the
    direction of MXCSR, which is not upward.
  */
  struct pown_er_lanes { __m128d v; };
  struct pown_er_ops {
    typedef pown_er_lanes lanes;
    typedef __m128d rests;
    static GAOL_POWN_INLINE_T lanes mul(lanes a, lanes b)
    {
      const lanes r = { er_mul2(a.v, b.v) };
      return r;
    }
    static GAOL_POWN_INLINE_T lanes fms(lanes a, lanes b, lanes c)
    {
      const lanes r = { back128(_mm512_fmsub_round_pd(lo512(a.v), lo512(b.v), lo512(c.v), gaol_er)) };
      return r;
    }
    static GAOL_POWN_INLINE_T lanes fma(lanes a, lanes b, lanes c)
    {
      const lanes r = { back128(_mm512_fmadd_round_pd(lo512(a.v), lo512(b.v), lo512(c.v), gaol_er)) };
      return r;
    }
    static GAOL_POWN_INLINE_T lanes fnmadd(lanes a, lanes b, lanes c)
    {
      const lanes r = { back128(_mm512_fnmadd_round_pd(lo512(a.v), lo512(b.v), lo512(c.v), gaol_er)) };
      return r;
    }
    static GAOL_POWN_INLINE_T lanes twice_plus(lanes a, lanes b)
    {
      const lanes r = { back128(_mm512_fmadd_round_pd(_mm512_set1_pd(2.0), lo512(a.v), lo512(b.v), gaol_er)) };
      return r;
    }
    static GAOL_POWN_INLINE_T lanes div(lanes a, lanes b)
    {
      const lanes r = { er_div2(a.v, b.v) };
      return r;
    }
    static GAOL_POWN_INLINE_T lanes add(lanes a, lanes b)
    {
      const lanes r = { back128(_mm512_add_round_pd(lo512(a.v), lo512(b.v), gaol_er)) };
      return r;
    }
    static GAOL_POWN_INLINE rests first_rest(lanes e) { return e.v; }
    static GAOL_POWN_INLINE void add_rest(rests& s, lanes e) { s = _mm_or_pd(s, e.v); }
  };

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
    pair, <-lower, upper>. It repeats the block of pown_large_inline() of the SSE2
    intervals (gaol_interval_sse.cpp), on the other lanes: keep them alike. They give the
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
    pown_powers<pown_er_ops>(gl, m, h, l, s);
    if (negative) {
      const pown_er_lanes one = { _mm_set1_pd(1.0) };
      const pown_er_lanes q = pown_er_ops::div(one, h);
      const pown_er_lanes r = pown_er_ops::fnmadd(q, l, pown_er_ops::fnmadd(q, h, one)); // (1 - q h) - q l
      const pown_er_lanes qr = pown_er_ops::mul(q, r);
      l = pown_er_ops::fma(qr, r, qr); // q (r + r^2)
      h = q;
      pown_er_ops::add_rest(s, r);
      K = _mm_set1_pd(pown_bound_inverse(m, L));
    } else {
      K = _mm_set1_pd(pown_bound(m, L));
    }
    K = _mm_andnot_pd(_mm_castsi128_pd(_mm_cmpeq_epi64(_mm_castpd_si128(s), _mm_setzero_si128())), K);
    const __m128d odd = ((m & 1u) != 0u) ? minus_zero : zero;
    const __m128d sign = _mm_xor_pd(_mm_set_pd(0.0, -0.0), _mm_and_pd(c, odd));
    const pown_er_lanes vs = { _mm_xor_pd(h.v, sign) }, ws = { _mm_xor_pd(l.v, sign) }, Kl = { K };
    res = pown_er_ops::add(vs, pown_er_ops::fma(Kl, h, ws)).v;
    return true;
  }
#endif
