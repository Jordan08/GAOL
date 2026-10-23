/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the rounding direction of the code using GAOL.
 *
 * GAOL computes its bounds with the rounding direction upward. By default,
 * each operation sets it upward when it is not, and leaves it upward; with
 * GAOL_PRESERVE_ROUNDING 1, each operation also restores the direction
 * it found (see gaol/gaol_fpu.h).
 *
 * Every operation of GAOL's interface is run on several operands with the
 * rounding direction of the calling code upward, to nearest, downward and
 * toward zero, and on x86 with the x87 unit and the SSE instructions in
 * different directions. Whatever the direction, it has to give the same
 * result, written exactly, as with the direction upward: a bound computed in
 * another direction differs, as does one a compiler computes after a change
 * of direction that the source code writes after its computation. It has to
 * leave the direction as it found it, or upward (as it found it only, with
 * GAOL_PRESERVE_ROUNDING). After it, the bounds of a product and a sum have to
 * be the tightest ones. Powers with a subnormal result, which CORE-MATH rounds
 * itself in the direction fegetround() gives, have to be the tightest
 * enclosures whatever the direction, with the x87 unit and the SSE
 * instructions differing too (GAOL v5). On x86 and ARM, the same holds with
 * the modes that flush the subnormals to zero set, flush-to-zero and
 * denormals-are-zero of the SSE instructions, FZ of ARM (what a program linked
 * with -Ofast gets from crtfastmath.o, and a plug-in built so): the bounds of
 * the operations with a subnormal operand or result are the tightest ones, and
 * the modes are cleared after them, or restored with GAOL_PRESERVE_ROUNDING;
 * and every operation that checks the rounding direction gives, with a mode
 * set before it, what it gives with the modes cleared, and clears or restores
 * the modes (GAOL v5); so do the functions that make no check, the relations,
 * the constructor from two bounds, max(), abs(), floor()..., which leave the
 * modes as they found them (GAOL v5, point Q of TODO.md). At the end,
 * gaol::cleanup() has to set back the direction the first gaol::init()
 * found (GAOL v5).
 *
 * The rest of the floating-point environment is checked too: the exceptions
 * stay masked, and an empty interval is told empty, with interval::emptyset(),
 * and every operation computes with an empty operand, without raising the
 * invalid-operation exception, which kills a program that enabled it (GAOL v5).
 * Nor do a NaN the program gives, the products with a zero and an infinite
 * bound, pow with an exponent of extreme magnitude, cancel_minus() and
 * cancel_plus() with bounds next to the largest double (point 78 of TODO.md)
 * and the relations in loops a compiler vectorizes raise it, nor the midpoint
 * of [DBL_MAX] the overflow exception; every operation
 * keeps the exception flags and masks of the program (GAOL v5, point D.24 of
 * TODO.md).
 *
 * On 64-bit ARM with GCC or Clang, whatever the intervals (NEON or FPU), the
 * control register FPCR is read and written directly, with mrs and msr, and on
 * 32-bit ARM with GCC and a floating-point unit FPSCR, with vmrs and vmsr (GAOL
 * v5): its bits other than the rounding direction and the modes that flush the
 * subnormals to zero (the default-NaN mode, FZ16, the enables of the traps)
 * are the ones the program had after every operation; the bounds of an
 * operation run right after a write of FPCR by the program (no isb between
 * them) are the tightest ones; and the sums, differences, squares, square
 * roots and operations with a double of intervals with infinite bounds, and of
 * the empty set, raise neither the invalid-operation nor the division-by-zero
 * exception. The difference, the quotient, the remainder, the square, the
 * square root, the inverse and their forms with a double and as compound
 * assignments give the tightest bounds after the program changes the
 * direction, as the product and the sum do (GAOL v5).
 *
 * On a 32-bit x86 processor, the control word of the x87 unit and MXCSR are
 * read and written directly (GAOL v5): what gaol_core::x86_*() decide to write,
 * and the values, are checked on every platform, with two variables for the
 * registers, in every state of the two rounding fields and the two flush modes;
 * and, with GCC, Clang or Visual C++ for 32-bit x86, the registers themselves,
 * which the tests read and write apart from the library, after operations and
 * right before them, in the six states where the units agree or differ.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#include "gaol_tests.h"

#include <algorithm>
#include <climits>
#include <cstring>
#include <exception>
#include <functional>
#include <initializer_list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// A program that enables the floating-point exceptions of the processor
// (feenableexcept(), a GNU extension of glibc's <fenv.h>), run in a child
// process (fork(), POSIX) that a signal kills without stopping the test: glibc
// only. Elsewhere the traps are not tested, and the test says so.
#if defined(__GLIBC__) && defined(_GNU_SOURCE) && defined(__unix__)
#  include <csignal>
#  include <cerrno>
#  include <sys/resource.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  define GAOL_TESTS_TRAPS 1
#else
#  define GAOL_TESTS_TRAPS 0
#endif

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  include <xmmintrin.h>
#  define GAOL_TESTS_SSE 1
#  define SSE_DIRECTION(d) static_cast<unsigned>(d)
#else
#  define SSE_DIRECTION(d) 0u
#endif

// A 32-bit x86 processor, where the library reads and writes the control word
// of the x87 unit and MXCSR directly (GAOL v5), and so do the tests below, with
// the assembly of each compiler
#if defined(GAOL_TESTS_SSE) && (defined(__i386__) || defined(_M_IX86)) \
    && (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER))
#  define GAOL_TESTS_X86_32 1
#else
#  define GAOL_TESTS_X86_32 0
#endif

// mingw-w64 on x86-64 (GAOL v5, point 110 of TODO.md): from its version 13,
// fegetround() joins the field RC of the x87 unit and that of MXCSR by OR, and
// fesetround() clears the exception flags of MXCSR when it changes it. The
// control word of the x87 unit is then read apart from fegetround(), as on a
// 32-bit x86 processor, and is part of the state the checks compare: where the
// two units differ, fegetround() gives the same value before and after an
// operation that left one of them in the other's direction. The checks of the
// exception flags run from each of the directions here too, and around
// gaol::cleanup() and gaol::restore_rounding()
#if defined(GAOL_TESTS_SSE) && defined(__x86_64__) && defined(__MINGW32__) && (defined(__GNUC__) || defined(__clang__))
#  define GAOL_TESTS_X87_STATE 1
#else
#  define GAOL_TESTS_X87_STATE 0
#endif

// 64-bit ARM with GCC or Clang: FPCR, the control register holding the rounding
// direction, the modes that flush the subnormals to zero, the default-NaN mode
// and the enables of the traps, is read with mrs and written with msr, as
// tests/gaol_tests.h does for the flush modes (GAOL v5)
#if defined(__aarch64__) && defined(__GNUC__)
#  define GAOL_TESTS_FPCR 1
#else
#  define GAOL_TESTS_FPCR 0
#endif
// The same for the sequence of a write of the control register and an
// operation, which 32-bit ARM processors with a floating-point unit have too:
// FPSCR, read with vmrs and written with vmsr (GAOL v5)
#ifndef GAOL_TESTS_FPCR_SEQUENCE
#  if GAOL_TESTS_FPCR || (defined(__arm__) && !defined(__aarch64__) && defined(__GNUC__) && defined(__ARM_FP) && (__ARM_FP & 8))
#    define GAOL_TESTS_FPCR_SEQUENCE 1
#  else
#    define GAOL_TESTS_FPCR_SEQUENCE 0
#  endif
#endif

using namespace gaol;
using namespace gaol_tests;

namespace
{
  // An operation of GAOL computed before main(), in the initialization of a
  // static object: it sets the rounding direction upward. GAOL has to
  // initialize itself before, so that the first gaol::init() finds the
  // direction the program started with: with GCC and Clang, the constructor of
  // the static library GAOL ran after those of the program, except with
  // MinGW-w64, and gaol::cleanup() left the direction upward (GAOL v5).
  const interval computed_before_main = interval(0.1, 0.3) * interval(1.5, 2.5);

  // The results of the operations, written exactly
  std::string S(const interval& x) { return hex(x); }
  std::string S(double x) { return hex(x); }
  std::string S(bool b) { return b ? "true" : "false"; }
  std::string S(int n) { return std::to_string(n); }
  std::string S(unsigned long long n) { return std::to_string(n); }

#if GAOL_TESTS_FLUSH
  /*
    The results of a function run with a mode flushing the subnormals to zero
    set, kept as they are and written once the modes are cleared: the C
    library writes the doubles (%a), and gdtoa (FreeBSD, macOS) writes a
    subnormal 0 under denormals-are-zero, which reads it as 0 (GAOL v5). The
    doubles are compared by their bits, the empty set by its NaN bounds.
  */
  struct Kept
  {
    std::vector<double> numbers;
    std::string text; // what needs no double written
  };

  Kept kept(const interval& x) { Kept k; k.numbers.push_back(x.left()); k.numbers.push_back(x.right()); return k; }
  Kept kept(const interval& x, const interval& y)
  {
    Kept k = kept(x);
    k.numbers.push_back(y.left());
    k.numbers.push_back(y.right());
    return k;
  }
  Kept kept(double x) { Kept k; k.numbers.push_back(x); return k; }
  Kept kept(bool b) { Kept k; k.text = b ? "true" : "false"; return k; }
  Kept kept(unsigned long long n) { Kept k; k.text = std::to_string(n); return k; }
  Kept kept_text(const std::string& s) { Kept k; k.text = s; return k; }

  bool same_kept(const Kept& a, const Kept& b)
  {
    return a.numbers.size() == b.numbers.size() && a.text == b.text
      && (a.numbers.empty() || std::memcmp(a.numbers.data(), b.numbers.data(), a.numbers.size()*sizeof(double)) == 0);
  }

  std::string written(const Kept& k)
  {
    std::string s;
    for (double x : k.numbers) {
      s += hex(x) + " ";
    }
    return s + k.text;
  }
#endif // GAOL_TESTS_FLUSH

  struct Operation
  {
    const char *name;
    std::string (*run)(const interval& x, const interval& y);
  };

  // Powers x^y with a result below the smallest normal double, which CORE-MATH
  // rounds by itself, in the direction fegetround() gives (GAOL v5). Their
  // tightest enclosure [lo, hi] is written as multiples of the smallest
  // subnormal, 2^-1074, and was computed apart from GAOL, with mpmath at 600
  // bits: exp(y*log(x)), rounded down and up to a multiple of 2^-1074. The
  // exact values are 1458662373659.29, 567863.26 and 1.30 of them: with a
  // fractional part below one half, rounding them to nearest, rather than
  // upward, gives a value below the exact one.
  struct SubnormalPower
  {
    double x, y;
    long long lo, hi;
  };

  const SubnormalPower subnormal_powers[] = {
    { 0x1.4a1249ba9b6d5p-1, 0x1.97f9e85175cf7p+10, 1458662373659LL, 1458662373660LL },
    { 0x1.0d00c0deaf87fp-2, 0x1.117ee54c23de1p+9, 567863LL, 567864LL },
    { 0x1.999999999999ap-1, 0x1.a0df226893p+11, 1LL, 2LL },
  };

  // A multiple of 2^-1074, exact in every rounding direction
  double subnormal(long long multiple) { return static_cast<double>(multiple) * 0x1p-1074; }

  // Writes x in the format f
  std::string write(const interval& x, interval_format::format_t f)
  {
    const interval_format::format_t saved = interval::format();
    interval::format(f);
    std::ostringstream s;
    s << x;
    interval::format(saved);
    return s.str();
  }

  // A rounding direction of the calling code: the one fesetround() sets, then
  // the one of the SSE instructions, where there are some
  struct Direction
  {
    const char *name;
    int fenv;
    unsigned sse;
  };

  const Direction directions[] = {
    { "upward", FE_UPWARD, SSE_DIRECTION(_MM_ROUND_UP) },
    { "to nearest", FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_NEAREST) },
    { "downward", FE_DOWNWARD, SSE_DIRECTION(_MM_ROUND_DOWN) },
    { "toward zero", FE_TOWARDZERO, SSE_DIRECTION(_MM_ROUND_TOWARD_ZERO) },
#if GAOL_TESTS_SSE
    { "upward, and to nearest for SSE", FE_UPWARD, SSE_DIRECTION(_MM_ROUND_NEAREST) },
    { "to nearest, and upward for SSE", FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_UP) },
#endif
  };

  void set(const Direction& d)
  {
    std::fesetround(d.fenv);
#if GAOL_TESTS_SSE
    _mm_setcsr((_mm_getcsr() & ~static_cast<unsigned int>(_MM_ROUND_MASK)) | d.sse);
#endif
  }

  /*
    An interval built in the function from its arguments (GAOL v5), so that its
    bounds are registers when width() is inlined there, and the call through
    a pointer that keeps the compiler from inlining the function in the loop of
    the test. GCC subtracted them before the check of the rounding direction
    wrote the control register, in the program's direction, with the control
    register of 64-bit and 32-bit ARM read and written by asm statements,
    which it does not order with the operations on registers (bug 34678):
    width([1, 2^53 + 2]) was 2^53 below the upward direction
  */
  double built_width(double a, double b) { return interval(a, b).width(); }
  double (*volatile width_of_arguments)(double, double) = built_width;

  // l*(1 - ratio) + r*ratio rounded upward, from operands read from memory:
  // the cut point of bisect()
  double upward_cut(double l, double r, double ratio)
  {
    const int saved = std::fegetround();
    std::fesetround(FE_UPWARD);
    volatile double vl = l, vr = r, vq = ratio;
    volatile double c = vl*(1.0 - vq) + vr*vq;
    const double kept = c;
    std::fesetround(saved);
    return kept;
  }

#if GAOL_TESTS_X87_STATE
  // The field RC of the control word of the x87 unit, as the hardware has it
  // (0x0c00 masks it), read apart from fegetround()
  unsigned x87_direction()
  {
    unsigned short cw;
    __asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
    return cw & 0x0c00u;
  }

  // The five flags of IEEE 754 raised, and the six of MXCSR (the denormal-operand one included)
  void raise_all_flags()
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    std::feraiseexcept(FE_ALL_EXCEPT);
    _mm_setcsr(_mm_getcsr() | 0x3fu);
  }
#endif

  // The rounding direction as fegetround() and the SSE control register give it,
  // and with mingw-w64 on x86-64 the x87 unit's too
  struct State
  {
    int fenv;
    unsigned sse;
#if GAOL_TESTS_X87_STATE
    unsigned x87 = 0u;
#endif

    bool operator==(const State& s) const
    {
      return fenv == s.fenv && sse == s.sse
#if GAOL_TESTS_X87_STATE
        && x87 == s.x87
#endif
        ;
    }
  };

  State state()
  {
    State s;
    s.fenv = std::fegetround();
#if GAOL_TESTS_SSE
    s.sse = _mm_getcsr() & _MM_ROUND_MASK;
#else
    s.sse = 0u;
#endif
#if GAOL_TESTS_X87_STATE
    s.x87 = x87_direction();
#endif
    return s;
  }

  std::string text(const State& s)
  {
    return "fegetround() " + std::to_string(s.fenv) + ", SSE rounding bits " + std::to_string(s.sse)
#if GAOL_TESTS_X87_STATE
      + ", x87 rounding bits " + std::to_string(s.x87)
#endif
      ;
  }

  // Products and sums computed in a loop that changes the rounding direction
  // before each of them. A compiler may read the rounding direction once for
  // the whole loop, when it does not know that fesetround() changes it: Clang 18
  // did so with a read of MXCSR. GAOL would then not set the direction upward.
  void products_and_sums_in_a_loop(const std::vector<double>& a, const std::vector<double>& b,
                                   std::vector<interval>& p, std::vector<interval>& s)
  {
    const std::size_t nb_directions = sizeof(directions)/sizeof(directions[0]);
    for (std::size_t i = 0; i < a.size(); ++i) {
      set(directions[i % nb_directions]);
      p[i] = interval(a[i]) * interval(b[i]);
      s[i] = interval(a[i]) + interval(b[i]);
    }
  }

  /*
    The control registers of a 32-bit x86 processor (GAOL v5): the control
    word of the x87 unit, whose field RC (bits 11 and 10) is its rounding
    direction, and MXCSR, whose field RC (bits 14 and 13) is the one of the SSE
    instructions, and whose bits FTZ (15) and DAZ (6) flush the subnormals to
    zero. gaol_core::x86_*() (gaol/gaol_fpu.h) decide what to write to them, and
    the values, from the registers they read, whatever the processor: they take
    the registers as a type with three members, the processor's on x86 and the
    two variables below here, so that every platform checks them in all the
    64 states of the two fields and the two modes, where a processor runs six
    of them at most (the states of directions[]). The registers are written
    from the fields below, with the numbers the hardware gives them, and never
    with the FE_* of <fenv.h> (0x0400 and 0x0800 with glibc, 0x0100 and 0x0200
    with the C runtime of Windows).
  */
  struct SimulatedRegisters
  {
    unsigned int cw, csr;
    mutable int reads;
    int x87_writes, mxcsr_writes;
    bool wide; // a value written to the control word that has more than 16 bits

    SimulatedRegisters(unsigned int x87_cw, unsigned int mxcsr)
      : cw(x87_cw), csr(mxcsr), reads(0), x87_writes(0), mxcsr_writes(0), wide(false) {}

    void read_control(unsigned int& x87_cw, unsigned int& mxcsr) const
    {
      ++reads;
      x87_cw = cw;
      mxcsr = csr;
    }

    void write_x87_control(unsigned int x87_cw)
    {
      wide = wide || x87_cw > 0xffffu;
      cw = x87_cw;
      ++x87_writes;
    }

    void write_mxcsr(unsigned int mxcsr)
    {
      csr = mxcsr;
      ++mxcsr_writes;
    }
  };

  // What gaol_core::x86_get_state() fills, and the rounding_state of a 32-bit x86 processor has
  struct SimulatedState
  {
    unsigned int x87, sse, flush;
  };

  // The fields of the two registers: a rounding direction, 0 to nearest, 1 downward, 2 upward and 3 toward zero,
  // in each unit, and the two flush modes
  struct X86Fields
  {
    unsigned int x87_rc, sse_rc;
    bool ftz, daz;
  };

  // The bits that the fields do not hold: of the control word, the precision (bits 9 and 8), the infinity control
  // (12) and the exception masks (5 to 0); of MXCSR, the exception flags (5 to 0) and the masks (12 to 7)
  const unsigned int x87_others[] = { 0x037fu, 0x027fu, 0x0000u, 0x133fu };
  const unsigned int mxcsr_others[] = { 0x1f80u, 0x1fbfu, 0x0000u, 0x1fa0u, 0x0081u };

  unsigned int x87_register(unsigned int other, const X86Fields& f)
  {
    return other | (f.x87_rc << 10);
  }

  unsigned int mxcsr_register(unsigned int other, const X86Fields& f)
  {
    return other | (f.sse_rc << 13) | (f.ftz ? 1u << 15 : 0u) | (f.daz ? 1u << 6 : 0u);
  }

  X86Fields x86_fields(unsigned int number)
  {
    X86Fields f;
    f.x87_rc = number & 3u;
    f.sse_rc = (number >> 2) & 3u;
    f.ftz = ((number >> 4) & 1u) != 0u;
    f.daz = ((number >> 5) & 1u) != 0u;
    return f;
  }

  const unsigned int nb_x86_fields = 64;

  bool x86_upward(const X86Fields& f)
  {
    return f.x87_rc == 2u && f.sse_rc == 2u && !f.ftz && !f.daz;
  }

  std::string x86_text(const X86Fields& f)
  {
    return "x87 " + std::to_string(f.x87_rc) + ", SSE " + std::to_string(f.sse_rc) + (f.ftz ? ", FTZ" : "") + (f.daz ? ", DAZ" : "");
  }

  std::string x86_hex(unsigned int x)
  {
    char buffer[16];
    std::snprintf(buffer, sizeof buffer, "0x%04x", x);
    return buffer;
  }

  // The decisions of gaol_core::x86_*() on the simulated registers, all of
  // them in every state of the registers, with other bits of the registers that
  // must not change, and the exception flags that an operation raises between
  // its start and its end, which must not be lost
  void x86_registers_simulated()
  {
    const unsigned int mxcsr_flags = 0x003fu;
    const std::string where = " of the registers of a 32-bit x86 processor";
    for (unsigned int i = 0; i < nb_x86_fields; ++i) {
      const X86Fields f = x86_fields(i);
      for (unsigned int x87_other : x87_others) {
        for (unsigned int mxcsr_other : mxcsr_others) {
          const auto describe = [&] {
            return x86_text(f) + ", control word " + x86_hex(x87_register(x87_other, f))
                 + ", MXCSR " + x86_hex(mxcsr_register(mxcsr_other, f));
          };
          const unsigned int cw = x87_register(x87_other, f), csr = mxcsr_register(mxcsr_other, f);

          // Whether the direction is upward in both units, with no flush mode
          check("x86_is_upward()" + where, gaol_core::x86_is_upward(cw, csr) == x86_upward(f), describe);

          // The state saved: the rounding fields as each register has them, and the flush modes
          {
            SimulatedRegisters r(cw, csr);
            SimulatedState s;
            gaol_core::x86_get_state(r, s);
            check("x86_get_state()" + where,
                  s.x87 == (f.x87_rc << 10) && s.sse == (f.sse_rc << 13)
                  && s.flush == ((f.ftz ? 1u << 15 : 0u) | (f.daz ? 1u << 6 : 0u)),
                  [&] { return describe() + ": " + x86_hex(s.x87) + ", " + x86_hex(s.sse) + ", " + x86_hex(s.flush); });
            check("x86_get_state() reads once, and writes nothing" + where,
                  r.reads == 1 && r.x87_writes == 0 && r.mxcsr_writes == 0 && r.cw == cw && r.csr == csr, describe);
          }

          // The start of an operation, from the registers read at once, and from the state saved: the direction
          // upward in both units, the flush modes cleared and nothing else changed, and the register that holds
          // none of that left alone
          X86Fields upward_fields = f;
          upward_fields.x87_rc = 2u;
          upward_fields.sse_rc = 2u;
          upward_fields.ftz = false;
          upward_fields.daz = false;
          const unsigned int cw_upward = x87_register(x87_other, upward_fields);
          const unsigned int csr_upward = mxcsr_register(mxcsr_other, upward_fields);
          const int x87_writes = f.x87_rc != 2u ? 1 : 0;
          const int mxcsr_writes = (f.sse_rc != 2u || f.ftz || f.daz) ? 1 : 0;
          for (int with_state = 0; with_state < 2; ++with_state) {
            SimulatedRegisters r(cw, csr);
            // The reads: one of the check, and one more of the writes, which the registers that are upward spare
            int reads_wanted = x86_upward(f) ? 1 : 2;
            if (with_state != 0) {
              SimulatedState s;
              gaol_core::x86_get_state(r, s);
              r.reads = 0;
              gaol_core::x86_enter_upward(r, s);
              reads_wanted = x86_upward(f) ? 0 : 1;
            } else {
              gaol_core::x86_round_upward_if_needed(r);
            }
            const char *start = with_state != 0 ? "x86_enter_upward()" : "x86_round_upward_if_needed()";
            check(std::string(start) + ": upward in both units, the modes cleared, nothing else changed" + where,
                  r.cw == cw_upward && r.csr == csr_upward && !r.wide,
                  [&] { return describe() + ": " + x86_hex(r.cw) + ", " + x86_hex(r.csr); });
            check(std::string(start) + ": the registers written that differ only" + where,
                  r.x87_writes == x87_writes && r.mxcsr_writes == mxcsr_writes,
                  [&] { return describe() + ": " + std::to_string(r.x87_writes) + " and " + std::to_string(r.mxcsr_writes) + " writes"; });
            check(std::string(start) + ": no more reads than the registers written need" + where, r.reads == reads_wanted,
                  [&] { return describe() + ": " + std::to_string(r.reads) + " reads"; });
          }

          // The end of an operation: what was found is back, the exception flags that the operation raised are kept
          {
            SimulatedRegisters r(cw, csr);
            SimulatedState s;
            gaol_core::x86_get_state(r, s);
            gaol_core::x86_enter_upward(r, s);
            r.csr |= mxcsr_flags;
            r.reads = 0;
            r.x87_writes = 0;
            r.mxcsr_writes = 0;
            gaol_core::x86_leave(r, s);
            check("x86_leave(): what the operation found is back, with the exception flags it raised" + where,
                  r.cw == cw && r.csr == (csr | mxcsr_flags) && !r.wide,
                  [&] { return describe() + ": " + x86_hex(r.cw) + ", " + x86_hex(r.csr); });
            check("x86_leave(): the registers written that differ only, none when the direction found was upward" + where,
                  r.x87_writes == x87_writes && r.mxcsr_writes == mxcsr_writes,
                  [&] { return describe() + ": " + std::to_string(r.x87_writes) + " and " + std::to_string(r.mxcsr_writes) + " writes"; });
            check("x86_leave(): no read when the direction found was upward, one for the writes otherwise" + where,
                  r.reads == (x86_upward(f) ? 0 : 1),
                  [&] { return describe() + ": " + std::to_string(r.reads) + " reads"; });
          }
        }
      }
    }

    // set_rounding() and set_rounding_and_flush_modes(), from each state of the registers to each state saved
    const unsigned int x87_base = 0x037fu, mxcsr_base = 0x1fa0u;
    for (unsigned int i = 0; i < nb_x86_fields; ++i) {
      const X86Fields now = x86_fields(i);
      for (unsigned int j = 0; j < nb_x86_fields; ++j) {
        const X86Fields saved = x86_fields(j);
        const auto describe = [&] { return "from " + x86_text(now) + " to " + x86_text(saved); };
        SimulatedState s;
        s.x87 = saved.x87_rc << 10;
        s.sse = saved.sse_rc << 13;
        s.flush = (saved.ftz ? 1u << 15 : 0u) | (saved.daz ? 1u << 6 : 0u);

        // The directions only: the flush modes stay as they are
        SimulatedRegisters r(x87_register(x87_base, now), mxcsr_register(mxcsr_base, now));
        gaol_core::x86_set_rounding(r, s);
        X86Fields expected = now;
        expected.x87_rc = saved.x87_rc;
        expected.sse_rc = saved.sse_rc;
        check("x86_set_rounding(): the directions set back, the flush modes and the other bits as they were" + where,
              r.cw == x87_register(x87_base, expected) && r.csr == mxcsr_register(mxcsr_base, expected) && !r.wide, describe);
        check("x86_set_rounding(): the registers written that differ only" + where,
              r.x87_writes == (now.x87_rc != saved.x87_rc ? 1 : 0) && r.mxcsr_writes == (now.sse_rc != saved.sse_rc ? 1 : 0), describe);

        // The directions and the flush modes
        SimulatedRegisters q(x87_register(x87_base, now), mxcsr_register(mxcsr_base, now));
        gaol_core::x86_set_rounding_and_flush_modes(q, s);
        check("x86_set_rounding_and_flush_modes(): the directions and the flush modes set back, the other bits as they were" + where,
              q.cw == x87_register(x87_base, saved) && q.csr == mxcsr_register(mxcsr_base, saved) && !q.wide, describe);
        check("x86_set_rounding_and_flush_modes(): the registers written that differ only" + where,
              q.x87_writes == (now.x87_rc != saved.x87_rc ? 1 : 0)
              && q.mxcsr_writes == ((now.sse_rc != saved.sse_rc || now.ftz != saved.ftz || now.daz != saved.daz) ? 1 : 0),
              describe);
      }
    }
  }

