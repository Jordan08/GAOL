#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: checks the bounds of pow in a rows file against the exact
# power, computed apart from GAOL with 500 bits of precision by mpmath
# (https://mpmath.org).
#
# A rows file has one box per line, "X | Y | S | H" (see gen_table.py): the
# base X, the exponent Y, S = gaol_ieee1788::pow(X, Y) and H = gaol::pow(X, Y),
# each "empty" or two numbers "l u" (decimal or hexadecimal, inf, -inf). They
# come from the table of tests/ieee1788.cpp (gen_table.py extract) or from a
# build of GAOL (evalrows).
#
# S has to be the pow of IEEE 1788-2015 (Table 9.1): the hull of x^y over the
# x of X in [0, +oo] and the y of Y, x = 0 taking only y > 0; H the same, but
# for a degenerate integer exponent Y = [n]: pown(X, n) on the whole of X
# within the ints, [-oo, +oo] beyond them. For each, the bounds have to enclose
# the exact hull, and be at most --max-distance doubles (1 by default) from
# the tightest ones, the doubles on each side of the exact bounds. Each result
# is printed with its distances, and the tightest bounds where it is not them;
# the boxes are numbered from 1, the comments left out, as gen_table.py diff
# numbers them.
#
#     python3 checkrows.py expected.txt [--max-distance 1] [--quiet]
#
# The exit status is 1 when a result fails.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ

import argparse
import struct
import sys

import mpmath
from mpmath import mpf

mpmath.mp.prec = 500
INF = float('inf')
DMAX = 1.7976931348623157e308


def number(t):
    if t in ('inf', '+inf', 'oo'):
        return INF
    if t in ('-inf', '-oo'):
        return -INF
    return float.fromhex(t) if '0x' in t.lower() else float(t)


def parse(field):
    """None for the empty set, else (l, u) as floats"""
    t = field.split()
    if t == ['empty']:
        return None
    return (number(t[0]), number(t[1]))


def ordinal(d):
    """A monotone integer image of the doubles, -0 and +0 both 0, the infinities next to DBL_MAX"""
    if d == 0.0:
        return 0
    (b,) = struct.unpack('<q', struct.pack('<d', d))
    return b if b >= 0 else -(b & 0x7fffffffffffffff)


def from_ordinal(o):
    if o >= 0:
        return struct.unpack('<d', struct.pack('<q', o))[0]
    return -struct.unpack('<d', struct.pack('<q', -o))[0]


def snap(v):
    """v, or the double it is within 2^-200 relative of: an exact power that 500 bits computed inexactly"""
    if v == 0 or v in (mpmath.inf, -mpmath.inf):
        return v
    try:
        d = float(v)
    except OverflowError:
        return v
    if d != 0.0 and d not in (INF, -INF) and abs(v - mpf(d)) < abs(v) * mpf(2) ** -200:
        return mpf(d)
    return v


def rd(v):
    """The greatest double at most v (v an mpf, or an infinity)"""
    if v in (-INF, -mpmath.inf):
        return -INF
    if v in (INF, mpmath.inf):
        return INF
    v = mpf(v)
    if v >= mpf(DMAX):
        return DMAX
    if v < -mpf(DMAX):
        return -INF
    d = float(v)  # to nearest, 0 or a subnormal below the least normal double
    if mpf(d) > v:
        d = from_ordinal(ordinal(d) - 1)
    return d


def ru(v):
    """The least double at least v"""
    return -rd(-v) if v not in (INF, -INF) else v


def power(x, y):
    """x^y for an extended double x >= 0 and an extended double y, with its limits (x^0 = 1, 0^y, oo^y, x^oo)"""
    if y == 0:
        return mpf(1)
    if x == 0:
        return mpf(0) if y > 0 else mpmath.inf
    if x == INF:
        return mpmath.inf if y > 0 else mpf(0)
    if y in (INF, -INF):
        if x == 1:
            return mpf(1)
        return mpmath.inf if (x > 1) == (y == INF) else mpf(0)
    return snap(mpf(x) ** mpf(y))


def exact_standard(X, Y):
    """The hull of x^y for x in X cut to [0, +oo], y in Y (Table 9.1): None for the empty set, else (lo, hi)"""
    if X is None or Y is None:
        return None
    xl, xu = max(X[0], 0.0), X[1]
    yl, yu = Y
    if xl > xu:
        return None
    if xu == 0.0:
        return (mpf(0), mpf(0)) if yu > 0 else None
    # x^y is monotonic in x and in y on each side of y = 0: the extrema are at
    # the bounds of X and at yl, yu and 0, limits at x = 0 included
    ys = [yl, yu] + ([0.0] if yl <= 0.0 <= yu else [])

    def least(y):
        return power(xl, y) if y > 0 else (power(xu, y) if y < 0 else mpf(1))

    def greatest(y):
        return power(xu, y) if y > 0 else (power(xl, y) if y < 0 else mpf(1))
    return (min(least(y) for y in ys), max(greatest(y) for y in ys))


