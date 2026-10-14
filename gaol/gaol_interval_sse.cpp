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
        by gaol/gaol_interval.cpp.
*/

// TODO: check existence of stdint.h in configure.in
#include <stdint.h>

	/* Written as vectors, lower double first, rather than computed with
	   _mm_set_pd() and _mm_cmpeq_pd(): the compilers put them in read-only
	   memory, where they are right before the program starts, whereas the
	   dynamic initialization of this file computed them after the static
	   objects of a program linked with the static library GAOL, except with
	   MinGW-w64, whose operations then found them 0 (GAOL v5). The SSE2 intervals are compiled
	   by GCC and Clang only, which know these vectors: none of the three
	   builds compiles them with Visual C++. m128d_02mask and m128d_20mask,
	   whose all-ones half no double writes, are integer vectors seen as
	   doubles; __extension__ keeps -pedantic from warning about them. */
	const __m128d interval::lbsignmask = {-0.0, 0.0};
	const __m128d interval::lbrbsignmask = {-0.0, -0.0};
	const __m128d interval::m128d_02mask = __extension__ (__m128d)(__m128i){0, -1};
	const __m128d interval::m128d_20mask = __extension__ (__m128d)(__m128i){-1, 0};
	const __m128d interval::m128_zero = {0.0, 0.0};
	const __m128d interval::m128_minus_one = {-1.0, 1.0};
	const __m128d interval::m128_nan = {std::numeric_limits<double>::quiet_NaN(),
	                                    std::numeric_limits<double>::quiet_NaN()};
	const __m128d interval::m128_infinf = {std::numeric_limits<double>::infinity(),
	                                       std::numeric_limits<double>::infinity()};

	void* interval::operator new(size_t sz)
	{
		void *buf;
		if (GAOL_MEMALIGN(buf,16,sz)) { // error ?
			throw std::bad_alloc();
		}
		return buf;
	}

    void interval::operator delete(void *p)
    {
        GAOL_MEMFREE(p);
    }

	void* interval::operator new[](size_t sz)
	{
		void *buf;
		if (GAOL_MEMALIGN(buf,16,sz)) { // error ?
			throw std::bad_alloc();
		}
		return buf;
	}



  void interval::operator delete[](void *p)
  {
      GAOL_MEMFREE(p);
  }

	void* interval::operator new(size_t, void *p)
	{
		if ((unsigned long long)p % 16 != 0) {
			throw std::bad_alloc();
		}
		return p;
	}

  // The placement delete, called when the constructor of an interval built
  // by the placement new above throws: the memory is the caller's, and was
  // freed with free(), which it may not have come from
  void interval::operator delete(void *, void *)
  {
  }

	double hausdorff(const interval &I1, const interval &I2)
	{
		if (I1.is_empty() || I2.is_empty()) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		// A bound infinite in one interval only puts them at distance +oo,
		// returned before the check of the rounding direction (GAOL v5, point
		// R.50 of TODO.md; GAOL 4 returned +oo there for every infinite bound):
		// the check and the differences made it two to four times as long
		// (Intel i7-1185G7, GCC 9.4 and Clang 18). Whether a bound is infinite
		// depends neither on the rounding direction nor on the flush-to-zero
		// modes, which read a subnormal bound as a zero, no infinity either. The
		// bounds compared after the check are read after it (GAOL_RND_BARRIER()
		// of gaol/gaol_fpu.h, and /fp:strict with Visual C++)
		if ((std::fabs(I1.left()) == GAOL_INFINITY) != (std::fabs(I2.left()) == GAOL_INFINITY)
		    || (std::fabs(I1.right()) == GAOL_INFINITY) != (std::fabs(I2.right()) == GAOL_INFINITY)) {
			return GAOL_INFINITY;
		}
		// The tightest upper bound of the distance, whatever the rounding
		// direction of the caller: each difference is rounded upward both ways.
		// Equal bounds, infinite ones included, are at distance 0 (inf - inf is a
		// NaN): the distance of [1, +oo] to [1, +oo] is 0 and the one to [2, +oo] is
		// 1, where GAOL 4 returned +oo.
		GAOL_RND_ENTER();
		const double a = I1.left(), b = I1.right(), c = I2.left(), e = I2.right();
		double d = fmax((a == c) ? 0.0 : fmax(a - c, c - a),
		                (b == e) ? 0.0 : fmax(b - e, e - b));
		GAOL_RND_KEEP(d);
		GAOL_RND_LEAVE();
		return d;
	}


