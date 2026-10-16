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
 * Copyright (c) 2026 ENSTA, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval.h
  \brief  The namespace gaol: GAOL under its own names

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/


#ifndef GAOL_INTERVAL_H
#define GAOL_INTERVAL_H

#include <string>

#include "gaol/gaol_interval_core.h"
#include "gaol/gaol_version.h"

/*
  The namespace gaol: GAOL under its own names (GAOL v5).

  The type interval and GAOL's functions are in gaol_core, and gaol names them
  one by one, with using-declarations (GAOL v5): gaol::interval and
  gaol::sin(x) are those of gaol_core. gaol holds the names GAOL 4 declared in
  its namespace gaol that GAOL v5 still names there: the type, its output
  format, the operators and the functions on intervals, the functions of
  doubles and the rounding direction that a program sets before it computes
  (previous_float(), next_float(), round_downward(), round_upward(),
  round_nearest() and is_finite()), the version of the library
  (version_major, version_minor, version_micro and version), and the names of
  gaol/gaol_common.h; the
  functions on intervals GAOL v5 adds (exp2, log2, hypot, trunc, sinpi, erf,
  fma, cancel_minus, round_ties_to_even...) and the nodes of the expressions
  it adds; and restore_rounding() and exact_string() of GAOL v5.
  It does not hold what stays in gaol_core, where a program names it:
  - what GAOL's code uses for itself: the functions saving and setting the
    rounding direction and the modes flushing the subnormals to zero
    (rounding_state, get_rounding(), set_rounding(), round_upward_if_needed()
    and gaol_set_rounding_x86(), which GAOL 4.3.2 declared in gaol,
    get_flush_modes()...), the functions computing GAOL's powers, which gaol
    names pow (gaol_pown(), gaol_uipow(), gaol_pown_ll(), gaol_pow_real(),
    gaol_pow_hybrid(), gaol_pown_exp(), gaol_pow_exp()), and the other
    internals, those GAOL 4 declared in gaol too (f_negate(),
    f_negate_simple(), gaol_signbit(), uintdouble, ullidouble,
    Interval_struct, the_null_expr, prec_t, modulo_k_pi(), reset_fpu_cw(),
    round_downward_sse(), round_to_nearest_sse() and round_upward_sse(), which
    its manual did not document) and rnd_reread(), gaol_sign_of(),
    rounding_guard and gaol_initializer;
  - the exceptions (gaol_exception, input_format_error,
    unavailable_feature_error and invalid_action_error), rnd_keep() of GAOL
    4.3.2, get_fpu_cw(), NaN_val of GAOL 4, which the macro GAOL_NAN names,
    eval_stack, and the reader of strings, parse_interval() and
    parsing_names.
  gaol took gaol_core whole before (using namespace gaol_core), and a program
  opening gaol met all of these names.

  A using-declaration names the overloads declared before it: this file
  includes gaol/gaol_interval_core.h, which includes the headers of what it
  names, and names it after them; gaol/gaol_common.h and the headers of the
  expressions name theirs at their end, and gaol/parser/gaol_expression.h
  names again the functions and the operators it overloads for the
  expressions. A call sin(x)
  on an interval finds by argument-dependent lookup the functions of gaol_core
  only, the namespace where interval is defined, never those of gaol or of
  gaol_ieee1788 (gaol/gaol_ieee1788.h): the same functions, whichever
  namespace the program opens. pow and textToInterval, which are not the same
  functions in the two, are therefore in each of them rather than in
  gaol_core, as plain functions of the same parameters: a program opens one of
  the two namespaces and gets its pow; with both open, pow(x, y) is ambiguous,
  and so is textToInterval(s).
*/
namespace gaol {

  // The version of the library (gaol/gaol_version.h)
  using gaol_core::version_major;
  using gaol_core::version_minor;
  using gaol_core::version_micro;
  using gaol_core::version;

  // The functions of doubles and the rounding direction of GAOL 4
  // (gaol/gaol_fpu_fenv.h or gaol/gaol_fpu_msvc.h, and gaol/gaol_port.h)
  using gaol_core::previous_float;
  using gaol_core::next_float;
  using gaol_core::round_downward;
  using gaol_core::round_upward;
  using gaol_core::round_nearest;
  using gaol_core::is_finite;

