// The benchmark with GAOL's intervals (see bench_ops.h)
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-09-20 by Jordan NININ
#include <gaol/gaol.h>
#include "bench_ops.h"

/* The same source builds the benchmark of two versions of GAOL: GAOL v5
   (GAOL_BENCH_NAME "gaol5") and the one before it, from the master branch
   ("gaol"), so that the tables hold both. GAOL v5 is measured through the
   names of IEEE 1788-2015, gaol_ieee1788, as the other libraries of the
   standard (GAOL v5, decided on 13 October 2026, TODO.md B): pown is the
   integer power and pow the real power of the standard, the same functions
   as gaol::pow(x, n) and, on the positive bases of the benchmark, as
   gaol::pow(x, y). GAOL 4 has only its own names, gaol. */
#if GAOL_MAJOR_VERSION >= 5
namespace names = gaol_ieee1788;
#else
namespace names = gaol;
#endif

struct Gaol
{
  typedef names::interval I;
#ifndef GAOL_BENCH_NAME
#define GAOL_BENCH_NAME "gaol"
#endif
  static constexpr const char *name = GAOL_BENCH_NAME;
  static I make(double lo, double hi) { return I(lo, hi); }
  // GAOL v5 reads a string with textToInterval(), GAOL 4 with a constructor
#if GAOL_MAJOR_VERSION >= 5
  static I decimal(const char *s) { return names::textToInterval(s); }
#else
  static I decimal(const char *s) { return I(s); }
#endif
  static I sqr(const I& x) { return names::sqr(x); }
  static I sqrt(const I& x) { return names::sqrt(x); }
  static I exp(const I& x) { return names::exp(x); }
  static I log(const I& x) { return names::log(x); }
  static I sin(const I& x) { return names::sin(x); }
  static I cos(const I& x) { return names::cos(x); }
#if GAOL_MAJOR_VERSION >= 5
  static I pown(const I& x, int n) { return names::pown(x, n); }
#else
  static I pown(const I& x, int n) { return names::pow(x, n); }
#endif
  static I pow(const I& x, const I& y) { return names::pow(x, y); }
  static double mid(const I& x) { return x.midpoint(); }
  static double wid(const I& x) { return x.width(); }
};

int main(int argc, char *argv[])
{
  gaol::init();
  const int status = bench_main<Gaol>(argc, argv);
  gaol::cleanup();
  return status;
}
