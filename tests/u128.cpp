/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the 128-bit unsigned integer of gaol/gaol_u128.h.
 *
 * The accurate phases of CORE-MATH's log, sin, cos, tan, atan2 and pow, and of
 * log2p1, log10p1, atan2pi, hypot, rsqrt and asinpi, compute with a 128-bit
 * integer (unsigned, and signed in asinpi), which Visual C++ has on no architecture and
 * GCC has on no 32-bit target: there GAOL computes with the two 64-bit halves
 * of gaol/gaol_u128.h. This test compiles those halves (GAOL_U128_FORCE_EMULATION)
 * and compares every operation with the same operation on the native type of
 * the compiler, over the values at the ends of the ranges, over the powers of
 * two and their neighbours, over random values, and over every shift count
 * from 0 to 127. The two have to give the same 128 bits, so that the bounds
 * GAOL computes with Visual C++ and on a 32-bit target are those it computes
 * elsewhere.
 *
 * Where the compiler has no 128-bit type of its own, which is where this code
 * runs in earnest, there is nothing to compare with: the test then checks the
 * identities the operations satisfy (a + b - b = a, shifting left then right,
 * the product of the halves against the schoolbook product, and the order).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-20 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#define GAOL_U128_FORCE_EMULATION 1
#include "gaol/gaol_u128.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

namespace
{
  long checks = 0, failures = 0;
  const long failures_shown = 5;

  void check(const char* name, bool ok, const std::string& what)
  {
    ++checks;
    if (!ok) {
      if (failures < failures_shown) {
        std::printf("FAILED %s: %s\n", name, what.c_str());
      }
      ++failures;
    }
  }

  std::string hex(uint64_t h, uint64_t l)
  {
    char b[64];
    std::snprintf(b, sizeof b, "0x%016llx%016llx",
                  (unsigned long long)h, (unsigned long long)l);
    return b;
  }

  // The values every operation is tried on: the ends of the ranges, the powers
  // of two and their neighbours, and values with a single half set
  std::vector<uint64_t> interesting_words()
  {
    std::vector<uint64_t> v;
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(UINT64_MAX);
    v.push_back(UINT64_MAX - 1);
    v.push_back(UINT32_MAX);
    v.push_back((uint64_t)UINT32_MAX + 1);
    for (int k = 1; k < 64; ++k) {
      const uint64_t p = (uint64_t)1 << k;
      v.push_back(p);
      v.push_back(p - 1);
      v.push_back(p + 1);
    }
    std::mt19937_64 gen(20260920u);
    for (int i = 0; i < 200; ++i) {
      v.push_back(gen());
      // values with few bits set, which the carries are sensitive to
      v.push_back(gen() & gen() & gen());
      v.push_back(gen() | gen() | gen());
    }
    return v;
  }
}

#if defined(__SIZEOF_INT128__)

typedef unsigned __int128 native;
typedef __int128 signed_native;

namespace
{
  native to_native(gaol_u128 a) { return ((native)a.h << 64) | a.l; }

  void same(const char* name, gaol_u128 got, native expected, const std::string& what)
  {
    const uint64_t eh = (uint64_t)(expected >> 64), el = (uint64_t)expected;
    check(name, got.h == eh && got.l == el,
          what + ": " + hex(got.h, got.l) + " rather than " + hex(eh, el));
  }

  void operations(uint64_t ah, uint64_t al, uint64_t bh, uint64_t bl)
  {
    const gaol_u128 a = gaol_u128_make(ah, al), b = gaol_u128_make(bh, bl);
    const native na = to_native(a), nb = to_native(b);
    const std::string what = hex(ah, al) + " and " + hex(bh, bl);

    same("add", gaol_u128_add(a, b), (native)(na + nb), what);
    same("add64", gaol_u128_add64(a, bl), (native)(na + bl), what);
    same("sub", gaol_u128_sub(a, b), (native)(na - nb), what);
    same("neg", gaol_u128_neg(a), (native)(-na), what);
    same("mul", gaol_u128_mul(a, b), (native)(na * nb), what);
    same("mul64", gaol_u128_mul64(al, bl),
         (native)((native)al * (native)bl), what);
    same("or", gaol_u128_or(a, b), (native)(na | nb), what);
    same("and64", gaol_u128_and64(a, bl), (native)(na & (native)bl), what);
    same("of", gaol_u128_of(al), (native)al, what);
    same("and", gaol_u128_and(a, b), (native)(na & nb), what);
    // the signed values, in two's complement
    same("of_i64", gaol_u128_of_i64((int64_t)al), (native)(signed_native)(int64_t)al, what);
    same("imul64", gaol_u128_imul64((int64_t)al, (int64_t)bl),
         (native)((signed_native)(int64_t)al * (int64_t)bl), what);

    check("lo", gaol_u128_lo(a) == (uint64_t)na, what);
    check("hi", gaol_u128_hi(a) == (uint64_t)(na >> 64), what);
    check("lt", gaol_u128_lt(a, b) == (na < nb), what);
    check("gt", gaol_u128_gt(a, b) == (na > nb), what);
    check("eq", gaol_u128_eq(a, b) == (na == nb), what);
  }

