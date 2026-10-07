/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * The literal _iv of intervals, in the namespace gaol::literals (GAOL v5).
 *
 * With using namespace gaol::literals, a number written with the suffix _iv
 * is the tightest interval enclosing the number the program writes, not the
 * double the compiler would make of it: 0.1_iv is the interval of the two
 * doubles around the decimal 0.1, as textToInterval("0.1") is, where
 * interval(0.1) is the point of the double 0.1, which is not 0.1;
 * 9007199254740993_iv encloses 2^53 + 1, which no double is. The literal
 * operator is the raw one, which receives the text of the literal as written
 * rather than its value: decimal and hexadecimal floating literals and
 * decimal integers are read by textToInterval(), the integers written in
 * hexadecimal, octal (010_iv is 8, as 010 is) and binary are taken as the
 * integers they are, and the digit separators of C++14 (1'000_iv) are left
 * out. A string literal with the suffix, "[1, 2]"_iv, is the interval
 * textToInterval() reads in it, and throws input_format_error if it is no
 * interval. Nothing is visible without the using-directive: the operators
 * are in gaol::literals only.
 *
 *   #include <gaol/gaol_literals.h>
 *   using namespace gaol::literals;
 *   const gaol::interval x = 0.1_iv, y = "[1, 2]"_iv;
 *
 * The operators are written operator""_iv, without a blank before the suffix:
 * C++23 deprecates the form with a blank, and C++11 takes both.
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
#include <cstddef>
#include <string>

#include "gaol/gaol_interval.h"

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

namespace gaol {

  /*!
    \brief The literal _iv of intervals (GAOL v5): using namespace gaol::literals.
  */
  namespace literals {

    /*!
      \brief A numeric literal with the suffix _iv: the tightest interval
      enclosing the number written (0.1_iv, 9007199254740993_iv, 0x1p-3_iv).
    */
    GAOL_NODISCARD inline ::gaol_core::interval operator""_iv(const char* text)
    {
      return ::gaol_detail::literal_interval(text);
    }

    /*!
      \brief A string literal with the suffix _iv: the interval
      textToInterval() reads in it ("[1, 2]"_iv, "1/3"_iv).
    */
    GAOL_NODISCARD inline ::gaol_core::interval operator""_iv(const char* text, std::size_t length)
    {
      return ::gaol::textToInterval(std::string(text, length));
    }

  } // namespace literals

} // namespace gaol

#endif /* GAOL_LITERALS_H */
