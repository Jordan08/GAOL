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
  \file   gaol_interval.cpp
  \brief  The definitions of the namespace gaol: textToInterval

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include <string>

#include "gaol/gaol_config.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_exceptions.h"
#include "gaol/parser/gaol_parser.h"
#include "gaol/gaol_interval.h"

/*
  The reading of the n characters at s (gaol/parser/gaol_interval_parser.ypp), which
  the readers of a std::string call with its length (GAOL v5): they called
  parse_interval() with its c_str(), which the lexer read up to its first NUL
  character, so that "1\0junk" was read as 1. A NUL character is no character
  of any token, and such a string is no expression.
*/
extern bool gaol_parse_string(const char* const s, std::size_t n, gaol::interval& out,
                              gaol::parsing_names names);

namespace gaol {

  /*
    textToInterval(s) of gaol/gaol_interval.h: what the constructor
    interval(const char*) of GAOL 4 did, which GAOL v5 no longer has
  */
  interval textToInterval(const std::string& s)
  {
    interval tmp;
    if (!gaol_parse_string(s.data(), s.size(), tmp, parsing_names::gaol)) {
      std::string err_msg("Syntax error in interval initialization: ");
      err_msg += gaol_detail::text_in_message(s);
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    return tmp;
  }

  // The constructor interval(const char*, const char*) of GAOL 4, as a
  // function (GAOL v5): the left bound of the interval sl writes and the
  // right bound of the one sr writes
  interval textToInterval(const std::string& sl, const std::string& sr)
  {
    interval tmpl, tmpr;
    if (!gaol_parse_string(sl.data(), sl.size(), tmpl, parsing_names::gaol)) {
      std::string err_msg("Syntax error in left bound initialization: ");
      err_msg += gaol_detail::text_in_message(sl);
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
      return interval::emptyset();
    }
    if (!gaol_parse_string(sr.data(), sr.size(), tmpr, parsing_names::gaol)) {
      std::string err_msg("Syntax error in right bound initialization: ");
      err_msg += gaol_detail::text_in_message(sr);
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
      return interval::emptyset();
    }
    // An empty one gives the empty set, from its NaN bounds, which the
    // constructor tells with a quiet comparison (GAOL v5: it was told first,
    // when the constructor compared them with <=, which raises the
    // invalid-operation exception on a NaN)
    return interval(tmpl.left(), tmpr.right());
  }

} // namespace gaol
