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
This file shall be included by gaol_interval_core.cpp only. It must not be included directly
by any other file.
This file contains all declarations related to the 'interval' class that consider
the bounds to be an __m128d variable.
**/

#ifndef GAOL_INTERVAL_CORE_H
#  error "File gaol_interval_sse.h shall only be included directly by gaol_interval_core.cpp"
#endif

#ifndef GAOL_INTERVAL_SSE_H
#define GAOL_INTERVAL_SSE_H

GAOL_PUBLIC std::ostream& operator<<(std::ostream& out, const __m128d& x);

GAOL_INLINE interval interval::universe(void)
{
  return interval(interval::m128_infinf, gaol_detail::unchecked_bounds);
}

GAOL_INLINE interval interval::zero(void)
{
  return interval(interval::m128_zero, gaol_detail::unchecked_bounds);
}

// Both bounds NaN, as interval(double) sets them for a NaN, but without the
// comparisons that decide it: they signal the invalid-operation exception on
// a NaN, and a build without optimization runs them (GAOL v5)
GAOL_INLINE interval interval::emptyset(void)
{
  return interval(_mm_set1_pd(std::numeric_limits<double>::quiet_NaN()), gaol_detail::unchecked_bounds);
}

GAOL_INLINE interval::interval(const __m128d& xmm, gaol_detail::unchecked_bounds_t)
{
  xmmbounds = xmm;
}

// The stored bounds <-l, r>: -oo in a lane is l = +oo or r = -oo, which no
// interval has (IEEE 1788-2015, 10.5.8). cmpeqpd raises nothing on a quiet NaN,
// the empty set holding it in both lanes
GAOL_INLINE interval::interval(const __m128d& xmm)
{
  if (_mm_movemask_pd(_mm_cmpeq_pd(xmm, _mm_set1_pd(-std::numeric_limits<double>::infinity()))) != 0) {
    xmmbounds = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
  } else {
    xmmbounds = xmm;
  }
}

GAOL_INLINE interval interval::positive(void)
{
  return interval(0.0,std::numeric_limits<double>::infinity());
}

GAOL_INLINE interval interval::negative(void)
{
  return interval(-std::numeric_limits<double>::infinity(),0.0);
}

// An infinite v gives the empty set, as in IBEX: IEEE 1788-2015 has no
// interval [+oo, +oo] nor [-oo, -oo] (10.5.8). A NaN gives it too, told by
// a quiet first comparison (see interval(double, double))
GAOL_INLINE interval::interval(double v)
{
  // The bounds are set in a register: written to a pair in memory, their
  // 16-byte load stalls on the two 8-byte stores, which costs several ns
  if (gaol_detail::quiet_less(-GAOL_INFINITY, v) && v < GAOL_INFINITY) { // false for a NaN
    xmmbounds = _mm_set_pd(v, -v);
  } else {
    xmmbounds = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
  }
}

// The empty set for a lower bound of +oo, an upper bound of -oo, bounds in
// the wrong order and NaN bounds, as in IBEX. The first comparison is a
// quiet one, as in is_empty(), ucomisd for comisd on x86, at the same
// cost: a NaN bound, which <= compared, raised the invalid-operation
// exception, and so did interval(NAN), x += NAN and the functions giving
// the constructor the NaN bounds of an empty operand (floor(), max()...),
// which had to tell the empty set first (GAOL v5, decided for point D.24 of
// TODO.md). The other two compare bounds that are no NaN once it is true:
// made quiet too, they cost GCC a conditional move through the integer
// registers where it if-converts them, and 30% more in a loop of
// constructions (GCC 9.4, FPU intervals). The first one is also right with
// the modes that flush the subnormals to zero
// (gaol_detail::bound_less_equal(), gaol_port.h): under denormals-are-zero,
// interval(2^-1073, 2^-1074) kept its bounds in the wrong order (GAOL v5,
// point Q of TODO.md); the other two compare with an infinity, which these
// modes do not change
GAOL_INLINE interval::interval(double l, double r)
{
  if (gaol_detail::bound_less_equal(l, r) && l < GAOL_INFINITY && r > -GAOL_INFINITY) {
    xmmbounds = _mm_set_pd(r, -l);
  } else {
    xmmbounds = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
  }
}

