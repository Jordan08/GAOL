/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_interval_io.cpp
  \brief  The input and the output of the intervals: operator>> and operator<<, their formats
          and precision, and the text of an interval (literal_text(), exact_string()).

  \author Goualard Frederic, NININ Jordan
  \date   2001-09-28
*/

#include "gaol/core/gaol_interval_internal.h"
#include "gaol/parser/gaol_parser.h"

#include <algorithm>

/*
  operator>> and operator<< of the intervals, their formats and precision,
  the text of an interval (operator std::string(), literal_text(),
  exact_string()) (GAOL v5: a part of gaol_interval_core.cpp, see
  gaol_interval_internal.h).
*/

namespace gaol_core {

  // Default format for output
  interval_format::format_t interval::output = interval_format::bounds;

  // Number of digits to print.
  std::streamsize interval::output_precision = 16;


  void interval::format(interval_format::format_t f)
  {
    output = f;
  }

  interval_format::format_t interval::format(void)
  {
    return output;
  }


  // Sets the failbit of is, without the std::ios_base::failure setstate()
  // throws when the exceptions of is include failbit: the error of a line
  // that is no interval is GAOL's exception. The bit stays set.
  static void set_failbit(istream& is)
  {
    try {
      is.setstate(std::ios_base::failbit);
    } catch (const std::ios_base::failure&) {
      // failbit is set
    }
  }

  /*
    Reads an interval from a line, after the blanks that precede it, line ends
    included, which std::ws skips as the reading of a double does, and under
    std::noskipws too, which keeps a double from skipping them (GAOL v5).
    GAOL read the line where the previous value stopped: over
    "1.5\n[1, 2]\n", "in >> d >> x" read the empty rest of the first line,
    and threw an exception, as while (in >> x) did over a file ending
    with an empty line. A blank line is now no line to read, and the intervals
    around it are read, as two numbers with a blank line between them are.

    Where no line is left, std::getline() fails, which sets failbit: I is left
    as it was and nothing is thrown, as for a double, so that while (is >> x)
    ends at the end of the input (GAOL v5). GAOL read the empty text then,
    threw an exception and emptied I, so that such a loop always ended
    with an exception. A line that is no interval sets failbit too, I becomes
    the empty set, and the exception of the reader is thrown:
    std::invalid_argument, or std::domain_error for a function called with an
    argument it does not take. A program reading on calls is.clear() first.

    The std::ws of libstdc++ before GCC 12 constructs no sentry (libc++, the
    library of Visual C++, and libstdc++ from GCC 12 on, construct one): it
    neither flushes the stream tied to is nor looks at the state of is, and
    reads the buffer whatever that state is. Used alone, it would crash there
    on an istream without buffer, consume the blanks of a stream that has
    failed, and leave a prompt written on cout unflushed until the user had
    typed the interval. The sentry of any extraction is therefore constructed
    first, without skipping (std::getline() does the same): it flushes the
    tied stream, and sets failbit where is is not good.
  */
  istream& operator >>(istream& is, interval& I)
  {
    std::string buffer;

    if (!istream::sentry(is,true)) {
      return is;
    }
    if (!std::getline(is >> std::ws,buffer)) {
      return is;
    }

    bool read;
    try {
      read = gaol_parse_string(buffer.data(), buffer.size(), I, parsing_names::gaol);
    } catch (...) { // An error the reader raised as it read the line
      I = interval::emptyset();
      set_failbit(is);
      throw;
    }
    if (!read) {
      std::string err_msg("Syntax error in expression of interval: ");
      err_msg += gaol_detail::text_in_message(buffer);
      I = interval::emptyset();
      set_failbit(is);
      gaol_ERROR(std::invalid_argument,err_msg.c_str());
    }
    return is;
  }

  /*
    The text of 10^exponent in the format of the digits of a bound: `fraction`
    digits after the point, none and no point if 0; in the scientific format
    d.ddde+XX, in the fixed one ddd.ddd, where a power of ten below the last
    digit is the last digit, 10^-fraction.
  */
  static std::string power_of_ten_text(int exponent, std::size_t fraction, bool scientific)
  {
    const std::string zeros(fraction, '0');
    if (scientific) {
      std::ostringstream suffix;
      suffix << 'e' << ((exponent < 0) ? '-' : '+') << std::setw(2) << std::setfill('0')
             << ((exponent < 0) ? -exponent : exponent);
      return std::string("1") + ((fraction > 0) ? "." + zeros : std::string()) + suffix.str();
    }
    if (exponent >= 0) {
      return "1" + std::string(static_cast<std::size_t>(exponent), '0')
             + ((fraction > 0) ? "." + zeros : std::string());
    }
    const std::size_t places = std::min(static_cast<std::size_t>(-exponent), fraction);
    if (places == 0) {
      return "1";
    }
    return "0." + std::string(places - 1, '0') + "1" + std::string(fraction - places, '0');
  }

