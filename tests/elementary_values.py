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


# The hard-to-round arguments of cos of CORE-MATH
# (https://gitlab.inria.fr/core-math/core-math, src/binary64/cos/cos.wc) at
# which mathlib 2.1.1 returned sin(x), before cmake/mathlib/prepare.cmake fixed
# its multiple-precision cosine, mpcos(), as glibc did in 2003
# (https://sourceware.org/git/?p=glibc.git;a=commit;h=86583139a4d746743ccffcd72e25d96c5fb8d488).
# Reported to mathlib in https://github.com/dreal-deps/mathlib/issues/2.
MPCOS_ARGUMENTS = [float.fromhex(x) for x in """
    0x1.9a25c721c7bfep-1 0x1.9a27a4b746fa2p-1 0x1.9a92cdb25a2e1p-1 0x1.9c3503f763063p-1
    0x1.9c445d0ecfbabp-1 0x1.9cd9b3bb42eeep-1 0x1.9e2eb96bbac15p-1 0x1.9efb0f4c665a3p-1
    0x1.9f1dca15e3c4fp-1 0x1.a037deedfef12p-1 0x1.a068fc11563cbp-1 0x1.a2ccde767b5bep-1
    0x1.a2d818554d9a2p-1 0x1.a2daa4715c263p-1 0x1.a2db798a76d51p-1 0x1.a3610c3b3512ep-1
    0x1.a40ac59889006p-1 0x1.a462b4eca016ap-1 0x1.a48078b5c8f6bp-1 0x1.a4b9b157b1341p-1
    0x1.a5ac57997209cp-1 0x1.a6a427a473eecp-1 0x1.a786eb7b48cf6p-1 0x1.a79c90f186829p-1
    0x1.a856249377deep-1 0x1.a8959110068fbp-1 0x1.a8d0c37d76c3dp-1 0x1.a90e5135be748p-1
    0x1.a96a3ee46bc45p-1 0x1.a9a75acc14c7bp-1 0x1.aa456f9d3af6ap-1 0x1.ab9aa131593adp-1
    0x1.abf74e0d821acp-1 0x1.ac758888bce59p-1 0x1.ad4eede693814p-1 0x1.ae17031f15ce4p-1
    0x1.ae3b990616ccfp-1 0x1.ae7472140f83dp-1 0x1.af4c1166abe16p-1 0x1.af89eeff8224p-1
    0x1.afacecdc00157p-1 0x1.aff1137dbb9f2p-1 0x1.b003f98660f2cp-1 0x1.b08ec50bdce6bp-1
    0x1.b1452182b7e85p-1 0x1.b31cef6342dd8p-1 0x1.b33ae70065978p-1 0x1.b35d8c88afcdp-1
    0x1.b3abae24db453p-1 0x1.b3c84d585eb78p-1 0x1.b3df954783e23p-1 0x1.b4287fe717028p-1
    0x1.b434e9418d78dp-1 0x1.b4b54238060cbp-1
""".split()]

# Arguments of atan at which mathlib 2.1.1 returned values far from atan(x)
# where long has 64 bits, before cmake/mathlib/prepare.cmake fixed fastiroot(),
# which starts its multiple-precision square roots, as glibc did in 2003
# (https://sourceware.org/git/?p=glibc.git;a=commit;h=bb3f4825c411e676c51479fea59643af540810b5):
# the three of Debian bug 210613 (https://bugs.debian.org/210613), on Alpha,
# and 14 of the 12003 hard-to-round arguments of atan of CORE-MATH
# (https://gitlab.inria.fr/core-math/core-math, src/binary64/atan/atan.wc) at
# which it did so on x86_64, spread over them. The 3967 at which atan() had not
# returned after 20 ms are left out, so that the tests fail rather than hang.
MPSQRT_ARGUMENTS = [0.062510113344606447, 1.016527294692847, 1.9966212994203429] + [float.fromhex(x) for x in """
    0x1.93cc5e08a67d8p-7 0x1.6eee0a3c0ab5ep-5 0x1.282a13c03fbfdp-3 0x1.f02aafdb4b606p-2
    0x1.97487d43cc91ep+0 0x1.72825238b729fp+2 0x1.1cf5d00ba6d37p+4 0x1.48e290e0b7cb9p+12
    0x1.bd3853630e373p+20 0x1.45e0b26d83c92p+24 0x1.051a9259b3d12p+28 0x1.c7a2c1d987b90p+37
    0x1.378fb8f1087cfp+41 0x1.49ff16b9c1e3ep+52
""".split()]