#if GAOL_TESTS_X86_32
  /*
    The control word of the x87 unit, read and written apart from the library
    (GAOL v5): the registers that gaol_core::x86_registers reads and writes,
    which the checks below read and write themselves, in the six states that the
    tests have for the two units, with the fields as the hardware has them (see
    SimulatedRegisters). Visual C++ for x86 and GCC and Clang do not take the
    same assembly.
  */
  unsigned int read_x87_word()
  {
    unsigned short cw;
#  if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
#  else
    __asm fnstcw cw
#  endif
    return cw;
  }

  void write_x87_word(unsigned int word)
  {
    unsigned short cw = static_cast<unsigned short>(word);
#  if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__ ("fldcw %0" : : "m" (cw) : "memory");
#  else
    __asm fldcw cw
#  endif
  }

  // The directions of directions[], as the fields of the two registers: the
  // four in which the units agree, upward in the x87 unit and to nearest in
  // the SSE instructions, and the reverse
  const X86Fields register_directions[] = {
    { 2u, 2u, false, false }, { 0u, 0u, false, false }, { 1u, 1u, false, false },
    { 3u, 3u, false, false }, { 2u, 0u, false, false }, { 0u, 2u, false, false },
  };

  // The field RC of the control word, and the fields of MXCSR, set as asked and nothing else
  void write_x86_fields(const X86Fields& f, unsigned int flush)
  {
    write_x87_word((read_x87_word() & ~0x0c00u) | (f.x87_rc << 10));
    _mm_setcsr((_mm_getcsr() & ~(0x6000u | 0x8040u)) | (f.sse_rc << 13) | flush);
  }

  // The bits of the registers that no direction and no flush mode has: the other bits of the control word, the
  // exception masks and the reserved bits of MXCSR, which the operations leave as they found them, and the
  // exception flags, which they only add to
  const unsigned int x87_not_direction = ~0x0c00u;
  const unsigned int mxcsr_not_direction = ~(0x6000u | 0x8040u | 0x003fu);

  struct RegisterOperation
  {
    const char *name;
    interval (*run)(const interval& x, const interval& y);
  };

  const RegisterOperation register_operations[] = {
    { "x + y", [](const interval& x, const interval& y) { return x + y; } },
    { "x - y", [](const interval& x, const interval& y) { return x - y; } },
    { "x * y", [](const interval& x, const interval& y) { return x * y; } },
    { "x / y", [](const interval& x, const interval& y) { return x / y; } },
    { "sqr(x)", [](const interval& x, const interval&) { return sqr(x); } },
    { "sqrt(x)", [](const interval& x, const interval&) { return sqrt(x); } },
    { "exp(x)", [](const interval& x, const interval&) { return exp(x); } },
    { "sin(x)", [](const interval& x, const interval&) { return sin(x); } },
    { "pow(x, 3)", [](const interval& x, const interval&) { return gaol::pow(x, 3); } },
    { "atan2(x, y)", [](const interval& x, const interval& y) { return atan2(x, y); } },
  };

  // The sums, the differences and the products that write the registers from their own
  // operands right before them, in the six states in turn, with and without a flush mode
  void x86_sequence_in_a_loop(const std::vector<double>& a, const std::vector<double>& b, std::vector<interval>& sum,
                              std::vector<interval>& product, unsigned int flush)
  {
    const std::size_t nb = sizeof(register_directions)/sizeof(register_directions[0]);
    for (std::size_t i = 0; i < a.size(); ++i) {
      write_x86_fields(register_directions[i % nb], flush);
      sum[i] = interval(a[i]) + interval(b[i]);
      product[i] = interval(a[i]) * interval(b[i]);
    }
  }