interval div_rel(const interval &K, const interval &J, const interval &I)
{
  // One check, before the bounds are compared (GAOL v5, see gaol/gaol_fpu.h):
  // the branches made their own checks, after comparing the bounds with 0,
  // which the denormals-are-zero mode made wrong for the subnormal ones, and
  // with GAOL_PRESERVE_ROUNDING the intersections with I compared bounds after
  // the mode was restored. The division and its intersection with I are made
  // by the lambda, whose returns come back before GAOL_RND_LEAVE_SSE(); it has
  // the access of div_rel(), a friend of interval
  GAOL_RND_ENTER_SSE();
  interval res = [&]() -> interval {
  // The emptiness tests of this file are not worth writing with SSE2
  // intrinsics, as the FIXMEs of GAOL here and in operator*=, operator/= and
  // operator%= asked: testing the bounds of both operands at once, with
  // _mm_cmpnge_pd and _mm_movemask_pd, gives the same results as
  // is_empty() || I.is_empty() but no faster, 1.3 to 1.6 ns rather than 1.0 to
  // 1.2 ns (GCC 9.4, x86-64)
  if (K.is_empty() || J.is_empty() || I.is_empty()) {
	return interval::emptyset();
  }

  // Where a branch keeps one half of a division, the other half is not
  // computed: _mm_div_sd() divides the low halves alone, the kept quotient
  // being moved to the high half where it belongs (GAOL v5, point D.24 of
  // TODO.md). _mm_div_pd() divided both, and the half thrown away divided an
  // infinite bound by an infinite one, which raised the invalid-operation
  // exception (P1 M, div_rel([2, +oo], [-oo, 2], I)), or a finite one by a
  // zero bound, which raised the division-by-zero one (N1 N0, N1 P0, P1 N0,
  // P1 P0); the FPU intervals divide the kept bounds alone. The halves thrown
  // away in N0 N1, N0 P1, P0 N1 and P0 P1 divide a zero bound by a nonzero
  // one, which raises nothing, and stay

  if ( K.right() < 0.0 ) { // [K] N1
    if ( J.right() < 0.0 ) { // [J] N1
      __m128d r = _mm_xor_pd(K.xmmbounds,interval::lbsignmask);
      __m128d r2 = _mm_div_pd(_mm_shuffle_pd(r,r,1),J.xmmbounds);
      return I & interval(r2); // N1 N1
    } else { // [J] P or Z
      if ( J.right() == 0.0 ) { // [J] N0 or Z
        if ( J.left() == 0.0 ) { // [J] Z
          return interval::emptyset(); // N1 Z
        } else {
          __m128d r = _mm_move_sd(interval::m128_infinf,_mm_div_sd(_mm_shuffle_pd(K.xmmbounds,K.xmmbounds,1),J.xmmbounds));
          return I & interval(r); // N1 N0
        }
      } else { // [J] P or M
        if ( J.left() < 0.0 ) { // [J] M // FIXME: replace with SSE code
			interval tmp(-GAOL_INFINITY,K.right()/J.right());
			// The lower bound rounded downward, the rounding direction being
			// upward (GAOL v5: K.right()/J.left() was rounded upward, and
			// the result left out the doubles below it that belong to the hull)
			interval tmp2(-((-K.right())/J.left()),GAOL_INFINITY);
	    	return (I&tmp) | (I&tmp2); // N1 M
        } else { // [J] P0 or P1
          if ( J.left() == 0.0 ) { // [I] P0
            __m128d r = _mm_unpacklo_pd(interval::m128_infinf,
                                        _mm_div_sd(_mm_unpackhi_pd(K.xmmbounds,K.xmmbounds),
                                                   _mm_unpackhi_pd(J.xmmbounds,J.xmmbounds))); // N1 P0
            return I & interval(r);
          } else { // [J] P1
            __m128d r = _mm_div_pd(K.xmmbounds,_mm_xor_pd(J.xmmbounds,interval::lbsignmask)); // N1 P1
            return I & interval(r);
          }
        }
      }
    }
  } else { // [K] N0, Z, M, or P
	if ( K.right() == 0.0 ) { // [K] N0 or Z
	  if ( K.left() == 0.0 ) { // [K] Z
		if ( J.left() <= 0.0 && J.right() >= 0.0 ) {
		  return I; // Z (N0 M, P0)
		} else {
		  return I & interval::zero(); // Z (N1,P1)
		}
	  } else { // [K] N0
		if ( J.right() < 0.0 ) { // [J] N1
			__m128d r1 = _mm_xor_pd(K.xmmbounds,interval::lbsignmask);
			__m128d r2 = _mm_move_sd(_mm_div_pd(_mm_shuffle_pd(r1,r1,1),J.xmmbounds),interval::m128_zero);	// N0 N1
			return I & interval(r2);
		} else { // [J] P or Z
		  if ( J.right() == 0.0 ) { // [J] N0 or Z
        if ( J.left() == 0.0 ) { // [J] Z
          return I; // N0 Z
        } else {
          return I;// N0 N0
        }
		  } else { // [I] P or M
			if ( J.left() < 0.0 ) { // [J] M
			  return I; // N0 M
			} else { // [J] P0 or P1
			  if ( J.left() == 0.0 ) { // [J] P0
          return I; // N0 P0
			  } else { // [J] P1
          __m128d r = _mm_move_sd(interval::m128_zero,_mm_div_pd(K.xmmbounds,_mm_xor_pd(J.xmmbounds,interval::lbsignmask))); 
          return I & interval(r); // N0 P1
			  }
			}
		  }
		}
	  }
	} else { // [K] M, P0, or P1
	  if ( K.left() < 0.0 ) { // [K] M
		if ( J.right() < 0.0 ) { // [J] N1
		  __m128d r1 = _mm_xor_pd(K.xmmbounds,interval::lbrbsignmask);
		  __m128d r2 = _mm_div_pd(_mm_shuffle_pd(r1,r1,1),_mm_unpackhi_pd(J.xmmbounds,J.xmmbounds));
      return I & interval(r2); // M N1
		} else { // [J] P or Z
		  if ( J.right() == 0.0 ) { // [J] N0 or Z
        return I; // M N0
		  } else { // [J] P or M
			if ( J.left() < 0.0 ) { // [J] M
			  return I; // M M
			} else { // [J] P0 or P1
			  if ( J.left() == 0.0 ) { // [J] P0
				return I; // M P0
			  } else { // [J] P1
          __m128d r1 = _mm_xor_pd(J.xmmbounds,interval::lbsignmask);
          __m128d r2 = _mm_div_pd(K.xmmbounds,_mm_unpacklo_pd(r1,r1)); 
          return I & interval(r2); // M P1
			  }
			}
		  }
		}
	  } else { // [K] P0 or P1
		if ( K.left() == 0.0 ) { // [K] P0
		  if ( J.right() < 0.0 ) { // [J] N1
        __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(K.xmmbounds,K.xmmbounds,1),interval::lbsignmask);
        __m128d r2 = _mm_shuffle_pd(J.xmmbounds,J.xmmbounds,1);
        __m128d r3 = _mm_move_sd(interval::m128_zero,_mm_div_pd(r1,r2));
        return I & interval(r3); // P0 N1
		  } else { // [J] P or Z
			if ( J.right() == 0.0 ) { // [J] N0 or Z
        return I; // P0 (Z or N0)
			} else { // [J] P or M
			  if ( J.left() < 0.0 ) { // [J] M
				return I; // P0 M
			  } else { // [J] P0 or P1
				if ( J.left() == 0.0 ) { // [J] P0
				  return I; // P0 P0
				} else { // [J] P1
				  __m128d r1 = _mm_xor_pd(J.xmmbounds,interval::lbsignmask);
				  __m128d r2 = _mm_shuffle_pd(r1,r1,1);
				  __m128d r3 = _mm_move_sd(_mm_div_pd(K.xmmbounds,r2),interval::m128_zero);// P0 P1
          return I & interval(r3);
				}
			  }
			}
		  }
		} else { // [K] P1
		  if ( J.right() < 0.0 ) { // [J] N1
        __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(K.xmmbounds,K.xmmbounds,1),interval::lbrbsignmask);
        __m128d r2 = _mm_xor_pd(J.xmmbounds,interval::lbsignmask);
        __m128d r3 = _mm_div_pd(r1,_mm_shuffle_pd(r2,r2,1)); 
        return I & interval(r3); // P1 N1
      } else { // [J] P or Z
        if ( J.right() == 0.0 ) { // [J] N0 or Z
			  if ( J.left() == 0.0 ) { // [J] Z
				return interval::emptyset(); // P1 Z
			  } else {
          __m128d r = _mm_unpacklo_pd(interval::m128_infinf,_mm_div_sd(K.xmmbounds,J.xmmbounds));
          return I & interval(r); // P1 N0
			  }
			} else { // [J] P or M
			  if ( J.left() < 0.0 ) { // [J] M
          __m128d r = _mm_unpacklo_pd(interval::m128_infinf,_mm_div_sd(K.xmmbounds,J.xmmbounds)); // P1 N0
				  __m128d r2 = _mm_move_sd(interval::m128_infinf,
                        _mm_div_sd(K.xmmbounds,
												_mm_shuffle_pd(J.xmmbounds,J.xmmbounds,1))); // P1 P0
          return (I & interval(r)) | (I & interval(r2)); // P1 M
			  } else { // [J] P0 or P1
				if ( J.left() == 0.0 ) { // [J] P0
				  __m128d r = _mm_move_sd(interval::m128_infinf,
                        _mm_div_sd(K.xmmbounds,
												_mm_shuffle_pd(J.xmmbounds,J.xmmbounds,1))); // P1 P0
          return I & interval(r);
				} else { // [J] P1
				  __m128d r1 = _mm_xor_pd(J.xmmbounds,interval::lbsignmask);
				  __m128d r2 = _mm_div_pd(K.xmmbounds,_mm_shuffle_pd(r1,r1,1));
          return I & interval(r2); // P1 P1
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
  GAOL_RND_LEAVE_SSE();
  return res;
}

  interval interval::operator-() const
  {
    return interval(_mm_shuffle_pd(xmmbounds,xmmbounds,_MM_SHUFFLE2(0,1)));
  }

  /*
    The square from one product, as the FPU intervals compute it (GAOL v5,
    point AA of TODO.md). It took the binary exponentiation of the products
    rounded outward, uipow_rounded(I,2), whose last product, of 1 by the
    square, is exact, and gives the same bounds, bit for bit: Clang 18 kept
    its loop for e = 2, two products and three branches, and sqr() took
    4.4 ns rather than 2.5 (gaol_performance, Intel Xeon E3-1220 v6), 9.4 ns
    rather than 7.2 in the benchmark of doc/compare, where GAOL 4.2.3 takes
    7.6. sqr() was the last caller of uipow_rounded(), uipow_upup() and
    uipow_dnup(), gone with it, as from the FPU intervals (x^n takes exact
    products, see pown_products() in gaol_interval.cpp).
  */
  interval sqr(const interval& I)
  {
    if (I.is_empty()) {
        return I;
    }
    GAOL_RND_ENTER_SSE();
    __m128d Ix = I.get_xmminterval();
    __m128d res;
    switch (_mm_movemask_pd(Ix)) { // Remember that the sign of the left bound is negated
    case 0: // 00: I.straddles_zero(), <0, max(rndup(l^2), rndup(r^2))>
      {
	__m128d sq = _mm_mul_pd(Ix,Ix);
	res = _mm_and_pd(_mm_max_pd(sq,_mm_shuffle_pd(sq,sq,1)),interval::m128d_02mask);
	break;
      }
    case 1: // 01: I.positive(), <-rnddn(l^2), rndup(r^2)>
      res = _mm_mul_pd(Ix,_mm_xor_pd(Ix,interval::lbsignmask));
      break;
    case 2: // 10: I.negative(), <-rnddn(r^2), rndup(l^2)>
      Ix = _mm_shuffle_pd(Ix,Ix,1);
      res = _mm_mul_pd(Ix,_mm_xor_pd(Ix,interval::lbsignmask));
      break;
    default: // 11: I=[0, -0] (empty intervals have been filtered already)
      res = _mm_set_pd(0.0,-0.0);
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE_SSE();
    return interval(res);
  }


 interval& interval::operator+=(const interval& I)
    {
#if GAOL_HAVE_AVX512_TARGET
      // The AVX-512 path: no rounding direction to set, the flush-to-zero
      // modes cleared where they are set (gaol/gaol_interval_avx512.cpp)
      if (avx512_arithmetic) {
        return fast_add_assign(*this, I);
      }
#endif
      GAOL_RND_ENTER_SSE();
      xmmbounds = _mm_add_pd(xmmbounds, I.xmmbounds);
      GAOL_RND_LEAVE_SSE();
      return *this;
    }

  interval& interval::operator-=(const interval& I)
    {
#if GAOL_HAVE_AVX512_TARGET
      if (avx512_arithmetic) {
        return fast_sub_assign(*this, I);
      }
#endif
      GAOL_RND_ENTER_SSE();
      xmmbounds = _mm_add_pd(xmmbounds, _mm_shuffle_pd(I.xmmbounds,I.xmmbounds,_MM_SHUFFLE2(0,1)));
      GAOL_RND_LEAVE_SSE();
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
#if GAOL_HAVE_AVX512_TARGET
      if (avx512_arithmetic) {
        return fast_add_assign(*this, d); // <-l, r> + <-d, d>
      }
#endif
      GAOL_RND_ENTER_SSE();
      xmmbounds = _mm_add_pd(xmmbounds, _mm_set_pd(d, -d)); // <-l, r> + <-d, d>
      GAOL_RND_LEAVE_SSE();
      return *this;
    }

  interval& interval::operator-=(double d)
    {
      if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
        *this = interval::emptyset();
        return *this;
      }
#if GAOL_HAVE_AVX512_TARGET
      if (avx512_arithmetic) {
        return fast_add_assign(*this, -d); // <-l, r> + <d, -d>
      }
#endif
      GAOL_RND_ENTER_SSE();
      xmmbounds = _mm_add_pd(xmmbounds, _mm_set_pd(-d, d)); // <-l, r> + <d, -d>
      GAOL_RND_LEAVE_SSE();
      return *this;
    }

  interval& interval::operator*=(double d)
    {
      if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
        *this = interval::emptyset();
        return *this;
      }
#if GAOL_HAVE_AVX512_TARGET
      /* The AVX-512 path: the zero and the sign of d read from its bits,
         which the denormals-are-zero mode does not change, where the SSE2
         path checks the modes before comparing d (a subnormal d was 0), and
         the product carries its rounding direction itself */
      // The product of the interval by [d, d]: the dispatch of fast_mul()
      // makes it, from the stored form of [d, d], its check clearing the
      // flush-to-zero modes, which the SSE2 path checks before this branch.
      // The zero of d, of either sign, gives the stored zero, as the SSE2
      // path does: the dispatch would give the signs of its zeros instead,
      // and the guard keeps the modes as the program left them, as the
      // dispatch keeps them
      if (avx512_arithmetic) {
        if (bit_zero(d)) {
          const flush_guard flush;
          xmmbounds = interval::m128_zero;
          return *this;
        }
        return fast_mul_assign(*this, d);
      }
#endif
      // The rounding direction checked before d is compared: with the modes
      // that flush the subnormals to zero, a subnormal d was 0 (GAOL v5). d
      // is read again after the check, which the compiler could otherwise
      // compare it before (see gaol/gaol_fpu.h)
      GAOL_RND_ENTER_SSE();
      d = rnd_reread(d);
      if (d == 0.0) {
        xmmbounds = interval::m128_zero;
        GAOL_RND_LEAVE_SSE();
        return *this;
      }
      if (d > 0.0) {
        xmmbounds = _mm_mul_pd(xmmbounds, _mm_set1_pd(d)); // <-l*d, r*d>
      } else { // d < 0.0
        xmmbounds = _mm_mul_pd(_mm_shuffle_pd(xmmbounds, xmmbounds, 1), _mm_set1_pd(-d)); // <r*(-d), -l*(-d)>
      }
      GAOL_RND_LEAVE_SSE();
      return *this;
    }

  interval& interval::operator/=(double d)
    {
      if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
        *this = interval::emptyset();
        return *this;
      }

#if GAOL_HAVE_AVX512_TARGET
      /* The AVX-512 path: the quotients lane by lane, as the SSE2 path
         divides them, the zero and the sign of d from its bits, the empty
         set of a zero divisor; the modes of the zero case are cleared by
         the guard, which gives them back as the program left them */
      if (avx512_arithmetic) {
        if (bit_zero(d)) {
          const flush_guard flush;
          *this = interval::emptyset();
          return *this;
        }
        return fast_div_assign(*this, d);
      }
#endif
      // The rounding direction checked before d is compared, as in
      // operator*=(double) (GAOL v5)
      GAOL_RND_ENTER_SSE();
      d = rnd_reread(d);
      if (d == 0.0) {
        *this = interval::emptyset();
        GAOL_RND_LEAVE_SSE();
        return *this;
      }
      if (d > 0.0) {
        xmmbounds = _mm_div_pd(xmmbounds, _mm_set1_pd(d)); // <-l/d, r/d>
      } else { // d < 0.0
        xmmbounds = _mm_div_pd(_mm_shuffle_pd(xmmbounds, xmmbounds, 1), _mm_set1_pd(-d)); // <r/(-d), -l/(-d)>
      }
      GAOL_RND_LEAVE_SSE();
      return *this;
    }

  interval& interval::operator%=(double d)
    {
      if (is_empty() || !(gaol_detail::quiet_less(-GAOL_INFINITY, d) && gaol_detail::quiet_less(d, GAOL_INFINITY))) { // interval(d) is empty for an infinite d and for a NaN
        *this = interval::emptyset();
        return *this;
      }
      // The rounding direction checked before d and the bounds are compared, as
      // in operator*=(double) (GAOL v5)
      GAOL_RND_ENTER_SSE();
      d = rnd_reread(d);
      if (d == 0.0) { // x*0 is in [this] for any x when [this] contains 0
        *this = straddles_zero() ? interval::universe() : interval::emptyset();
        GAOL_RND_LEAVE_SSE();
        return *this;
      }
      if (d > 0.0) {
        xmmbounds = _mm_div_pd(xmmbounds, _mm_set1_pd(d)); // <-l/d, r/d>
      } else { // d < 0.0
        xmmbounds = _mm_div_pd(_mm_shuffle_pd(xmmbounds, xmmbounds, 1), _mm_set1_pd(-d)); // <r/(-d), -l/(-d)>
      }
      GAOL_RND_LEAVE_SSE();
      return *this;
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
    double l = left_internal();
    double r = right_internal();
    // As midpoint(), rounded outward: the half of the sum of the bounds, or the
    // sum of their halves where it may overflow, each half rounded outward too
    // (halving a bound below 2^-1021 is not exact). The sum may overflow only
    // where a bound is 2^1023 or more in magnitude, which is tested first, so
    // that mid([DBL_MAX]) raises no overflow exception (GAOL v5: the sum was
    // computed first, and its overflow told the other case); the sum of the
    // halves gives the same bounds as the half of the sum there
    double mid_left, mid_right;
    const double big = 8.9884656743115795e+307; // 2^1023 (C++11 has no hexadecimal floating literal)
    if (std::fabs(l) >= big || std::fabs(r) >= big) {
      mid_left  = -(.5*l + (-.5)*r);
      mid_right = (-.5)*l + .5*r;
    } else {
      double sum = r - l;     // a+b, rounded upward
      double opp_sum = l - r; // -(a+b), rounded upward
      mid_left  = -(.5*opp_sum);
      mid_right = .5*sum;
    }
    GAOL_RND_KEEP(mid_left); GAOL_RND_KEEP(mid_right);
    GAOL_RND_LEAVE();
    return interval(mid_left,mid_right);
  }

#if 0
interval& interval::operator*=(const interval& I)
{
  if (is_empty() || I.is_empty()) {
	*this = interval::emptyset();
	return *this;
  }
  
  if (is_zero()) {
	return *this;
  }
  if (I.is_zero()) {
	xmmbounds=interval::m128_zero;
	return *this;
  }
  GAOL_RND_ENTER_SSE();
  
  if (certainly_positive()) { // P ?
	if (I.certainly_positive()) { // P P
	  xmmbounds = _mm_mul_pd(_mm_xor_pd(xmmbounds,lbsignmask),I.xmmbounds);
	} else {
	  if (I.strictly_straddles_zero()) { // P M
		xmmbounds = _mm_mul_pd(_mm_unpackhi_pd(xmmbounds,xmmbounds),I.xmmbounds);
	  } else { // P N
		__m128d ba = _mm_xor_pd(xmmbounds,lbsignmask);
		xmmbounds = _mm_mul_pd(_mm_shuffle_pd(ba,ba,1),I.xmmbounds);
	  }
	}
  } else {
	if (strictly_straddles_zero()) { // M ?
	  if (I.certainly_positive()) { // M P
		xmmbounds = _mm_mul_pd(xmmbounds,_mm_unpackhi_pd(I.xmmbounds,I.xmmbounds));
	  } else {
		if (I.strictly_straddles_zero()) { // M M
		  xmmbounds = _mm_max_pd(_mm_mul_pd(_mm_unpacklo_pd(xmmbounds,xmmbounds),
											_mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1)),
								 _mm_mul_pd(_mm_unpackhi_pd(xmmbounds,xmmbounds),
											I.xmmbounds));
		} else { // M N
		  xmmbounds = _mm_mul_pd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),
								 _mm_unpacklo_pd(I.xmmbounds,I.xmmbounds));
		}
	  }
	} else { // N  (case Z already tested)
	  if (I.certainly_positive()) { // N P
		__m128d tmp = _mm_xor_pd(I.xmmbounds,lbsignmask);
		xmmbounds = _mm_mul_pd(xmmbounds,_mm_shuffle_pd(tmp,tmp,1));
	  } else {
		if (I.strictly_straddles_zero()) { // N M
		  xmmbounds = _mm_mul_pd(_mm_unpacklo_pd(xmmbounds,xmmbounds),
								 _mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1));
		} else { // N N
		  xmmbounds = _mm_mul_pd(_mm_xor_pd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),lbsignmask),
								 _mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1));
		}
	  }
	}
  }
  
  GAOL_RND_LEAVE_SSE();
  return *this;
}
#endif 
#if 0
// Modified Lambov algorithm
  interval& interval::operator*=(const interval& Itv)
    {
      if (is_empty() || Itv.is_empty()) {
       *this = interval::emptyset();
       return *this;
       }
      GAOL_RND_ENTER_SSE();

      // *this = <b, -a>
      // Itv = <d, -c>
      __m128d A = _mm_xor_pd(xmmbounds,interval::lbsignmask);  	// < b, a >
      __m128d B = _mm_shuffle_pd(Itv.xmmbounds,Itv.xmmbounds,1); 	// < -c, d >
      __m128d C = _mm_cmplt_pd(A,interval::m128_zero);				// < b<0, a<0 >
      __m128d D = _mm_xor_pd(B,interval::lbrbsignmask);		// < c, -d >
      __m128d G = _mm_shuffle_pd(C,C,1); 				// < a<0, b<0 >
      __m128d E = _mm_and_pd(C,D); 						// < (b<0)?c:0, (a<0)?-d:0 >
      __m128d F = _mm_andnot_pd(C,Itv.xmmbounds);		// < (b>=0)?d:0, (a>=0)?-c:0 >
      __m128d K = _mm_andnot_pd(G,Itv.xmmbounds);		// < (a>=0)?d:0, (b>=0)?-c:0 >
      __m128d I = _mm_and_pd(D,G);						// < (a<0)?c:0, (b<0)?-d:0 >
      __m128d H = _mm_or_pd(E,F);						// < (b<0)?c:d, (a<0)?-d:-c >
      __m128d L = _mm_shuffle_pd(A,A,1);				// < a, b >
      __m128d M = _mm_or_pd(I,K);						// < (a<0)?c:d, (b<0)?-d:-c >
      __m128d J = _mm_mul_pd(A,H);						// < (b<0)?bc:bd, (a<0)?a(-d):a(-c) >
      __m128d N = _mm_mul_pd(L,M);						// < (a<0)?ac:ad, (b<0)?b(-d):b(-c) >
      __m128d ordJ = _mm_cmpord_pd(J,J);				//
      __m128d newJ = _mm_and_pd(J,ordJ);
      __m128d ordN = _mm_cmpord_pd(N,N);
      __m128d newN = _mm_and_pd(N,ordN);

      xmmbounds = _mm_max_pd(newJ,newN);							// < (b<0)?max(ac,bc)
      // 		 :(a<0)?max(ac,bd)
      //					 :max(ad,bd),
      //	  (b<0)?max(b(-d),a(-d))
      //        :(a<0)?max(b(-c),a(-d))
      //			       :max(b(-c),a(-c)) >
      GAOL_RND_LEAVE_SSE();
      return *this;
    }
