// The test of enclosure of doc/compare/enclosure.md, written once for every C++
// library: each elementary function is computed on the point intervals [x, x]
// of the arguments enclosure.py drew, and the bounds of the results are written
// for enclosure.py to check against mpmath. T gives the library: its interval
// type T::I, T::point(x) the interval [x, x], T::lower and T::upper, and the
// functions of ENCLOSURE_FUNCTIONS below; pow3 is the integer power x^3.
//
// enclosure_all<T>(DIR) reads DIR/args_<function>.bin (doubles, in the byte
// order of the machine) and writes DIR/<T::name>_<function>.bin, the lower and
// upper bound of each result, or NaN and NaN when the library threw an
// exception.
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#ifndef ENCLOSURE_H
#define ENCLOSURE_H

#include <cstdio>
#include <exception>
#include <limits>
#include <string>
#include <vector>

// The functions, in the order of enclosure.py
#define ENCLOSURE_FUNCTIONS(F)                                                  \
  F(exp) F(log) F(sin) F(cos) F(tan) F(asin) F(acos) F(atan) F(sinh) F(cosh)   \
  F(tanh) F(asinh) F(acosh) F(atanh) F(sqrt) F(pow3)

inline std::vector<double> enclosure_read(const std::string& path)
{
  std::vector<double> v;
  std::FILE *f = std::fopen(path.c_str(), "rb");
  if (!f) {
    std::fprintf(stderr, "cannot read %s\n", path.c_str());
    return v;
  }
  double x;
  while (std::fread(&x, sizeof x, 1, f) == 1) {
    v.push_back(x);
  }
  std::fclose(f);
  return v;
}

inline bool enclosure_write(const std::string& path, const std::vector<double>& v)
{
  std::FILE *f = std::fopen(path.c_str(), "wb");
  if (!f || std::fwrite(v.data(), sizeof(double), v.size(), f) != v.size()) {
    std::fprintf(stderr, "cannot write %s\n", path.c_str());
    if (f) {
      std::fclose(f);
    }
    return false;
  }
  return std::fclose(f) == 0;
}

// The bounds of f([x, x]) for each argument x of the function name, written for
// the library lib; T::I is the interval type, f calls the function
template<class T, class F>
bool enclosure_run(const std::string& dir, const char *lib, const char *name, F f)
{
  const std::vector<double> args = enclosure_read(dir + "/args_" + name + ".bin");
  if (args.empty()) {
    return false;
  }
  std::vector<double> bounds;
  bounds.reserve(2 * args.size());
  const double nan = std::numeric_limits<double>::quiet_NaN();
  for (const double x : args) {
    try {
      const typename T::I r = f(T::point(x));
      bounds.push_back(T::lower(r));
      bounds.push_back(T::upper(r));
    } catch (std::exception&) {
      bounds.push_back(nan);
      bounds.push_back(nan);
    }
  }
  return enclosure_write(dir + "/" + lib + "_" + name + ".bin", bounds);
}

// Every function of ENCLOSURE_FUNCTIONS with the library T, whose results are
// written under the name T::name
template<class T>
bool enclosure_all(const std::string& dir)
{
  bool ok = true;
#define ENCLOSURE_RUN(NAME)                                                     \
  ok = enclosure_run<T>(dir, T::name, #NAME, [](const typename T::I& x) { return T::NAME(x); }) && ok;
  ENCLOSURE_FUNCTIONS(ENCLOSURE_RUN)
#undef ENCLOSURE_RUN
  return ok;
}

#endif // ENCLOSURE_H
