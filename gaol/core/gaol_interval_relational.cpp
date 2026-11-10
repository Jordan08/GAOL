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
  \file   gaol_interval_relational.cpp
  \brief  The relational functions of the intervals: gaol_core

  The relational (reverse) functions: div_rel(), pow_rel(),
  pow_exponent_rel(), nth_root_rel(), sqrt_rel(), the _rel() of the
  trigonometric and of the hyperbolic functions, atan2_rel_x(),
  atan2_rel_y(), invabs_rel(), max_rel(), min_rel(), sign_rel() and
  floor_rel() (GAOL v5: a part of gaol_interval_core.cpp, and div_rel() of
  the source of each kind of intervals, see gaol_interval_internal.h).

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"

namespace gaol_core {

  // 2^52, from which on the doubles are integers, which gaol_port.h declared
  // for the code using GAOL too (GAOL v5)
  const double two_power_52 = 4503599627370496.0;

  /*
    div_rel(), mulRev of IEEE 1788-2015, after the check of the rounding
    direction: the hull of the x of I with an x y in K for a y of J, from the
    quotients of the bounds of K by those of J, whichever the signs. The FPU
    and the NEON intervals compute it so, and the SSE2 intervals where a
    quotient of their vectors could overflow (see div_rel() below). The
    quotients are those of quotient_sat():
    one that overflows is the infinity it gives, which is intersected with a
    finite I or joined to an infinite bound of the hull, where the division
    raised the overflow exception for a finite result, as in div_rel([0, 1/2],
    [2^-1074, 2^-1022], [0, 1]) (GAOL v5, point 79 of TODO.md). The code is
    that of the FPU and the NEON intervals, which took the stored bounds of
    their operands, the opposite of the lower bound being -K.left().
  */
  static interval div_rel_upward(const interval &K, const interval &J, const interval &I)
  {
    if (K.is_empty() || J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }

    if (K.right() < 0.0) { // [this] N1
      if (J.right() < 0.0) { // [J] N1
	// I & [K.r/J.l, K.l/J.r]
	interval tmp(-(quotient_sat(K.right(), -J.left())),quotient_sat(-K.left(), -J.right()));
	return I & tmp; // N1 N1
      } else { // [J] P or Z
	if (J.right() == 0.0) { // [J] N0 or Z
	  if (J.left() == 0.0) { // [J] Z
	    return interval::emptyset(); // N1 Z
	  }
	  // I & [K.r/J.l,+oo]
	  interval tmp(-(quotient_sat(K.right(), -J.left())),GAOL_INFINITY);
	  return I & tmp; // N1 N0
	} else { // [I] P or M
	  if (J.left() < 0.0) { // [J] M
	    interval tmp(-GAOL_INFINITY,quotient_sat(K.right(), J.right()));
	    // The lower bound rounded downward, the rounding direction being
	    // upward (GAOL v5: K.right()/J.left() was rounded upward, and the
	    // result left out the doubles below it that belong to the hull)
	    interval tmp2(-(quotient_sat(K.right(), -J.left())),GAOL_INFINITY);
	    return (I&tmp) | (I&tmp2); // N1 M
	  } else { // [J] P0 or P1
	    if (J.left() == 0.0) { // [J] P0
	      interval tmp2 = I & interval(-GAOL_INFINITY,quotient_sat(K.right(), J.right())); // N1 P0
	      return tmp2; // N1 P0
	    } else { // [J] P1
	      interval tmp(-(quotient_sat(-K.left(), J.left())),quotient_sat(K.right(), J.right()));
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
	    interval tmp2 = I & interval(0.0,quotient_sat(K.left(), J.right())); // N0 N1
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
		  interval tmp(-(quotient_sat(-K.left(), J.left())),0.0);
		  return I & tmp; // N0 P1
		}
	      }
	    }
	  }
	}
      } else { // [this] M, P0, or P1
	if (K.left() < 0.0) { // [this] M
	  if (J.right() < 0.0) { // [J] N1
	    interval tmp(-(quotient_sat(K.right(), -J.right())),quotient_sat(-K.left(), -J.right()));
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
		  interval tmp(-(quotient_sat(-K.left(), J.left())),quotient_sat(K.right(), J.left()));
		  return I & tmp; // M P1
		}
	      }
	    }
	  }
	} else { // [this] P0 or P1
	  if (K.left() == 0.0) { // [this] P0
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(quotient_sat(K.right(), -J.right())),0.0);
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
		    interval tmp2 = I & interval(0.0,quotient_sat(K.right(), J.left())); // P0 P1
		    return tmp2;
		  }
		}
	      }
	    }
	  } else { // [this] P1
	    if (J.right() < 0.0) { // [J] N1
	      interval tmp(-(quotient_sat(K.right(), -J.right())),quotient_sat(-K.left(), -J.left()));
	      return I & tmp; // P1 N1
	    } else { // [J] P or Z
	      if (J.right() == 0.0) { // [J] N0 or Z
		if (J.left() == 0.0) { // [J] Z
		  return interval::emptyset(); // P1 Z
		}
		interval tmp2 = I & interval(-GAOL_INFINITY,quotient_sat(-K.left(), -J.left())); // P1 N0
		return tmp2;
	      } else { // [J] P or M
		if (J.left() < 0.0) { // [J] M
		  interval tmp(-GAOL_INFINITY,quotient_sat(-K.left(), -J.left()));
		  interval tmp2 = interval(-(quotient_sat(-K.left(), J.right())),GAOL_INFINITY);
		  return (I&tmp) | (I&tmp2); // P1 M
		} else { // [J] P0 or P1
		  if (J.left() == 0.0) { // [J] P0
		    interval tmp(-(quotient_sat(-K.left(), J.right())),GAOL_INFINITY);
		    return I & tmp; // P1 P0
		  } else { // [J] P1
		    interval tmp(-(quotient_sat(-K.left(), J.right())),quotient_sat(K.right(), J.left()));
		    return I & tmp; // P1 P1
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  /*
    The relational division of the SSE2 intervals, on their registers, and
    of the NEON and the FPU ones, from div_rel_upward() (GAOL v5: each was in
    the source of its kind of intervals, gaol_interval_sse_impl.h,
    gaol_interval_neon_impl.h and gaol_interval_fpu_impl.h)
  */
#if GAOL_USING_SSE2_INSTRUCTIONS
interval div_rel(const interval &K, const interval &J, const interval &I)
{
  // One check, before the bounds are compared (GAOL v5, see gaol/gaol_fpu.h):
  // the branches made their own checks, after comparing the bounds with 0,
  // which the denormals-are-zero mode made wrong for the subnormal ones, and
  // with GAOL_PRESERVE_ROUNDING the intersections with I compared bounds after
  // the mode was restored. The division and its intersection with I are made
  // by the lambda, whose returns come back before GAOL_RND_LEAVE_SSE(). The
  // bounds of K and J are read by get_xmminterval(): div_rel() is no friend
  // of interval (GAOL v5)
  GAOL_RND_ENTER_SSE();
  interval res = [&]() -> interval {
  // The emptiness tests of the SSE2 intervals are not worth writing with SSE2
  // intrinsics, as the FIXMEs of GAOL here and in operator*=, operator/= and
  // operator%= (gaol_interval_sse_impl.h) asked: testing the bounds of both
  // operands at once, with _mm_cmpnge_pd and _mm_movemask_pd, gives the same
  // results as is_empty() || I.is_empty() but no faster, 1.3 to 1.6 ns rather
  // than 1.0 to 1.2 ns (GCC 9.4, x86-64)
  if (K.is_empty() || J.is_empty() || I.is_empty()) {
	return interval::emptyset();
  }

  // A quotient of the vectors that may overflow, though the result is finite,
  // is computed on the bounds by div_rel_upward(), which saturates it (GAOL
  // v5, point 79 of TODO.md): a finite bound of K of magnitude 2^500 or more,
  // or a bound of J that is not 0 and is below 2^-500 in magnitude (the
  // quotients are then below 2^1000 otherwise; those with a zero bound of J
  // are not formed, and an infinite bound gives an infinity or a zero with no
  // exception). The magnitudes are compared in the vectors, the NaN of an
  // empty operand having been left above
  {
	const __m128d sign = _mm_set1_pd(-0.0), infinity = _mm_set1_pd(GAOL_INFINITY);
	const __m128d k = _mm_andnot_pd(sign, K.get_xmminterval()), j = _mm_andnot_pd(sign, J.get_xmminterval());
	const __m128d big = _mm_and_pd(_mm_cmpge_pd(k, _mm_set1_pd(two_power_500)), _mm_cmplt_pd(k, infinity));
	const __m128d tiny = _mm_and_pd(_mm_cmplt_pd(j, _mm_set1_pd(two_power_minus_500)), _mm_cmpneq_pd(j, _mm_setzero_pd()));
	if (_mm_movemask_pd(_mm_or_pd(big, tiny)) != 0) {
	  return div_rel_upward(K, J, I);
	}
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
      __m128d r = _mm_xor_pd(K.get_xmminterval(),interval::lbsignmask);
      __m128d r2 = _mm_div_pd(_mm_shuffle_pd(r,r,1),J.get_xmminterval());
      return I & interval(r2, gaol_detail::unchecked_bounds); // N1 N1
    } else { // [J] P or Z
      if ( J.right() == 0.0 ) { // [J] N0 or Z
        if ( J.left() == 0.0 ) { // [J] Z
          return interval::emptyset(); // N1 Z
        } else {
          __m128d r = _mm_move_sd(interval::m128_infinf,_mm_div_sd(_mm_shuffle_pd(K.get_xmminterval(),K.get_xmminterval(),1),J.get_xmminterval()));
          return I & interval(r, gaol_detail::unchecked_bounds); // N1 N0
        }
      } else { // [J] P or M
        if ( J.left() < 0.0 ) { // [J] M
          // The two quotients, [-oo, K.right()/J.right()] and [K.right()/J.left(),
          // +oo], have K.right() for numerator and no divisor is 0 (J.left() < 0
          // < J.right()): one division of the vectors gives both bounds (GAOL
          // v5, point PP of TODO.md; the scalar code made two intervals). The
          // bounds are stored as (-lower, upper): the low half of the quotient,
          // K.right()/(-J.left()) rounded upward, is minus the lower bound
          // K.right()/J.left() rounded downward (GAOL v5: it was rounded upward,
          // and the result left out the doubles below it that belong to the
          // hull), the rounding direction being upward; its high half is the
          // upper bound K.right()/J.right() rounded upward. No half is thrown
          // away, and neither raises the invalid-operation or the
          // division-by-zero exception: the numerator is finite (K is not
          // empty, and [-oo, -oo] is) and not 0, and the divisors are not 0. The
          // overflow of the finite quotients is left to div_rel_upward() above
          __m128d q = _mm_div_pd(_mm_unpackhi_pd(K.get_xmminterval(),K.get_xmminterval()),J.get_xmminterval());
          __m128d r = _mm_move_sd(q,interval::m128_infinf);   // [-oo, K.right()/J.right()]
          __m128d r2 = _mm_move_sd(interval::m128_infinf,q);  // [K.right()/J.left(), +oo]
          return (I & interval(r, gaol_detail::unchecked_bounds)) | (I & interval(r2, gaol_detail::unchecked_bounds)); // N1 M
        } else { // [J] P0 or P1
          if ( J.left() == 0.0 ) { // [I] P0
            __m128d r = _mm_unpacklo_pd(interval::m128_infinf,
                                        _mm_div_sd(_mm_unpackhi_pd(K.get_xmminterval(),K.get_xmminterval()),
                                                   _mm_unpackhi_pd(J.get_xmminterval(),J.get_xmminterval()))); // N1 P0
            return I & interval(r, gaol_detail::unchecked_bounds);
          } else { // [J] P1
            __m128d r = _mm_div_pd(K.get_xmminterval(),_mm_xor_pd(J.get_xmminterval(),interval::lbsignmask)); // N1 P1
            return I & interval(r, gaol_detail::unchecked_bounds);
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
			__m128d r1 = _mm_xor_pd(K.get_xmminterval(),interval::lbsignmask);
			__m128d r2 = _mm_move_sd(_mm_div_pd(_mm_shuffle_pd(r1,r1,1),J.get_xmminterval()),interval::m128_zero);	// N0 N1
			return I & interval(r2, gaol_detail::unchecked_bounds);
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
          __m128d r = _mm_move_sd(interval::m128_zero,_mm_div_pd(K.get_xmminterval(),_mm_xor_pd(J.get_xmminterval(),interval::lbsignmask)));
          return I & interval(r, gaol_detail::unchecked_bounds); // N0 P1
			  }
			}
		  }
		}
	  }
	} else { // [K] M, P0, or P1
	  if ( K.left() < 0.0 ) { // [K] M
		if ( J.right() < 0.0 ) { // [J] N1
		  __m128d r1 = _mm_xor_pd(K.get_xmminterval(),interval::lbrbsignmask);
		  __m128d r2 = _mm_div_pd(_mm_shuffle_pd(r1,r1,1),_mm_unpackhi_pd(J.get_xmminterval(),J.get_xmminterval()));
      return I & interval(r2, gaol_detail::unchecked_bounds); // M N1
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
          __m128d r1 = _mm_xor_pd(J.get_xmminterval(),interval::lbsignmask);
          __m128d r2 = _mm_div_pd(K.get_xmminterval(),_mm_unpacklo_pd(r1,r1));
          return I & interval(r2, gaol_detail::unchecked_bounds); // M P1
			  }
			}
		  }
		}
	  } else { // [K] P0 or P1
		if ( K.left() == 0.0 ) { // [K] P0
		  if ( J.right() < 0.0 ) { // [J] N1
        __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(K.get_xmminterval(),K.get_xmminterval(),1),interval::lbsignmask);
        __m128d r2 = _mm_shuffle_pd(J.get_xmminterval(),J.get_xmminterval(),1);
        __m128d r3 = _mm_move_sd(interval::m128_zero,_mm_div_pd(r1,r2));
        return I & interval(r3, gaol_detail::unchecked_bounds); // P0 N1
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
				  __m128d r1 = _mm_xor_pd(J.get_xmminterval(),interval::lbsignmask);
				  __m128d r2 = _mm_shuffle_pd(r1,r1,1);
				  __m128d r3 = _mm_move_sd(_mm_div_pd(K.get_xmminterval(),r2),interval::m128_zero);// P0 P1
          return I & interval(r3, gaol_detail::unchecked_bounds);
				}
			  }
			}
		  }
		} else { // [K] P1
		  if ( J.right() < 0.0 ) { // [J] N1
        __m128d r1 = _mm_xor_pd(_mm_shuffle_pd(K.get_xmminterval(),K.get_xmminterval(),1),interval::lbrbsignmask);
        __m128d r2 = _mm_xor_pd(J.get_xmminterval(),interval::lbsignmask);
        __m128d r3 = _mm_div_pd(r1,_mm_shuffle_pd(r2,r2,1)); 
        return I & interval(r3, gaol_detail::unchecked_bounds); // P1 N1
      } else { // [J] P or Z
        if ( J.right() == 0.0 ) { // [J] N0 or Z
			  if ( J.left() == 0.0 ) { // [J] Z
				return interval::emptyset(); // P1 Z
			  } else {
          __m128d r = _mm_unpacklo_pd(interval::m128_infinf,_mm_div_sd(K.get_xmminterval(),J.get_xmminterval()));
          return I & interval(r, gaol_detail::unchecked_bounds); // P1 N0
			  }
			} else { // [J] P or M
			  if ( J.left() < 0.0 ) { // [J] M
          __m128d r = _mm_unpacklo_pd(interval::m128_infinf,_mm_div_sd(K.get_xmminterval(),J.get_xmminterval())); // P1 N0
				  __m128d r2 = _mm_move_sd(interval::m128_infinf,
                        _mm_div_sd(K.get_xmminterval(),
												_mm_shuffle_pd(J.get_xmminterval(),J.get_xmminterval(),1))); // P1 P0
          return (I & interval(r, gaol_detail::unchecked_bounds)) | (I & interval(r2, gaol_detail::unchecked_bounds)); // P1 M
			  } else { // [J] P0 or P1
				if ( J.left() == 0.0 ) { // [J] P0
				  __m128d r = _mm_move_sd(interval::m128_infinf,
                        _mm_div_sd(K.get_xmminterval(),
												_mm_shuffle_pd(J.get_xmminterval(),J.get_xmminterval(),1))); // P1 P0
          return I & interval(r, gaol_detail::unchecked_bounds);
				} else { // [J] P1
				  __m128d r1 = _mm_xor_pd(J.get_xmminterval(),interval::lbsignmask);
				  __m128d r2 = _mm_div_pd(K.get_xmminterval(),_mm_shuffle_pd(r1,r1,1));
          return I & interval(r2, gaol_detail::unchecked_bounds); // P1 P1
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
#else
  interval div_rel(const interval &K, const interval &J, const interval &I)
  {
    // One check, before the bounds are compared, as in the SSE2 build (GAOL
    // v5, see gaol/gaol_fpu.h). The quotients are those of div_rel_upward()
    // above, which raise no overflow exception for a finite result (GAOL v5,
    // point 79 of TODO.md)
    GAOL_RND_ENTER();
    interval res = div_rel_upward(K, J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }
#endif // GAOL_USING_SSE2_INSTRUCTIONS

  /*
    The reverse functions of pow (GAOL v5): pow_rel(K, J, I), the hull of the
    x of I with an x^y in K for a y of J, and pow_exponent_rel(K, H, I), the
    hull of the y of I with an x^y in K for an x of H, which are powRev1(J, K,
    I) and powRev2(H, K, I) of IEEE 1788-2015 (10.5.4, Table 10.1).

    They reverse the pow of the standard (Table 9.1), that of
    gaol_ieee1788::pow and pow_standard(): x^y = e^(y log x) for x > 0, 0^y = 0
    for y > 0, and no value elsewhere. gaol::pow takes the integer power for a
    degenerate integer exponent [n], which has negative bases too: an
    interval function that is not monotone in its exponent
    (gaol_pow_hybrid()), and has no set of solutions to reverse; the reverse
    of x^n is nth_root_rel(K, n, I).

    The solutions are those of x = 0 (0^y = 0 in K for y > 0) and those of
    x > 0, where x^y is in K, that is in K & ]0, +oo], exactly when y log(x) is
    in log(K & ]0, +oo]): the reverse of a product, of y and log(x). Where 1
    is in K and the other argument holds the value that makes the product 0
    (y = 0, x^0 = 1; x = 1, 1^y = 1), every x > 0 of I, or every y of I, is a
    solution. Otherwise, each sign of the other argument (y > 0 and y < 0;
    x > 1 and x < 1) gives a piece over which x^y is monotone in both x and
    y: the solutions of each piece are an interval, whose ends are reached
    at the bounds of the other argument and of K, or are limits there, and
    pieces of each sign are intersected with I, then joined.

    The ends of the pieces are c^(1/y) for pow_rel and log(c)/log(x) for
    pow_exponent_rel, c a bound of K and x or y a bound of the other argument;
    1 for pow_rel and 0 for pow_exponent_rel where c is 1 (exactly reached);
    0 and the infinities; and 1 (x^y -> 1 for y -> +oo or -oo) for pow_rel and
    0 (log(c)/log(x) -> 0 for x -> 0 or +oo) for pow_exponent_rel, as limits:
    a piece that ends at such a limit does not hold it, and a bound of I equal
    to it leaves that piece out (pow_rel([2, 3], [1, +oo], [0, 1]) is empty:
    x^y >= 2 for a y >= 1 asks x > 1).

    c^(1/y) and log(c)/log(x) are irrational but where they are doubles, and
    their bounds are proved with the power they reverse: d <= c^(1/y) exactly
    when d^y <= c (y > 0), which holds exactly when d^y rounded upward,
    CORE-MATH's pow in the upward rounding GAOL computes in, is at most c, c
    being a double; d^y >= c exactly when d^y rounded downward is at least c,
    which pow_lo() gives, taking the power itself where it is a double
    (pow_is_double()). y < 0, which makes d^y decrease, swaps the two, and
    log(c)/log(x) is bounded the same way by the powers x^e of a double e. The
    lower bound is the largest double proved to be below the end, the upper
    bound the smallest one proved to be above it: they are looked for by
    steps that double then by bisection, from an approximation, on the
    doubles taken in their order (double_key()), each step costing one pow.
    The bounds are the tightest: a power rounded upward is CORE-MATH's,
    correctly rounded, and pow_is_double() tells every power that is a double,
    which only makes the bounds tighter, its answer that a power is no double
    never being used to leave a point out.

    A piece is left out only where it is proved to miss I: its lower end
    proved above the upper bound u of I, or its upper end proved below the
    lower bound. Where the bound of the end is u itself, c^(1/y) > u is
    proved by u^y < c (y > 0), u^y rounded upward being below c, which holds
    unless u^y is within one double below c: there, telling u^y < c from
    u^y = c asks whether u^y is a double, which only pow_is_double() tells,
    and the point u is kept rather than left out on that answer:
    pow_rel([1 + 2^-52], [0.5], [0, 1 + 2^-51]) is [1 + 2^-51] rather than
    empty, (1 + 2^-52)^2 being 1 + 2^-51 + 2^-104. The result is then the
    tightest one for I widened by one double, which accurate (12.10.1)
    allows.
  */

  /*
    The doubles in their order, as unsigned integers: the key of d >= 0 is
    2^63 plus the bits of d, and that of d < 0 is 2^63 minus the bits of -d,
    so that the keys of -oo, -DBL_MAX, ..., -0 = +0, ..., DBL_MAX, +oo
    increase with the doubles, consecutive doubles having consecutive keys,
    from 2^52 to 2^64 - 2^52. The searches below step and bisect on the keys,
    exactly, without nextafter() nor arithmetic on the doubles.
  */
  static inline std::uint64_t double_key(double d)
  {
    const double a = std::fabs(d);
    std::uint64_t bits;
    std::memcpy(&bits, &a, sizeof bits);
    const std::uint64_t half = static_cast<std::uint64_t>(1) << 63;
    return (d < 0.0) ? half - bits : half + bits;
  }

  static inline double key_double(std::uint64_t k)
  {
    const std::uint64_t half = static_cast<std::uint64_t>(1) << 63;
    const std::uint64_t bits = (k >= half) ? k - half : half - k;
    double a;
    std::memcpy(&a, &bits, sizeof a);
    return (k >= half) ? a : -a;
  }

  /*
    The largest key of [a, b] at which holds() is true, holds() being true at
    a, where it is not called, and false from some key on: from the key k of
    an approximation, by steps that double, up while it holds and down while
    it does not, then by bisection between the last key where it holds and the
    first one where it does not. At most about 2 x 64 calls of holds(), and two
    from a k next to the result.
  */
  template <class Holds>
  static std::uint64_t last_key_holding(const Holds& holds, std::uint64_t a, std::uint64_t b, std::uint64_t k)
  {
    const std::uint64_t largest_step = static_cast<std::uint64_t>(1) << 62;
    k = (k < a) ? a : ((k > b) ? b : k);
    // holds(yes), and !holds(no) once failed is true
    std::uint64_t yes = a, no = b;
    bool failed = false;
    if (k != a) {
      if (holds(k)) {
        yes = k;
      } else {
        no = k;
        failed = true;
      }
    }
    if (!failed) {
      for (std::uint64_t step = 1;; step = (step < largest_step) ? 2*step : step) {
        if (b - yes <= step) {
          if (yes == b || holds(b)) {
            return b;
          }
          no = b;
          break;
        }
        if (holds(yes + step)) {
          yes += step;
        } else {
          no = yes + step;
          break;
        }
      }
    } else {
      for (std::uint64_t step = 1; no - a > step; step = (step < largest_step) ? 2*step : step) {
        if (holds(no - step)) {
          yes = no - step;
          break;
        }
        no -= step;
      }
    }
    while (no - yes > 1) {
      const std::uint64_t m = yes + (no - yes)/2;
      if (holds(m)) {
        yes = m;
      } else {
        no = m;
      }
    }
    return yes;
  }

  // The smallest key of [a, b] at which holds() is true, holds() being true at
  // b, where it is not called, and false up to some key: the search above on
  // the keys taken backward
  template <class Holds>
  static std::uint64_t first_key_holding(const Holds& holds, std::uint64_t a, std::uint64_t b, std::uint64_t k)
  {
    // static, so that the lambda needs no capture of it: Clang warned of the
    // capture as not required (-Wunused-lambda-capture), and Visual C++
    // refused the constant uncaptured (C3493)
    static constexpr std::uint64_t last = ~static_cast<std::uint64_t>(0);
    return last - last_key_holding([&holds](std::uint64_t j) { return holds(last - j); }, last - b, last - a, last - k);
  }

  /*
    An approximation of c^(1/y), c > 0 finite and other than 1 and y finite
    and other than 0, which the bounds below are looked for from: CORE-MATH's
    pow with the exponent 1/y rounded, |log(c^(1/y))| 2^-53 away relatively,
    after a step of Newton's method on r^y = c, r - r (r^y - c)/(y r^y),
    which brings it within about 2^-52/|y| relatively. 1/y is +oo for a tiny
    y, and -DBL_MAX for a tiny negative one (quotient_sat()): the exponents of
    extreme magnitude pow_rounded_up() takes apart, 1/y included. Rounding
    upward
  */
  static double root_of_power_near(double c, double y)
  {
    const double largest = std::numeric_limits<double>::max();
    // 1/y overflows for a y next to 2^-1074, to +oo (-DBL_MAX for y < 0): the
    // quotient_sat() that raises no exception (GAOL v5, point 79 of TODO.md)
    const double r = pow_rounded_up<true>(c, quotient_sat(1.0, y));
    // The step is for |y| up to 1024 and c up to 2^1000: r is then within
    // about 2^-43 relatively of the root, so that r^y is within 2^-33 of c, and
    // r^y cannot overflow. For a greater |y| the rounded 1/y moves r by less
    // than 745/|y| 2^-53 relatively, below one double, and a step would take
    // r^y, which overflows for |y| next to 2^52 (GAOL v5, point 79 of
    // TODO.md: pow_rel([3], [2^512], I) raised the overflow exception)
    if (r > 0.0 && r <= largest && std::fabs(y) <= 1024.0 && c <= huge_root_argument) {
      const double p = pow_rounded_up<true>(r, y);
      if (p != c && p >= std::numeric_limits<double>::min() && p <= largest) {
        // r (p - c)/(y p), as (p - c)/p then /y, whose quotients saturate,
        // where y p and the quotient of (p - c) by it overflowed (GAOL v5,
        // point 79 of TODO.md), and which is left out where it is 1 or more
        // in magnitude, the power being then no approximation of c
        const double t = quotient_sat(quotient_sat(p - c, p), y);
        if (std::fabs(t) < 1.0) {
          const double s = r - r*t;
          if (s > 0.0 && s <= largest) {
            return s;
          }
        }
      }
    }
    return r;
  }

  /*
    The tightest bounds of c^(1/y), the x > 0 with x^y = c, for c > 0 finite
    and other than 1, and y finite and other than 0: d <= c^(1/y) is proved
    by d^y <= c for y > 0 and by d^y >= c for y < 0, and d >= c^(1/y) the
    other way round. 0 is below c^(1/y), where pow() is not called (CORE-MATH
    gives +oo for 0^y, y < 0, with the divide-by-zero exception), and +oo
    above it: [DBL_MAX, +oo] for a c^(1/y) beyond the doubles, and
    [0, 2^-1074] for one below them. Rounding upward
  */
  static double root_of_power_dn(double c, double y, double near)
  {
    const auto below = [c, y](std::uint64_t k) {
      const double d = key_double(k);
      return (y > 0.0) ? pow_hi<true>(d, y) <= c : pow_lo<true>(d, y) >= c;
    };
    return key_double(last_key_holding(below, double_key(0.0), double_key(std::numeric_limits<double>::max()),
                                       double_key(near)));
  }

  static double root_of_power_up(double c, double y, double near)
  {
    const auto above = [c, y](std::uint64_t k) {
      const double d = key_double(k);
      return (y > 0.0) ? pow_lo<true>(d, y) >= c : pow_hi<true>(d, y) <= c;
    };
    return key_double(first_key_holding(above, double_key(double_denorm_min),
                                        double_key(GAOL_INFINITY), double_key(near)));
  }

  /*
    The tightest bounds of log(c)/log(x), the y with x^y = c, for c > 0
    finite and other than 1, and x > 0 finite and other than 1: e <=
    log(c)/log(x) is proved by x^e <= c for x > 1 and by x^e >= c for x < 1,
    and e >= log(c)/log(x) the other way round. The value is finite, below
    2^63 in magnitude (|log(c)| < 745 and |log(x)| > 2^-53), and the
    approximation log(c)/log(x) of CORE-MATH's logarithms is within a few
    doubles of it. Rounding upward
  */
  static double log_ratio_near(double c, double x)
  {
    return upward::log_up(c)/upward::log_up(x);
  }

  static double log_ratio_dn(double c, double x)
  {
    const auto below = [c, x](std::uint64_t k) {
      const double e = key_double(k);
      return (x > 1.0) ? pow_hi<true>(x, e) <= c : pow_lo<true>(x, e) >= c;
    };
    return key_double(last_key_holding(below, double_key(-GAOL_INFINITY),
                                       double_key(std::numeric_limits<double>::max()),
                                       double_key(log_ratio_near(c, x))));
  }

  static double log_ratio_up(double c, double x)
  {
    const auto above = [c, x](std::uint64_t k) {
      const double e = key_double(k);
      return (x > 1.0) ? pow_lo<true>(x, e) >= c : pow_hi<true>(x, e) <= c;
    };
    return key_double(first_key_holding(above, double_key(-std::numeric_limits<double>::max()),
                                        double_key(GAOL_INFINITY), double_key(log_ratio_near(c, x))));
  }

  /*
    Whether c^(1/y) > d (above) or c^(1/y) < d (below) is proved, for a
    double d > 0: d^y < c, or d^y > c, for y > 0, the other way round for
    y < 0. A power rounded upward below c, or one rounded downward above it,
    proves it without telling whether a power is a double. Rounding upward
  */
  static bool root_of_power_beyond(double c, double y, double d, bool above)
  {
    return (above == (y > 0.0)) ? pow_hi<true>(d, y) < c : pow_lo<true>(d, y) > c;
  }

  // The same for log(c)/log(x) and a double e: x^e < c, or x^e > c, for x > 1
  static bool log_ratio_beyond(double c, double x, double e, bool above)
  {
    return (above == (x > 1.0)) ? pow_hi<true>(x, e) < c : pow_lo<true>(x, e) > c;
  }

  /*
    An end of a piece of the solutions: for a lower end, a double at most the
    end, and for an upper end, a double at least it; whether the piece only
    comes as close to the end as one likes, without holding it (the limits
    0, 1 and the infinities); and the end, c^(1/w) or log(c)/log(w), where
    its bound is computed
  */
  enum pow_rel_value { pow_rel_exact, pow_rel_root, pow_rel_log_ratio };

  struct pow_rel_end {
    double bound;
    bool limit;
    pow_rel_value value;
    double c, w;
  };

  static inline pow_rel_end reached(double d)
  {
    pow_rel_end e = { d, false, pow_rel_exact, 0.0, 0.0 };
    return e;
  }

  static inline pow_rel_end approached(double d)
  {
    pow_rel_end e = { d, true, pow_rel_exact, 0.0, 0.0 };
    return e;
  }

  // c^(1/y), below it for a lower end, above it for an upper one
  static pow_rel_end root_end(double c, double y, bool lower)
  {
    const double near = root_of_power_near(c, y);
    pow_rel_end e = { lower ? root_of_power_dn(c, y, near) : root_of_power_up(c, y, near), false, pow_rel_root, c, y };
    return e;
  }

  // log(c)/log(x), below it for a lower end, above it for an upper one
  static pow_rel_end log_ratio_end(double c, double x, bool lower)
  {
    pow_rel_end e = { lower ? log_ratio_dn(c, x) : log_ratio_up(c, x), false, pow_rel_log_ratio, c, x };
    return e;
  }

  // Whether the end is proved to be above d (above) or below it
  static bool pow_rel_beyond(const pow_rel_end& e, double d, bool above)
  {
    switch (e.value) {
    case pow_rel_root:
      return root_of_power_beyond(e.c, e.w, d, above);
    case pow_rel_log_ratio:
      return log_ratio_beyond(e.c, e.w, d, above);
    default:
      return false;
    }
  }

  /*
    Adds to the hull [lo, hi] the part of [l, u] in the piece from the lower
    end to the upper end, unless the piece is proved to miss [l, u]: an end
    beyond l or u, or a limit at it, or a computed end whose bound is l or u
    and which is proved to be beyond it all the same. Where the power of l or
    u is within one double of the bound of K, that proof would need to know
    whether the power is a double, and the point is kept
  */
  static void add_pow_rel_piece(const pow_rel_end& lower, const pow_rel_end& upper, double l, double u,
                                double& lo, double& hi)
  {
    if (lower.bound > u || (lower.limit && lower.bound >= u) || (lower.bound == u && pow_rel_beyond(lower, u, true))
        || upper.bound < l || (upper.limit && upper.bound <= l) || (upper.bound == l && pow_rel_beyond(upper, l, false))) {
      return;
    }
    lo = minimum(lo, maximum(lower.bound, l));
    hi = maximum(hi, minimum(upper.bound, u));
  }

  /*
    pow_rel() after its check, made before the bounds are compared (GAOL v5,
    see gaol/gaol_fpu.h)
  */
  static interval pow_rel_upward(const interval& K, const interval& J, const interval& I)
  {
    // A power or an exponent [+oo] or [-oo] holds no real number: no y and no
    // x (GAOL v5: pow_rel([2], [+oo], I) was [1, 1 + 2^-52] and
    // pow_rel([+oo], [2], I) [DBL_MAX, +oo], limits of x^y). The x to find, I,
    // needs no test: interval(lo, hi) of the result is empty for lo = +oo or
    // hi = -oo
    if (K.is_empty() || J.is_empty() || I.is_empty() || holds_no_real(K) || holds_no_real(J)) {
      return interval::emptyset();
    }
    const double xu = I.right(), cu = K.right();
    if (xu < 0.0 || cu < 0.0) {
      return interval::emptyset(); // No x >= 0, or no power
    }
    const double xl = (I.left() > 0.0) ? I.left() : 0.0, cl = K.left(), yl = J.left(), yu = J.right();
    double lo = GAOL_INFINITY, hi = -GAOL_INFINITY;
    if (xl == 0.0 && cl <= 0.0 && yu > 0.0) {
      lo = hi = 0.0; // 0^y = 0 for y > 0
    }
    if (xu > 0.0 && cu > 0.0) {
      if (cl <= 1.0 && cu >= 1.0 && yl <= 0.0 && yu >= 0.0) {
        // x^0 = 1 for every x > 0
        lo = minimum(lo, xl);
        hi = xu;
      } else {
        const pow_rel_end zero = approached(0.0), one = approached(1.0), plus_infinity = approached(GAOL_INFINITY);
        pow_rel_end lower, upper;
        if (yu > 0.0) {
          /*
            y in [p1, p2], p1 > 0 or p1 = 0, which is then left out (x^0 = 1
            is not in K): x in [cl^(1/y), cu^(1/y)], whose lower end
            decreases with y for cl > 1 and increases for cl < 1, whose upper
            end does the same with cu, and which tends to 1 for y -> +oo and
            to 0 or +oo for y -> 0
          */
          const double p1 = (yl > 0.0) ? yl : 0.0, p2 = yu;
          if (cl <= 0.0) {
            lower = zero;
          } else if (cl == 1.0) {
            lower = reached(1.0);
          } else if (cl > 1.0) {
            lower = (p2 == GAOL_INFINITY) ? one : root_end(cl, p2, true);
          } else {
            lower = (p1 == 0.0) ? zero : root_end(cl, p1, true);
          }
          if (cu == GAOL_INFINITY) {
            upper = plus_infinity;
          } else if (cu == 1.0) {
            upper = reached(1.0);
          } else if (cu > 1.0) {
            upper = (p1 == 0.0) ? plus_infinity : root_end(cu, p1, false);
          } else {
            upper = (p2 == GAOL_INFINITY) ? one : root_end(cu, p2, false);
          }
          add_pow_rel_piece(lower, upper, xl, xu, lo, hi);
        }
        if (yl < 0.0) {
          /*
            y in [n1, n2], n2 < 0 or n2 = 0, which is then left out: x^y
            decreases with x, and x in [cu^(1/y), cl^(1/y)], whose lower end
            decreases with y for cu > 1 and increases for cu < 1, whose upper
            end does the same with cl, and which tends to 1 for y -> -oo and
            to 0 or +oo for y -> 0
          */
          const double n1 = yl, n2 = (yu < 0.0) ? yu : 0.0;
          if (cu == GAOL_INFINITY) {
            lower = zero;
          } else if (cu == 1.0) {
            lower = reached(1.0);
          } else if (cu > 1.0) {
            lower = (n2 == 0.0) ? zero : root_end(cu, n2, true);
          } else {
            lower = (n1 == -GAOL_INFINITY) ? one : root_end(cu, n1, true);
          }
          if (cl <= 0.0) {
            upper = plus_infinity;
          } else if (cl == 1.0) {
            upper = reached(1.0);
          } else if (cl > 1.0) {
            upper = (n1 == -GAOL_INFINITY) ? one : root_end(cl, n1, false);
          } else {
            upper = (n2 == 0.0) ? plus_infinity : root_end(cl, n2, false);
          }
          add_pow_rel_piece(lower, upper, xl, xu, lo, hi);
        }
      }
    }
    // Empty where nothing was added, lo being +oo and hi -oo
    return interval(lo, hi);
  }

  interval pow_rel(const interval& K, const interval& J, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = pow_rel_upward(K, J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    pow_exponent_rel() after its check, made before the bounds are compared
  */
  static interval pow_exponent_rel_upward(const interval& K, const interval& H, const interval& I)
  {
    // A power or a base [+oo] or [-oo] holds no real number, as in pow_rel_upward()
    if (K.is_empty() || H.is_empty() || I.is_empty() || holds_no_real(K) || holds_no_real(H)) {
      return interval::emptyset();
    }
    const double xu = H.right(), cu = K.right();
    if (xu < 0.0 || cu < 0.0) {
      return interval::emptyset(); // No base x >= 0, or no power
    }
    const double xl = (H.left() > 0.0) ? H.left() : 0.0, cl = K.left(), yl = I.left(), yu = I.right();
    double lo = GAOL_INFINITY, hi = -GAOL_INFINITY;
    if (xl == 0.0 && cl <= 0.0 && yu > 0.0) {
      // 0^y = 0 for y > 0: the y > 0 of I
      lo = (yl > 0.0) ? yl : 0.0;
      hi = yu;
    }
    if (xu > 0.0 && cu > 0.0) {
      if (cl <= 1.0 && cu >= 1.0 && xl <= 1.0 && xu >= 1.0) {
        // 1^y = 1 for every y
        lo = yl;
        hi = yu;
      } else {
        const pow_rel_end zero = approached(0.0), plus_infinity = approached(GAOL_INFINITY),
          minus_infinity = approached(-GAOL_INFINITY);
        pow_rel_end lower, upper;
        if (xu > 1.0) {
          /*
            x in [a1, a2], a1 > 1 or a1 = 1, which is then left out (1^y = 1
            is not in K): y in [log(cl)/log(x), log(cu)/log(x)], whose lower
            end decreases with x for cl > 1 and increases for cl < 1, whose
            upper end does the same with cu, and which tends to 0 for
            x -> +oo and to -oo or +oo for x -> 1
          */
          const double a1 = (xl > 1.0) ? xl : 1.0, a2 = xu;
          if (cl <= 0.0) {
            lower = minus_infinity;
          } else if (cl == 1.0) {
            lower = reached(0.0);
          } else if (cl > 1.0) {
            lower = (a2 == GAOL_INFINITY) ? zero : log_ratio_end(cl, a2, true);
          } else {
            lower = (a1 == 1.0) ? minus_infinity : log_ratio_end(cl, a1, true);
          }
          if (cu == GAOL_INFINITY) {
            upper = plus_infinity;
          } else if (cu == 1.0) {
            upper = reached(0.0);
          } else if (cu > 1.0) {
            upper = (a1 == 1.0) ? plus_infinity : log_ratio_end(cu, a1, false);
          } else {
            upper = (a2 == GAOL_INFINITY) ? zero : log_ratio_end(cu, a2, false);
          }
          add_pow_rel_piece(lower, upper, yl, yu, lo, hi);
        }
        if (xl < 1.0) {
          /*
            x in [b1, b2], b1 > 0 or b1 = 0, b2 < 1 or b2 = 1, both then left
            out: x^y decreases with y, and y in [log(cu)/log(x),
            log(cl)/log(x)], whose lower end decreases with x for cu > 1 and
            increases for cu < 1, whose upper end does the same with cl, and
            which tends to 0 for x -> 0 and to -oo or +oo for x -> 1
          */
          const double b1 = xl, b2 = (xu < 1.0) ? xu : 1.0;
          if (cu == GAOL_INFINITY) {
            lower = minus_infinity;
          } else if (cu == 1.0) {
            lower = reached(0.0);
          } else if (cu > 1.0) {
            lower = (b2 == 1.0) ? minus_infinity : log_ratio_end(cu, b2, true);
          } else {
            lower = (b1 == 0.0) ? zero : log_ratio_end(cu, b1, true);
          }
          if (cl <= 0.0) {
            upper = plus_infinity;
          } else if (cl == 1.0) {
            upper = reached(0.0);
          } else if (cl > 1.0) {
            upper = (b1 == 0.0) ? zero : log_ratio_end(cl, b1, false);
          } else {
            upper = (b2 == 1.0) ? plus_infinity : log_ratio_end(cl, b2, false);
          }
          add_pow_rel_piece(lower, upper, yl, yu, lo, hi);
        }
      }
    }
    return interval(lo, hi);
  }

  interval pow_exponent_rel(const interval& K, const interval& H, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = pow_exponent_rel_upward(K, H, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // The body of sqrt_rel(), defined below, which does not check the rounding
  // direction again (GAOL v5); those of nth_root() and sqrt() are
  // nth_root_upward() and sqrt_upward() (gaol/core/gaol_interval_internal.h)
  static interval sqrt_rel_upward(const interval& J, const interval& I);

  /*
    Code inspired by ia_math code by Timothy Hickey. The rounding direction
    is upward, after the check of nth_root_rel(), whose intersections and
    hulls are taken before the modes are restored with
    GAOL_PRESERVE_ROUNDING (GAOL v5, see gaol/gaol_fpu.h); the roots are
    those of the bodies of sqrt_rel() and nth_root(), which do not check the
    direction again (GAOL v5: they checked it a second time)
  */
  static interval nth_root_rel_upward(const interval& J, unsigned int n, const interval& I)
  {
    switch (n) {
    case 0:
      if (J.set_contains(1.0)) {
	return I;
      } else {
	return interval::emptyset();
      }
    case 1:
      return J & I;
    case 2:
      return sqrt_rel_upward(J,I);
    default:
      break;
    }
    // The roots of nth_root(), proved to be bounds (GAOL v5: GAOL took the
    // powers of the mathematical library with the exponent 1/n here too)
    if (I.is_empty()) {
      return interval::emptyset();
    }
    const interval tmp = nth_root_upward(J,n);
    if (odd(n) || tmp.is_empty()) {
      return (tmp & I);
    }
    // n is even: the roots of either sign
    if (I.certainly_positive()) {
      return (tmp & I);
    }
    if (I.certainly_negative()) {
      return (-tmp & I);
    }
    return ((tmp & I) | ((-tmp) & I));
  }

  interval nth_root_rel(const interval& J, unsigned int n, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = nth_root_rel_upward(J, n, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The tightest enclosure of the 1/x for the x != 0 of root, an interval of
    one sign (0 may be an end of it), the rounding direction being upward: what
    inverse() gives, but for the overflow. 1/x is beyond the largest double for
    an x next to 2^-1074, and inverse() raised the overflow exception for it,
    where nth_root_rel() intersects the result with an interval that may make
    it finite. The quotients are those of quotient_sat(): the infinity, or the
    largest double for a lower bound, without the exception (GAOL v5, point 79
    of TODO.md). The inverse of 0, an end of root, is +oo or -oo by the sign of
    the other end, without a division by zero, and [0] has no inverse.
  */
  static interval reciprocal_sat_upward(const interval& root)
  {
    // bound_equal(): where GAOL cannot clear the modes that flush the
    // subnormals to zero (ARM, the FPU intervals of Visual C++), a subnormal
    // bound is no 0
    const double l = root.left(), r = root.right();
    const bool l_is_0 = gaol_detail::bound_equal(l, 0.0), r_is_0 = gaol_detail::bound_equal(r, 0.0);
    if (l_is_0 && r_is_0) {
      return interval::emptyset();
    }
    const double upper = l_is_0 ? GAOL_INFINITY : quotient_sat(1.0, l);
    const double lower = r_is_0 ? -GAOL_INFINITY : -quotient_sat(-1.0, r);
    return interval(lower, upper);
  }

  /*
    The roots of the part J of one sign for the power of exponent -n, n > 0,
    that nth_root(J, -n) gives: 1/x^(1/n), the root then its inverse, the
    rounding direction being upward (GAOL v5)
  */
  static interval negative_roots_upward(const interval& J, unsigned int n)
  {
    const interval root = nth_root_upward(J, n);
    return root.is_empty() ? root : reciprocal_sat_upward(root);
  }

  /*
    The reverse of x^(-n), n > 0, the rounding direction being upward
    (GAOL v5). x^(-n) = 1/x^n has the sign of x for an odd n, and is positive
    for an even one: for x^(-n) in J, only the part of J at least 0 matters
    for an even n, and each part of J gives the roots of its sign, whose
    parts in I give the hull. J is cut at 0 before the roots: the inverse of
    an interval holding 0 is the hull of the inverses of its two parts, which
    joins them across the gap (nth_root([-1, 2], -3) is [-oo, +oo]), and
    intersecting the hull with I would keep the part of I in the gap, which
    is no solution, hull and intersection not commuting. Every part of I is
    taken without 0, which has no power of negative exponent
    (gaol_detail::nonzero_part()).
  */
  static interval nth_root_rel_negative_upward(const interval& J, unsigned int n, const interval& I)
  {
    if (J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    const interval roots = negative_roots_upward(J & interval::positive(), n);
    if (odd(n)) {
      return gaol_detail::nonzero_part(roots, I)
             | gaol_detail::nonzero_part(negative_roots_upward(J & interval::negative(), n), I);
    }
    if (roots.is_empty()) {
      return roots;
    }
    return gaol_detail::nonzero_part(roots, I) | gaol_detail::nonzero_part(-roots, I);
  }

  interval nth_root_rel(const interval& J, int p, const interval& I)
  {
    if (p > 0) {
      return nth_root_rel(J, static_cast<unsigned int>(p), I);
    }
    // One check for the roots, their inverses and the intersections. |p|
    // without a signed negation, as in nth_root(I, q): -p overflows for INT_MIN
    GAOL_RND_ENTER();
    interval res = p == 0 ? nth_root_rel_upward(J, 0u, I)
                          : nth_root_rel_negative_upward(J, 0u - static_cast<unsigned int>(p), I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    The relational atan2, atan2Rev1 and atan2Rev2 of IEEE 1788-2015 (GAOL v5).
    The points of the box Y x X whose angle is in C, (0, 0) left out, are
    taken quadrant by quadrant: the angles of C within (-pi, pi] are cut at
    the axes into the pieces [0, pi/2], [pi/2, pi], (-pi, -pi/2] and
    [-pi/2, 0], each the angles of a convex cone of a closed quadrant, whose
    intersection with the box is convex. A symmetry takes each quadrant to
    the first one, (u, v) being (x, y), (-x, y), (-x, -y) or (x, -y), and the
    angle theta to phi = theta, pi - theta, theta + pi or -theta in
    [0, pi/2]. The cone of a piece [phi_lo, phi_hi] is then
      { (u, v) | u, v >= 0, u tan(phi_lo) <= v <= u tan(phi_hi) },
    whose slopes are, up to their signs, tan(theta) at the bounds theta of C,
    computed by CORE-MATH from theta itself, correctly rounded, without
    rounding pi - theta nor theta + pi first; the ends of the pieces at the
    axes have the slopes 0 and +oo exactly. Rounding the lower slope downward
    and the upper one upward widens the cone, which keeps every point of it.
    The bounds of C are compared with the doubles around pi/2 and pi, which
    are no doubles: theta <= pi/2 is theta <= gaol_half_pi_dn.
  */

  // [a, b], or [-b, -a] when negated, a zero bound being +0 (the opposite of
  // a zero bound, and the upper bound -0 of X or Y, gave -0)
  static inline interval signed_interval(bool negated, double a, double b)
  {
    const double l = negated ? -b : a, u = negated ? -a : b;
    return interval((l == 0.0) ? 0.0 : l, (u == 0.0) ? 0.0 : u);
  }

  /*
    The points (u, v) of [ul, uu] x [vl, vu], in the first quadrant, with
    t_lo u <= v <= t_hi u, t_lo being 0 or positive, and finite, t_hi being
    0, positive or +oo; their u, with x = u or -u (x_negated), are added to
    the hull Xr, and their v, with y = v or -v, to Yr. By elimination of v,
    their u are those with vl / t_hi <= u <= vu / t_lo, and by elimination of
    u, their v those with ul t_lo <= v <= uu t_hi, the products and quotients
    rounded outward. At the slopes 0 and +oo these are the limits the cone
    has, the slope +oo leaving the v of u = 0 free, and the slope 0 the u of
    v = 0. The point (0, 0) is left out: a piece holding no other point adds
    nothing, nor does a piece with no point off v = 0 when without_v_zero (the
    half-line y = 0, x < 0, whose angle is pi, out of the third quadrant).
    Neither projection is added when either is empty: the box and the cone do
    not meet.
    t_hi_above tells that t_hi is a subnormal angle theta itself, its
    tangent being above it by less than theta^2/2 < 2^-2044 of it (see
    upper_slope()): a product or a quotient with theta is then the tightest
    bound with the tangent where it is inexact, its rounding moving it by more
    than that, and is moved one double outward where it is exact.
  */
  static void atan2_rel_piece(double ul, double uu, double vl, double vu, double t_lo, double t_hi, bool t_hi_above,
                              bool without_v_zero, bool x_negated, bool y_negated, interval& Xr, interval& Yr)
  {
    if (t_hi == 0.0 && vl > 0.0) { // The half-line v = 0 alone, which the v of V do not reach
      return;
    }
    // The slopes read in the branches that divide or multiply by them: 0/0,
    // +oo/+oo, +oo * 0 and v/0 are left out (see read_in_branch())
    // The quotients and the products of finite bounds, which are intersected
    // with the box below, are those of the saturating operations: a slope next
    // to 2^-1074 or to 2^54, with a bound next to the largest double, overflows
    // for a finite result (GAOL v5, point 79 of TODO.md)
    double u_lo = (t_hi == GAOL_INFINITY || vl == 0.0) ? 0.0 : quotient_dn_sat(vl, t_hi);
    const double u_hi = (t_lo == 0.0) ? GAOL_INFINITY
      : ((vu == 0.0) ? 0.0 : ((vu == GAOL_INFINITY) ? GAOL_INFINITY : quotient_up_sat(vu, t_lo)));
    const double v_lo = (ul == 0.0 || t_lo == 0.0) ? 0.0 : product_dn_sat(ul, t_lo);
    double v_hi = (t_hi == GAOL_INFINITY) ? GAOL_INFINITY
      : ((uu == 0.0 || t_hi == 0.0) ? 0.0 : ((uu == GAOL_INFINITY) ? GAOL_INFINITY : product_up_sat(uu, t_hi)));
    if (t_hi_above) {
      if (u_lo > 0.0 && fma_once(u_lo, read_in_branch(t_hi), -vl) == 0.0) {
        u_lo = previous_float(u_lo);
      }
      if (v_hi > 0.0 && v_hi < GAOL_INFINITY && fma_once(uu, read_in_branch(t_hi), -v_hi) == 0.0) {
        v_hi = next_float(v_hi);
      }
    }
    const double a = maximum(ul, u_lo), b = minimum(uu, u_hi), c = maximum(vl, v_lo), d = minimum(vu, v_hi);
    if (a > b || c > d || (b == 0.0 && d == 0.0) || (without_v_zero && d == 0.0)) {
      return;
    }
    Xr |= signed_interval(x_negated, a, b);
    Yr |= signed_interval(y_negated, c, d);
  }

  /*
    The tangent of theta in [0, pi/2) rounded upward, the upper slope of a
    cone, or theta itself for a subnormal theta, above being set: the
    tangent rounded upward is then the next double, which can be twice theta,
    when tan(theta) is above theta by theta^3/3 only (see atan2_rel_piece())
  */
  static inline double upper_slope(double theta, bool& above)
  {
    above = theta > 0.0 && theta < std::numeric_limits<double>::min();
    return above ? theta : tan_hi(theta);
  }

  // The hulls Yr and Xr of the ordinates and of the abscissas, computed after
  // the check of the rounding direction, before which no bound is compared
  // (see atan2())
  static void atan2_rel_upward(const interval& C, const interval& Y, const interval& X, interval& Yr, interval& Xr)
  {
    Yr = Xr = interval::emptyset();
    if (C.is_empty() || Y.is_empty() || X.is_empty()) {
      return;
    }
    const double cl = C.left(), cu = C.right(), yl = Y.left(), yu = Y.right(), xl = X.left(), xu = X.right();
    // The parts of X and Y on either side of the axes, as magnitudes: X & [0, +oo]
    // is [xp_l, xu] and -X & [0, +oo] is [xn_l, -xl] when not empty
    const double xp_l = (xl > 0.0) ? xl : 0.0, xn_l = (xu < 0.0) ? -xu : 0.0;
    const double yp_l = (yl > 0.0) ? yl : 0.0, yn_l = (yu < 0.0) ? -yu : 0.0;
    // [0, pi/2]: (u, v) = (x, y), phi = theta
    if (xu >= 0.0 && yu >= 0.0 && cu >= 0.0 && cl <= gaol_half_pi_dn) {
      bool above = false;
      const double t_lo = (cl <= 0.0) ? 0.0 : tan_lo(cl);
      const double t_hi = (cu >= gaol_half_pi_up) ? GAOL_INFINITY : upper_slope(cu, above);
      atan2_rel_piece(xp_l, xu, yp_l, yu, t_lo, t_hi, above, false, false, false, Xr, Yr);
    }
    // [pi/2, pi]: (u, v) = (-x, y), phi = pi - theta, whose slope is -tan(theta)
    if (xl <= 0.0 && yu >= 0.0 && cu >= gaol_half_pi_up && cl <= gaol_pi_dn) {
      const double t_lo = (cu >= gaol_pi_up) ? 0.0 : -tan_hi(cu);
      const double t_hi = (cl <= gaol_half_pi_dn) ? GAOL_INFINITY : -tan_lo(cl);
      atan2_rel_piece(xn_l, -xl, yp_l, yu, t_lo, t_hi, false, false, true, false, Xr, Yr);
    }
    // (-pi, -pi/2]: (u, v) = (-x, -y), phi = theta + pi, whose slope is
    // tan(theta); -pi is no angle, the half-line y = 0, x < 0 having the angle pi
    if (xl <= 0.0 && yl <= 0.0 && cu >= -gaol_pi_dn && cl <= -gaol_half_pi_up) {
      const double t_lo = (cl <= -gaol_pi_up) ? 0.0 : tan_lo(cl);
      const double t_hi = (cu >= -gaol_half_pi_dn) ? GAOL_INFINITY : tan_hi(cu);
      atan2_rel_piece(xn_l, -xl, yn_l, -yl, t_lo, t_hi, false, true, true, true, Xr, Yr);
    }
    // [-pi/2, 0]: (u, v) = (x, -y), phi = -theta, whose slope is -tan(theta),
    // tan(-theta)
    if (xu >= 0.0 && yl <= 0.0 && cu >= -gaol_half_pi_dn && cl <= 0.0) {
      bool above = false;
      const double t_lo = (cu >= 0.0) ? 0.0 : -tan_hi(cu);
      const double t_hi = (cl <= -gaol_half_pi_up) ? GAOL_INFINITY : upper_slope(-cl, above);
      atan2_rel_piece(xp_l, xu, yn_l, -yl, t_lo, t_hi, above, false, false, true, Xr, Yr);
    }
  }

  interval atan2_rel_y(const interval& C, const interval& X, const interval& Y)
  {
    GAOL_RND_ENTER();
    interval Yr, Xr;
    atan2_rel_upward(C, Y, X, Yr, Xr);
    GAOL_RND_KEEP(Yr);
    GAOL_RND_LEAVE();
    return Yr;
  }

  interval atan2_rel_x(const interval& C, const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval Yr, Xr;
    atan2_rel_upward(C, Y, X, Yr, Xr);
    GAOL_RND_KEEP(Xr);
    GAOL_RND_LEAVE();
    return Xr;
  }

  /*
    k pi + X, k being an integer double and X a bounded interval, enclosed
    within about one double (GAOL v5, issue #6): pi = pi_hi + pi_lo,
    pi_hi being the double below pi, and pi_lo lying between two consecutive
    doubles; k pi_hi is p + e exactly, p being the product rounded and e its
    rest, from fma(); k pi_lo is bounded by the products with the two
    doubles, rounded outward. The bounds of X are added to e + k pi_lo, and
    the sums to p, rounded once at the magnitude of the result: adding X to
    k pi rounded would round twice, and the bounds could be two doubles from
    the tightest. GAOL computed k [gaol_pi_dn, gaol_pi_up] + X, whose width, k 2^-51,
    the relational functions took on: their bounds were up to
    2^-49 max(1, |x|) from the values they had to keep. The rounding
    direction is upward.
  */
  static interval k_pi_plus(double k, const interval& X)
  {
    // 0x1.1a62633145c06p-53 and 0x1.1a62633145c07p-53, the doubles around
    // pi - gaol_pi_dn = 1.2246467991473531772e-16
    static const double pi_lo_dn = std::ldexp(4967757600021510.0,-105);
    static const double pi_lo_up = std::ldexp(4967757600021511.0,-105);
    const double p = k*gaol_pi_dn;
    const double e = fma_once(k,gaol_pi_dn,-p);
    // The products of k with the doubles around pi_lo, rounded outward
    const double below = (k >= 0.0) ? pi_lo_dn : pi_lo_up, above = (k >= 0.0) ? pi_lo_up : pi_lo_dn;
    const double lo_lo = -((-k)*below), lo_hi = k*above;
    const double hi = p + ((e + lo_hi) + X.right());
    const double lo = -((-p) + (((-e) - lo_lo) - X.left()));
    return interval(lo,hi);
  }

  /*
    The bounds of interval(x)/interval::pi() + shift, for a finite x of
    magnitude at most 2^52, rounded upward: the doubles operator/=(interval)
    and operator+=(double) give, in the two builds, computed on doubles. The
    division of interval(x), whose stored bounds are -x and x, by
    [gaol_pi_dn, gaol_pi_up] divides a stored bound above 0 by gaol_pi_dn and one below 0 by
    gaol_pi_up, and gives interval::zero() for x = 0. The stored bounds of
    interval::zero() are -0 and +0 in the FPU build, +0 and +0 in the SSE2
    build, and the sign of the zero reaches the bounds of acos_rel() through
    k_pi_plus(): they are taken from interval::zero(), -left() being the
    stored lower bound, rather than written 0.
  */
  static inline double lower_of_x_over_pi_plus(double x, double shift)
  {
    const double nl = (x == 0.0) ? -interval::zero().left() : (-x) / ((x < 0.0) ? gaol_pi_dn : gaol_pi_up);
    return -(nl - shift);
  }

  static inline double upper_of_x_over_pi_plus(double x, double shift)
  {
    const double nr = (x == 0.0) ? interval::zero().right() : x / ((x > 0.0) ? gaol_pi_dn : gaol_pi_up);
    return nr + shift;
  }

  /*
    The hull of the x of I whose image by a periodic function is in J (the
    relational acos_rel(), asin_rel() and atan_rel()), the preimage of J
    being the union of the pieces piece(i), the piece i lying on
    [(i - shift) pi, (i + 1 - shift) pi], shift being 0 for the cosine and
    1/2 for the sine and the tangent (GAOL v5, issue #6: GAOL had three
    copies of this, with k [pi]).

    A bound of I lies on the piece floor(x/pi + shift), or on the next one
    when the quotient, rounded outward, is off by one: the leftmost point of
    the preimage within I is on the piece of the left bound or the next one,
    the union of the pieces being symmetric about every multiple of pi (plus
    shift pi), and the rightmost point on the piece of the right bound or the
    previous one. Beyond 2^52 the quotient may be off by more, and I is kept
    as it is on that side; an I of a single double is decided by the image of
    the function, at every magnitude.

    The rounding direction is checked once (GAOL v5): the inverse image of J
    (inverse(), with the bounds of acos_lo(), asin_lo()... and no check of
    their own) and the quotients x/pi + shift are computed after the check
    here, where the relational functions called acos(J), asin(J) or atan(J),
    and four operations of intervals, which checked it once more each.

    For acos_rel() and asin_rel(), whose function has its image in [-1, 1]
    (bounded), a J outside [-1, 1] has no preimage, and a J containing it has
    the whole line: these are decided after the check too, though the modes
    flushing the subnormals to zero do not change how J compares with -1 and
    1, so that the check, which clears the modes, is made on every path of a
    non-empty J and I (GAOL v5, second review of point 4).
  */
  template<class Inverse, class Piece, class Image>
  static interval periodic_rel(const interval& J, const interval& I, double shift, bool bounded, Inverse inverse,
                               Piece piece, Image image)
  {
    if (J.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }
    // The check before the bounds are compared (see gaol/gaol_fpu.h)
    GAOL_RND_ENTER();
    if (bounded && (J.left() > 1.0 || J.right() < -1.0)) {
      GAOL_RND_LEAVE();
      return interval::emptyset();
    }
    if (bounded && J.set_contains(interval::minus_one_plus_one())) {
      GAOL_RND_LEAVE();
      return I;
    }
    if (I.left() == I.right()) {
      interval K = image(I) & J;
      GAOL_RND_KEEP(K);
      GAOL_RND_LEAVE();
      return K.is_empty() ? interval::emptyset() : I;
    }
    const interval Jinv = inverse(J);
    interval Ileft, Iright;
    if (std::fabs(I.left()) > two_power_52) {
      Ileft = I;
    } else {
      const double kl = std::floor(lower_of_x_over_pi_plus(I.left(), shift));
      Ileft = piece(kl, Jinv) & I;
      if (Ileft.is_empty()) {
	Ileft = piece(kl + 1.0, Jinv) & I;
      }
    }
    if (std::fabs(I.right()) > two_power_52) {
      Iright = I;
    } else {
      const double kr = std::floor(upper_of_x_over_pi_plus(I.right(), shift));
      Iright = piece(kr, Jinv) & I;
      if (Iright.is_empty()) {
	Iright = piece(kr - 1.0, Jinv) & I;
      }
    }
    GAOL_RND_KEEP(Ileft); GAOL_RND_KEEP(Iright);
    GAOL_RND_LEAVE();
    if (Ileft.is_empty() || Iright.is_empty()) {
      return interval::emptyset();
    }
    return interval(Ileft.left(),Iright.right());
  }

  interval acos_rel(const interval& J, const interval &I)
  {
    // The preimage of J: i pi + acos(J) for an even i, (i + 1) pi - acos(J)
    // for an odd i. acos(J) as acos() computes it, J & [-1, 1] being non-empty
    return periodic_rel(J, I, 0.0, true,
			[](const interval& X) {
			  const interval K = X & interval::minus_one_plus_one();
			  return interval(acos_lo(K.right()), acos_hi(K.left()));
			},
			[](double i, const interval& Jacos) { return integer_is_even(i) ? k_pi_plus(i, Jacos) : k_pi_plus(i + 1.0, -Jacos); },
			[](const interval& X) { return cos(X); });
  }

  interval asin_rel(const interval& J, const interval &I)
  {
    // The preimage of J: i pi + asin(J) for an even i, i pi - asin(J) for an
    // odd i (GAOL v5: GAOL computed pi/2 + acos_rel(J, I - pi/2), two
    // additions of an enclosure of pi/2 more). asin(J) as asin() computes it,
    // J & [-1, 1] being non-empty
    return periodic_rel(J, I, 0.5, true,
			[](const interval& X) {
			  const interval K = X & interval::minus_one_plus_one();
			  return interval(asin_lo(K.left()), asin_hi(K.right()));
			},
			[](double i, const interval& Jasin) { return k_pi_plus(i, integer_is_even(i) ? Jasin : -Jasin); },
			[](const interval& X) { return sin(X); });
  }

  interval atan_rel(const interval& J, const interval& I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }
    // The preimage of J: i pi + atan(J), atan(J) as atan() computes it
    return periodic_rel(J, I, 0.5, false,
			[](const interval& X) { return interval(atan_lo(X.left()), atan_hi(X.right())); },
			[](double i, const interval& Jatan) { return k_pi_plus(i, Jatan); },
			[](const interval& X) { return tan(X); });
  }

  /*
    acosh_rel(), asinh_rel() and atanh_rel() check the rounding direction
    before they compare the bounds of the inverse image with those of I, which
    acosh(), asinh() and atanh() returned with the modes that flush the
    subnormals to zero restored, with GAOL_PRESERVE_ROUNDING (GAOL v5, see
    gaol/gaol_fpu.h). The inverse images are those of the bodies of these
    functions, which do not check the direction again (GAOL v5)
  */
  static interval acosh_rel_upward(const interval &J, const interval &I)
  {
    if (I.is_empty() || J.is_empty()) {
      return interval::emptyset();
    }

    interval tmp = acosh_upward(J);

    if (I.certainly_positive()) {
      return I & tmp;
    }
    if (I.certainly_negative()) {
      return I & (-tmp);
    }
    return (I & -tmp) | (I & tmp);
  }

  interval acosh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = acosh_rel_upward(J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval asinh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = asinh_upward(J) & I;
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  interval atanh_rel(const interval &J, const interval &I)
  {
    GAOL_RND_ENTER();
    interval res = atanh_upward(J, false) & I;
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
   * invabs_rel: the hull of the x of I whose magnitude is in J, absRev of
   * IEEE 1788, the negative part of J holding no magnitude (GAOL v5:
   * GAOL took it as one, invabs_rel([-2, -1], I) being [-2, 2] & I instead of
   * the empty set, and invabs_rel([-2, 0], I) [-2, 2] & I instead of [0])
   */
  interval invabs_rel(const interval &Jall, const interval &I)
  {
    const interval J = Jall & interval::positive();
    if (J.is_empty()) {
      return interval::emptyset();
    }
    if (I.certainly_geq(interval::zero())) {
      return (J & I);
    }

    if (I.certainly_leq(interval::zero())) {
      return ((-J) & I);
    }
    return (J&I) | ((-J)&I);
  }

  /*
    The reverse functions of max, min, sign and floor (GAOL v5, point P.25 of
    TODO.md), which IBEX wrote itself: each returns the hull of the x of X
    that the function, given the other operand Y for max and min, sends into
    Z, the natural reverse extension of IEEE 1788-2015 (10.5.4, (5) and (6)),
    which names none of them. The sets are intervals, or unions of pieces,
    whose ends are bounds of X and Z, 0 and integers: no bound is rounded but
    the end floor(sup Z) + 1 of floor_rel(), and each result is the tightest
    enclosure of its set.

    Each makes the check of the rounding direction before it compares a bound,
    as the relational functions above, though only floor_rel() computes: the
    check clears the modes that flush the subnormal numbers to zero, under
    which a subnormal bound compares as a zero, and
    sign_rel([1], [-2^-1074, 2^-1074]), whose upper bound is then no longer
    above 0, would be empty rather than [0, 2^-1074] (see gaol/gaol_fpu.h).
    The bounds are compared with the quiet comparisons, as in the relations,
    and only once the empty arguments, whose bounds are NaN, are put aside.
  */

  // max_rel() after its check. max(x, y) is in Z if and only if x and y are
  // at most sup Z and one of them is at least inf Z.
  static interval max_rel_upward(const interval& Z, const interval& Y, const interval& X)
  {
    if (Z.is_empty() || Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    // Every y above Z: max(x, y) >= y > sup Z
    if (gaol_detail::quiet_greater(Y.left(), Z.right())) {
      return interval::emptyset();
    }
    // A y of Y in Z: max(x, y) is y for x <= y and x for y < x <= sup Z, in Z
    // both, so that every x up to sup Z is kept
    if (gaol_detail::quiet_greater_equal(Y.right(), Z.left())) {
      return X & interval(-GAOL_INFINITY, Z.right());
    }
    // Every y below Z: max(x, y) has to be x, in Z
    return X & Z;
  }

  interval max_rel(const interval& Z, const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = max_rel_upward(Z, Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  // min_rel() after its check, max_rel() turned upside down: min(x, y) is in Z
  // if and only if x and y are at least inf Z and one of them is at most sup Z
  static interval min_rel_upward(const interval& Z, const interval& Y, const interval& X)
  {
    if (Z.is_empty() || Y.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    // Every y below Z
    if (gaol_detail::quiet_less(Y.right(), Z.left())) {
      return interval::emptyset();
    }
    // A y of Y in Z: every x from inf Z on is kept
    if (gaol_detail::quiet_less_equal(Y.left(), Z.right())) {
      return X & interval(Z.left(), GAOL_INFINITY);
    }
    // Every y above Z: min(x, y) has to be x, in Z
    return X & Z;
  }

  interval min_rel(const interval& Z, const interval& Y, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = min_rel_upward(Z, Y, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    sign_rel() after its check. The x of X whose sign is in Z are the union of
    the negative x of X if Z holds -1, of 0 if Z holds 0 and X does, and of the
    positive x of X if Z holds 1; Z holding none of them gives the empty set
    (Z = [0.5, 0.7]). The negative x of X, where X has some, are
    [inf X, min(sup X, 0)), open at 0 when X goes on to 0 or beyond: their
    tightest closed enclosure ends at 0 all the same, every negative double
    next to 0 being in the set (sign_rel([-1], [-2, 3]) is [-2, 0], and
    sign_rel([-1], [0, 3]) is empty, sign(0) being 0). The positive x are
    their mirror image. Z cannot hold -1 and 1 without 0, being an interval.
  */
  static interval sign_rel_upward(const interval& Z, const interval& X)
  {
    if (Z.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double zl = Z.left(), zu = Z.right(), xl = X.left(), xu = X.right();
    const bool negative = gaol_detail::quiet_less_equal(zl, -1.0) && gaol_detail::quiet_less_equal(-1.0, zu)
      && gaol_detail::quiet_less(xl, 0.0);
    const bool zero = gaol_detail::quiet_less_equal(zl, 0.0) && gaol_detail::quiet_less_equal(0.0, zu)
      && gaol_detail::quiet_less_equal(xl, 0.0) && gaol_detail::quiet_less_equal(0.0, xu);
    const bool positive = gaol_detail::quiet_less_equal(zl, 1.0) && gaol_detail::quiet_less_equal(1.0, zu)
      && gaol_detail::quiet_greater(xu, 0.0);
    if (!negative && !zero && !positive) {
      return interval::emptyset();
    }
    // The lower bound of the first piece, the upper bound of the last one
    const double l = negative ? xl : ((zero || gaol_detail::quiet_less_equal(xl, 0.0)) ? 0.0 : xl);
    const double u = positive ? xu : ((zero || gaol_detail::quiet_greater_equal(xu, 0.0)) ? 0.0 : xu);
    return interval(l, u);
  }

  interval sign_rel(const interval& Z, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = sign_rel_upward(Z, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

  /*
    floor_rel() after its check. floor(x) is the integer n for x in [n, n + 1):
    the x whose floor is in Z make [ceil(inf Z), floor(sup Z) + 1), and Z
    holding no integer (ceil(inf Z) > floor(sup Z), as for Z = [0.5, 0.7])
    gives the empty set. That set is open at its upper end e = floor(sup Z) + 1:
    a closed interval containing it contains e as well, the reals just below e
    being in it, so that the tightest closed enclosure of its part in X ends at
    min(sup X, e), and an X that starts at e or above holds none of it
    (floor_rel([0], [1, 2]) is empty, where [0, 1] & [1, 2] would be [1]).
    e is rounded upward. Where it is no double, beyond 2^53 in magnitude,
    every double is an integer and none lies between floor(sup Z) and e: the
    rounded e is the double above floor(sup Z), the smallest double at least
    e, the tightest bound, and a double is below e if and only if it is below
    the rounded e, so that inf X < e and min(sup X, e) are decided on it
    exactly. The rounded e is +oo for sup Z = DBL_MAX, the tightest bound of
    [DBL_MAX, DBL_MAX + 1), and for sup Z = +oo: it is taken as +oo there
    rather than computed, DBL_MAX + 1 raising the overflow exception even where
    the result has no infinite bound (floor_rel([DBL_MAX], [0, 1]) is empty),
    and the overflow flag GAOL raises stands for an infinite bound of its
    result only (doc/using.md). The sum reads floor(sup Z) in the branch that
    tests it (read_in_branch()), so that no compiler adds 1 before the test:
    Clang 18 and 21 for 64-bit ARM, at -O3 with -frounding-math, and
    AppleClang computed floor(sup Z) + 1 for every Z, then chose between it
    and +oo with fcsel, and DBL_MAX + 1 raised the overflow flag (GAOL v5,
    tests/rounding_direction.cpp).
  */
  static interval floor_rel_upward(const interval& Z, const interval& X)
  {
    if (Z.is_empty() || X.is_empty()) {
      return interval::emptyset();
    }
    const double first = std::ceil(Z.left()), last = std::floor(Z.right());
    if (gaol_detail::quiet_greater(first, last)) {
      return interval::emptyset();
    }
    double end = GAOL_INFINITY;
    if (gaol_detail::quiet_less(last, (std::numeric_limits<double>::max)())) {
      end = read_in_branch(last) + 1.0; // rounded upward
    }
    const double xl = X.left(), xu = X.right();
    if (gaol_detail::quiet_greater(first, xu) || gaol_detail::quiet_greater_equal(xl, end)) {
      return interval::emptyset();
    }
    return interval(gaol_detail::quiet_greater(first, xl) ? first : xl,
                    gaol_detail::quiet_less(end, xu) ? end : xu);
  }

  interval floor_rel(const interval& Z, const interval& X)
  {
    GAOL_RND_ENTER();
    interval res = floor_rel_upward(Z, X);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }


  // sqrt_rel() after its check, made before the bounds are compared, as in
  // sqrt()
  static interval sqrt_rel_upward(const interval& J, const interval& I)
  {
    // The part of J in [0, +oo], computed as in sqrt() (GAOL v5)
    const interval Jpos = J & interval::positive();

    if (Jpos.is_empty() || I.is_empty()) {
      return interval::emptyset();
    }

	double l, r;

    // The lower bound as sqrt() computes it, in the upward rounding: GAOL set
    // the direction downward for it, then upward again, two changes of
    // direction per call whichever way GAOL is built (GAOL v5)
    if (Jpos.left() == 0.0) {
      l = 0.0;
      r = gaol_sqrt_up(Jpos.right());
    } else {
      const double x = Jpos.left();
      l = -gaol_minus_sqrt_down(x, gaol_sqrt_up(x));
      r = gaol_sqrt_up(Jpos.right());
    }

	interval Res(l,r);

    if (I.certainly_positive()) {
      return Res & I;
    }
    if (I.certainly_negative()) {
      return (-Res) & I;
    }
    return (I & Res) | (I & (-Res));
  }

  interval sqrt_rel(const interval& J, const interval& I)
  {
    GAOL_RND_ENTER();
    interval res = sqrt_rel_upward(J, I);
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    return res;
  }

} // namespace gaol_core
