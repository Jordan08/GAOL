#!/usr/bin/env python3
# Tests of GAOL v5: generates compoundm1_values.h, the values of
# (1 + x)^y - 1 (compoundm1 of IEEE 1788-2015, Table 10.5) tests/compoundm1.cpp
# compares GAOL's bounds with.
#
# - point_values: for a pair of doubles (x, y), the greatest double at most
#   (1 + x)^y - 1 and the least double at least it, which are the same double
#   where the value is one. The value is computed with 1500 bits of precision
#   by mpmath (https://mpmath.org), as exp(y log1p(x)) - 1, and again with
#   3000 and 7000 bits while the interval around it holds a double. A double
#   in it is the value itself if (1 + x)^y = 1 + d, which Fractions decide
#   exactly (y = p / q is a ratio of integers, and the powers of 1 + x and of
#   1 + d are compared as such): mpmath alone says 2.375 + 10^-450 for
#   (1.5)^3 - 1, and 2 u + 10^-640 for (1 + u)^2 - 1, u = 2^-1074, where the
#   tightest bounds are [2 u, 3 u]. A pair left ambiguous after 7000 bits is
#   left out.
# - box_values: the hull of the function over a box [xl, xu] x [yl, yu], which
#   is monotone in x and in y on each side of the lines x = 0 and y = 0, where
#   it is 0: the hull of the values at the four corners and of 0 where the box
#   crosses a line. At x = -1 and x = +oo, and for an infinite y, the value is
#   the limit, +oo or -1; x < -1 is outside the domain, and x = -1 is in it
#   for y > 0 only. This reads the definition and not GAOL's code, which
#   chooses the corner of each bound.
# - log1p_values and expm1_values: the logarithm log(1 + x) and the
#   exponential exp(t) - 1 to 256 bits, which the balls of
#   gaol/math/gaol_compoundm1.h have to hold, their widths being bounded.
# - power_triples and product_triples: whether (1 + x)^y is 1 + d, or d, and
#   whether x y is d, for doubles, exactly, by fractions: the exact values and
#   the doubles next to them, which a test of the equality has to tell apart.
# - g_values: (exp(r) - 1) / r on 127 bits, the series of the balls.
#
#     python3 tests/compoundm1_values.py > tests/compoundm1_values.h
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-04)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

import math
import random
import sys
from fractions import Fraction

import mpmath

MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)
INF = math.inf
rng = random.Random(20261104)


def to_fraction(v):
    """The mpf v, exactly"""
    sign, man, exp, _ = v._mpf_
    f = Fraction(int(man)) * (Fraction(2) ** int(exp))
    return -f if sign else f


def neighbours(f):
    """The greatest double at most the Fraction f and the least one at least it
    (-inf or +inf beyond the largest double: MAX and +inf above it)"""
    if f > Fraction(MAX):
        return MAX, INF
    if f < -Fraction(MAX):
        return -INF, -MAX
    d = float(f)
    if Fraction(d) == f:
        return d, d
    if Fraction(d) < f:
        return d, math.nextafter(d, INF)
    return math.nextafter(d, -INF), d


def is_exact(x, y, d):
    """Whether (1 + x)^y is 1 + d exactly, for doubles: y = p / q"""
    ratio = Fraction(y)
    p, q = ratio.numerator, ratio.denominator
    if abs(p) > 3000 or q > 4096:
        return False
    s, w = 1 + Fraction(x), 1 + Fraction(d)
    if w <= 0:
        return False
    if p > 0:
        return s ** p == w ** q
    return (s ** (-p)) * (w ** q) == 1