#endif // GAOL_TESTS_X86_32

  /*
    The operations other than the product and the sum, after the caller
    changes the direction (GAOL v5): the difference, the quotient, the
    remainder, the square, the square root and the inverse, each of them with
    intervals, with a double and as a compound assignment, are run on operands
    from the random doubles of the tests, and each result is compared with the
    tightest enclosure of the exact result, computed apart with exact
    arithmetic: the bounds of the four corners of the operands (the square
    root of a bound being told by its square). The intervals of the operations
    that divide have a divisor of one sign, 0 outside it, and the square root
    is also run on bounds of any sign. A bound computed in another direction
    than upward is one double away from the tightest one, where the exact
    result is not a double, which the random mantissas make almost certain.
  */
  enum class Formula { sum, difference, product, quotient, square, root, inverse };

  struct TightCase
  {
    const char *name;
    Formula formula;
    bool swapped;     // the formula applies to y then x: d - x, d / x
    bool mixed_signs; // x and y of any signs, else each of one sign
    bool with_double; // y is the point [d]
    interval (*run)(const interval& x, const interval& y);
  };

  const TightCase tight_cases[] = {
    { "x + y", Formula::sum, false, true, false, [](const interval& x, const interval& y) { return x + y; } },
    { "x - y", Formula::difference, false, true, false, [](const interval& x, const interval& y) { return x - y; } },
    { "x * y", Formula::product, false, true, false, [](const interval& x, const interval& y) { return x * y; } },
    { "x / y", Formula::quotient, false, false, false, [](const interval& x, const interval& y) { return x / y; } },
    { "x % y", Formula::quotient, false, false, false, [](const interval& x, const interval& y) { return x % y; } },
    { "x += y", Formula::sum, false, true, false, [](const interval& x, const interval& y) { interval z(x); z += y; return z; } },
    { "x -= y", Formula::difference, false, true, false, [](const interval& x, const interval& y) { interval z(x); z -= y; return z; } },
    { "x *= y", Formula::product, false, true, false, [](const interval& x, const interval& y) { interval z(x); z *= y; return z; } },
    { "x /= y", Formula::quotient, false, false, false, [](const interval& x, const interval& y) { interval z(x); z /= y; return z; } },
    { "x %= y", Formula::quotient, false, false, false, [](const interval& x, const interval& y) { interval z(x); z %= y; return z; } },
    { "x + d", Formula::sum, false, true, true, [](const interval& x, const interval& y) { return x + y.left(); } },
    { "d + x", Formula::sum, false, true, true, [](const interval& x, const interval& y) { return y.left() + x; } },
    { "x - d", Formula::difference, false, true, true, [](const interval& x, const interval& y) { return x - y.left(); } },
    { "d - x", Formula::difference, true, true, true, [](const interval& x, const interval& y) { return y.left() - x; } },
    { "x * d", Formula::product, false, true, true, [](const interval& x, const interval& y) { return x * y.left(); } },
    { "d * x", Formula::product, false, true, true, [](const interval& x, const interval& y) { return y.left() * x; } },
    { "x / d", Formula::quotient, false, false, true, [](const interval& x, const interval& y) { return x / y.left(); } },
    { "d / x", Formula::quotient, true, false, true, [](const interval& x, const interval& y) { return y.left() / x; } },
    { "x % d", Formula::quotient, false, false, true, [](const interval& x, const interval& y) { return x % y.left(); } },
    { "d % x", Formula::quotient, true, false, true, [](const interval& x, const interval& y) { return y.left() % x; } },
    { "x += d", Formula::sum, false, true, true, [](const interval& x, const interval& y) { interval z(x); z += y.left(); return z; } },
    { "x -= d", Formula::difference, false, true, true, [](const interval& x, const interval& y) { interval z(x); z -= y.left(); return z; } },
    { "x *= d", Formula::product, false, true, true, [](const interval& x, const interval& y) { interval z(x); z *= y.left(); return z; } },
    { "x /= d", Formula::quotient, false, false, true, [](const interval& x, const interval& y) { interval z(x); z /= y.left(); return z; } },
    { "x %= d", Formula::quotient, false, false, true, [](const interval& x, const interval& y) { interval z(x); z %= y.left(); return z; } },
    { "sqr(x)", Formula::square, false, true, false, [](const interval& x, const interval&) { return sqr(x); } },
    { "pow(x, 2)", Formula::square, false, true, false, [](const interval& x, const interval&) { return pow(x, 2); } },
    { "sqrt(x) of a positive x", Formula::root, false, false, false, [](const interval& x, const interval&) { return sqrt(x); } },
    { "sqrt(x) of any x", Formula::root, false, true, false, [](const interval& x, const interval&) { return sqrt(x); } },
    { "inverse(y)", Formula::inverse, false, false, false, [](const interval&, const interval& y) { return inverse(y); } },
    { "y.inverse()", Formula::inverse, false, false, false, [](const interval&, const interval& y) { return y.inverse(); } },
  };

  // The operands of a case: x and y from two random doubles each, as one-signed
  // intervals unless the case takes any, x positive for the square root; y is
  // the point [d] for the operations with a double
  void tight_operands(Random& random, const TightCase& c, interval& x, interval& y)
  {
    double a1 = random(-30, 30), a2 = random(-30, 30);
    double b1 = random(-30, 30), b2 = random(-30, 30);
    if (!c.mixed_signs) {
      if (c.formula == Formula::root) {
        a1 = std::fabs(a1);
        a2 = std::fabs(a2);
      } else {
        a2 = ((a1 < 0.0) == (a2 < 0.0)) ? a2 : -a2;
      }
      b2 = ((b1 < 0.0) == (b2 < 0.0)) ? b2 : -b2;
    }
    x = hull(a1, a2);
    y = c.with_double ? interval(b1) : hull(b1, b2);
  }

  // The exact bounds of the result of the formula, on the bounds of x and y
  // (those of y alone for the inverse, of x alone for the square)
  void exact_bounds(Formula f, const interval& x, const interval& y, Exact& lo, Exact& hi)
  {
    const Dyadic xl = dyadic(x.left()), xr = dyadic(x.right()), yl = dyadic(y.left()), yr = dyadic(y.right());
    switch (f) {
      case Formula::sum:
        lo = exact(xl + yl);
        hi = exact(xr + yr);
        break;
      case Formula::difference:
        lo = exact(xl - yr);
        hi = exact(xr - yl);
        break;
      case Formula::product: {
        const std::vector<Exact> p = { exact(xl*yl), exact(xl*yr), exact(xr*yl), exact(xr*yr) };
        lo = min(p);
        hi = max(p);
        break;
      }
      case Formula::quotient: { // y holds no zero
        const std::vector<Exact> q = { quotient(xl, yl), quotient(xl, yr), quotient(xr, yl), quotient(xr, yr) };
        lo = min(q);
        hi = max(q);
        break;
      }
      case Formula::square: {
        const std::vector<Exact> p = { exact(xl*xl), exact(xr*xr) };
        lo = (x.left() <= 0.0 && x.right() >= 0.0) ? exact(0.0) : min(p);
        hi = max(p);
        break;
      }
      case Formula::inverse: { // y holds no zero
        const Dyadic one = dyadic(1.0);
        const std::vector<Exact> q = { quotient(one, yl), quotient(one, yr) };
        lo = min(q);
        hi = max(q);
        break;
      }
      case Formula::root:
        break;
    }
  }

  // Whether r is the tightest enclosure of the square roots of the part of x
  // in [0, +oo), the empty set where x < 0: a lower bound l is the tightest one
  // if l*l <= a < next(l)*next(l), an upper bound u if u*u >= b > previous(u)*previous(u),
  // told with exact products (the bounds of x are not zero, but for 0 itself)
  bool is_tightest_root(const interval& r, const interval& x)
  {
    if (x.right() < 0.0) {
      return r.is_empty();
    }
    if (r.is_empty()) {
      return false;
    }
    const Dyadic a = dyadic(std::max(x.left(), 0.0)), b = dyadic(x.right());
    const double l = r.left(), u = r.right();
    const double l_next = next_double(l), u_previous = previous_double(u);
    return l >= 0.0 && compare(dyadic(l)*dyadic(l), a) <= 0 && compare(dyadic(l_next)*dyadic(l_next), a) > 0
        && compare(dyadic(u)*dyadic(u), b) >= 0 && compare(dyadic(u_previous)*dyadic(u_previous), b) < 0;
  }

  bool is_tightest_result(const TightCase& c, const interval& r, const interval& x, const interval& y)
  {
    if (c.formula == Formula::root) {
      return is_tightest_root(r, x);
    }
    if (r.is_empty()) {
      return false;
    }
    Exact lo, hi;
    if (c.swapped) {
      exact_bounds(c.formula, y, x, lo, hi);
    } else {
      exact_bounds(c.formula, x, y, lo, hi);
    }
    return is_tightest_enclosure(r, lo, hi);
  }

  // The operation of a case on the operands i = 0, 1... , the direction
  // being changed by fesetround() before each of them, in the directions in turn
  void tight_cases_in_a_loop(const TightCase& c, const std::vector<interval>& x, const std::vector<interval>& y,
                             std::vector<interval>& r)
  {
    const std::size_t nb_directions = sizeof(directions)/sizeof(directions[0]);
    for (std::size_t i = 0; i < x.size(); ++i) {
      set(directions[i % nb_directions]);
      r[i] = c.run(x[i], y[i]);
    }
  }

  std::string run(const Operation& op, const interval& x, const interval& y)
  {
    try {
      return op.run(x, y);
    } catch (const std::exception& e) {
      return std::string("exception: ") + e.what();
    } catch (...) {
      return "exception";
    }
  }

  // An interval the compiler knows nothing about: it would else compute at
  // compile time that the empty sets below are empty, and the comparison of
  // their NaN bounds, which is what raises the exception, would never run
  interval opaque(const interval& x)
  {
    return gaol_core::rnd_keep(x);
  }

  // An interval computed by make() from operands the compiler does not know
  struct NamedInterval
  {
    const char *name;
    interval (*make)();
  };

  // The empty sets of the checks below, each computed in a way of its own:
  // the constant, an interval whose bounds are in the wrong order, a function
  // outside its domain, disjoint intervals. None of them compares a NaN, so
  // that each is computed without an exception with the invalid-operation
  // exception enabled.
  const NamedInterval empty_sets[] = {
    { "interval::emptyset()", [] { return opaque(interval::emptyset()); } },
    { "interval(3, 2)", [] { return opaque(interval(gaol_core::rnd_keep(3.0), gaol_core::rnd_keep(2.0))); } },
    { "sqrt([-2, -1])", [] { return opaque(sqrt(interval(gaol_core::rnd_keep(-2.0), gaol_core::rnd_keep(-1.0)))); } },
    { "log([-2, -1])", [] { return opaque(log(interval(gaol_core::rnd_keep(-2.0), gaol_core::rnd_keep(-1.0)))); } },
    { "[1, 2] & [3, 4]", [] {
        return opaque(interval(gaol_core::rnd_keep(1.0), gaol_core::rnd_keep(2.0)) & interval(gaol_core::rnd_keep(3.0), gaol_core::rnd_keep(4.0))); } },
    { "[3, 4] & [1, 2]", [] {
        return opaque(interval(gaol_core::rnd_keep(3.0), gaol_core::rnd_keep(4.0)) & interval(gaol_core::rnd_keep(1.0), gaol_core::rnd_keep(2.0))); } },
  };

  // The nonempty intervals is_empty() has to tell nonempty as well
  const NamedInterval nonempty_sets[] = {
    { "[1, 2]", [] { return opaque(interval(gaol_core::rnd_keep(1.0), gaol_core::rnd_keep(2.0))); } },
    { "[1]", [] { return opaque(interval(gaol_core::rnd_keep(1.0))); } },
    { "[-oo, +oo]", [] { return opaque(interval::universe()); } },
  };

  // The empty set and [-1, 2], operands the compiler knows nothing about
  interval E() { return opaque(interval::emptyset()); }
  interval X() { return opaque(interval(gaol_core::rnd_keep(-1.0), gaol_core::rnd_keep(2.0))); }

  /*
    The operations of GAOL's interface with an empty operand, each on either
    side, run with the invalid-operation exception enabled below (GAOL v5).
    Each returns whether its result is the one it gives for the empty set.
    The first 48 compared a bound of the empty set with <, <=, >= or >, or
    gave its NaN bounds to the constructor, which compares them: each raised
    the exception, and died on SIGFPE where it was enabled (all 48 with the
    FPU intervals, all but the negations with the SSE2 ones). The groups after
    them were already quiet. The last five are choices the program makes on
    is_empty() of an empty interval: GCC for 32-bit ARM made them conditional
    moves with a signaling comparison, of the NaN bounds of the left operand
    of an intersection when operator&= told that operand empty otherwise than
    is_empty() did (#60), and of the bounds of an empty interval itself in the
    choice on its own is_empty(), and GCC for POWER9 (-mcpu=power9) too, where
    is_empty() was one quiet comparison (GAOL v5, point D.24 of TODO.md).
  */
  struct EmptyOperand
  {
    const char *name;
    bool (*run)();
  };

  const EmptyOperand empty_operands[] = {
    // The predicates and the relations
    { "is_canonical()", [] { return !E().is_canonical(); } },
    { "straddles_zero()", [] { return !E().straddles_zero(); } },
    { "strictly_straddles_zero()", [] { return !E().strictly_straddles_zero(); } },
    { "set_contains(double)", [] { return !E().set_contains(1.5); } },
    { "set_strictly_contains(double)", [] { return !E().set_strictly_contains(1.5); } },
    { "gaol_ieee1788::isMember(m, empty)", [] { return !gaol_ieee1788::isMember(1.5, E()); } },
    { "empty.set_contains(x)", [] { return !E().set_contains(X()); } },
    { "empty.set_geq(x)", [] { return !E().set_geq(X()); } },
    { "empty.set_disjoint(x)", [] { return E().set_disjoint(X()); } },
    { "x.set_disjoint(empty)", [] { return X().set_disjoint(E()); } },
    { "gaol_ieee1788::disjoint(empty, x)", [] { return gaol_ieee1788::disjoint(E(), X()); } },
    { "x.set_leq(empty)", [] { return !X().set_leq(E()); } },
    { "gaol_ieee1788::subset(x, empty)", [] { return !gaol_ieee1788::subset(X(), E()); } },
    // The intersection with an empty right operand, and the functions
    // intersecting their result with it
    { "x & empty", [] { return (X() & E()).is_empty(); } },
    { "x &= empty", [] { interval z = X(); z &= E(); return z.is_empty(); } },
    { "gaol_ieee1788::intersection(x, empty)", [] { return gaol_ieee1788::intersection(X(), E()).is_empty(); } },
    { "nth_root_rel(x, 1, empty)", [] { return nth_root_rel(X(), 1u, E()).is_empty(); } },
    { "asinh_rel(x, empty)", [] { return asinh_rel(X(), E()).is_empty(); } },
    { "atanh_rel(x, empty)", [] { return atanh_rel(X(), E()).is_empty(); } },
    { "invabs_rel(x, empty)", [] { return invabs_rel(X(), E()).is_empty(); } },
    // The functions that built their result from the NaN bounds
    { "sqrt(empty)", [] { return sqrt(E()).is_empty(); } },
    { "nth_root(empty, 2)", [] { return nth_root(E(), 2u).is_empty(); } },
    { "nth_root(empty, -2)", [] { return nth_root(E(), -2).is_empty(); } },
    { "sqrt_rel(empty, x)", [] { return sqrt_rel(E(), X()).is_empty(); } },
    { "sqrt_rel(empty, empty)", [] { return sqrt_rel(E(), E()).is_empty(); } },
    { "nth_root_rel(empty, 2, x)", [] { return nth_root_rel(E(), 2u, X()).is_empty(); } },
    { "gaol_ieee1788::sqrRev(empty)", [] { return gaol_ieee1788::sqrRev(E()).is_empty(); } },
    { "exp(empty)", [] { return exp(E()).is_empty(); } },
    { "min(empty, x)", [] { return min(E(), X()).is_empty(); } },
    { "min(x, empty)", [] { return min(X(), E()).is_empty(); } },
    { "max(empty, x)", [] { return max(E(), X()).is_empty(); } },
    { "max(x, empty)", [] { return max(X(), E()).is_empty(); } },
    { "floor(empty)", [] { return floor(E()).is_empty(); } },
    { "ceil(empty)", [] { return ceil(E()).is_empty(); } },
    { "integer(empty)", [] { return integer(E()).is_empty(); } },
    { "split()", [] { interval l, r; E().split(l, r); return l.is_empty() && r.is_empty(); } },
    { "split_left()", [] { return E().split_left().is_empty(); } },
    { "split_right()", [] { return E().split_right().is_empty(); } },
    { "modulo_k_pi(empty)", [] {
        double k_left = 0.0, k_right = 0.0;
        return modulo_k_pi(E(), k_left, k_right) == 0 && std::isnan(k_left) && std::isnan(k_right); } },
    { "textToInterval(\"[empty]\", \"[1, 2]\")", [] { return textToInterval("[empty]", "[1, 2]").is_empty(); } },
    { "textToInterval(\"[1, 2]\", \"[empty]\")", [] { return textToInterval("[1, 2]", "[empty]").is_empty(); } },
    // The negation of the FPU intervals, and cancel_plus, which negates
    { "-empty", [] { return (-E()).is_empty(); } },
    { "gaol_ieee1788::neg(empty)", [] { return gaol_ieee1788::neg(E()).is_empty(); } },
    { "cancel_plus(x, empty)", [] { return cancel_plus(X(), E()).is_entire(); } },
    // The outputs
    { "chi(empty)", [] { return std::isnan(chi(E())); } },
    { "operator<<", [] { return write(E(), interval_format::bounds) == "[empty]"; } },
    { "operator std::string", [] { const std::string s = E(); return s == "[empty]"; } },
    { "gaol_ieee1788::intervalToText(empty)", [] { return gaol_ieee1788::intervalToText(E()) == "[empty]"; } },
    // Already quiet
    { "the arithmetic operations", [] {
        return (E() + X()).is_empty() && (X() + E()).is_empty() && (E() - X()).is_empty() && (X() - E()).is_empty()
          && (E() * X()).is_empty() && (X() * E()).is_empty() && (E() / X()).is_empty() && (X() / E()).is_empty()
          && (E() % X()).is_empty() && (X() % E()).is_empty() && (E() + 1.5).is_empty() && (1.5 - E()).is_empty()
          && (E() * 0.0).is_empty() && (1.5 / E()).is_empty() && (E() & X()).is_empty() && (E() | X()).set_eq(X())
          && (X() | E()).set_eq(X()) && inverse(E()).is_empty() && sqr(E()).is_empty()
          && fma(E(), X(), X()).is_empty() && fma(X(), E(), X()).is_empty() && fma(X(), X(), E()).is_empty()
          && cancel_minus(E(), X()).is_empty() && cancel_minus(X(), E()).is_entire(); } },
    // The compound operators and the operations with a double on the empty
    // set, whose lanes the NEON operations add, multiply or divide without a
    // comparison of them (GAOL v5)
    { "the compound operators and the operations with a double", [] {
        interval a = E(), b = E(), c = E(), d = E(), e = E(), f = E(), g = E(), h = E(), i = E(), j = E();
        a += X(); b -= X(); c *= X(); d /= X(); e %= X();
        f += 1.5; g -= 1.5; h *= 1.5; i /= 1.5; j %= 1.5;
        return a.is_empty() && b.is_empty() && c.is_empty() && d.is_empty() && e.is_empty() && f.is_empty()
          && g.is_empty() && h.is_empty() && i.is_empty() && j.is_empty()
          && (E() - 1.5).is_empty() && (1.5 + E()).is_empty() && (E() * 1.5).is_empty() && (1.5 * E()).is_empty()
          && (E() / 1.5).is_empty() && (E() % 1.5).is_empty() && (1.5 % E()).is_empty() && (E() - E()).is_empty()
          && (E() + E()).is_empty() && sqr(E()).is_empty() && sqrt(E()).is_empty(); } },
    { "the powers and the roots", [] {
        return pow(E(), 3).is_empty() && pow(E(), -2).is_empty() && pow(E(), 0).is_empty() && pow(E(), 2u).is_empty()
          && pow(E(), 2).is_empty() && gaol_ieee1788::pown(E(), 2).is_empty() && pow(E(), interval(2.0)).is_empty()
          && pow(E(), 2.5).is_empty() && pow(E(), X()).is_empty() && pow(X(), E()).is_empty()
          && gaol_ieee1788::pow(E(), X()).is_empty() && gaol_ieee1788::pow(X(), E()).is_empty()
          && nth_root(E(), 3u).is_empty() && nth_root(E(), 4u).is_empty() && nth_root(E(), -3).is_empty(); } },
    { "the exponentials and the logarithms", [] {
        return log(E()).is_empty() && exp2(E()).is_empty() && exp10(E()).is_empty() && log2(E()).is_empty()
          && log10(E()).is_empty() && expm1(E()).is_empty() && exp2m1(E()).is_empty() && exp10m1(E()).is_empty()
          && log1p(E()).is_empty() && log2p1(E()).is_empty() && log10p1(E()).is_empty() && rsqrt(E()).is_empty(); } },
    { "the trigonometric and hyperbolic functions", [] {
        return cos(E()).is_empty() && sin(E()).is_empty() && tan(E()).is_empty() && acos(E()).is_empty()
          && asin(E()).is_empty() && atan(E()).is_empty() && atan2(E(), X()).is_empty() && atan2(X(), E()).is_empty()
          && sinpi(E()).is_empty() && cospi(E()).is_empty() && tanpi(E()).is_empty() && asinpi(E()).is_empty()
          && acospi(E()).is_empty() && atanpi(E()).is_empty() && atan2pi(E(), X()).is_empty()
          && atan2pi(X(), E()).is_empty() && cosh(E()).is_empty() && sinh(E()).is_empty() && tanh(E()).is_empty()
          && acosh(E()).is_empty() && asinh(E()).is_empty() && atanh(E()).is_empty() && hypot(E(), X()).is_empty()
          && hypot(X(), E()).is_empty(); } },
    { "erf and erfc", [] { return erf(E()).is_empty() && erfc(E()).is_empty(); } },
    { "abs, sign and the roundings", [] {
        return abs(E()).is_empty() && sign(E()).is_empty() && trunc(E()).is_empty()
          && round_ties_to_even(E()).is_empty() && round_ties_to_away(E()).is_empty(); } },
    { "the reverse functions", [] {
        return acos_rel(E(), X()).is_empty() && acos_rel(X(), E()).is_empty() && asin_rel(E(), X()).is_empty()
          && asin_rel(X(), E()).is_empty() && atan_rel(E(), X()).is_empty() && atan_rel(X(), E()).is_empty()
          && acosh_rel(E(), X()).is_empty() && acosh_rel(X(), E()).is_empty() && asinh_rel(E(), X()).is_empty()
          && atanh_rel(E(), X()).is_empty() && invabs_rel(E(), X()).is_empty() && sqrt_rel(X(), E()).is_empty()
          && nth_root_rel(E(), 3u, X()).is_empty() && nth_root_rel(X(), 3u, E()).is_empty()
          && div_rel(E(), X(), X()).is_empty() && div_rel(X(), E(), X()).is_empty()
          && div_rel(X(), X(), E()).is_empty() && max_rel(E(), X(), X()).is_empty()
          && max_rel(X(), E(), X()).is_empty() && max_rel(X(), X(), E()).is_empty()
          && min_rel(E(), X(), X()).is_empty() && min_rel(X(), E(), X()).is_empty()
          && min_rel(X(), X(), E()).is_empty() && sign_rel(E(), X()).is_empty() && sign_rel(X(), E()).is_empty()
          && floor_rel(E(), X()).is_empty() && floor_rel(X(), E()).is_empty(); } },
    // The reverse functions of pow (GAOL v5)
    { "pow_rel and pow_exponent_rel", [] {
        return pow_rel(E(), X(), X()).is_empty() && pow_rel(X(), E(), X()).is_empty() && pow_rel(X(), X(), E()).is_empty()
          && pow_exponent_rel(E(), X(), X()).is_empty() && pow_exponent_rel(X(), E(), X()).is_empty()
          && pow_exponent_rel(X(), X(), E()).is_empty() && gaol_ieee1788::powRev1(X(), E()).is_empty()
          && gaol_ieee1788::powRev2(E(), X()).is_empty(); } },
    { "atan2_rel_y and atan2_rel_x", [] {
        return atan2_rel_y(E(), X(), X()).is_empty() && atan2_rel_y(X(), E(), X()).is_empty()
          && atan2_rel_y(X(), X(), E()).is_empty() && atan2_rel_x(E(), X(), X()).is_empty()
          && atan2_rel_x(X(), E(), X()).is_empty() && atan2_rel_x(X(), X(), E()).is_empty()
          && gaol_ieee1788::atan2Rev1(E(), X()).is_empty() && gaol_ieee1788::atan2Rev2(X(), E()).is_empty(); } },
    { "the numeric functions", [] {
        double m, r;
        E().mid_rad(m, r);
        return std::isnan(E().midpoint()) && E().mid().is_empty() && std::isnan(E().width()) && std::isnan(E().rad())
          && std::isnan(m) && std::isnan(r) && std::isnan(E().mig()) && std::isnan(E().mag()) && std::isnan(E().smig())
          && std::isnan(hausdorff(E(), X())) && std::isnan(hausdorff(X(), E()))
          && gaol_ieee1788::inf(E()) == GAOL_INFINITY && gaol_ieee1788::sup(E()) == -GAOL_INFINITY; } },
    { "the other predicates", [] {
        return E().certainly_positive() && E().certainly_negative() && E().certainly_strictly_positive()
          && E().certainly_strictly_negative() && !E().is_entire() && !E().is_common_interval()
          && !E().is_symmetric() && !E().is_zero() && !E().is_a_double() && !E().is_an_int(); } },
    { "the other relations", [] {
        return E().certainly_leq(X()) && X().certainly_leq(E()) && E().certainly_le(X()) && X().certainly_le(E())
          && E().certainly_geq(X()) && X().certainly_geq(E()) && E().certainly_ge(X()) && X().certainly_ge(E())
          && (E() <= X()) && (X() < E()) && (E() >= X()) && (X() > E()) && X().set_contains(E())
          && E().set_contains(E()) && !E().set_strictly_contains(X()) && X().set_strictly_contains(E())
          && !E().set_eq(X()) && E().set_neq(X()) && E().set_leq(X()) && E().set_le(X()) && !X().set_le(E())
          && !E().less(X()) && !X().strictly_less(E()); } },
    { "the other outputs", [] {
        return write(E(), interval_format::width) == "[empty]" && write(E(), interval_format::center) == "[empty]"
          && write(E(), interval_format::hexa) == "[empty]" && write(E(), interval_format::agreeing) == "[empty]"
          && exact_string(E()) == "[empty]"; } },
    // A value the program chooses with is_empty() of an empty interval: GCC
    // for 32-bit ARM made the choice a conditional move, and compared the NaN
    // bounds with the signaling vcmpe, after an intersection whose left
    // operand is empty where it did not know the answer from operator&= (#60),
    // and on the empty interval alone, as GCC for POWER9 did with xscmpgedp;
    // is_empty() now tells the NaN bounds with std::isunordered() first there
    // (see gaol/gaol_interval_core.h). Where the compiler optimizes for size (-Os),
    // GCC 14 for 32-bit ARM and GCC 13 for POWER9 call operator& or operator&=
    // rather than inlining it, and the choice is then made on is_empty() of an
    // empty interval alone: these choices were not checked there
    // (GAOL_TESTS_CHOICES) until jobs of the continuous integration built the
    // tests for size, on armhf and for POWER9 (containers.yml)
    { "(empty & x).is_empty() ? 0 : right()", [] {
        const interval z = E() & X();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "(empty & empty).is_empty() ? 0 : right()", [] {
        const interval z = E() & E();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "gaol_ieee1788::intersection(empty, x).is_empty() ? 0 : right()", [] {
        const interval z = gaol_ieee1788::intersection(E(), X());
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "empty.is_empty() ? 0 : right()", [] {
        const interval z = E();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
    { "(x & empty).is_empty() ? 0 : right()", [] {
        const interval z = X() & E();
        const volatile double r = z.is_empty() ? 0.0 : z.right();
        return r == 0.0; } },
  };

  /*
    A NaN the program gives GAOL, which interval(d) makes the empty set, to
    the constructors, the operations and the relations with a double (GAOL v5,
    point D.24 of TODO.md): the first comparison of the constructors, and of
    the operations with a double testing d, compared the NaN with <, <= or >,
    which raised the invalid-operation exception, and so did pow(x, d). Each
    returns whether its result is the one of the empty set.
  */
  double nan_double() { return gaol_core::rnd_keep(std::numeric_limits<double>::quiet_NaN()); }

  const EmptyOperand nan_operands[] = {
    { "interval(NAN)", [] { return interval(nan_double()).is_empty(); } },
    { "interval(NAN, 1)", [] { return interval(nan_double(), gaol_core::rnd_keep(1.0)).is_empty(); } },
    { "interval(1, NAN)", [] { return interval(gaol_core::rnd_keep(1.0), nan_double()).is_empty(); } },
    { "interval(NAN, NAN)", [] { return interval(nan_double(), nan_double()).is_empty(); } },
    { "x = NAN", [] { interval z = X(); z = nan_double(); return z.is_empty(); } },
    { "x += NAN", [] { interval z = X(); z += nan_double(); return z.is_empty(); } },
    { "x -= NAN", [] { interval z = X(); z -= nan_double(); return z.is_empty(); } },
    { "x *= NAN", [] { interval z = X(); z *= nan_double(); return z.is_empty(); } },
    { "x /= NAN", [] { interval z = X(); z /= nan_double(); return z.is_empty(); } },
    { "x %= NAN", [] { interval z = X(); z %= nan_double(); return z.is_empty(); } },
    { "x &= NAN", [] { interval z = X(); z &= nan_double(); return z.is_empty(); } },
    { "x |= NAN", [] { interval z = X(); z |= nan_double(); return z.set_eq(X()); } },
    { "x + NAN, NAN - x, x * NAN, NAN / x, x % NAN", [] {
        return (X() + nan_double()).is_empty() && (nan_double() - X()).is_empty() && (X()*nan_double()).is_empty()
          && (nan_double()/X()).is_empty() && (X() % nan_double()).is_empty(); } },
    { "x <= NAN, NAN < x, x >= NAN, NAN > x", [] {
        return (X() <= nan_double()) && (nan_double() < X()) && (X() >= nan_double()) && (nan_double() > X()); } },
    { "set_contains(NAN)", [] { return !X().set_contains(nan_double()); } },
    { "set_strictly_contains(NAN)", [] { return !X().set_strictly_contains(nan_double()); } },
    { "gaol_ieee1788::isMember(NAN, x)", [] { return !gaol_ieee1788::isMember(nan_double(), X()); } },
    { "pow(x, NAN)", [] { return pow(X(), nan_double()).is_empty(); } },
  };

  /*
    Products of intervals with a zero bound and an infinite one (GAOL v5,
    point D.24): the product of the SSE2 intervals multiplied the bounds
    pairwise, 0 by +oo among them, which raised the invalid-operation
    exception, and made a 0 of the NaN afterwards: [0]*[1, +oo] and
    [0, +oo]*[0] raised it, and so did pow([1], [1, +oo]) through them. The
    bounds of every product of two intervals with bounds in
    {-oo, -2, -0, 0, 3, +oo} are compared with the extrema of the products of
    their bounds, a zero bound and an infinite one giving 0 (IEEE 1788-2015),
    computed on these small integers, where they are exact.
  */
  double bound_product(double a, double b)
  {
    // The product is computed on operands that are never 0 and an infinity
    // (GAOL v5): Apple clang on arm64 computed a*b before the choice, then
    // chose, and 0*+oo raised the invalid-operation flag in this reference,
    // which the check below then took for one GAOL had raised, depending on
    // what was inlined around it
    const double a1 = (a == 0.0) ? 1.0 : a, b1 = (b == 0.0) ? 1.0 : b;
    const double p = a1*b1;
    return (a == 0.0 || b == 0.0) ? 0.0 : p;
  }

  // The nonempty intervals with bounds in bounds
  std::vector<interval> intervals_with_bounds(std::initializer_list<double> bounds)
  {
    std::vector<interval> intervals;
    for (double l : bounds) {
      for (double r : bounds) {
        const interval z(l, r);
        if (!z.is_empty()) {
          intervals.push_back(z);
        }
      }
    }
    return intervals;
  }

  bool products_with_zero_and_infinite_bounds_are_right()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 3.0, GAOL_INFINITY });
    bool right = true;
    for (const interval& x : intervals) {
      for (const interval& y : intervals) {
        const double p[4] = { bound_product(x.left(), y.left()), bound_product(x.left(), y.right()),
                              bound_product(x.right(), y.left()), bound_product(x.right(), y.right()) };
        const interval z = opaque(x)*opaque(y);
        right = right && z.left() == std::min(std::min(p[0], p[1]), std::min(p[2], p[3]))
                && z.right() == std::max(std::max(p[0], p[1]), std::max(p[2], p[3]));
      }
    }
    return right && pow(opaque(interval(1.0)), opaque(interval(1.0, GAOL_INFINITY))).set_eq(interval(1.0));
  }

  // The operations of products_with_zero_and_infinite_bounds_are_right() that
  // raise the invalid-operation flag, each run alone, for the message of a
  // failure: the flag tells that one of them raised it, not which
  std::string products_raising_invalid()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 3.0, GAOL_INFINITY });
    std::string raising;
    for (const interval& x : intervals) {
      for (const interval& y : intervals) {
        std::feclearexcept(FE_ALL_EXCEPT);
        const interval z = opaque(x)*opaque(y);
        if (std::fetestexcept(FE_INVALID) != 0) {
          raising += " " + hex(x) + "*" + hex(y) + " = " + hex(z) + ";";
        }
      }
    }
    std::feclearexcept(FE_ALL_EXCEPT);
    const interval p = pow(opaque(interval(1.0)), opaque(interval(1.0, GAOL_INFINITY)));
    if (std::fetestexcept(FE_INVALID) != 0) {
      raising += " pow([1], [1, +oo]) = " + hex(p) + ";";
    }
    // and the reference products of the bounds, which the test computes
    for (const interval& x : intervals) {
      for (const interval& y : intervals) {
        std::feclearexcept(FE_ALL_EXCEPT);
        const volatile double q[4] = { bound_product(x.left(), y.left()), bound_product(x.left(), y.right()),
                                       bound_product(x.right(), y.left()), bound_product(x.right(), y.right()) };
        static_cast<void>(q);
        if (std::fetestexcept(FE_INVALID) != 0) {
          raising += " the reference products of the bounds of " + hex(x) + " and " + hex(y) + ";";
        }
      }
    }
    return raising.empty() ? std::string(" none when run alone") : raising;
  }

  /*
    div_rel(K, J, [-oo, +oo]) for every K and J with bounds in
    {-oo, -2, -0, 0, 4, +oo} (GAOL v5, point D.24): with the SSE2 intervals,
    the branches keeping one half of a division divided both halves, and the
    half thrown away divided +oo by +oo, which raised the invalid-operation
    exception (div_rel([4, +oo], [-oo, 4], I)), or a nonzero bound by a zero
    one, which raised the division-by-zero one (div_rel([-2], [-2, 0], I)).
    Where J does not contain 0, the bounds are compared with the extrema of
    the quotients of the bounds, exact on these, the quotient of two infinite
    bounds left out (any positive number is a limit of such quotients, and
    the others reach 0 and +oo then), and computed apart from GAOL, without
    raising anything.
  */
  /*
    The extrema of the quotients of the bounds of K by those of J, which has
    no zero, the quotient of two infinite bounds left out. Each quotient is
    computed on operands for which it raises nothing, whatever the choice
    that keeps it or not, as bound_product() does for the products: a
    compiler computing the quotients before the choice, or several at once,
    would raise a flag in the reference, which the check of the flags would
    take for one div_rel() had raised. They are kept in volatile variables
    for the same reason.
  */
  void bound_quotients(const interval& k, const interval& j, double& l, double& r)
  {
    l = GAOL_INFINITY;
    r = -GAOL_INFINITY;
    for (double a : { k.left(), k.right() }) {
      for (double b : { j.left(), j.right() }) {
        const bool both_infinite = std::isinf(a) && std::isinf(b);
        const double a1 = both_infinite ? 1.0 : a;
        const double b1 = (both_infinite || b == 0.0) ? 1.0 : b;
        const volatile double q = a1/b1;
        if (!both_infinite) {
          const double d = q;
          l = std::min(l, d);
          r = std::max(r, d);
        }
      }
    }
  }

  bool quotients_with_zero_and_infinite_bounds_are_right()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    bool right = true;
    for (const interval& k : intervals) {
      for (const interval& j : intervals) {
        const interval z = div_rel(opaque(k), opaque(j), opaque(interval::universe()));
        if (j.left() > 0.0 || j.right() < 0.0) {
          double l, r;
          bound_quotients(k, j, l, r);
          right = right && z.left() == l && z.right() == r;
        }
      }
    }
    return right;
  }

  // The div_rel(K, J, [-oo, +oo]) of quotients_with_zero_and_infinite_bounds_are_right()
  // that raise the invalid-operation or the division-by-zero flag, each run
  // alone, and those whose quotient is not the one expected, for the message
  // of a failure: the flag tells that one of them, or the reference quotients
  // of the test, raised it, not which
  std::string quotients_raising()
  {
    std::string raising;
    // the intervals themselves, which the test builds after clearing the flags
    std::feclearexcept(FE_ALL_EXCEPT);
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO) != 0) {
      raising += " the building of the intervals;";
    }
    for (const interval& k : intervals) {
      for (const interval& j : intervals) {
        std::feclearexcept(FE_ALL_EXCEPT);
        const interval z = div_rel(opaque(k), opaque(j), opaque(interval::universe()));
        const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
        if (raised != 0) {
          raising += " div_rel(" + hex(k) + ", " + hex(j) + ", R) = " + hex(z) + ((raised & FE_INVALID) ? " (invalid);" : " (division by zero);");
        }
      }
    }
    // and the reference quotients of the bounds, which the test computes
    for (const interval& k : intervals) {
      for (const interval& j : intervals) {
        if (j.left() > 0.0 || j.right() < 0.0) {
          std::feclearexcept(FE_ALL_EXCEPT);
          double l, r;
          bound_quotients(opaque(k), opaque(j), l, r);
          if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO) != 0) {
            raising += " the reference quotients of " + hex(k) + " and " + hex(j) + ";";
          }
        }
      }
    }
    return raising.empty() ? std::string(" none when run alone") : raising;
  }

  std::string quotients_wrong()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    std::string wrong;
    for (const interval& k : intervals) {
      for (const interval& j : intervals) {
        const interval z = div_rel(opaque(k), opaque(j), opaque(interval::universe()));
        if (j.left() > 0.0 || j.right() < 0.0) {
          double l, r;
          bound_quotients(k, j, l, r);
          if (!(z.left() == l && z.right() == r)) {
            wrong += " div_rel(" + hex(k) + ", " + hex(j) + ", R) = " + hex(z) + " (expected [" + hex(l) + ", " + hex(r) + "]);";
          }
        }
      }
    }
    return wrong.empty() ? std::string(" none when run alone") : wrong;
  }

  /*
    The divisions and the inverses of intervals with zero and infinite bounds
    (GAOL v5): K/J, K%J, 1.0/J, inverse(J), pow(J, -1) and
    gaol_ieee1788::pown(J, -1), for every K and J with bounds in {-oo, -2, -0,
    0, 4, +oo}. With the SSE2 intervals, the branches keeping one half of a
    quotient divided both halves, and the half thrown away divided a nonzero
    bound by a zero one, which raised the division-by-zero exception
    ([1, 2]/[0, 2], inverse([0, 2]), pow([0, 2], -1)), where the FPU intervals
    raise nothing. The inverses are [1]/J; where K does not contain 0, K/J
    and K%J are div_rel(K, J, [-oo, +oo]), the hull of the k/j for j != 0,
    whose halves D.24 made apart.
  */
  bool divisions_with_zero_and_infinite_bounds_are_right()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    bool right = true;
    for (const interval& j : intervals) {
      const interval inv = interval(1.0)/opaque(j);
      right = right && inverse(opaque(j)).set_eq(inv) && pow(opaque(j), -1).set_eq(inv)
        && gaol_ieee1788::pown(opaque(j), -1).set_eq(inv) && (1.0/opaque(j)).set_eq(inv);
      for (const interval& k : intervals) {
        const interval q = opaque(k)/opaque(j), r = opaque(k)%opaque(j);
        if (!k.set_contains(0.0)) {
          const interval z = div_rel(opaque(k), opaque(j), interval::universe());
          right = right && q.set_eq(z) && r.set_eq(z);
        }
      }
    }
    return right;
  }

  // The operations of divisions_with_zero_and_infinite_bounds_are_right()
  // that raise the invalid-operation or the division-by-zero flag, each run
  // alone, for the message of a failure
  std::string divisions_raising()
  {
    const std::vector<interval> intervals = intervals_with_bounds({ -GAOL_INFINITY, -2.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    std::string raising;
    const auto run = [&raising](const std::string& name, const std::function<interval()>& f) {
      std::feclearexcept(FE_ALL_EXCEPT);
      const interval z = f();
      const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
      if (raised != 0) {
        raising += " " + name + " = " + hex(z) + ((raised & FE_INVALID) ? " (invalid);" : " (division by zero);");
      }
    };
    for (const interval& j : intervals) {
      run("inverse(" + hex(j) + ")", [&j] { return inverse(opaque(j)); });
      run("pow(" + hex(j) + ", -1)", [&j] { return pow(opaque(j), -1); });
      run("pown(" + hex(j) + ", -1)", [&j] { return gaol_ieee1788::pown(opaque(j), -1); });
      run("1.0/" + hex(j), [&j] { return 1.0/opaque(j); });
      for (const interval& k : intervals) {
        run(hex(k) + "/" + hex(j), [&k, &j] { return opaque(k)/opaque(j); });
        run(hex(k) + "%" + hex(j), [&k, &j] { return opaque(k)%opaque(j); });
        if (!k.set_contains(0.0)) {
          run("div_rel(" + hex(k) + ", " + hex(j) + ", R)", [&k, &j] { return div_rel(opaque(k), opaque(j), interval::universe()); });
        }
      }
    }
    return raising.empty() ? std::string(" none when run alone") : raising;
  }

  /*
    pow for an exponent of extreme magnitude (GAOL v5, point D.24): CORE-MATH's
    pow makes its approximation of log(x) a NaN on purpose for |y| < 2^-969 and
    |y| >= 2^1014, and compares the NaN product with <, which raised the
    invalid-operation exception at the corners of these boxes. The bounds are
    the tightest ones, known apart from any pow: for |y| < 2^-969 and a
    positive x other than 1, |y log(x)| < 745 2^-969 < 2^-959, and x^y is
    within 2^-958 of 1, above 1 where y log(x) > 0, so that its tightest
    enclosure is [1, 1 + 2^-52] there and [1 - 2^-53, 1] otherwise; for
    |y| >= 2^1014, |y log(x)| > 2^1014 2^-53, and x^y is above DBL_MAX or
    below 2^-1074.
  */
  bool bounds_are(const interval& z, double l, double r)
  {
    return z.left() == l && z.right() == r;
  }

  const double above_one = 1.0 + std::numeric_limits<double>::epsilon();
  const double below_one = 1.0 - 0.5*std::numeric_limits<double>::epsilon();
  const double largest = (std::numeric_limits<double>::max)();
  const double smallest = std::numeric_limits<double>::denorm_min();

  // [l, r], an interval the compiler knows nothing about
  interval make(double l, double r) { return opaque(interval(gaol_core::rnd_keep(l), gaol_core::rnd_keep(r))); }

  const EmptyOperand extreme_powers[] = {
    { "pow([1, 2], [2^-1074])", [] { return bounds_are(pow(make(1.0, 2.0), make(smallest, smallest)), 1.0, above_one); } },
    { "pow([1, 2], [1e-300])", [] { return bounds_are(pow(make(1.0, 2.0), make(1e-300, 1e-300)), 1.0, above_one); } },
    { "pow([0.5], [1e-300])", [] { return bounds_are(pow(make(0.5, 0.5), make(1e-300, 1e-300)), below_one, 1.0); } },
    { "pow([0.5], [-1e-300])", [] { return bounds_are(pow(make(0.5, 0.5), make(-1e-300, -1e-300)), 1.0, above_one); } },
    { "pow([2, 3], [-2^-1074, 2^-1074])", [] { return bounds_are(pow(make(2.0, 3.0), make(-smallest, smallest)), below_one, above_one); } },
    { "pow([2], 1e-300)", [] { return bounds_are(pow(make(2.0, 2.0), gaol_core::rnd_keep(1e-300)), 1.0, above_one); } },
    { "pow([1, 2], [1e300, 1e308])", [] { return bounds_are(pow(make(1.0, 2.0), make(1e300, 1e308)), 1.0, GAOL_INFINITY); } },
    { "pow([1.5], [1e300, 1e308])", [] { return bounds_are(pow(make(1.5, 1.5), make(1e300, 1e308)), largest, GAOL_INFINITY); } },
    { "pow([0.5], [1e300, 1e308])", [] { return bounds_are(pow(make(0.5, 0.5), make(1e300, 1e308)), 0.0, smallest); } },
    { "pow([0.5], [-1e308, -1e300])", [] { return bounds_are(pow(make(0.5, 0.5), make(-1e308, -1e300)), largest, GAOL_INFINITY); } },
    // An integer beyond the ints, from 2^1014 on, as gaol_ieee1788::pow takes it
    { "gaol_ieee1788::pow([1.5], [1e308])", [] {
        return bounds_are(gaol_ieee1788::pow(make(1.5, 1.5), make(1e308, 1e308)), largest, GAOL_INFINITY); } },
    { "gaol_ieee1788::pow([0, 0.5], [1e308])", [] {
        return bounds_are(gaol_ieee1788::pow(make(0.0, 0.5), make(1e308, 1e308)), 0.0, smallest); } },
    { "gaol_ieee1788::pow([0.5, 2], [-1e308])", [] {
        return bounds_are(gaol_ieee1788::pow(make(0.5, 2.0), make(-1e308, -1e308)), 0.0, GAOL_INFINITY); } },
  };

  /*
    Powers next to the largest double (GAOL v5, point 77 of TODO.md):
    for a power next to 1.7975e308, y log(x) from 709.78263 to 709.78267,
    CORE-MATH's pow scaled its first approximation by 2^1024, an infinity,
    and computed Inf - Inf, which raised the invalid-operation exception
    before its second phase gave the right bounds. 1023.9999 (the double
    0x1.fffffcb923a2ap+9) is such an exponent for the base 2, and so is
    3000000001 for b = 0x1.000003f82a1fp+0, whichever the sign of b. The
    bounds are the tightest ones, the two doubles around the power, computed
    with mpmath at 3000 bits: 2^1023.9999 is 0x1.fff6ea43bd988p+1023 to
    0x1.fff6ea43bd989p+1023, and b^3000000001 is 0x1.fff7155c90a68p+1023 to
    0x1.fff7155c90a69p+1023. pow_rel and pow_exponent_rel have to contain the
    base and the exponent the power was made from (their bounds are not
    tested).
  */
  const double near_overflow_exponent = 1023.9999;
  const double near_overflow_base = 0x1.000003f82a1fp+0;
  const double near_overflow_lower = 0x1.fff6ea43bd988p+1023, near_overflow_upper = 0x1.fff6ea43bd989p+1023;
  const double near_overflow_power_lower = 0x1.fff7155c90a68p+1023, near_overflow_power_upper = 0x1.fff7155c90a69p+1023;

  const EmptyOperand powers_near_overflow[] = {
    { "pow([2], [1023.9999])", [] {
        return bounds_are(pow(make(2.0, 2.0), make(near_overflow_exponent, near_overflow_exponent)),
                          near_overflow_lower, near_overflow_upper); } },
    { "pow([2], 1023.9999)", [] {
        return bounds_are(pow(make(2.0, 2.0), gaol_core::rnd_keep(near_overflow_exponent)),
                          near_overflow_lower, near_overflow_upper); } },
    { "pow([0.5], [-1023.9999])", [] {
        return bounds_are(pow(make(0.5, 0.5), make(-near_overflow_exponent, -near_overflow_exponent)),
                          near_overflow_lower, near_overflow_upper); } },
    { "gaol_ieee1788::pow([2], [1023.9999])", [] {
        return bounds_are(gaol_ieee1788::pow(make(2.0, 2.0), make(near_overflow_exponent, near_overflow_exponent)),
                          near_overflow_lower, near_overflow_upper); } },
    { "pow([2], [1023.9999]) in a text", [] {
        return bounds_are(textToInterval("pow(2, 0x1.fffffcb923a2ap+9)"), near_overflow_lower, near_overflow_upper); } },
    // The integer exponents beyond the ints go through CORE-MATH's pow too
    { "gaol_ieee1788::pow([b], [3000000001])", [] {
        return bounds_are(gaol_ieee1788::pow(make(near_overflow_base, near_overflow_base), make(3000000001.0, 3000000001.0)),
                          near_overflow_power_lower, near_overflow_power_upper); } },
    { "pow([b], 3000000001)", [] {
        return bounds_are(gaol::pow(make(near_overflow_base, near_overflow_base), 3000000001LL),
                          near_overflow_power_lower, near_overflow_power_upper); } },
    { "pow([-b], 3000000001)", [] {
        return bounds_are(gaol::pow(make(-near_overflow_base, -near_overflow_base), 3000000001LL),
                          -near_overflow_power_upper, -near_overflow_power_lower); } },
    { "pow_rel: the base of [2]^[1023.9999]", [] {
        return pow_rel(make(near_overflow_lower, near_overflow_upper), make(near_overflow_exponent, near_overflow_exponent),
                       make(1.0, 4.0)).set_contains(2.0); } },
    { "pow_exponent_rel: the exponent of [2]^y", [] {
        return pow_exponent_rel(make(near_overflow_lower, near_overflow_upper), make(2.0, 2.0),
                                make(1000.0, 1030.0)).set_contains(near_overflow_exponent); } },
  };

  /*
    Integer powers at the ends of the doubles (GAOL v5, points 8, 72, 73 and
    74 of TODO.md): an infinite bound, a power beyond the largest double or
    below the least subnormal, 0 for n < 0, and the exponents beyond the ints.
    The exponentiation of the exact products would compute oo - oo there; it
    is not run on these corners, which are taken one at a time, scaled. Its
    rests, or-ed together to tell the exact powers, are compared as
    integers: as doubles, the bits of [5.3, 5.5]^25 made a signaling NaN, and
    the comparison raised the invalid-operation exception.
  */
  const EmptyOperand integer_powers_at_the_ends[] = {
    { "pow([2, +oo], 3)", [] { return bounds_are(pow(make(2.0, GAOL_INFINITY), 3), 8.0, GAOL_INFINITY); } },
    { "pow([-oo, -2], 3)", [] { return bounds_are(pow(make(-GAOL_INFINITY, -2.0), 3), -GAOL_INFINITY, -8.0); } },
    { "pow([-oo, +oo], 4)", [] { return bounds_are(pow(make(-GAOL_INFINITY, GAOL_INFINITY), 4), 0.0, GAOL_INFINITY); } },
    { "pow([2, +oo], -2)", [] { return bounds_are(pow(make(2.0, GAOL_INFINITY), -2), 0.0, 0.25); } },
    { "pow([1e200, 1e201], 3)", [] { return bounds_are(pow(make(1e200, 1e201), 3), largest, GAOL_INFINITY); } },
    { "pow([1e-200, 2], 3)", [] { return bounds_are(pow(make(1e-200, 2.0), 3), 0.0, 8.0); } },
    { "pow([1e-200], -3)", [] { return bounds_are(pow(make(1e-200, 1e-200), -3), largest, GAOL_INFINITY); } },
    { "pow([0, 2], -2)", [] { return bounds_are(pow(make(0.0, 2.0), -2), 0.25, GAOL_INFINITY); } },
    { "pow([-2, 3], -3)", [] { return bounds_are(pow(make(-2.0, 3.0), -3), -GAOL_INFINITY, GAOL_INFINITY); } },
    { "pow([5.3, 5.5], 25)", [] {
        return bounds_are(pow(make(0x1.53904dcdd067fp+2, 0x1.5f7bfe0574cedp+2), 25),
                          0x1.23b94c6b3ce21p+60, 0x1.5996e94ba6f8fp+61); } },
    { "pow([2, +oo], 3000000000u)", [] { return bounds_are(gaol::pow(make(2.0, GAOL_INFINITY), 3000000000u), largest, GAOL_INFINITY); } },
    { "pow([-oo, -0.5], -5e9 - 1)", [] {
        return bounds_are(gaol::pow(make(-GAOL_INFINITY, -0.5), -5000000001LL), -GAOL_INFINITY, 0.0); } },
    { "pow([0, 2], [-1e10])", [] { return bounds_are(pow(make(0.0, 2.0), make(-1e10, -1e10)), 0.0, GAOL_INFINITY); } },
  };

  /*
    The differences of the bounds of cancel_minus() and cancel_plus() next to
    the largest double (GAOL v5, point 78 of TODO.md). The TwoSum that
    compares the two differences read an infinite sum, whose terms ap = s - b
    and bp = s - ap are inf and inf - inf, and raised the invalid-operation
    exception (it stops a program that enabled it), though the result has an
    infinite bound. The first seven cases have a difference beyond the largest
    double: 2 DBL_MAX, 1.5 DBL_MAX, and DBL_MAX + 2^970, which is halfway
    between DBL_MAX and 2^1024 and rounds to the infinity to nearest; their
    tightest enclosures are [DBL_MAX, +oo] and [-oo, -DBL_MAX]. In the last
    two the sum is finite, but halfway between two doubles and rounded away
    from the exact one by 2^970 (the mantissa of DBL_MAX being odd): s - b was
    then DBL_MAX + 2^970, infinite, and the error NaN, so that the second read
    X, as wide as Y, as narrower than Y and gave [-oo, +oo]. The exact upper
    bound of both is DBL_MAX - 3 2^970, halfway between DBL_MAX - 4 2^970 and
    DBL_MAX - 2 2^970; the lower bound is 0 for the first and the same number
    for the second.
  */
  const double half_ulp_of_largest = std::ldexp(1.0, 970);

  const EmptyOperand cancels_next_to_the_largest[] = {
    { "cancel_minus([DBL_MAX], [-DBL_MAX])", [] {
        return bounds_are(cancel_minus(make(largest, largest), make(-largest, -largest)), largest, GAOL_INFINITY); } },
    { "cancel_plus([DBL_MAX], [DBL_MAX])", [] {
        return bounds_are(cancel_plus(make(largest, largest), make(largest, largest)), largest, GAOL_INFINITY); } },
    { "cancel_minus([-DBL_MAX], [DBL_MAX])", [] {
        return bounds_are(cancel_minus(make(-largest, -largest), make(largest, largest)), -GAOL_INFINITY, -largest); } },
    { "cancel_plus([-DBL_MAX], [-DBL_MAX])", [] {
        return bounds_are(cancel_plus(make(-largest, -largest), make(-largest, -largest)), -GAOL_INFINITY, -largest); } },
    { "cancel_minus([DBL_MAX], [-2^970])", [] {
        return bounds_are(cancel_minus(make(largest, largest), make(-half_ulp_of_largest, -half_ulp_of_largest)),
                          largest, GAOL_INFINITY); } },
    { "cancel_minus([0, DBL_MAX], [-DBL_MAX, -DBL_MAX/2])", [] {
        return bounds_are(cancel_minus(make(0.0, largest), make(-largest, -0.5*largest)), largest, GAOL_INFINITY); } },
    { "cancel_minus([-DBL_MAX, 0], [DBL_MAX/2, DBL_MAX])", [] {
        return bounds_are(cancel_minus(make(-largest, 0.0), make(0.5*largest, largest)), -GAOL_INFINITY, -largest); } },
    // A finite sum halfway between two doubles, 3 2^970 below DBL_MAX: the
    // doubles around it are DBL_MAX - 2 ulp and DBL_MAX - ulp, ulp = 2^971
    { "cancel_minus([0, DBL_MAX], [0, 3 2^970])", [] {
        return bounds_are(cancel_minus(make(0.0, largest), make(0.0, 3.0*half_ulp_of_largest)),
                          0.0, largest - 2.0*half_ulp_of_largest); } },
    { "cancel_minus([DBL_MAX - 2^971, DBL_MAX], [2^970, 3 2^970])", [] {
        return bounds_are(cancel_minus(make(largest - 2.0*half_ulp_of_largest, largest),
                                       make(half_ulp_of_largest, 3.0*half_ulp_of_largest)),
                          largest - 4.0*half_ulp_of_largest, largest - 2.0*half_ulp_of_largest); } },
  };

  /*
    The sums, the differences, the squares, the square roots and the
    operations with a double of intervals with infinite bounds, and of the
    empty set (GAOL v5): none of them raises the invalid-operation nor the
    division-by-zero exception, and their bounds are right. The operations of
    the NEON intervals are vector instructions, which compute both bounds at
    once and compare none of them but through the bits: the sum of the stored
    bounds <-l, r> is never inf - inf, the square is made of l*l and r*r only,
    the quotient by a double d divides the bounds by |d|, and a vector
    comparison of 64-bit ARM, fcmgt or fcmge, signals the invalid-operation
    exception on the NaN bounds of the empty set. The bounds are those of
    {-oo, -4, -0, 0, 4, +oo}, the doubles d powers of two, so that every
    expected bound is exact, computed apart from GAOL on the bounds
    themselves: a bound that is an infinity stays one, and 0 times d is 0.
    A d that is infinite or a NaN gives the empty set, as interval(d) is.
    The remainders and the quotients by a double are those of the intervals
    that hold no zero: where 0 is in x, d / x is left out (the "extended"
    quotient is checked above). Each function returns "" where its results
    are right, else the first that is not.
  */
  struct Expected
  {
    enum Kind { empty, bounds, skipped } kind;
    double l, r;
  };

  Expected expect_empty() { return Expected{ Expected::empty, 0.0, 0.0 }; }
  Expected expect_bounds(double l, double r) { return Expected{ Expected::bounds, l, r }; }
  Expected expect_skipped() { return Expected{ Expected::skipped, 0.0, 0.0 }; }

  bool matches(const interval& z, const Expected& e)
  {
    switch (e.kind) {
      case Expected::empty: return z.is_empty();
      case Expected::bounds: return !z.is_empty() && z.left() == e.l && z.right() == e.r;
      case Expected::skipped: break;
    }
    return true;
  }

  std::string rather_than(const interval& z, const Expected& e)
  {
    return " = " + hex(z) + " rather than " + ((e.kind == Expected::empty) ? std::string("the empty set")
                                                  : "[" + hex(e.l) + ", " + hex(e.r) + "]");
  }

  // The nonempty intervals with bounds in {-oo, -4, -0, 0, 4, +oo}, and the empty set
  std::vector<interval> infinite_operands()
  {
    std::vector<interval> operands = intervals_with_bounds({ -GAOL_INFINITY, -4.0, -0.0, 0.0, 4.0, GAOL_INFINITY });
    operands.push_back(E());
    return operands;
  }

  // The doubles of the operations with a double: finite ones, then the ones for which interval(d) is empty
  std::vector<double> finite_doubles() { return std::vector<double>{ -8.0, -0.5, 0.0, 2.0 }; }
  std::vector<double> empty_doubles()
  {
    return std::vector<double>{ GAOL_INFINITY, -GAOL_INFINITY, std::numeric_limits<double>::quiet_NaN() };
  }

  // The flags the operation raised, so that the log of a job names the operands
  // that raised them (the check below tells only the operation)
  std::string flags_text()
  {
    return std::string((std::fetestexcept(FE_INVALID) != 0) ? "invalid-operation " : "")
           + ((std::fetestexcept(FE_DIVBYZERO) != 0) ? "division-by-zero" : "");
  }

  typedef interval (*PairOperation)(const interval&, const interval&);
  typedef Expected (*PairExpected)(const interval&, const interval&);
  typedef interval (*DoubleOperation)(const interval&, double);
  typedef Expected (*DoubleExpected)(const interval&, double);

  std::string over_pairs(const char *symbol, PairOperation op, PairExpected expected)
  {
    const std::vector<interval> operands = infinite_operands();
    for (const interval& x : operands) {
      for (const interval& y : operands) {
        std::feclearexcept(FE_ALL_EXCEPT);
        const interval z = op(opaque(x), opaque(y));
        if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO)) {
          return hex(x) + symbol + hex(y) + " raised a flag: " + flags_text();
        }
        const Expected e = (x.is_empty() || y.is_empty()) ? expect_empty() : expected(x, y);
        if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO)) {
          return hex(x) + symbol + hex(y) + ": the reference raised a flag: " + flags_text();
        }
        if (!matches(z, e)) {
          return hex(x) + symbol + hex(y) + rather_than(z, e);
        }
      }
    }
    return std::string();
  }

  std::string over_doubles(const char *symbol, DoubleOperation op, DoubleExpected expected)
  {
    const std::vector<interval> operands = infinite_operands();
    std::vector<double> doubles = finite_doubles();
    const std::vector<double> empty_ones = empty_doubles();
    doubles.insert(doubles.end(), empty_ones.begin(), empty_ones.end());
    for (const interval& x : operands) {
      for (const double d : doubles) {
        const bool gives_empty = x.is_empty() || !std::isfinite(d);
        std::feclearexcept(FE_ALL_EXCEPT);
        const interval z = op(opaque(x), gaol_core::rnd_keep(d));
        if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO)) {
          return hex(x) + symbol + hex(d) + " raised a flag: " + flags_text();
        }
        const Expected e = gives_empty ? expect_empty() : expected(x, d);
        if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO)) {
          return hex(x) + symbol + hex(d) + ": the reference raised a flag: " + flags_text();
        }
        if (!matches(z, e)) {
          return hex(x) + symbol + hex(d) + rather_than(z, e);
        }
      }
    }
    return std::string();
  }

  std::string over_intervals(const char *name, interval (*op)(const interval&), Expected (*expected)(const interval&))
  {
    for (const interval& x : infinite_operands()) {
      std::feclearexcept(FE_ALL_EXCEPT);
      const interval z = op(opaque(x));
      if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO)) {
        return std::string(name) + "(" + hex(x) + ") raised a flag: " + flags_text();
      }
      const Expected e = x.is_empty() ? expect_empty() : expected(x);
      if (std::fetestexcept(FE_INVALID | FE_DIVBYZERO)) {
        return std::string(name) + "(" + hex(x) + "): the reference raised a flag: " + flags_text();
      }
      if (!matches(z, e)) {
        return std::string(name) + "(" + hex(x) + ")" + rather_than(z, e);
      }
    }
    return std::string();
  }

  /*
    The references of the operations below, computed one operation at a time:
    each operand and each result goes through volatile memory (GAOL v5). GCC
    turned the pairs of operations of the bounds into vector instructions and
    computed the operations of the other lanes too, where they are, for the
    bounds [-oo, 4] and [-oo, +oo], an infinity less an infinity, and the
    reference raised the invalid-operation flag that the check is made to find
    in GAOL (continuous integration, GCC for x86 and for 64-bit ARM)
  */
  double held(double v) { return gaol_core::rnd_keep(v); }
  double plus(double a, double b) { return held(held(a) + held(b)); }
  double minus(double a, double b) { return held(held(a) - held(b)); }
  double times(double a, double b) { return held(held(a) * held(b)); }
  double over(double a, double b) { return held(held(a) / held(b)); }

  Expected sum_of(const interval& x, const interval& y)
  {
    const double l = plus(x.left(), y.left());
    return expect_bounds(l, plus(x.right(), y.right()));
  }
  Expected difference_of(const interval& x, const interval& y)
  {
    const double l = minus(x.left(), y.right());
    return expect_bounds(l, minus(x.right(), y.left()));
  }
  Expected sum_with(const interval& x, double d)
  {
    const double l = plus(x.left(), d);
    return expect_bounds(l, plus(x.right(), d));
  }
  Expected difference_with(const interval& x, double d)
  {
    const double l = minus(x.left(), d);
    return expect_bounds(l, minus(x.right(), d));
  }
  Expected difference_from(const interval& x, double d)
  {
    const double l = minus(d, x.right());
    return expect_bounds(l, minus(d, x.left()));
  }
  // x*0 is [0]; for d != 0 no bound is 0 times an infinity
  Expected product_with(const interval& x, double d)
  {
    if (d == 0.0) {
      return expect_bounds(0.0, 0.0);
    }
    if (d > 0.0) {
      const double l = times(x.left(), d);
      return expect_bounds(l, times(x.right(), d));
    }
    const double l = times(x.right(), d);
    return expect_bounds(l, times(x.left(), d));
  }
  // x/0 is empty, as x/[0]
  Expected quotient_by(const interval& x, double d)
  {
    if (d == 0.0) {
      return expect_empty();
    }
    if (d > 0.0) {
      const double l = over(x.left(), d);
      return expect_bounds(l, over(x.right(), d));
    }
    const double l = over(x.right(), d);
    return expect_bounds(l, over(x.left(), d));
  }
  // x % 0 is the whole line where x holds 0, else empty
  Expected remainder_by(const interval& x, double d)
  {
    if (d == 0.0) {
      return (x.left() <= 0.0 && x.right() >= 0.0) ? expect_bounds(-GAOL_INFINITY, GAOL_INFINITY) : expect_empty();
    }
    return quotient_by(x, d);
  }
  // d/x for an x that holds no zero: d/oo is 0
  Expected quotient_of(const interval& x, double d)
  {
    if (x.left() <= 0.0 && x.right() >= 0.0) {
      return expect_skipped();
    }
    const double a = over(d, x.left()), b = over(d, x.right());
    return expect_bounds(std::min(a, b), std::max(a, b));
  }
  Expected square_of(const interval& x)
  {
    const double a = std::fabs(x.left()), b = std::fabs(x.right());
    const double aa = times(a, a), bb = times(b, b);
    return expect_bounds((x.left() <= 0.0 && x.right() >= 0.0) ? 0.0 : std::min(aa, bb), std::max(aa, bb));
  }
  // The roots of 0, 4 and +oo are 0, 2 and +oo
  Expected root_of(const interval& x)
  {
    if (x.right() < 0.0) {
      return expect_empty();
    }
    const double a = std::max(x.left(), 0.0), b = x.right();
    return expect_bounds((a == 4.0) ? 2.0 : a, (b == 4.0) ? 2.0 : b);
  }

  struct InfiniteOperation
  {
    const char *name;
    std::string (*run)();
  };

  const InfiniteOperation arithmetic_with_infinite_bounds[] = {
    { "x + y", [] { return over_pairs(" + ", [](const interval& x, const interval& y) { return x + y; }, sum_of); } },
    { "x - y", [] { return over_pairs(" - ", [](const interval& x, const interval& y) { return x - y; }, difference_of); } },
    { "x += y", [] { return over_pairs(" += ", [](const interval& x, const interval& y) { interval z(x); z += y; return z; }, sum_of); } },
    { "x -= y", [] { return over_pairs(" -= ", [](const interval& x, const interval& y) { interval z(x); z -= y; return z; }, difference_of); } },
    { "sqr(x)", [] { return over_intervals("sqr", [](const interval& x) { return sqr(x); }, square_of); } },
    { "sqrt(x)", [] { return over_intervals("sqrt", [](const interval& x) { return sqrt(x); }, root_of); } },
    { "x + d", [] { return over_doubles(" + ", [](const interval& x, double d) { return x + d; }, sum_with); } },
    { "d + x", [] { return over_doubles(" + ", [](const interval& x, double d) { return d + x; }, sum_with); } },
    { "x - d", [] { return over_doubles(" - ", [](const interval& x, double d) { return x - d; }, difference_with); } },
    { "d - x", [] { return over_doubles(" -from- ", [](const interval& x, double d) { return d - x; }, difference_from); } },
    { "x * d", [] { return over_doubles(" * ", [](const interval& x, double d) { return x * d; }, product_with); } },
    { "d * x", [] { return over_doubles(" * ", [](const interval& x, double d) { return d * x; }, product_with); } },
    { "x / d", [] { return over_doubles(" / ", [](const interval& x, double d) { return x / d; }, quotient_by); } },
    { "d / x", [] { return over_doubles(" divided-by ", [](const interval& x, double d) { return d / x; }, quotient_of); } },
    { "x % d", [] { return over_doubles(" % ", [](const interval& x, double d) { return x % d; }, remainder_by); } },
    { "d % x", [] { return over_doubles(" remainder-of ", [](const interval& x, double d) { return d % x; }, quotient_of); } },
    { "x += d", [] { return over_doubles(" += ", [](const interval& x, double d) { interval z(x); z += d; return z; }, sum_with); } },
    { "x -= d", [] { return over_doubles(" -= ", [](const interval& x, double d) { interval z(x); z -= d; return z; }, difference_with); } },
    { "x *= d", [] { return over_doubles(" *= ", [](const interval& x, double d) { interval z(x); z *= d; return z; }, product_with); } },
    { "x /= d", [] { return over_doubles(" /= ", [](const interval& x, double d) { interval z(x); z /= d; return z; }, quotient_by); } },
    { "x %= d", [] { return over_doubles(" %= ", [](const interval& x, double d) { interval z(x); z %= d; return z; }, remainder_by); } },
  };