#endif 

#if 0
// Brute force algorithm
interval& interval::operator*=(const interval& I)
{
        if (is_empty() || I.is_empty()) {
                *this = interval::emptyset();
                return *this;
        }

	  GAOL_RND_ENTER_SSE();
        __m128d r1  = _mm_shuffle_pd(xmmbounds,xmmbounds,1);            // r1  = <-a, b>
        __m128d r2  = _mm_xor_pd(r1,lbrbsignmask);                    // r2  = <a, -b>
        __m128d r3  = _mm_xor_pd(I.xmmbounds,lbsignmask);             // r3  = <d, c>
        __m128d r4  = _mm_shuffle_pd(r3,r3,1);                          // r4  = <c, d>
        __m128d r5  = _mm_unpacklo_pd(I.xmmbounds,I.xmmbounds);         // r5  = <-c, -c>
        __m128d r6  = _mm_unpackhi_pd(I.xmmbounds,I.xmmbounds);         // r6  = <d, d>
        __m128d r7  = _mm_mul_pd(xmmbounds,r3);                         // r7  = <bd, (-a)c>
		__m128d ordr7 = _mm_cmpord_pd(r7,r7);
		r7 = _mm_and_pd(r7,ordr7);
        __m128d r8  = _mm_mul_pd(r1,r5);                                // r8  = <ac, b(-c)>
		__m128d ordr8 = _mm_cmpord_pd(r8,r8);
		r8 = _mm_and_pd(r8,ordr8);
        __m128d r9  = _mm_mul_pd(xmmbounds,r4);                         // r9  = <bc, (-a)d>
		__m128d ordr9 = _mm_cmpord_pd(r9,r9);
		r9 = _mm_and_pd(r9,ordr9);
        __m128d r10 = _mm_mul_pd(r2,r6);                                // r10 = <ad,(-b)d>
		__m128d ordr10 = _mm_cmpord_pd(r10,r10);
		r10 = _mm_and_pd(r10,ordr10);
        __m128d r11 = _mm_max_pd(r7, r8);
        __m128d r12 = _mm_max_pd(r9, r10);
        xmmbounds   = _mm_max_pd(r11,r12);
		GAOL_RND_LEAVE_SSE();
        return *this;

}
#endif

