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

namespace gaol {

  /*
    textToInterval(s) of gaol/gaol_interval.h: what the constructor
    interval(const char*) of GAOL 4 did, which GAOL v5 no longer has
  */
  interval textToInterval(const std::string& s)
  {
    interval tmp;
    if (!gaol_core::gaol_parse_string(s.data(), s.size(), tmp, gaol_core::parsing_names::gaol)) {
      std::string err_msg("Syntax error in interval initialization: ");
      err_msg += gaol_detail::text_in_message(s);
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    return tmp;
  }

  namespace {
    /*
      A null pointer is no string (GAOL v5, point 128 of TODO.md): the
      overloads of std::string would build a std::string from it, which the
      standard leaves undefined (libstdc++ throws std::logic_error, and libc++
      and Visual C++ probably read it with strlen()). what names the argument
      in the message, as a syntax error does. The throw is the error of GAOL for
      a reading that fails, input_format_error; without exceptions, a message
      and abort() then.
    */
    void refuse_null_pointer(const char* s, const char* what)
    {
      if (s == nullptr) {
        std::string err_msg("Null pointer in ");
        err_msg += what;
        err_msg += " initialization: a null pointer is no string";
        GAOL_ERRNO = -1;
        gaol_ERROR(input_format_error,err_msg.c_str());
      }
    }
  }

  // textToInterval(s) of a C string, which takes the null pointer
  interval textToInterval(const char* s)
  {
    refuse_null_pointer(s, "interval");
    return textToInterval(std::string(s));
  }

  // The constructor interval(const char*, const char*) of GAOL 4, as a
  // function (GAOL v5): the left bound of the interval sl writes and the
  // right bound of the one sr writes
  interval textToInterval(const std::string& sl, const std::string& sr)
  {
    interval tmpl, tmpr;
    if (!gaol_core::gaol_parse_string(sl.data(), sl.size(), tmpl, gaol_core::parsing_names::gaol)) {
      std::string err_msg("Syntax error in left bound initialization: ");
      err_msg += gaol_detail::text_in_message(sl);
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    if (!gaol_core::gaol_parse_string(sr.data(), sr.size(), tmpr, gaol_core::parsing_names::gaol)) {
      std::string err_msg("Syntax error in right bound initialization: ");
      err_msg += gaol_detail::text_in_message(sr);
      GAOL_ERRNO = -1;
      gaol_ERROR(input_format_error,err_msg.c_str());
    }
    // An empty one gives the empty set, from its NaN bounds, which the
    // constructor tells with a quiet comparison (GAOL v5: it was told first,
    // when the constructor compared them with <=, which raises the
    // invalid-operation exception on a NaN)
    return interval(tmpl.left(), tmpr.right());
  }

  // textToInterval(sl, sr) with C strings, in either place or both, which take
  // the null pointer (GAOL v5, point 128 of TODO.md)
  interval textToInterval(const char* sl, const char* sr)
  {
    refuse_null_pointer(sl, "left bound");
    refuse_null_pointer(sr, "right bound");
    return textToInterval(std::string(sl), std::string(sr));
  }

  interval textToInterval(const char* sl, const std::string& sr)
  {
    refuse_null_pointer(sl, "left bound");
    return textToInterval(std::string(sl), sr);
  }

  interval textToInterval(const std::string& sl, const char* sr)
  {
    refuse_null_pointer(sr, "right bound");
    return textToInterval(sl, std::string(sr));
  }

} // namespace gaol
