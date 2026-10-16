/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The text of numbers and of intervals that the library reads and writes
 * (GAOL v5).
 *
 * gaol_detail::literal_interval(text) is an interval enclosing the number
 * that a literal of C++ writes, text being the characters of the literal as
 * the program writes them, not the double the compiler would make of it:
 * "0.1" gives the interval of the two doubles around the decimal 0.1, as
 * textToInterval("0.1") does, where interval(0.1) is the point of the double
 * 0.1, which is not 0.1, and "9007199254740993" encloses 2^53 + 1, which no
 * double is. Decimal and hexadecimal floating literals and decimal integers
 * are read by textToInterval(), the integers written in hexadecimal, octal
 * ("010" is 8, as 010 is) and binary are taken as the integers they are, and
 * the digit separators of C++14 ("1'000") are left out.
 * gaol_core::literal_text(x) is the text that gaol_ieee1788::intervalToText()
 * writes for x.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-10-06 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_LITERALS_H
#define GAOL_LITERALS_H

#include <climits>
#include <string>

#include "gaol/gaol_interval.h"

namespace gaol_core {

  /*
    The text of x that gaol_ieee1788::intervalToText() returns, an interval
    literal of IEEE 1788-2015 (12.11), the literal that textToInterval() reads
    back (GAOL v5). A function of the library for its own files, which it does
    not export (GAOL_PUBLIC). Defined in gaol/gaol_interval_core.cpp, with the
    other writers of intervals.
  */
  std::string literal_text(const interval& x);

} // namespace gaol_core

namespace gaol_detail {

  /*
    The integer written with the digits of an integer literal of C++ in the
    given base, its prefix left out (GAOL v5): interval(n), the tightest
    interval enclosing it, where it holds in an unsigned long long, and an
    enclosure computed with the operations of intervals beyond, sound but
    not always the tightest.
  */
  inline ::gaol_core::interval literal_integer(const std::string& digits, unsigned int base)
  {
    unsigned long long n = 0;
    bool fits = true;
    for (std::string::size_type i = 0; i < digits.size(); ++i) {
      const char ch = digits[i];
      const unsigned int d = (ch >= '0' && ch <= '9') ? static_cast<unsigned int>(ch - '0')
                             : (ch >= 'a' && ch <= 'f') ? static_cast<unsigned int>(ch - 'a' + 10)
                             : static_cast<unsigned int>(ch - 'A' + 10);
      if (n > (ULLONG_MAX - d) / base) {
        fits = false;
        break;
      }
      n = n*base + d;
    }
    if (fits) {
      return ::gaol_core::interval(n);
    }
    ::gaol_core::interval x(0.0);
    for (std::string::size_type i = 0; i < digits.size(); ++i) {
      const char ch = digits[i];
      const double d = (ch >= '0' && ch <= '9') ? static_cast<double>(ch - '0')
                       : (ch >= 'a' && ch <= 'f') ? static_cast<double>(ch - 'a' + 10)
                       : static_cast<double>(ch - 'A' + 10);
      x = x*static_cast<double>(base) + ::gaol_core::interval(d);
    }
    return x;
  }

  // The interval of the text of a numeric literal of C++ (see gaol_literals.h)
  inline ::gaol_core::interval literal_interval(const char* text)
  {
    std::string t;
    for (; *text != '\0'; ++text) {
      if (*text != '\'') {
        t += *text;
      }
    }
    if (t.size() > 1 && t[0] == '0') {
      if (t[1] == 'x' || t[1] == 'X') {
        if (t.find_first_of("pP") != std::string::npos) {
          return ::gaol::textToInterval(t);
        }
        return literal_integer(t.substr(2), 16);
      }
      if (t[1] == 'b' || t[1] == 'B') {
        return literal_integer(t.substr(2), 2);
      }
      if (t.find_first_of(".eE") == std::string::npos) {
        return literal_integer(t.substr(1), 8);
      }
    }
    return ::gaol::textToInterval(t);
  }

} // namespace gaol_detail

#endif /* GAOL_LITERALS_H */