def exact_pown(X, n):
    """The hull of x^n for x in X, n an integer (pown of Table 9.1): None for the empty set"""
    if X is None:
        return None
    a, b = X
    if n == 0:
        return (mpf(1), mpf(1))
    m = abs(n)
    odd = (m % 2 == 1)

    def p(x):  # x^m, its limits included
        if x == INF:
            return mpmath.inf
        if x == -INF:
            return -mpmath.inf if odd else mpmath.inf
        return snap(mpf(x) ** m)
    if n > 0:
        if odd or a >= 0:
            return (p(a), p(b))
        if b <= 0:
            return (p(b), p(a))
        return (mpf(0), max(p(a), p(b)))

    def inv(v):  # 1/v, the limits included
        if v == 0:
            return mpmath.inf
        if v in (mpmath.inf, -mpmath.inf):
            return mpf(0)
        return snap(1 / v)
    if a == 0.0 and b == 0.0:
        return None
    if a >= 0:  # decreasing
        return (inv(p(b)), mpmath.inf if a == 0.0 else inv(p(a)))
    if b <= 0:
        if b == 0.0:
            return (-mpmath.inf, inv(p(a))) if odd else (inv(p(a)), mpmath.inf)
        return (inv(p(b)), inv(p(a))) if odd else (inv(p(a)), inv(p(b)))
    if odd:  # a < 0 < b
        return (-mpmath.inf, mpmath.inf)
    return (inv(max(p(a), p(b))), mpmath.inf)


def exact_gaol(X, Y):
    """gaol::pow(X, Y): pown for a degenerate integer exponent, [-oo, +oo] beyond the ints, Table 9.1 otherwise"""
    if X is None or Y is None:
        return None
    yl, yu = Y
    if yl == yu and abs(yl) < INF and yl == int(yl):
        if -2147483648 <= yl <= 2147483647:
            return exact_pown(X, int(yl))
        return (-mpmath.inf, mpmath.inf)
    return exact_standard(X, Y)


def hexa(d):
    """d in hexadecimal, as printf's %a writes it"""
    if d in (INF, -INF):
        return 'inf' if d > 0 else '-inf'
    mant, exp = d.hex().split('p')
    return mant.rstrip('0').rstrip('.') + 'p' + exp


def check(got, exact, limit):
    """(ok, text, distance) for the bounds got (None or (l, u)) and the exact hull (None or (lo, hi))"""
    if got is None or exact is None:
        if (got is None) == (exact is None):
            return True, 'empty', 0
        return False, 'empty set mismatch: %s for %s' % (
            'empty' if got is None else 'non-empty', 'empty' if exact is None else 'non-empty'), 0
    lo, hi = exact
    l, r = got
    encloses = (l == -INF or mpf(l) <= lo) and (r == INF or mpf(r) >= hi)
    tl, tr = rd(lo), ru(hi)
    dl, dr = abs(ordinal(tl) - ordinal(l)), abs(ordinal(tr) - ordinal(r))
    ok = encloses and max(dl, dr) <= limit
    text = '%s, distance lower %d upper %d' % ('encloses' if encloses else 'DOES NOT ENCLOSE', dl, dr)
    if dl or dr:
        text += ', tightest %s %s' % (hexa(tl), hexa(tr))
    return ok, text, max(dl, dr)


def main():
    p = argparse.ArgumentParser(description='Checks the bounds of pow in a rows file against mpmath.')
    p.add_argument('rows', help='a rows file: X | Y | S | H')
    p.add_argument('--max-distance', type=int, default=1,
                   help='the largest distance allowed from the tightest bounds, in doubles (1)')
    p.add_argument('--quiet', action='store_true', help='print only the results not at the tightest bounds')
    a = p.parse_args()
    boxes = failures = 0
    histogram = {}
    for n, line in enumerate(open(a.rows), 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        f = [x.strip() for x in line.split('|')]
        if len(f) != 4:
            sys.exit('checkrows.py: %s:%d: not a box with its bounds: %s' % (a.rows, n, line))
        boxes += 1
        X, Y = parse(f[0]), parse(f[1])
        for name, got, exact in (('std', parse(f[2]), exact_standard(X, Y)), ('hyb', parse(f[3]), exact_gaol(X, Y))):
            ok, text, d = check(got, exact, a.max_distance)
            histogram[d] = histogram.get(d, 0) + 1
            failures += not ok
            if not a.quiet or not ok or d:
                print('box %3d %s %-4s %s | %s -> %s: %s' % (boxes, name, 'ok' if ok else 'FAIL', f[0], f[1],
                                                             f[2 if name == 'std' else 3], text))
    print('boxes: %d, results: %d, failed: %d; results by distance from the tightest bounds, in doubles: %s'
          % (boxes, 2 * boxes, failures, ', '.join('%d: %d' % kv for kv in sorted(histogram.items()))))
    sys.exit(1 if failures else 0)


if __name__ == '__main__':
    main()