#if 1
/*
  The products of the pairs of stored bounds u and v, lane by lane, for
  operator*= below. Where neither has an infinite lane, the plain products,
  which are never NaN. Otherwise a lane with a factor 0 and the other infinite
  gives +0, the product of such bounds in interval arithmetic, as the
  ordered-mask of the products gave from the NaN of 0*oo; the multiplication
  itself is then made with both factors +0 there, since 0*oo raised the
  invalid-operation exception, which killed a program that enabled it:
  [0]*[1, +oo], [0, +oo]*[0], and pow([1], [1, +oo]) through them (GAOL v5).
  The other lanes are the plain products, so that every product has the bits
  it had (checked against the former code on the products of the intervals
  with bounds among 23 special values: the infinities, the largest doubles,
  2^1023, the subnormals, the zeros of both signs...).
*/
namespace {
  struct finite_products
  {
    __m128d operator()(__m128d u, __m128d v) const
    {
      return _mm_mul_pd(u, v);
    }
  };

  struct products_with_infinities
  {
    __m128d operator()(__m128d u, __m128d v) const
    {
      // Quiet comparisons (cmpeqpd), on bounds that are not NaN anyway
      const __m128d abs_u = _mm_andnot_pd(interval::lbrbsignmask, u);
      const __m128d abs_v = _mm_andnot_pd(interval::lbrbsignmask, v);
      const __m128d zero_by_infinity =
        _mm_or_pd(_mm_and_pd(_mm_cmpeq_pd(u, interval::m128_zero), _mm_cmpeq_pd(abs_v, interval::m128_infinf)),
                  _mm_and_pd(_mm_cmpeq_pd(v, interval::m128_zero), _mm_cmpeq_pd(abs_u, interval::m128_infinf)));
      return _mm_mul_pd(_mm_andnot_pd(zero_by_infinity, u), _mm_andnot_pd(zero_by_infinity, v));
    }
  };

