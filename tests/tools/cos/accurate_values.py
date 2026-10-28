#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: the correct roundings of cos(x) in the four rounding
# directions, computed with mpmath (4000 bits) and exact rationals, apart from
# CORE-MATH and from GAOL, for the table of cos_accurate_path() in
# tests/core_math.cpp.
#
#     python3 accurate_values.py [--file tests/core_math.cpp]
#     python3 accurate_values.py --rows 0x1.2ee37e0264e18p+30 -0x1.8000000000009p-23
#
# The first form reads the rows {x, rn, ru, rd, rz} of cos_accurate_path()
# (nearest, upward, downward, toward zero), recomputes each from its x and
# compares: the exit status is 1 where a value differs. The second form
# prints the rows for the arguments given, to put in the table. The arguments
# are those of the accurate path of cos.c in the upward rounding, found with
# a copy of cos.c that counts the branches of cos_large_accurate() (see
# README.md).
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-27 by Jordan NININ

import argparse
import math
import os
import re
import struct
import sys
from fractions import Fraction

import mpmath

mpmath.mp.prec = 4000


def hexfloat(d):
    """d written as the table of the test writes it: no trailing zeros."""
    m, e = float.hex(d).split('p')
    if '.' in m:
        m = m.rstrip('0').rstrip('.')
    return m + 'p' + e


def roundings(x):
    """(nearest, upward, downward, toward zero) of cos(x), x a double."""
    sign, man, exp, _ = mpmath.cos(mpmath.mpf(x))._mpf_
    v = Fraction(int(man)) * Fraction(2) ** int(exp)
    if sign:
        v = -v
    lo = float(v)  # nearest, ties to even
    while Fraction(lo) > v:
        lo = math.nextafter(lo, -math.inf)
    while Fraction(math.nextafter(lo, math.inf)) <= v:
        lo = math.nextafter(lo, math.inf)
    hi = lo if Fraction(lo) == v else math.nextafter(lo, math.inf)
    if lo == hi:
        rn = lo
    else:
        below, above = v - Fraction(lo), Fraction(hi) - v
        if below != above:
            rn = lo if below < above else hi
        else:  # a tie: the even significand
            rn = lo if struct.unpack('<q', struct.pack('<d', lo))[0] % 2 == 0 else hi
    return rn, hi, lo, (lo if v > 0 else hi)


def row(x):
    return '{%s}' % ', '.join(hexfloat(d) for d in (x,) + roundings(x))


def table_rows(path):
    text = open(path).read()
    start = text.index('void cos_accurate_path()')
    start = text.index('const Value values[] = {', start)
    end = text.index('};', start)
    return re.findall(r'\{([^{}]+)\}', text[start + len('const Value values[] = {'):end])


def main():
    parser = argparse.ArgumentParser(description='Roundings of cos(x), against the table of cos_accurate_path()')
    parser.add_argument('--file', default=os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                                       '..', '..', 'core_math.cpp'))
    parser.add_argument('--rows', nargs='+', metavar='X', help='print the rows of these hexadecimal arguments')
    args = parser.parse_args()
    if args.rows:
        for a in args.rows:
            print(row(float.fromhex(a)) + ',')
        return 0
    rows = table_rows(args.file)
    bad = 0
    for r in rows:
        want = [float.fromhex(t.strip()) for t in r.split(',')]
        got = (want[0],) + roundings(want[0])
        if tuple(want) != got:
            bad += 1
            print('MISMATCH x = %s: table %s, computed %s' % (hexfloat(want[0]), r.strip(), row(want[0])))
    print('%d rows, %d mismatches' % (len(rows), bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
