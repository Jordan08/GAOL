/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily
 * developed at the Swiss Federal Institute of Technology, Lausanne,
 * Switzerland, and is now developed at the Laboratoire d'Informatique de
 * Nantes-Atlantique, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * CVS: $Id: gaol_interval.h 247 2015-05-21 07:00:45Z goualard $
 * Last modified: Thu May  7 09:57:51 2009 on almighty
 * By: Frederic Goualard <Frederic.Goualard@univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval.h
  \brief  The interval class and operators

  \author Goualard Frederic
  \date   2001-09-28
*/


#ifndef __gaol_interval_h__
#define __gaol_interval_h__

#include <cmath>
#include <iosfwd>
#include <ios>
#include <string>
#include <limits>
#include <type_traits>
#include "gaol/gaol_config.h"
#include "gaol/gaol_double_op.h"
#include "gaol/gaol_port.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_limits.h"

#if defined (_MSC_VER)
#  include <float.h>
#  undef min
#  undef max
#endif


namespace gaol {

  /*!
    \brief Format to use for the output of intervals.

    The supported formats so far are the following:
    - bounds: the interval is output in the form "[l, r]" where l and r
    are respectively its left and right bounds
    - width: the interval is output in the form "c (+/- w)" where
    c is its center and w its width
    - center: the interval is output as a single value, its center.
    - hexa: same as "bounds" except that bounds are printed as hexadecimal
    values to avoid problems due to round-off errors when translating
    the floats into decimal
    - agreeing: the interval is output in the form "r [l, r]" where
    r is the number containing all the digits that are the same in both
    left and right bounds, and where l and r are the disagreeing
    remaining digits. See "Factored Notation for Interval I/O", Maarten
    Herman van Emden, CoRR index=cs.NA/0102023.
  */
  struct __GAOL_PUBLIC__ interval_format {
    enum format_t {
      bounds,
      width,
      center,
      hexa,
      agreeing
    };
  };


  class __GAOL_PUBLIC__ interval {
  public:
    //! Creates [-oo, +oo]
    interval(void);
    /*!
      \brief Creates [a, b], and the empty set when [a, b] is not an interval

      The empty set is the result for a lower bound of +oo, an upper bound of
      -oo, bounds in the wrong order and NaN bounds, as in IBEX: IEEE 1788-2015
      has no interval [+oo, +oo] nor [-oo, -oo] (10.5.8), and its constructor
      gives the empty set there (12.12.7).
    */
    interval(double a, double b);
    //! Creates [a, a], and the empty set for an infinite a or a NaN
    interval(double a);
    interval(const interval& I);
#if USING_SSE2_INSTRUCTIONS
    interval(const __m128d& xmm);
#endif // USING_SSE2_INSTRUCTIONS

    /*!
      \brief Creation of an interval from a C string.

      Allows creating intervals with bounds not representable with
      floats, e.g.: interval("0.1")=[0.09999,0.10001]. See the
      file jail_parser.h for a description of the supported format
      for representing intervals with std::strings.
     */
    interval(const char *const s);
    /*!
      \brief Creation of an interval from two C strings

      Allows creating intervals by providing a std::string for each bound. Each std::string
      is evaluated using interval arithmetic; the left (resp. right) bound of the first
      (resp. second) std::string is used to represent the left (resp. right) bound of the
      interval constructed.
      */
    interval(const char *const sl, const char *const sr);
    interval& operator+=(double d);
    interval& operator-=(double d);
    interval& operator*=(double d);
    interval& operator/=(double d);
    //! Relational division
    interval& operator%=(double d);
    interval& operator+=(const interval& I);
    interval& operator-=(const interval& I);
    interval& operator*=(const interval& I);
    interval& operator/=(const interval& I);
    interval& operator%=(const interval& I);

    interval operator+(void) const;
    interval operator-(void) const;

    interval inverse() const; // TODO: document inverse() method

    //! Intersection of *this and I
    interval& operator&=(const interval& I);
    //! Union of *this and I
    interval& operator|=(const interval& I);

