/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Jordan NININ (created 2026-11-05)
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
 *--------------------------------------------------------------------------*/

// Harness of task PK: loops over relations, with empty operands, that must not
// raise FE_INVALID (the flag is read after each loop), and whose results must
// be those of the relation computed on its own.
#include <gaol/gaol>
#include <cfenv>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using gaol::interval;

#if defined(__GNUC__) || defined(__clang__)
#  define NOINLINE __attribute__((noinline))
#else
#  define NOINLINE
#endif

static const int N = 64;

// operands the compiler knows nothing about
static interval opaque(interval x)
{
  asm volatile("" : "+m"(x));
  return x;
}

static double opaque_d(double d)
{
  asm volatile("" : "+r"(d));
  return d;
}

#define RELS(F) F(certainly_le) F(certainly_leq) F(certainly_ge) F(certainly_geq)

// shape A: x[i].R(y[i])
// shape B: x[i].R(interval(-7.5, c))
// shape C: interval(c, 7.5).R(x[i])
// shape D: x[i].R(K) with a constant interval
// shape E: x[i].R(y[i]) accumulated (a count)
// shape F: x[i].R(interval(2.5)) a point constant
// shape G: interval(-7.5, c).R(y[i])
// shape H: x[i].R(y[i]) with early exit
// shape I: operators < <= > >= between intervals
// shape J: x[i].R(y[i]) with the result stored as a bool
// shape K: x[i].R(interval::emptyset()) and emptyset().R(x[i])
// shape L: x[i].R(interval(c)) a point of a runtime value
// shape M: R with a double bound constant on either side, via interval(1.5, 1.5)

#define LOOP_A(R) NOINLINE void A_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)c; for (int i = 0; i < N; ++i) r[i] = x[i].R(y[i]); } \
  unsigned char A_##R##_at(const interval &x, const interval &y, double c) { (void)c; return x.R(y); }
#define LOOP_B(R) NOINLINE void B_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; for (int i = 0; i < N; ++i) r[i] = x[i].R(interval(-7.5, c)); } \
  unsigned char B_##R##_at(const interval &x, const interval &y, double c) { (void)y; return x.R(interval(-7.5, c)); }
#define LOOP_C(R) NOINLINE void C_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; for (int i = 0; i < N; ++i) r[i] = interval(c, 7.5).R(x[i]); } \
  unsigned char C_##R##_at(const interval &x, const interval &y, double c) { (void)y; return interval(c, 7.5).R(x); }
#define LOOP_D(R) NOINLINE void D_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; (void)c; static const interval K(-7.5, 3.5); for (int i = 0; i < N; ++i) r[i] = x[i].R(K); } \
  unsigned char D_##R##_at(const interval &x, const interval &y, double c) { (void)y; (void)c; static const interval K(-7.5, 3.5); return x.R(K); }
#define LOOP_E(R) NOINLINE void E_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)c; unsigned n = 0; for (int i = 0; i < N; ++i) n += x[i].R(y[i]); for (int i = 0; i < N; ++i) r[i] = 0; r[0] = static_cast<unsigned char>(n); } \
  unsigned char E_##R##_at(const interval &x, const interval &y, double c) { (void)x; (void)y; (void)c; return 0; }
#define LOOP_F(R) NOINLINE void F_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; (void)c; for (int i = 0; i < N; ++i) r[i] = x[i].R(interval(2.5)); } \
  unsigned char F_##R##_at(const interval &x, const interval &y, double c) { (void)y; (void)c; return x.R(interval(2.5)); }
#define LOOP_G(R) NOINLINE void G_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)x; for (int i = 0; i < N; ++i) r[i] = interval(-7.5, c).R(y[i]); } \
  unsigned char G_##R##_at(const interval &x, const interval &y, double c) { (void)x; return interval(-7.5, c).R(y); }
#define LOOP_H(R) NOINLINE void H_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)c; for (int i = 0; i < N; ++i) r[i] = 0; for (int i = 0; i < N; ++i) { if (x[i].R(y[i])) { r[0] = static_cast<unsigned char>(i); break; } } } \
  unsigned char H_##R##_at(const interval &x, const interval &y, double c) { (void)x; (void)y; (void)c; return 0; }
#define LOOP_K(R) NOINLINE void K_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; (void)c; for (int i = 0; i < N; ++i) r[i] = x[i].R(interval::emptyset()) + 2 * interval::emptyset().R(x[i]); } \
  unsigned char K_##R##_at(const interval &x, const interval &y, double c) { (void)y; (void)c; return x.R(interval::emptyset()) + 2 * interval::emptyset().R(x); }
#define LOOP_L(R) NOINLINE void L_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; for (int i = 0; i < N; ++i) r[i] = x[i].R(interval(c)); } \
  unsigned char L_##R##_at(const interval &x, const interval &y, double c) { (void)y; return x.R(interval(c)); }
