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
  \file   gaol_parser.h
  \brief

  <long description>

  \author Frederic Goualard, Jordan NININ
  \date   2001-10-19
*/


#ifndef GAOL_PARSER_H
#define GAOL_PARSER_H

#include "gaol/gaol_config.h"
#include <cstddef>
#include <iosfwd>

/*
  The parser is in gaol_core, where interval is, and the namespace gaol names
  it (see the end of this file, and gaol/gaol_interval.h). The code of the
  parser (gaol/parser/gaol_interval_parser.ypp) opens gaol_core only, and names
  the functions of the standard in gaol_ieee1788; the pow of GAOL, which no
  namespace but gaol holds, is gaol_pow_hybrid() there (GAOL v5).
*/
namespace gaol_core {

  class interval;

  /*!
    \brief The names of the functions a string calls (GAOL v5)

    parsing_names::gaol, the names of GAOL, which gaol::textToInterval() and
    operator>> read, and parsing_names::ieee1788, those of IEEE 1788-2015,
    which gaol_ieee1788::textToInterval() reads: nth_root(8,3) is read with
    the names of GAOL, rootn(8,3) with those of the standard, and pow is the
    pow of each namespace.
  */
  enum class parsing_names { gaol, ieee1788 };

  /*!
    \brief Parse a string to create an interval.

    \param s   the string used for the input, up to its first NUL character
    \param out	the interval created.
    \param names the names of the functions s calls
    \return false if the input could not be parsed into a valid interval
    and true otherwise

    The string is an expression, computed with interval arithmetic (GAOL v5):
    numbers, constants and literals of intervals, combined in any order with
    the operators +, -, * and /, the signs, the parentheses and the calls of
    functions, a literal standing wherever a number may, in a bound too:
    "1+[1,2]", "2*cos([0,1])" and "[cos([0,1]), 2]" are read. GAOL 4 read the
    formats n, [n], [l, r], [l, +inf], [-inf, r] and [empty], and expressions
    of numbers only.

    - A number is decimal ("12", "0.1", ".12e3") or hexadecimal, with a
      binary exponent ("0x1.3p-1"). A number that is a double is read as that
      double, any other one as the tightest interval of doubles containing
      it, whatever its digits and the locale of the program: "0.1" is
      [0.09999999...,0.100...01].
    - The constants are pi, inf (or infinity), dmin and dmax, the least
      positive normal double and the largest double. inf alone is
      [dmax, +inf], and -inf [-inf, -dmax].
    - The literals of IEEE 1788-2015 (12.11): [a], which is a, [a, b], [a, ]
      and [, b], a bound left out being infinite, [ ] and [empty], the empty
      set, [entire] and [,], the whole real line, and the uncertain form
      3.56?1, [3.55, 3.57], which is no bound. A literal whose left bound is
      +inf or whose right bound is -inf is the empty set. <a, b> is [a, b],
      whose bounds have to be the same double.
    - The functions are those of GAOL, or those of IEEE 1788-2015 with
      parsing_names::ieee1788, with their arguments.

    Spaces are not significant except in numbers, and the case of letters is
    ignored: "[Empty]", "SIN(1)" and "1E3" are read, whatever the locale of
    the C library (GAOL v5).

    \note The out interval is not modified whenever an error occur.

    \note A call with a wrong number of arguments, an uncertain number as a
    bound and two different bounds in <a, b> throw input_format_error, and a
    second argument of nth_root, pown or rootn that is no integer
    invalid_action_error. Without exceptions, parse_interval() returns false
    for them (GAOL v5), as for any other string that is no expression.
    std::bad_alloc is thrown when memory runs out.

    \note A string may be as long as memory allows (GAOL v5): a sum of 100000
    terms is read in a time proportional to its length, and takes no more of
    the stack of the program than a sum of two terms. A token, a number, a
    name or a run of blanks, holds 2^29 characters at most, the buffer of the
    lexer: a longer one throws std::bad_alloc. The nesting of a string is
    limited by the stack of the parser, which holds 10000 entries: an open
    parenthesis, an open bracket or a sign takes one until the expression it
    opens is complete, the name and the parenthesis of a call two, and an
    operand and the operator or the comma after it two, until the operand
    that follows them is read, so that about 10000 nested parentheses,
    brackets or signs, 5000 nested calls of one argument and 3300 levels of
    "1+(1+(...))" are read. A string nested deeper is refused, as any string
    that is no expression.

    The readers of a std::string, gaol::textToInterval(),
    gaol_ieee1788::textToInterval() and operator>>, read the whole of it: a
    NUL character is no character of any token, and a string holding one is
    no expression (GAOL v5).
  */
  extern GAOL_PUBLIC bool parse_interval(const char* const s, interval& out,
                                         parsing_names names = parsing_names::gaol);

  /*
    The functions below are the reader's own, which the files of the library
    call across one another; they are no part of its interface, and the
    library does not export them (GAOL_PUBLIC).

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
  bool gaol_parse_string(const char* const s, std::size_t n, interval& out, parsing_names names);

  /*
    The scanner of flex for a reading, whose extra data is its context (a
    gaol_parsing_context of gaol/parser/gaol_interval_parser.ypp), and its
    destruction: gaol_parse_string() calls them. Defined in
    gaol/parser/gaol_interval_lexer.lpp. gaol_initialize_parsing() gives 0 if
    the scanner could not be allocated.
  */
  void* gaol_initialize_parsing(void* context);
  void gaol_cleanup_parsing(void* scanner);

} // namespace gaol_core

namespace gaol {
  using gaol_core::parsing_names;
  using gaol_core::parse_interval;
} // namespace gaol

#endif /* GAOL_PARSER_H */