  // Test algorithm (9 cases, à la Wolff von Gudenberg): the product of the
  // nonempty x and y, from the products mul makes of their stored bounds
  template<class Products>
  GAOL_INLINE __m128d product_of_bounds(__m128d x, __m128d y, const Products& mul)
  {
    __m128d res = interval::m128_zero;
    int signs = (_mm_movemask_pd(y) << 2) | _mm_movemask_pd(x);
    // signs: bits 0 and 2 shall be negated since the left bounds are stored negated
    switch (signs) {
	case 3:	 // 0011b -> 0110b
	case 7:  // 0111b -> 0010b
	case 11: // 1011b -> 1110b
	case 12: // 1100b -> 1001b
	case 13: // 1101b -> 1000b
	case 15: // 1111b -> 1010b
	case 14: // 1110b -> 1011b
	  // [0, 0]
	  {
		res = interval::m128_zero;
	  }
	  break;
	case 0:  // 0000b -> 0101b
	  // (5) [min(ad,bc), max(ac, bd)]
	  {
		__m128d r0 = _mm_unpacklo_pd(x,x);
		__m128d r1 = _mm_shuffle_pd(y,y,1);
		__m128d r2 = mul(r0, r1);
		__m128d r3 = _mm_unpackhi_pd(x,x);
		__m128d r4 = mul(r3, y);
		res = _mm_max_pd(r2,r4);
	  }
	  break;
	case 1:  // 0001b -> 0100b
	  // (8) [bc, bd]
	  {
		__m128d r0 = _mm_unpackhi_pd(x,x);
		res = mul(r0,y);
	  }
	  break;
	case 2:  // 0010b -> 0111b
	  // (2) [ad, ac]
	  {
		__m128d r0 = _mm_unpacklo_pd(x,x);
		__m128d r1 = _mm_shuffle_pd(y,y,1);
		res = mul(r0, r1);
	  }
	  break;
	case 4:  // 0100b -> 0001b
	  // (6) [ad, bd]
	  {
		__m128d r0 = _mm_unpackhi_pd(y,y);
		res = mul(x, r0);
	  }
	  break;
	case 5:  // 0101b -> 0000b
	  // (9) [ac, bd]
	  {
		__m128d r0 = _mm_xor_pd(x,interval::lbsignmask);
		res = mul(r0,y);
	  }
	  break;
	case 6:  // 0110b -> 0011b
	  // (3) [ad, bc]
	  {
		__m128d r0 = _mm_xor_pd(y,interval::lbsignmask);
		__m128d r1 = _mm_shuffle_pd(r0,r0,1);
		res = mul(x,r1);
	  }
	  break;
	case 8:  // 1000b -> 1101b
	  // (4) [bc, ac]
	  {
		__m128d r0 = _mm_shuffle_pd(x,x,1);
		__m128d r1 = _mm_unpacklo_pd(y,y);
		res = mul(r0, r1);
	  }
	  break;
	case 9:  // 1001b -> 1100b
	  // (7) [bc, ad]
	  {
		__m128d r0 = _mm_xor_pd(x,interval::lbsignmask);
		__m128d r1 = _mm_shuffle_pd(r0,r0,1);
		res = mul(r1,y);
	  }
	  break;
	case 10: // 1010b -> 1111b
	  // (1) [bd, ac]
	  {
		__m128d r0 = _mm_shuffle_pd(x,x,1);
		__m128d r1 = _mm_xor_pd(r0, interval::lbsignmask);
		__m128d r2 = _mm_shuffle_pd(y,y,1);
		res = mul(r1,r2);
	  }
	  break;
    }
    return res;
  }