def compound_bounds(x, y):
    """The doubles around (1 + x)^y - 1 for x > -1 other than 0 and y other
    than 0, or None if mpmath does not settle them"""
    for prec in (1500, 3000, 7000):
        mpmath.mp.prec = prec
        t = mpmath.mpf(y) * mpmath.log1p(mpmath.mpf(x))
        if t > 2000:
            return MAX, INF
        if t < -1000:
            return -1.0, math.nextafter(-1.0, 0.0)
        value = to_fraction(mpmath.expm1(t))
        width = abs(value) * Fraction(1, 2 ** (prec - 40))
        below, _ = neighbours(value - width)
        _, above = neighbours(value + width)
        if above == math.nextafter(below, INF) or below == above:
            return below, above
        d = math.nextafter(below, INF)  # a double in the interval
        if is_exact(x, y, d):
            return d, d
    return None


def corner(x, y):
    """The bounds of the function at a corner of a box, the limits included"""
    if x == 0 or y == 0:
        return 0.0, 0.0
    if x == -1 or x == INF or abs(y) == INF:
        v = INF if (x > 0) == (y > 0) else -1.0
        return v, v
    return compound_bounds(x, y)


def box_hull(xl, xu, yl, yu):
    """The bounds of the hull over the box, or None if it is empty"""
    xl = max(xl, -1.0)
    if xu < xl:
        return None
    if xu == -1.0:
        return (-1.0, -1.0) if yu > 0 else None
    lows, highs = [], []
    for x in (xl, xu):
        for y in (yl, yu):
            low, high = corner(x, y)
            lows.append(low)
            highs.append(high)
    if xl <= 0 <= xu or yl <= 0 <= yu:
        lows.append(0.0)
        highs.append(0.0)
    return min(lows), max(highs)


def literal(v):
    if v == INF:
        return "gaol_tests::inf"
    if v == -INF:
        return "-gaol_tests::inf"
    return float(v).hex()


def log1p_double(x):
    mpmath.mp.prec = 200
    return float(mpmath.log1p(mpmath.mpf(x)))


def random_double(emin, emax):
    return math.ldexp(1.0 + rng.getrandbits(52) / 2.0 ** 52, rng.randint(emin, emax))


# --- The pairs (x, y)

special_x = [TINY, 2.0 ** -1073, 2.0 ** -1022, 1e-300, 2.0 ** -200, 2.0 ** -100, 2.0 ** -54, 2.0 ** -53,
             2.0 ** -52, 1e-10, 0.001, 0.1, 0.5, 1.0, 1.5, 2.0, 3.0, 7.0, 8.0, 100.0, 2.0 ** 30, 2.0 ** 53,
             1e100, 1e300, MAX,
             -TINY, -2.0 ** -100, -2.0 ** -53, -1e-10, -0.001, -0.1, -0.5, -0.75, -0.875,
             -1 + 2.0 ** -52, -1 + 2.0 ** -53, -1 + 2.0 ** -30, -0.999]
special_y = [TINY, 1e-300, 2.0 ** -60, 1e-5, 0.25, 0.5, 1.0, 1.5, 2.0, 3.0, 10.0, 1000.0, 1e6, 2.0 ** 53,
             1e20, 1e100, MAX]
special_y = special_y + [-y for y in special_y]

points = [(x, y) for x in special_x for y in special_y]

# Random pairs of every magnitude, y chosen for a value of y log(1 + x) of
# every size, from tiny to past the doubles
for _ in range(900):
    kind = rng.random()
    if kind < 0.3:
        x = rng.uniform(-1, 5)
    elif kind < 0.5:
        x = -1 + 2.0 ** -rng.uniform(1, 53)
    elif kind < 0.8:
        x = rng.choice([-1, 1]) * 10.0 ** rng.uniform(-300, 0) if rng.random() < 0.7 else 10.0 ** rng.uniform(0, 300)
    else:
        x = 10.0 ** rng.uniform(-20, 20)
    if not x > -1 or x == 0:
        continue
    log = log1p_double(x)
    if log == 0:
        continue
    t = rng.choice([1e-8, 1e-3, 0.1, 1, 10, 100, 700]) * rng.uniform(-1, 1) if rng.random() < 0.85 \
        else rng.uniform(-60, 800)
    y = t / log
    if rng.random() < 0.15:
        y = 10.0 ** rng.uniform(-10, 10) * rng.choice([-1, 1])
    if y == 0 or math.isinf(y):
        continue
    points.append((x, y))

