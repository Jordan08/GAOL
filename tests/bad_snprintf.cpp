/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the bounds of an interval written with a C library whose
 * snprintf() writes wrong digits.
 *
 * GAOL writes the bound of an interval from the text snprintf() gives of the
 * double, rounded to nearest, and moves its last digit until the number it
 * writes is on the side of the interval (gaol/core/gaol_interval_io.cpp). The
 * msvcrt.dll of Windows, which MinGW-w64 calls for snprintf() when
 * __USE_MINGW_ANSI_STDIO is 0, writes no more than 17 significant digits
 * right and zeros after them: a bound of 20 digits or more was left on the
 * wrong side, inside the interval, by the four moves of its last digit, and
 * move_to_side() now writes a number of the shape of the text that is on the
 * right side, loose by up to ten times away from zero and 0 toward it.
 *
 * This test writes such an snprintf(): the linker sends the calls of the
 * program and of the static libgaol to snprintf() and __snprintf_chk() (which
 * the fortified calls of the C library of GNU reach) to the functions below,
 * which call the C library, round the number to 17 significant digits and
 * replace the following ones by zeros, as the msvcrt.dll does
 * (-Wl,--wrap, which tests/CMakeLists.txt, tests/Makefile.am and
 * tests/meson.build give this test alone). A shared libgaol is out of the
 * linker's reach, so the build makes this test with a static one only, on
 * Linux; where the functions are not called all the same, it is skipped (exit
 * status 77): Clang with _FORTIFY_SOURCE has the library call
 * __vsnprintf_chk(), which is not wrapped.
 *
 * The 264 intervals [x, the next double after x], of the doubles below, both
 * signs, in the three formats and with 17, 20, 25 and 30 digits, are written
 * with the C library right, then with it wrong, and each bound is read back
 * by gaol::textToInterval(), which reads a number as the doubles around it,
 * whose bounds tell on which side of the double the number is: the lower
 * bound has to be at most x and the upper one at least the next double. With
 * the code of GAOL before the fix, 202 of the 528 bounds written with the
 * wrong digits were on the wrong side, 92 lower bounds and 110 upper ones,
 * the rounding of the 17th digit putting the number now above, now below the
 * double. How loose a bound is, up to ten times
 * away from zero, is not checked: in the fixed format, a bound is no closer
 * than its last digit.
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-14)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#include <cstdarg>

#include "gaol_tests.h"

using namespace gaol;
using namespace gaol_tests;

namespace
{
  bool wrong_digits = false;  // does the snprintf() below write wrong digits?
  long snprintf_calls = 0;    // the calls to it, of GAOL or of the program
  long digits_replaced = 0;   // the digits after the 17th that it replaced by zeros

  // The text snprintf() wrote, as the msvcrt.dll writes it: the number rounded
  // to 17 significant digits (0.1 is 0.10000000000000001, above the double),
  // then zeros, in the mantissa of the formats %e and %f, the exponent being
  // left alone. The rounding goes up as the 18th digit says, half up, and
  // toward zero where the 17 digits are all 9, whose carry would add a digit.
  // The text is above the number or below it, so that the bounds of both sides
  // of an interval are tried
  void write_wrong_digits(char *buffer, std::size_t size, int written)
  {
    ++snprintf_calls;
    if (!wrong_digits || written <= 0 || static_cast<std::size_t>(written) >= size) {
      return;
    }
    char *end = buffer;
    while (*end != '\0' && *end != 'e' && *end != 'E') {
      ++end;
    }
    // The significant digits of the mantissa: from the first that is not 0
    std::vector<char *> digit;
    for (char *p = buffer; p != end; ++p) {
      if (*p >= '1' && *p <= '9') {
        digit.push_back(p);
      } else if (*p == '0' && !digit.empty()) {
        digit.push_back(p);
      }
    }
    if (digit.size() <= 17) {
      return;
    }
    bool up = (*digit[17] >= '5');
    std::size_t last = 16;  // the last of the 17 digits that is not a 9, which the carry reaches
    while (last > 0 && *digit[last] == '9') {
      --last;
    }
    up = up && *digit[last] != '9';
    for (std::size_t i = 17; i < digit.size(); ++i) {
      digits_replaced += (*digit[i] != '0');
      *digit[i] = '0';
    }
    if (up) {
      ++*digit[last];
      for (std::size_t i = last + 1; i < 17; ++i) {
        *digit[i] = '0';
      }
      ++digits_replaced;
    }
  }
}