  /*
    A number of the shape of text, in the fixed format or in the scientific
    one, that is on the side of the double `magnitude` asked for whatever the
    digits the C library wrote (GAOL v5): 0 toward zero, and away from it the
    least power of ten that is not below magnitude. Loose, up to ten times
    away from zero, and to 0 toward it, but outward, as the bound of a
    subnormal under denormals-are-zero in number_to_text(). It is the last
    resort of move_to_side().
  */
  static void outward_text(std::string& text, double magnitude, bool away, bool scientific)
  {
    const std::size_t last = scientific ? text.find_first_of("eE") : text.size();
    const std::size_t point = text.find('.');
    const std::size_t fraction = (point < last) ? last - point - 1 : 0;
    if (!away) {
      text = std::string("0") + ((fraction > 0) ? "." + std::string(fraction, '0') : std::string())
             + (scientific ? "e+00" : "");
      return;
    }
    // The decimal logarithm is within one of the exponent, so that the loop
    // makes a few turns at most; a power below the last digit is written as
    // the last digit, which is above a magnitude below it
    for (int exponent = static_cast<int>(std::ceil(std::log10(magnitude))) - 1; ; ++exponent) {
      std::string power = power_of_ten_text(exponent, fraction, scientific);
      if (gaol_detail::compare_number_with_double(power.c_str(), magnitude) >= 0) {
        text.swap(power);
        return;
      }
    }
  }

  /*
    Moves the last digit of text by one unit, away from zero or toward it,
    until the number it writes is on the side of the double `magnitude` asked
    for: at least it if `away`, at most it otherwise. text is a positive
    number written to nearest in the fixed format, or in the scientific one if
    `scientific`: one digit, the others after a point, and an exponent with
    its sign. A C library that wrote digits wrong by more than the four moves
    make up for (the msvcrt.dll of Windows, which MinGW-w64 calls for
    snprintf() when __USE_MINGW_ANSI_STDIO is 0, writes no more than 17
    significant digits right) left the text on the wrong side, a bound inside
    the interval: outward_text() is then written instead (GAOL v5).
  */
  static void move_to_side(std::string& text, double magnitude, bool away, bool scientific)
  {
    for (int moves = 0; ; ++moves) {
      const std::size_t last = scientific ? text.find_first_of("eE") : text.size(); // One past the mantissa
      const int sign = gaol_detail::compare_number_with_double(text.c_str(), magnitude);
      if (sign == 0 || (sign > 0) == away) {
        return;
      }
      if (moves == 4) {
        outward_text(text, magnitude, away, scientific);
        return;
      }
      std::size_t i = last;
      bool carry = true;
      while (carry && i > 0) {
        --i;
        if (text[i] == '.') {
          continue;
        }
        if (away) {
          carry = (text[i] == '9');
          text[i] = carry ? '0' : static_cast<char>(text[i] + 1);
        } else {
          carry = (text[i] == '0');
          text[i] = carry ? '9' : static_cast<char>(text[i] - 1);
        }
      }
      int exponent_move = 0;
      if (away && carry) {
        // 9.99 has become 0.00: 1.00 with the next exponent, or 10.00
        if (scientific) {
          text[0] = '1';
          exponent_move = 1;
        } else {
          text.insert(0, 1, '1');
        }
      } else if (!away && scientific && text[0] == '0') {
        // 1.00 has become 0.99: 9.99 with the previous exponent, which is
        // below the magnitude as well, 1.00 being it rounded to nearest
        text[0] = '9';
        exponent_move = -1;
      } else if (!away && text[0] == '0' && text.size() > 1 && text[1] != '.') {
        text.erase(0, 1); // 0999.99
      }
      if (exponent_move != 0) {
        const long e = std::strtol(text.c_str() + last + 1, NULL, 10) + exponent_move;
        std::ostringstream exponent;
        exponent << ((e < 0) ? '-' : '+') << std::setw(2) << std::setfill('0') << ((e < 0) ? -e : e);
        text.replace(last + 1, std::string::npos, exponent.str());
      }
    }
  }

  /*
    What GAOL takes from a stream to write a number: its flags, its precision
    and its locale, whose facet numpunct gives the decimal point and, if
    `grouped`, the grouping of the digits before it (GAOL v5). The text of an
    interval is made from these in a character string, rather than in
    std::ostringstreams, one for each interval and one for each bound: each
    built a locale and its cached facets, and destroyed them, a third of the
    work of writing an interval.
  */
  namespace {
  struct text_format {
    std::ios_base::fmtflags flags;
    std::streamsize precision;
    std::locale loc;
    const std::numpunct<char>& punct; // The facet of loc, which the copy keeps
    bool grouped;

    text_format(std::ios_base::fmtflags f, std::streamsize p, const std::locale& l, bool g = false)
      : flags(f), precision(p), loc(l), punct(std::use_facet<std::numpunct<char> >(loc)), grouped(g)
    {
    }
  };

  // How the text of a number is rounded: to nearest, as a stream writes a
  // double, or downward or upward for the bound of an interval
  enum text_rounding { text_nearest, text_downward, text_upward };
  } // namespace

