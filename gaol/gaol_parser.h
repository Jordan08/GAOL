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

  \author Frederic Goualard, Joran NININ
  \date   2001-10-19
*/


#ifndef GAOL_PARSER_H
#define GAOL_PARSER_H

#include "gaol/gaol_config.h"
#include <iosfwd>

namespace gaol_core {
  class interval;
}

/*
  The parser is in the namespace gaol, not in gaol_core: it calls GAOL's
  functions under their names in gaol, pow being GAOL's pow there (GAOL v5).
*/
namespace gaol {

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
    ignored: "[Empty]", "SIN(1)" and "1E3" are read.

    \note The out interval is not modified whenever an error occur.

    \note A call with a wrong number of arguments, an uncertain number as a
    bound and two different bounds in <a, b> throw input_format_error, and a
    second argument of nth_root, pown or rootn that is no integer
    invalid_action_error. Without exceptions, parse_interval() returns false
    for them (GAOL v5), as for any other string that is no expression.
    std::bad_alloc is thrown when memory runs out.

    \note A string may be as long as memory allows (GAOL v5): a sum of 100000
    terms is read in a time proportional to its length, and takes no more of
    the stack of the program than a sum of two terms. Its nesting is limited by
    the stack of the parser, which holds 10000 entries: an open parenthesis, an
    open bracket or a sign takes one until the expression it opens is complete,
    and a call of a function two or more, so that about 10000 nested
    parentheses, brackets or signs, and about 5000 nested calls, are read. A
    string nested deeper is refused, as any string that is no expression.

    The readers of a std::string, gaol::textToInterval(),
    gaol_ieee1788::textToInterval() and operator>>, read the whole of it: a
    NUL character is no character of any token, and a string holding one is
    no expression (GAOL v5).
  */
  extern GAOL_PUBLIC bool parse_interval(const char* const s, gaol_core::interval& out,
                                         parsing_names names = parsing_names::gaol);

} // namespace gaol

#endif /* GAOL_PARSER_H */