  void shifts(uint64_t ah, uint64_t al)
  {
    const gaol_u128 a = gaol_u128_make(ah, al);
    const native na = to_native(a);
    for (int n = 0; n < 128; ++n) {
      const std::string what = hex(ah, al) + " by " + std::to_string(n);
      same("shl", gaol_u128_shl(a, n), (native)(na << n), what);
      same("shr", gaol_u128_shr(a, n), (native)(na >> n), what);
      same("sar", gaol_u128_sar(a, n), (native)((signed_native)na >> n), what);
      same("bit", gaol_u128_bit(n), (native)((native)1 << n),
           std::to_string(n));
    }
  }
}

int main()
{
  const std::vector<uint64_t> words = interesting_words();
  // every pair of the first words, then random pairs of all of them
  const size_t dense = 40;
  for (size_t i = 0; i < dense && i < words.size(); ++i) {
    for (size_t j = 0; j < dense && j < words.size(); ++j) {
      operations(words[i], words[j], words[j], words[i]);
      operations(0, words[i], 0, words[j]);
      operations(words[i], 0, words[j], 0);
    }
  }
  std::mt19937_64 gen(20260921u);
  for (int i = 0; i < 200000; ++i) {
    const uint64_t ah = words[gen() % words.size()], al = words[gen() % words.size()];
    const uint64_t bh = words[gen() % words.size()], bl = words[gen() % words.size()];
    operations(ah, al, bh, bl);
    operations(gen(), gen(), gen(), gen());
  }
  for (size_t i = 0; i < words.size(); ++i) {
    shifts(words[i], words[(i + 1) % words.size()]);
    shifts(0, words[i]);
    shifts(words[i], 0);
  }
  std::printf("%ld checks, %ld failed\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

#else /* no native 128-bit type: the identities */

namespace
{
  void identities(uint64_t ah, uint64_t al, uint64_t bh, uint64_t bl)
  {
    const gaol_u128 a = gaol_u128_make(ah, al), b = gaol_u128_make(bh, bl);
    const std::string what = hex(ah, al) + " and " + hex(bh, bl);

    check("add then sub", gaol_u128_eq(gaol_u128_sub(gaol_u128_add(a, b), b), a), what);
    check("neg twice", gaol_u128_eq(gaol_u128_neg(gaol_u128_neg(a)), a), what);
    check("add is commutative",
          gaol_u128_eq(gaol_u128_add(a, b), gaol_u128_add(b, a)), what);
    check("mul is commutative",
          gaol_u128_eq(gaol_u128_mul(a, b), gaol_u128_mul(b, a)), what);
    check("add64 is add of the low half",
          gaol_u128_eq(gaol_u128_add64(a, bl), gaol_u128_add(a, gaol_u128_of(bl))), what);
    check("mul64 is mul of the low halves",
          gaol_u128_eq(gaol_u128_mul64(al, bl),
                       gaol_u128_mul(gaol_u128_of(al), gaol_u128_of(bl))), what);
    check("the order is total",
          gaol_u128_lt(a, b) + gaol_u128_gt(a, b) + gaol_u128_eq(a, b) == 1, what);
    const gaol_u128 ones = gaol_u128_make(UINT64_MAX, UINT64_MAX);
    check("and is commutative",
          gaol_u128_eq(gaol_u128_and(a, b), gaol_u128_and(b, a)), what);
    check("and with all ones", gaol_u128_eq(gaol_u128_and(a, ones), a), what);
    // the signed product is the product of the values extended with their sign
    check("imul64 is mul of the values extended",
          gaol_u128_eq(gaol_u128_imul64((int64_t)al, (int64_t)bl),
                       gaol_u128_mul(gaol_u128_of_i64((int64_t)al),
                                     gaol_u128_of_i64((int64_t)bl))), what);
    check("of_i64 is the high half shifted with its sign",
          gaol_u128_eq(gaol_u128_of_i64((int64_t)al),
                       gaol_u128_sar(gaol_u128_make(al, 0), 64)), what);
    for (int n = 0; n < 64; ++n) {
      const gaol_u128 low = gaol_u128_of(al >> n << n);
      check("shl then shr",
            gaol_u128_eq(gaol_u128_shr(gaol_u128_shl(low, n), n), low),
            what + " by " + std::to_string(n));
    }
    // ~a is ones - a: sar is shr for a >= 0 and ~shr(~a) for a < 0
    const gaol_u128 not_a = gaol_u128_sub(ones, a);
    for (int n = 0; n < 128; ++n) {
      const gaol_u128 expected = (ah >> 63) ? gaol_u128_sub(ones, gaol_u128_shr(not_a, n))
                                            : gaol_u128_shr(a, n);
      check("sar", gaol_u128_eq(gaol_u128_sar(a, n), expected),
            what + " by " + std::to_string(n));
    }
  }
}

int main()
{
  const std::vector<uint64_t> words = interesting_words();
  std::mt19937_64 gen(20260921u);
  for (int i = 0; i < 100000; ++i) {
    identities(words[gen() % words.size()], words[gen() % words.size()],
               words[gen() % words.size()], words[gen() % words.size()]);
  }
  std::printf("%ld checks, %ld failed\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

#endif