  // x *= y for nonempty x and y with an infinite bound, out of operator*=,
  // where it is rare, and its code with it (the SSE2 intervals are compiled
  // by GCC and Clang only). operator*= calls it last: GCC 13 makes it a
  // tail call, Clang 18 a call, and both save registers in operator*=,
  // which also calls fast_mul() on the AVX-512 path (GAOL v5, point AA of
  // TODO.md)
  __attribute__((noinline, cold)) interval& multiply_with_infinities(interval& x, const interval& y)
  {
    x.get_xmminterval() = product_of_bounds(x.get_xmminterval(), y.get_xmminterval(), products_with_infinities());
    return x;
  }
}

interval& interval::operator*=(const interval& I)
{
  if (is_empty() || I.is_empty()) {
	*this = interval::emptyset();
	return *this;
  }

#if GAOL_HAVE_AVX512_TARGET
  /* The AVX-512 path: the products by the embedded rounding, the dispatch
     and the infinite bounds by the sign and the magnitude bits, which the
     denormals-are-zero mode does not change (the SSE2 path checks the
     modes for the comparisons below) */
  if (avx512_arithmetic) {
    xmmbounds = fast_mul(xmmbounds, I.xmmbounds);
    return *this;
  }
#endif

  GAOL_RND_ENTER_SSE();

  /*
    An infinite bound in either operand: their stored bounds are -l and r,
    so that +oo is the only infinity they hold, GAOL building no interval
    [+oo, +oo] nor [-oo, -oo]. maxpd and cmpeqpd raise nothing, no bound
    being NaN in nonempty operands. That test costs the product of two
    intervals 0.15 to 0.35 ns on an Intel i7-1185G7 (gaol_performance,
    medians of 9 interleaved runs: 4.22 ns rather than 3.88 ns with Clang 18,
    4.93 to 5.16 ns rather than 4.80 ns with GCC 9.4, in two series; GAOL v5).
  */
  if (_mm_movemask_pd(_mm_cmpeq_pd(_mm_max_pd(xmmbounds, I.xmmbounds), interval::m128_infinf)) != 0) {
    interval& res = multiply_with_infinities(*this, I);
    GAOL_RND_LEAVE_SSE();
    return res;
  }
  xmmbounds = product_of_bounds(xmmbounds, I.xmmbounds, finite_products());
  GAOL_RND_LEAVE_SSE();
  return *this;
}
#endif


/*
  Where a branch keeps one half of a division, the other half is not
  computed, as in div_rel(): _mm_div_sd() divides the low halves alone, the
  kept quotient being moved to the high half where it belongs (GAOL v5). The
  half thrown away divided a nonzero bound by a zero one (N1 N0, N1 P0,
  P1 N0, P1 P0), which raised the division-by-zero exception, [1, 2]/[0, 2]
  among them, where the FPU intervals, which divide the kept bounds alone,
  raise nothing. operator%=() below does the same.
*/
interval& interval::operator/=(const interval& I)
{ // TODO: replace left() and right() by local variable
	if (is_empty() || I.is_empty()) {
		*this = interval::emptyset();
		return *this;
	}
#if GAOL_HAVE_AVX512_TARGET
	/* The AVX-512 path: the quotients by the embedded rounding, the
	   dispatch by the sign and the magnitude bits (see operator*= above) */
	if (avx512_arithmetic) {
		return fast_div_assign(*this, I);
	}
#endif
	GAOL_RND_ENTER_SSE();

	if ( right() < 0.0 ) { // [this] N1
		if ( I.right() < 0.0 ) { // [I] N1
			__m128d r = _mm_xor_pd(xmmbounds,interval::lbsignmask);
			xmmbounds = _mm_div_pd(_mm_shuffle_pd(r,r,1),I.xmmbounds); // N1 N1
		} else { // [I] P or Z
			if ( I.right() == 0.0 ) { // [I] N0 or Z
				if ( I.left() == 0.0 ) { // [I] Z
					*this = interval::emptyset(); // N1 Z
				} else {
					xmmbounds = _mm_move_sd(interval::m128_infinf,_mm_div_sd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),I.xmmbounds)); // N1 N0
				}
			} else { // [I] P or M
				if ( I.left() < 0.0 ) { // [I] M
					*this = interval::universe(); // N1 M
				} else { // [I] P0 or P1
					if ( I.left() == 0.0 ) { // [I] P0
						xmmbounds = _mm_unpacklo_pd(interval::m128_infinf,
						                           _mm_div_sd(_mm_unpackhi_pd(xmmbounds,xmmbounds),
						                                      _mm_unpackhi_pd(I.xmmbounds,I.xmmbounds))); // N1 P0
					} else { // [I] P1
						xmmbounds = _mm_div_pd(xmmbounds,_mm_xor_pd(I.xmmbounds,interval::lbsignmask)); // N1 P1
					}
				}
			}
		}
	} else { // [this] N0, Z, M, or P
		if ( right() == 0.0 ) { // [this] N0 or Z
			if ( left() == 0.0 ) { // [this] Z
				if ( I.left() == 0.0 && I.right() == 0.0 ) {
					*this = interval::emptyset(); // Z Z
				} else {
					xmmbounds = interval::m128_zero; // Z (N,M,P)
				}
			} else { // [this] N0
				if ( I.right() < 0.0 ) { // [I] N1
					__m128d r1 = _mm_xor_pd(xmmbounds,interval::lbsignmask);
					xmmbounds = _mm_move_sd(_mm_div_pd(_mm_shuffle_pd(r1,r1,1),I.xmmbounds),interval::m128_zero);	// N0 N1
				} else { // [I] P or Z
					if ( I.right() == 0.0 ) { // [I] N0 or Z
						if ( I.left() == 0.0 ) { // [I] Z
							*this = interval::emptyset(); // N0 Z
						} else {
							*this = interval::positive();// N0 N0
						}
					} else { // [I] P or M
						if ( I.left() < 0.0 ) { // [I] M
							*this = interval::universe(); // N0 M
						} else { // [I] P0 or P1
							if ( I.left() == 0.0 ) { // [I] P0
								*this = interval::negative(); // N0 P0
							} else { // [I] P1
								xmmbounds = _mm_move_sd(interval::m128_zero,_mm_div_pd(xmmbounds,_mm_xor_pd(I.xmmbounds,interval::lbsignmask))); // N0 P1
							}
						}
					}
				}
			}
		} else { // [this] M, P0, or P1
			if ( left() < 0.0 ) { // [this] M
				if ( I.right() < 0.0 ) { // [I] N1
					__m128d r1 = _mm_xor_pd(xmmbounds,interval::lbrbsignmask);
					xmmbounds = _mm_div_pd(_mm_shuffle_pd(r1,r1,1),_mm_unpackhi_pd(I.xmmbounds,I.xmmbounds));// M N1
				} else { // [I] P or Z
					if ( I.right() == 0.0 ) { // [I] N0 or Z
						if ( I.left() == 0.0 ) { // [I] Z
							*this = interval::emptyset(); // M Z
						} else {
							*this = interval::universe(); // M N0
						}
					} else { // [I] P or M
						if ( I.left() < 0.0 ) { // [I] M
							*this = interval::universe(); // M M
						} else { // [I] P0 or P1
							if ( I.left() == 0.0 ) { // [I] P0
								*this = interval::universe(); // M P0
							} else { // [I] P1
								__m128d r1 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
								xmmbounds = _mm_div_pd(xmmbounds,_mm_unpacklo_pd(r1,r1)); // M P1
							}
						}
					}
				}
			} else { // [this] P0 or P1
				if ( left() == 0.0 ) { // [this] P0
					if ( I.right() < 0.0 ) { // [I] N1
						__m128d r1 = _mm_xor_pd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),interval::lbsignmask);
						__m128d r2 = _mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1);
						xmmbounds = _mm_move_sd(interval::m128_zero,_mm_div_pd(r1,r2));// P0 N1
					} else { // [I] P or Z
						if ( I.right() == 0.0 ) { // [I] N0 or Z
							if ( I.left() == 0.0 ) { // [I] Z
								*this = interval::emptyset(); // P0 Z
							} else {
								*this = interval::negative(); // P0 N0
							}
						} else { // [I] P or M
							if ( I.left() < 0.0 ) { // [I] M
								*this = interval::universe(); // P0 M
							} else { // [I] P0 or P1
								if ( I.left() == 0.0 ) { // [I] P0
									*this = interval::positive(); // P0 P0
								} else { // [I] P1
									__m128d r1 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
									__m128d r2 = _mm_shuffle_pd(r1,r1,1);
									xmmbounds = _mm_move_sd(_mm_div_pd(xmmbounds,r2),interval::m128_zero);// P0 P1
								}
							}
						}
					}
				} else { // [this] P1
					if ( I.right() < 0.0 ) { // [I] N1
						__m128d r1 = _mm_xor_pd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),interval::lbrbsignmask);
						__m128d r2 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
						xmmbounds = _mm_div_pd(r1,_mm_shuffle_pd(r2,r2,1)); // P1 N1
					} else { // [I] P or Z
						if ( I.right() == 0.0 ) { // [I] N0 or Z
							if ( I.left() == 0.0 ) { // [I] Z
								*this = interval::emptyset(); // P1 Z
							} else {
								xmmbounds = _mm_unpacklo_pd(interval::m128_infinf,_mm_div_sd(xmmbounds,I.xmmbounds)); // P1 N0
							}
						} else { // [I] P or M
							if ( I.left() < 0.0 ) { // [I] M
								*this = interval::universe(); // P1 M
							} else { // [I] P0 or P1
								if ( I.left() == 0.0 ) { // [I] P0
									xmmbounds = _mm_move_sd(interval::m128_infinf,
									                        _mm_div_sd(xmmbounds,_mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1))); // P1 P0
								} else { // [I] P1
									__m128d r1 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
									xmmbounds = _mm_div_pd(xmmbounds,_mm_shuffle_pd(r1,r1,1));// P1 P1
								}
							}
						}
					}
				}
			}
		}
	}

	GAOL_RND_LEAVE_SSE();
	return *this;
}

