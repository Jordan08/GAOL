#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of GAOL v5: generates pow_rel_values.h, the cases of the reverse
# functions of pow, powRev1 and powRev2 of IEEE 1788-2015 (10.5.4, Table
# 10.1), with which tests/reverse.cpp checks pow_rel() and
# pow_exponent_rel() (GAOL v5).
#
# IEEE 1788 defines, x being the whole line when not given,
#     powRev1(b, c, x) = hull{ x in x | x^y is defined and in c for a y in b },
#     powRev2(a, c, x) = hull{ y in x | x^y is defined and in c for an x in a },
# pow being that of Table 9.1: x^y = e^(y log x) for x > 0, 0^y = 0 for
# y > 0, and no value elsewhere. For each case, this script computes the
# tightest interval of doubles enclosing that hull, from the definition and
# apart from GAOL, by a decomposition into cells:
#  - the solutions are 0, for powRev1 where 0 is in x and in c and b holds a
#    y > 0, and for powRev2 the y > 0 of x where 0 is in a and in c; and the
#    points where x^y = e^(y t) is in c for a t = log(x) of log(a & ]0, +oo]) (or
#    of x & ]0, +oo] for powRev1), that is where y t is in log(c & ]0, +oo]);
#  - whether a single point t (powRev1, in the logarithms of the x) or y
#    (powRev2) is a solution is decided directly: the values y t, for the y
#    of b or the t of log(a), are an interval, whose ends are reached at
#    finite bounds and limits at infinite ones, which meets log(c) or not;
#  - that answer changes only where an end of these values crosses an end of
#    log(c), and at 0: at the t = log(c)/y and the y = log(c)/log(x), c, x
#    and y bounds of the arguments, at 0, and at the bounds of x. Between two
#    such points it is the answer at their middle, and the hull goes from
#    the first point or cell that is a solution to the last one;
#  - the reals are mpmath's at 2000 bits, and two of them within 2^-1900
#    relatively are taken as equal: the points the bounds of the arguments
#    give are equal only where the powers are, which makes x^y = c exactly,
#    and so do the bounds that are doubles (x = 4, y = 1/2 and c = 2). The
#    hull is then rounded outward, to the double itself where it is one.
# For each case, the table gives that tightest enclosure and the tightest
# enclosure of the solutions in x widened by one double, intersected with x,
# which GAOL's results have to lie between (see tests/reverse.cpp).
#
# The cases:
#  - those of pow_rev.itl of ITF1788 (https://github.com/oheim/ITF1788, at
#    commit b6ee1e2, the test cases of Oliver Heimlich, 2015-2016, whose
#    copying is permitted in any medium), structured after table B.1 and B.2
#    of Heimlich, "The General Interval Power Function", 2011: the empty set,
#    the whole line, 0^y = 0, 1^y = x^0 = 1, the bounds about 0 and 1, the
#    infinite ones. Their literals are read as the tightest intervals of
#    doubles holding them ([0.0, 0.9] has the upper bound 0.9 rounded
#    upward), and their results are checked to enclose the tightest ones;
#  - random cases, whose bounds are 0, 1, the infinities, small integers
#    and dyadic numbers, and doubles of many magnitudes;
#  - cases whose solutions are doubles exactly (x^y = c with x, y and c
#    doubles), and cases where a bound of x is the double next to an end of
#    the solutions, which the solutions leave out or not.
#
#     python3 tests/pow_rel_values.py <ITF1788>/itl/pow_rev.itl > tests/pow_rel_values.h
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ
import math
import random
import re
import struct
import sys
from fractions import Fraction

import mpmath

mpmath.mp.prec = 2000
INF = math.inf
MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)
EMPTY = "empty"
MP = mpmath.mpf
MP_INF = mpmath.inf
TOLERANCE = MP(2) ** -1900


#---------------------------------------------------------------------------
# Doubles
#---------------------------------------------------------------------------
def next_up(x):
    """The least double above x (math.nextafter(x, inf) of Python 3.9)"""
    if x != x or x == INF:
        return x
    if x == 0.0:
        return TINY
    n = struct.unpack("<q", struct.pack("<d", x))[0]
    n = n + 1 if x > 0 else n - 1
    return struct.unpack("<d", struct.pack("<q", n))[0]