# y log(1 + x) next to the thresholds: the largest double (709.78...), the
# doubles past which the value is -1 + 2^-53 or less (36.7...), the
# reduction by log(2) (0.3466, 1.04), the cut of -64 and of 2048
for t in [709.782712893384, 709.78271289338397, 709.7827128933841, 710.0, 745.0, 2047.0, 2049.0,
          36.7368, -36.7368005696771, -36.73680056967711, -37.0, -40.0, -63.9, -64.1,
          0.3465, 0.3466, 0.3467, 0.693, 0.6931471805599453, 1.0397, -0.3466, -0.693, -1.0, 1e-300]:
    for _ in range(12):
        x = rng.choice([rng.uniform(-1, 0), rng.uniform(0, 3), 10.0 ** rng.uniform(-30, 30),
                        -1 + 2.0 ** -rng.uniform(1, 53)])
        if not x > -1 or x == 0:
            continue
        log = log1p_double(x)
        if log == 0:
            continue
        y = t / log
        if y == 0 or math.isinf(y):
            continue
        points += [(x, y), (x, math.nextafter(y, INF)), (x, math.nextafter(y, -INF))]

# The values that are doubles: (c^q)^(p/q) - 1 = c^p - 1 for a dyadic c, and
# the powers of two, 2^m - 1 with m past 53, which a double next to 2^m holds
for _ in range(400):
    q = rng.choice([1, 1, 2, 2, 4, 8, 16, 32])
    c = Fraction(rng.choice([1, 3, 5, 7, 9, 15, 17, 31, 33, 255, 257])) * Fraction(2) ** rng.randint(-8, 8)
    x = c ** q - 1
    if Fraction(float(x)) != x or not x > -1 or x == 0:
        continue
    p = rng.choice([1, 2, 3, 5, 7, 11, -1, -2, -3, -5, 1, 1, 2, 4, 6, 10])
    y = Fraction(p, q) if rng.random() < 0.8 else Fraction(rng.randint(1, 40), q)
    if y == 0 or Fraction(float(y)) != y:
        continue
    points.append((float(x), float(y)))
for m in [54, 60, 100, 112, 113, 120, 200, 500, 1000, 1023]:
    for x, y in [(1.0, float(m)), (-0.5, float(-m)), (3.0, m / 2.0), (-0.75, -m / 2.0), (15.0, m / 4.0)]:
        points.append((x, y))
# A tiny x, whose value is next to y x: the double y x if it is one, the
# value being on one side of it by y (y - 1) x^2 / 2
for _ in range(250):
    e = rng.randint(-1074, -3) if rng.random() < 0.7 else rng.randint(-200, -50)
    m = rng.choice([1, 1, 1, 3, 5, 7, (1 << 52) + 1, rng.getrandbits(53) | 1])
    x = math.ldexp(float(m), e) * rng.choice([1, -1])
    if x == 0 or not x > -1:
        continue
    y = rng.choice([2.0, 3.0, 0.5, -1.0, -2.0, 10.0, 1.5, 0.25, 4.0, 1 + 2.0 ** -52, 1 - 2.0 ** -53,
                    2.0 ** rng.randint(-60, 60), -(2.0 ** rng.randint(0, 60)), float(rng.randint(2, 1000)),
                    1.0 / 3, 0.1])
    points.append((x, y))
# y next to 1, and x subnormal with a tiny y: subnormal values
for _ in range(120):
    x = rng.choice([rng.uniform(-0.9, 5), 10.0 ** rng.uniform(-30, 0), -10.0 ** rng.uniform(-30, -0.01)])
    if not x > -1 or x == 0:
        continue
    y = 1.0
    for _ in range(rng.randint(1, 3)):
        y = math.nextafter(y, rng.choice([0.0, 2.0]))
    points.append((x, y))