    double left(void) const;
    double right(void) const;
    /*!
     \brief Returns the midpoint of an interval.

     Cases are:
     - \emptyset  -> Quiet NaN
     - [-oo, +oo] -> midP = 0.0
     - [-oo, b]   -> midP = -MAXREAL
     - [a, +oo]   -> midP = MAXREAL
     - [a, b]     -> midP = (a+b)/2 rounded to nearest, ties to even
    */
    double midpoint(void) const;
    /*!
      \brief Midpoint enclosing interval.

     Returns the tightest interval enclosing the midpoint of '*this', which is
     included in '*this'.
     - mid(\emptyset) = \emptyset
     - mid([a,b])     = the tightest interval enclosing (a+b)/2
     - mid([-oo,b])   = [-MAXREAL,-MAXREAL]
     - mid([a,+oo])   = [MAXREAL,MAXREAL]
     - mid([-oo,+oo]) = [0,0]
    */
    interval mid(void) const;
    /*!
      \brief Radius of an interval, rad of IEEE 1788-2015 (12.12.8)

      The smallest double r such that '*this' is included in [m-r, m+r], m
      being midpoint(): NaN for the empty set, +oo for an unbounded interval.
    */
    double rad(void) const;
    /*!
      \brief Midpoint and radius at once, midRad of IEEE 1788-2015 (12.12.8)

      Sets m to midpoint() and r to rad().
    */
    void mid_rad(double& m, double& r) const;


    bool certainly_positive(void) const;
    bool certainly_negative(void) const;
    bool certainly_strictly_positive(void) const;
    bool certainly_strictly_negative(void) const;
    bool certainly_geq(const interval &I) const;
    bool certainly_leq(const interval &I) const;
    bool certainly_ge(const interval &I) const;
    bool certainly_le(const interval &I) const;
    bool certainly_eq(const interval &I) const;
    bool certainly_neq(const interval &I) const;

    bool possibly_geq(const interval &I) const;
    bool possibly_leq(const interval &I) const;
    bool possibly_ge(const interval &I) const;
    bool possibly_le(const interval &I) const;
    bool possibly_eq(const interval &I) const;
    bool possibly_neq(const interval &I) const;

    bool set_contains(const interval& I) const;
    //! d shall not be a NaN
    bool set_contains(double d) const;
    bool set_strictly_contains(const interval& I) const;
    bool set_strictly_contains(double d) const;
    bool set_disjoint(const interval &I) const;
    bool set_eq(const interval& I) const;
    bool set_neq(const interval& I) const;
    bool set_leq(const interval& I) const;
    bool set_geq(const interval& I) const;
    bool set_le(const interval& I) const;
    bool set_ge(const interval& I) const;

    bool is_empty(void) const;
    bool is_symmetric(void) const;
    bool is_finite(void) const;
    /*!
      An interval is canonical if it is of the form [a, a] or
      of the form [a,a+] with a+ the smallest floating-point number larger
      than a
    */
    bool is_canonical(void) const;
    bool is_zero(void) const;
    bool straddles_zero(void) const;
    bool strictly_straddles_zero(void) const;
    bool is_a_double(void) const;
    bool is_an_int(void) const;

    ///! Size of the interval
    double width(void) const;

    /*!
     \brief Mignitude.

     Returns the mignitude of '*this', that is, mig(*this)=
     - min_  if *this > 0
     - -max_ if *this < 0
     - 0 otherwise
     See "Global Optimization using interval Analysis", chap. 3, Eldon Hansen
     */
    double mig(void) const;
       /*!
      \brief Magnitude

      Returns the magnitude of '*this', that is,
      mag(*this)=max(|min_|, |max_|)
    */
    double mag(void) const;

    /*!
      \brief Signed mignitude.

     Returns:
     - min_ if *this > 0
     - max_ if *this < 0
     - 0 otherwise

     See "interval Methods for Bounding the Range of Polynomials and Solving
     Systems of Nonlinear Equations", Volker Stahl PhD. thesis, def. 1.3.28.
    */
    double smig(void) const;