// The branch #else, not compiled, has no inverse_upward(), which the integer
// powers and nth_root(I, q) for q < 0 call (GAOL v5)
#define OPT_INV 1
#if OPT_INV
/*
  The rounding direction being upward already (see gaol_interval.h). A zero
  bound is divided by 1 rather than 0, its quotient being thrown away: -1/0
  raised the division-by-zero exception, inverse([0, 2]) and pow([0, 2], -1)
  among them, where the FPU intervals raise nothing (GAOL v5)
*/
interval interval::inverse_upward(const interval& I)
{
  const __m128d xmmbounds = I.xmmbounds;
  __m128d r1 = _mm_xor_pd(xmmbounds,interval::lbsignmask); // r1 == < d, c >
  __m128d r2 = _mm_shuffle_pd(r1,r1,1); // r2 == <c, d>
  __m128d r4 = _mm_cmpeq_pd(r2,interval::m128_zero); // r4 == < c==0, d==0 >
  // r3 == < -1/c, -1/d >, -1 or 1 where c or d is +-0
  __m128d r3 = _mm_div_pd(interval::m128_minus_one,_mm_or_pd(r2,_mm_and_pd(r4,_mm_set1_pd(1.0))));
  __m128d r5 = _mm_shuffle_pd(r4,r4,1); // r5 == < d==0, c==0 >
  __m128d r6 = _mm_and_pd(r4,r5); // r6 == < c==0 && d==0, c==0 && d==0 >
  __m128d r7 = _mm_and_pd(r6,interval::m128_nan); // < r7 == < (c==0 && d==0) ? NaN : 0, (c==0 && d==0) ? NaN : 0 >
  __m128d r8 = _mm_and_pd(r4,interval::m128_infinf); // r8 == < (c==0) ? inf : 0, (d==0) ? inf : 0 >
  __m128d r9 = _mm_andnot_pd(r4,r3); // r9 == < (c!=0) ? -1/c : 0, (d!=0) ? -1/d : 0 >
  __m128d r10 = _mm_or_pd(r9,r8); // r10 == < (c==0) ? inf : -1/c, (d==0) ? inf : -1/d >
  __m128d r11 = _mm_andnot_pd(r6,r10); // r11 == < (c!=0 || d!=0) ? hi(r10) : 0, (c!=0 || d!=0) ? lo(r10) : 0 >
  __m128d r12 = _mm_or_pd(r11,r7); // r12 == < (c==0 && d==0) ? NaN : hi(r10), (c==0 && d==0) ? NaN : lo(r10) >
  __m128d r13 = _mm_cmpgt_pd(xmmbounds,interval::m128_zero); // r13 == < d>0, (-c)>0 >
  __m128d r14 = _mm_shuffle_pd(r13,r13,1); // r14 == < (-c)>0, d>0 >
  __m128d r15 = _mm_and_pd(r13,r14); // r15 == < d>0 && (-c)>0, (-c)>0 && d>0 >
  __m128d r16 = _mm_and_pd(r15, interval::m128_infinf); // r14 == < ((-c)>0 && d>0) ? inf : 0, ((-c)>0 && d>0) ? inf : 0 >
  __m128d r17 = _mm_andnot_pd(r15, r12); // r17 ==  < ((-c)<=0 || d<=0) ? hi(r12) : 0, ((-c)<=0 || d<=0) ? lo(r12) : 0 >
  __m128d r18 = _mm_or_pd(r16,r17); // < ((-c)>0 && d>0) ? inf : hi(r12), ((-c)>0 && d>0) ? inf : lo(r12) >
  return interval(r18);
}

interval interval::inverse() const
{
  if (is_empty()) {
    return *this;
  }
  GAOL_RND_ENTER_SSE();
  __m128d res = inverse_upward(*this).get_xmminterval();
  GAOL_RND_KEEP(res);
  GAOL_RND_LEAVE_SSE();
  return interval(res);
}
#else
  interval interval::inverse() const
  {
    if (is_empty()) {
      return *this;
    }

    interval res;
    xmm2d itv;
    get_bounds(itv);
    GAOL_RND_ENTER_SSE();
    if (itv[1] <= 0.0) { // (Z N0 N1)
      if (itv[0] > 0) { // (N0 N1)
        if (itv[1] < 0) { // N1
          __m128d r0 = _mm_set1_pd(-1.0); // r0 == <-1, -1>
          __m128d r1 = _mm_shuffle_pd(xmmbounds,xmmbounds,1); // r1 == <-c, d>
          res.xmmbounds = _mm_div_pd(r0,r1);
        } else { // N0
          __m128d r0 = _mm_set_pd(-1.0,1.0); // r0 == <-1, 1>
          __m128d r1 = _mm_shuffle_pd(xmmbounds,xmmbounds,1); // r1 == <-c, d>
          __m128d r2 = _mm_and_pd(r1,interval::m128d_02mask); // r2 == <-c, 0>
          res.xmmbounds = _mm_div_pd(r0,r2);
        }
      } else { // Z
        res = interval::emptyset();
      }
    } else { // (M P0 P1)
      if (itv[0]>0) { // M : straddles_zero() (left bound negated)
            res = interval::universe();
      } else { // (P0 P1)
        if (itv[0] == 0.0) { // P0
          __m128d r0 = _mm_set_pd(1.0,-1.0); // r0 == <1, -1>
          __m128d r1 = _mm_xor_pd(xmmbounds,interval::lbsignmask); // r1 == <d, c>
          __m128d r2 = _mm_shuffle_pd(r1,r1,1); // r2 == <c, d>
          __m128d r3 = _mm_and_pd(r2,interval::m128d_20mask); // r3 == <0, d>
          res.xmmbounds = _mm_div_pd(r0,r3);
        } else { // P1
          __m128d r0 = _mm_set1_pd(-1.0); // r0 == <-1, -1>
          __m128d r1 = _mm_shuffle_pd(xmmbounds,xmmbounds,1); // r1 == <-c, d>
          res.xmmbounds = _mm_div_pd(r0,r1);
        }
      }
    }
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE_SSE();
    return res;
  }
