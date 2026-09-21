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
#include <exception>

using std::istream;

#include "gaol/gaol_parser.h"
#include "gaol/gaol_interval.h"
#include "gaol/gaol_common.h"
#include "gaol/gaol_fpu.h"
#include "gaol/gaol_expr_eval.h"

extern void gaol_initialize_parsing(const char* const str,
				    gaol::interval* itv);
extern bool gaol_cleanup_parsing(void);
extern int gaol_parse(void);
extern std::exception_ptr gaol_parsing_exception;

namespace gaol {

  bool parse_interval(const char* const s, interval& out)
  {
    interval itv;
    gaol_parsing_exception = nullptr;
    gaol_initialize_parsing(s,&itv);
    try {
      gaol_parse();
    } catch (...) {
      // The buffer of the lexer is freed whatever the parser throws, as
      // gaol_ERROR() in its actions (GAOL v5)
      gaol_cleanup_parsing();
      throw;
    }
    bool parsing_ok = gaol_cleanup_parsing();
    if (gaol_parsing_exception) {
      // Thrown in the parser, which aborted to free its nodes
      std::exception_ptr e = gaol_parsing_exception;
      gaol_parsing_exception = nullptr;
      std::rethrow_exception(e);
    }
    if (parsing_ok) {
      // The infinities read give [dmax, +oo] and [-oo, -dmax] already
      // (gaol_expr_eval.h): interval(+oo) and interval(-oo) are the empty set
      out = itv;
    }
    return parsing_ok;
  }




} // namespace gaol
