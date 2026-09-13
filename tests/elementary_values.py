#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of this fork of GAOL: generates elementary_values.h, the values of the
# elementary functions tests/elementary.cpp compares GAOL's bounds with.
#
# For each function f and argument x, a double, the table gives the greatest
# double below f(x) and the least double above it, which are the same double
# when f(x) is one, and are -inf or +inf beyond the largest double. f(x) is
# computed with 2000 bits of precision by mpmath (https://mpmath.org).
#
#     python3 tests/elementary_values.py > tests/elementary_values.h
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------

import math
import random
import struct
import sys

import mpmath

mpmath.mp.prec = 2000

MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)
PI = math.pi
rng = random.Random(20260913)


def next_up(x):
    """The least double above x (math.nextafter(x, math.inf) from Python 3.9)"""
    if x != x or x == math.inf:
        return x
    if x == 0.0:
        return TINY
    bits = struct.unpack("<Q", struct.pack("<d", x))[0]
    bits = bits + 1 if x > 0.0 else bits - 1
    return struct.unpack("<d", struct.pack("<Q", bits))[0]


def next_down(x):
    """The greatest double below x"""
    return -next_up(-x)


def neighbours(v):
    """The greatest double below v and the least double above it"""
    if v > MAX:
        return MAX, math.inf
    if v < -MAX:
        return -math.inf, -MAX
    lo = float(v)
    while mpmath.mpf(lo) > v:
        lo = next_down(lo)
    while mpmath.mpf(next_up(lo)) <= v:
        lo = next_up(lo)
    hi = next_up(lo)
    # f(x) is a double when computed so (exp(0), sqrt(4)...), or within
    # 2^-1800 of one relatively, which only a double is (pow(0.25, 1.5))
    for d in (lo, hi):
        if d == 0.0 and v == 0 or d != 0.0 and abs(v - d) <= mpmath.ldexp(abs(mpmath.mpf(d)), -1800):
            return d, d
    return lo, hi


def random_double(emin, emax, signed=True):
    """A double with a random mantissa and an exponent from emin to emax"""
    x = math.ldexp(1.0 + rng.getrandbits(52) / 2.0**52, rng.randint(emin, emax))
    return -x if signed and rng.getrandbits(1) else x


def uniform(a, b, n):
    return [rng.uniform(a, b) for _ in range(n)]


def exponents(emin, emax, n, signed=True):
    return [random_double(emin, emax, signed) for _ in range(n)]


