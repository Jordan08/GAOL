// The test of enclosure (see enclosure.h) with GAOL v5, whose elementary
// functions are CORE-MATH's, correctly rounded: the control, whose bounds
// must all enclose the exact values
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#include <gaol/gaol.h>
#include "enclosure.h"

/* GAOL v5 under its own names, gaol ("gaol5"), and, compiled with
   -DGAOL_ENCLOSURE_IEEE1788, under the names of IEEE 1788-2015,
   gaol_ieee1788 ("gaol5std"): the same functions but for the cube, pow(x, 3)
   in gaol and pown(x, 3) in gaol_ieee1788, both GAOL's integer power (GAOL
   v5, decided on 13 October 2026, TODO.md B) */
#ifdef GAOL_ENCLOSURE_IEEE1788
namespace names = gaol_ieee1788;
#else
namespace names = gaol;
#endif

struct Gaol
{
  typedef names::interval I;
#ifdef GAOL_ENCLOSURE_IEEE1788
  static constexpr const char *name = "gaol5std";
#else
  static constexpr const char *name = "gaol5";
#endif
  static I point(double x) { return I(x, x); }
  static double lower(const I& x) { return x.left(); }
  static double upper(const I& x) { return x.right(); }
  static I exp(const I& x) { return names::exp(x); }
  static I log(const I& x) { return names::log(x); }
  static I sin(const I& x) { return names::sin(x); }
  static I cos(const I& x) { return names::cos(x); }
  static I tan(const I& x) { return names::tan(x); }
  static I asin(const I& x) { return names::asin(x); }
  static I acos(const I& x) { return names::acos(x); }
  static I atan(const I& x) { return names::atan(x); }
  static I sinh(const I& x) { return names::sinh(x); }
  static I cosh(const I& x) { return names::cosh(x); }
  static I tanh(const I& x) { return names::tanh(x); }
  static I asinh(const I& x) { return names::asinh(x); }
  static I acosh(const I& x) { return names::acosh(x); }
  static I atanh(const I& x) { return names::atanh(x); }
  static I sqrt(const I& x) { return names::sqrt(x); }
#ifdef GAOL_ENCLOSURE_IEEE1788
  static I pow3(const I& x) { return names::pown(x, 3); }
#else
  static I pow3(const I& x) { return names::pow(x, 3); }
#endif
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