// The functions the linker redirects the calls to, kept by used: a link with
// -flto -static dropped them, as the code of the library that calls them is
// seen at the link only
extern "C" {
  __attribute__((used)) int __wrap_snprintf(char *buffer, std::size_t size, const char *format, ...)
  {
    std::va_list arguments;
    va_start(arguments, format);
    const int written = std::vsnprintf(buffer, size, format, arguments);
    va_end(arguments);
    write_wrong_digits(buffer, size, written);
    return written;
  }

  // What the C library of GNU calls for snprintf() of a program compiled with
  // _FORTIFY_SOURCE: the flag and the size of the object are of no use here
  __attribute__((used)) int __wrap___snprintf_chk(char *buffer, std::size_t size, int, std::size_t,
                                                  const char *format, ...)
  {
    std::va_list arguments;
    va_start(arguments, format);
    const int written = std::vsnprintf(buffer, size, format, arguments);
    va_end(arguments);
    write_wrong_digits(buffer, size, written);
    return written;
  }
}

namespace
{
  const double inf = std::numeric_limits<double>::infinity();

  /*
    The intervals [x, the next double] of every x of the list, of both signs,
    written in the three formats with four precisions, and each bound read
    back: it is on the side of the interval. kind says what snprintf() does,
    for the names of the checks. Returns the number of bounds written.
  */
  long write_and_read(const char *kind)
  {
    // 1/3 and 2/3 to nearest as hexadecimal literals: 1.0 / 3.0 is computed
    // at run time, in the rounding direction of GAOL (-frounding-math)
    const double values[] = { 0.1, 0.3, 0x1.5555555555555p-2, 0x1.5555555555555p-1, 1e-300, 1.7e308, 123456.789,
                              5e-324, 0.7, 9.999999999999999e22, 1.0 };
    const int precisions[] = { 17, 20, 25, 30 };
    const std::ios_base::fmtflags formats[] = { std::ios_base::fmtflags(), std::ios_base::scientific,
                                                std::ios_base::fixed };
    const std::streamsize saved_precision = interval::precision();
    long bounds = 0;
    for (const double magnitude : values) {
      for (const double sign : { 1.0, -1.0 }) {
        const double x = sign * magnitude, next = std::nextafter(x, inf);
        for (const std::ios_base::fmtflags format : formats) {
          for (const int precision : precisions) {
            interval::precision(precision);
            std::ostringstream out;
            out.setf(format, std::ios_base::floatfield);
            out << interval(x, next);
            const std::string text = out.str();
            const std::size_t comma = text.find(", ");
            const auto describe = [&] {
              return text + " for " + hex(x) + " and " + hex(next) + ", " + std::to_string(precision) + " digits";
            };
            if (!check(std::string("the interval [x, next] is written [lower, upper]") + kind,
                       text.size() > 4 && text[0] == '[' && comma != std::string::npos && text.back() == ']',
                       describe)) {
              continue;
            }
            const interval lower = textToInterval(text.substr(1, comma - 1)),
                           upper = textToInterval(text.substr(comma + 2, text.size() - comma - 3));
            bounds += 2;
            // The number that the lower bound writes is at most x if the
            // double above it is: the bounds of its reading are the doubles
            // around it. The same for the upper bound and the next double
            check(std::string("the lower bound written is at most x") + kind, lower.right() <= x, describe);
            check(std::string("the upper bound written is at least the double above x") + kind,
                  upper.left() >= next, describe);
          }
        }
      }
    }
    interval::precision(saved_precision);
    return bounds;
  }
}

int main()
{
  // The snprintf() below writes the digits right: the checks of the test hold
  // with the C library, whatever GAOL does
  wrong_digits = false;
  write_and_read(", with the digits of the C library");
  if (snprintf_calls == 0) {
    std::printf("The calls of GAOL to snprintf() are not wrapped (a shared libgaol, or a C library without it): "
                "skipped\n");
    return 77;
  }

  // The digits after the 17th are zeros, as the msvcrt.dll writes them
  wrong_digits = true;
  const long bounds = write_and_read(", with the digits rounded to 17, then zeros");
  check("snprintf() of the test replaced digits by zeros, and bounds were read back",
        digits_replaced > 0 && bounds > 0, [&] {
          return std::to_string(digits_replaced) + " digits replaced, " + std::to_string(bounds) + " bounds";
        });
  return summary();
}
