#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of GAOL v5: generates extended_precision_values.h, the values
# tests/extended_precision.cpp compares CORE-MATH and GAOL's bounds with.
#
# For each function f and argument, the table gives the greatest double below
# f(x), the least double above it and the double nearest to it (ties to even),
# which is +inf beyond the largest double and its half ulp. f(x) is computed
# with 5000 bits of precision by mpmath (https://mpmath.org): cospi(2^-1074)
# is 1 - 2^-2145 or so.
#
# The arguments are those at which CORE-MATH, computing its doubles on the x87
# unit of an x86 processor, rounded them wrongly (see
# tests/extended_precision.cpp).
#
#     python3 tests/extended_precision_values.py > tests/extended_precision_values.h
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-21 by Jordan NININ

import math
import struct
import sys

import mpmath

mpmath.mp.prec = 5000

MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)


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
    """The greatest double below v and the least double above it, the same
    double when v is one"""
    if v > MAX:
        return MAX, math.inf
    if v < -MAX:
        return -math.inf, -MAX
    lo = float(v)
    while mpmath.mpf(lo) > v:
        lo = next_down(lo)
    while mpmath.mpf(next_up(lo)) <= v:
        lo = next_up(lo)
    if mpmath.mpf(lo) == v:
        return lo, lo
    return lo, next_up(lo)


def nearest(v, lo, hi):
    """The double nearest to v, ties to the even one"""
    if lo == hi:
        return lo
    if hi == math.inf:
        # Beyond MAX + ulp(MAX)/2 = 2^1024 - 2^970, to +inf
        return math.inf if v >= mpmath.mpf(2) ** 1024 - mpmath.mpf(2) ** 970 else MAX
    if lo == -math.inf:
        return -math.inf if v <= -(mpmath.mpf(2) ** 1024 - mpmath.mpf(2) ** 970) else -MAX
    middle = (mpmath.mpf(lo) + mpmath.mpf(hi)) / 2
    if v < middle:
        return lo
    if v > middle:
        return hi
    even = struct.unpack("<Q", struct.pack("<d", lo))[0] % 2 == 0
    return lo if even else hi


two = mpmath.mpf(2)
ten = mpmath.mpf(10)

FUNCTIONS = {
    "exp": mpmath.exp,
    "exp2": lambda x: two ** x,
    "exp10": lambda x: ten ** x,
    "expm1": mpmath.expm1,
    "exp2m1": lambda x: two ** x - 1,
    "exp10m1": lambda x: ten ** x - 1,
    "log2": lambda x: mpmath.log(x, 2),
    "log10": lambda x: mpmath.log(x, 10),
    "cos": mpmath.cos,
    "tan": mpmath.tan,
    "atan": mpmath.atan,
    "sinh": mpmath.sinh,
    "cosh": mpmath.cosh,
    "tanh": mpmath.tanh,
    "asinh": mpmath.asinh,
    "acosh": mpmath.acosh,
    "cbrt": lambda x: mpmath.sign(x) * mpmath.cbrt(abs(x)),
    "sinpi": lambda x: mpmath.sinpi(x),
    "cospi": lambda x: mpmath.cospi(x),
    "tanpi": lambda x: mpmath.sinpi(x) / mpmath.cospi(x),
    "atan2": mpmath.atan2,
}

h = float.fromhex