    static void format(interval_format::format_t f);
    static interval_format::format_t format(void);

// TODO: Change all static constants in interval class as functions
#if USING_SSE2_INSTRUCTIONS
     /// Mask for the sign bit of the left operand of an xmm register
    static const __m128d lbsignmask;
    /// Mask for the sign bits of both bounds
    static const __m128d lbrbsignmask;
    /// <0, 0>
    static const __m128d m128_zero;
    ///<-1, -1>
    static const __m128d m128_minus_one;
    ///<NaN Nan>
    static const __m128d m128_nan;
    /// < +oo +oo>
    static const __m128d m128_infinf;
    static const __m128d m128d_02mask;
    static const __m128d m128d_20mask;

    // Allocators defined to enforce alignment on a 16 bytes boundary
	void *operator new(size_t sz);
	void operator delete(void *p);

	void *operator new[](size_t sz);
	void operator delete[](void *p);

	void *operator new(size_t sz, void *p);
	void operator delete(void *p, void *place);

#else // !USING_SSE2_INSTRUCTIONS
    static const interval cst_emptyset;
    static const interval cst_universe;
    static const interval cst_zero;
    static const interval cst_positive;
    static const interval cst_negative;

#endif // USING_SSE2_INSTRUCTIONS
    static interval zero(void);
    static interval universe(void);
    static interval emptyset(void);
    static interval positive(void); // [0, +oo]
    static interval negative(void); // [-oo, 0]

    static const interval cst_one;
    static const interval cst_minus_one_plus_one;
    static const interval cst_pi;
    static const interval cst_two_pi;
    static const interval cst_half_pi;
    static const interval cst_one_plus_infinity;


    static interval one(void);
    static interval minus_one_plus_one(void);
    static interval pi(void);
    static interval two_pi(void);
    static interval half_pi(void);
    static interval one_plus_infinity(void);

    /*!
      @name Splitting methods
    */
    //@{
    /*!
      \brief Splits *this in two using midpoint().

      Returns the left part in I1 and the right one in I2
    */
    void split(interval &I1, interval &I2) const;

    //! Returns the left part (i.e. [left,midpoint()] of the domain.
    interval split_left(void) const;
    //! Returns the right part (i.e. [midpoint(),right] of the domain.
    interval split_right(void) const;
    //@} // End of splitting methods

    //! Conversion to std::string
    operator std::string() const;

    //! Returns the current number of digits used for display
    static std::streamsize precision(void);
    /*!
      Sets the number of digits for display to 'n'.
      \return The number of digits previously used
    */
    static std::streamsize precision(std::streamsize n);


    friend __GAOL_PUBLIC__ interval uipow(const interval& I, unsigned int e);
    friend __GAOL_PUBLIC__ interval sqr(const interval& I);
    friend __GAOL_PUBLIC__ interval cos(const interval& I);
    friend __GAOL_PUBLIC__ interval sin(const interval& I);
    friend __GAOL_PUBLIC__ interval sqrt(const interval& I);
    friend __GAOL_PUBLIC__ interval sqrt_rel(const interval& J, const interval& I);
    friend __GAOL_PUBLIC__ interval div_rel(const interval &K, const interval &J, const interval &I);

#if USING_SSE2_INSTRUCTIONS
    typedef GAOL_ALIGN16(double xmm2d[2]);
    void get_bounds(xmm2d& b) const;
    const __m128d& get_xmminterval(void) const;
    __m128d& get_xmminterval(void);
#endif // USING_SSE2_INSTRUCTIONS

  private:
#if USING_SSE2_INSTRUCTIONS
    __m128d xmmbounds; // Lower 64 bits: left bound negated, upper 64 bits: right bound
#else
    double lb_; // The left bound is stored negated
    double rb_;
#endif // USING_SSE2_INSTRUCTIONS
    double left_internal() const;
    double right_internal() const;
    static interval_format::format_t output;
    //! Number of digits to display
    static std::streamsize output_precision;
  };


#if USING_SSE2_INSTRUCTIONS
#   include "gaol/gaol_interval_sse.h"
#else
#   include "gaol/gaol_interval_fpu.h"
#endif // USING_SSE2_INSTRUCTIONS


  INLINE interval inverse(const interval& I)
  { // TODO: document inverse() function
    return I.inverse();
  }

