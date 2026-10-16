/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The functions that the source files of the library call across one
 * another, declared once here rather than by hand in each file that calls
 * them (GAOL v5). They are internals of the library: the header is not
 * installed, and none of the functions is part of its interface.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-16 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_INTERNAL_H
#define GAOL_INTERNAL_H

#include <cstddef>
#include <string>

#include "gaol/gaol_interval_core.h"
#include "gaol/parser/gaol_parser.h"

namespace gaol_core {

  /*
    The reading of the n characters at s, NUL characters included, with the
    names of the functions given: true, with out set to the interval read, if
    they are an expression, and false, with out unchanged, otherwise. It
    throws again the exception an action or the lexer raised, and
    std::bad_alloc when the stack of the parser could not grow. Defined in
    gaol/parser/gaol_interval_parser.ypp, and called by the readers of a
    std::string (gaol::textToInterval(), gaol_ieee1788::textToInterval(),
    operator>>) with its length, where parse_interval() reads up to the first
    NUL character of its argument: a NUL character is no character of any
    token, and a string holding one is no expression (GAOL v5).
  */
  bool gaol_parse_string(const char* const s, std::size_t n, interval& out, ::gaol::parsing_names names);

  /*
    The scanner of flex for a reading, whose extra data is its context (a
    gaol_parsing_context of gaol/parser/gaol_interval_parser.ypp), and its
    destruction: gaol_parse_string() calls them. Defined in
    gaol/parser/gaol_interval_lexer.lpp. gaol_initialize_parsing() gives 0 if
    the scanner could not be allocated.
  */
  void* gaol_initialize_parsing(void* context);
  void gaol_cleanup_parsing(void* scanner);

  /*
    The pow of IEEE 1788-2015 (Table 9.1), after one check of the rounding
    direction: gaol_ieee1788::pow() is this function. Defined in
    gaol/gaol_interval_core.cpp, with the other powers.
  */
  interval pow_standard(const interval& x, const interval& y);

  /*
    The text of x that gaol_ieee1788::intervalToText() returns, an interval
    literal of IEEE 1788-2015. Defined in gaol/gaol_interval_core.cpp, with the
    other writers of intervals.
  */
  std::string literal_text(const interval& x);

} // namespace gaol_core

namespace gaol_detail {

  /*
    The sign of v - x, v being the number s writes, with no sign, and x a
    double, compared exactly: for the output of intervals, whose decimal bounds
    are rounded outward whatever the C library. Defined in
    gaol/parser/gaol_interval_lexer.lpp, which reads the numbers.
  */
  int compare_number_with_double(const char* s, double x);

} // namespace gaol_detail

#endif /* GAOL_INTERNAL_H */