  /*
    x written by a stream that has the flags, the precision and the locale of
    fmt, under the rounding direction asked: what number_to_text() leaves to
    the C library and the facets of the locale, whatever they are, as it
    cannot write it as the stream does. That is a NaN, the hexadecimal
    floating-point format, and a conversion of the C library that fails.
  */
  static std::string stream_text(double x, text_rounding rounding, const text_format& fmt)
  {
    std::ostringstream out;
    out.flags(fmt.flags);
    out.precision(fmt.precision);
    out.imbue(fmt.loc);
    // Zero has no digit to round, and the C runtime of Windows, asked to write
    // it in the upward direction, writes 0.1 (the point interval [0, 0] came
    // out as <0.0, 0.1>): it is written to nearest. The other cases keep the
    // direction of the bound, the hexadecimal format being rounded by the C
    // library when the stream limits its digits.
    // The guard sets the direction back, by an exception of the stream too
    // (std::bad_alloc): literal_text() and operator<<(), the only callers,
    // hold one already, and this function no longer counts on them (GAOL v5)
    {
      const rounding_guard rnd;
      if (x == 0.0 || rounding == text_nearest) {
        round_nearest();
      } else if (rounding == text_downward) {
        round_downward();
      }
      out << x;
    }
    return out.str();
  }

  /*
    The magnitude of a double written by the C library with `prec` digits after
    the point, in the fixed format, or in the scientific one, rounded to
    nearest: the same digits as the stream writes. printf follows the decimal
    point of the C locale, a comma under some, where a stream of C++ follows
    its own locale: the point is written as a '.' whatever the locale, from
    its position among the digits, which the precision gives.
  */
  static bool magnitude_text(std::string& text, double magnitude, bool fixed, int prec)
  {
    char buffer[512];
    const char *const format = fixed ? "%.*f" : "%.*e";
    int n;
    // The guard sets the direction back, by an exception of text.assign()
    // too (std::bad_alloc), as in stream_text() (GAOL v5)
    {
      const rounding_guard rnd;
      round_nearest();
      n = std::snprintf(buffer, sizeof buffer, format, prec, magnitude);
      if (n >= 0 && static_cast<std::size_t>(n) < sizeof buffer) {
        text.assign(buffer, static_cast<std::size_t>(n));
      } else if (n >= 0) {
        text.assign(static_cast<std::size_t>(n) + 1, '\0');
        n = std::snprintf(&text[0], text.size(), format, prec, magnitude);
        text.resize((n >= 0) ? static_cast<std::size_t>(n) : 0);
      }
    }
    if (n < 0 || text.empty()) {
      return false;
    }
    if (prec > 0) {
      // 3.14e+00 or 3.14: the point is where the digits before it end, and it
      // ends where the digits after it, of the precision, begin
      const std::size_t point = fixed ? text.find_first_not_of("0123456789") : 1;
      const std::size_t digits_end = fixed ? text.size() : text.find('e');
      if (point == std::string::npos || digits_end == std::string::npos
          || digits_end < point + static_cast<std::size_t>(prec) + 1) {
        return false;
      }
      const std::size_t length = digits_end - static_cast<std::size_t>(prec) - point;
      if (length != 1 || text[point] != '.') {
        text.replace(point, length, 1, '.');
      }
    }
    return true;
  }

  /*
    The digits text starts with, those before its decimal point, grouped as
    std::num_put groups those of a double: from the point leftward, each group
    as long as its character of grouping() says, the last one repeated, and
    no group past a size of 0, a negative one or CHAR_MAX, which the standard
    takes as unlimited (22.4.3.1.2). libstdc++ differs there for a facet a
    program defines: it repeats the size before a 0 ("\3\0" groups 1234567 as
    1,234,567, where libc++ and GAOL write 1234,567), and takes the sizes 128
    to 254 for negative ones where char is unsigned (ARM, ppc64le, s390x).
  */
  static void group_digits(std::string& text, const std::numpunct<char>& punct)
  {
    const std::string grouping = punct.grouping();
    std::size_t pos = text.find_first_not_of("0123456789");
    if (pos == std::string::npos) {
      pos = text.size();
    }
    std::size_t g = 0;
    while (g < grouping.size()) {
      const char size = grouping[g];
      if (size <= 0 || size == CHAR_MAX || pos <= static_cast<std::size_t>(size)) {
        break;
      }
      pos -= static_cast<std::size_t>(size);
      text.insert(pos, 1, punct.thousands_sep());
      if (g + 1 < grouping.size()) {
        ++g;
      }
    }
  }

