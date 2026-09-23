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
// std::mutex, which libstdc++ has only when built with a thread model: without
// one (MinGW-w64 with the win32 threads, before GCC 13), there are no
// std::thread either, and nothing to lock
#if !defined(__GLIBCXX__) || defined(_GLIBCXX_HAS_GTHREADS)
#  include <mutex>
#  define GAOL_PARSING_LOCK 1
#endif

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
extern gaol::parsing_names gaol_parsing_names;

#if GAOL_PARSING_LOCK
namespace {
  /*
    One string read at a time (GAOL v5). The lexer of flex keeps its buffer
    and its position in globals, the parser of bison its token and its value,
    and GAOL the interval read, the success of the reading, the exception an
    action raised and the names of the functions: two threads reading a string
    each at once crashed, "fatal flex scanner internal error" or a
    segmentation fault. interval(const char*), operator>>, and the
    textToInterval() of gaol and of gaol_ieee1788 all go through
    parse_interval(), which holds this lock while it reads.
  */
  std::mutex gaol_parsing_mutex;
}
#endif

namespace gaol {

  bool parse_interval(const char* const s, interval& out, parsing_names names)
  {
#if GAOL_PARSING_LOCK
    std::lock_guard<std::mutex> lock(gaol_parsing_mutex);
#endif
    interval itv;
    gaol_parsing_exception = nullptr;
    // The names of the functions the lexer looks up (GAOL v5)
    gaol_parsing_names = names;
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
