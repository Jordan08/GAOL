/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * User-defined literals for intervals: gaol::literals.
 *
 * This header provides user-defined literals so that `using namespace gaol::literals;`
 * allows writing interval literals as `0.1_iv`, `1e-3_iv`, `3_iv`,
 * etc. The RAW STRING literal operator ("..."_iv) parses the text using
 * textToInterval(), ensuring the interval ENCLOSING the exact value of the
 * literal text. The INTEGER and FLOATING-POINT literal operators return point
 * intervals for the given value.
 *
 * Note on behavior:
 *   - "0.1"_iv encloses the decimal 0.1 (not the double 0.1), via textToInterval
 *   - 3_iv returns the point interval [3, 3]
 *   - 0.1_iv returns the point interval [0.1, 0.1] (the double 0.1)
 *
 * For an enclosure of a floating-point literal, use the string form: "0.1"_iv
 * instead of 0.1_iv.
 *
 * The operators are header-only and inline, and compile as C++11 on GCC 9, Clang 18
 * and Visual C++. They give no warning under -Wall -Wextra -Wpedantic.
 *
 * Usage:
 *   #include "gaol/gaol_literals.h"
 *   using namespace gaol::literals;
 *   auto x = "0.1"_iv;  // interval enclosing the decimal 0.1
 *   auto y = 1e-3_iv;   // point interval [0.001, 0.001]
 *   auto z = 3_iv;      // point interval [3, 3]
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#ifndef GAOL_LITERALS_H
#define GAOL_LITERALS_H

#include <cstddef>
#include <string>

#include "gaol/gaol_interval.h"

namespace gaol {

  /*!
    \brief Namespace for user-defined literals.

    This namespace contains the user-defined literal operators for interval
    literals. Use `using namespace gaol::literals;` to enable them.
    
    \note The RAW STRING operator ("..."_iv) gives an enclosure via textToInterval.
    The INTEGER and FLOATING-POINT operators return point intervals.
    For an enclosure of a numeric literal, use the string form: "0.1"_iv.
  */
  namespace literals {

    /*!
      \brief Raw literal operator for interval literals.

      Converts a string literal to an interval using textToInterval().
      The literal text is parsed exactly as it appears, so `"0.1"_iv` reads
      the decimal 0.1 (not the double 0.1), `"1e-3"_iv` reads 0.001, etc.
      This gives an ENCLOSING interval.
      
      \tparam N The length of the literal string.
      \param str The literal string (without the _iv suffix).
      \return An interval enclosing the value of the literal text.
    */
    GAOL_NODISCARD inline ::gaol_core::interval operator"" _iv(const char* str, std::size_t N)
    {
      return ::gaol::textToInterval(std::string(str, N));
    }

    /*!
      \brief Integer literal operator for interval literals.

      Converts an unsigned long long integer literal to a point interval.
      This is a point interval [n, n], not an enclosure.
      
      \param n The integer value.
      \return A point interval [n, n].
      \see For an enclosure, use the string form: "3"_iv
    */
    GAOL_NODISCARD inline ::gaol_core::interval operator"" _iv(unsigned long long n)
    {
      return ::gaol_core::interval(static_cast<double>(n));
    }

    /*!
      \brief Floating-point literal operator for interval literals.

      Converts a long double floating-point literal to a point interval.
      This is a point interval [d, d] where d is converted to double, not an
      enclosure.
      
      \param d The floating-point value.
      \return A point interval [d, d] where d is converted to double.
      \see For an enclosure, use the string form: "0.1"_iv
    */
    GAOL_NODISCARD inline ::gaol_core::interval operator"" _iv(long double d)
    {
      double val = static_cast<double>(d);
      return ::gaol_core::interval(val);
    }

  } // namespace literals

} // namespace gaol

#endif /* GAOL_LITERALS_H */
