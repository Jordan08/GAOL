#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: writes rows { x, n, lo, hi } for the table `negative[]` of
# large_powers() in tests/arithmetic.cpp: the tightest bounds of x^n for a
# large negative n (from -2^31 + 1 to -2^27), computed with mpmath on 2000
# bits and rounded downward and upward to a double,
#
#     v = mpmath.mpf(x) ** n
#     lo, hi = (float(mpmath.mpf(v, prec=53, rounding=r)) for r in 'du')
#
# for x within 220/|n| of 1, so that n log(x) is at most 220 in magnitude, as
# in the first rows of the table. A row is kept when the power is a normal
# double between 2^-1000 and 2^1000 and lies between 1/16 and 15/16 of the
# spacing of the doubles around it: the bounds of GAOL are the tightest only
# where x^n is farther from a double than twice the bound K of the error of
# 1/x^|n| (pown_bound_inverse() in gaol/core/gaol_interval_core.cpp: 2^-61.7 at
# n = -2^31 + 1), and these rows are at least 13 times farther in every build.
#
#     python3 negative_power_rows.py [COUNT] [--seed S]
#
# writes COUNT rows (40 by default) to the standard output, as the lines of
# the table. The 40 rows of the table are those of the default seed.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-25 by Jordan NININ

import argparse
import random

import mpmath

DEFAULT_SEED = 20261025


def rows(count, seed):
    mpmath.mp.prec = 2000
    rng = random.Random(seed)
    result = []
    while len(result) < count:
        n = -rng.randint(1 << 27, (1 << 31) - 1)
        delta = rng.uniform(0.2, 1.0) * 220.0 / -n
        x = 1.0 + (delta if rng.random() < 0.5 else -delta)
        v = mpmath.mpf(x) ** n
        lo, hi = (float(mpmath.mpf(v, prec=53, rounding=r)) for r in 'du')
        if lo == hi or not (2.0 ** -1000 < lo and hi < 2.0 ** 1000):
            continue
        where = (v - mpmath.mpf(lo)) / (mpmath.mpf(hi) - mpmath.mpf(lo))  # in 0..1 between the two doubles
        if mpmath.mpf(1) / 16 <= where <= mpmath.mpf(15) / 16:
            result.append((x, n, lo, hi))
    return result


def main():
    p = argparse.ArgumentParser(description='Rows of the table of large negative powers of tests/arithmetic.cpp.')
    p.add_argument('count', nargs='?', type=int, default=40)
    p.add_argument('--seed', type=int, default=DEFAULT_SEED)
    args = p.parse_args()
    for x, n, lo, hi in rows(args.count, args.seed):
        print('      { %s, %d, %s, %s },' % (x.hex(), n, lo.hex(), hi.hex()))


if __name__ == '__main__':
    main()