  /*
    The text of a number x written as a stream with the flags, the precision
    and the locale of fmt would write it, and, for the bound of an interval,
    rounded downward, or upward, whatever the C library does. GAOL set the
    rounding direction and let the C library write x, but not every C library
    rounds its decimal conversions in the rounding direction: the C runtime of
    Windows rounds the magnitude, so that -2/3 rounded downward was written
    -0.6666, and musl on 64-bit ARM processors rounds to nearest whatever the
    direction (issue #3). The magnitude is now written rounding to nearest, in
    the fixed format or in the scientific one, and compared exactly with x:
    when it is on the wrong side of x, its last digit is moved by one. The
    general format is made from the scientific one, by the rules of printf's
    %g, rather than asked of the C library: glibc 2.31 writes 999999.5 with six
    digits and the zeros kept "1.e+06", whose last digit is not the sixth.
    The text of a zero, or of an infinity, is made the same way, without
    the comparison: "inf" is "INF" with the flag uppercase, but in the fixed
    format, whose conversion the standard has as %f (a stream of libc++, which
    uses %F, wrote "INF" there). With fmt.grouped, the
    digits before the point are grouped as the locale has them, as a stream
    groups those of a double: the midpoint and the radius of the width and
    center formats, both alike (GAOL v5: a stream wrote the midpoint, grouped,
    and GAOL the radius, which was not). The other texts are never grouped.
    Left to a stream, with the rounding direction set (stream_text()): what
    the text cannot be made of.
  */
  static std::string number_to_text(double x, text_rounding rounding, const text_format& fmt)
  {
    const std::ios_base::fmtflags floatfield = fmt.flags & std::ios_base::floatfield;
    // A subnormal under denormals-are-zero, which a program may set, compares
    // equal to 0. operator<< and intervalToText() clear that mode before
    // they write (GAOL v5), where GAOL can (x86, and ARM
    // with GCC and Clang): only where it cannot (ARM with Visual C++, the
    // program having set FZ itself) does a subnormal come here. The C
    // library may write it 0 under the mode (the snprintf called below did
    // for 5e-324 with MSYS2 CLANG64, gdtoa does), and GAOL wrote it to
    // nearest, below an upper bound: a bound is written as the double on its
    // outer side that no mode changes, the smallest normal double, 2^-1022,
    // away from zero, and 0 toward it, loose but outward (GAOL v5).
    // A number written to nearest is written by a stream, as before. Tested
    // before the hexadecimal format, which a stream writes too.
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    if (x == 0.0 && (bits << 1) != 0) {
      if (rounding == text_nearest) {
        return stream_text(x, text_nearest, fmt);
      }
      const bool away = (rounding == text_upward) == !std::signbit(x);
      const double smallest_normal = std::numeric_limits<double>::min();
      return number_to_text(away ? (std::signbit(x) ? -smallest_normal : smallest_normal) : 0.0, rounding, fmt);
    }
    if (!(x == x) || floatfield == (std::ios_base::fixed | std::ios_base::scientific)) {
      return stream_text(x, rounding, fmt);
    }

    const bool fixed = (floatfield == std::ios_base::fixed);
    const bool general = (floatfield == 0);
    const bool showpoint = ((fmt.flags & std::ios_base::showpoint) != 0);
    std::string text;
    if (x == GAOL_INFINITY || x == -GAOL_INFINITY) {
      text = ((fmt.flags & std::ios_base::uppercase) && !fixed) ? "INF" : "inf";
    } else {
      // A stream writes 6 digits for a negative precision: so are the numbers
      // written to nearest and the zeros. A bound that is not 0 takes it as 0
      // in the general format, 1 digit, and as 6 in the fixed and scientific
      // ones, as the stream that wrote it did
      const bool as_stream = (rounding == text_nearest || x == 0.0);
      const std::streamsize precision = (as_stream && fmt.precision < 0) ? 6 : fmt.precision;
      // %g takes a precision of 0 as 1, and writes that many significant digits
      const std::streamsize digits = (general && precision <= 0) ? 1 : precision;
      std::streamsize prec = general ? digits - 1 : precision;
      if (prec < 0) {
        prec = 6;
      } else if (prec > std::numeric_limits<int>::max() / 4) {
        prec = std::numeric_limits<int>::max() / 4;
      }
      const double magnitude = std::fabs(x);
      if (!magnitude_text(text, magnitude, fixed, static_cast<int>(prec))) {
        // snprintf failed (a precision its buffer cannot hold), or its point
        // was not found: the digits of a stream of the C locale, to nearest,
        // as GAOL wrote them before, then moved outward as above. Never those
        // of the C library under the directed rounding, which the C runtime of
        // Windows and musl do not honour (issue #3)
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out.setf(fixed ? std::ios_base::fixed : std::ios_base::scientific, std::ios_base::floatfield);
        out.precision(prec);
        {
          const rounding_guard rnd; // as in stream_text() (GAOL v5)
          round_nearest();
          out << magnitude;
        }
        text = out.str();
      }
      // The magnitude written has to be at least that of x when rounding away
      // from zero, and at most that of x otherwise
      if (rounding != text_nearest && x != 0.0) {
        move_to_side(text, magnitude, (rounding == text_upward) == (x > 0.0), !fixed);
      }

      if (general) {
        // d.ddde+XX as %g writes it: without an exponent when XX is from -4 to
        // the precision, and without the zeros ending its fractional part,
        // unless showpoint
        const std::size_t e = text.find('e');
        const long exponent = std::strtol(text.c_str() + e + 1, NULL, 10);
        std::string mantissa = text.substr(0, e);
        mantissa.erase(1, (mantissa.size() > 1) ? 1 : 0); // The digits
        std::string tail;
        if (exponent < -4 || exponent >= digits) {
          tail = text.substr(e);
          mantissa.insert(1, 1, '.');
        } else if (exponent >= 0) {
          mantissa.insert(static_cast<std::size_t>(exponent) + 1, 1, '.');
        } else {
          mantissa.insert(0, "0." + std::string(static_cast<std::size_t>(-exponent - 1), '0'));
        }
        if (!showpoint) {
          mantissa.erase(mantissa.find_last_not_of('0') + 1);
          if (mantissa[mantissa.size() - 1] == '.') {
            mantissa.erase(mantissa.size() - 1);
          }
        }
        text = mantissa + tail;
      } else if (precision == 0 && showpoint) {
        text.insert(fixed ? text.size() : text.find('e'), 1, '.');
      }

      const char point = fmt.punct.decimal_point();
      for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '.') {
          text[i] = point;
        } else if (text[i] == 'e' && (fmt.flags & std::ios_base::uppercase)) {
          text[i] = 'E';
        }
      }
      // After the point is in place: the separator of the groups may be '.'
      if (fmt.grouped) {
        group_digits(text, fmt.punct);
      }
    }
    if (std::signbit(x)) { // -0 is written -0, as a stream does
      text.insert(0, 1, '-');
    } else if (fmt.flags & std::ios_base::showpos) {
      text.insert(0, 1, '+');
    }
    return text;
  }

  static std::string bound_to_text(double x, bool upward, const text_format& fmt)
  {
    return number_to_text(x, upward ? text_upward : text_downward, fmt);
  }

  /*
    The interval [l, r] as two bounds rounded outward, [l, r], or as [a], the
    literal of IEEE 1788-2015 for a point (12.11), for a point interval whose
    double the text writes exactly (GAOL v5). The text of the lower bound being
    at most it, and that of the upper bound at least it, two equal texts are
    the double itself, which the reader reads back as the point; the other
    point intervals are written as any other interval, [0.1, 0.1000000000000001]
    for interval(0.1), which is read back as an interval enclosing it. A zero
    is written exactly, and a point interval of zero, [-0, +0], [+0, +0] or
    [-0, -0], is written [0], the text of +0: the three are the set {0}, which
    [0] and [-0] are both read back as, and the text of interval::zero() is
    then the same whatever the build ([-0, +0] with the SSE2 intervals,
    [+0, +0] with the FPU ones), and the hexadecimal format writes it [0x0p+0].
    The bounds are compared by their bits: under denormals-are-zero,
    which a program may set, a subnormal compares equal to 0, and l == r and
    l == 0.0 would have [0, 5e-324] written [0]. (Under that mode,
    bound_to_text() took a subnormal bound for 0 as well, and wrote it
    rounded to nearest rather than outward, so that two equal texts need not
    have been the double: with 1 digit, [22u] (u = 5e-324) was written
    [1e-322], which is read as [20u, 21u]. operator<< and intervalToText()
    clear that mode before they write, where GAOL can, and number_to_text()
    writes such a bound outward where it cannot: GAOL v5.)
    fmt has the C locale, whose decimal point is '.', the reader's: operator<<
    writes the bounds format with it whatever the locale of the stream (GAOL
    v5). Under a locale writing a decimal comma, the reader took the comma of
    [0,5, 2,5] for the one between two bounds and refused the text, and would
    have read [-2,5] for interval(-2.5) as the interval [-2, 5], [12,5] as the
    empty set and [0,] (a zero in the fixed format with the showpoint flag and
    no digit) as [0, +oo]; GAOL wrote that point [-2,5, -2,5], which it
    refused. A point is still written alone between the brackets only when
    the decimal point of fmt is '.'.
    GAOL wrote every point interval <a, b>, the text of its double rounded
    downward and upward, which the reader takes for two numbers that are the
    same double only: <0.1, 0.1000000000000001> for interval(0.1) was refused,
    and textToInterval(intervalToText(interval(0.1))) was the empty set. The
    reader still takes the angles, which are no literal of the standard.
  */
  static void display_bounds(double l, double r, std::string& out, const text_format& fmt)
  {
    // A quiet comparison, which raises no invalid-operation exception on the
    // NaN bounds of the empty set (GAOL v5)
    if (!gaol_detail::quiet_less_equal(l, r)) {
      out += "[empty]";
    } else {
      std::uint64_t lbits, rbits;
      std::memcpy(&lbits, &l, sizeof lbits);
      std::memcpy(&rbits, &r, sizeof rbits);
      const bool zero = ((lbits << 1) == 0 && (rbits << 1) == 0); // +0 or -0, both
      const std::string left = bound_to_text(zero ? 0.0 : l, false, fmt);
      const std::string right = bound_to_text(zero ? 0.0 : r, true, fmt);
      out += '[';
      out += left;
      if (!((zero || (lbits == rbits && left == right)) && fmt.punct.decimal_point() == '.')) {
        out += ", ";
        out += right;
      }
      out += ']';
    }
  }

  /*
    The text of x of gaol_ieee1788::intervalToText() (gaol/parser/gaol_parser.h):
    the bounds of x as display_bounds() writes them, with the flags a new
    stream starts with, the digits of the intervals and the C locale
  */
  std::string literal_text(const interval& x)
  {
    const text_format fmt(std::ios_base::skipws | std::ios_base::dec, interval::precision(), std::locale::classic());
    std::string out;
    // The guard, rather than GAOL_RND_ENTER() and GAOL_RND_LEAVE():
    // display_bounds() allocates, and a failed allocation left the direction
    // upward, which GAOL_PRESERVE_ROUNDING promises to set back (GAOL v5)
    const rounding_guard rnd;
    round_upward_if_needed();
    display_bounds(x.left(), x.right(), out, fmt);
    return out;
  }


  /*
    A bound written in the hexadecimal-significand form of IEEE 1788-2015
    (13.4.1), which textToInterval() reads back exactly (GAOL v5).

    The recovery requirement of 13.4 asks that writing an interval and reading
    it again give the same bounds. GAOL wrote the sixteen hexadecimal digits of
    each double instead, which is no interval literal at all: the parser
    refused them, and the note of 13.4.1 gives that very form as the one that
    fails the readability test. The form is the one printf("%a") of the glibc
    writes: as many digits as the value needs, "0x1.999999999999ap-4",
    "0x0.0000000000001p-1022" for a subnormal, "0x0p+0" for zero, and the
    lexer reads it.
    It is written from the bits of the double rather than by printf("%a"),
    which follows the decimal point of the C locale: under a locale writing a
    decimal comma, it wrote "0x1,999999999999ap-4", which cannot be read
    back (GAOL v5).
  */
  static void write_hexa_bound(double x, char *buf, std::size_t n)
  {
    if (x == GAOL_INFINITY) {
      std::snprintf(buf, n, "inf");
      return;
    }
    if (x == -GAOL_INFINITY) {
      std::snprintf(buf, n, "-inf");
      return;
    }
    std::uint64_t bits;
    std::memcpy(&bits, &x, sizeof bits);
    const int biased = static_cast<int>((bits >> 52) & 0x7ff);
    std::uint64_t fraction = bits & ((static_cast<std::uint64_t>(1) << 52) - 1);
    // 1.f 2^(e-1023) for a normal double, 0.f 2^-1022 for a subnormal one, 0
    const int leading = (biased == 0) ? 0 : 1;
    const int exponent = (biased == 0) ? ((fraction == 0) ? 0 : -1022) : biased - 1023;
    // The 13 hexadecimal digits of the fraction, without the zeros ending them
    char digits[14];
    for (int i = 12; i >= 0; --i, fraction >>= 4) {
      digits[i] = "0123456789abcdef"[fraction & 0xf];
    }
    int nb_digits = 13;
    while (nb_digits > 0 && digits[nb_digits - 1] == '0') {
      --nb_digits;
    }
    digits[nb_digits] = '\0';
    std::snprintf(buf, n, "%s0x%d%s%sp%+d", (bits >> 63) ? "-" : "", leading,
                  (nb_digits > 0) ? "." : "", digits, exponent);
  }

  /*
    A point interval is written [a], as in the decimal format, and a point
    interval of zero [0x0p+0], whatever the signs of its bounds, the three
    being the set {0} (GAOL v5: it was written with its two bounds,
    [0x1p+2, 0x1p+2], and [-0x0p+0, 0x0p+0] for interval::zero() with the
    SSE2 intervals). The bounds are compared by their bits, which
    denormals-are-zero does not change.
  */
  std::string exact_string(const interval& I)
  {
    if (I.is_empty()) {
      return "[empty]";
    }
    const double l = I.left(), r = I.right();
    std::uint64_t lbits, rbits;
    std::memcpy(&lbits, &l, sizeof lbits);
    std::memcpy(&rbits, &r, sizeof rbits);
    char lo[64], hi[64];
    if ((lbits << 1) == 0 && (rbits << 1) == 0) {
      return "[0x0p+0]";
    }
    write_hexa_bound(l, lo, sizeof lo);
    if (lbits == rbits) {
      return std::string("[") + lo + "]";
    }
    write_hexa_bound(r, hi, sizeof hi);
    return std::string("[") + lo + ", " + hi + "]";
  }

  /*
    text into os as one item, padded to the width of os with its fill, to the
    left, to the right, or inside (std::internal). os pads a string to the left
    or to the right only: for std::internal, the fill is put where a number of
    os has it (GAOL v5).
  */
  static void write_text(ostream& os, std::string& text)
  {
    // std::internal puts the fill after the sign of a number and its 0x
    // prefix, before its digits. The text of the width and center formats
    // starts with the sign of the midpoint, if it has one, and that of the
    // agreeing digits with the sign of the bounds, and the fill goes after
    // it, as std::internal padded the midpoint alone before the interval was
    // written as a whole. The other texts start with '[', before which the
    // fill goes, as with std::right, and std::complex writes its parentheses.
    const std::streamsize width = os.width();
    if ((os.flags() & std::ios_base::adjustfield) == std::ios_base::internal
        && width > static_cast<std::streamsize>(text.size())) {
      std::size_t at = (!text.empty() && (text[0] == '-' || text[0] == '+')) ? 1 : 0;
      if (text.size() > at + 1 && text[at] == '0' && (text[at + 1] == 'x' || text[at + 1] == 'X')) {
        at += 2;
      }
      if (at > 0) {
        text.insert(at, static_cast<std::size_t>(width) - text.size(), os.fill());
      }
    }
    os << text;
  }

  /*
    The text is made in a character string, from the flags and the locale of
    os and the precision of interval::precision(), then written into os at once
    (GAOL v5): os keeps its precision, and its width (std::setw) and
    adjustment apply to the whole interval. GAOL set the precision of os to
    interval::precision() and left it so, the doubles written afterwards
    getting 16 digits, and wrote the interval piece by piece, std::setw
    padding its '[' only.

    The text is not written in a std::ostringstream of its own, whose locale
    and cached facets, built and destroyed for each interval and each bound,
    took a third of the work of writing an interval. os is only read, and is
    never given another precision or width for the time of the writing: the
    threads of a program writing to std::cout at once would race on them.
  */
  ostream& operator<<(ostream& os, const interval& I)
  {
    //    double l = ((I.left()==0.0) ? 0.0  : I.left()); // Avoids printing -0
    //    double r = ((I.right()==0.0) ? 0.0 : I.right());  // Avoids printing -0
    std::string text;

    // The guard, rather than GAOL_RND_PRESERVE() and GAOL_RND_RESTORE(): the
    // construction of the text may fail (std::bad_alloc), and a program
    // reading the exception found the direction left as this function had
    // set it, to nearest (GAOL v5). The check of the direction, as in
    // intervalToText(), clears the modes that flush the subnormals to zero,
    // which the guard sets back with GAOL_PRESERVE_ROUNDING: under
    // denormals-are-zero, a subnormal bound was 0 for the comparisons and
    // was written to nearest, [22*2^-1074] as [1e-322] with 1 digit, and the
    // C library writes it 0 (gdtoa of FreeBSD and macOS, the Debug runtime
    // of Visual C++, issue #68; GAOL v5)
    const rounding_guard rnd;
	round_upward_if_needed();

    double l = I.left(), r = I.right();
    const interval_format::format_t format = interval::format();

    switch (format) {
    case interval_format::bounds: { // Display in the form "[ l, r ]"
      // The bounds format, and the agreeing format where it writes bounds,
      // in the C locale, whose decimal point is the reader's (GAOL v5); the
      // width and center formats, which are for the eye, in the locale of os
      const text_format fmt(os.flags(), interval::precision(), std::locale::classic());
      display_bounds(l,r,text,fmt);
      break;
    }
    case interval_format::hexa: // The exact text representation of 13.4
      text = exact_string(I);
      break;
    case interval_format::width:  // Display in the form "c (+/- w)"
    case interval_format::center: // Display in the form "c"
      if (I.is_empty()) {
        text = "[empty]";
      } else {
        /*
          The midpoint c and the radius w of IEEE 1788-2015 (12.12.8),
          midpoint() and rad() (GAOL v5): [c-w, c+w] contains the interval, w
          being the smallest double that makes it so, and +oo for an
          unbounded interval, whose midpoint is 0 or plus or minus the
          largest double. c is written rounded to nearest, and w upward, so
          that a radius that is not 0 is never written 0. GAOL wrote (l+r)/2
          and (r-l)/2, both rounded to nearest: c and w did not contain the
          interval ([1, 1+2^-52] was "1 (+/- 1.11e-16)"), w was 0 for
          [0, 5e-324], l+r overflowed for [1e308, 1.7e308], and the center of
          a point interval was written rounded upward (0.1 was
          0.1000000000000001).
          A point interval has no radius: it is written as its center, which
          tells it from an interval too narrow for the digits of its center.
          These formats are for the eye: c has the digits of the precision, so
          that c (+/- w) written need not contain the interval, where the
          bounds format does. A radius that made the digits of c enclose it
          would show 0.1 (+/- 5.6e-18) for the point interval 0.1, and a
          radius worth the resolution of the digits of c for any interval
          narrower than that: the formats could no longer show that an
          interval is narrower than its digits (the manual, Output format).
        */
        // The center format writes no radius: midpoint() alone
        double c, w = 0.0;
        if (format == interval_format::width) {
          I.mid_rad(c, w);
        } else {
          c = I.midpoint();
        }
        // The decimal point of the locale of os, and its grouping of the
        // digits, for c and w alike; a midpoint 0 is written 0, as the bounds
        // format writes a zero, where midpoint() gives -0 for
        // [-2^-1073, 2^-1074]: its bits, which denormals-are-zero leaves alone
        const text_format shown(os.flags(), interval::precision(), os.getloc(), true);
        std::uint64_t cbits;
        std::memcpy(&cbits, &c, sizeof cbits);
        if ((cbits << 1) == 0) {
          c = 0.0;
        }
        text = number_to_text(c, text_nearest, shown);
        if (format == interval_format::width && w != 0.0) {
          // The radius is never negative, and has no sign under showpos,
          // which is for the midpoint: "+2 (+/- 1)" (GAOL v5: "+2 (+/- +1)")
          text_format radius(shown);
          radius.flags &= ~std::ios_base::showpos;
          text += " (+/- ";
          text += bound_to_text(w, true, radius);
          text += ')';
        }
      }
      break;
    case interval_format::agreeing:
      if (I.is_empty()) {
        text = "[empty]";
      } else {
        // Each bound is written with the digits of the precision and the flag
        // showpoint, in the locale of the program, as a stream of its own,
        // whose flags are the ones a stream starts with, would write it: the
        // flags and the locale of os do not apply here
        const text_format shown(std::ios_base::skipws | std::ios_base::dec | std::ios_base::showpoint,
                                interval::precision(), std::locale());
        const char point = shown.punct.decimal_point();
        const std::string lb = bound_to_text(l, false, shown), rb = bound_to_text(r, true, shown);
        std::size_t i = 0;
        while (i < lb.length() && i < rb.length() && lb[i] == rb[i]) {
          ++i;
        }
        /*
          The digits both bounds start with are written once, before the rest
          of each, only where they line up and say something: two bounds
          finite, not 0 and of the same sign, written with the same number of
          digits before the point and the same exponent, that share their
          first digit that is not 0. The other intervals are written as in
          the bounds format (GAOL v5). GAOL tested r > 10 l, true for every
          interval with a negative bound or 0, which were all written with
          their bounds, and false for [1, 10], written 1~[., 0.], the 1 of 10
          taken for the 1 of 1, or for [1, 2], written ~[1., 2.] with no
          digit shared, and for zeros, written ~[-0., 0.] for
          interval::zero() with the SSE2 intervals where the bounds format
          writes [0].
        */
        std::uint64_t lbits, rbits;
        std::memcpy(&lbits, &l, sizeof lbits);
        std::memcpy(&rbits, &r, sizeof rbits);
        const std::size_t le = lb.find_first_of("eE"), re = rb.find_first_of("eE");
        const bool line_up = (lbits << 1) != 0 && (rbits << 1) != 0
          && l > -GAOL_INFINITY && r < GAOL_INFINITY && std::signbit(l) == std::signbit(r)
          && lb.find(point) == rb.find(point)
          && ((le == std::string::npos) ? std::string() : lb.substr(le))
             == ((re == std::string::npos) ? std::string() : rb.substr(re))
          && lb.find_first_of("123456789") < i;
        if (!line_up) {
          const text_format fmt(os.flags(), interval::precision(), std::locale::classic());
          display_bounds(l,r,text,fmt);
        } else {
          // The characters both bounds start with, then what is left of each,
          // without the zeros ending it (GAOL v5: GAOL dropped from both
          // bounds the characters after the last one of the left bound that is
          // not a zero, and wrote [1.25, 1.2567] "1.25~[, ]"; zeros ending an
          // exponent are not dropped)
          text = lb.substr(0, i);
          if (i < lb.length() || i < rb.length()) {
            const std::string *bounds[2] = { &lb, &rb };
            text += "~[";
            for (int k = 0; k < 2; ++k) {
              const std::string& b = *bounds[k];
              std::size_t end = b.length();
              if (b.find_first_of("eE") == std::string::npos && b.find(point) != std::string::npos) {
                while (end > i && end > b.find(point) + 1 && b[end - 1] == '0') {
                  --end;
                }
              }
              text += (end > i) ? b.substr(i, end - i) : std::string("0");
              text += (k == 0) ? ", " : "]";
            }
          }
        }
      }
    }
    // No double is computed from here on: the text is written before the
    // guard sets the direction back
    write_text(os, text);
    return os;
  }

  /*
    The name of a state of overlap() as Table 10.7 spells it, written as a
    string is: the width of os applies to it, and no format of interval does
    (GAOL v5). A value that is no state, which a cast makes, is written
    overlapping_state(k), k in decimal whatever the locale of os, which
    std::to_string does not have.
  */
  ostream& operator<<(ostream& os, overlapping_state s)
  {
    static const char* const names[16] = {
      "bothEmpty", "firstEmpty", "secondEmpty", "before", "meets", "overlaps", "starts", "containedBy",
      "finishes", "equals", "finishedBy", "contains", "startedBy", "overlappedBy", "metBy", "after" };
    const unsigned k = static_cast<unsigned>(s);
    if (k < 16) {
      return os << names[k];
    }
    return os << "overlapping_state(" + std::to_string(k) + ")";
  }


  interval::operator std::string() const
  {
    // Not named output, the format of the intervals written, which Visual C++
    // warned that it hid (C4458, GAOL v5)
    std::ostringstream text;
    text.precision(interval::precision());
    text << *this;
    return text.str();
  }

  std::streamsize interval::precision(void)
  {
    return output_precision;
  }

  std::streamsize interval::precision(std::streamsize n)
  {
    std::streamsize old = output_precision;
    output_precision = n;
    return old;
  }

} // namespace gaol_core
