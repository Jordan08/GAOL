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
 * CVS: $Id: gaol_parser.cpp 191 2012-03-06 17:08:58Z goualard $
 * Last modified:
 * By:
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_parser.cpp
  \brief

  <long description>

  \author Frederic Goualard
  \date   2001-10-19
*/

#include <iostream>
#include <cstring>
#include <cctype>

using std::istream;

#include "gaol/gaol_parser.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_expr_eval.h"

// The reading of a string by the parser of bison and the lexer of flex
// (gaol/gaol_interval_parser.ypp)
extern bool gaol_parse_string(const char* const s, gaol::interval& out,
                              gaol::parsing_names names);

namespace gaol {

  /*
    Several threads read strings at once (GAOL v5): the lexer is a reentrant
    one and the parser a pure one, and each reading has its own scanner and its
    own context, with the interval read, the exception an action raised and the
    names of the functions. The lexer and the parser of GAOL kept their state
    in globals: two threads reading a string each crashed, "fatal flex scanner
    internal error" or a segmentation fault.
  */
  bool parse_interval(const char* const s, interval& out, parsing_names names)
  {
    return gaol_parse_string(s, out, names);
  }

} // namespace gaol