def next_down(x):
    return -next_up(-x)


def mp(x):
    """A double as an mpf, exactly, the infinities as mpmath's"""
    if x == INF:
        return MP_INF
    if x == -INF:
        return -MP_INF
    return MP(x)


def compare(a, b):
    """The sign of a - b, for mpfs or infinities, 0 within 2^-1900 relatively"""
    if a == b:
        return 0
    if mpmath.isinf(a) or mpmath.isinf(b):
        return -1 if a < b else 1
    if abs(a - b) <= TOLERANCE * max(abs(a), abs(b)):
        return 0
    return -1 if a < b else 1


def round_down(v):
    """The greatest double at most v, an mpf or an infinity"""
    if v == MP_INF:
        return MAX  # Only a limit reaches +oo: the values are finite
    if v == -MP_INF:
        return -INF
    if compare(v, MP(MAX)) > 0:
        return MAX
    if compare(v, -MP(MAX)) < 0:
        return -INF
    d = float(v)  # the double nearest v, within the doubles
    if compare(mp(d), v) == 0:
        return d
    return next_down(d) if mp(d) > v else d


def round_up(v):
    return -round_down(-v)


def exp_round_down(t):
    """The greatest double at most e^t, t an mpf or -oo"""
    if t == -MP_INF:
        return 0.0
    if compare(t, mpmath.log(MP(MAX))) >= 0:
        return MAX
    if compare(t, mpmath.log(MP(TINY))) < 0:
        return 0.0
    return round_down(mpmath.exp(t))


def exp_round_up(t):
    """The least double at least e^t, t an mpf or +oo"""
    if t == MP_INF:
        return INF
    if compare(t, mpmath.log(MP(MAX))) > 0:
        return INF
    if compare(t, mpmath.log(MP(TINY))) <= 0:
        return 0.0 if t == -MP_INF else TINY
    return round_up(mpmath.exp(t))


#---------------------------------------------------------------------------
# The solutions, by cells
#---------------------------------------------------------------------------
def meets(a, b):
    """Whether two intervals of reals meet, each (lo, lo_reached, hi,
    hi_reached), an end not reached being a limit (the infinities)"""
    def below(lo, lo_reached, hi, hi_reached):
        c = compare(lo, hi)
        return c < 0 or (c == 0 and lo_reached and hi_reached)
    return below(a[0], a[1], b[2], b[3]) and below(b[0], b[1], a[2], a[3])


def log_of_powers(c):
    """log(c & ]0, +oo]), c holding a positive number: (lo, reached, hi, reached)"""
    cl, cu = c
    lo = (-MP_INF, False) if cl <= 0.0 else (mpmath.log(mp(cl)), True)
    hi = (MP_INF, False) if cu == INF else (mpmath.log(mp(cu)), True)
    return (lo[0], lo[1], hi[0], hi[1])


def products(s, ends):
    """The values s*v for the v of an interval of reals (lo, reached, hi,
    reached), s an mpf: an interval, an infinite end being a limit"""
    if s == 0:
        return (MP(0), True, MP(0), True)
    values = []
    for v, v_reached in ((ends[0], ends[1]), (ends[2], ends[3])):
        if mpmath.isinf(v):
            values.append((MP_INF if (v > 0) == (s > 0) else -MP_INF, False))
        else:
            values.append((s * v, v_reached))
    values.sort(key=lambda e: e[0])
    return (values[0][0], values[0][1], values[1][0], values[1][1])


