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
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_ieee1788.cpp
  \brief  The definitions of the namespace gaol_ieee1788: textToInterval and
          intervalToText

  \author NININ Jordan
  \date   2026-10-16
*/

#include <string>

#include "gaol/gaol_config.h"
#include "gaol/gaol_exceptions.h"
#include "gaol/gaol_ieee1788.h"
#include "gaol/parser/gaol_parser.h"

namespace gaol_ieee1788 {

  /*
    textToInterval(s) of IEEE 1788-2015 (gaol/gaol_ieee1788.h, 12.12.7), in
    the library rather than inline in its header, for the reading of the
    whole of s, NUL characters included (GAOL v5): the empty set for a string
    that is no literal, as for one that the reader refuses with an exception of
    GAOL, input_format_error or invalid_action_error. The other exceptions go
    through, std::bad_alloc among them: the reader threw it where memory ran
    out, and textToInterval() turned it into the empty set, the interval of no
    literal, catching every std::exception.
  */
  interval textToInterval(const std::string& s)
  {
#if GAOL_EXCEPTIONS_ENABLED
    try {
      interval x;
      return ::gaol_core::gaol_parse_string(s.data(), s.size(), x, ::gaol_core::parsing_names::ieee1788) ? x : interval::emptyset();
    } catch (const ::gaol_core::gaol_exception&) {
      return interval::emptyset();
    }
#else
    // An error of the reader fails the reading, as a syntax error does
    interval x;
    return ::gaol_core::gaol_parse_string(s.data(), s.size(), x, ::gaol_core::parsing_names::ieee1788) ? x : interval::emptyset();
#endif
  }

  /*
    intervalToText(x) of IEEE 1788-2015 (gaol/gaol_ieee1788.h, 13.3): the
    bounds of x rounded outward, [l, r], [a] for a point interval whose double
    the digits write exactly, as [4], or [empty], with the C locale, the flags
    a stream starts with and the digits of interval::precision() (16 unless
    the program sets another), so that the text is a portable literal
    (12.11.5) whatever the global output format, the flags of a stream and the
    locale of the program (GAOL v5). It is the text operator<< writes in the
    bounds format under these settings. GAOL wrote what operator<< writes: the
    width format "1.5 (+/- 0.5)", the agreeing digits, a decimal comma under
    the locale of a program that sets one, and <4, 4> for the point interval
    4, none of them a literal, and the text changed with the format another
    thread was setting.
    The writing itself is ::gaol_core::literal_text() (gaol/parser/gaol_parser.h),
    in gaol/gaol_interval_core.cpp, which holds the writers of intervals.
  */
  std::string intervalToText(const interval& x)
  {
    return ::gaol_core::literal_text(x);
  }

} // namespace gaol_ieee1788