  INLINE
  interval interval::operator+(void) const
  {
    return *this;
  }


 INLINE
  bool interval::is_empty(void) const
  {
    return !(left() <= right()); // Negation to handle NaNs
  }

  INLINE
  bool interval::is_symmetric(void) const
  {
    return !is_empty() && ((-left()) == right());
  }

  INLINE
  bool interval::certainly_positive(void) const
  {
    return is_empty() || (left() >= 0.0);
  }

  INLINE
  bool interval::certainly_negative(void) const
  {
    return is_empty() || (right() <= 0.0);
  }

  INLINE
  bool interval::certainly_strictly_positive(void) const
  {
    return is_empty() || (left() > 0.0);

  }

  INLINE
  bool interval::certainly_strictly_negative(void) const
  {
    return is_empty() || (right() < 0.0);
  }

  /*
    certainly_le() and certainly_leq() are strictPrecedes and precedes of IEEE
    1788-2015 (Table 10.3): for all x in *this and all y in I, x < y (x <= y),
    which is true when either interval is empty (Table 10.4), as for
    certainly_ge() and certainly_geq(). GAOL gave false when only I was empty,
    respectively only *this (fork of GAOL).
  */
  INLINE bool interval::certainly_ge(const interval &I) const
  {
    return is_empty() || I.is_empty() || (left() > I.right());
  }

  INLINE bool interval::certainly_geq(const interval &I) const
  {
    return is_empty() || I.is_empty() || (left() >= I.right());
  }

  INLINE bool interval::certainly_le(const interval &I) const
  {
    return is_empty() || I.is_empty() || (right() < I.left());
  }

  INLINE bool interval::certainly_leq(const interval &I) const
  {
    return is_empty() || I.is_empty() || (right() <= I.left());
  }


  /*
    For all x in *this and all y in I, x = y: both intervals are the same
    double, or both are empty. GAOL did not look at the lower bound of I, and
    [2] was certainly equal to [1,2] (fork of GAOL).
  */
  INLINE bool interval::certainly_eq(const interval &I) const
  {
    return (is_empty() && I.is_empty())
      || (left() == right() && I.left() == I.right() && left() == I.left());
  }

  INLINE bool interval::certainly_neq(const interval &I) const
  {
    return !certainly_eq(I);
  }


  INLINE bool interval::possibly_geq(const interval &I) const
  {
    return right()>= I.left();
  }


  INLINE bool interval::possibly_leq(const interval &I) const
  {
    return left() <= I.right();
  }

  INLINE bool interval::possibly_ge(const interval &I) const
  {
    return right() > I.left();
  }
  INLINE bool interval::possibly_le(const interval &I) const
  {
    return left() < I.right();
  }


  INLINE bool interval::possibly_eq(const interval &I) const
  {
    return (left() <= I.right()) && (right() >= I.left());
  }

  INLINE bool interval::possibly_neq(const interval &I) const
  {
    return (!is_empty() && !I.is_empty()) && !((right()<= I.left()) && (left()>=I.right()));
  }

  INLINE
  bool interval::is_zero(void) const
  {
    return (left() == 0.0 && right() == 0.0);
  }

  INLINE
  bool interval::straddles_zero(void) const
  {
    return (left() <= 0.0 && right() >= 0.0);
  }

  INLINE
  bool interval::strictly_straddles_zero(void) const
  {
    return (left() < 0.0 && right() > 0.0);
  }

  INLINE
  bool interval::is_a_double(void) const
  {
    return left() == right();
  }

  INLINE
  bool interval::is_an_int(void) const
  {
    return (left() == right()) && (std::floor(left()) == left()) &&
      (left() <= (std::numeric_limits<int>::max)() && // Strange call way needed by msvc++
       left() >= (std::numeric_limits<int>::min)());
  }



  INLINE bool interval::set_contains(const interval& I) const
  {
    return (I.is_empty() || ((left()<=I.left()) && (right()>=I.right())));
  }

  /**
    @note d should not be a NaN
    */
  INLINE bool interval::set_contains(double d) const
  {
    return ((left()<=d)&&(right()>=d));
  }