for _ in range(80):
    points.append((math.ldexp(rng.uniform(1, 2), rng.randint(-1074, -900)), 10.0 ** rng.uniform(-300, -1)))

# y = -1, whose value -x / (1 + x) is the quotient of two integers
# (reciprocal_m1() of the header), and the domain of the fast phase: the
# pairs below come from a generator of their own, so as not to move the
# ones that were drawn before
rng2 = random.Random(20261105)


def reciprocal_x():
    kind = rng2.random()
    if kind < 0.25:
        return rng2.choice([-1, 1]) * math.ldexp(1.0 + rng2.getrandbits(52) / 2.0 ** 52, -rng2.randint(1, 130))
    if kind < 0.40:
        return math.ldexp(1.0 + rng2.getrandbits(52) / 2.0 ** 52, rng2.randint(0, 130))
    if kind < 0.55:
        return -1 + math.ldexp(1.0 + rng2.getrandbits(52) / 2.0 ** 52, -rng2.randint(1, 53))
    if kind < 0.70:
        return float(rng2.randint(1, 60)) * rng2.choice([1, 0.5, 0.25, 0.125])
    if kind < 0.80:
        return -float(rng2.randint(1, 31)) / rng2.choice([32, 64, 128, 1024])
    return rng2.uniform(-1, 2)


# The contrived values x = 2^e + 2^(2e-1) and their opposites, whose 1/(1 + x) is within 2^-155 of a double, and
# the doubles next to them
for e in range(-60, -29):
    x = math.ldexp(1.0, e) + math.ldexp(1.0, 2 * e - 1)
    for v in (x, -x, math.nextafter(x, INF), math.nextafter(x, 0.0), math.nextafter(-x, -INF)):
        if v > -1 and v != 0:
            points.append((v, -1.0))
for _ in range(500):
    v = reciprocal_x()
    if v > -1 and v != 0 and not math.isinf(v):
        points.append((v, -1.0))
# The edges of the fast phase: |log1p(x)| = 1/2 and 8, and the (x, y) with y log(1 + x) next to 8 and to log(2) / 2
for lx in (0.5, -0.5, 8.0, -8.0, 0.25, -0.25, 2.0, -2.0, 7.9, -7.9):
    center = math.expm1(lx)
    x = center
    for _ in range(3):
        x = math.nextafter(x, -INF)
    for _ in range(7):
        log = log1p_double(x)
        for t in (0.001, 0.3, 0.3466, 0.35, 1.0, 5.0, 7.9, 8.0, 8.1, -0.3466, -0.35, -2.0, -7.9, -8.0):
            y = t / log
            if y != 0 and not math.isinf(y) and y != 1.0 and y != -1.0:
                points.append((x, y))
        x = math.nextafter(x, INF)
# Random pairs inside the domain of the fast phase, where the value is far from a double nearly always
for _ in range(400):
    x = rng2.choice([rng2.uniform(-0.39, 0.65), rng2.uniform(-0.9, 5.0), 10.0 ** rng2.uniform(-12, 0)])
    if not x > -1 or x == 0:
        continue
    log = log1p_double(x)
    if log == 0:
        continue
    y = rng2.uniform(-8, 8) / log * rng2.choice([1, 1, 0.1, 0.01])
    if y != 0 and not math.isinf(y) and y != 1.0 and y != -1.0:
        points.append((x, y))

# --- The boxes

box_x = [-INF, -3.0, -1.0, -0.999, -0.75, -0.5, -0.1, -2.0 ** -40, 0.0, 2.0 ** -40, 0.1, 0.5, 1.0, 2.0, 7.0,
         100.0, 1e10, INF]
box_y = [-INF, -1000.0, -10.0, -2.5, -1.0, -0.5, -2.0 ** -30, 0.0, 2.0 ** -30, 0.5, 1.0, 1.5, 3.0, 30.0, 700.0,
         INF]
