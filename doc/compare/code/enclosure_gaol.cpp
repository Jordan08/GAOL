// The test of enclosure (see enclosure.h) with GAOL v5, whose elementary
// functions are CORE-MATH's, correctly rounded: the control, whose bounds
// must all enclose the exact values
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#include <gaol/gaol.h>
#include "enclosure.h"

struct Gaol
{
  typedef gaol::interval I;
  static constexpr const char *name = "gaol5";
  static I point(double x) { return I(x, x); }
  static double lower(const I& x) { return x.left(); }
  static double upper(const I& x) { return x.right(); }
  static I exp(const I& x) { return gaol::exp(x); }
  static I log(const I& x) { return gaol::log(x); }
  static I sin(const I& x) { return gaol::sin(x); }
  static I cos(const I& x) { return gaol::cos(x); }
  static I tan(const I& x) { return gaol::tan(x); }
  static I asin(const I& x) { return gaol::asin(x); }
  static I acos(const I& x) { return gaol::acos(x); }
  static I atan(const I& x) { return gaol::atan(x); }
  static I sinh(const I& x) { return gaol::sinh(x); }
  static I cosh(const I& x) { return gaol::cosh(x); }
  static I tanh(const I& x) { return gaol::tanh(x); }
  static I asinh(const I& x) { return gaol::asinh(x); }
  static I acosh(const I& x) { return gaol::acosh(x); }
  static I atanh(const I& x) { return gaol::atanh(x); }
  static I sqrt(const I& x) { return gaol::sqrt(x); }
  static I pow3(const I& x) { return gaol::pow(x, 3); }
};

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::fprintf(stderr, "usage: %s DIR\n", argv[0]);
    return 1;
  }
  const int status = enclosure_all<Gaol>(argv[1]) ? 0 : 1;
  gaol::cleanup();
  return status;
}