m = mpmath.mpf
unary = [
    ("exp", mpmath.exp,
     [0.0, 1.0, -1.0, 0.5, 10.0, -20.0, 1e-10, -1e-10, 1e-300, TINY, 700.0, 709.0, 709.78,
      709.782712893384, 709.79, 710.0, -700.0, -708.4, -740.0, -745.1, -800.0]
     + uniform(-700, 700, 25) + exponents(-60, 0, 10)),
    ("log", mpmath.log,
     [1.0, 2.0, 10.0, 0.1, 1.5, 3.0, 0.5, 1.0 + 2.0**-52, 1.0 - 2.0**-53, 2.0**-1022, TINY, MAX,
      1e300, 1e-300]
     + exponents(-1074, 1023, 25, signed=False) + uniform(0.5, 2.0, 10)),
    ("sin", mpmath.sin,
     [0.0, 1.0, 0.5, 3.0, 4.0, 5.0, -2.0, 7.0, 100.0, 355.0, 710.0, 1e-3, PI, PI / 2, 2 * PI, 1e22,
      2.0**60, 1e-300, TINY, 1e300, MAX]
     + uniform(-10, 10, 30) + exponents(10, 80, 10)),
    ("cos", mpmath.cos,
     [0.0, 1.0, 0.5, 3.0, 4.0, 5.0, -2.0, 7.0, 100.0, 355.0, 710.0, 1e-3, PI, PI / 2, 2 * PI, 1e22,
      2.0**60, 1e-300, TINY, 1e300, MAX]
     + uniform(-10, 10, 30) + exponents(10, 80, 10)),
    ("tan", mpmath.tan,
     [0.0, 1.0, -1.0, 0.5, 3.0, -2.0, 7.0, 100.0, 355.0, 1e-3, PI, PI / 2, next_down(PI / 2),
      -PI / 2, 1e22, 2.0**60, 1e-300, TINY, 1e300]
     + uniform(-10, 10, 30) + exponents(10, 80, 10)),
    ("asin", mpmath.asin,
     [0.0, 0.5, -0.5, 1.0, -1.0, 0.1, 1e-10, -1e-10, 1.0 - 2.0**-53, -(1.0 - 2.0**-53), TINY,
      0.7071067811865476]
     + uniform(-1, 1, 25)),
    ("acos", mpmath.acos,
     [0.0, 0.5, -0.5, 1.0, -1.0, 0.1, 1e-10, -1e-10, 1.0 - 2.0**-53, -(1.0 - 2.0**-53), TINY,
      0.7071067811865476]
     + uniform(-1, 1, 25)),
    ("atan", mpmath.atan,
     [0.0, 1.0, 0.5, -2.0, 1000.0, 1e300, -1e300, 1e-300, MAX, TINY, 2.0**53]
     + exponents(-1074, 1023, 20) + uniform(-10, 10, 10)),
    ("sinh", mpmath.sinh,
     [0.0, 1.0, -1.0, 0.5, 10.0, -20.0, 1e-10, 700.0, 710.0, 710.47, 710.5, -711.0, TINY, 1e-300]
     + uniform(-30, 30, 25) + uniform(-700, 700, 10)),
    ("cosh", mpmath.cosh,
     [0.0, 1.0, -1.0, 0.5, 10.0, -20.0, 1e-10, 700.0, 710.0, 710.47, 710.5, -711.0, TINY, 1e-300]
     + uniform(-30, 30, 25) + uniform(-700, 700, 10)),
    ("tanh", mpmath.tanh,
     [0.0, 1.0, -1.0, 0.5, 10.0, 19.0, 20.0, -25.0, 1e-10, TINY, 1e300]
     + uniform(-30, 30, 25)),
    ("asinh", mpmath.asinh,
     [0.0, 1.0, -1.0, 0.5, 1e-10, 1e10, 1e300, MAX, -MAX, TINY]
     + exponents(-60, 60, 20) + exponents(60, 1023, 5)),
    ("acosh", mpmath.acosh,
     [1.0, 1.0 + 2.0**-52, 1.5, 2.0, 10.0, 1e10, 1e300, MAX]
     + [1.0 + x for x in exponents(-60, 0, 10, signed=False)] + exponents(1, 60, 15, signed=False)),
    ("atanh", mpmath.atanh,
     [0.0, 0.5, -0.5, 0.99, 1e-10, 1.0 - 2.0**-53, -(1.0 - 2.0**-53), TINY, 1e-300]
     + uniform(-1, 1, 25)),
    ("sqrt", mpmath.sqrt,
     [0.0, 1.0, 2.0, 3.0, 4.0, 0.25, 0.1, 1e300, MAX, TINY, 2.0**-1022, 2.0**-1074 * 9]
     + exponents(-1074, 1023, 25, signed=False)),
]

binary = [
    # pow(a, b) = a^b, for a > 0
    ("pow", lambda a, b: mpmath.power(a, b),
     [(2.0, 0.5), (4.0, 0.5), (2.0, 10.0), (0.25, 1.5), (10.0, -3.5), (0.5, 3.3), (1.5, 100.1),
      (3.0, 1.0 / 3.0), (7.0, 2.5), (1e-10, 0.3), (1e10, -0.3), (0.99, 1e5), (2.0, 1023.5),
      (0.5, 1100.0), (1.0, 12345.678)]
     + [(rng.uniform(0.0, 100.0), rng.uniform(-10.0, 10.0)) for _ in range(20)]),
    # GAOL does not implement atan2(), which raises unavailable_feature_error
]


def literal(x):
    if x == math.inf:
        return "gaol_tests::inf"
    if x == -math.inf:
        return "-gaol_tests::inf"
    return float.hex(x)


print("// Generated by tests/elementary_values.py, which says what it is: do not edit")
print()
print("struct UnaryValue { const char *function; double x, below, above; };")
print()
print("const UnaryValue unary_values[] = {")
for name, f, xs in unary:
    for x in sorted(set(xs)):
        below, above = neighbours(f(m(x)))
        print('  { "%s", %s, %s, %s },' % (name, literal(x), literal(below), literal(above)))
print("};")
print()
print("struct BinaryValue { const char *function; double a, b, below, above; };")
print()
print("const BinaryValue binary_values[] = {")
for name, f, abs_ in binary:
    for a, b in abs_:
        below, above = neighbours(f(m(a), m(b)))
        print('  { "%s", %s, %s, %s, %s },' % (name, literal(a), literal(b), literal(below), literal(above)))
print("};")