  // The type, its output format, and the functions and operators on intervals
  // of GAOL 4
  using gaol_core::interval;
  using gaol_core::interval_format;
  using gaol_core::operator+;
  using gaol_core::operator-;
  using gaol_core::operator*;
  using gaol_core::operator/;
  using gaol_core::operator%;
  using gaol_core::operator&;
  using gaol_core::operator|;
  using gaol_core::operator<;
  using gaol_core::operator<=;
  using gaol_core::operator>;
  using gaol_core::operator>=;
  using gaol_core::operator<<;
  using gaol_core::operator>>;
  using gaol_core::abs;
  using gaol_core::acos;
  using gaol_core::acos_rel;
  using gaol_core::acosh;
  using gaol_core::acosh_rel;
  using gaol_core::asin;
  using gaol_core::asin_rel;
  using gaol_core::asinh;
  using gaol_core::asinh_rel;
  using gaol_core::atan;
  using gaol_core::atan2;
  using gaol_core::atan_rel;
  using gaol_core::atanh;
  using gaol_core::atanh_rel;
  using gaol_core::ceil;
  using gaol_core::chi;
  using gaol_core::cos;
  using gaol_core::cosh;
  using gaol_core::div_rel;
  using gaol_core::exp;
  using gaol_core::feven;
  using gaol_core::floor;
  using gaol_core::hausdorff;
  using gaol_core::integer;
  using gaol_core::invabs_rel;
  using gaol_core::inverse;
  using gaol_core::lexicographic_less;
  using gaol_core::log;
  using gaol_core::max;
  using gaol_core::min;
  using gaol_core::nb_fp_numbers;
  using gaol_core::nth_root;
  using gaol_core::nth_root_rel;
  using gaol_core::sin;
  using gaol_core::sinh;
  using gaol_core::sqr;
  using gaol_core::sqrt;
  using gaol_core::sqrt_rel;
  using gaol_core::tan;
  using gaol_core::tanh;
  // The functions on intervals of GAOL v5, and the exact text of an interval
  using gaol_core::acospi;
  using gaol_core::asinpi;
  using gaol_core::atan2_rel_x;
  using gaol_core::atan2_rel_y;
  using gaol_core::atan2pi;
  using gaol_core::atanpi;
  using gaol_core::cancel_minus;
  using gaol_core::cancel_plus;
  using gaol_core::cospi;
  using gaol_core::erf;
  using gaol_core::erfc;
  using gaol_core::exp10;
  using gaol_core::exp10m1;
  using gaol_core::exp2;
  using gaol_core::exp2m1;
  using gaol_core::expm1;
  using gaol_core::floor_rel;
  using gaol_core::fma;
  using gaol_core::hull;
  using gaol_core::hypot;
  using gaol_core::intersect;
  using gaol_core::log10;
  using gaol_core::log10p1;
  using gaol_core::log1p;
  using gaol_core::log2;
  using gaol_core::log2p1;
  using gaol_core::max_rel;
  using gaol_core::min_rel;
  using gaol_core::pow_exponent_rel;
  using gaol_core::pow_rel;
  using gaol_core::round_ties_to_away;
  using gaol_core::round_ties_to_even;
  using gaol_core::rsqrt;
  using gaol_core::sign;
  using gaol_core::sign_rel;
  using gaol_core::sinpi;
  using gaol_core::tanpi;
  using gaol_core::trunc;
  using gaol_core::exact_string;

  //! pow(I, e), pow(I, J), pow(I, p): GAOL's power, gaol_pown(I, e), gaol_pow_hybrid(I, J), gaol_pow_real(I, p)
  GAOL_NODISCARD inline interval pow(const interval& I, int e) { return gaol_core::gaol_pown(I, e); }
  GAOL_NODISCARD inline interval pow(const interval& I, const interval& J) { return gaol_core::gaol_pow_hybrid(I, J); }
  GAOL_NODISCARD inline interval pow(const interval& I, double p) { return gaol_core::gaol_pow_real(I, p); }
  //! pow(I, e): gaol_uipow(I, e), I^e for an unsigned e
  GAOL_NODISCARD inline interval pow(const interval& I, unsigned int e) { return gaol_core::gaol_uipow(I, e); }
  //! pow(I, n) for an integer n of another type, long, long long, short... (GAOL v5): see gaol_detail::integer_power()
  template <class T, gaol_detail::if_integer<T> = 0>
  GAOL_NODISCARD inline interval pow(const interval& I, T n) { return gaol_detail::integer_power(I, n); }

  /*!
    textToInterval(s): the interval s writes, read with the names of the
    functions of GAOL (GAOL v5), where GAOL 4 had the constructor
    interval(const char*). The syntax of s is given in the section "Input
    format" of the manual of GAOL v5 (manual/v5/gaol.tex). A string that is
    no interval throws input_format_error. gaol_ieee1788 has its own, which
    reads the names of IEEE 1788-2015 and gives the empty set for such a
    string: as for pow, a program opens one of the two namespaces.
  */
  GAOL_NODISCARD extern GAOL_PUBLIC interval textToInterval(const std::string& s);
  /*!
    textToInterval(sl, sr): the left bound of the interval sl writes and the
    right bound of the one sr writes, which the constructor from two strings
    of GAOL 4 gave; a string that is no interval throws input_format_error
  */
  GAOL_NODISCARD extern GAOL_PUBLIC interval textToInterval(const std::string& sl, const std::string& sr);

} // namespace gaol

#endif /* GAOL_INTERVAL_H */