#define LOOP_M(R) NOINLINE void M_##R(const interval *x, const interval *y, unsigned char *r, double c) { (void)y; (void)c; for (int i = 0; i < N; ++i) r[i] = x[i].R(interval(1.5, 1.5)) + 2 * interval(1.5, 1.5).R(x[i]); } \
  unsigned char M_##R##_at(const interval &x, const interval &y, double c) { (void)y; (void)c; return x.R(interval(1.5, 1.5)) + 2 * interval(1.5, 1.5).R(x); }

RELS(LOOP_A) RELS(LOOP_B) RELS(LOOP_C) RELS(LOOP_D) RELS(LOOP_E) RELS(LOOP_F) RELS(LOOP_G) RELS(LOOP_H) RELS(LOOP_K) RELS(LOOP_L) RELS(LOOP_M)

// shape I: the operators
NOINLINE void I_ops(const interval *x, const interval *y, unsigned char *r, double c)
{
  (void)c;
  for (int i = 0; i < N; ++i) r[i] = (x[i] <= y[i]) + 2 * (x[i] < y[i]) + 4 * (x[i] >= y[i]) + 8 * (x[i] > y[i]);
}
unsigned char I_ops_at(const interval &x, const interval &y, double c)
{
  (void)c;
  return (x <= y) + 2 * (x < y) + 4 * (x >= y) + 8 * (x > y);
}
NOINLINE void I_ops_const(const interval *x, const interval *y, unsigned char *r, double c)
{
  (void)y; (void)c;
  for (int i = 0; i < N; ++i) r[i] = (x[i] <= interval(-7.5, 3.5)) + 2 * (x[i] < interval(-7.5, 3.5)) + 4 * (x[i] >= interval(-7.5, 3.5)) + 8 * (x[i] > interval(-7.5, 3.5));
}
unsigned char I_ops_const_at(const interval &x, const interval &y, double c)
{
  (void)y; (void)c;
  return (x <= interval(-7.5, 3.5)) + 2 * (x < interval(-7.5, 3.5)) + 4 * (x >= interval(-7.5, 3.5)) + 8 * (x > interval(-7.5, 3.5));
}

struct Loop
{
  const char *name;
  void (*loop)(const interval *, const interval *, unsigned char *, double);
  unsigned char (*at)(const interval &, const interval &, double);
};

#define ENTRY(S, R) { #S "_" #R, S##_##R, S##_##R##_at },
#define ENTRIES(S) ENTRY(S, certainly_le) ENTRY(S, certainly_leq) ENTRY(S, certainly_ge) ENTRY(S, certainly_geq)

static const Loop loops[] = {
  ENTRIES(A) ENTRIES(B) ENTRIES(C) ENTRIES(D) ENTRIES(E) ENTRIES(F) ENTRIES(G) ENTRIES(H) ENTRIES(K) ENTRIES(L) ENTRIES(M)
  { "I_ops", I_ops, I_ops_at },
  { "I_ops_const", I_ops_const, I_ops_const_at },
};

int main()
{
  gaol::init();
  std::vector<interval> x(N), y(N);
  const double c = opaque_d(3.5);
  int raising = 0, wrong = 0, total = 0;
  // several families of operands: the empty ones every third / fifth, only x empty, only y empty, all empty
  for (int family = 0; family < 4; ++family) {
    for (int i = 0; i < N; ++i) {
      const double d = static_cast<double>(i);
      switch (family) {
        case 0: x[i] = (i % 3 == 0) ? opaque(interval::emptyset()) : opaque(interval(d - 10.0, d + 0.5));
                y[i] = (i % 5 == 0) ? opaque(interval::emptyset()) : opaque(interval(0.5 * d, d + 1.0)); break;
        case 1: x[i] = opaque(interval::emptyset()); y[i] = opaque(interval(0.5 * d - 20.0, d - 5.0)); break;
        case 2: x[i] = opaque(interval(d - 40.0, d - 30.0)); y[i] = opaque(interval::emptyset()); break;
        default: x[i] = opaque(interval::emptyset()); y[i] = opaque(interval::emptyset()); break;
      }
    }
    for (const Loop &l : loops) {
      unsigned char r[N];
      std::feclearexcept(FE_ALL_EXCEPT);
      l.loop(x.data(), y.data(), r, c);
      const bool invalid = std::fetestexcept(FE_INVALID) != 0;
      std::feclearexcept(FE_ALL_EXCEPT);
      bool right = true;
      if (l.name[0] != 'E' && l.name[0] != 'H') {
        for (int i = 0; i < N; ++i) right = right && r[i] == l.at(opaque(x[i]), opaque(y[i]), c);
      }
      ++total;
      if (invalid) { ++raising; std::printf("INVALID family %d: %s\n", family, l.name); }
      if (!right) { ++wrong; std::printf("WRONG   family %d: %s\n", family, l.name); }
    }
  }
  std::printf("%d loop runs, %d raising FE_INVALID, %d wrong\n", total, raising, wrong);
  gaol::cleanup();
  return (raising != 0 || wrong != 0) ? 1 : 0;
}