  /*
    I is interior to *this: interior of IEEE 1788-2015 (Table 10.3), where an
    infinite bound of *this is beyond the same infinite bound of I, and the
    empty set is interior to any interval, itself included. GAOL gave false for
    set_strictly_contains(I) when both had the same infinite bound, as
    interior(Entire, Entire), which is true (fork of GAOL).
  */
  INLINE bool interval::set_strictly_contains(const interval& I) const
  {
    return I.is_empty()
      || (!is_empty()
          && (left() < I.left() || left() == -GAOL_INFINITY)
          && (right() > I.right() || right() == GAOL_INFINITY));
  }

  /**
    @note d should not be a NaN
    */
  INLINE bool interval::set_strictly_contains(double d) const
  {
    return ((left()<d) && (right()>d));
  }

  INLINE bool interval::set_disjoint(const interval &I) const
  {
      return (right() < I.left()) || (left() > I.right())
	  || (is_empty() || I.is_empty());
  }

  INLINE bool interval::set_eq(const interval& I) const
  {
    return (is_empty() && I.is_empty()) || ((left()==I.left()) && (right()==I.right()));
  }

  INLINE bool interval::set_neq(const interval& I) const
  {
    return !set_eq(I);
  }

   // *this is interior to I (see set_strictly_contains()): GAOL gave false
   // for the empty set in the empty set (fork of GAOL)
   INLINE bool interval::set_le(const interval& I) const
   {
       return I.set_strictly_contains(*this);
   }

   INLINE bool interval::set_leq(const interval& I) const
   {
       return is_empty() || (left()>=I.left() && right()<=I.right());
   }

   INLINE bool interval::set_ge(const interval& I) const
  {
    return set_strictly_contains(I);
  }

   INLINE bool interval::set_geq(const interval& I) const
  {
    return set_contains(I);
  }

  INLINE void interval::split(interval &I1, interval &I2) const
  {
    double l = left(), m = midpoint(), r = right();
    I1 = interval(l,m);
    I2 = interval(m,r);
  }

  INLINE interval interval::split_left(void) const
  {
    return interval(left(),midpoint());
  }

  INLINE interval interval::split_right(void) const
  {
    return interval(midpoint(),right());
  }

  //! \brief Prototypes
  //@{
  /*!
    \brief Chi function

    Function given in:
    Interval Methods. Helmut Ratschek and Jon Rokne. Handbook of Global Optimization,
    R. Horst and P.M. Pardalos (eds.), p. 751--828, 1995 Kluwer Academic Publishers.

    This function characterizes the degree of symmetry of intervals.

    Definition:
    chi([a,b]) = 0,    if a==b==0
               = a/b,  if |a| <= |b|
               = b/a,  otherwise

   */
  extern __GAOL_PUBLIC__ double chi(const interval &I);
  /*!
    \brief maximum of 2 intervals
   */
  extern __GAOL_PUBLIC__ interval max(const interval &I, const interval &J);
  /*!
    \brief minimum of 2 intervals
   */
  extern __GAOL_PUBLIC__ interval min(const interval &I, const interval &J);

  extern __GAOL_PUBLIC__ std::ostream& operator<<(std::ostream& os,
					     const interval& I);
  extern __GAOL_PUBLIC__ std::istream& operator>>(std::istream& is,
					     interval& I);
  //! I^2
  extern __GAOL_PUBLIC__   interval sqr(const interval& I);
  //! I^e for an integer e.
  extern __GAOL_PUBLIC__   interval pow(const interval& I, int e);
  /*
    I^e for an unsigned e, the pown of IEEE 1788-2015: [1] for e = 0 and the
    empty set for an empty I. Declared here too, and defined out of line with
    the SSE2 intervals, where it was INLINE (fork of GAOL): gaol::uipow() was
    not found, and uipow() did not link. uipow_upup() and uipow_dnup(), which
    only computed parts of it on the stored bounds, are no longer declared.
  */
  extern __GAOL_PUBLIC__   interval uipow(const interval& I, unsigned int e);

