// Copyright (c) 2026 ENSTA, France
//
// Created 2026-09-20 by Jordan NININ
//
// The benchmark of doc/compare/code: reading the intervals of bench.py, timing
// the operations of bench_ops.h, and writing the results as CSV lines:
//   library,operation,n,repeats,best time (s),time per operation (ns),
//   sum of the midpoints of the results,sum of their widths
// The sums tell whether the libraries computed the same things.
#ifndef BENCH_COMMON_H
#define BENCH_COMMON_H

#include <algorithm>
#include <cfenv>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct Data
{
  std::int64_t n;
  // Lower and upper bounds of the intervals a and b (in [-10, 10]), p (in
  // [0.5, 10.5]), e (exponents in [0.5, 2.5]) and s[0..3] (the arguments of
  // Shekel 5, in [0, 10])
  std::vector<double> lo[8], hi[8];
};

enum { DATA_A = 0, DATA_B, DATA_P, DATA_E, DATA_S1, DATA_S2, DATA_S3, DATA_S4 };

inline Data load(const char *path)
{
  Data d;
  std::FILE *f = std::fopen(path, "rb");
  if (!f || std::fread(&d.n, sizeof d.n, 1, f) != 1) {
    std::fprintf(stderr, "cannot read %s\n", path);
    std::exit(1);
  }
  for (int k = 0; k < 8; ++k) {
    d.lo[k].resize(d.n);
    d.hi[k].resize(d.n);
    if (std::fread(d.lo[k].data(), sizeof(double), d.n, f) != static_cast<std::size_t>(d.n)
        || std::fread(d.hi[k].data(), sizeof(double), d.n, f) != static_cast<std::size_t>(d.n)) {
      std::fprintf(stderr, "%s is too short\n", path);
      std::exit(1);
    }
  }
  std::fclose(f);
  return d;
}

// Whether the operation name is among the comma-separated list only (all of
// them when only is empty)
inline bool selected(const std::string& only, const char *name)
{
  if (only.empty()) {
    return true;
  }
  return ("," + only + ",").find(std::string(",") + name + ",") != std::string::npos;
}

// Runs loop() repeats times, and prints the best time
template<class Loop, class Result, class Mid, class Wid>
void bench(const char *library, const char *name, int repeats, Loop loop, const std::vector<Result>& r,
           Mid mid, Wid wid)
{
  double best = 1e300;
  for (int k = 0; k < repeats; ++k) {
    const auto start = std::chrono::steady_clock::now();
    loop();
    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
    best = std::min(best, elapsed.count());
  }
  // GAOL's midpoint() and width() leave the rounding upward: the sums are
  // computed afterwards, to nearest, as in the other programs
  std::vector<double> mids, wids;
  mids.reserve(r.size());
  wids.reserve(r.size());
  for (const Result& x : r) {
    mids.push_back(mid(x));
    wids.push_back(wid(x));
  }
  std::fesetround(FE_TONEAREST);
  double sum_mid = 0.0, sum_wid = 0.0;
  for (std::size_t i = 0; i < r.size(); ++i) {
    sum_mid += mids[i];
    sum_wid += wids[i];
  }
  std::printf("%s,%s,%lld,%d,%.9f,%.3f,%.17g,%.17g\n", library, name, static_cast<long long>(r.size()), repeats, best,
              best / static_cast<double>(r.size()) * 1e9, sum_mid, sum_wid);
  std::fflush(stdout);
}

#endif // BENCH_COMMON_H