#if GAOL_TESTS_FPCR_SEQUENCE
  /*
    FPCR, the control register of 64-bit ARM processors, read with mrs and
    written with msr, and FPSCR of the 32-bit ones, with vmrs and vmsr (GAOL
    v5), apart from the library: whatever the
    intervals, NEON or FPU, the operations write it, and the tests below check
    what they write and when. Its bits: the rounding direction (RMode, bits
    23:22), the modes that flush the subnormals to zero (FZ, bit 24, and FIZ,
    bit 0 from Armv8.7), which GAOL clears and sets back with
    GAOL_PRESERVE_ROUNDING, and the others, which the program owns and no
    operation may change: the default-NaN mode (DN, bit 25), the flush of the
    half-precision subnormals (FZ16, bit 19) and the enables of the traps of
    the five exceptions (IOE, DZE, OFE, UFE, IXE, bits 8 to 12) and of the
    input denormal (IDE, bit 15). Not every processor retains them: the bits
    that one does not are not checked, and the test says so.
  */
  unsigned long long read_fpcr()
  {
#if defined(__aarch64__)
    unsigned long long fpcr;
    __asm__ __volatile__ ("mrs %0, fpcr" : "=r" (fpcr));
    return fpcr;
#else
    unsigned int fpscr; // its exception flags and its comparison flags are in the register too
    __asm__ __volatile__ ("vmrs %0, fpscr" : "=r" (fpscr));
    return fpscr;
#endif
  }

  void write_fpcr(unsigned long long fpcr)
  {
#if defined(__aarch64__)
    __asm__ __volatile__ ("msr fpcr, %0" : : "r" (fpcr) : "memory");
#else
    const unsigned int fpscr = static_cast<unsigned int>(fpcr);
    __asm__ __volatile__ ("vmsr fpscr, %0" : : "r" (fpscr) : "memory");
#endif
  }

  std::string fpcr_text(unsigned long long fpcr)
  {
    char text[32];
    std::snprintf(text, sizeof text, "0x%llx", fpcr);
    return text;
  }

  const unsigned long long fpcr_rmode_bits = 0xc00000ull;
#if defined(__aarch64__)
  const unsigned long long fpcr_flush_bits = 0x1000001ull; // FZ and FIZ
  const unsigned long long fpcr_dn_fz16_bits = 0x2080000ull;
  const unsigned long long fpcr_flag_bits = 0ull; // the flags of the exceptions are in FPSR
#else
  // FPSCR of 32-bit ARM: FZ alone (bit 0 is the flag of the invalid-operation
  // exception), DN alone (no FZ16), and the flags of the exceptions and of the
  // comparisons, which the operations change: N, Z, C, V, QC, IDC and IXC to IOC
  const unsigned long long fpcr_flush_bits = 0x1000000ull;
  const unsigned long long fpcr_dn_fz16_bits = 0x2000000ull;
  const unsigned long long fpcr_flag_bits = 0xf800009full;
#endif
  const unsigned long long fpcr_mode_bits = fpcr_rmode_bits | fpcr_flush_bits;
  const unsigned long long fpcr_upward_bits = 0x400000ull;
  const unsigned long long fpcr_fz_bit = 0x1000000ull;
  const unsigned long long fpcr_ide_bit = 0x8000ull;
  // The bits no operation of GAOL may change: neither the mode bits nor the flags
  const unsigned long long fpcr_owned_bits = ~(fpcr_mode_bits | fpcr_flag_bits);

  // The rounding directions as the field RMode holds them, upward last
  const unsigned long long fpcr_directions[4] = { 0x0ull, 0x800000ull, 0xc00000ull, 0x400000ull };
  const char *const fpcr_direction_names[4] = { "to nearest", "downward", "toward zero", "upward" };

  /*
    A write of FPCR right before an operation (GAOL v5): the operation reads
    FPCR, writes it upward when it is not, and computes. The program's msr is
    followed by no isb and by no read of FPCR; the library's msr is followed by
    nothing but the barrier of the compiler. A processor that did not apply a
    write of FPCR to the instructions that follow it in program order, or a
    compiler that computed the vector operation before the write, would give
    bounds of another direction at times, which a loop of 2^16 operations per case,
    the four directions in turn, shows. The results are compared with the ones
    of the same operands with the direction upward, which are the tightest
    enclosures: they are checked with exact arithmetic beforehand.
  */
  void sequence_in_a_loop(const TightCase& c, const std::vector<interval>& x, const std::vector<interval>& y,
                          std::vector<interval>& r, unsigned long long base)
  {
    const std::size_t k = x.size();
    for (std::size_t i = 0; i < r.size(); ++i) {
      write_fpcr(base | fpcr_directions[i & 3]);
      r[i] = c.run(x[i % k], y[i % k]);
    }
  }

  bool same_result(const interval& a, const interval& b)
  {
    return a.is_empty() ? b.is_empty() : (!b.is_empty() && a.left() == b.left() && a.right() == b.right());
  }

  /*
    Operations whose results are exact, on [1, 2] and [4, 8]: none raises an
    exception flag, bar the inexact one the probe of the direction raises in a
    build with the FPU intervals (the test tells which, and enables the traps
    of the exceptions that none raised only). They are run with the traps of FPCR
    enabled, where the processor retains the enable bits: an operation that
    writes FPCR whole, or that raised an exception, would kill the program
  */
  const EmptyOperand exact_operations[] = {
    { "x + y", [] { return (make(1.0, 2.0) + make(4.0, 8.0)).set_eq(interval(5.0, 10.0)); } },
    { "x - y", [] { return (make(1.0, 2.0) - make(4.0, 8.0)).set_eq(interval(-7.0, -2.0)); } },
    { "x * y", [] { return (make(1.0, 2.0) * make(4.0, 8.0)).set_eq(interval(4.0, 16.0)); } },
    { "y / x", [] { return (make(4.0, 8.0) / make(1.0, 2.0)).set_eq(interval(2.0, 8.0)); } },
    { "y % x", [] { return (make(4.0, 8.0) % make(1.0, 2.0)).set_eq(interval(2.0, 8.0)); } },
    { "x += y", [] { interval z = make(1.0, 2.0); z += make(4.0, 8.0); return z.set_eq(interval(5.0, 10.0)); } },
    { "x -= y", [] { interval z = make(1.0, 2.0); z -= make(4.0, 8.0); return z.set_eq(interval(-7.0, -2.0)); } },
    { "x *= y", [] { interval z = make(1.0, 2.0); z *= make(4.0, 8.0); return z.set_eq(interval(4.0, 16.0)); } },
    { "y /= x", [] { interval z = make(4.0, 8.0); z /= make(1.0, 2.0); return z.set_eq(interval(2.0, 8.0)); } },
    { "y %= x", [] { interval z = make(4.0, 8.0); z %= make(1.0, 2.0); return z.set_eq(interval(2.0, 8.0)); } },
    { "x + d", [] { return (make(1.0, 2.0) + 2.0).set_eq(interval(3.0, 4.0)); } },
    { "x - d", [] { return (make(1.0, 2.0) - 0.5).set_eq(interval(0.5, 1.5)); } },
    { "x * d", [] { return (make(1.0, 2.0) * 0.5).set_eq(interval(0.5, 1.0)); } },
    { "x / d", [] { return (make(1.0, 2.0) / 0.5).set_eq(interval(2.0, 4.0)); } },
    { "x % d", [] { return (make(1.0, 2.0) % 0.5).set_eq(interval(2.0, 4.0)); } },
    { "x += d", [] { interval z = make(1.0, 2.0); z += 2.0; return z.set_eq(interval(3.0, 4.0)); } },
    { "x -= d", [] { interval z = make(1.0, 2.0); z -= 0.5; return z.set_eq(interval(0.5, 1.5)); } },
    { "x *= d", [] { interval z = make(1.0, 2.0); z *= 0.5; return z.set_eq(interval(0.5, 1.0)); } },
    { "x /= d", [] { interval z = make(1.0, 2.0); z /= 0.5; return z.set_eq(interval(2.0, 4.0)); } },
    { "x %= d", [] { interval z = make(1.0, 2.0); z %= 0.5; return z.set_eq(interval(2.0, 4.0)); } },
    { "sqr(x)", [] { return sqr(make(1.0, 2.0)).set_eq(interval(1.0, 4.0)); } },
    { "sqrt(y)", [] { return sqrt(make(4.0, 16.0)).set_eq(interval(2.0, 4.0)); } },
    { "inverse(y)", [] { return inverse(make(4.0, 8.0)).set_eq(interval(0.125, 0.25)); } },
  };
#endif // GAOL_TESTS_FPCR_SEQUENCE

  /*
    The midpoint of an interval with a bound of 2^1023 or more in magnitude,
    finite (GAOL v5, point D.24): midpoint() and mid() computed the sum of the
    bounds first, which overflowed for [DBL_MAX] and raised the overflow
    exception, before they took the sum of the halves. The midpoints are
    computed by midpoints_of(), which calls GAOL only, and checked against the
    exact midpoint, computed with dyadic numbers, apart.
  */
  struct HugeInterval
  {
    const char *name;
    double l, r;
  };

  /*
    The intervals, made at run time from doubles read back from volatile
    memory: GCC 12.1 to 12.3 and 13.1 to 13.2 with -frounding-math wrote the
    long double 2^-1074 into a double member of an aggregate given
    std::numeric_limits<double>::denorm_min(), as a table at namespace scope
    or in a function. Its significand read as -0 and the rest overwrote the
    next member, so that [2^-1074, DBL_MAX] was [-0, 0x3bcd 2^-1074] on
    x86-64, where its checks passed, and [-0, NaN], the empty set, for 32-bit
    x86, where the checks of its midpoints failed (Debian 12 i386 with GCC
    12.2, MinGW-w64 12.2 and 13.2 for x86), though midpoint() and mid() give
    the right ones there. A scalar double or an array of doubles initialized
    so have the right value, as with GCC 12.4, 13.3 and 14.
  */
  std::vector<HugeInterval> huge_intervals()
  {
    const double dbl_max = gaol_core::rnd_keep((std::numeric_limits<double>::max)());
    const double smallest_subnormal = gaol_core::rnd_keep(std::numeric_limits<double>::denorm_min());
    const double two_1023 = gaol_core::rnd_keep(0x1p1023);
    std::vector<HugeInterval> intervals;
    intervals.push_back(HugeInterval{ "[DBL_MAX]", dbl_max, dbl_max });
    intervals.push_back(HugeInterval{ "[-DBL_MAX]", -dbl_max, -dbl_max });
    intervals.push_back(HugeInterval{ "[1e308, DBL_MAX]", gaol_core::rnd_keep(1e308), dbl_max });
    intervals.push_back(HugeInterval{ "[-DBL_MAX, -2^1023]", -dbl_max, -two_1023 });
    intervals.push_back(HugeInterval{ "[2^-1074, DBL_MAX]", smallest_subnormal, dbl_max });
    return intervals;
  }

  struct Midpoints
  {
    double midpoint, rad, mid_rad_m, mid_rad_r;
    interval mid, split_l, split_r, split_left, split_right;

    bool operator==(const Midpoints& m) const
    {
      return std::memcmp(&midpoint, &m.midpoint, sizeof midpoint) == 0 && std::memcmp(&rad, &m.rad, sizeof rad) == 0
        && mid_rad_m == m.mid_rad_m && mid_rad_r == m.mid_rad_r && mid.set_eq(m.mid) && split_l.set_eq(m.split_l)
        && split_r.set_eq(m.split_r) && split_left.set_eq(m.split_left) && split_right.set_eq(m.split_right);
    }
  };

  Midpoints midpoints_of(const HugeInterval& h)
  {
    const interval x = opaque(interval(gaol_core::rnd_keep(h.l), gaol_core::rnd_keep(h.r)));
    Midpoints m;
    m.midpoint = x.midpoint();
    m.rad = x.rad();
    x.mid_rad(m.mid_rad_m, m.mid_rad_r);
    m.mid = x.mid();
    x.split(m.split_l, m.split_r);
    m.split_left = x.split_left();
    m.split_right = x.split_right();
    return m;
  }

  bool midpoints_are_right(const HugeInterval& h, const Midpoints& m)
  {
    const Exact middle = exact(dyadic(h.l)*dyadic(0.5) + dyadic(h.r)*dyadic(0.5));
    const double c = m.midpoint;
    return is_tightest_enclosure(m.mid, middle) && m.mid.set_contains(c) && (h.l != h.r || c == h.l)
      && m.mid_rad_m == c && m.mid_rad_r == m.rad && m.rad >= 0.0 && (h.l != h.r || m.rad == 0.0)
      && bounds_are(m.split_l, h.l, c) && bounds_are(m.split_r, c, h.r)
      && m.split_left.set_eq(m.split_l) && m.split_right.set_eq(m.split_r);
  }

  /*
    floor_rel(Z, X) of a Z ending at DBL_MAX, whose set ends at DBL_MAX + 1,
    beyond the doubles (GAOL v5, point P.25): its upper end is +oo, which
    floor_rel() takes without computing DBL_MAX + 1, whose overflow raised the
    overflow exception where the result has no infinite bound:
    floor_rel([DBL_MAX], [0, 1]) is empty and floor_rel([0, DBL_MAX], [0, 1])
    is [0, 1]
  */
  bool floor_rel_at_dbl_max_is_right()
  {
    const double dbl_max = gaol_core::rnd_keep((std::numeric_limits<double>::max)());
    const interval x = opaque(interval(gaol_core::rnd_keep(0.0), gaol_core::rnd_keep(1.0)));
    const interval a = floor_rel(opaque(interval(dbl_max)), x);
    const interval b = floor_rel(opaque(interval(gaol_core::rnd_keep(0.0), dbl_max)), x);
    return a.is_empty() && bounds_are(b, 0.0, 1.0);
  }

  /*
    The relations in loops, which a compiler vectorizes or if-converts (GAOL
    v5, point D.24): the comparison that follows a test of the empty set is
    then made for the empty operands too, and <, <=, >= or > raise the
    invalid-operation exception on their NaN bounds, as GCC 9.4 at -O3 made
    for the FPU intervals the constructor raise it (and with -mavx2,
    certainly_positive(), less() and others). Every third x and every fifth y
    is empty; each result is compared with the relation computed on its own,
    outside the loop.
  */