#endif

  interval& interval::operator%=(const interval& I)
  {
	if (is_empty() || I.is_empty()) {
		*this = interval::emptyset();
		return *this;
	}

	GAOL_RND_ENTER_SSE();
	if ( right() < 0.0 ) { // [this] N1
		if ( I.right() < 0.0 ) { // [I] N1
			__m128d r = _mm_xor_pd(xmmbounds,interval::lbsignmask);
			xmmbounds = _mm_div_pd(_mm_shuffle_pd(r,r,1),I.xmmbounds); // N1 N1
		} else { // [I] P or Z
			if ( I.right() == 0.0 ) { // [I] N0 or Z
				if ( I.left() == 0.0 ) { // [I] Z
					*this = interval::emptyset(); // N1 Z
				} else {
					xmmbounds = _mm_move_sd(interval::m128_infinf,_mm_div_sd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),I.xmmbounds)); // N1 N0
				}
			} else { // [I] P or M
				if ( I.left() < 0.0 ) { // [I] M
					*this = interval::universe(); // N1 M
				} else { // [I] P0 or P1
					if ( I.left() == 0.0 ) { // [I] P0
						xmmbounds = _mm_unpacklo_pd(interval::m128_infinf,
						                           _mm_div_sd(_mm_unpackhi_pd(xmmbounds,xmmbounds),
						                                      _mm_unpackhi_pd(I.xmmbounds,I.xmmbounds))); // N1 P0
					} else { // [I] P1
						xmmbounds = _mm_div_pd(xmmbounds,_mm_xor_pd(I.xmmbounds,interval::lbsignmask)); // N1 P1
					}
				}
			}
		}
	} else { // [this] N0, Z, M, or P
		if ( right() == 0.0 ) { // [this] N0 or Z
			if ( left() == 0.0 ) { // [this] Z
				if ( I.right() < 0.0 || I.left() > 0.0 ) {
					xmmbounds = interval::m128_zero; // Z (N1 P1)
				} else {
					*this = interval::universe(); // Z (N0 Z M P0)
				}
			} else { // [this] N0
				if ( I.right() < 0.0 ) { // [I] N1
					__m128d r1 = _mm_xor_pd(xmmbounds,interval::lbsignmask);
					xmmbounds = _mm_move_sd(_mm_div_pd(_mm_shuffle_pd(r1,r1,1),I.xmmbounds),interval::m128_zero);	// N0 N1
				} else { // [I] P or Z
					if ( I.right() == 0.0 ) { // [I] N0 or Z
							*this = interval::universe(); // N0 Z
					} else { // [I] P or M
						if ( I.left() <= 0.0 ) { // [I] M
							*this = interval::universe(); // N0 P0
            } else { // [I] P1
							xmmbounds = _mm_move_sd(interval::m128_zero,_mm_div_pd(xmmbounds,_mm_xor_pd(I.xmmbounds,interval::lbsignmask))); // N0 P1
						}
					}
				}
			}
		} else { // [this] M, P0, or P1
			if ( left() < 0.0 ) { // [this] M
				if ( I.right() < 0.0 ) { // [I] N1
					__m128d r1 = _mm_xor_pd(xmmbounds,interval::lbrbsignmask);
					xmmbounds = _mm_div_pd(_mm_shuffle_pd(r1,r1,1),_mm_unpackhi_pd(I.xmmbounds,I.xmmbounds));// M N1
				} else { // [I] P or Z
					if ( I.right() == 0.0 ) { // [I] N0 or Z
						*this = interval::universe(); // M N0
					} else { // [I] P or M
						if ( I.left() <= 0.0 ) { // [I] M
							*this = interval::universe(); // M M
            } else { // [I] P1
								__m128d r1 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
								xmmbounds = _mm_div_pd(xmmbounds,_mm_unpacklo_pd(r1,r1)); // M P1
            }
					}
				}
			} else { // [this] P0 or P1
				if ( left() == 0.0 ) { // [this] P0
					if ( I.right() < 0.0 ) { // [I] N1
						__m128d r1 = _mm_xor_pd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),interval::lbsignmask);
						__m128d r2 = _mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1);
						xmmbounds = _mm_move_sd(interval::m128_zero,_mm_div_pd(r1,r2));// P0 N1
					} else { // [I] P or Z
						if ( I.right() == 0.0 ) { // [I] N0 or Z
							*this = interval::universe(); // P0 N0
						} else { // [I] P or M
							if ( I.left() <= 0.0 ) { // [I] M
								*this = interval::universe(); // P0 M
              } else { // [I] P1
									__m128d r1 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
									__m128d r2 = _mm_shuffle_pd(r1,r1,1);
									xmmbounds = _mm_move_sd(_mm_div_pd(xmmbounds,r2),interval::m128_zero);// P0 P1
							}
						}
					}
				} else { // [this] P1
					if ( I.right() < 0.0 ) { // [I] N1
						__m128d r1 = _mm_xor_pd(_mm_shuffle_pd(xmmbounds,xmmbounds,1),interval::lbrbsignmask);
						__m128d r2 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
						xmmbounds = _mm_div_pd(r1,_mm_shuffle_pd(r2,r2,1)); // P1 N1
					} else { // [I] P or Z
						if ( I.right() == 0.0 ) { // [I] N0 or Z
							if ( I.left() == 0.0 ) { // [I] Z
								*this = interval::emptyset(); // P1 Z
							} else {
								xmmbounds = _mm_unpacklo_pd(interval::m128_infinf,_mm_div_sd(xmmbounds,I.xmmbounds)); // P1 N0
							}
						} else { // [I] P or M
							if ( I.left() < 0.0 ) { // [I] M
								*this = interval::universe(); // P1 M
							} else { // [I] P0 or P1
								if ( I.left() == 0.0 ) { // [I] P0
									xmmbounds = _mm_move_sd(interval::m128_infinf,
									                        _mm_div_sd(xmmbounds,_mm_shuffle_pd(I.xmmbounds,I.xmmbounds,1))); // P1 P0
								} else { // [I] P1
									__m128d r1 = _mm_xor_pd(I.xmmbounds,interval::lbsignmask);
									xmmbounds = _mm_div_pd(xmmbounds,_mm_shuffle_pd(r1,r1,1));// P1 P1
								}
							}
						}
					}
				}
			}
		}
	}

	GAOL_RND_LEAVE_SSE();
	return *this;
  }

  std::ostream& operator<<(std::ostream& out, const __m128d& x)
	{
		interval::xmm2d tmp;
		_mm_store_pd(tmp,x);
		out << tmp[1] << " : " << tmp[0];
		return out;
	}
