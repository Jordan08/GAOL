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
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*
    This file shall be included by gaol_interval.h only. It must not be included directly
        by any other file.
    This file contains all declarations related to the 'interval' class that consider
    the bounds to be an __m128d variable.
**/

#ifndef GAOL_INTERVAL_H
#  error "File gaol_interval_sse.h shall only be included directly by gaol_interval.h"
#endif

#ifndef GAOL_INTERVAL_SSE_H
#define GAOL_INTERVAL_SSE_H

  GAOL_PUBLIC std::ostream& operator<<(std::ostream& out, const __m128d& x);

  GAOL_INLINE interval interval::universe(void)
    {
      return interval(interval::m128_infinf);
    }

  GAOL_INLINE interval interval::zero(void)
    {
      return interval(interval::m128_zero);
    }

  // Both bounds NaN, as interval(double) sets them for a NaN, but without the
  // comparisons that decide it: they signal the invalid-operation exception on
  // a NaN, and a build without optimization runs them (GAOL v5)
  GAOL_INLINE interval interval::emptyset(void)
    {
      return interval(_mm_set1_pd(std::numeric_limits<double>::quiet_NaN()));
    }

  GAOL_INLINE interval::interval(const __m128d& xmm)
    {
      xmmbounds = xmm;
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
  // constructions (GCC 9.4, FPU intervals)
  GAOL_INLINE interval::interval(double l, double r)
    {
      if (gaol_detail::quiet_less_equal(l, r) && l < GAOL_INFINITY && r > -GAOL_INFINITY) {
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
    for 32-bit ARM and POWER9.
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
    if (!gaol_detail::quiet_greater_equal(Ibd[0], bd[0])) { // Left bounds negated
      bd[0] = Ibd[0];
    }
    if (!gaol_detail::quiet_greater_equal(Ibd[1], bd[1])) {
      bd[1] = Ibd[1];
    }

    // Disjoint intervals give the empty set, [NaN, NaN] as
    // interval::emptyset() (GAOL v5): their bounds in the wrong
    // order, [3, 2] for [1, 2] & [3, 4], were empty for is_empty(), but
    // the operations computing on the bounds gave [3, 2] + [0, 1] = [3, 3]
    if (!gaol_detail::quiet_less_equal(-bd[0], bd[1])) {
      xmmbounds = _mm_set1_pd(std::numeric_limits<double>::quiet_NaN());
    } else {
      xmmbounds = _mm_load_pd(bd);
    }
    return *this;
  }

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

    	if (Ibd[0] > bd[0]) { // Left bounds negated
      	bd[0] = Ibd[0];
    	}
    	if (Ibd[1] > bd[1]) {
      	bd[1] = Ibd[1];
    	}
		xmmbounds = _mm_load_pd(bd);
    	return *this;
   }

  GAOL_INLINE double interval::left_internal() const
    {
      GAOL_ALIGN16(double l);
      _mm_storel_pd(&l,xmmbounds);
      return l;
    }
  GAOL_INLINE double interval::right_internal() const
    {
      GAOL_ALIGN16(double r);
      _mm_storeh_pd(&r,xmmbounds);
      return r;
    }

 GAOL_INLINE void interval::get_bounds(interval::xmm2d& b) const
    {
      _mm_store_pd(b,xmmbounds);
    }

  GAOL_INLINE const __m128d& interval::get_xmminterval(void) const
    {
      return xmmbounds;
    }

  GAOL_INLINE __m128d& interval::get_xmminterval(void)
    {
      return xmmbounds;
    }



#endif // GAOL_INTERVAL_SSE_H