def hull_of_cells(lo, hi, points, solution):
    """The hull [first, last] of the solutions in [lo, hi] (mpfs or
    infinities), the answer changing at the points only (mpfs within
    [lo, hi], its finite bounds among them): the first and the last point
    or cell between two points that holds solutions, or None"""
    points = sorted(points)
    distinct = []
    for p in points:
        if not distinct or compare(distinct[-1], p) != 0:
            distinct.append(p)
    cells = []  # (left, right, a point within)
    if lo == -MP_INF:
        cells.append((-MP_INF, distinct[0], distinct[0] - 1 - abs(distinct[0])))
    for i, p in enumerate(distinct):
        cells.append((p, p, p))
        if i + 1 < len(distinct):
            cells.append((p, distinct[i + 1], (p + distinct[i + 1]) / 2))
    if hi == MP_INF:
        cells.append((distinct[-1], MP_INF, distinct[-1] + 1 + abs(distinct[-1])))
    held = [cell for cell in cells if solution(cell[2])]
    if not held:
        return None
    return held[0][0], held[-1][1]


def within(points, lo, hi):
    return [p for p in points if compare(p, lo) >= 0 and compare(p, hi) <= 0]


def pow_rev1(b, c, x):
    """The tightest enclosure of powRev1(b, c, x), the x of x"""
    if EMPTY in (b, c, x):
        return EMPTY
    (yl, yu), (cl, cu), (xl, xu) = b, c, x
    if xu < 0.0 or cu < 0.0:
        return EMPTY
    xl = max(xl, 0.0)
    zero = xl == 0.0 and cl <= 0.0 and yu > 0.0  # 0^y = 0, y > 0
    lo = hi = None
    if xu > 0.0 and cu > 0.0:
        # t = log(x) for x > 0: x^y = e^(y t) in c where y t is in log(c)
        w = log_of_powers(c)
        tl = -MP_INF if xl == 0.0 else mpmath.log(mp(xl))
        tu = MP_INF if xu == INF else mpmath.log(mp(xu))
        exponents = (mp(yl), yl != -INF, mp(yu), yu != INF)
        points = [MP(0)] + [t for t in (tl, tu) if not mpmath.isinf(t)]
        for cb in (cl, cu):
            if 0.0 < cb < INF and cb != 1.0:
                points += [mpmath.log(mp(cb)) / mp(yb) for yb in (yl, yu) if abs(yb) < INF and yb != 0.0]
        cells = hull_of_cells(tl, tu, within(points, tl, tu), lambda t: meets(products(t, exponents), w))
        if cells is not None:
            lo, hi = exp_round_down(cells[0]), exp_round_up(cells[1])
    if zero:
        lo, hi = 0.0, (0.0 if hi is None else hi)
    return EMPTY if lo is None else (lo, hi)


def pow_rev2(a, c, x):
    """The tightest enclosure of powRev2(a, c, x), the y of x"""
    if EMPTY in (a, c, x):
        return EMPTY
    (al, au), (cl, cu), (yl, yu) = a, c, x
    if au < 0.0 or cu < 0.0:
        return EMPTY
    al = max(al, 0.0)
    zero = al == 0.0 and cl <= 0.0  # 0^y = 0 for the y > 0
    positive = au > 0.0 and cu > 0.0
    w = log_of_powers(c) if positive else None
    logs = (-MP_INF if al == 0.0 else mpmath.log(mp(al)), al > 0.0,
            MP_INF if au == INF else mpmath.log(mp(au)), au < INF)

    def solution(y):
        if zero and y > 0:
            return True
        return positive and meets(products(y, logs), w)

    ylo, yhi = mp(yl), mp(yu)
    points = [MP(0)] + [v for v in (ylo, yhi) if not mpmath.isinf(v)]
    for cb in (cl, cu):
        if 0.0 < cb < INF and cb != 1.0:
            points += [mpmath.log(mp(cb)) / mpmath.log(mp(ab)) for ab in (al, au) if 0.0 < ab < INF and ab != 1.0]
    cells = hull_of_cells(ylo, yhi, within(points, ylo, yhi), solution)
    if cells is None:
        return EMPTY
    return (-INF if cells[0] == -MP_INF else round_down(cells[0]), INF if cells[1] == MP_INF else round_up(cells[1]))


def tightest(f, b, c, x):
    return pow_rev1(b, c, x) if f == 1 else pow_rev2(b, c, x)