GAOL_INLINE interval::interval()
{
  xmmbounds = _mm_set1_pd(std::numeric_limits<double>::infinity());
}


GAOL_INLINE interval::interval(const interval& I)
{
  xmmbounds = I.xmmbounds;
}

/*
One quiet comparison, std::isunordered() of the two stored lower bounds,
tells whether either operand is empty, both bounds of the empty set being
NaN, and the intersection is then [NaN, NaN], interval::emptyset() (GAOL
v5, decided for point D.24 of TODO.md): is_empty() of *this, and the NaN
bounds of an empty I carried through the comparisons below, took two
comparisons more. The bounds are compared after it with quiet comparisons
too: GCC 9.4 at -O3 computed them before that test, for an empty I, in a
loop of intersections where it turned the function into straight-line
code, and compiled plain comparisons with comisd, which raised the
invalid-operation exception (tests/rounding_direction.cpp). An empty
*this was returned as it was, with the NaN it held; telling it apart
again, after the test, made GCC 9.4 turn the whole function into
straight-line code, and x &= y 2.3 times slower. See gaol_interval_fpu.h
for 32-bit ARM and POWER9. They are the comparisons of bounds that the
modes flushing the subnormals to zero do not change (gaol_port.h): under
denormals-are-zero, [3, 100]*2^-1074 & [50, 200]*2^-1074 was
[3, 100]*2^-1074, and two disjoint intervals of subnormals were not
empty (GAOL v5, point Q of TODO.md).
*/
GAOL_INLINE interval& interval::operator&=(const interval& I)
{
  xmm2d bd, Ibd;
  _mm_store_pd(bd,xmmbounds);
  _mm_store_pd(Ibd,I.xmmbounds);
  if (gaol_detail::quiet_unordered(bd[0], Ibd[0])) {
    xmmbounds = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
    return *this;
  }
  // The bound of I that replaces one of *this is taken in the register
  // (movsd, shufpd), and the bound kept for the test below is a double of its
  // own: GCC 15.2 moved the bounds written to bd[] back through general
  // registers (vpextrq, vpinsrq), x &= y 2.2 ns rather than 1.5 and x | y
  // 3.2 rather than 1.3 (Intel i7-1185G7, GCC of Ubuntu), and Clang 21 takes
  // the same time with both (GAOL v5, point PG of TODO.md)
  __m128d res = xmmbounds;
  double lo = bd[0], hi = bd[1];
  if (!gaol_detail::bound_greater_equal(Ibd[0], bd[0])) { // Left bounds negated
    res = _mm_move_sd(res, I.xmmbounds);
    lo = Ibd[0];
  }
  if (!gaol_detail::bound_greater_equal(Ibd[1], bd[1])) {
    res = _mm_shuffle_pd(res, I.xmmbounds, 2);
    hi = Ibd[1];
  }
  
  // Disjoint intervals give the empty set, [NaN, NaN] as
  // interval::emptyset() (GAOL v5): their bounds in the wrong
  // order, [3, 2] for [1, 2] & [3, 4], were empty for is_empty(), but
  // the operations computing on the bounds gave [3, 2] + [0, 1] = [3, 3]
  if (!gaol_detail::bound_less_equal(-lo, hi)) {
    xmmbounds = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
  } else {
    xmmbounds = res;
  }
  return *this;
}

// The bounds compared as in operator&=() (GAOL v5): under
// denormals-are-zero, [3, 100]*2^-1074 | [200*2^-1074] was
// [3, 100]*2^-1074
GAOL_INLINE interval& interval::operator|=(const interval& I)
{
  if (is_empty()) {
    *this = I;
    return *this;
  }
  if (I.is_empty()) {
    return *this;
  }
  xmm2d bd, Ibd;
  _mm_store_pd(bd,xmmbounds);
  _mm_store_pd(Ibd,I.xmmbounds);
  
  // The bounds of I that replace those of *this are taken in the register, as
  // in operator&=()
  __m128d res = xmmbounds;
  if (gaol_detail::bound_greater(Ibd[0], bd[0])) { // Left bounds negated
    res = _mm_move_sd(res, I.xmmbounds);
  }
  if (gaol_detail::bound_greater(Ibd[1], bd[1])) {
    res = _mm_shuffle_pd(res, I.xmmbounds, 2);
  }
  xmmbounds = res;
  return *this;
}