#if defined(__GNUC__) || defined(__clang__)
#  define GAOL_TESTS_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define GAOL_TESTS_NOINLINE __declspec(noinline)
#else
#  define GAOL_TESTS_NOINLINE
#endif

  const int loop_size = 64;

  // name() computes expression of the arrays x and y for each index i, and
  // name_at() for one pair, outside any loop
#define GAOL_TESTS_RELATION_LOOP(name, expression)                                                    \
  GAOL_TESTS_NOINLINE void name(const interval *x, const interval *y, unsigned char *result)          \
  {                                                                                                    \
    static_cast<void>(y);                                                                              \
    for (int i = 0; i < loop_size; ++i) {                                                              \
      result[i] = static_cast<unsigned char>(expression);                                              \
    }                                                                                                  \
  }                                                                                                    \
  unsigned char name##_at(const interval& x_at, const interval& y_at)                                  \
  {                                                                                                    \
    const interval *x = &x_at, *y = &y_at;                                                             \
    static_cast<void>(y);                                                                              \
    const int i = 0;                                                                                   \
    return static_cast<unsigned char>(expression);                                                     \
  }

  GAOL_TESTS_RELATION_LOOP(loop_less, x[i].less(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_strictly_less, x[i].strictly_less(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_is_symmetric, x[i].is_symmetric())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_positive, x[i].certainly_positive())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_negative, x[i].certainly_negative())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_strictly_positive, x[i].certainly_strictly_positive())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_strictly_negative, x[i].certainly_strictly_negative())
  GAOL_TESTS_RELATION_LOOP(loop_certainly_ge, x[i].certainly_ge(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_certainly_geq, x[i].certainly_geq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_certainly_le, x[i].certainly_le(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_certainly_leq, x[i].certainly_leq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_is_zero, x[i].is_zero())
  GAOL_TESTS_RELATION_LOOP(loop_is_a_double, x[i].is_a_double())
  GAOL_TESTS_RELATION_LOOP(loop_is_an_int, x[i].is_an_int())
  GAOL_TESTS_RELATION_LOOP(loop_is_entire, x[i].is_entire())
  GAOL_TESTS_RELATION_LOOP(loop_is_empty, x[i].is_empty())
  GAOL_TESTS_RELATION_LOOP(loop_straddles_zero, x[i].straddles_zero())
  GAOL_TESTS_RELATION_LOOP(loop_set_contains, x[i].set_contains(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_contains_double, x[i].set_contains(1.5))
  GAOL_TESTS_RELATION_LOOP(loop_set_strictly_contains, x[i].set_strictly_contains(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_strictly_contains_double, x[i].set_strictly_contains(1.5))
  GAOL_TESTS_RELATION_LOOP(loop_set_eq, x[i].set_eq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_disjoint, x[i].set_disjoint(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_le, x[i].set_le(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_set_leq, x[i].set_leq(y[i]))
  GAOL_TESTS_RELATION_LOOP(loop_operators, (x[i] <= y[i]) + 2*(x[i] < y[i]) + 4*(x[i] >= 1.5) + 8*(x[i] > 1.5))
  GAOL_TESTS_RELATION_LOOP(loop_constructor, interval(x[i].left(), y[i].right()).is_empty())
  GAOL_TESTS_RELATION_LOOP(loop_intersection, (x[i] & y[i]).is_empty())
  GAOL_TESTS_RELATION_LOOP(loop_floor, floor(x[i]).is_empty())

  struct RelationLoop
  {
    const char *name;
    void (*loop)(const interval *x, const interval *y, unsigned char *result);
    unsigned char (*at)(const interval& x, const interval& y);
  };

  const RelationLoop relation_loops[] = {
    { "less()", loop_less, loop_less_at },
    { "strictly_less()", loop_strictly_less, loop_strictly_less_at },
    { "is_symmetric()", loop_is_symmetric, loop_is_symmetric_at },
    { "certainly_positive()", loop_certainly_positive, loop_certainly_positive_at },
    { "certainly_negative()", loop_certainly_negative, loop_certainly_negative_at },
    { "certainly_strictly_positive()", loop_certainly_strictly_positive, loop_certainly_strictly_positive_at },
    { "certainly_strictly_negative()", loop_certainly_strictly_negative, loop_certainly_strictly_negative_at },
    { "certainly_ge()", loop_certainly_ge, loop_certainly_ge_at },
    { "certainly_geq()", loop_certainly_geq, loop_certainly_geq_at },
    { "certainly_le()", loop_certainly_le, loop_certainly_le_at },
    { "certainly_leq()", loop_certainly_leq, loop_certainly_leq_at },
    { "is_zero()", loop_is_zero, loop_is_zero_at },
    { "is_a_double()", loop_is_a_double, loop_is_a_double_at },
    { "is_an_int()", loop_is_an_int, loop_is_an_int_at },
    { "is_entire()", loop_is_entire, loop_is_entire_at },
    { "is_empty()", loop_is_empty, loop_is_empty_at },
    { "straddles_zero()", loop_straddles_zero, loop_straddles_zero_at },
    { "set_contains()", loop_set_contains, loop_set_contains_at },
    { "set_contains(double)", loop_set_contains_double, loop_set_contains_double_at },
    { "set_strictly_contains()", loop_set_strictly_contains, loop_set_strictly_contains_at },
    { "set_strictly_contains(double)", loop_set_strictly_contains_double, loop_set_strictly_contains_double_at },
    { "set_eq()", loop_set_eq, loop_set_eq_at },
    { "set_disjoint()", loop_set_disjoint, loop_set_disjoint_at },
    { "set_le()", loop_set_le, loop_set_le_at },
    { "set_leq()", loop_set_leq, loop_set_leq_at },
    { "<=, < and >=, > with a double", loop_operators, loop_operators_at },
    { "interval(l, r)", loop_constructor, loop_constructor_at },
    { "&", loop_intersection, loop_intersection_at },
    { "floor()", loop_floor, loop_floor_at },
  };

  // The operands of the loops, from operands the compiler does not know
  void loop_operands(std::vector<interval>& x, std::vector<interval>& y)
  {
    x.resize(loop_size);
    y.resize(loop_size);
    for (std::size_t i = 0; i < x.size(); ++i) {
      const double d = static_cast<double>(i);
      x[i] = (i % 3 == 0) ? E() : make(d - 10.0, d + 0.5);
      y[i] = (i % 5 == 0) ? E() : make(0.5*d, d + 1.0);
    }
  }

  // Runs loop on the operands, and compares its results with at()
  bool relation_loop_is_right(const RelationLoop& r)
  {
    std::vector<interval> x, y;
    loop_operands(x, y);
    unsigned char result[loop_size];
    r.loop(x.data(), y.data(), result);
    bool right = true;
    for (std::size_t i = 0; i < x.size(); ++i) {
      right = right && result[i] == r.at(opaque(x[i]), opaque(y[i]));
    }
    return right;
  }

#if GAOL_TESTS_TRAPS
  // How a child process that enabled the floating-point exceptions of excepts
  // and ran f() ended: f() returned true or false, or the process was stopped
  // by SIGFPE, which the processor raises for an exception that is enabled, or
  // otherwise. The exit status tells which: 40 and 41 are the answers of f(),
  // 42 the SIGFPE the child caught, 43 an exception that could not be enabled.
  enum Outcome { returned_true, returned_false, trapped, other };

  const int status_true = 40, status_false = 41, status_trapped = 42, status_not_enabled = 43;

  void exit_on_sigfpe(int)
  {
    _exit(status_trapped);
  }

  Outcome run_with_exceptions_enabled(int excepts, const std::function<bool()>& f)
  {
    std::fflush(stdout);
    std::fflush(stderr);
    const pid_t child = fork();
    if (child < 0) {
      return other;
    }
    if (child == 0) {
      // The SIGFPE is caught to exit with a status of its own: the sanitizers
      // report it as an error and abort otherwise, and no core file is wanted
      // for the one that would kill the process where the handler is ignored
      const struct rlimit no_core = { 0, 0 };
      setrlimit(RLIMIT_CORE, &no_core);
      struct sigaction action;
      std::memset(&action, 0, sizeof action);
      action.sa_handler = exit_on_sigfpe;
      sigemptyset(&action.sa_mask);
      sigaction(SIGFPE, &action, nullptr);
      std::feclearexcept(FE_ALL_EXCEPT);
      if (feenableexcept(excepts) == -1) {
        _exit(status_not_enabled);
      }
      const bool result = f();
      fedisableexcept(FE_ALL_EXCEPT);
      _exit(result ? status_true : status_false);
    }
    int status = 0;
    pid_t waited;
    do {
      waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited != child) {
      return other;
    }
    if (WIFSIGNALED(status)) {
      return WTERMSIG(status) == SIGFPE ? trapped : other;
    }
    if (WIFEXITED(status)) {
      switch (WEXITSTATUS(status)) {
        case status_true: return returned_true;
        case status_false: return returned_false;
        case status_trapped: return trapped;
      }
    }
    return other;
  }

  const char *outcome_text(Outcome o)
  {
    switch (o) {
      case returned_true: return "returned true";
      case returned_false: return "returned false";
      case trapped: return "died on SIGFPE";
      default: return "ended otherwise (an error, another signal)";
    }
  }

  // An invalid operation, 0/0, which traps where the processor delivers the
  // exception: the control of the checks below, whose result is only
  // meaningful there
  bool invalid_operation()
  {
    volatile double zero = 0.0;
    volatile double quotient = zero/zero;
    return quotient != quotient;
  }

  // An overflow, 2 DBL_MAX: the control of the checks with the overflow
  // exception enabled
  bool overflow_operation()
  {
    volatile double big = (std::numeric_limits<double>::max)();
    volatile double product = 2.0*big;
    return product > big;
  }
#endif // GAOL_TESTS_TRAPS
}

int main()
{
  gaol::init();
#if !GAOL_PRESERVE_ROUNDING
  check("rounding upward after gaol::init()", std::fegetround() == FE_UPWARD);
#endif

  // What gaol_core::x86_*() decide on the registers of a 32-bit x86 processor,
  // simulated, on every platform (GAOL v5)
  x86_registers_simulated();

  // x within [0,1] and y above 1, the domain of every function below
  Random random;
  std::vector<std::pair<interval, interval> > operands;
  operands.push_back(std::make_pair(interval(0.1, 0.3), interval(1.5, 2.5)));
  for (int i = 0; i < 5; ++i) {
    // One draw per declarator, in order: the two arguments of std::make_pair
    // were evaluated in an order the compiler chose, and gave other operands
    // with other compilers
    const double a = random.uniform(0.01, 0.6), b = random.uniform(1.1, 2.0);
    const double wa = random.uniform(0.001, 0.39), wb = random.uniform(0.001, 1.0);
    operands.push_back(std::make_pair(interval(a, a + wa), interval(b, b + wb)));
  }

  const Operation operations[] = {
    { "interval()", [](const interval&, const interval&) { interval z; return S(z); } },
    { "interval(a,b)", [](const interval&, const interval&) { return S(interval(0x1.999999999999ap-4, 0x1.5555555555555p-2)); } },
    { "interval(a)", [](const interval&, const interval&) { return S(interval(0.1)); } },
    { "textToInterval(const std::string&)", [](const interval&, const interval&) { return S(textToInterval("[0.1, 1/3]")); } },
    { "textToInterval(sl, sr)", [](const interval&, const interval&) { return S(textToInterval("0.1", "0.3")); } },
    { "textToInterval(\"sin(1)+exp(0.1)\")", [](const interval&, const interval&) { return S(textToInterval("sin(1)+exp(0.1)")); } },
    { "textToInterval(\"3.56?1e-2\")", [](const interval&, const interval&) { return S(textToInterval("3.56?1e-2")); } },
    { "textToInterval(\"[-0x1.00000000000001p0, 2/3]\")", [](const interval&, const interval&) { return S(textToInterval("[-0x1.00000000000001p0, 2/3]")); } },
    { "operator+", [](const interval& x, const interval& y) { return S(x + y); } },
    { "operator-", [](const interval& x, const interval& y) { return S(x - y); } },
    { "operator*", [](const interval& x, const interval& y) { return S(x * y); } },
    { "operator/", [](const interval& x, const interval& y) { return S(x / y); } },
    { "operator+(interval,double)", [](const interval& x, const interval&) { return S(x + 0.1); } },
    { "operator-(double,interval)", [](const interval& x, const interval&) { return S(0.1 - x); } },
    { "operator*(double,interval)", [](const interval& x, const interval&) { return S(0.1 * x); } },
    { "operator/(interval,double)", [](const interval& x, const interval&) { return S(x / 0.3); } },
    { "operator+=(interval)", [](const interval& x, const interval& y) { interval z(x); z += y; return S(z); } },
    { "operator-=(interval)", [](const interval& x, const interval& y) { interval z(x); z -= y; return S(z); } },
    { "operator*=(interval)", [](const interval& x, const interval& y) { interval z(x); z *= y; return S(z); } },
    { "operator/=(interval)", [](const interval& x, const interval& y) { interval z(x); z /= y; return S(z); } },
    { "operator%=(interval)", [](const interval& x, const interval& y) { interval z(x); z %= y; return S(z); } },
    { "operator+=(double)", [](const interval& x, const interval&) { interval z(x); z += 0.1; return S(z); } },
    { "operator-=(double)", [](const interval& x, const interval&) { interval z(x); z -= 0.1; return S(z); } },
    { "operator*=(double)", [](const interval& x, const interval&) { interval z(x); z *= 0.1; return S(z); } },
    { "operator/=(double)", [](const interval& x, const interval&) { interval z(x); z /= 0.1; return S(z); } },
    { "operator%=(double)", [](const interval& x, const interval&) { interval z(x); z %= 0.1; return S(z); } },
    { "operator&=", [](const interval& x, const interval& y) { interval z(x); z &= y; return S(z); } },
    { "operator|=", [](const interval& x, const interval& y) { interval z(x); z |= y; return S(z); } },
    { "operator/ by an interval containing 0", [](const interval& x, const interval&) { return S(x/interval(-1., 1.)); } },
    { "unary -", [](const interval& x, const interval&) { return S(-x); } },
    { "inverse()", [](const interval&, const interval& y) { return S(y.inverse()); } },
    { "midpoint()", [](const interval& x, const interval&) { return S(x.midpoint()); } },
    { "mid()", [](const interval& x, const interval&) { return S(x.mid()); } },
    { "width()", [](const interval& x, const interval&) { return S(x.width()); } },
    { "rad()", [](const interval& x, const interval&) { return S(x.rad()); } },
    { "mid_rad()", [](const interval& x, const interval&) { double m, r; x.mid_rad(m, r); return S(m) + " " + S(r); } },
    { "mig()", [](const interval& x, const interval&) { return S(x.mig()); } },
    { "mag()", [](const interval& x, const interval&) { return S(x.mag()); } },
    { "smig()", [](const interval& x, const interval&) { return S(x.smig()); } },
    { "split()", [](const interval& x, const interval&) { interval l, r; x.split(l, r); return S(l) + " " + S(r); } },
    { "split_left()", [](const interval& x, const interval&) { return S(x.split_left()); } },
    { "split_right()", [](const interval& x, const interval&) { return S(x.split_right()); } },
    { "is_canonical()", [](const interval& x, const interval&) { return S(x.is_canonical()); } },
    { "certainly_le()", [](const interval& x, const interval& y) { return S(x.certainly_le(y)); } },
    { "set_disjoint()", [](const interval& x, const interval& y) { return S(x.set_disjoint(y)); } },
    { "set_contains()", [](const interval& x, const interval& y) { return S(x.set_contains(y)); } },
    { "operator<=", [](const interval& x, const interval& y) { return S(x <= y); } },
    { "operator<", [](const interval& x, const interval& y) { return S(x < y); } },
    { "operator std::string", [](const interval& x, const interval&) { const std::string s = x; return s; } },
    { "operator<< (bounds)", [](const interval& x, const interval&) { return write(x, interval_format::bounds); } },
    { "operator<< (width)", [](const interval& x, const interval&) { return write(x, interval_format::width); } },
    { "operator<< (center)", [](const interval& x, const interval&) { return write(x, interval_format::center); } },
    { "operator<< (hexa)", [](const interval& x, const interval&) { return write(x, interval_format::hexa); } },
    { "operator<< (agreeing)", [](const interval& x, const interval&) { return write(x, interval_format::agreeing); } },
    { "operator<< of a degenerate interval", [](const interval&, const interval&) { return write(interval(0.1), interval_format::bounds); } },
    { "gaol_ieee1788::intervalToText", [](const interval& x, const interval&) {
        return gaol_ieee1788::intervalToText(x) + " " + gaol_ieee1788::intervalToText(interval(0.1)); } },
    { "operator>>", [](const interval&, const interval&) { std::istringstream s("[0.1, 0.3]"); interval z; s >> z; return S(z); } },
    { "sqr", [](const interval& x, const interval&) { return S(sqr(x)); } },
    { "pow(x,int)", [](const interval&, const interval& y) { return S(pow(y, 3)); } },
    { "pow(x,-int)", [](const interval&, const interval& y) { return S(pow(y, -3)); } },
    { "pow(x,int) for a large int", [](const interval&, const interval& y) { return S(pow(y, 1000)); } },
    { "pow(x,-int) for a large int", [](const interval&, const interval& y) { return S(pow(y, -1000)); } },
    { "pow(x,unsigned) beyond the ints", [](const interval&, const interval& y) { return S(gaol::pow(y, 3000000001u)); } },
    { "pow(x,long long) beyond 2^53", [](const interval&, const interval& y) { return S(gaol::pow(y, 9007199254740993LL)); } },
    { "pow(x,[n]) beyond the ints", [](const interval&, const interval& y) { return S(pow(y, interval(-5e9))); } },
    { "pow(x,interval)", [](const interval& x, const interval& y) { return S(pow(y, x)); } },
    { "pow(x,double)", [](const interval&, const interval& y) { return S(pow(y, 2.5)); } },
    { "pow(x,interval) with a negative base", [](const interval&, const interval&) { return S(pow(interval(-4., 9.), interval(0.5))); } },
    { "sqrt", [](const interval& x, const interval&) { return S(sqrt(x)); } },
    { "nth_root", [](const interval&, const interval& y) { return S(nth_root(y, 3)); } },
    { "exp", [](const interval& x, const interval&) { return S(exp(x)); } },
    { "log", [](const interval& x, const interval&) { return S(log(x)); } },
    { "cos", [](const interval& x, const interval&) { return S(cos(x)); } },
    { "sin", [](const interval& x, const interval&) { return S(sin(x)); } },
    { "tan", [](const interval& x, const interval&) { return S(tan(x)); } },
    { "acos", [](const interval& x, const interval&) { return S(acos(x)); } },
    { "asin", [](const interval& x, const interval&) { return S(asin(x)); } },
    { "atan", [](const interval& x, const interval&) { return S(atan(x)); } },
    { "atan2", [](const interval& x, const interval& y) { return S(atan2(x, y)); } },
    { "atan2 across the half-line y = 0, x < 0", [](const interval& x, const interval&) { return S(atan2(x - x, -abs(x))); } },
    { "cosh", [](const interval& x, const interval&) { return S(cosh(x)); } },
    { "sinh", [](const interval& x, const interval&) { return S(sinh(x)); } },
    { "tanh", [](const interval& x, const interval&) { return S(tanh(x)); } },
    { "acosh", [](const interval&, const interval& y) { return S(acosh(y)); } },
    { "asinh", [](const interval& x, const interval&) { return S(asinh(x)); } },
    { "atanh", [](const interval& x, const interval&) { return S(atanh(x)); } },
    { "erf", [](const interval& x, const interval&) { return S(erf(x)); } },
    { "erfc", [](const interval& x, const interval&) { return S(erfc(x)); } },
    { "abs", [](const interval& x, const interval&) { return S(abs(x)); } },
    { "min", [](const interval& x, const interval& y) { return S(min(x, y)); } },
    { "max", [](const interval& x, const interval& y) { return S(max(x, y)); } },
    { "floor", [](const interval&, const interval& y) { return S(floor(y)); } },
    { "ceil", [](const interval&, const interval& y) { return S(ceil(y)); } },
    { "integer", [](const interval&, const interval& y) { return S(integer(y)); } },
    { "chi", [](const interval& x, const interval&) { return S(chi(x)); } },
    { "hausdorff", [](const interval& x, const interval& y) { return S(hausdorff(x, y)); } },
    { "sqrt_rel", [](const interval& x, const interval& y) { return S(sqrt_rel(y, x)); } },
    { "div_rel", [](const interval& x, const interval& y) { return S(div_rel(y, y, x)); } },
    { "nth_root_rel", [](const interval& x, const interval& y) { return S(nth_root_rel(y, 3, x)); } },
    { "acos_rel", [](const interval& x, const interval& y) { return S(acos_rel(y, x)); } },
    { "asin_rel", [](const interval& x, const interval&) { return S(asin_rel(x, x)); } },
    { "atan_rel", [](const interval& x, const interval&) { return S(atan_rel(x, x)); } },
    { "acosh_rel", [](const interval& x, const interval& y) { return S(acosh_rel(x, y)); } },
    { "asinh_rel", [](const interval& x, const interval&) { return S(asinh_rel(x, x)); } },
    { "atanh_rel", [](const interval& x, const interval&) { return S(atanh_rel(x, x)); } },
    { "invabs_rel", [](const interval& x, const interval& y) { return S(invabs_rel(x, y)); } },
    // The reverse functions of pow, their bounds proved by powers (GAOL v5)
    { "pow_rel", [](const interval& x, const interval& y) {
        return S(pow_rel(y, x, interval::universe())) + " " + S(pow_rel(x, -y, interval::universe())) + " "
          + S(pow_rel(y, x - y, interval(0.5, 4.0))); } },
    { "pow_exponent_rel", [](const interval& x, const interval& y) {
        return S(pow_exponent_rel(y, y, interval::universe())) + " " + S(pow_exponent_rel(y, x, interval::universe())) + " "
          + S(pow_exponent_rel(x, y + x, interval(-1.0, 0.5))); } },
    { "max_rel", [](const interval& x, const interval& y) { return S(max_rel(y, x, y - 1.0)) + " " + S(max_rel(y, y, x + y)); } },
    { "min_rel", [](const interval& x, const interval& y) { return S(min_rel(x, y, 2.0*x)) + " " + S(min_rel(x, x, y - x)); } },
    { "sign_rel", [](const interval& x, const interval&) {
        return S(sign_rel(interval(1.0), x - 0.5)) + " " + S(sign_rel(interval(-1.0, 0.0), x - 0.5)); } },
    // floor(sup Z) + 1 rounded upward: 2^60 + 1 is 2^60 rounded to nearest
    { "floor_rel", [](const interval& x, const interval& y) {
        return S(floor_rel(y, x + y)) + " " + S(floor_rel(interval(0x1p60), interval::universe())); } },
    { "atan2_rel_y", [](const interval& x, const interval& y) { return S(atan2_rel_y(x, y, y)); } },
    { "atan2_rel_x", [](const interval& x, const interval& y) { return S(atan2_rel_x(x, y, y)); } },
    { "nb_fp_numbers", [](const interval& x, const interval&) { return S(nb_fp_numbers(x.left(), x.right())); } },
    // Operations that compute after their one check of the rounding direction
    // what they computed with operations of intervals checking it again, or
    // that the operations above do not reach (GAOL v5)
    { "tan next to a pole", [](const interval& x, const interval&) {
        return S(tan(x + 1.0)) + " " + S(tan(interval(1.5, 0x1.921fb54442d18p+0))); } },
    { "acos_rel, asin_rel and atan_rel on their pieces", [](const interval& x, const interval& y) {
        return S(acos_rel(x, y)) + " " + S(acos_rel(x + 0.8, y)) + " " + S(asin_rel(x - 1.5, y)) + " " + S(atan_rel(y, x + 4.0)); } },
    { "atan2_rel_y and atan2_rel_x in the four quadrants", [](const interval& x, const interval& y) {
        return S(atan2_rel_y(x + 2.0, -y, y)) + " " + S(atan2_rel_x(-x - 2.0, -y, -y)) + " " + S(atan2_rel_y(-x, y, -y))
          + " " + S(atan2_rel_x(x - 4.0, -y, -y)) + " " + S(atan2_rel_y(x*0x1p-1070, y, y)); } },
    { "nth_root(x,n) for n >= 4", [](const interval&, const interval& y) { return S(nth_root(y, 4)) + " " + S(nth_root(-y, 5)); } },
    // The roots of negative order, the integer powers of pow(x, y),
    // pow(x, double) and gaol_ieee1788::pow, and modulo_k_pi(), which compute
    // their bounds after one check of the rounding direction (GAOL v5)
    { "nth_root(x,q) for q < 0", [](const interval& x, const interval& y) {
        return S(nth_root(y, -1)) + " " + S(nth_root(y, -2)) + " " + S(nth_root(-y, -3)) + " " + S(nth_root(x, -4)) + " "
          + S(nth_root(-y, -5)) + " " + S(nth_root(y, INT_MIN)); } },
    // An order beyond the ints and the unsigned ints (GAOL v5)
    { "nth_root(x,q) beyond the unsigned ints", [](const interval& x, const interval& y) {
        return S(nth_root(y, 1LL << 33)) + " " + S(nth_root(-y, (1LL << 33) + 1)) + " " + S(nth_root(x, -(1LL << 33) - 1))
          + " " + S(nth_root(-y, (1ULL << 34) + 1)); } },
    { "pow(x,[n]), pow(x,n.0) and gaol_ieee1788::pow(x,[n]) for n < 0", [](const interval& x, const interval& y) {
        return S(pow(y, interval(-3.0))) + " " + S(pow(-y, interval(-3.0))) + " " + S(pow(y, -3.0)) + " "
          + S(gaol_ieee1788::pow(y, interval(-3.0))) + " " + S(gaol_ieee1788::pow(x, interval(-2.0))); } },
    // [a, b], a/gaol_pi_up and b/gaol_pi_dn being just below 3 and 5: their floors are
    // 2 and 5 rounding upward, and 3 and 4 downward. modulo_k_pi() is called
    // on it first, before the operations of intervals of the other operands
    // set the direction upward
    { "modulo_k_pi next to multiples of pi", [](const interval& x, const interval& y) {
        const double a = 0x1.2d97c7f3321d2p+3, b = 0x1.f6a7a2955385dp+3;
        double k_left = 0.0, k_right = 0.0;
        std::string s = S(static_cast<int>(modulo_k_pi(interval(a, b), k_left, k_right)));
        s += " " + S(k_left) + " " + S(k_right);
        s += " " + S(static_cast<int>(modulo_k_pi(interval(-b, -a), k_left, k_right)));
        s += " " + S(k_left) + " " + S(k_right);
        for (const interval& I : { interval::pi()*3.0, x*y*100.0, interval(-1e-310, 1e-300) }) {
          s += " " + S(static_cast<int>(modulo_k_pi(I, k_left, k_right)));
          s += " " + S(k_left) + " " + S(k_right);
        }
        return s; } },
    { "pow(x,n) from the rounded products", [](const interval&, const interval& y) {
        return S(pow(y*0x1p-330, 3)) + " " + S(pow(interval(0., 1.)*y, 3)) + " " + S(pow(y*1e200, 3)); } },
    { "pow(x,-n) where x^n overflows", [](const interval&, const interval& y) {
        return S(pow(y*1e200, -2)) + " " + S(pow((y - 2.)*1e200, -2)) + " " + S(pow((y - 2.)*1e200, -3)) + " "
          + S(pow(interval(0., 1.)*y, -1)); } },
    { "gaol_ieee1788::pow with an integer exponent beyond the ints", [](const interval&, const interval& y) {
        return S(gaol_ieee1788::pow(y, interval(2147483649.))) + " " + S(gaol_ieee1788::pow(interval(0.5, 0.9), interval(1e10))) + " "
          + S(gaol_ieee1788::pow(1. + (y - 1.)*0x1p-40, interval(-2147483649.))); } },
    // Powers with a subnormal result, which CORE-MATH rounds by itself in the
    // direction fegetround() gives, where the doubles GAOL and CORE-MATH
    // compute are rounded in the direction of MXCSR (GAOL v5)
    { "gaol_ieee1788::pow with a subnormal result", [](const interval&, const interval&) {
        std::string s;
        for (const SubnormalPower& p : subnormal_powers) {
          s += S(gaol_ieee1788::pow(interval(p.x), interval(p.y))) + " ";
        }
        return s; } },
    { "feven", [](const interval&, const interval&) { return S(feven(2.0)); } },
    { "interval::pi()", [](const interval&, const interval&) { return S(interval::pi()); } },
    { "interval::precision(n)", [](const interval&, const interval&) { const int p = static_cast<int>(interval::precision(17)); interval::precision(p); return S(p); } },
  };

  set(directions[0]);
#if !GAOL_PRESERVE_ROUNDING
  // The state an operation may leave without GAOL_PRESERVE_ROUNDING, read
  // only then: GCC 15 refused it as set but not used with -Werror (GAOL v5)
  const State upward = state();
#endif

  for (const Operation& op : operations) {
    const std::string name = op.name;
    for (const std::pair<interval, interval>& operand : operands) {
      const interval& x = operand.first;
      const interval& y = operand.second;
      set(directions[0]);
      const std::string reference = run(op, x, y);
      for (const Direction& d : directions) {
        set(d);
        const State before = state();
        const std::string result = run(op, x, y);
        const State after = state();
        const auto describe = [&] { return std::string("rounding direction ") + d.name + ", x=" + hex(x) + " y=" + hex(y); };
        check(name + ": the same result whatever the rounding direction", result == reference,
              [&] { return describe() + ": " + result + " rather than " + reference; });
#if GAOL_PRESERVE_ROUNDING
        check("rounding direction restored after " + name, after == before,
              [&] { return describe() + ": " + text(after) + " after it, " + text(before) + " before"; });
#else
        check("rounding direction unchanged or upward after " + name, after == before || after == upward,
              [&] { return describe() + ": " + text(after) + " after it, " + text(before) + " before"; });
#endif

        // A product and a sum whose exact results are not doubles, as the
        // random mantissas make almost certain: their bounds are the tightest
        // ones only if they are computed upward
        const double a = random(-30, 30), b = random(-30, 30);
        const interval p = interval(a) * interval(b), s = interval(a) + interval(b);
        check("[a]*[b] the tightest enclosure after " + name, is_tightest_enclosure(p, exact(dyadic(a)*dyadic(b))),
              [&] { return describe() + ", a=" + hex(a) + " b=" + hex(b) + ": " + hex(p); });
        check("[a]+[b] the tightest enclosure after " + name, is_tightest_enclosure(s, exact(dyadic(a) + dyadic(b))),
              [&] { return describe() + ", a=" + hex(a) + " b=" + hex(b) + ": " + hex(s); });
        // and a difference, a quotient, a square and a square root (GAOL v5)
        const interval df = interval(a) - interval(b), q = interval(a) / interval(b), sq = sqr(interval(a)),
                       rt = sqrt(interval(std::fabs(a)));
        check("[a]-[b] the tightest enclosure after " + name, is_tightest_enclosure(df, exact(dyadic(a) - dyadic(b))),
              [&] { return describe() + ", a=" + hex(a) + " b=" + hex(b) + ": " + hex(df); });
        check("[a]/[b] the tightest enclosure after " + name, is_tightest_enclosure(q, quotient(dyadic(a), dyadic(b))),
              [&] { return describe() + ", a=" + hex(a) + " b=" + hex(b) + ": " + hex(q); });
        check("sqr([a]) the tightest enclosure after " + name, is_tightest_enclosure(sq, exact(dyadic(a)*dyadic(a))),
              [&] { return describe() + ", a=" + hex(a) + ": " + hex(sq); });
        check("sqrt([a]) the tightest enclosure after " + name, is_tightest_root(rt, interval(std::fabs(a))),
              [&] { return describe() + ", a=" + hex(a) + ": " + hex(rt); });
      }
    }
  }

  // Products and sums in a loop changing the rounding direction before each
  {
    const std::size_t n = 600, nb_directions = sizeof(directions)/sizeof(directions[0]);
    std::vector<double> a(n), b(n);
    for (std::size_t i = 0; i < n; ++i) {
      a[i] = random(-30, 30);
      b[i] = random(-30, 30);
    }
    std::vector<interval> p(n), s(n);
    products_and_sums_in_a_loop(a, b, p, s);
    set(directions[0]);
    for (std::size_t i = 0; i < n; ++i) {
      const auto describe = [&] { return std::string("rounding direction ") + directions[i % nb_directions].name
                                         + ", a=" + hex(a[i]) + " b=" + hex(b[i]); };
      check("[a]*[b] the tightest enclosure in a loop changing the rounding direction",
            is_tightest_enclosure(p[i], exact(dyadic(a[i])*dyadic(b[i]))), [&] { return describe() + ": " + hex(p[i]); });
      check("[a]+[b] the tightest enclosure in a loop changing the rounding direction",
            is_tightest_enclosure(s[i], exact(dyadic(a[i]) + dyadic(b[i]))), [&] { return describe() + ": " + hex(s[i]); });
    }
  }

#if GAOL_TESTS_X86_32
  /*
    The registers themselves, on a 32-bit x86 processor (GAOL v5): the control
    word of the x87 unit and MXCSR, read and written by the test apart from the
    library, in each of the six states of the two rounding fields, and with each
    of the modes that flush the subnormals to zero set. After an operation that
    checks the direction, both units round upward and the modes are clear, or
    with GAOL_PRESERVE_ROUNDING, as they were found (the registers that the
    SSE2 operations leave alone, the x87 unit among them, being found as they
    were); no other bit of the registers changes, but the exception flags,
    which an operation adds to; and the bounds are the tightest ones when the
    registers have just been written by the program (the check of the
    direction reads them, and nothing before it has reused the previous read).
  */
  {
    const std::size_t nb_directions = sizeof(register_directions)/sizeof(register_directions[0]);
    static_assert(sizeof(directions)/sizeof(directions[0]) == sizeof(register_directions)/sizeof(register_directions[0]),
                  "the six states of the tests");
    const std::vector<FlushMode> modes = flush_modes();
    std::vector<unsigned int> flushes(1, 0u);
    for (const FlushMode& m : modes) {
      flushes.push_back(m.bits);
    }
    const interval x(0.1, 0.3), y(1.5, 2.5);
    for (std::size_t d = 0; d < nb_directions; ++d) {
      for (unsigned int flush : flushes) {
        for (const RegisterOperation& op : register_operations) {
          write_x86_fields(register_directions[d], flush);
          const unsigned int cw_before = read_x87_word(), csr_before = static_cast<unsigned int>(_mm_getcsr());
          const interval r = op.run(x, y);
          const unsigned int cw_after = read_x87_word(), csr_after = static_cast<unsigned int>(_mm_getcsr());
          write_x86_fields(register_directions[0], 0u);
          const auto describe = [&] {
            return std::string(op.name) + " in " + directions[d].name + " with the modes " + x86_hex(flush) + ": control word "
                 + x86_hex(cw_before) + " then " + x86_hex(cw_after) + ", MXCSR " + x86_hex(csr_before) + " then " + x86_hex(csr_after)
                 + ", " + hex(r);
          };
#if GAOL_PRESERVE_ROUNDING
          const unsigned int x87_wanted = cw_before & 0x0c00u, sse_wanted = csr_before & 0x6000u, flush_wanted = flush;
#else
          const unsigned int x87_wanted = 0x0800u, sse_wanted = 0x4000u, flush_wanted = 0u;
#endif
          check("the control word of the x87 unit: the direction after an operation", (cw_after & 0x0c00u) == x87_wanted, describe);
          check("MXCSR: the direction after an operation", (csr_after & 0x6000u) == sse_wanted, describe);
          check("MXCSR: the modes that flush the subnormals to zero after an operation", (csr_after & 0x8040u) == flush_wanted, describe);
          check("the control word of the x87 unit: no other bit changes", (cw_after & x87_not_direction) == (cw_before & x87_not_direction), describe);
          check("MXCSR: no other bit changes, but the exception flags", (csr_after & mxcsr_not_direction) == (csr_before & mxcsr_not_direction), describe);
          check("the exception flags of an operation are added to the ones the program had", (csr_after & 0x003fu) == ((csr_after & 0x003fu) | (csr_before & 0x003fu)), describe);
        }
      }
    }

    // The program writes the registers, and an operation comes right after
    Random random_registers;
    const std::size_t n = 600;
    std::vector<double> a(n), b(n);
    for (std::size_t i = 0; i < n; ++i) {
      a[i] = random_registers(-30, 30);
      b[i] = random_registers(-30, 30);
    }
    for (unsigned int flush : flushes) {
      std::vector<interval> sum(n), product(n);
      x86_sequence_in_a_loop(a, b, sum, product, flush);
      write_x86_fields(register_directions[0], 0u);
      for (std::size_t i = 0; i < n; ++i) {
        const auto describe = [&] {
          return std::string("in ") + directions[i % nb_directions].name + " with the modes " + x86_hex(flush) + ", a=" + hex(a[i]) + " b=" + hex(b[i]);
        };
        check("[a]+[b] the tightest enclosure right after the program wrote the control registers",
              is_tightest_enclosure(sum[i], exact(dyadic(a[i]) + dyadic(b[i]))), [&] { return describe() + ": " + hex(sum[i]); });
        check("[a]*[b] the tightest enclosure right after the program wrote the control registers",
              is_tightest_enclosure(product[i], exact(dyadic(a[i])*dyadic(b[i]))), [&] { return describe() + ": " + hex(product[i]); });
      }
    }
    set(directions[0]);
  }
#endif // GAOL_TESTS_X86_32

  // The difference, the quotient, the remainder, the square, the square root,
  // the inverse and their forms with a double and as compound assignments, in
  // a loop changing the rounding direction before each of them (GAOL v5)
  {
    Random random_tight;
    const std::size_t n = 96, nb_directions = sizeof(directions)/sizeof(directions[0]);
    for (const TightCase& c : tight_cases) {
      std::vector<interval> x(n), y(n), r(n);
      for (std::size_t i = 0; i < n; ++i) {
        tight_operands(random_tight, c, x[i], y[i]);
      }
      tight_cases_in_a_loop(c, x, y, r);
      set(directions[0]);
      for (std::size_t i = 0; i < n; ++i) {
        check(std::string(c.name) + ": the tightest enclosure in a loop changing the rounding direction",
              is_tightest_result(c, r[i], x[i], y[i]),
              [&] {
                return std::string("rounding direction ") + directions[i % nb_directions].name + ", x=" + hex(x[i])
                     + " y=" + hex(y[i]) + ": " + hex(r[i]);
              });
      }
    }
  }

#if GAOL_TESTS_FPCR_SEQUENCE
  /*
    The same operations right after the program wrote FPCR, in the four
    directions in turn (see sequence_in_a_loop()), 2^16 times each
  */
  {
    Random random_sequence;
    const std::size_t k = 7, n = static_cast<std::size_t>(1) << 16; // k prime to 4, so that each direction meets all the operands
    const unsigned long long saved = read_fpcr();
    const unsigned long long base = saved & ~fpcr_rmode_bits;
    std::vector<interval> r(n);
    for (const TightCase& c : tight_cases) {
      std::vector<interval> x(k), y(k), reference(k);
      for (std::size_t j = 0; j < k; ++j) {
        tight_operands(random_sequence, c, x[j], y[j]);
      }
      write_fpcr(base | fpcr_upward_bits);
      for (std::size_t j = 0; j < k; ++j) {
        reference[j] = c.run(x[j], y[j]);
      }
      write_fpcr(saved);
      for (std::size_t j = 0; j < k; ++j) {
        check(std::string(c.name) + ": the tightest enclosure with the direction upward, the reference of the sequence of writes of FPCR",
              is_tightest_result(c, reference[j], x[j], y[j]), [&] { return "x=" + hex(x[j]) + " y=" + hex(y[j]) + ": " + hex(reference[j]); });
      }
      sequence_in_a_loop(c, x, y, r, base);
      write_fpcr(saved);
      std::size_t differing[4] = { 0, 0, 0, 0 }, first[4] = { 0, 0, 0, 0 };
      for (std::size_t i = 0; i < n; ++i) {
        if (!same_result(r[i], reference[i % k])) {
          if (differing[i & 3] == 0) {
            first[i & 3] = i;
          }
          ++differing[i & 3];
        }
      }
      for (std::size_t d = 0; d < 4; ++d) {
        check(std::string(c.name) + ": the same result right after the program wrote FPCR, in each direction", differing[d] == 0,
              [&] {
                const std::size_t i = first[d];
                return std::string("rounding direction ") + fpcr_direction_names[d] + ": " + std::to_string(differing[d])
                     + " results differ, the first x=" + hex(x[i % k]) + " y=" + hex(y[i % k]) + ": " + hex(r[i])
                     + " rather than " + hex(reference[i % k]);
              });
      }
    }
  }
#endif // GAOL_TESTS_FPCR_SEQUENCE

  /*
    The exception flags the program raised, and its exception masks, are
    kept by every operation, whichever way GAOL is built (GAOL v5, point D.24
    of TODO.md): with GAOL_PRESERVE_ROUNDING, the SSE2 operations (+, -, *,
    /, %, sqr(), inverse(), the operations with a double...) wrote the SSE
    control register whole, every exception masked and every flag cleared.
    Each operation is run with the five flags of IEEE 754 raised, and on x86
    the six of the SSE control register: they have to be raised after it, and
    the masks unchanged. Where feraiseexcept() raises no flag that
    fetestexcept() reads back (an emulator, a processor without the flags),
    this is not checked, and the test says so. An exception the program
    enabled is checked below, with the invalid-operation exception enabled.
    With mingw-w64 on x86-64 the operations are run from each of the six
    directions, not from upward only: the functions that set the direction to
    nearest and back (midpoint(), chi()...) changed MXCSR through fesetround()
    from the other directions only, which cleared its flags there (GAOL v5,
    point 110 of TODO.md).
  */
  std::feclearexcept(FE_ALL_EXCEPT);
  std::feraiseexcept(FE_ALL_EXCEPT);
  const bool flags_readable = std::fetestexcept(FE_ALL_EXCEPT) == FE_ALL_EXCEPT;
  std::feclearexcept(FE_ALL_EXCEPT);
  if (!flags_readable) {
    std::printf("The exception flags raised by feraiseexcept() are not read back here: the flags kept by "
                "the operations are not checked\n");
  } else {
#if GAOL_TESTS_X87_STATE
    const std::size_t nb_flag_directions = sizeof(directions)/sizeof(directions[0]);
#else
    const std::size_t nb_flag_directions = 1;
#endif
    for (const Operation& op : operations) {
      for (std::size_t k = 0; k < nb_flag_directions; ++k) {
        const std::string name = op.name;
        const std::string from = std::string(" from ") + directions[k].name;
        set(directions[k]);
        std::feraiseexcept(FE_ALL_EXCEPT);
#if GAOL_TESTS_SSE
        _mm_setcsr(_mm_getcsr() | 0x3fu); // the six flags of MXCSR, denormal-operand included
        const unsigned masks_before = _mm_getcsr() & _MM_MASK_MASK;
#endif
        static_cast<void>(run(op, operands[0].first, operands[0].second));
        const int raised = std::fetestexcept(FE_ALL_EXCEPT);
#if GAOL_TESTS_SSE
        const unsigned csr = _mm_getcsr();
#endif
        std::feclearexcept(FE_ALL_EXCEPT);
        check("the exception flags the program raised kept by " + name, raised == FE_ALL_EXCEPT,
              [&] { return "flags " + std::to_string(raised) + " after it" + from; });
#if GAOL_TESTS_SSE
        check("the SSE exception flags and masks kept by " + name, (csr & 0x3fu) == 0x3fu && (csr & _MM_MASK_MASK) == masks_before,
              [&] { return "MXCSR " + std::to_string(csr) + " after it, masks " + std::to_string(masks_before) + " before" + from; });
#endif
      }
    }
    set(directions[0]);
  }

#if GAOL_TESTS_FPCR_SEQUENCE
  /*
    FPCR itself (GAOL v5, see read_fpcr()); FPSCR on 32-bit ARM, whose flags the
    operations change, and whose other bits (DN, the enables of the traps) the
    test sets as far as the processor retains them. After each operation of the table
    above, run from each direction with and without FZ set, the bits of FPCR
    other than the rounding direction and the flush modes are the ones the
    program had. They are set to a pattern first: the default-NaN mode (DN)
    and FZ16, flipped from what the program has, those of them that the
    processor retains (written, read back; FIZ, which the processors before
    Armv8.7 do not have, is not part of it). With GAOL_PRESERVE_ROUNDING, the
    rounding direction and the flush modes are the ones the program had; else
    the direction is the one it had or upward, and the modes the ones it had
    or cleared: the operations that make no check leave both as they were,
    and the reader of numbers sets the direction upward and leaves the modes
    (rounding_guard).
    Then the traps: the bits that enable them, set where the processor
    retains them, for operations whose results are exact. A first run with
    the traps disabled shows which exceptions they raise, none in a build
    with the NEON intervals; the traps of those they raise (the inexact
    exception the probe of the direction raises with the FPU intervals) are
    left disabled, so that no operation can trap. Each operation gives its
    result and leaves the bits as they were.
  */
  std::feclearexcept(FE_ALL_EXCEPT);
  {
    const unsigned long long saved = read_fpcr();
    const auto retained = [saved](unsigned long long candidates) {
      write_fpcr(saved | candidates);
      const unsigned long long got = read_fpcr() & candidates;
      write_fpcr(saved);
      return got;
    };
    const unsigned long long pattern = retained(fpcr_dn_fz16_bits);
    const unsigned long long flush = retained(fpcr_fz_bit);
    if (pattern != fpcr_dn_fz16_bits) {
      std::printf("FPCR retains %s here: the bits other than the rounding direction and the flush modes "
                  "are checked without it\n", pattern == 0 ? "neither DN nor FZ16" : "only one of DN and FZ16");
    }
    std::vector<unsigned long long> flushes(1, 0ull);
    if (flush != 0) {
      flushes.push_back(flush);
    }
    for (std::size_t d = 0; d < 4; ++d) {
      for (const unsigned long long fz : flushes) {
        // The bits of the pattern flipped from what the program has, so that they differ from it
        const unsigned long long start = ((saved & ~fpcr_mode_bits) ^ pattern) | fpcr_directions[d] | fz;
        for (const Operation& op : operations) {
          write_fpcr(start);
          static_cast<void>(run(op, operands[0].first, operands[0].second));
          const unsigned long long after = read_fpcr();
          write_fpcr(saved);
          const auto describe = [&] {
            return std::string("rounding direction ") + fpcr_direction_names[d] + (fz != 0 ? ", FZ set" : "")
                 + ": FPCR " + fpcr_text(after) + " after it, " + fpcr_text(start) + " before";
          };
          check("the bits of FPCR other than the rounding direction and the flush modes kept by " + std::string(op.name),
                ((after ^ start) & fpcr_owned_bits) == 0ull, describe);
#if GAOL_PRESERVE_ROUNDING
          check("the rounding direction and the flush modes of FPCR restored by " + std::string(op.name),
                (after & fpcr_mode_bits) == (start & fpcr_mode_bits), describe);
#else
          // The reader of numbers sets the direction upward, and leaves the modes (rounding_guard)
          check("the rounding direction of FPCR unchanged or upward after " + std::string(op.name),
                (after & fpcr_rmode_bits) == (start & fpcr_rmode_bits) || (after & fpcr_rmode_bits) == fpcr_upward_bits, describe);
          check("the flush modes of FPCR unchanged or cleared after " + std::string(op.name),
                (after & fpcr_flush_bits) == (start & fpcr_flush_bits) || (after & fpcr_flush_bits) == 0ull, describe);
#endif
        }
      }
    }

    // cancel_minus() and cancel_plus() make their TwoSum to nearest and set the direction back
    // (GAOL_RND_NEAREST_LEAVE()), which the table above does not reach
    const Operation nearest_operations[] = {
      { "cancel_minus", [](const interval& x, const interval& y) { return S(cancel_minus(y, x)); } },
      { "cancel_plus", [](const interval& x, const interval& y) { return S(cancel_plus(y, x)); } },
    };
    for (std::size_t d = 0; d < 4; ++d) {
      for (const unsigned long long fz : flushes) {
        const unsigned long long start = ((saved & ~fpcr_mode_bits) ^ pattern) | fpcr_directions[d] | fz;
        for (const Operation& op : nearest_operations) {
          write_fpcr(start);
          static_cast<void>(run(op, operands[0].first, operands[0].second));
          const unsigned long long after = read_fpcr();
          write_fpcr(saved);
          check("the bits of FPCR other than the rounding direction and the flush modes kept by " + std::string(op.name),
                ((after ^ start) & fpcr_owned_bits) == 0ull,
                [&] {
                  return std::string("rounding direction ") + fpcr_direction_names[d] + (fz != 0 ? ", FZ set" : "")
                       + ": FPCR " + fpcr_text(after) + " after it, " + fpcr_text(start) + " before";
                });
        }
      }
    }

    // The operations with exact results (below) all check the direction: after one, the direction
    // is upward and the flush modes are cleared, or they are the ones the program had with
    // GAOL_PRESERVE_ROUNDING
    for (std::size_t d = 0; d < 4; ++d) {
      for (const unsigned long long fz : flushes) {
        const unsigned long long start = ((saved & ~fpcr_mode_bits) ^ pattern) | fpcr_directions[d] | fz;
        for (const EmptyOperand& e : exact_operations) {
          write_fpcr(start);
          const bool right = e.run();
          const unsigned long long after = read_fpcr();
          write_fpcr(saved);
          const auto describe = [&] {
            return std::string(e.name) + ", rounding direction " + fpcr_direction_names[d] + (fz != 0 ? ", FZ set" : "")
                 + ": FPCR " + fpcr_text(after) + " after it, " + fpcr_text(start) + " before";
          };
          check("an operation with exact results gives them from each direction, with and without FZ", right, describe);
#if GAOL_PRESERVE_ROUNDING
          check("the rounding direction and the flush modes of FPCR restored by an operation with exact results",
                (after & fpcr_mode_bits) == (start & fpcr_mode_bits), describe);
#else
          check("the rounding direction of FPCR upward and the flush modes cleared by an operation with exact results",
                (after & fpcr_mode_bits) == fpcr_upward_bits, describe);
#endif
        }
      }
    }

    // The traps: a control run with the exceptions masked, then the same operations with the traps
    // of the exceptions that none of them raised enabled. Not the underflow one: with it enabled,
    // a processor signals the underflow exception for every tiny result, exact ones included (an
    // Apple M2 does, to the sum of a subnormal and 0 that the probe of the direction of the FPU
    // intervals computes), which the flags of a run with the exception masked do not show
    struct Trap
    {
      int excepts;
      unsigned long long bit;
    };
    const Trap traps[] = { { FE_INVALID, 0x100ull }, { FE_DIVBYZERO, 0x200ull }, { FE_OVERFLOW, 0x400ull },
                           { FE_INEXACT, 0x1000ull } };
    std::feclearexcept(FE_ALL_EXCEPT);
    bool control_right = true;
    for (const EmptyOperand& e : exact_operations) {
      control_right = e.run() && control_right;
    }
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    std::feclearexcept(FE_ALL_EXCEPT);
    check("the operations with exact results give them", control_right);
    unsigned long long candidates = fpcr_ide_bit;
    for (const Trap& t : traps) {
      if ((raised & t.excepts) == 0) {
        candidates |= t.bit;
      }
    }
    const unsigned long long trap_bits = retained(candidates);
    if (trap_bits == 0ull) {
      std::printf("FPCR retains none of the bits that enable the traps here: the operations with the "
                  "traps enabled are not checked\n");
    } else {
      for (std::size_t d = 0; d < 4; ++d) {
        for (const EmptyOperand& e : exact_operations) {
          const unsigned long long start = (saved & ~fpcr_mode_bits) | trap_bits | fpcr_directions[d];
          write_fpcr(start);
          const bool right = e.run();
          const unsigned long long after = read_fpcr();
          write_fpcr(saved);
          const auto describe = [&] {
            return std::string("rounding direction ") + fpcr_direction_names[d] + ", traps " + fpcr_text(trap_bits)
                 + ": FPCR " + fpcr_text(after) + " after it, " + fpcr_text(start) + " before";
          };
          check("an operation with exact results gives them with the traps of FPCR enabled", right,
                [&] { return std::string(e.name) + ", " + describe(); });
          check("the traps of FPCR kept enabled by an operation with exact results", ((after ^ start) & fpcr_owned_bits) == 0ull,
                [&] { return std::string(e.name) + ", " + describe(); });
        }
      }
    }
    write_fpcr(saved);
    std::feclearexcept(FE_ALL_EXCEPT);
  }
#endif // GAOL_TESTS_FPCR_SEQUENCE

#if GAOL_TESTS_FLUSH
  /*
    The modes that flush the subnormals to zero (GAOL v5): flush-to-zero,
    denormals-are-zero and both on x86, FZ (and FIZ, where the processor has
    it) on ARM. With one of them set, an operation whose result or operand is
    subnormal gave a zero: [1e-300]*[1e-20] was [0, 0], when a program linked
    with -Ofast, whose crtfastmath.o sets them after GAOL initialized itself,
    or a plug-in built so, set them, and the rounding direction upward that the
    probe of each operation looked at was still there; the FPU intervals
    compared the bounds, and the operations with a double the double, with 0
    before their probe: [1e-300]/[100*2^-1074] was the empty set. The direction
    is tried with each of the modes, upward included, which is what such a
    program has. The bounds have to be the tightest ones, computed apart with
    exact rational arithmetic, or with mpmath for the exponential: the operands,
    the exact values and the bounds are read with the modes cleared, which
    change frexp(), on which the exact arithmetic of the tests relies, for a
    subnormal. The modes are cleared after the operation, or restored with
    GAOL_PRESERVE_ROUNDING. tests/fast_math_link.cpp checks the program linked
    with -ffast-math.
  */
  const std::vector<FlushMode> flush_modes_honoured = flush_modes();
  {
    const Dyadic half = dyadic(0.5);
    struct Case
    {
      const char *name;
      interval x, y;
      interval (*run)(const interval&, const interval&);
      Exact value; // the exact result, or a number between the same two doubles
    };
    const Case cases[] = {
      // Subnormal results of normal operands
      { "[1e-300]*[1e-20]", interval(1e-300), interval(1e-20),
        [](const interval& x, const interval& y) { return x*y; },
        exact(dyadic(1e-300)*dyadic(1e-20)) },
      { "[1e-300]/[1e20]", interval(1e-300), interval(1e20),
        [](const interval& x, const interval& y) { return x/y; },
        quotient(dyadic(1e-300), dyadic(1e20)) },
      // The exact difference, a subnormal
      { "[3e-308]-[2.9e-308]", interval(3e-308), interval(2.9e-308),
        [](const interval& x, const interval& y) { return x - y; },
        exact(dyadic(3e-308) - dyadic(2.9e-308)) },
      { "sqr([1e-160])", interval(1e-160), interval(),
        [](const interval& x, const interval&) { return sqr(x); },
        exact(dyadic(1e-160)*dyadic(1e-160)) },
      // Subnormal operands
      { "[100*2^-1074]*[1e10]", interval(subnormal(100)), interval(1e10),
        [](const interval& x, const interval& y) { return x*y; },
        exact(dyadic(subnormal(100))*dyadic(1e10)) },
      { "[100*2^-1074]+[3*2^-1074]", interval(subnormal(100)), interval(subnormal(3)),
        [](const interval& x, const interval& y) { return x + y; },
        exact(dyadic(subnormal(100)) + dyadic(subnormal(3))) },
      { "[100*2^-1074]/[3]", interval(subnormal(100)), interval(3.0),
        [](const interval& x, const interval& y) { return x/y; },
        quotient(dyadic(subnormal(100)), dyadic(3.0)) },
      // A normal result of a subnormal divisor
      { "[1e-300]/[100*2^-1074]", interval(1e-300), interval(subnormal(100)),
        [](const interval& x, const interval& y) { return x/y; },
        quotient(dyadic(1e-300), dyadic(subnormal(100))) },
      // A subnormal double, a literal rather than subnormal(100), which the
      // modes would flush: the operations compared it with 0 before their check
      // of the rounding direction, and it was 0
      { "[1e10]*(100*2^-1074)", interval(1e10), interval(),
        [](const interval& x, const interval&) { return x*0x64p-1074; },
        exact(dyadic(1e10)*dyadic(subnormal(100))) },
      { "[1e-300]/(100*2^-1074)", interval(1e-300), interval(),
        [](const interval& x, const interval&) { return x/0x64p-1074; },
        quotient(dyadic(1e-300), dyadic(subnormal(100))) },
      // exp(-740) is 84.78 times 2^-1074 (mpmath), between the same two
      // doubles as 84.5 times 2^-1074
      { "exp([-740])", interval(-740.0), interval(),
        [](const interval& x, const interval&) { return exp(x); },
        exact(dyadic(subnormal(169))*half) },
      // erf(100*2^-1074) is 112.84 times 2^-1074, and erfc(27) 105999.05
      // times 2^-1074 (mpmath): between the same two doubles as 112.5 and
      // 105999.5 times 2^-1074 (GAOL v5)
      { "erf([100*2^-1074])", interval(subnormal(100)), interval(),
        [](const interval& x, const interval&) { return erf(x); },
        exact(dyadic(subnormal(225))*half) },
      { "erfc([27])", interval(27.0), interval(),
        [](const interval& x, const interval&) { return erfc(x); },
        exact(dyadic(subnormal(211999))*half) },
    };

    for (const FlushMode& m : flush_modes_honoured) {
      for (const Case& c : cases) {
        for (const Direction& d : directions) {
          set(d);
          set_flush_bits(m.bits);
          const unsigned int found = flush_bits();
          const interval r = c.run(c.x, c.y);
          const unsigned int left = flush_bits();
          set_flush_bits(0u);
          const auto describe = [&] {
            return m.name + " set, rounding direction " + d.name + ": modes " + std::to_string(left) + " after it, "
                 + std::to_string(found) + " before";
          };
          check(std::string(c.name) + " the tightest enclosure with a flush-to-zero mode", is_tightest_enclosure(r, c.value),
                [&] { return describe() + ", " + hex(r); });
#if GAOL_PRESERVE_ROUNDING
          check(std::string(c.name) + ": flush-to-zero modes restored", left == found, describe);
#else
          check(std::string(c.name) + ": flush-to-zero modes cleared", left == 0u, describe);
#endif
        }
      }
    }
    set(directions[0]);
  }

  /*
    Every operation that checks the rounding direction, with each of the modes
    set just before it, gives what it gives with the modes cleared, which the
    cases above show to be the tightest bounds, and leaves them cleared, or
    restored with GAOL_PRESERVE_ROUNDING (GAOL v5, review of point 4).
    Its check clears the modes before it reads, compares or copies a bound, and
    with GAOL_PRESERVE_ROUNDING it makes its result before it restores them;
    the compilers do not model the modes, so this is checked on what each
    compiler of the continuous integration emits. With GCC 13 at -O3, asinpi()
    compared the bounds it had read with 0 before the check, and with the modes
    set asinpi([100*2^-1074]) was [32*2^-1074], above the exact value,
    31.83*2^-1074 (mpmath); sqrt([-1e-310, 4]), whose intersection with
    [0, +oo] kept -1e-310 under denormals-are-zero, was empty once the check
    had cleared it; atan2([100*2^-1074], [100*2^-1074]) was empty, the point
    taken for (0, 0); and with GAOL_PRESERVE_ROUNDING sinpi([100, 1000]*2^-1074)
    was empty, its minimum taken once the modes were restored. acos_rel() and
    asin_rel() of a J outside [-1, 1], or containing it, returned before their
    check, with the right result but the modes still set. The functions
    that make no check are tried after these. Nothing of the C library runs
    while a mode is set: the results are read and written with the modes
    cleared.
  */
  {
    struct FlushOperation
    {
      const char *name;
      int arity; // 1: x, 2: x and y, a double being y.left(), 3: x, y and z
      interval (*run)(const interval& x, const interval& y, const interval& z);
    };
    const FlushOperation flush_operations[] = {
      { "sqr(x)", 1, [](const interval& x, const interval&, const interval&) { return sqr(x); } },
      { "sqrt(x)", 1, [](const interval& x, const interval&, const interval&) { return sqrt(x); } },
      { "rsqrt(x)", 1, [](const interval& x, const interval&, const interval&) { return rsqrt(x); } },
      { "inverse(x)", 1, [](const interval& x, const interval&, const interval&) { return inverse(x); } },
      { "exp(x)", 1, [](const interval& x, const interval&, const interval&) { return exp(x); } },
      { "exp2(x)", 1, [](const interval& x, const interval&, const interval&) { return exp2(x); } },
      { "exp10(x)", 1, [](const interval& x, const interval&, const interval&) { return exp10(x); } },
      { "expm1(x)", 1, [](const interval& x, const interval&, const interval&) { return expm1(x); } },
      { "exp2m1(x)", 1, [](const interval& x, const interval&, const interval&) { return exp2m1(x); } },
      { "exp10m1(x)", 1, [](const interval& x, const interval&, const interval&) { return exp10m1(x); } },
      { "log(x)", 1, [](const interval& x, const interval&, const interval&) { return log(x); } },
      { "log2(x)", 1, [](const interval& x, const interval&, const interval&) { return log2(x); } },
      { "log10(x)", 1, [](const interval& x, const interval&, const interval&) { return log10(x); } },
      { "log1p(x)", 1, [](const interval& x, const interval&, const interval&) { return log1p(x); } },
      { "log2p1(x)", 1, [](const interval& x, const interval&, const interval&) { return log2p1(x); } },
      { "log10p1(x)", 1, [](const interval& x, const interval&, const interval&) { return log10p1(x); } },
      { "sin(x)", 1, [](const interval& x, const interval&, const interval&) { return sin(x); } },
      { "cos(x)", 1, [](const interval& x, const interval&, const interval&) { return cos(x); } },
      { "tan(x)", 1, [](const interval& x, const interval&, const interval&) { return tan(x); } },
      { "sinpi(x)", 1, [](const interval& x, const interval&, const interval&) { return sinpi(x); } },
      { "cospi(x)", 1, [](const interval& x, const interval&, const interval&) { return cospi(x); } },
      { "tanpi(x)", 1, [](const interval& x, const interval&, const interval&) { return tanpi(x); } },
      { "asin(x)", 1, [](const interval& x, const interval&, const interval&) { return asin(x); } },
      { "acos(x)", 1, [](const interval& x, const interval&, const interval&) { return acos(x); } },
      { "atan(x)", 1, [](const interval& x, const interval&, const interval&) { return atan(x); } },
      { "asinpi(x)", 1, [](const interval& x, const interval&, const interval&) { return asinpi(x); } },
      { "acospi(x)", 1, [](const interval& x, const interval&, const interval&) { return acospi(x); } },
      { "atanpi(x)", 1, [](const interval& x, const interval&, const interval&) { return atanpi(x); } },
      { "sinh(x)", 1, [](const interval& x, const interval&, const interval&) { return sinh(x); } },
      { "cosh(x)", 1, [](const interval& x, const interval&, const interval&) { return cosh(x); } },
      { "tanh(x)", 1, [](const interval& x, const interval&, const interval&) { return tanh(x); } },
      { "asinh(x)", 1, [](const interval& x, const interval&, const interval&) { return asinh(x); } },
      { "acosh(x)", 1, [](const interval& x, const interval&, const interval&) { return acosh(x); } },
      { "atanh(x)", 1, [](const interval& x, const interval&, const interval&) { return atanh(x); } },
      { "erf(x)", 1, [](const interval& x, const interval&, const interval&) { return erf(x); } },
      { "erfc(x)", 1, [](const interval& x, const interval&, const interval&) { return erfc(x); } },
      { "pow(x, 2)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, 2); } },
      { "pow(x, 3)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, 3); } },
      { "pow(x, -2)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, -2); } },
      { "pow(x, -3)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, -3); } },
      { "pow(x, 5u)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, 5u); } },
      { "nth_root(x, 2)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 2); } },
      { "nth_root(x, 3)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 3); } },
      { "nth_root(x, 4)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 4); } },
      { "nth_root(x, 5)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, 5); } },
      { "nth_root(x, -2)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, -2); } },
      { "nth_root(x, -3)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, -3); } },
      { "nth_root(x, -5)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, -5); } },
      { "nth_root(x, 2^33 + 1)", 1, [](const interval& x, const interval&, const interval&) { return nth_root(x, (1LL << 33) + 1); } },
      { "pow(x, [-3])", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, interval(-3.0)); } },
      { "pow(x, -3.0)", 1, [](const interval& x, const interval&, const interval&) { return gaol::pow(x, -3.0); } },
      { "gaol_ieee1788::pow(x, [-3])", 1, [](const interval& x, const interval&, const interval&) { return gaol_ieee1788::pow(x, interval(-3.0)); } },
      // The two integers modulo_k_pi() gives, as an interval
      { "modulo_k_pi(x)", 1, [](const interval& x, const interval&, const interval&) {
          double k_left = 0.0, k_right = 0.0;
          modulo_k_pi(x, k_left, k_right);
          return interval(k_left, k_right); } },
      { "x.mid()", 1, [](const interval& x, const interval&, const interval&) { return x.mid(); } },
      { "x.width()", 1, [](const interval& x, const interval&, const interval&) { return interval(x.width()); } },
      { "x.rad()", 1, [](const interval& x, const interval&, const interval&) { return interval(x.rad()); } },
      { "x + y", 2, [](const interval& x, const interval& y, const interval&) { return x + y; } },
      { "x - y", 2, [](const interval& x, const interval& y, const interval&) { return x - y; } },
      { "x * y", 2, [](const interval& x, const interval& y, const interval&) { return x * y; } },
      { "x / y", 2, [](const interval& x, const interval& y, const interval&) { return x / y; } },
      { "x % y", 2, [](const interval& x, const interval& y, const interval&) { return x % y; } },
      { "hypot(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return hypot(x, y); } },
      { "atan2(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atan2(x, y); } },
      { "atan2pi(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atan2pi(x, y); } },
      { "pow(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return gaol::pow(x, y); } },
      { "gaol_ieee1788::pow(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return gaol_ieee1788::pow(x, y); } },
      { "pow(x, double)", 2, [](const interval& x, const interval& y, const interval&) { return gaol::pow(x, y.left()); } },
      { "sqrt_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return sqrt_rel(x, y); } },
      { "nth_root_rel(x, 3, y)", 2, [](const interval& x, const interval& y, const interval&) { return nth_root_rel(x, 3, y); } },
      { "nth_root_rel(x, 4, y)", 2, [](const interval& x, const interval& y, const interval&) { return nth_root_rel(x, 4, y); } },
      { "acos_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return acos_rel(x, y); } },
      { "asin_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return asin_rel(x, y); } },
      { "atan_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atan_rel(x, y); } },
      { "acosh_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return acosh_rel(x, y); } },
      { "asinh_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return asinh_rel(x, y); } },
      { "atanh_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return atanh_rel(x, y); } },
      { "sign_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return sign_rel(x, y); } },
      { "floor_rel(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return floor_rel(x, y); } },
      { "cancel_minus(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return cancel_minus(x, y); } },
      { "cancel_plus(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return cancel_plus(x, y); } },
      { "hausdorff(x, y)", 2, [](const interval& x, const interval& y, const interval&) { return interval(hausdorff(x, y)); } },
      { "x * double", 2, [](const interval& x, const interval& y, const interval&) { return x * y.left(); } },
      { "double * x", 2, [](const interval& x, const interval& y, const interval&) { return y.left() * x; } },
      { "x / double", 2, [](const interval& x, const interval& y, const interval&) { return x / y.left(); } },
      { "double / x", 2, [](const interval& x, const interval& y, const interval&) { return y.left() / x; } },
      { "x % double", 2, [](const interval& x, const interval& y, const interval&) { return x % y.left(); } },
      { "x + double", 2, [](const interval& x, const interval& y, const interval&) { return x + y.left(); } },
      { "double - x", 2, [](const interval& x, const interval& y, const interval&) { return y.left() - x; } },
      { "fma(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return fma(x, y, z); } },
      { "div_rel(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return div_rel(x, y, z); } },
      { "pow_rel(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return pow_rel(x, y, z); } },
      { "pow_exponent_rel(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return pow_exponent_rel(x, y, z); } },
      { "max_rel(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return max_rel(x, y, z); } },
      { "min_rel(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return min_rel(x, y, z); } },
      { "atan2_rel_y(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return atan2_rel_y(x, y, z); } },
      { "atan2_rel_x(x, y, z)", 3, [](const interval& x, const interval& y, const interval& z) { return atan2_rel_x(x, y, z); } },
    };
    // Subnormal, mixed and normal bounds; the first ones for two and three operands
    const interval flush_operands[] = {
      interval(subnormal(100)), interval(-subnormal(100)), interval(subnormal(3), subnormal(100)),
      interval(-subnormal(100), subnormal(1000)), interval(0.0, subnormal(100)), interval(-1e-310, 4.0),
      interval(1e-300), interval(1.0, 2.0), interval(-2.0, 3.0), interval(-1.0, subnormal(5)), interval(3.0),
      interval(subnormal(100), subnormal(1000)), interval(-subnormal(100), -subnormal(3)), interval(0.5),
      interval(-subnormal(100), 0.0), interval(1e-310, 1e-309), interval(-1e-309, -1e-310), interval(1e-310, 2.0),
      interval(1e-160), interval(3e-308), interval(1e-308, 3e-308), interval(-740.0), interval(-1070.0),
      interval(-320.0), interval(1e-300, 1e-200), interval(1e10), interval(0x1p-1022), interval(100.0, 1000.0),
    };
    const std::size_t all_operands = sizeof(flush_operands)/sizeof(flush_operands[0]);
    const std::size_t operands_of[] = { 0, all_operands, 14, 6 }; // by arity
    const auto same = [](const interval& a, const interval& b) {
      if (a.is_empty() || b.is_empty()) {
        return a.is_empty() && b.is_empty();
      }
      const double ab[2] = { a.left(), a.right() }, bb[2] = { b.left(), b.right() };
      return std::memcmp(ab, bb, sizeof ab) == 0;
    };
    for (const FlushOperation& op : flush_operations) {
      const std::string name = std::string(op.name) + " with a flush-to-zero mode set before it, as with the modes cleared";
#if GAOL_PRESERVE_ROUNDING
      const std::string modes_name = std::string(op.name) + ": flush-to-zero modes restored after it";
#else
      const std::string modes_name = std::string(op.name) + ": flush-to-zero modes cleared after it";
#endif
      const std::size_t n = operands_of[op.arity];
      const std::size_t ny = (op.arity >= 2) ? n : 1, nz = (op.arity == 3) ? n : 1;
      for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < ny; ++j) {
          for (std::size_t k = 0; k < nz; ++k) {
            const interval& x = flush_operands[i];
            const interval& y = flush_operands[j];
            const interval& z = flush_operands[k];
            set(directions[0]);
            set_flush_bits(0u);
            const interval cleared = op.run(x, y, z);
            for (const FlushMode& m : flush_modes_honoured) {
              for (const Direction& d : directions) {
                set(d);
                set_flush_bits(m.bits);
                const interval r = op.run(x, y, z);
                const unsigned int left = flush_bits();
                set_flush_bits(0u);
                const auto args = [&] {
                  std::string a = hex(x);
                  if (op.arity >= 2) {
                    a += ", " + hex(y);
                  }
                  if (op.arity == 3) {
                    a += ", " + hex(z);
                  }
                  return m.name + " set, rounding direction " + d.name + ", (" + a + ")";
                };
                check(name, same(r, cleared),
                      [&] { return args() + ": " + hex(r) + " rather than " + hex(cleared); });
                const auto modes_left = [&] { return args() + ": modes " + std::to_string(left) + " after it"; };
#if GAOL_PRESERVE_ROUNDING
                check(modes_name, left == m.bits, modes_left);
#else
                check(modes_name, left == 0u, modes_left);
#endif
              }
            }
          }
        }
      }
    }
    set(directions[0]);

    /*
      The functions that make no check of the rounding direction, which only
      compare or copy bounds (the constructor from two bounds, the relations,
      &, |, max(), min(), abs(), sign(), floor(), ceil()...), give with a mode
      set before them what they give with the modes cleared, and leave the
      modes as they found them: they compare the bounds as bounds
      (gaol_detail::bound_less()..., gaol_port.h), which no mode changes, and
      write nothing to the control register (GAOL v5, point Q of TODO.md,
      issue #68). Under denormals-are-zero, interval(2^-1073, 2^-1074) kept
      its bounds in the wrong order, max([3, 100]*2^-1074, [200*2^-1074]) was
      the first one, abs([-1e-309, -1e-310]) stayed negative, sign([-2^-1074])
      was [0], floor([-1e-310]) [-0], [3*2^-1074] < [5*2^-1074] false and
      [2, 5]*2^-1074 contained 7*2^-1074. They do not depend on the rounding
      direction, and are run with it upward. midpoint(), split(), chi() and
      the writing of an interval compute: they check the direction, which
      clears the modes, and are run in each direction, the modes cleared after
      them, or restored with GAOL_PRESERVE_ROUNDING; under denormals-are-zero,
      the midpoint of [3, 100]*2^-1074 was 0, outside the interval, and
      [22*2^-1074] was written [1e-322] with 1 digit, below its bound. The
      results are kept as they are and compared by their bits, the empty set
      by its NaN bounds, so that bounds in the wrong order, which is_empty()
      takes for the empty set with the modes cleared, are told apart; their
      doubles are written once the modes are cleared (Kept). The check of
      <a, b> by the parser, which compares the two numbers as doubles, is left
      to point E.
    */
    {
      struct UncheckedFunction
      {
        const char *name;
        int arity; // 1: x, 2: x and y, a double being y.left()
        bool checks; // checks the rounding direction, which clears the modes
        Kept (*run)(const interval& x, const interval& y);
      };
      const UncheckedFunction unchecked_functions[] = {
        { "interval(x.left(), y.left())", 2, false, [](const interval& x, const interval& y) { return kept(interval(x.left(), y.left())); } },
        { "interval(x.right(), y.left())", 2, false, [](const interval& x, const interval& y) { return kept(interval(x.right(), y.left())); } },
        { "x & y", 2, false, [](const interval& x, const interval& y) { return kept(x & y); } },
        { "x | y", 2, false, [](const interval& x, const interval& y) { return kept(x | y); } },
        { "x &= double", 2, false, [](const interval& x, const interval& y) { interval z(x); z &= y.left(); return kept(z); } },
        { "x |= double", 2, false, [](const interval& x, const interval& y) { interval z(x); z |= y.left(); return kept(z); } },
        { "max(x, y)", 2, false, [](const interval& x, const interval& y) { return kept(max(x, y)); } },
        { "min(x, y)", 2, false, [](const interval& x, const interval& y) { return kept(min(x, y)); } },
        { "abs(x)", 1, false, [](const interval& x, const interval&) { return kept(abs(x)); } },
        { "invabs_rel(x, y)", 2, false, [](const interval& x, const interval& y) { return kept(invabs_rel(x, y)); } },
        { "sign(x)", 1, false, [](const interval& x, const interval&) { return kept(sign(x)); } },
        { "floor(x)", 1, false, [](const interval& x, const interval&) { return kept(floor(x)); } },
        { "ceil(x)", 1, false, [](const interval& x, const interval&) { return kept(ceil(x)); } },
        { "integer(x)", 1, false, [](const interval& x, const interval&) { return kept(integer(x)); } },
        { "trunc(x)", 1, false, [](const interval& x, const interval&) { return kept(trunc(x)); } },
        { "round_ties_to_even(x)", 1, false, [](const interval& x, const interval&) { return kept(round_ties_to_even(x)); } },
        { "round_ties_to_away(x)", 1, false, [](const interval& x, const interval&) { return kept(round_ties_to_away(x)); } },
        { "x.mig()", 1, false, [](const interval& x, const interval&) { return kept(x.mig()); } },
        { "x.smig()", 1, false, [](const interval& x, const interval&) { return kept(x.smig()); } },
        { "x.mag()", 1, false, [](const interval& x, const interval&) { return kept(x.mag()); } },
        { "gaol_ieee1788::inf(x)", 1, false, [](const interval& x, const interval&) { return kept(gaol_ieee1788::inf(x)); } },
        { "gaol_ieee1788::sup(x)", 1, false, [](const interval& x, const interval&) { return kept(gaol_ieee1788::sup(x)); } },
        { "nb_fp_numbers(x.left(), x.right())", 1, false, [](const interval& x, const interval&) {
            return x.is_common_interval() ? kept(nb_fp_numbers(x.left(), x.right())) : kept_text("not finite"); } },
        { "x.certainly_le(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.certainly_le(y)); } },
        { "x.certainly_leq(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.certainly_leq(y)); } },
        { "x.certainly_ge(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.certainly_ge(y)); } },
        { "x.certainly_geq(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.certainly_geq(y)); } },
        { "x.certainly_positive()", 1, false, [](const interval& x, const interval&) { return kept(x.certainly_positive()); } },
        { "x.certainly_negative()", 1, false, [](const interval& x, const interval&) { return kept(x.certainly_negative()); } },
        { "x.certainly_strictly_positive()", 1, false, [](const interval& x, const interval&) { return kept(x.certainly_strictly_positive()); } },
        { "x.certainly_strictly_negative()", 1, false, [](const interval& x, const interval&) { return kept(x.certainly_strictly_negative()); } },
        { "x.set_contains(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_contains(y)); } },
        { "x.set_contains(double)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_contains(y.left())); } },
        { "x.set_strictly_contains(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_strictly_contains(y)); } },
        { "x.set_strictly_contains(double)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_strictly_contains(y.left())); } },
        { "x.set_disjoint(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_disjoint(y)); } },
        { "x.set_eq(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_eq(y)); } },
        { "x.set_leq(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_leq(y)); } },
        { "x.set_le(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.set_le(y)); } },
        { "x.less(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.less(y)); } },
        { "x.strictly_less(y)", 2, false, [](const interval& x, const interval& y) { return kept(x.strictly_less(y)); } },
        { "lexicographic_less()(x, y)", 2, false, [](const interval& x, const interval& y) { return kept(lexicographic_less()(x, y)); } },
        { "x.is_zero()", 1, false, [](const interval& x, const interval&) { return kept(x.is_zero()); } },
        { "x.is_symmetric()", 1, false, [](const interval& x, const interval&) { return kept(x.is_symmetric()); } },
        { "x.straddles_zero()", 1, false, [](const interval& x, const interval&) { return kept(x.straddles_zero()); } },
        { "x.strictly_straddles_zero()", 1, false, [](const interval& x, const interval&) { return kept(x.strictly_straddles_zero()); } },
        { "x.is_a_double()", 1, false, [](const interval& x, const interval&) { return kept(x.is_a_double()); } },
        { "x.is_an_int()", 1, false, [](const interval& x, const interval&) { return kept(x.is_an_int()); } },
        { "x.is_canonical()", 1, false, [](const interval& x, const interval&) { return kept(x.is_canonical()); } },
        { "feven(x.left())", 1, false, [](const interval& x, const interval&) { return kept(feven(x.left())); } },
        { "x.midpoint()", 1, true, [](const interval& x, const interval&) { return kept(x.midpoint()); } },
        { "x.split()", 1, true, [](const interval& x, const interval&) { interval l, r; x.split(l, r); return kept(l, r); } },
        { "x.split_left()", 1, true, [](const interval& x, const interval&) { return kept(x.split_left()); } },
        { "x.split_right()", 1, true, [](const interval& x, const interval&) { return kept(x.split_right()); } },
        { "chi(x)", 1, true, [](const interval& x, const interval&) { return kept(chi(x)); } },
        { "operator<< (bounds)", 1, true, [](const interval& x, const interval&) { return kept_text(write(x, interval_format::bounds)); } },
        { "operator<< (bounds, 1 digit)", 1, true, [](const interval& x, const interval&) {
            const std::streamsize p = interval::precision(1);
            const std::string s = write(x, interval_format::bounds);
            interval::precision(p);
            return kept_text(s); } },
        { "operator<< (center)", 1, true, [](const interval& x, const interval&) { return kept_text(write(x, interval_format::center)); } },
        { "gaol_ieee1788::intervalToText(x)", 1, true, [](const interval& x, const interval&) { return kept_text(gaol_ieee1788::intervalToText(x)); } },
      };
      // A point interval of subnormals, whose bounds 1 digit writes outward
      const interval point_of_subnormals = interval(subnormal(22));
      for (const UncheckedFunction& f : unchecked_functions) {
        const std::string name = std::string(f.name) + " with a flush-to-zero mode set before it, as with the modes cleared";
        const std::string modes_name = std::string(f.name) + (f.checks ?
#if GAOL_PRESERVE_ROUNDING
          ": flush-to-zero modes restored after it" :
#else
          ": flush-to-zero modes cleared after it" :
#endif
          ": flush-to-zero modes left as they were after it");
        const std::size_t n = all_operands + 1;
        const std::size_t ny = (f.arity == 2) ? n : 1;
        const std::size_t nd = f.checks ? sizeof(directions)/sizeof(directions[0]) : 1;
        for (std::size_t i = 0; i < n; ++i) {
          for (std::size_t j = 0; j < ny; ++j) {
            const interval& x = (i < all_operands) ? flush_operands[i] : point_of_subnormals;
            const interval& y = (j < all_operands) ? flush_operands[j] : point_of_subnormals;
            set(directions[0]);
            set_flush_bits(0u);
            const Kept cleared = f.run(x, y);
            for (const FlushMode& m : flush_modes_honoured) {
              for (std::size_t k = 0; k < nd; ++k) {
                const Direction& d = directions[k];
                set(d);
                set_flush_bits(m.bits);
                const Kept r = f.run(x, y);
                const unsigned int left = flush_bits();
                set_flush_bits(0u);
                const auto args = [&] {
                  return m.name + " set, rounding direction " + d.name + ", (" + hex(x)
                         + ((f.arity == 2) ? ", " + hex(y) : std::string()) + ")";
                };
                check(name, same_kept(r, cleared), [&] { return args() + ": " + written(r) + "rather than " + written(cleared); });
                const unsigned int expected =
#if GAOL_PRESERVE_ROUNDING
                  m.bits;
#else
                  f.checks ? 0u : m.bits;
#endif
                check(modes_name, left == expected, [&] { return args() + ": modes " + std::to_string(left) + " after it"; });
              }
            }
          }
        }
      }
      set(directions[0]);
    }
  }
#else
  std::printf("No mode flushing the subnormals to zero that the test can set: the operations under one are not checked\n");
#endif // GAOL_TESTS_FLUSH

  /*
    The floating-point exceptions stay masked (GAOL v5). CORE-MATH's cbrt, pow
    and atan2 keep the exception flags around their work with
    fegetexceptflag() and fesetexceptflag() outside x86-64, and the
    fesetexceptflag() of mingw-w64 for a 32-bit target cleared the mask bits of
    MXCSR along with the flags: an invalid operation then trapped rather than
    raised a flag, and the first comparison of the NaN bounds of an empty
    interval killed the program. Where the SSE control register is readable,
    its mask bits have to be the same after each of these functions, in each
    rounding direction; everywhere, an empty interval has to be told empty
    after them, which is where the program died.
  */
  for (const Direction& d : directions) {
    set(d);
#if GAOL_TESTS_SSE
    const unsigned masks_before = _mm_getcsr() & _MM_MASK_MASK;
#endif
    const interval roots = nth_root(interval(1.0, 8.0), 3);     // cbrt
    const interval powers = pow(interval(1.5, 2.5), interval(0.1, 0.3));
    const interval angles = atan2(interval(1.0, 2.0), interval(3.0, 4.0));
#if GAOL_TESTS_SSE
    const unsigned masks_after = _mm_getcsr() & _MM_MASK_MASK;
    check("the SSE exception masks unchanged by cbrt, pow and atan2", masks_after == masks_before,
          [&] {
            return std::string("rounding direction ") + d.name + ": masks " + std::to_string(masks_after)
                 + " after them, " + std::to_string(masks_before) + " before";
          });
#endif
    const interval empty = interval::emptyset();
    check("an empty interval still told empty after cbrt, pow and atan2", empty.is_empty(),
          [&] { return std::string("rounding direction ") + d.name; });
    check("cbrt, pow and atan2 not empty on their domain",
          !roots.is_empty() && !powers.is_empty() && !angles.is_empty(),
          [&] { return std::string("rounding direction ") + d.name; });
  }

  /*
    pow with a subnormal result gives the tightest enclosure of the exact
    value, whatever the rounding direction of the calling code (GAOL v5). The
    check of the operations above compares it with GAOL's own result when the
    direction is upward; here the bounds are those computed apart, with mpmath
    (see subnormal_powers). With the x87 unit to nearest and the SSE
    instructions upward, the state the predicates of Shewchuk and of Triangle
    leave, CORE-MATH rounded these results to nearest, the direction
    fegetround() gives on x86-64 with glibc, and the upper bound was below the
    exact value.
  */
  for (const Direction& d : directions) {
    for (const SubnormalPower& p : subnormal_powers) {
      set(d);
      const interval z = gaol_ieee1788::pow(interval(p.x), interval(p.y));
      const double lo = subnormal(p.lo), hi = subnormal(p.hi);
      check("gaol_ieee1788::pow with a subnormal result: the tightest enclosure",
            z.left() == lo && z.right() == hi,
            [&] {
              return std::string("rounding direction ") + d.name + ", x=" + hex(p.x) + " y=" + hex(p.y) + ": "
                     + hex(z) + " rather than [" + hex(lo) + ", " + hex(hi) + "]";
            });
    }
  }

  /*
    An empty interval is told empty without an exception (GAOL v5). GAOL holds
    the empty set as the two bounds NaN, and is_empty() compared them with <=,
    which signals the invalid-operation exception on a quiet NaN: a program
    that enabled that exception (feenableexcept() of glibc) died on SIGFPE at
    each emptiness test of an empty interval, and so did the operations, which
    start with it; with the exception masked, the test raised the flag
    FE_INVALID, which a program reading the flags to find its own NaNs took for
    one of its own. interval::emptyset() did as much in a build without
    optimization, where it compared the NaN it builds. Both are quiet now
    (std::islessequal, and no comparison): neither raises a flag, on any
    platform, and neither traps on glibc, where the checks enable the exception
    in a child process.
    The operations with an empty operand, which compared its NaN bounds as
    well, are checked the same way below. The exceptions that GAOL's
    operations raise legitimately (the inexact one for almost each, the
    divide-by-zero and the overflow ones for an infinite bound), and the
    invalid-operation one some raise on nonempty operands, are not checked
    here: the exceptions have to be masked while GAOL computes (doc/using.md).
  */
  for (const NamedInterval& e : empty_sets) {
    const interval empty = e.make();
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool told_empty = empty.is_empty();
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("is_empty() of an empty set raises no exception flag", raised == 0,
          [&] { return std::string(e.name) + ": flags " + std::to_string(raised) + " raised"; });
    check("is_empty() of an empty set is true", told_empty, [&] { return std::string(e.name); });
  }
  for (const NamedInterval& e : nonempty_sets) {
    const interval nonempty = e.make();
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool told_empty = nonempty.is_empty();
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("is_empty() of a nonempty interval raises no exception flag", raised == 0,
          [&] { return std::string(e.name) + ": flags " + std::to_string(raised) + " raised"; });
    check("is_empty() of a nonempty interval is false", !told_empty, [&] { return std::string(e.name); });
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const interval empty = interval::emptyset();
    static_cast<void>(empty);
    const int raised = std::fetestexcept(FE_ALL_EXCEPT);
    check("interval::emptyset() raises no exception flag", raised == 0,
          [&] { return "flags " + std::to_string(raised) + " raised"; });
  }

  // Every operation with an empty operand gives its result without the
  // invalid-operation exception (GAOL v5): the other exceptions (inexact, for
  // the rounding direction checked with an addition) are not checked
  for (const EmptyOperand& e : empty_operands) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("an operation with an empty operand raises no invalid-operation flag", raised == 0,
          [&] { return std::string(e.name); });
    check("an operation with an empty operand gives the result of the empty set", right,
          [&] { return std::string(e.name); });
  }

  // A NaN the program gives, the products with a zero and an infinite bound,
  // the powers with an exponent of extreme magnitude and the relations in
  // loops raise no invalid-operation flag, and the midpoints of intervals
  // with a huge bound no overflow flag (GAOL v5, point D.24), nor floor_rel()
  // of a Z ending at DBL_MAX (GAOL v5, point P.25)
  for (const EmptyOperand& e : nan_operands) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("an operation with a NaN double raises no invalid-operation flag", raised == 0, [&] { return std::string(e.name); });
    check("an operation with a NaN double gives the result of the empty set", right, [&] { return std::string(e.name); });
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = products_with_zero_and_infinite_bounds_are_right();
    const int raised = std::fetestexcept(FE_INVALID);
    check("the products with a zero and an infinite bound raise no invalid-operation flag", raised == 0,
          [] { return "raised by" + products_raising_invalid(); });
    check("the products with a zero and an infinite bound are right", right);
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = quotients_with_zero_and_infinite_bounds_are_right();
    const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
    check("div_rel with zero and infinite bounds raises no invalid-operation nor division-by-zero flag", raised == 0,
          [&] { return std::string((raised & FE_INVALID) ? "invalid-operation" : "division-by-zero") + " raised by" + quotients_raising(); });
    check("div_rel with zero and infinite bounds: the quotients", right, [] { return "wrong:" + quotients_wrong(); });
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = divisions_with_zero_and_infinite_bounds_are_right();
    const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
    check("the divisions and inverses with zero and infinite bounds raise no invalid-operation nor division-by-zero flag",
          raised == 0, [] { return "raised by" + divisions_raising(); });
    check("the divisions and inverses with zero and infinite bounds: the quotients", right);
  }
  for (const EmptyOperand& e : extreme_powers) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("pow with an exponent of extreme magnitude raises no invalid-operation flag", raised == 0,
          [&] { return std::string(e.name); });
    check("pow with an exponent of extreme magnitude: the tightest enclosure", right, [&] { return std::string(e.name); });
  }
  for (const EmptyOperand& e : powers_near_overflow) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("a power next to the largest double raises no invalid-operation flag", raised == 0,
          [&] { return std::string(e.name); });
    check("a power next to the largest double: the expected result", right, [&] { return std::string(e.name); });
  }
  for (const EmptyOperand& e : integer_powers_at_the_ends) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("an integer power at the ends of the doubles raises no invalid-operation flag", raised == 0,
          [&] { return std::string(e.name); });
    check("an integer power at the ends of the doubles: the tightest enclosure", right, [&] { return std::string(e.name); });
  }
  for (const EmptyOperand& e : cancels_next_to_the_largest) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = e.run();
    const int raised = std::fetestexcept(FE_INVALID);
    check("cancel_minus and cancel_plus next to the largest double raise no invalid-operation flag",
          raised == 0, [&] { return std::string(e.name); });
    check("cancel_minus and cancel_plus next to the largest double: the tightest enclosure", right,
          [&] { return std::string(e.name); });
  }
  for (const InfiniteOperation& e : arithmetic_with_infinite_bounds) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const std::string wrong = e.run();
    const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO);
    check("an operation with infinite bounds or an empty operand raises no invalid-operation nor division-by-zero flag",
          raised == 0, [&] { return std::string(e.name) + ((raised & FE_INVALID) ? ": invalid-operation" : ": division-by-zero"); });
    check("an operation with infinite bounds or an empty operand gives the right bounds", wrong.empty(),
          [&] { return std::string(e.name) + ": " + wrong; });
  }
  {
    /* The differences of the bounds of cancel_minus() are computed by TwoSum
       in the rounding to nearest, their terms kept before the direction
       changes back: GCC computed them after it, upward, where TwoSum is no
       longer exact, and the terms of the error of
       cancel_minus([DBL_MAX], [1]) read +oo and inf - inf, and raised the
       invalid-operation flag (GAOL v5, point 48; the bounds are checked in
       tests/arithmetic.cpp) */
    std::feclearexcept(FE_ALL_EXCEPT);
    const interval z = cancel_minus(interval((std::numeric_limits<double>::max)()), interval(1.0));
    const int raised = std::fetestexcept(FE_INVALID);
    check("cancel_minus with a huge bound raises no invalid-operation flag", raised == 0,
          [&] { return "flags " + std::to_string(raised) + " raised"; });
    check("cancel_minus([DBL_MAX], [1]) is the hull of DBL_MAX",
          z.left() == std::nextafter((std::numeric_limits<double>::max)(), 0.0)
              && z.right() == (std::numeric_limits<double>::max)(),
          [&] { return hex(z); });
  }
  /*
    width() of an interval built in the function, and the cut point of
    bisect(), are computed upward whatever the direction of the calling code
    (GAOL v5; see built_width()): the width of [1, 2^53 + 2] is 2^53 + 1,
    which the upward direction rounds to 2^53 + 2, where the other directions
    round it to 2^53; the cuts of [0.3, 7.7] at 0.3 and of [1, 1e16] at 0.7 are
    inexact too
  */
  for (const Direction& d : directions) {
    set(d);
    const double w = width_of_arguments(1.0, 9007199254740994.0);
    const std::pair<interval, interval> a = interval(0.3, 7.7).bisect(0.3), b = interval(1.0, 1e16).bisect(0.7);
    set(directions[0]);
    check("width() of an interval built from its arguments is rounded upward", w == 9007199254740994.0,
          [&] { return std::string("the direction of the program ") + d.name + ": " + hex(w); });
    check("the cut point of bisect(0.3) is computed upward", a.first.right() == upward_cut(0.3, 7.7, 0.3),
          [&] { return std::string("the direction of the program ") + d.name + ": " + hex(a.first.right()) + " rather than " + hex(upward_cut(0.3, 7.7, 0.3)); });
    check("the cut point of bisect(0.7) is computed upward", b.first.right() == upward_cut(1.0, 1e16, 0.7),
          [&] { return std::string("the direction of the program ") + d.name + ": " + hex(b.first.right()) + " rather than " + hex(upward_cut(1.0, 1e16, 0.7)); });
  }
  const std::vector<HugeInterval> huge = huge_intervals();
  check("the intervals with a huge bound: the subnormal bound is 2^-1074",
        huge.back().l == std::ldexp(gaol_core::rnd_keep(1.0), -1074) && huge.back().r == (std::numeric_limits<double>::max)(),
        [&] { return "[" + hex(huge.back().l) + ", " + hex(huge.back().r) + "]"; });
  std::vector<Midpoints> huge_midpoints;
  for (const HugeInterval& h : huge) {
    std::feclearexcept(FE_ALL_EXCEPT);
    huge_midpoints.push_back(midpoints_of(h));
    const int raised = std::fetestexcept(FE_OVERFLOW);
    const Midpoints& m = huge_midpoints.back();
    check("the midpoints of an interval with a huge bound raise no overflow flag", raised == 0, [&] { return std::string(h.name); });
    check("the midpoints of an interval with a huge bound are right", midpoints_are_right(h, m),
          [&] {
            return std::string(h.name) + ": midpoint() " + hex(m.midpoint) + ", mid() " + hex(m.mid) + ", rad() " + hex(m.rad)
                 + ", split() " + hex(m.split_l) + " " + hex(m.split_r);
          });
  }
  {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = floor_rel_at_dbl_max_is_right();
    const int raised = std::fetestexcept(FE_OVERFLOW);
    check("floor_rel() of a Z ending at DBL_MAX raises no overflow flag where its result has no infinite bound", raised == 0);
    check("floor_rel([DBL_MAX], [0, 1]) is empty and floor_rel([0, DBL_MAX], [0, 1]) is [0, 1]", right);
  }
  for (const RelationLoop& r : relation_loops) {
    std::feclearexcept(FE_ALL_EXCEPT);
    const volatile bool right = relation_loop_is_right(r);
    const int raised = std::fetestexcept(FE_INVALID);
    check("a loop of relations with empty operands raises no invalid-operation flag", raised == 0, [&] { return std::string(r.name); });
    check("a loop of relations with empty operands gives the relations", right, [&] { return std::string(r.name); });
  }

#if GAOL_TESTS_TRAPS
  // The empty sets, computed and told empty with the invalid-operation
  // exception enabled: what the processor delivers where it can, which the
  // control shows first (the FPU of some processors, and of the emulators of
  // others, has no exception to enable)
  if (run_with_exceptions_enabled(FE_INVALID, invalid_operation) != trapped) {
    std::printf("The processor does not trap an invalid operation here: the checks with the "
                "exception enabled are skipped\n");
  } else {
    for (const NamedInterval& e : empty_sets) {
      const Outcome o = run_with_exceptions_enabled(FE_INVALID, [&e] { return e.make().is_empty(); });
      check("is_empty() of an empty set, FE_INVALID enabled", o == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(o); });
    }
    for (const NamedInterval& e : nonempty_sets) {
      const Outcome o = run_with_exceptions_enabled(FE_INVALID, [&e] { return !e.make().is_empty(); });
      check("is_empty() of a nonempty interval, FE_INVALID enabled", o == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(o); });
    }
    const Outcome o = run_with_exceptions_enabled(FE_INVALID, [] { return interval::emptyset().is_empty(); });
    check("interval::emptyset().is_empty(), FE_INVALID enabled", o == returned_true,
          [&] { return outcome_text(o); });
    for (const EmptyOperand& e : empty_operands) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("an operation with an empty operand, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    for (const EmptyOperand& e : nan_operands) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("an operation with a NaN double, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    const Outcome op = run_with_exceptions_enabled(FE_INVALID, products_with_zero_and_infinite_bounds_are_right);
    check("the products with a zero and an infinite bound, FE_INVALID enabled", op == returned_true,
          [&] { return outcome_text(op); });
    const Outcome oq = run_with_exceptions_enabled(FE_INVALID | FE_DIVBYZERO, quotients_with_zero_and_infinite_bounds_are_right);
    check("div_rel with zero and infinite bounds, FE_INVALID and FE_DIVBYZERO enabled", oq == returned_true,
          [&] { return outcome_text(oq); });
    const Outcome od = run_with_exceptions_enabled(FE_INVALID | FE_DIVBYZERO, divisions_with_zero_and_infinite_bounds_are_right);
    check("the divisions and inverses with zero and infinite bounds, FE_INVALID and FE_DIVBYZERO enabled",
          od == returned_true, [&] { return outcome_text(od); });
    for (const EmptyOperand& e : extreme_powers) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("pow with an exponent of extreme magnitude, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    for (const EmptyOperand& e : powers_near_overflow) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("a power next to the largest double, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    for (const EmptyOperand& e : integer_powers_at_the_ends) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("an integer power at the ends of the doubles, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    for (const EmptyOperand& e : cancels_next_to_the_largest) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID, e.run);
      check("cancel_minus and cancel_plus next to the largest double, FE_INVALID enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    for (const InfiniteOperation& e : arithmetic_with_infinite_bounds) {
      const Outcome oe = run_with_exceptions_enabled(FE_INVALID | FE_DIVBYZERO, [&e] { return e.run().empty(); });
      check("an operation with infinite bounds or an empty operand, FE_INVALID and FE_DIVBYZERO enabled", oe == returned_true,
            [&] { return std::string(e.name) + ": " + outcome_text(oe); });
    }
    const Outcome oc = run_with_exceptions_enabled(FE_INVALID, [] {
      const interval z = cancel_minus(interval((std::numeric_limits<double>::max)()), interval(1.0));
      return z.left() == std::nextafter((std::numeric_limits<double>::max)(), 0.0)
          && z.right() == (std::numeric_limits<double>::max)();
    });
    check("cancel_minus([DBL_MAX], [1]), FE_INVALID enabled", oc == returned_true,
          [&] { return outcome_text(oc); });
    for (const RelationLoop& r : relation_loops) {
      const Outcome ol = run_with_exceptions_enabled(FE_INVALID, [&r] { return relation_loop_is_right(r); });
      check("a loop of relations with empty operands, FE_INVALID enabled", ol == returned_true,
            [&] { return std::string(r.name) + ": " + outcome_text(ol); });
    }
    // The exception the program enabled stays enabled after each operation
    // (GAOL v5, point D.24): with GAOL_PRESERVE_ROUNDING, the SSE2 operations
    // wrote the SSE control register with every exception masked
    const Outcome om = run_with_exceptions_enabled(FE_INVALID, [&] {
      bool enabled = true;
      for (const Operation& o : operations) {
        static_cast<void>(run(o, operands[0].first, operands[0].second));
#if GAOL_TESTS_SSE
        enabled = enabled && (_mm_getcsr() & _MM_MASK_INVALID) == 0u;
#else
        enabled = enabled && (fegetexcept() & FE_INVALID) != 0;
#endif
      }
      return enabled;
    });
    check("the invalid-operation exception the program enabled stays enabled after every operation", om == returned_true,
          [&] { return outcome_text(om); });
  }
  if (run_with_exceptions_enabled(FE_OVERFLOW, overflow_operation) != trapped) {
    std::printf("The processor does not trap an overflow here: the midpoints with the overflow exception "
                "enabled are not checked\n");
  } else {
    for (std::size_t i = 0; i < huge_midpoints.size(); ++i) {
      // The midpoints computed above, apart from the exception enabled
      const Outcome oe = run_with_exceptions_enabled(FE_OVERFLOW, [&] { return midpoints_of(huge[i]) == huge_midpoints[i]; });
      check("the midpoints of an interval with a huge bound, FE_OVERFLOW enabled", oe == returned_true,
            [&] { return std::string(huge[i].name) + ": " + outcome_text(oe); });
    }
    const Outcome of = run_with_exceptions_enabled(FE_OVERFLOW, floor_rel_at_dbl_max_is_right);
    check("floor_rel() of a Z ending at DBL_MAX, without an infinite bound, FE_OVERFLOW enabled", of == returned_true,
          [&] { return std::string(outcome_text(of)); });
  }
#else
  std::printf("feenableexcept() and fork() are those of glibc: the checks with the exceptions enabled are skipped\n");
#endif

  /*
    The sign of the zero bounds of x * d and x *= d of the interval
    [+0, -0] (left() +0, right() -0), the one case where the AVX-512 path of
    GAOL_PREFER_AVX512 and the SSE2 intervals give another bit pattern of the
    same bounds (+0 and -0 are one number): [-0, +0] with the first, [+0, -0]
    with the second, for a nonzero d (doc/using.md, The AVX-512 path; decided
    on 2026-10-18 as an acceptable exception to "bit for bit", point AF, and
    its bits checked here from 2026-10-23, the checks of the interval with == not
    seeing the sign of a zero). Whatever the direction of the program. d * x
    has bits of its own (SSE2: [-0, +0] for a positive d, [+0, -0] for a
    negative one), which nothing documents and the test leaves alone. Only the
    SSE2 intervals document the sign; the others (the FPU and NEON intervals)
    are not checked, and the test says so.
  */
#if defined(GAOL_USING_SSE2_INSTRUCTIONS) && GAOL_USING_SSE2_INSTRUCTIONS
  {
#  if defined(GAOL_AVX512_ARITHMETIC) && GAOL_AVX512_ARITHMETIC
    const bool avx512_path = true;
#  else
    const bool avx512_path = false;
#  endif
    const double doubles[] = { 2.5, -2.5, 1.0, -1.0, 1e300, -1e300, 1e-310, -1e-310 };
    for (const Direction& dir : directions) {
      for (const double d : doubles) {
        set(dir);
        const interval x = opaque(interval(gaol_core::rnd_keep(0.0), gaol_core::rnd_keep(-0.0)));
        const interval p = x * gaol_core::rnd_keep(d);
        interval r = x;
        r *= gaol_core::rnd_keep(d);
        const bool left_negative = avx512_path, right_negative = !avx512_path;
        const auto has_the_signs = [&](const interval& z) {
          return z.left() == 0.0 && z.right() == 0.0
            && std::signbit(z.left()) == left_negative && std::signbit(z.right()) == right_negative;
        };
        check("x * d and x *= d of [+0, -0] have the zero bounds of their path (AVX-512: [-0, +0], SSE2: [+0, -0])",
              has_the_signs(p) && has_the_signs(r),
              [&] {
                return std::string("d = ") + std::to_string(d) + ", " + (avx512_path ? "AVX-512 path" : "SSE2 path") + ", "
                  + dir.name + ": x * d = [" + (std::signbit(p.left()) ? "-" : "+") + "0, "
                  + (std::signbit(p.right()) ? "-" : "+") + "0]";
              });
      }
    }
    set(directions[0]);
  }
#else
  std::printf("The sign of the zero bounds of x * d is documented for the SSE2 intervals only: not checked here\n");
#endif

  /*
    gaol::cleanup() sets back the rounding direction the first gaol::init()
    found, the x87 unit and the SSE instructions each theirs (GAOL v5): to
    nearest, as a program starts (C11, F.8.3), GAOL initializing itself before
    the static objects of the program. With GAOL_PRESERVE_ROUNDING, it leaves
    the direction as it is.
  */
  set(directions[0]);
  const State before_cleanup = state();
#if GAOL_TESTS_X87_STATE
  // The exception flags raised since gaol::init() are the program's: neither
  // gaol::cleanup() nor gaol::restore_rounding() clears the six of MXCSR, as
  // fesetround() of mingw-w64 did when it changed the direction (GAOL v5,
  // point 110 of TODO.md), with GAOL_PRESERVE_ROUNDING or without
  raise_all_flags();
#endif
  gaol::cleanup();
#if GAOL_TESTS_X87_STATE
  const unsigned csr_after_cleanup = static_cast<unsigned>(_mm_getcsr());
#endif
  const State after_cleanup = state();
#if GAOL_TESTS_X87_STATE
  check("the SSE exception flags kept by gaol::cleanup()", (csr_after_cleanup & 0x3fu) == 0x3fu,
        [&] { return "MXCSR " + std::to_string(csr_after_cleanup) + " after it"; });
  std::feclearexcept(FE_ALL_EXCEPT);
#endif
#if GAOL_PRESERVE_ROUNDING
  check("rounding direction unchanged by gaol::cleanup()", after_cleanup == before_cleanup,
        [&] { return text(after_cleanup) + " after it, " + text(before_cleanup) + " before"; });
#else
  const State to_nearest = { FE_TONEAREST, SSE_DIRECTION(_MM_ROUND_NEAREST) };
  check("rounding direction set back to nearest by gaol::cleanup()", after_cleanup == to_nearest,
        [&] { return text(after_cleanup) + " after it, " + text(before_cleanup) + " before"; });
#endif

  /*
    gaol::restore_rounding() sets back the direction that the first
    gaol::init() found, as many times as the program needs, where gaol::cleanup()
    does so at its first call only (GAOL v5): a program that goes back to GAOL
    after cleanup() calls it after each use, the operations of GAOL setting the
    direction upward again. With GAOL_PRESERVE_ROUNDING, the operations restore
    the direction themselves and restore_rounding() does nothing.
  */
#if !GAOL_PRESERVE_ROUNDING
  gaol::restore_rounding();
  const State after_restore = state();
  check("rounding direction set back to nearest by gaol::restore_rounding()", after_restore == to_nearest,
        [&] { return text(after_restore); });
  const interval back = interval(1.0) + interval(1.0);   // sets the direction upward again
  static_cast<void>(back);
  const State after_operation = state();
#if GAOL_TESTS_X87_STATE
  // The first call changes MXCSR, upward after the addition, back to nearest
  raise_all_flags();
#endif
  gaol::restore_rounding();
#if GAOL_TESTS_X87_STATE
  const unsigned csr_after_restore = static_cast<unsigned>(_mm_getcsr());
#endif
  gaol::restore_rounding();
  const State after_twice = state();
#if GAOL_TESTS_X87_STATE
  check("the SSE exception flags kept by gaol::restore_rounding()", (csr_after_restore & 0x3fu) == 0x3fu,
        [&] { return "MXCSR " + std::to_string(csr_after_restore) + " after it"; });
  std::feclearexcept(FE_ALL_EXCEPT);
#endif
  /* Where GAOL_AVX512_ARITHMETIC is (GAOL_PREFER_AVX512, and a machine
     building that runs the AVX-512 instructions), the addition takes the
     embedded rounding: it sets no direction, and the one the program left
     stays (to nearest here, as restore_rounding() set it), as it stays with
     GAOL_PRESERVE_ROUNDING (GAOL v5, gaol/gaol_interval_avx512.cpp) */
#if defined(GAOL_USING_SSE2_INSTRUCTIONS) && GAOL_USING_SSE2_INSTRUCTIONS \
    && defined(GAOL_AVX512_ARITHMETIC) && GAOL_AVX512_ARITHMETIC
  check("rounding direction set back by gaol::restore_rounding() as many times as needed",
        after_operation == to_nearest && after_twice == to_nearest,
        [&] {
          return text(after_twice) + " after it, " + text(after_operation) + " after the operation";
        });
#else
  check("rounding direction set back by gaol::restore_rounding() as many times as needed",
        !(after_operation == to_nearest) && after_twice == to_nearest,
        [&] {
          return text(after_twice) + " after it, " + text(after_operation) + " after the operation";
        });
#endif
#else
  gaol::restore_rounding();
  const State after_restore = state();
  check("gaol::restore_rounding() leaves the direction of the program with GAOL_PRESERVE_ROUNDING",
        after_restore == after_cleanup, [&] { return text(after_restore); });
#endif
  return summary();
}