def near(f, b, c, x):
    """The tightest enclosure of the solutions in x widened by one double,
    intersected with x"""
    if EMPTY in (b, c, x):
        return EMPTY
    t = tightest(f, b, c, (next_down(x[0]), next_up(x[1])))
    if t == EMPTY:
        return EMPTY
    lo, hi = max(t[0], x[0]), min(t[1], x[1])
    return EMPTY if lo > hi else (lo, hi)


def encloses(a, t):
    if t == EMPTY:
        return True
    if a == EMPTY:
        return False
    return a[0] <= t[0] and t[1] <= a[1]


#---------------------------------------------------------------------------
# The cases of ITF1788
#---------------------------------------------------------------------------
def literal_bound(s):
    """A bound of an interval literal, as an exact Fraction or an infinity"""
    s = s.strip().lower()
    if s in ("infinity", "+infinity", "inf"):
        return INF
    if s in ("-infinity", "-inf"):
        return -INF
    if "0x" in s:
        return Fraction(float.fromhex(s))
    return Fraction(s)


def bound_down(v):
    if abs(v) == INF:
        return v
    d = float(v)  # the double nearest v: Python divides integers correctly rounded
    return next_down(d) if Fraction(d) > v else d


def bound_up(v):
    return v if abs(v) == INF else -bound_down(-v)


def literal(s):
    """The tightest interval of doubles holding the interval literal s"""
    inner = s.strip()[1:-1].strip().lower()
    if inner == "empty":
        return EMPTY
    if inner == "entire":
        return (-INF, INF)
    bounds = [literal_bound(t) for t in inner.split(",")]
    l, u = bounds[0], bounds[-1]
    return (bound_down(l), bound_up(u))


def itf1788_cases(path):
    """(f, b, c, x, x given, result of ITF1788)"""
    out = []
    pattern = re.compile(r"^\s*powRev([12])\s+(\[[^\]]*\])\s+(\[[^\]]*\])\s*(\[[^\]]*\])?\s*=\s*(\[[^\]]*\])\s*;")
    for line in open(path):
        m = pattern.match(line)
        if m:
            f, b, c, x, expected = int(m.group(1)), literal(m.group(2)), literal(m.group(3)), m.group(4), literal(m.group(5))
            out.append((f, b, c, (-INF, INF) if x is None else literal(x), x is not None, expected))
    return out


#---------------------------------------------------------------------------
# Random cases
#---------------------------------------------------------------------------
rng = random.Random(1788)

EXPONENTS = [-INF, -1e300, -100.0, -10.0, -3.0, -2.0, -1.5, -1.0, -0.5, -0.25, -1e-300, -0.0, 0.0, 1e-300, 1e-10,
             0.25, 1.0 / 3.0, 0.5, 1.0, 1.5, 2.0, 3.0, 7.0, 10.0, 100.0, 1e10, 1e300, INF]
BASES = [-INF, -2.0, -1.0, -0.0, 0.0, TINY, 1e-300, 0.25, 0.5, 0.75, next_down(1.0), 1.0, next_up(1.0), 1.5, 2.0,
         3.0, 4.0, 10.0, 1e300, MAX, INF]
POWERS = [-INF, -1.0, -0.0, 0.0, TINY, 1e-300, 0.1, 0.25, 0.5, 0.9, 1.0, 1.1, 2.0, 4.0, 8.0, 9.0, 10.0, 1e300,
          MAX, INF]


def random_double(lo_exp, hi_exp):
    return rng.choice([-1.0, 1.0]) * math.ldexp(rng.uniform(1.0, 2.0), rng.randint(lo_exp, hi_exp))


def random_bound(pool, positive):
    r = rng.random()
    if r < 0.6:
        return rng.choice(pool)
    d = random_double(-40, 40)
    return abs(d) if positive and r < 0.95 else d


def random_interval(pool, positive=False):
    while True:
        a, b = sorted([random_bound(pool, positive), random_bound(pool, positive)])
        if a < INF and b > -INF:
            return (a, b)


def random_cases(n):
    out = []
    for f in (1, 2):
        for _ in range(n):
            if f == 1:
                b, c, x = random_interval(EXPONENTS), random_interval(POWERS, True), random_interval(BASES, True)
            else:
                b, c, x = random_interval(BASES, True), random_interval(POWERS, True), random_interval(EXPONENTS)
            x_given = rng.random() < 0.7
            out.append((f, b, c, x if x_given else (-INF, INF), x_given, "random"))
    return out