/*
  The stored bounds, read from the register. Visual C++ 19.44 (Visual Studio
  2022 17.14) with /O2 and /arch:AVX2 read 0 from the local r after
  _mm_storeh_pd(&r, ...): base.right() was 0 in pow_standard_upward(), and
  pow([3, 4], [2, 3]) was empty (Compiler Explorer, and the Visual Studio 2022
  jobs of the continuous integration; GAOL v5). It reads them with the
  instructions of the register there; the other compilers keep the stores
*/
GAOL_INLINE double interval::left_internal() const
{
#if defined(_MSC_VER) && !defined(__clang__)
  return _mm_cvtsd_f64(xmmbounds);
#else
  GAOL_ALIGN16(double l);
  _mm_storel_pd(&l,xmmbounds);
  return l;
#endif
}
GAOL_INLINE double interval::right_internal() const
{
#if defined(_MSC_VER) && !defined(__clang__)
  return _mm_cvtsd_f64(_mm_unpackhi_pd(xmmbounds, xmmbounds));
#else
  GAOL_ALIGN16(double r);
  _mm_storeh_pd(&r,xmmbounds);
  return r;
#endif
}

GAOL_INLINE void interval::get_bounds(interval::xmm2d& b) const
{
  _mm_store_pd(b,xmmbounds);
}

GAOL_INLINE const interval::xmm_bounds& interval::get_xmminterval(void) const
{
  return xmmbounds;
}

GAOL_INLINE interval::xmm_bounds& interval::get_xmminterval(void)
{
  return xmmbounds;
}

/*
  The sum, the difference, the product, the quotient and the square of
  intervals, on their stored bounds (the left one negated), computed by the
  library: the operators call these functions with the two doubles in a
  register, which they give back in one, where operator+=() was a function of
  the library that read and wrote them through memory, the result going back
  to memory for the operation that followed (GAOL v5, point ZZ of TODO.md).
  They are not part of the interface: they are in gaol_detail, with the other
  helpers of the library, and not in gaol_core, which holds the interface. The
  check of the rounding direction, and the code that depends on it, are in the
  library, compiled with its flags, not in the code of the program that calls
  them. With Visual C++ and clang-cl they are __vectorcall (GAOL_VECTORCALL of
  gaol/gaol_config.h), for the arguments to go in registers: the convention of
  Windows for x86-64 passes a __m128d by a reference to a copy.
*/
} // namespace gaol_core

namespace gaol_detail {
GAOL_NODISCARD extern GAOL_PUBLIC __m128d GAOL_VECTORCALL add_bounds(__m128d x, __m128d y);
GAOL_NODISCARD extern GAOL_PUBLIC __m128d GAOL_VECTORCALL sub_bounds(__m128d x, __m128d y);
GAOL_NODISCARD extern GAOL_PUBLIC __m128d GAOL_VECTORCALL mul_bounds(__m128d x, __m128d y);
GAOL_NODISCARD extern GAOL_PUBLIC __m128d GAOL_VECTORCALL div_bounds(__m128d x, __m128d y);
GAOL_NODISCARD extern GAOL_PUBLIC __m128d GAOL_VECTORCALL sqr_bounds(__m128d x);
} // namespace gaol_detail

namespace gaol_core {

GAOL_INLINE interval& interval::operator+=(const interval& I)
{
  xmmbounds = gaol_detail::add_bounds(xmmbounds, I.xmmbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator-=(const interval& I)
{
  xmmbounds = gaol_detail::sub_bounds(xmmbounds, I.xmmbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator*=(const interval& I)
{
  xmmbounds = gaol_detail::mul_bounds(xmmbounds, I.xmmbounds);
  return *this;
}

GAOL_INLINE interval& interval::operator/=(const interval& I)
{
  xmmbounds = gaol_detail::div_bounds(xmmbounds, I.xmmbounds);
  return *this;
}

//! I^2
GAOL_NODISCARD GAOL_INLINE interval sqr(const interval& I)
{
  return interval(gaol_detail::sqr_bounds(I.get_xmminterval()), gaol_detail::unchecked_bounds);
}



#endif // GAOL_INTERVAL_SSE_H