  /*!
    \brief I^J (fork of GAOL)

    A degenerate integer exponent J = [n] always takes the integer power
    pow(const interval&, int), the pown of IEEE 1788, which is defined for a
    negative base too, gives x^0 = 1 for any x, 0 included, and has no value
    at 0 for n < 0; for an n beyond the ints, the result is [-oo,+oo]. Any
    other exponent takes the pow of IEEE 1788, exp(J log(I)) on the part of I
    in [0,+oo], where 0^y is 0 for y > 0 and has no value for y <= 0:
    pow([-4,9],[0.5]) is [0,3], pow([0],[0,1]) is [0], and
    pow([-4,-1],[1.5,2.5]) and pow([0],[-0.5]) are empty. An exponent [+oo] or
    [-oo] contains no real number, and gives the empty set. The result is thus
    not monotone for the inclusion of J: pow([-4,-1],[2]) is [1,16], and
    pow([0],[0]) is [1].
  */
  extern __GAOL_PUBLIC__   interval pow(const interval &I, const interval &J);

  /*!
    \brief I^p for a floating-point p (fork of GAOL)

    Without it, pow(I,2.5) called pow(const interval&, int), converting a
    double to an int being a standard conversion and converting it to an
    interval a user-defined one: the exponent was truncated, and pow([4],0.5)
    returned [1]. An integer p within the ints is computed by
    pow(const interval&, int), which is defined for negative bases too, any
    other p by pow(const interval&, const interval&), and an infinite or NaN p
    gives the empty set. Ported from the fix of Codac (commit 74086ccb, Jordan
    Ninin).
  */
  extern __GAOL_PUBLIC__  interval pow(const interval& I, double p);

  /*!
    \brief relational square root of J w.r.t. I
    \f$sqrt_rel(J,I) = Hull{x\in I\mid \exists y\in J\colon y=x^2}\f$
  */
extern __GAOL_PUBLIC__   interval sqrt_rel(const interval& J, const interval& I);

  /*!
    \brief relational division of K by J w.r.t. I

    div_rel(K,J,I) = hull(x\in I\mid\exists z\in K\exists y\in J:z=xy)
   */
extern __GAOL_PUBLIC__   interval div_rel(const interval &K,
				      const interval &J, const interval &I);

  /*!
    \brief square root of I
  */
extern __GAOL_PUBLIC__   interval sqrt(const interval& I);
  /*!
    \brief relational nth root of J w.r.t. I, for a positive n
    \f$nthroot_rel(J,I) = Hull{x\in I\mid \exists y\in J\colon y=x^n}\f$
  */
extern __GAOL_PUBLIC__   interval nth_root_rel(const interval& J,
					   unsigned int n, const interval& I);
  /*!
    \brief nth root of I for a positive n

    rootn of IEEE 1788-2015: for an odd n, the root of a negative x is
    -(-x)^(1/n), and nth_root([-8,27],3) encloses [-2,3]; for an even n, the
    roots of the part of I in [0,+oo]. nth_root(I,0) is the empty set.
  */
extern __GAOL_PUBLIC__   interval nth_root(const interval& I, unsigned int n);

