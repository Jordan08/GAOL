// The checks of the programs that itf1788.py generates from the test cases of
// ITF1788: each test case is one call of an operation of gaol_ieee1788 (the
// names of IEEE 1788-2015), whose result is compared with the one the .itl
// file gives, and written as one line on the standard output, separated by
// tabs:
//   status  file  testcase  line  operation  signal  got  expected  test case
// status is pass, fail, threw (the operation threw an exception, whose
// message is in got), notprovided or notapplicable (the case was not run: got
// says why). The bounds are written in C hexadecimal (%a), a zero bound as
// 0x0p+0 whatever its sign, "[empty]" for the empty set, and the numbers in
// C hexadecimal too. The last line, "end  file  n", tells itf1788.py report
// that the program went through its n cases.
//
// The comparison is that of ITF1788 (the result must be the tightest one the
// file gives), with two precisions: an interval is a set, so that a zero bound
// equals a zero of either sign, while a number result must have the sign of
// zero 12.12.8 of IEEE 1788-2015 gives (-0 for inf, +0 for the other numeric
// functions) where its exact value is 0, and a NaN equals a NaN.
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#ifndef ITF1788_GAOL_H
#define ITF1788_GAOL_H

#include <cmath>
#include <cstdio>
#include <exception>
#include <limits>
#include <string>
#include <utility>

#include <gaol/gaol.h>
#include <gaol/gaol_ieee1788.h>

namespace itf {

  const double inf = std::numeric_limits<double>::infinity();
  const double nan = std::numeric_limits<double>::quiet_NaN();

  //! [lo, hi] with the doubles of the literal: the numeric constructor
  inline gaol_ieee1788::interval iv(double lo, double hi) { return gaol_ieee1788::interval(lo, hi); }

  inline std::string hex(double d)
  {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%a", d);
    return buf;
  }

  // A bound 0 written 0x0p+0, whatever its sign: an interval is a set, and
  // the SSE2 and the FPU intervals of GAOL do not keep the same zeros
  inline std::string text(const gaol_ieee1788::interval& x)
  {
    if (x.is_empty()) {
      return "[empty]";
    }
    return "[" + hex(x.left() == 0.0 ? 0.0 : x.left()) + ", " + hex(x.right() == 0.0 ? 0.0 : x.right()) + "]";
  }

  inline std::string text(double d) { return hex(d); }
  inline std::string text(bool b) { return b ? "true" : "false"; }

  inline bool same(const gaol_ieee1788::interval& a, const gaol_ieee1788::interval& b)
  {
    if (a.is_empty() || b.is_empty()) {
      return a.is_empty() && b.is_empty();
    }
    return a.left() == b.left() && a.right() == b.right();
  }

  inline bool same(double a, double b)
  {
    if (std::isnan(a) || std::isnan(b)) {
      return std::isnan(a) && std::isnan(b);
    }
    return a == b && std::signbit(a) == std::signbit(b);
  }

  inline bool same(bool a, bool b) { return a == b; }

  // The tabs and the line ends of a field (an exception's message) as spaces
  inline std::string field(std::string s)
  {
    for (std::string::size_type i = 0; i < s.size(); ++i) {
      if (s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
        s[i] = ' ';
      }
    }
    return s;
  }

  class recorder {
  public:
    explicit recorder(const char* file) : file_(file), count_(0), line_(0), ok_(true), threw_(false) {}

    void testcase(const char* name) { testcase_ = name; }

    //! Starts the case of line `line` of the file: the operation, the exception
    //! it should signal ("-" for none) and its text in the file
    void begin(int line, const char* op, const char* signal, const char* source)
    {
      line_ = line;
      op_ = op;
      signal_ = signal;
      source_ = source;
      ok_ = true;
      threw_ = false;
      got_.clear();
      expected_.clear();
    }

    //! Compares one output of the operation with the one the file gives
    template <class T>
    void eq(const T& got, const T& expected)
    {
      if (!got_.empty()) {
        got_ += " ";
        expected_ += " ";
      }
      got_ += text(got);
      expected_ += text(expected);
      ok_ = ok_ && same(got, expected);
    }

    //! The same for a number whose zero may have either sign: the midpoint of
    //! an interval whose exact midpoint is not 0 but rounds to it, as
    //! -2^-1075, which IEEE 754 rounds to -0 and the .itl file writes 0.0
    void eq_any_zero(double got, double expected)
    {
      if (got == 0.0 && expected == 0.0) {
        got = expected;
      }
      eq(got, expected);
    }

    //! In a catch block: the operation threw the exception being handled
    void threw()
    {
      threw_ = true;
      try {
        throw;
      } catch (const std::exception& e) {
        got_ = e.what();
      } catch (...) {
        got_ = "an exception that is not a std::exception";
      }
    }

    void end() { write(threw_ ? "threw" : (ok_ ? "pass" : "fail"), got_, expected_); }

    //! A case not run: status is notprovided or notapplicable, why says why
    void skip(int line, const char* op, const char* signal, const char* source, const char* status, const char* why)
    {
      begin(line, op, signal, source);
      write(status, why, "-");
    }

    //! The last line, which tells the runner that the program did not stop before it
    int finish()
    {
      std::printf("end\t%s\t%d\n", file_, count_);
      return std::fflush(stdout) == 0 ? 0 : 1;
    }

  private:
    void write(const char* status, const std::string& got, const std::string& expected)
    {
      ++count_;
      std::printf("%s\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%s\n", status, file_, testcase_.c_str(), line_, op_.c_str(),
                  signal_.c_str(), field(got).c_str(), field(expected).c_str(), source_.c_str());
    }

    const char* file_;
    int count_;
    std::string testcase_;
    int line_;
    std::string op_, signal_, source_, got_, expected_;
    bool ok_, threw_;
  };

} // namespace itf

#endif