def exact_power(x, n):
    """x^n for a double x > 0 and a nonzero integer n, as a double where it is
    one exactly, None otherwise"""
    p = Fraction(x) ** n
    d = float(p) if p < Fraction(MAX) * 2 else INF
    return d if d < INF and d > 0.0 and Fraction(d) == p else None


def exact_cases():
    """Solutions that are doubles: c = x^y with x, y and c doubles, y an
    integer or the inverse of a power of two"""
    out = []
    for x0 in [0.5, 0.75, 1.5, 2.0, 3.0, 4.0, 9.0, 16.0, 0.0625, 2.0 ** 100, 2.0 ** -100, 3.0 ** 20]:
        for n in [1, 2, 3, 5, -1, -2, -3]:
            c0 = exact_power(x0, n)
            if c0 is None:
                continue
            # x0^n = c0, and c0^(1/n) = x0
            out.append((1, (float(n), float(n)), (c0, c0), (-INF, INF), False, "exact"))
            out.append((1, (float(n), float(n)), (c0, c0), (x0, x0), True, "exact"))
            out.append((1, (float(n), float(n)), (c0, 2 * c0), (0.0, x0), True, "exact"))
            out.append((2, (x0, x0), (c0, c0), (-INF, INF), False, "exact"))
            out.append((2, (x0, x0), (c0, c0), (float(n), float(n)), True, "exact"))
            out.append((2, (x0, 2 * x0), (c0, c0), (float(n), INF), True, "exact"))
            for k in [2, 4]:
                # x0^(1/k), where it is a double r, and r^k = x0
                y = 1.0 / k
                r = x0 ** y
                if Fraction(r) ** k == Fraction(x0):
                    out.append((1, (y, y), (r, r), (-INF, INF), False, "exact"))
                    out.append((1, (y, 2 * y), (r, r), (0.0, x0), True, "exact"))
                    out.append((2, (x0, x0), (r, r), (-INF, INF), False, "exact"))
                    out.append((2, (x0, x0), (r, r), (y, y), True, "exact"))
    return out


def boundary_cases():
    """x ending at the double next to an end of the solutions: x^y = c for x
    the root, irrational, of c = 2, 3, 10 and y = 2, 3, -2, 0.5..."""
    out = []
    for c0 in [2.0, 3.0, 10.0, 0.1, 1e300]:
        for y0 in [2.0, 3.0, -2.0, 0.5, 1.5, -0.75, 7.0]:
            root = mpmath.power(mp(c0), 1 / mp(y0))
            if root > MP(MAX) or root < MP(TINY):
                continue
            dn, up = round_down(root), round_up(root)
            if dn == up:
                continue
            for x in [(0.0, dn), (up, INF), (dn, dn), (up, up), (dn, up), (next_down(dn), dn), (up, next_up(up))]:
                out.append((1, (y0, y0), (c0, c0), x, True, "boundary"))
                out.append((1, (y0, y0), (c0, INF), x, True, "boundary"))
                out.append((1, (y0, y0), (0.0, c0), x, True, "boundary"))
        # The exponent log(c0)/log(x0), x0 a double
        for x0 in [2.0, 3.0, 0.5, 10.0]:
            e = mpmath.log(mp(c0)) / mpmath.log(mp(x0))
            edn, eup = round_down(e), round_up(e)
            if edn == eup:
                continue
            for y in [(-INF, edn), (eup, INF), (edn, edn), (eup, eup), (edn, eup), (next_down(edn), edn),
                      (eup, next_up(eup))]:
                out.append((2, (x0, x0), (c0, c0), y, True, "boundary"))
                out.append((2, (x0, x0), (c0, INF), y, True, "boundary"))
                out.append((2, (x0, x0), (0.0, c0), y, True, "boundary"))
    return out