boxes = []
while len(boxes) < 700:
    xl, xu = sorted(rng.sample(box_x, 2)) if rng.random() < 0.9 else (rng.choice(box_x),) * 2
    yl, yu = sorted(rng.sample(box_y, 2)) if rng.random() < 0.9 else (rng.choice(box_y),) * 2
    # [+oo] and [-oo] hold no number: the constructors give the empty set
    if abs(xl) == INF and xl == xu or abs(yl) == INF and yl == yu:
        continue
    boxes.append((xl, xu, yl, yu))
for _ in range(150):
    # bounds that are neither special nor on a grid of powers of two
    xl, xu = sorted([rng.choice([-1.0, 0.0, rng.uniform(-1, 0), rng.uniform(0, 4)]) for _ in range(2)])
    yl, yu = sorted([rng.choice([0.0, rng.uniform(-8, 0), rng.uniform(0, 8)]) for _ in range(2)])
    boxes.append((xl, xu, yl, yu))

# --- The equalities, by fractions


def exact_value(x, y, w):
    """Whether (1 + x)^y is w exactly, for the Fraction w > 0; None if the numbers are too big to say"""
    ratio = Fraction(y)
    p, q = ratio.numerator, ratio.denominator
    if abs(p) > 2000 or q > 4096:
        return None
    s = 1 + Fraction(x)
    if p > 0:
        return s ** p == w ** q
    return (s ** (-p)) * (w ** q) == 1


def dyadic_bits(f):
    """The number of bits of the odd part of the dyadic Fraction f > 0, and the span of its bits"""
    num, den = f.numerator, f.denominator
    low = (num & -num).bit_length() - 1 if num else 0
    return (num >> low).bit_length(), num.bit_length() - low + (den.bit_length() - 1)


def fits(x, y, w):
    """Whether the equality of (1 + x)^y and w can be proved with 128 bits, with room to spare: 1 + x and w
    of less than 100 bits, y = P / 2^k with P below 100 and k at most 6"""
    s = 1 + Fraction(x)
    ratio = Fraction(y)
    p, q = ratio.numerator, ratio.denominator
    if dyadic_bits(s)[1] > 100 or dyadic_bits(w)[1] > 100:
        return False
    if s.numerator & (s.numerator - 1) == 0 and s.denominator & (s.denominator - 1) == 0 and \
            (s.numerator == 1 or s.denominator == 1):
        return abs(p) < 2 ** 12 and q <= 2 ** 10 and True  # a power of two: a y is an integer or not
    return p > 0 and p < 100 and q <= 64


power_triples = []   # (x, y, d, value, expected, complete): value 0 compares with 1 + d, 1 with d


def add_power(x, y, d, value):
    w = (1 + Fraction(d)) if value == 0 else Fraction(d)
    if w <= 0:
        return
    e = exact_value(x, y, w)
    if e is None:
        return
    power_triples.append((x, y, d, value, 1 if e else 0, 1 if (e and fits(x, y, w)) else 0))


