/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2009 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>, Jordan NININ
 *--------------------------------------------------------------------------*/

/*
    This file is not meant to be compiled. It is only to be included
        by gaol/gaol_interval_core.cpp.

    The intervals of the builds that have neither SSE2 nor NEON intervals
    (gaol/gaol_configuration.h): 32-bit ARM, POWER, s390x, RISC-V and the other
    processors; Windows on 32-bit x86 with Visual C++ and clang-cl (MinGW-w64
    takes the SSE2 intervals there) and on arm64 with Visual C++, clang-cl and
    MinGW-w64; and the builds with GAOL_SIMD=OFF or GAOL_ARM64=OFF. The
    product, the quotient and the square take no branch on the signs of the
    bounds in the source (GAOL v5, the kernels of fpu_detail below, on two
    doubles as those of gaol/gaol_interval_sse.cpp and
    gaol/gaol_interval_neon.cpp are on a vector): see doc/using.md, The FPU
    intervals.
*/


//#include "cycle.h"


double hausdorff(const interval &I1, const interval &I2)
{
	if (I1.is_empty() || I2.is_empty()) {
	return GAOL_NAN;
	}
	// A bound infinite in one interval only puts them at distance +oo, returned
	// before the check of the rounding direction (GAOL v5, point R.50 of
	// TODO.md; GAOL 4 returned +oo there for every infinite bound): the check
	// and the differences made it two to four times as long (Intel i7-1185G7,
	// GCC 9.4 and Clang 18). Whether a bound is infinite depends neither on the
	// rounding direction nor on the flush-to-zero modes, which read a subnormal
	// bound as a zero, no infinity either. The bounds compared after the check
	// are read after it (GAOL_RND_BARRIER() of gaol/gaol_fpu.h, and /fp:strict
	// with Visual C++)
	if ((std::fabs(I1.left()) == GAOL_INFINITY) != (std::fabs(I2.left()) == GAOL_INFINITY)
		|| (std::fabs(I1.right()) == GAOL_INFINITY) != (std::fabs(I2.right()) == GAOL_INFINITY)) {
	return GAOL_INFINITY;
	}
	// The tightest upper bound of the distance, whatever the rounding direction
	// of the caller: each difference is rounded upward both ways.
	// Equal bounds, infinite ones included, are at distance 0 (inf - inf is a
	// NaN): the distance of [1, +oo] to [1, +oo] is 0 and the one to [2, +oo] is
	// 1, where GAOL 4 returned +oo.
	GAOL_RND_ENTER();
	const double a = I1.left(), b = I1.right(), c = I2.left(), e = I2.right();
	double d = maximum((a == c) ? 0.0 : maximum(a - c, c - a),
					(b == e) ? 0.0 : maximum(b - e, e - b));
	GAOL_RND_KEEP(d);
	GAOL_RND_LEAVE();
	return d;
}