#---------------------------------------------------------------------------
# The header
#---------------------------------------------------------------------------
def double(x):
    if x == INF:
        return "inf"
    if x == -INF:
        return "-inf"
    if x == 0.0:
        return "-0x0p+0" if math.copysign(1.0, x) < 0 else "0x0p+0"
    # The hexadecimal form without the zeros ending the significand: 0x1p+1
    return re.sub(r"\.?0+p", "p", x.hex())


def interval(a):
    if a == EMPTY:
        return "E"
    if a == (-INF, INF):
        return "W"
    return "i(%s, %s)" % (double(a[0]), double(a[1]))


def main():
    table, wider = [], 0
    for f, b, c, x, x_given, expected in itf1788_cases(sys.argv[1]):
        t = tightest(f, b, c, x)
        if not encloses(expected, t):
            raise ValueError("ITF1788's powRev%d(%r, %r, %r) = %r does not enclose %r" % (f, b, c, x, expected, t))
        if expected != t:
            wider += 1
            print("ITF1788 wider than the tightest: powRev%d(%r, %r, %r) = %r, tightest %r"
                  % (f, b, c, x, expected, t), file=sys.stderr)
        table.append((f, b, c, x, x_given, t, near(f, b, c, x), "ITF1788"))
    n_itf1788 = len(table)
    seen = set()
    for f, b, c, x, x_given, source in random_cases(400) + exact_cases() + boundary_cases():
        if (f, b, c, x, x_given) not in seen:
            seen.add((f, b, c, x, x_given))
            table.append((f, b, c, x, x_given, tightest(f, b, c, x), near(f, b, c, x), source))

    print("// Generated by tests/pow_rel_values.py: the reverse functions of pow of IEEE")
    print("// 1788-2015, powRev1 and powRev2, on doubles, in the cases of pow_rev.itl of")
    print("// ITF1788 and in random and structured ones, with the tightest enclosure of the")
    print("// hull the standard defines, and that of the solutions in x widened by one")
    print("// double, intersected with x. Do not edit.")
    print("//")
    print("// Copyright (c) 2026 ENSTA, France")
    print("//")
    print("// Created 2026-10-06 by Jordan NININ")
    print("#ifndef GAOL_TESTS_POW_REL_VALUES_H")
    print("#define GAOL_TESTS_POW_REL_VALUES_H")
    print("namespace pow_rel_values")
    print("{")
    print("  // Constant expressions, which the compiler writes as data rather than")
    print("  // initializing the table at run time, at length")
    print("  constexpr double inf = std::numeric_limits<double>::infinity();")
    print("  // An interval of the table: i(l, u), E the empty set, W the whole line")
    print("  struct I { double l, u; bool empty; };")
    print("  constexpr I E = { 0.0, 0.0, true }, W = { -inf, inf, false };")
    print("  constexpr I i(double l, double u) { return I{ l, u, false }; }")
    print("  // f: 1 for powRev1(b, c, x), the x of x with x^y in c for a y of b, and 2")
    print("  // for powRev2(b, c, x), the y of x with a^y in c for an a of b; x is W when")
    print("  // the case does not give it (x_given false); tightest, the tightest interval")
    print("  // enclosing the hull; near, the tightest enclosure of the solutions in x")
    print("  // widened by one double, intersected with x; from, where the case comes from")
    print("  struct Case { int f; I b, c, x; bool x_given; I tightest, near; const char* from; };")
    print("  constexpr Case cases[] = {")
    for f, b, c, x, x_given, t, n, source in table:
        print("    { %d, %s, %s, %s, %s, %s, %s, \"%s\" }," % (
            f, interval(b), interval(c), interval(x), "true" if x_given else "false", interval(t), interval(n), source))
    print("  };")
    print("}")
    print("#endif // GAOL_TESTS_POW_REL_VALUES_H")
    counts = {}
    for row in table:
        counts[row[-1]] = counts.get(row[-1], 0) + 1
    print("pow_rel_values.py: %d cases of ITF1788 (%d of whose results are wider than the tightest); %s"
          % (n_itf1788, wider, ", ".join("%s %d" % kv for kv in sorted(counts.items()))), file=sys.stderr)


if __name__ == "__main__":
    main()