for _ in range(900):
    q = rng.choice([1, 1, 2, 2, 4, 8, 16, 32, 64])
    c = Fraction(rng.choice([1, 3, 5, 7, 9, 15, 17, 31, 33, 255, 257])) * Fraction(2) ** rng.randint(-8, 8)
    s = c ** q
    x = s - 1
    if Fraction(float(x)) != x or not x > -1 or x == 0:
        continue
    p = rng.choice([1, 2, 3, 5, 7, 11, -1, -2, -3, 4, 6, 10, 20])
    y = Fraction(p, q) if rng.random() < 0.8 else Fraction(rng.randint(1, 40), q)
    if y == 0 or Fraction(float(y)) != y:
        continue
    xf, yf = float(x), float(y)
    u = (1 + Fraction(xf)) ** Fraction(yf) if Fraction(yf).denominator == 1 else None
    # the value c^p, when it is a double (or a double next to it), and its neighbours
    value = c ** p
    for v, d in ((0, value - 1), (1, value)):
        if v == 1 and d <= 0:
            continue
        if Fraction(float(d)) != d:
            continue
        df = float(d)
        for cand in (df, math.nextafter(df, INF), math.nextafter(df, -INF)):
            if cand != cand or abs(cand) == INF:
                continue
            add_power(xf, yf, cand, v)
    # The odd part right and the power of two wrong: the value times 2^j, and the base times 2^j (j not a
    # multiple of q, so that the 2^q-th root of the base is no integer power of two)
    for j in (-2, -1, 1, 2):
        for v, d in ((0, value * Fraction(2) ** j - 1), (1, value * Fraction(2) ** j)):
            if d > (0 if v == 1 else -1) and Fraction(float(d)) == d:
                add_power(xf, yf, float(d), v)
    for j in (1, -1, 3, -3, 5):
        if j % q == 0:
            continue
        shifted = s * Fraction(2) ** j - 1
        if Fraction(float(shifted)) == shifted and shifted > -1 and shifted != 0:
            for v, d in ((0, value - 1), (1, value)):
                if d > (0 if v == 1 else -1) and Fraction(float(d)) == d:
                    add_power(float(shifted), yf, float(d), v)
# random pairs and random doubles, which are almost all false
for _ in range(300):
    xf = rng.choice([1.0, 3.0, 0.5, 15.0, -0.5, -0.75, 8.0, 24.0, 1.25, 2.0 ** -40]) * rng.choice([1.0, 1.0, 1.5])
    if not xf > -1 or xf == 0:
        continue
    yf = rng.choice([0.5, 2.0, 3.0, -1.0, -2.0, 0.25, 1.5, 0.75, 4.0, 10.0, 1 / 3, 0.1])
    add_power(xf, yf, rng.choice([-0.5, 0.25, 1.0, 2.0, 7.0, 1.25, 2.0 ** -30]) + rng.choice([0.0, 2.0 ** -52]), rng.randint(0, 1))
# the powers of two: 2^(a y) with a y an integer or not
for a in [-60, -53, -10, -3, -2, -1, 1, 2, 3, 10, 53]:
    xf = 2.0 ** a - 1
    if xf <= -1 or xf == 0 or abs(a) > 53:
        continue
    for yf in [1.0, 2.0, 3.0, 0.5, 0.25, 1.5, -1.0, -2.0, -0.5, 10.0, 53.0, 1 / 3, 7.0, -7.0]:
        m = a * Fraction(yf)
        for d in (2.0 ** float(m) - 1 if m.denominator == 1 and abs(m) < 1000 else 0.5,):
            for v in (0, 1):
                dd = d if v == 0 else d + 1
                if dd > 0 or v == 0:
                    for cand in (dd, math.nextafter(dd, INF), math.nextafter(dd, -INF)):
                        add_power(xf, yf, cand, v)

product_triples = []   # (x, y, d, expected)
for _ in range(900):
    mx, my = rng.getrandbits(rng.randint(1, 53)) | 1, rng.getrandbits(rng.randint(1, 53)) | 1
    try:
        x = math.ldexp(float(mx), rng.randint(-1070, 900)) * rng.choice([1, -1])
        y = math.ldexp(float(my), rng.randint(-1070, 900)) * rng.choice([1, -1])
    except OverflowError:
        continue
    if x == 0 or y == 0 or math.isinf(x) or math.isinf(y):
        continue
    f = Fraction(x) * Fraction(y)
    cands = []
    if abs(f) < Fraction(MAX) and f != 0:
        below, above = neighbours(f)
        cands += [below, above]
        if below != above:
            cands += [math.nextafter(below, -INF), math.nextafter(above, INF)]
    cands += [-d for d in cands]   # the opposite sign
    for d in cands:
        if d == 0 or abs(d) == INF:
            continue
        product_triples.append((x, y, d, 1 if f == Fraction(d) else 0))