# Rounded to nearest, CORE-MATH computing on the x87 unit gave the other
# neighbour of f(x): its result is rounded twice there, to the 64 bits of the
# x87 registers, then to a double, and f(x) was within 2^-64 or so of the
# middle of two doubles. Found on Debian 12 i386 with GCC 12, as on x86-64
# with -mfpmath=387; those of expm1, exp2m1, sinpi, cospi and tanpi on x86-64.
NEAREST = [
    ("exp", h("-0x1.74910d52d3051p+9")), ("exp", h("-0x1.74910d52d304fp+9")),
    ("exp2", h("-0x1.0cbffffffffffp+10")), ("exp2", h("-0x1.0cbfffffffffcp+10")),
    ("exp10", h("-0x1.0024f40ea956dp+5")), ("exp10", h("-0x1.9676a8f8f7257p-1")),
    ("exp2m1", h("0x1.0de65d6192ee7p+0")),
    ("log2", h("0x1.ff14ac6b1df1ep+1013")), ("log10", h("0x1.4f5b4118dbe1bp+0")),
    ("cos", h("-0x1.c5edf099902e4p+4")), ("tan", h("-0x1.deadd0f15d335p+187")),
    ("atan", h("0x1.b1aff86528049p+0")),
    ("sinh", h("0x1.966ce0e262bc8p+0")), ("sinh", h("-0x1.8fd856caa732cp-2")),
    ("cosh", h("0x1.3a234bdf3eb05p-1")), ("cosh", h("-0x1.74a2a194b738ep-3")),
    ("tanh", h("0x1.30fc1931f09c9p+4")), ("tanh", h("0x1.30fc1931f09c6p+4")),
    ("asinh", h("0x1.279dfc4ce17bcp+7")),
    ("acosh", h("0x1.05b07f909b43bp+937")), ("acosh", h("0x1.2792401729230p+10")),
    ("cbrt", h("-0x1.03826f09eeb07p+3")), ("cbrt", h("-0x1.3bdc651e51463p+10")),
    ("sinpi", h("-0x1.559b8c0370720p-443")), ("sinpi", h("-0x1.7e13629b39a0ep+3")),
    ("cospi", h("0x1.46ab83f903022p+8")), ("cospi", h("0x1.17d3b45cf70dap-1")),
    ("tanpi", h("0x1.5deecb6973603p-658")), ("tanpi", h("0x1.2356cc2b555fcp-3")),
]

# Rounded upward, downward or toward zero, CORE-MATH computing on the x87 unit
# gave 0 for a result below the least subnormal (exp2, exp10, atan2), -1 for
# one just above -1 (expm1, exp2m1, exp10m1), 1 for one just below 1 (cospi),
# and +inf for one beyond the largest double (cosh, exp10): it computes them
# as 0x1p-1074 * 0.5, -1.0 + 0x1p-54 or 0x1p1023 + 0x1p1023, which are exact
# in the x87 registers, and GCC 9 turned them into doubles at compile time,
# rounded to nearest. Found on x86-64 with -mfpmath=387.
DIRECTED = [
    ("exp2", h("-0x1.0cc0000000000p+10")), ("exp2", -2000.0), ("exp2", -MAX),
    ("exp10", h("-0x1.747d4d25cd55fp+339")), ("exp10", -400.0),
    ("exp10", h("0x1.34413509f79ffp+8")), ("exp10", MAX),
    ("expm1", h("-0x1.62e42fefa39efp+9")), ("expm1", -800.0),
    ("exp2m1", -1024.0), ("exp2m1", -1100.0),
    ("exp10m1", h("-0x1.34413509f79ffp+8")), ("exp10m1", -400.0),
    ("cosh", h("0x1.633ce8fb9f87ep+9")), ("cosh", MAX), ("cosh", -MAX),
    ("cospi", TINY), ("cospi", -2 * TINY),
    ("atan2", h("0x1.56e1fc2f8f359p-997"), h("0x1.7e43c8800759cp+996")),
    ("atan2", h("0x1.415ac65b4ab1dp-767"), h("0x1.dfb75d0ec2aadp+747")),
    ("atan2", h("-0x1.592f3da6af564p-793"), h("0x1.03a828d452c31p+434")),
]


def literal(x):
    if x == math.inf:
        return "gaol_tests::inf"
    if x == -math.inf:
        return "-gaol_tests::inf"
    return float.hex(x)


print("// Generated by tests/extended_precision_values.py, which says what it is: do not edit")
print("//")
print("// Copyright (c) 2026 ENSTA, France")
print("//")
print("// Created 2026-09-21 by Jordan NININ")
print()
print("struct ExtendedPrecisionValue { const char *function; double a, b, below, above, nearest; };")
print()
for table, cases in (("nearest_values", NEAREST), ("directed_values", DIRECTED)):
    print("const ExtendedPrecisionValue %s[] = {" % table)
    for case in cases:
        name, args = case[0], case[1:]
        v = FUNCTIONS[name](*[mpmath.mpf(a) for a in args])
        lo, hi = neighbours(v)
        a = args[0]
        b = args[1] if len(args) > 1 else 0.0
        print('  {"%s", %s, %s, %s, %s, %s},' % (name, literal(a), literal(b), literal(lo), literal(hi),
                                                 literal(nearest(v, lo, hi))))
    print("};")
    print()