# The subnormal hard-to-round arguments of log of CORE-MATH
# (https://gitlab.inria.fr/core-math/core-math, src/binary64/log/log.wc) at
# which mathlib 2.1.1 returned about 2^54, before cmake/mathlib/prepare.cmake
# gave the last, multiple-precision stage of ulog() the argument it had not
# scaled by 2^54. glibc had the same code until it removed that stage in 2018
# (https://sourceware.org/git/?p=glibc.git;a=commit;h=b7c83ca30ef8e85b6642151d95600a36535f8d97).
ULOG_ARGUMENTS = [float.fromhex(x) for x in """
    0x0.8819864d7985dp-1022 0x0.8e26ace5de305p-1022 0x0.a39291c8ef4a7p-1022 0x0.abae673b61d1dp-1022
    0x0.b3974779bda24p-1022 0x0.b3fd99be2faf7p-1022 0x0.b194cc6373ac4p-1022 0x0.c24fffdba64fep-1022
    0x0.e9fcf1c18a54cp-1022 0x0.ee0f1d6e3d717p-1022 0x0.47609196c917bp-1022 0x0.55a71a1f01e0dp-1022
    0x0.57f11408b2353p-1022 0x0.5f94a9574427bp-1022 0x0.61dcf2ed723c4p-1022 0x0.671349642929ep-1022
    0x0.72131e05b2645p-1022 0x0.2483e71c48997p-1022 0x0.29d204655850ap-1022 0x0.1104d89f02ap-1022
    0x0.11b74c68c438ep-1022 0x0.133789cb86cd6p-1022 0x0.1549018654431p-1022 0x0.19b99592dfc98p-1022
    0x0.087b50e3c0a7fp-1022 0x0.00b7751dfaafap-1022
""".split()]


m = mpmath.mpf
unary = [
    ("exp", mpmath.exp,
     [0.0, 1.0, -1.0, 0.5, 10.0, -20.0, 1e-10, -1e-10, 1e-300, TINY, 700.0, 709.0, 709.78,
      709.782712893384, 709.79, 710.0, -700.0, -708.4, -740.0, -745.1, -800.0]
     + uniform(-700, 700, 25) + exponents(-60, 0, 10)),
    ("log", mpmath.log,
     [1.0, 2.0, 10.0, 0.1, 1.5, 3.0, 0.5, 1.0 + 2.0**-52, 1.0 - 2.0**-53, 2.0**-1022, TINY, MAX,
      1e300, 1e-300]
     + exponents(-1074, 1023, 25, signed=False) + uniform(0.5, 2.0, 10) + ULOG_ARGUMENTS),
    ("sin", mpmath.sin,
     [0.0, 1.0, 0.5, 3.0, 4.0, 5.0, -2.0, 7.0, 100.0, 355.0, 710.0, 1e-3, PI, PI / 2, 2 * PI, 1e22,
      2.0**60, 1e-300, TINY, 1e300, MAX]
     + uniform(-10, 10, 30) + exponents(10, 80, 10)),
    ("cos", mpmath.cos,
     [0.0, 1.0, 0.5, 3.0, 4.0, 5.0, -2.0, 7.0, 100.0, 355.0, 710.0, 1e-3, PI, PI / 2, 2 * PI, 1e22,
      2.0**60, 1e-300, TINY, 1e300, MAX]
     + uniform(-10, 10, 30) + exponents(10, 80, 10) + MPCOS_ARGUMENTS),
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
     + exponents(-1074, 1023, 20) + uniform(-10, 10, 10) + MPSQRT_ARGUMENTS),
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
