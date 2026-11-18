/*-*-C-*--------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the choice between two 128-bit integers, compiled as C.
 *
 * The sources of CORE-MATH are C, and take the 128-bit integer of
 * gaol/math/gaol_u128.h that the C compiler gives: `unsigned __int128` on
 * 64-bit targets, `_BitInt(128)` of GCC 14 and Clang on 32-bit ones, and two
 * halves elsewhere. The GCC 14.2, 15.3 and 16.2 for 32-bit x86 (Debian 13 and
 * 14 i386) miscompile from -O1, with -ftree-coalesce-vars, the choice between
 * two `_BitInt(128)` values,
 *
 *     if (swap) { larger = y; smaller = x; } else { larger = x; smaller = y; }
 *
 * and gives `larger` and `smaller` that are not what the branch says: the
 * first version of gaol_fma() was wrong for every triple that took the first
 * branch of that line, and gaol_u128.h takes the two halves for 32-bit x86
 * with GCC since. tests/u128.cpp is C++, where GCC has no `_BitInt`, and cannot
 * see it: this program is the same choice as C, on the values of every kind, in
 * a function that is not inlined, and has to give what two 64-bit halves give.
 * It is a program for every compiler: the choice is right with all of them but
 * GCC for 32-bit x86 (Clang 21 for it is right).
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-18)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdio.h>

#include "gaol/math/gaol_u128.h"

#if defined(_MSC_VER) && !defined(__clang__)
#  define GAOL_TEST_NOINLINE __declspec(noinline)
#else
#  define GAOL_TEST_NOINLINE __attribute__((noinline))
#endif

/* The larger of two values by its place, and what gaol_fma() does with the two:
   2*larger - smaller, which uses both */
GAOL_TEST_NOINLINE static gaol_u128 pick(gaol_u128 x, gaol_u128 y, int swap)
{
  gaol_u128 larger, smaller;
  if (swap) {
    larger = y;
    smaller = x;
  } else {
    larger = x;
    smaller = y;
  }
  return gaol_u128_sub(gaol_u128_shl(larger, 1), smaller);
}

/* The same on two halves of 64 bits, with the operators of C on uint64_t */
static void expected(uint64_t xh, uint64_t xl, uint64_t yh, uint64_t yl, int swap,
                     uint64_t* rh, uint64_t* rl)
{
  const uint64_t lh = swap ? yh : xh, ll = swap ? yl : xl;
  const uint64_t sh = swap ? xh : yh, sl = swap ? xl : yl;
  const uint64_t dh = (lh << 1) | (ll >> 63), dl = ll << 1; /* 2 * larger */
  *rl = dl - sl;
  *rh = dh - sh - (dl < sl ? 1 : 0);
}

static uint64_t state = 88172645463325252ull;
static uint64_t next(void)
{
  state ^= state << 13;
  state ^= state >> 7;
  state ^= state << 17;
  return state;
}

int main(void)
{
  long checks = 0, failed = 0;
  long i;
  for (i = 0; i < 20000; ++i) {
    /* the first values are those of the smallest program that showed the
       defect, then random ones, with the high halves small or zero as often as
       not, which are the values of the products of gaol_fma() */
    volatile uint64_t xh = i == 0 ? 5 : (next() >> (next() % 64));
    volatile uint64_t xl = i == 0 ? 7 : next();
    volatile uint64_t yh = i == 0 ? 11 : (next() >> (next() % 64));
    volatile uint64_t yl = i == 0 ? 13 : next();
    int swap;
    for (swap = 0; swap < 2; ++swap) {
      uint64_t rh, rl;
      const gaol_u128 r = pick(gaol_u128_make(xh, xl), gaol_u128_make(yh, yl), swap);
      expected(xh, xl, yh, yl, swap, &rh, &rl);
      ++checks;
      if (gaol_u128_hi(r) != rh || gaol_u128_lo(r) != rl) {
        if (failed++ < 5) {
          printf("pick(%016llx%016llx, %016llx%016llx, %d) = %016llx%016llx rather than %016llx%016llx\n",
                 (unsigned long long)xh, (unsigned long long)xl, (unsigned long long)yh,
                 (unsigned long long)yl, swap, (unsigned long long)gaol_u128_hi(r),
                 (unsigned long long)gaol_u128_lo(r), (unsigned long long)rh, (unsigned long long)rl);
        }
      }
    }
  }
  printf("u128_select: %ld checks, %ld failed (GAOL_U128_NATIVE %d)\n", checks, failed, GAOL_U128_NATIVE);
  return failed != 0;
}
