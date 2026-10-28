// The inline functions of the headers that make their own check of the rounding direction, called with intervals built in
// the function from its arguments, so that their bounds are registers: the case of width() and bisect() on 64-bit and
// 32-bit ARM (#150), for tests/tools/asm-order/check_order.py to read. Only compiled, with -S.
//
// Copyright (c) 2026 ENSTA, France
// Created 2026-10-27 by Jordan NININ

#include "gaol/gaol.h"

double gaol_asm_width(double a, double b)
{
  return gaol::interval(a, b).width();
}

double gaol_asm_rad(double a, double b)
{
  return gaol::interval(a, b).rad();
}

void gaol_asm_mid_rad(double a, double b, double& m, double& r)
{
  gaol::interval(a, b).mid_rad(m, r);
}
