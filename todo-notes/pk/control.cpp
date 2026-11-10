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

// Positive control of task PK: a raw `<=` between doubles, with NaNs, signals
// (it must raise FE_INVALID), which shows that fetestexcept() sees the flag in the
// benches. Build it like them: it must print "raised".
#include <cfenv>
#include <cstdio>
#include <cmath>
#include <vector>
__attribute__((noinline)) void raw(const double *l, unsigned char *r, int n) { for (int i = 0; i < n; ++i) r[i] = l[i] <= 1.5; }
int main()
{
  std::vector<double> l(64, 0.0);
  for (size_t i = 0; i < l.size(); i += 3) l[i] = std::nan("");
  unsigned char r[64];
  std::feclearexcept(FE_ALL_EXCEPT);
  raw(l.data(), r, 64);
  std::printf("control raw <=: FE_INVALID %s\n", std::fetestexcept(FE_INVALID) ? "raised" : "not raised");
}