/*
	The integer powers are computed in gaol_interval_core.cpp, from exact
	products, the same way as with the SSE2 intervals (GAOL v5, see
	pown_products() there): the products rounded outward of GAOL 4,
	uipow_dnup(), uipow_upup() and uipow_rounded(), are gone.
*/


  /*
    The kernels of the product and of the quotient of two intervals, on the
    stored bounds <-l, r> (GAOL v5), as those of the SSE2 intervals
    (gaol_interval_sse.cpp) and of the NEON ones (gaol_interval_neon.cpp), but
    on two doubles each: the common case has no branch on the signs of the
    bounds, where the former code took one of nine cases of the signs for a
    product, and went down a chain of up to eight tests for a quotient, which
    the processor mispredicted when the signs came at random (x * y of
    operands of random signs took 12.4 ns, and 3.3 ns of operands of one sign:
    16 384 operands, Apple M2, AppleClang 21, FPU intervals). The price: a
    line of products of operands of one sign, which a processor predicts, takes
    1.5 times as long on the M2 and 1.7 with GCC on x86-64, each product
    waiting for the largest of four (doc/using.md, The FPU intervals). They
    work with the rounding direction upward, on bounds read after its check
    (gaol/gaol_fpu.h), and compare only bounds that are no NaN: the operators
    tell the empty set before the check, and no product that would be NaN (0
    times an infinite bound) is formed, the intervals with an infinite bound
    taking mul_infinite(). GCC for 32-bit ARM and for POWER9 turns a quiet
    comparison into a signaling one where it makes a choice on it with a
    conditional move, which raises the invalid-operation exception on a NaN
    (see is_empty() in gaol/gaol_interval_core.h): no comparison is made here
    that a NaN could reach, and the only choices on bounds are the maxima of
    larger_of().
  */
  namespace fpu_detail {

  /*
    The larger of two doubles that are no NaN (GAOL v5): a comparison and a selection,
    the `maxsd` of x86 (`maxpd` of the SSE2 intervals), which returns the
    second operand when they compare equal, +0 and -0 included. What the
    compiler returns there is its own, and compilers differ: GCC 13 for POWER9
    makes `(a > b) ? a : b` an `xsmaxcdp`, whose result for a +0 and a -0 is
    the first operand (as QEMU computes it, in the continuous integration; the
    comment of GCC lets the instruction stand for `a >= b ? a : b` as well).
    The value of a bound is the same for all of them, but where two products are
    zeros of opposite signs, +0 and -0, the sign of the zero bound of a product
    is the compiler's and the target's: the lower bound of [-1, 0] * [-1, 0] is
    +0 with `maxsd` and -0 with `xsmaxcdp`. It is not specified for the FPU
    intervals (doc/using.md, The FPU intervals), and no test reads it. The
    maximum that ranks +0 above -0 (fmax(), the `fmax` of 64-bit ARM) is one
    instruction fewer, and is not one with Visual C++ or GCC for 32-bit ARM.
    The selection is a conditional move with GCC (13 to 16) for 32-bit ARM,
    arm64 and x86, with Clang, and with Visual C++ for x86 and arm64, an
    `xsmaxcdp` with GCC for POWER9 (a branch with GCC for POWER8, s390x and
    RISC-V, which have no conditional move of doubles)
  */
  GAOL_INLINE double larger_of(double a, double b)
  {
    return (a > b) ? a : b;
  }

  /*
    Whether a stored bound is +oo (GAOL v5), told on its bits: it is the only infinity
    that is stored, <-left, right> holding no -oo (an interval has neither a
    left bound +oo nor a right bound -oo), and it is no NaN here, so that the
    high word of its bits tells it, 0x7ff00000. Told on integers, because four
    comparisons of doubles took 1.2 of the 3.4 ns of x * y (Apple M2, Clang
    21), and on the high word, because Visual C++ for x86 compiled a comparison
    of 64-bit integers to eight branches (and GCC for 32-bit ARM to a dozen
    instructions). The four are joined with ||, not |: Clang turned the
    four tests joined with | into vector instructions (cmeq, umaxv and fmov to
    a general register), after which a product that waited for the one before
    it took 56 ns for seven, and 35 with four branches that no sign decides
  */
  GAOL_INLINE bool is_infinite(double u)
  {
    return static_cast<std::uint32_t>(gaol_detail::bound_bits(u) >> 32) == 0x7ff00000u;
  }

  /*
    The product of x = <xl, xr> = <-l1, r1> and y = <yl, yr> = <-l2, r2>
    (GAOL v5), whose bounds are all finite: the largest of the four products of the corners is
    the upper bound, and the largest of their opposites the lower one, stored
    negated. The rounding direction being upward, the opposite of a product
    is not the opposite of its rounding (-RU(p) is RD(-p), not RU(-p)): the
    four products of the lower bound are other products, RU(-l1*l2), RU(-l1*r2),
    RU(-r1*l2) and RU(-r1*r2), each with a factor negated, which is exact. That
    is eight products, the only rounding direction there is being upward. The
    factors negated are those of y: x comes from the operation before, in a
    line of products, and a negation of its bounds would add to the time that
    line waits. Four products with the factors chosen by the signs of y (l1
    for a positive l2 and r1 for a negative one in the lower bound) would do,
    but the compilers turn these choices into branches on the signs, where they
    make the maxima of the eight products with conditional moves (GCC for 32-bit
    ARM, arm64 and x86, Visual C++ for x86: x * y of random signs took
    3.3 ns with the eight products and 2.8 with the choices, Apple M2,
    AppleClang 21, and they were branches with the others). The maxima commute with
    the rounding upward, which is monotone, so that each product is rounded
    once. Neither a 0 nor an infinite bound can make a product NaN here
  */
  GAOL_INLINE void mul_finite(double xl, double xr, double yl, double yr, double& lb, double& rb)
  {
    const double nyl = -yl, nyr = -yr;
    lb = larger_of(larger_of(xl*nyl, xl*yr), larger_of(xr*yl, xr*nyr));
    rb = larger_of(larger_of(xr*yr, xr*nyl), larger_of(xl*nyr, xl*yl));
  }

  /*
    A zero times an infinity is NaN, and raises the invalid-operation
    exception (GAOL v5): a product of the bounds of intervals, where one is 0 and the
    other infinite, is 0. Both factors are set to 0 then, before the product
    that would form it. The stored bounds hold no -oo, a negated one is not
    tested
  */
  GAOL_INLINE void clear_zero_infinity(double& a, double& b)
  {
    const bool zero_infinity = ((a == 0.0) && is_infinite(b)) || ((b == 0.0) && is_infinite(a));
    a = zero_infinity ? 0.0 : a;
    b = zero_infinity ? 0.0 : b;
  }

  /*
    The product of x and y, one of whose bounds is infinite (GAOL v5): the
    eight products of mul_finite(), grouped by the pair of bounds that each two
    share, which clear_zero_infinity() clears. The nine cases of the signs of
    the former code took 12.4 ns for operands with an infinite bound drawn at
    random, 20.2 behind the test of the four bounds, and this function takes
    9.2 (16 384 of them, Apple M2). Cold, as divide_by_zero_containing(): a
    program computes with infinite bounds seldom, and a processor predicts the
    test of each operation when it does
  */
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((noinline, cold))
#elif defined(_MSC_VER)
  __declspec(noinline)
#endif
  void mul_infinite(double xl, double xr, double yl, double yr, double& lb, double& rb)
  {
    double a = xl, b = yl;
    clear_zero_infinity(a, b);
    const double l1 = a*(-b), u1 = a*b;
    a = xl; b = yr;
    clear_zero_infinity(a, b);
    const double l2 = a*b, u2 = a*(-b);
    a = xr; b = yl;
    clear_zero_infinity(a, b);
    const double l3 = a*b, u3 = a*(-b);
    a = xr; b = yr;
    clear_zero_infinity(a, b);
    const double l4 = a*(-b), u4 = a*b;
    lb = larger_of(larger_of(l1, l2), larger_of(l3, l4));
    rb = larger_of(larger_of(u4, u3), larger_of(u2, u1));
  }

  /*
    The quotient of x = <a, b> = <-l, r> by a y = <p, q> = <-c, d> that has a
    zero, c <= 0 <= d, neither of them empty (GAOL v5), in the cases of the
    signs of x and of the bounds of y that are 0 (the table below), one
    division at most, of two numbers that are no zero, so that none raises the
    division-by-zero or the invalid-operation exception (the divisor goes
    through read_in_branch(): Clang for 64-bit ARM computes the divisions of
    both sides of a test and chooses afterwards).
    modulo is the "extended" quotient of operator%=(): where [0] / [0] is empty
    and x / y is empty for a y of zeros, x % y is the whole line wherever x
    contains zero. With x N1 for r < 0, N0 for l < 0 = r, Z for [0], M for l <
    0 < r, P0 for l = 0 < r and P1 for l > 0:
                  y = [0]   N0: c < 0 = d   P0: c = 0 < d   M: c < 0 < d
      x N1        empty     [r/c, +oo]       [-oo, r/d]      universe
      x N0        empty     [0, +oo]         [-oo, 0]        universe
      x Z         empty     [0]              [0]             [0]
      x M         empty     universe         universe        universe
      x P0        empty     [-oo, 0]         [0, +oo]        universe
      x P1        empty     [-oo, l/c]       [l/d, +oo]      universe
    where x % y is empty for [0] only when x is N1 or P1, and the whole line in
    the rows N0, Z, M and P0 of every column. The bounds are those of the former
    code and of the NEON intervals, which have the same table. Cold: a
    divisor with a zero is rare, and its code is out of the way of the common
    one
  */
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((noinline, cold))
#elif defined(_MSC_VER)
  __declspec(noinline)
#endif
  void divide_by_zero_containing(double a, double b, double p, double q, bool modulo,
                                 double& lb, double& rb)
  {
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    enum { N1, N0, Z, M, P0, P1 } xs;
    if (b < 0.0) {
      xs = N1;
    } else if (b == 0.0) {
      xs = (a == 0.0) ? Z : N0;
    } else if (a > 0.0) {
      xs = M;
    } else {
      xs = (a == 0.0) ? P0 : P1;
    }
    const bool c_zero = (p == 0.0), d_zero = (q == 0.0);
    if (c_zero && d_zero) { // y = [0]
      if (!modulo || xs == N1 || xs == P1) { // the empty set
        lb = rb = nan;
      } else {
        lb = rb = inf;
      }
      return;
    }
    if (xs == Z) {
      if (modulo) {
        lb = rb = inf;
      } else {
        lb = -0.0; // interval::zero()
        rb = 0.0;
      }
      return;
    }
    if (xs == M || (modulo && (xs == N0 || xs == P0)) || (!c_zero && !d_zero)) { // x M, or y M
      lb = rb = inf;
      return;
    }
    if (d_zero) { // y = [c, 0] with c < 0
      switch (xs) {
      case N1:
        lb = b / read_in_branch(p);
        rb = inf;
        return;
      case P1:
        lb = inf;
        rb = a / read_in_branch(p);
        return;
      case N0: // [0, +oo]
        lb = -0.0;
        rb = inf;
        return;
      default: // P0: [-oo, 0]
        lb = inf;
        rb = 0.0;
        return;
      }
    }
    // y = [0, d] with d > 0
    switch (xs) {
    case N1:
      lb = inf;
      rb = b / read_in_branch(q);
      return;
    case P1:
      lb = a / read_in_branch(q);
      rb = inf;
      return;
    case N0: // [-oo, 0]
      lb = inf;
      rb = 0.0;
      return;
    default: // P0: [0, +oo]
      lb = 0.0;
      rb = inf;
      return;
    }
  }

  /*
    The quotient of the stored bounds x = <xl, xr> and y = <yl, yr> of two
    intervals that are no empty set (GAOL v5), with the rounding direction
    upward, on bounds read after its check. Without a zero in y, c > 0 or d < 0, it is one
    division for each bound, by the bound of y that the sign of the bound
    chooses, as div_bounds() of the SSE2 intervals computes it: the divisor
    is made positive (x and y exchanged and negated, <-l, r> becoming <r,
    -l>, when its upper bound is negative: x/y = (-x)/(-y)), y being [c', d']
    with 0 < c' <= d'; the lower bound of the quotient is then -(l / d') for
    l >= 0 and -(l / c') for l < 0, the upper one r / c' for r >= 0 and r / d'
    for r < 0. An infinite bound of x is divided by c' or d', which are
    finite, never by an infinity, and a zero bound by a number that is no zero:
    no 0 / 0, no oo / oo, and every division is by a number above 0. The
    bounds of the quotient are the ones of the SSE2 intervals, bit for bit
    (no tie of two zeros is chosen: a division of IEEE gives the sign of a
    zero). The choices of the denominators are conditional moves with Clang
    and Visual C++ for arm64, but one branch on a sign with GCC for arm64, two
    with GCC 13 to 15 and four with GCC 16 for 32-bit ARM (-O2 and -O3), where a
    division takes longer than a branch mispredicted. A y with a zero is the
    other function
  */
  GAOL_INLINE void quotient(double xl, double xr, double yl, double yr, bool modulo,
                            double& lb, double& rb)
  {
    // y is made positive, -y = [c', d'] where d < 0, and <p, q> = <-c', d'> is
    // then <yl, yr>, or <yr, yl>: y has no zero where c' > 0, that is p < 0,
    // and has one where p >= 0 (c <= 0 <= d: <p, q> = <-c, d>). One branch,
    // the unlikely one, and not one on the sign of y, which a test of "c > 0 or
    // d < 0" would make at random
    const bool negative = yr < 0.0;
    const double p = negative ? yr : yl;
    const double q = negative ? yl : yr;
    if (GAOL_UNLIKELY(!(p < 0.0))) { // a zero in y
      divide_by_zero_containing(xl, xr, yl, yr, modulo, lb, rb);
      return;
    }
    const double a = negative ? xr : xl;   // <a, b> is x, or -x
    const double b = negative ? xl : xr;
    const double lower_denominator = (a <= 0.0) ? q : -p; // d' for l >= 0, c' for l < 0
    const double upper_denominator = (b < 0.0) ? q : -p;  // d' for r < 0, c' for r >= 0
    lb = a / lower_denominator;
    rb = b / upper_denominator;
  }

  } // namespace fpu_detail


  interval& interval::operator+=(const interval& I)
  {
    GAOL_RND_ENTER();
		lb_ += I.lb_;
		rb_ += I.rb_;

    GAOL_RND_LEAVE();
    return *this;

  }


  interval& interval::operator-=(const interval& I)
  {
    // I may be *this (x -= x): its bounds are read before one is written,
    // or x -= x was [a - b, 2b - a] for x = [a, b] (GAOL v5). They are read
    // after the check of the rounding direction, as x += y reads them: read
    // before it, they were kept across the calls its rare path makes, in
    // registers that every call saves (x - y took 3.6 ns, three times x + y,
    // Apple M2)
    GAOL_RND_ENTER();
    const double ilb = I.lb_, irb = I.rb_;
		lb_ += irb;
		rb_ += ilb;
    GAOL_RND_LEAVE();
    return *this;
  }


  // The operations with a double tell an infinite d and a NaN with quiet
  // comparisons, as interval(double): a NaN d, which < compared, raised the
  // invalid-operation exception (GAOL v5)
  interval& interval::operator+=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
		lb_ = lb_-d;
		rb_ += d;
    GAOL_RND_LEAVE();
    return *this;
  }


  interval& interval::operator-=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    GAOL_RND_ENTER();
		lb_ += d;
		rb_ -= d;
    GAOL_RND_LEAVE();
    return *this;
  }


  interval& interval::operator*=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    // One check for both branches, which both compute, as in the SSE2 build,
    // before d is compared: with the modes that flush the subnormals to zero,
    // a subnormal d was 0 (GAOL v5). d is read again after the check, which
    // the compiler could otherwise compare it before (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    d = rnd_reread(d);
    if (d==0.0) {
      *this=interval::zero();
      GAOL_RND_LEAVE();
      return *this;
    }
    if (d>0.0) {
			lb_ *= d;
			rb_ *= d;
    } else {
			double tmp = (-d)*rb_;
			rb_ = (-d)*lb_;
			lb_ = tmp;
    }
    GAOL_RND_LEAVE();
    return *this;
  }


  interval& interval::operator/=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }
    // One check for both branches, which both compute, as in the SSE2 build,
    // before d is compared, as in operator*=(double) (GAOL v5)
    GAOL_RND_ENTER();
    d = rnd_reread(d);
    if (d == 0.0) {
      *this = interval::emptyset();
      GAOL_RND_LEAVE();
      return *this;
    }
    if (d > 0.0) {
		lb_ /= d;
		rb_ /= d;
    } else { // d < 0.0
		double tmp = rb_/(-d);
		rb_ = lb_/(-d);
		lb_ = tmp;
    }
    GAOL_RND_LEAVE();
    return *this;
  }


  interval& interval::operator%=(double d)
  {
    if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
      *this = interval::emptyset();
      return *this;
    }

    // One check for both branches, which both compute, as in the SSE2 build,
    // before d and the bounds are compared, as in operator*=(double) (GAOL v5)
    GAOL_RND_ENTER();
    d = rnd_reread(d);
    if (d == 0.0) {
      if (straddles_zero()) {
		*this = interval::universe();
		GAOL_RND_LEAVE();
		return *this;
      } else { // this does not contain 0
		*this = interval::emptyset();
		GAOL_RND_LEAVE();
		return *this;
      }
    }

    if (d > 0.0) {
			lb_ /= d;
			rb_ /= d;
    } else { // d < 0.0
			double tmp = rb_/(-d);
			rb_ = lb_/(-d);
			lb_ = tmp;
    }
    GAOL_RND_LEAVE();
    return *this;
  }








  /*
    x * y (GAOL v5): the largest of the products of the corners for each
    bound, with no branch on the signs of the bounds (fpu_detail::mul_finite()),
    where the former code took one of nine cases of the signs, which the
    processor mispredicted when they came at random. The empty set, a NaN
    lower bound, is told by one quiet comparison of the two lower bounds before
    the check of the rounding direction, as in operator&=(): no NaN reaches a
    comparison of bounds. The bounds are read after the check, once each,
    through rnd_reread() (see gaol/gaol_fpu.h), before one is written: *this
    may be I (x *= x). The result is written into the bounds of *this, which
    is memory of the caller, and *this is also given by address to
    mul_infinite(), which is not inlined: a call (fesetround()) or an asm with
    a "memory" clobber that GAOL_RND_LEAVE() makes finds the stores before it,
    and so the products they hold, which the rounding direction upward has to
    compute. That is how the compilers treat them (GCC 15 for 32-bit ARM with
    GAOL_PRESERVE_ROUNDING: the stores precede the call), not a rule of the
    language. GAOL_RND_KEEP() (gaol/gaol_fpu.h) would make it one, for 2.6 ns
    here with GAOL_PRESERVE_ROUNDING (an Apple M2), and x += y and the former
    product did without: decided on 2026-10-23 to leave it out and to hold the
    protection by tests instead. The checks of tests/rounding_direction.cpp
    that run x *= y, x /= y and x %= y after the program changed the
    direction (tight_cases) and with a flush mode set fail where a product is
    computed after GAOL_RND_LEAVE(): the jobs of GCC and Clang with the FPU
    intervals and GAOL_PRESERVE_ROUNDING (.github/workflows/linux.yml) run
    them, and the product written after GAOL_RND_LEAVE() fails 53 of the 96
    checks of x *= y in a loop (x /= y 58, x %= y 57) with GCC 15.2 and Clang 21
    on x86-64. An interval with an
    infinite bound is the rare case, which mul_infinite() computes: the
    kernel would form 0 times an infinite bound among its eight products, a
    NaN that raises the invalid-operation exception, and that must be 0.
  */
  interval& interval::operator*=(const interval& I)
  {
    if (gaol_detail::quiet_unordered(lb_, I.lb_)) { // an empty operand
      *this=interval::emptyset();
      return *this;
    }

    // The rounding direction checked before the bounds are compared, as in
    // operator/=(): with the modes that flush the subnormals to zero,
    // [100*2^-1074] was zero, and [100*2^-1074]*[1e10] was [100*2^-1074]
    // (GAOL v5)
    GAOL_RND_ENTER();
    const double xl = rnd_reread(lb_), xr = rnd_reread(rb_);
    const double yl = rnd_reread(I.lb_), yr = rnd_reread(I.rb_);
    if (GAOL_UNLIKELY(fpu_detail::is_infinite(xl) || fpu_detail::is_infinite(xr)
                      || fpu_detail::is_infinite(yl) || fpu_detail::is_infinite(yr))) {
      fpu_detail::mul_infinite(xl, xr, yl, yr, lb_, rb_);
      GAOL_RND_LEAVE();
      return *this;
    }

    fpu_detail::mul_finite(xl, xr, yl, yr, lb_, rb_);
    GAOL_RND_LEAVE();
    return *this;
  }


  /*
    The rounding direction being upward already (see gaol_interval_core.h). The
    bound that is 0 in the other branch, I.rb_ for N1 and I.lb_ for P1, is
    read in its branch (read_in_branch(), gaol/gaol_interval_core.cpp):
    AppleClang computed -1/I.rb_ for N0 too, inverse() choosing between the
    branches afterwards, and inverse([-2, 0]) raised the division-by-zero
    exception (macOS arm64, GAOL_PRESERVE_ROUNDING; GAOL v5). Not the
    quotient of [1] by I of fpu_detail::quotient(), whose selections took more
    time than the branches of these five cases, which the operands of an
    inverse follow (3.2 ns rather than 2.1 for operands of one sign, 6.2
    rather than 5.8 for operands of random signs, Apple M2)
  */
  interval interval::inverse_upward(const interval& I)
  {
    interval res;
    if (I.right() <= 0.0) { // (Z N0 N1)
      if (I.left() < 0) { // (N0 N1)
        if (I.right() < 0) { // N1
          res.lb_ = (-1.0)/read_in_branch(I.rb_);
          res.rb_ = (-1.0)/I.lb_;
        } else { // N0
          res.lb_ = GAOL_INFINITY;
          res.rb_ = (-1.0)/I.lb_;
        }
      } else {  // Z
        res = interval::emptyset();
      }
    } else { // (M P0 P1)
      if (I.left() < 0) { // M
        res = interval::universe();
      } else { // (P0 P1)
        if (I.left() == 0.0) { // P0
          res.lb_ = (-1.0)/I.rb_;
          res.rb_ = GAOL_INFINITY;
        } else { // P1
          res.lb_ = (-1.0)/I.rb_;
          res.rb_ = (-1.0)/read_in_branch(I.lb_);
        }
      }
    }
    return res;
  }

  interval interval::inverse() const
  {
    if (is_empty()) {
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    interval res = inverse_upward(*this);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    x / y and x % y (GAOL v5): one division of each bound of x, by the bound of
    y that the signs choose, whatever the signs, as the SSE2 and the NEON
    intervals do (fpu_detail::quotient()), where the former code went down a
    chain of up to eight tests of the signs. The empty set, the check of the
    rounding direction and the bounds read after it are as in operator*=():
    with the modes that flush the subnormals to zero, which the check clears, a
    subnormal bound compared as 0, and [1e-300]/[100*2^-1074] was the empty set
    (GAOL v5, see gaol/gaol_fpu.h)
  */
  interval& interval::operator/=(const interval& I)
  {
    if (gaol_detail::quiet_unordered(lb_, I.lb_)) { // an empty operand
      *this = interval::emptyset();
      return *this;
    }

    GAOL_RND_ENTER();
    const double xl = rnd_reread(lb_), xr = rnd_reread(rb_);
    const double yl = rnd_reread(I.lb_), yr = rnd_reread(I.rb_);
    fpu_detail::quotient(xl, xr, yl, yr, false, lb_, rb_);
    GAOL_RND_LEAVE();
    return *this;
  }

  interval& interval::operator%=(const interval& I)
  {
    if (gaol_detail::quiet_unordered(lb_, I.lb_)) { // an empty operand
      *this = interval::emptyset();
      return *this;
    }

    GAOL_RND_ENTER();
    const double xl = rnd_reread(lb_), xr = rnd_reread(rb_);
    const double yl = rnd_reread(I.lb_), yr = rnd_reread(I.rb_);
    fpu_detail::quotient(xl, xr, yl, yr, true, lb_, rb_);
    GAOL_RND_LEAVE();
    return *this;
  }

  interval div_rel(const interval &K, const interval &J, const interval &I)
  {
    // One check, before the bounds are compared, as in the SSE2 build (GAOL
    // v5, see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    interval res = [&]() -> interval {
    if (K.is_empty() || J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    
    if (K.right() < 0.0) { // [this] N1
      if (J.right() < 0.0) { // [J] N1
	// I & [K.r/J.l, K.l/J.r]
	interval tmp(-(K.rb_/J.lb_),K.lb_/(-J.rb_));
	return I & tmp; // N1 N1
      } else { // [J] P or Z
	if (J.right() == 0.0) { // [J] N0 or Z
	  if (J.left() == 0.0) { // [J] Z
	    return interval::emptyset(); // N1 Z
	  }
	  // I & [K.r/J.l,+oo]
	  interval tmp(-(K.rb_/J.lb_),GAOL_INFINITY);
	  return I & tmp; // N1 N0
	} else { // [I] P or M
	  if (J.left() < 0.0) { // [J] M
	    interval tmp(-GAOL_INFINITY,K.rb_/J.rb_);
	    // The lower bound rounded downward, the rounding direction being
	    // upward (GAOL v5: K.rb_/(-J.lb_) was rounded upward, and the
	    // result left out the doubles below it that belong to the hull)
	    interval tmp2(-(K.rb_/J.lb_),GAOL_INFINITY);
	    return (I&tmp) | (I&tmp2); // N1 M
	  } else { // [J] P0 or P1
	    if (J.left() == 0.0) { // [J] P0
	      interval tmp2 = I & interval(-GAOL_INFINITY,K.right()/J.right()); // N1 P0
	      return tmp2; // N1 P0
	    } else { // [J] P1
	      interval tmp(-(K.lb_/(-J.lb_)),K.rb_/J.rb_);
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
	    interval tmp2 = I & interval(0.0,K.left()/J.right()); // N0 N1
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
		  interval tmp(-(K.lb_/(-J.lb_)),0.0);
		  return I & tmp; // N0 P1
		}
	      }
	    }
	  }
	}
      } else { // [this] M, P0, or P1
	if (K.left() < 0.0) { // [this] M
	  if (J.right() < 0.0) { // [J] N1
	    interval tmp(-(K.rb_/(-J.rb_)),K.lb_/(-J.rb_));
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
		  interval tmp(-(K.lb_/(-J.lb_)),K.rb_/(-J.lb_));
		  return I & tmp; // M P1
		}
	      }
	    }
	  }
	} else { // [this] P0 or P1
	  if (K.left() == 0.0) { // [this] P0
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(K.rb_/(-J.rb_)),0.0);
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
		    interval tmp2 = I & interval(0.0,K.right()/J.left()); // P0 P1
		    return tmp2;
		  }
		}
	      }
	    }
	  } else { // [this] P1
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(K.rb_/(-J.rb_)),K.lb_/J.lb_);
	      return I & tmp; // P1 N1
	    } else { // [J] P or Z
	      if (J.right() == 0.0) { // [J] N0 or Z
		if (J.left() == 0.0) { // [J] Z
		  return interval::emptyset(); // P1 Z
		}
		interval tmp2 = I & interval(-GAOL_INFINITY,K.lb_/J.lb_); // P1 N0
		return tmp2;
	      } else { // [J] P or M
		if (J.left() < 0.0) { // [J] M
		  interval tmp(-GAOL_INFINITY,K.lb_/J.lb_);
		  interval tmp2 = interval(-(K.lb_/J.rb_),GAOL_INFINITY);
		  return (I&tmp) | (I&tmp2); // P1 M
		} else { // [J] P0 or P1
		  if (J.left() == 0.0) { // [J] P0
		    interval tmp(-(K.lb_/J.rb_),GAOL_INFINITY);
		    return I & tmp; // P1 P0
		  } else { // [J] P1
		    interval tmp(-(K.lb_/J.rb_),K.rb_/(-J.lb_));
		    return I & tmp; // P1 P1
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    }();
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  /*
    The square from two products and a maximum, no branch but the one of the
    empty set (GAOL v5), as sqr_bounds() of the SSE2 and the NEON intervals:
    with I = <a, b> = <-l, r>, the stored lower bound is the larger of a * (-a)
    and b * (-b), which are -l^2 and -r^2 rounded upward, the opposite of the
    smaller of the squares, and the upper one the larger of a * a and b * b,
    whatever the signs: <-l^2, r^2> for l >= 0, <-r^2, l^2> for r <= 0. For an
    interval that strictly straddles zero the lower bound is 0, which a
    selection sets, stored -0 so that left() is +0 (the former code set
    l = 0.0 too, and GAOL gives [0, 4] for [-1, 2], where the SSE2 and the
    NEON intervals give [-0, 4]). The bounds are those of the former code, bit
    for bit: every product of a bound by itself is the same one. The check of
    the rounding direction covers the three cases, which all compute. A zero
    or an infinite bound needs no case of its own: no product of a zero by an
    infinity is formed.
    The rounding direction being upward already (see gaol_interval_core.h), of
    a non-empty I: the body of sqr() after its check, which pow() takes for
    x^2 after its own (GAOL v5). The bounds are read here, after the check
    of the caller, as sqr() reads them.
  */
  interval interval::sqr_upward(const interval& I)
  {
    const double a = rnd_reread(I.lb_), b = rnd_reread(I.rb_);
    double lo = fpu_detail::larger_of(a*(-a), b*(-b));
    double hi = fpu_detail::larger_of(a*a, b*b);
    // l < 0 < r: both stored bounds are above 0. & rather than &&, which is a
    // branch on the sign of a: the selection is a conditional move with Clang
    // and Visual C++ for arm64 and with GCC for arm64, POWER and 32-bit ARM
    // (-O2 and -O3: two branches at -Os with GCC 14 and 15), but a branch with
    // Visual C++ for x86
    lo = ((a > 0.0) & (b > 0.0)) ? -0.0 : lo;
    interval res;
    res.lb_ = lo;
    res.rb_ = hi;
    return res;
  }

  interval sqr(const interval& I)
  {
    if (gaol_detail::quiet_unordered(I.lb_, I.rb_)) { // the empty set
      return interval::emptyset();
    }

    GAOL_RND_ENTER();
    interval res = interval::sqr_upward(I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  interval interval::mid() const
  {
    if (is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are compared (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    if (is_symmetric()) { // symmetric case handles [-oo, +oo]
      GAOL_RND_LEAVE();
      return interval(0.0);
    }
    if (left() == -GAOL_INFINITY) {
      GAOL_RND_LEAVE();
      return interval(-std::numeric_limits<double>::max());
    }
    if (right() == GAOL_INFINITY) {
      GAOL_RND_LEAVE();
      return interval(std::numeric_limits<double>::max());
    }
    // As midpoint(), rounded outward: the half of the sum of the bounds, or the
    // sum of their halves where it may overflow, each half rounded outward too
    // (halving a bound below 2^-1021 is not exact). The sum may overflow only
    // where a bound is 2^1023 or more in magnitude, which is tested first, so
    // that mid([DBL_MAX]) raises no overflow exception (GAOL v5: the sum was
    // computed first, and its overflow told the other case); the sum of the
    // halves gives the same bounds as the half of the sum there
    double mid_left, mid_right;
    const double big = 8.9884656743115795e+307; // 2^1023 (C++11 has no hexadecimal floating literal)
    if (std::fabs(lb_) >= big || std::fabs(rb_) >= big) {
      // Each half with a negated factor goes through rnd_keep(), whose volatile
      // memory keeps it as written: Visual C++ (/O2 /fp:strict) rewrites
      // (-.5)*x + y into y - .5*x, which rounds the half of x in the wrong
      // direction when it is inexact, and mid([2^-1074, MAX]) was [MAX/2, MAX/2]
      mid_left  = -(.5*lb_ + gaol_core::rnd_keep((-.5)*rb_));
      mid_right = gaol_core::rnd_keep((-.5)*lb_) + .5*rb_;
    } else {
      double sum = rb_ - lb_;     // a+b, rounded upward
      double opp_sum = lb_ - rb_; // -(a+b), rounded upward
      mid_left  = -(.5*opp_sum);
      mid_right = .5*sum;
    }
    GAOL_RND_KEEP(mid_left); GAOL_RND_KEEP(mid_right);
    GAOL_RND_LEAVE();
    return interval(mid_left,mid_right);
  }