  /*!
    \brief Returns the number of floating-point numbers in [a,b]

    - nb_fp_numbers(a,a+) == 2
    - nb_fp_numbers(a,a) == 1

    \warning Returns numeric_limits<ULONGLONGINT>::max() if either
    a or b is a NaN or +/-oo. In addition, raises an invalid_action_error
    exception or calls gaol_error depending on the way the library was
    configured.
    \precond a must be smaller or equal to b
  */
extern __GAOL_PUBLIC__   ULONGLONGINT nb_fp_numbers(double a, double b);

extern __GAOL_PUBLIC__	  unsigned short int modulo_k_pi(const interval &I, double &k_left, double &k_right);

extern __GAOL_PUBLIC__   interval exp(const interval& I);
extern __GAOL_PUBLIC__   interval log(const interval& I);

extern __GAOL_PUBLIC__   interval cos(const interval& I);
extern __GAOL_PUBLIC__   interval sin(const interval& I);
extern __GAOL_PUBLIC__   interval tan(const interval& I);

extern __GAOL_PUBLIC__   interval acos(const interval& I);
extern __GAOL_PUBLIC__   interval asin(const interval& I);
extern __GAOL_PUBLIC__   interval atan(const interval& I);

extern __GAOL_PUBLIC__   interval atan2(const interval& Y, const interval& X);

extern __GAOL_PUBLIC__   interval cosh(const interval& I);
extern __GAOL_PUBLIC__   interval sinh(const interval& I);
extern __GAOL_PUBLIC__   interval tanh(const interval& I);

extern __GAOL_PUBLIC__   interval acosh(const interval& I);
extern __GAOL_PUBLIC__   interval asinh(const interval& I);
extern __GAOL_PUBLIC__   interval atanh(const interval& I);

extern __GAOL_PUBLIC__   interval acos_rel(const interval& J, const interval &I);
extern __GAOL_PUBLIC__   interval asin_rel(const interval& J, const interval &I);
extern __GAOL_PUBLIC__   interval atan_rel(const interval& J, const interval &I);

extern __GAOL_PUBLIC__   interval acosh_rel(const interval& J, const interval &I);
extern __GAOL_PUBLIC__   interval asinh_rel(const interval& J, const interval &I);
extern __GAOL_PUBLIC__   interval atanh_rel(const interval& J, const interval &I);

extern __GAOL_PUBLIC__   interval abs(const interval &I);
extern __GAOL_PUBLIC__   interval invabs_rel(const interval &J, const interval &I);

  //@}




  // Non-methods
  //@{
  /*!
    \brief
   */

INLINE interval floor(const interval &I)
  {
    return interval(std::floor(I.left()),std::floor(I.right()));
  }

INLINE interval ceil(const interval &I)
  {
    return interval(std::ceil(I.left()),std::ceil(I.right()));
  }

INLINE interval integer(const interval &I)
  {
    return interval(std::ceil(I.left()),std::floor(I.right()));
  }

INLINE interval operator+(const interval& I, double d)
  {
    return interval(I)+=d;
  }

INLINE interval operator-(const interval& I, double d)
  {
    return interval(I)-=d;
  }


INLINE interval operator*(const interval& I, double d)
  {
    return interval(I)*=d;
  }

INLINE interval operator/(const interval& I, double d)
  {
    return interval(I)/=d;
  }

INLINE interval operator%(const interval& I, double d)
  {
    return interval(I)%=d;
  }

INLINE interval operator+(double d,const interval& I)
  {
    return interval(d)+=I;
  }

INLINE interval operator-(double d, const interval& I)
  {
    return interval(d)-=I;
  }

INLINE interval operator*(double d, const interval& I)
  {
    return interval(d)*=I;
  }

INLINE interval operator/(double d, const interval& I)
  {
    return interval(d)/=I;
  }

INLINE interval operator%(double d, const interval& I)
  {
    return interval(d)%=I;
  }

INLINE interval operator+(const interval& I1, const interval& I2)
  {
    return interval(I1) += I2;
  }

INLINE interval operator-(const interval& I1, const interval& I2)
  {
    return interval(I1) -= I2;
  }

INLINE interval operator*(const interval& I1, const interval& I2)
  {
    return interval(I1) *= I2;
  }

INLINE interval operator/(const interval& I1, const interval& I2)
  {
    return interval(I1) /= I2;
  }

INLINE interval operator%(const interval& I1, const interval& I2)
  {
    return interval(I1) %= I2;
  }

INLINE interval operator&(const interval& I1, const interval& I2)
  {
    return interval(I1) &= I2;
  }

INLINE interval operator|(const interval& I1, const interval& I2)
  {
    return interval(I1) |= I2;
  }

  //@}

 /*!
    \brief Diameter of an interval
    Returns the width of the interval (rounded upward).
    \note Returns NaN if the interval is empty, as wid of IEEE 1788-2015
    (12.12.8) does, rather than -1 (fork of GAOL)
   */
INLINE double interval::width(void) const
{
    if (is_empty()) {
        return GAOL_NAN;
    } else {
        GAOL_RND_ENTER();
        double res = right() - left();
        GAOL_RND_KEEP(res);
        GAOL_RND_LEAVE();
        return res;
    }
}