# The true ones all, and as many false ones, the neighbours of the true ones first among them
def subsample(triples, expected_at, limit):
    true = [t for t in triples if t[expected_at] == 1]
    false = [t for t in triples if t[expected_at] == 0]
    rng.shuffle(false)
    return true[:limit // 2] + false[:limit - min(len(true), limit // 2)]


power_triples = subsample(power_triples, 4, 2400)
product_triples = subsample(product_triples, 3, 1200)

g_values = []   # (R, negative, G): R = |r| 2^128, G = floor((exp(r) - 1) / r 2^127)
for _ in range(500):
    shift = rng.choice([0, 0, 0, 1, 2, 5, 20, 60, 100, 127])
    R = (rng.getrandbits(128) % int(0.36 * 2 ** 128)) >> shift
    if R == 0:
        continue
    negative = rng.getrandbits(1)
    mpmath.mp.prec = 400
    r = mpmath.mpf(R) / mpmath.mpf(2) ** 128 * (-1 if negative else 1)
    G = int(mpmath.floor((mpmath.expm1(r) / r) * mpmath.mpf(2) ** 127))
    g_values.append((R, negative, G))

# --- The reference values of the balls, to 256 bits


def ball_reference(value):
    """(negative, e, M) with |value| = M 2^(e-256), 2^255 <= M < 2^256"""
    sign, man, exp, bits = value._mpf_
    man = int(man)
    if bits >= 256:
        man >>= bits - 256
    else:
        man <<= 256 - bits
    return int(sign), int(exp) + int(bits), man


def ball_literal(x, value):
    negative, e, m = ball_reference(value)
    words = [(m >> (64 * i)) & ((1 << 64) - 1) for i in (3, 2, 1, 0)]
    return "{ %s, %d, %d, %s }" % (literal(x), negative, e, ", ".join("0x%016xULL" % w for w in words))


log_arguments = []
for _ in range(120):
    log_arguments.append(rng.uniform(-1, 5))
    log_arguments.append(-1 + 2.0 ** -rng.uniform(1, 53))
    log_arguments.append(math.ldexp(1.0 + rng.getrandbits(52) / 2.0 ** 52, rng.randint(-1074, -2)) * rng.choice([1, -1]))
    log_arguments.append(10.0 ** rng.uniform(0, 308))
log_arguments += [TINY, -TINY, 2.0 ** -100, -2.0 ** -53, -1 + 2.0 ** -53, 0.5, 1.0, 3.0, MAX, 0.4, -0.29, 0.41, 0.3]
log_arguments = [x for x in log_arguments if x > -1 and x != 0]

exp_arguments = []
for _ in range(100):
    exp_arguments.append(rng.uniform(-64, 746))
    exp_arguments.append(rng.uniform(-1, 1))
    exp_arguments.append(math.ldexp(1.0 + rng.getrandbits(52) / 2.0 ** 52, rng.randint(-1074, -2)) * rng.choice([1, -1]))
exp_arguments += [TINY, -TINY, 1e-300, 2.0 ** -53, 0.3466, -0.3466, 0.3467, 0.6931471805599453, 1.0, -1.0, 10.0,
                  100.0, 709.0, 745.0, 2047.0, -63.9, -36.75, -40.0, 0.5, -0.5, 0.25, 3.0]

# --- The header

print("// Generated by tests/compoundm1_values.py, which says what it is: do not edit")
print("//")
print("// Copyright (c) 2026 ENSTA, France")
print("//")
print("// By: Jordan NININ (created 2026-11-04)")
print("//")
print("// GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated")
print("// COPYING.LIB file for information.")
print("")
print()
print("// The greatest double at most (1 + x)^y - 1 and the least double at least it")
print("struct CompoundPoint { double x, y, below, above; };")
print()
print("const CompoundPoint compound_points[] = {")
kept = skipped = 0
for x, y in points:
    r = compound_bounds(x, y)
    if r is None:
        skipped += 1
        continue
    kept += 1
    print("  { %s }," % ", ".join(literal(v) for v in (x, y) + r))
print("};")
print()
print("// The hull of (1 + x)^y - 1 over [xl, xu] x [yl, yu]: empty (empty = 1), or the bounds")
print("struct CompoundBox { double xl, xu, yl, yu; int empty; double below, above; };")
print()
print("const CompoundBox compound_boxes[] = {")
for box in boxes:
    hull = box_hull(*box)
    if hull is None:
        print("  { %s, 1, 0.0, 0.0 }," % ", ".join(literal(v) for v in box))
    else:
        print("  { %s, 0, %s }," % (", ".join(literal(v) for v in box), ", ".join(literal(v) for v in hull)))
print("};")
print()
print("// A value to 256 bits: the sign, e and M with |value| = M 2^(e-256), 2^255 <= M < 2^256, from the")
print("// most significant 64 bits of M to the least")
print("struct BallValue { double x; int negative; int e; unsigned long long m3, m2, m1, m0; };")
print()
print("// log(1 + x)")
print("const BallValue log1p_values[] = {")
for x in log_arguments:
    mpmath.mp.prec = 600
    print("  %s," % ball_literal(x, mpmath.log1p(mpmath.mpf(x))))
print("};")
print()
print("// exp(x) - 1")
print("const BallValue expm1_values[] = {")
for x in exp_arguments:
    mpmath.mp.prec = 600
    print("  %s," % ball_literal(x, mpmath.expm1(mpmath.mpf(x))))
print("};")
print()
print("// Whether (1 + x)^y is 1 + d (value = 0) or d (value = 1) exactly; complete = 1 where 128 bits")
print("// are enough to prove it, so that the proof has to be found")
print("struct PowerTriple { double x, y, d; int value, expected, complete; };")
print()
print("const PowerTriple power_triples[] = {")
for x, y, d, value, expected, complete in power_triples:
    print("  { %s, %d, %d, %d }," % (", ".join(literal(v) for v in (x, y, d)), value, expected, complete))
print("};")
print()
print("// Whether x y is d exactly")
print("struct ProductTriple { double x, y, d; int expected; };")
print()
print("const ProductTriple product_triples[] = {")
for x, y, d, expected in product_triples:
    print("  { %s, %d }," % (", ".join(literal(v) for v in (x, y, d)), expected))
print("};")
print()
print("// floor(log(2) 2^120), the multiple of log(2) of the fast phase reduces by: 56 high bits, 64 low bits")
mpmath.mp.prec = 400
ln2_120 = int(mpmath.floor(mpmath.log(2) * 2 ** 120))
print("const unsigned long long ln2_120_high = 0x%016xULL, ln2_120_low = 0x%016xULL;" % (ln2_120 >> 64, ln2_120 & (2 ** 64 - 1)))
print()
print("// G = floor((exp(r) - 1) / r 2^127) for r = +-R / 2^128, R = r1 2^64 + r0, G = g1 2^64 + g0")
print("struct GValue { unsigned long long r1, r0; int negative; unsigned long long g1, g0; };")
print()
print("const GValue g_values[] = {")
for R, negative, G in g_values:
    print("  { 0x%016xULL, 0x%016xULL, %d, 0x%016xULL, 0x%016xULL }," % (R >> 64, R & (2 ** 64 - 1), negative, G >> 64, G & (2 ** 64 - 1)))
print("};")
sys.stderr.write("power triples: %d (%d to find), product triples: %d, g: %d\n"
                 % (len(power_triples), sum(t[5] for t in power_triples), len(product_triples), len(g_values)))
sys.stderr.write("points: %d kept, %d left out; boxes: %d; log1p: %d; expm1: %d\n"
                 % (kept, skipped, len(boxes), len(log_arguments), len(exp_arguments)))