 /*!
    \brief Midpoint and radius (fork of GAOL)

    The radius is the smallest double r such that m-r <= left() and
    m+r >= right(), m being the midpoint: the greater of m-left() and
    right()-m, rounded upward.
   */
INLINE void interval::mid_rad(double& m, double& r) const
{
    m = midpoint();
    if (is_empty()) {
        r = GAOL_NAN;
        return;
    }
    GAOL_RND_ENTER();
    double below = m - left();
    double above = right() - m;
    double res = (below > above) ? below : above;
    GAOL_RND_KEEP(res);
    GAOL_RND_LEAVE();
    r = res;
}

INLINE double interval::rad(void) const
{
    double m, r;
    mid_rad(m, r);
    return r;
}

  //! Relations
  //@{
INLINE bool operator==(const interval &I1, const interval &I2)
  {
#if GAOL_SET_RELATION
    return I1.set_eq(I2);
#elif GAOL_CERTAINLY_RELATIONS
    return I1.certainly_eq(I2);
#else
    // No other case than GAOL_POSSIBLY_RELATIONS
    return I1.possibly_eq(I2);
#endif
  }

INLINE bool operator!=(const interval &I1, const interval &I2)
  {
#if GAOL_SET_RELATION
    return I1.set_neq(I2);
#elif GAOL_CERTAINLY_RELATIONS
    return I1.certainly_neq(I2);
#else
    // No other case than GAOL_POSSIBLY_RELATIONS
    return I1.possibly_neq(I2);
#endif
  }

INLINE bool operator<=(const interval &I1, const interval &I2)
  {
#if GAOL_SET_RELATION
    return I1.set_leq(I2);
#elif GAOL_CERTAINLY_RELATIONS
    return I1.certainly_leq(I2);
#else
    // No other case than GAOL_POSSIBLY_RELATIONS
    return I1.possibly_leq(I2);
#endif
  }

INLINE bool operator>=(const interval &I1, const interval &I2)
  {
#if GAOL_SET_RELATION
    return I1.set_geq(I2);
#elif GAOL_CERTAINLY_RELATIONS
    return I1.certainly_geq(I2);
#else
    // No other case than GAOL_POSSIBLY_RELATIONS
    return I1.possibly_geq(I2);
#endif
  }

INLINE bool operator<(const interval &I1, const interval &I2)
  {
#if GAOL_SET_RELATION
    return I1.set_le(I2);
#elif GAOL_CERTAINLY_RELATIONS
    return I1.certainly_le(I2);
#else
    // No other case than GAOL_POSSIBLY_RELATIONS
    return I1.possibly_le(I2);
#endif
  }

INLINE bool operator>(const interval &I1, const interval &I2)
  {
#if GAOL_SET_RELATION
    return I1.set_ge(I2);
#elif GAOL_CERTAINLY_RELATIONS
    return I1.certainly_ge(I2);
#else
    // No other case than GAOL_POSSIBLY_RELATIONS
    return I1.possibly_ge(I2);
#endif
  }

  //@}

INLINE double
  interval::left(void) const
  {
    return -left_internal();
  }

  INLINE double
  interval::right(void) const
  {
    return right_internal();
  }

extern __GAOL_PUBLIC__ bool feven(double d);


  /*!(void)
    \brief Hausdorff distance between two intervals:

    hausdorff([a,b],[c,d]) = max(|a-c|,|b-d|)
   */
  extern __GAOL_PUBLIC__ double hausdorff(const interval &I1, const interval &I2);

   INLINE interval interval::one(void)
	{
		return interval::cst_one;
	}
   INLINE interval interval::minus_one_plus_one(void)
	{
		return interval::cst_minus_one_plus_one;
	}

   INLINE interval interval::pi(void)
	{
		return interval::cst_pi;
	}

   INLINE interval interval::two_pi(void)
	{
		return interval::cst_two_pi;
	}

   INLINE interval interval::half_pi(void)
	{
		return interval::cst_half_pi;
	}

   INLINE interval interval::one_plus_infinity(void)
	{
		return interval::cst_one_plus_infinity;
	}



} // namespace gaol

#endif /* __gaol_interval_h__ */
