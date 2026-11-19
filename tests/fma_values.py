#!/usr/bin/env python3
# Tests of GAOL v5: generates fma_values.h, the values tests/core_math.cpp
# compares gaol_fma() (gaol/math/gaol_fma.h) with.
#
# For each triple (a, b, c) the table gives a*b + c rounded once to a double in
# the four rounding directions: to nearest (ties to even), upward, downward
# and toward zero. a*b + c is computed exactly, with Python's rational numbers
# (fractions.Fraction), and rounded by the division of two integers, which
# Python rounds to nearest, ties to even, in the whole range of the doubles
# the subnormal numbers included; the other directions take the neighbour of
# that double when it is on the wrong side of the exact value. A result that
# overflows is the infinity or the largest double by the rule of each
# direction, and the sum of two exact opposites is +0, or -0 rounding downward.
#
# The triples are of the kinds on which a fused multiply-add goes wrong:
#
# - error-free products, c = -a*b rounded to nearest, where the result is the
#   exact error of the product, and the products that a few bits make exact,
#   which cancel to zero;
# - halfway cases of both parities, where the result is a double and half of
#   its last bit, and just above and below them;
# - a product far below c or far above it, which only a sticky bit remembers,
#   at every distance from 1 to 130 bits;
# - cancellations to a few of the last bits, and just beyond;
# - the subnormal range: results that are subnormal numbers, that round to the
#   least one, that underflow to zero;
# - the edge of the overflow: products and sums just below and above the
#   largest double and the half of its last bit;
# - random exponents.
#
#     python3 tests/fma_values.py > tests/fma_values.h
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-18)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

import math
import random
import sys
from fractions import Fraction

MAX = sys.float_info.max
MIN_NORMAL = sys.float_info.min
TINY = math.ldexp(1.0, -1074)

rng = random.Random(20261118)


def nearest(x):
    """The double nearest to the rational x, ties to even; the infinity beyond
    the largest double and its half unit"""
    try:
        return float(x)
    except OverflowError:
        return math.inf if x > 0 else -math.inf


def rounded(x, direction):
    """The exact rational x rounded to a double in 'n' (to nearest), 'u'
    (upward), 'd' (downward) or 'z' (toward zero)"""
    if x == 0:
        return -0.0 if direction == 'd' else 0.0
    r = nearest(x)
    if direction == 'n':
        return r
    if direction == 'z':
        direction = 'd' if x > 0 else 'u'
    if math.isinf(r):
        # beyond the largest double: the infinity of that sign, or the largest
        # double where the direction goes the other way
        if x > 0:
            return math.inf if direction == 'u' else MAX
        return -math.inf if direction == 'd' else -MAX
    fr = Fraction(r)
    if fr == x:
        return r
    if direction == 'u':
        return r if fr > x else math.nextafter(r, math.inf)
    return r if fr < x else math.nextafter(r, -math.inf)


def literal(x):
    if x == math.inf:
        return "gaol_tests::inf"
    if x == -math.inf:
        return "-gaol_tests::inf"
    return float.hex(x)


def random_double(lo_exp, hi_exp, bits=53):
    """A double of exponent in [lo_exp, hi_exp], random sign, and `bits` random
    significant bits (the leading one included)"""
    e = rng.randint(lo_exp, hi_exp)
    m = rng.getrandbits(bits - 1) | (1 << (bits - 1)) if bits > 0 else 1
    x = math.ldexp(m, e - (bits - 1))
    return -x if rng.getrandbits(1) else x


def times(x, k):
    """x, k units of its last place above (toward +infinity)"""
    for _ in range(abs(k)):
        x = math.nextafter(x, math.inf if k > 0 else -math.inf)
    return x


CASES = []


def add(a, b, c):
    for v in (a, b, c):
        assert math.isfinite(v)
    CASES.append((a, b, c))


# Error-free products: c = -(a*b rounded), so that the result is the error of
# the product, computed exactly by a fused multiply-add
for _ in range(24):
    a = random_double(-40, 40)
    b = random_double(-40, 40)
    add(a, b, -(a * b))

# Products that a few bits make exact, and c their opposite: zero, and its
# sign by the direction
for _ in range(6):
    a = random_double(-6, 6, 20)
    b = random_double(-6, 6, 20)
    add(a, b, -(a * b))
    add(-a, b, a * b)

# Halfway cases: c is a double A, a = 1 and b is half a unit of A, so that the
# result is a double and half of its last bit; A of both parities, both signs
# of the half unit, and the products a little above and below the half
for _ in range(24):
    c = random_double(-30, 30)
    ulp = math.ulp(c)
    half = ulp / 2
    sign = rng.choice((1.0, -1.0))
    add(1.0, sign * half, c)
    add(sign * half, 1.0, c)
    add(1.0 + 2.0 ** -52, sign * half, c)           # a little above the half
    add(1.0 - 2.0 ** -53, sign * half, c)           # a little below it
    add(2.0, sign * half / 2, times(c, rng.choice((-1, 1))))

# A product far from c, which leaves only a sticky bit: c large and the
# product tiny, and the converse, at every distance from 1 to 130 bits, so that
# the sticky bit goes from the 73 bits below c to nothing at all
for d in range(1, 131, 3):
    a = random_double(-2, 2)
    b = random_double(-2, 2)
    # |a*b| about 2^0 to 2^4, c about 2^d above it
    c = random_double(d + 1, d + 2)
    add(a, b, c)
    add(a, b, -c)
    # the converse: c far below the product
    a = random_double(d // 2 + 1, d // 2 + 2) if d < 400 else a
    b = random_double(d // 2, d // 2 + 1) if d < 400 else b
    add(a, b, random_double(-2, 2))
    add(a, b, random_double(-2 - d // 4, -1 - d // 4))
# Around the 126 bits of the sum, where the bits of the product that fall below
# the last one kept decide a rounding by their sticky bit alone: the triples the
# direction of which, up or down, only that bit gets right, with c of 2^124 to
# 2^130 and a product of 2^-2 to 2^4
for d in range(112, 140):
    for _ in range(4):
        a = random_double(-3, 2)
        b = random_double(-1, 2)
        c = random_double(d, d + 1)
        add(a, b, c)
        add(a, b, -c)
for d in (140, 200, 400, 600, 1000):
    add(random_double(300, 310), random_double(300, 310), random_double(-d, -d + 5))
    add(random_double(-300, -290), random_double(-300, -290), random_double(d - 600, d - 595))

# Cancellations: c next to -a*b, by up to 3 units of the last place, and
# with a few bits less in the product so that it is exact
for _ in range(20):
    a = random_double(-200, 200)
    b = random_double(-200, 200)
    p = -(a * b)
    for k in (-3, -2, -1, 1, 2, 3):
        if rng.getrandbits(1):
            add(a, b, times(p, k))
for _ in range(4):
    a = random_double(-20, 20, 30)
    b = random_double(-20, 20, 20)
    p = -(a * b)
    for k in (-1, 1):
        add(a, b, times(p, k))

# The subnormal range: products and sums of 2^-1074 to 2^-1022, the results
# that round to the least number, to zero, to the least normal number
for _ in range(10):
    add(random_double(-540, -500), random_double(-560, -520), random_double(-1074, -1040))
    add(random_double(-540, -500), random_double(-560, -520), 0.0)
    sub = math.ldexp(rng.getrandbits(52) | 1, -1074)
    add(rng.choice((1.0, -1.0)) * sub, rng.choice((0.5, 1.5, 2.0, 3.0)), -rng.choice((0.0, 1.0)) * sub / 2)
add(TINY, 0.5, 0.0)
add(TINY, -0.5, 0.0)
add(TINY, 0.5, TINY)
add(TINY, 0.5, -TINY)
add(TINY, 0.75, 0.0)
add(TINY, 0.25, 0.0)
add(MIN_NORMAL, 0.5, 0.0)
add(MIN_NORMAL, 1.0 - 2.0 ** -53, 0.0)
add(MIN_NORMAL, 1.0 - 2.0 ** -53, -MIN_NORMAL)
add(MIN_NORMAL, 1.0 + 2.0 ** -52, -MIN_NORMAL)
add(math.ldexp(1.0, -537), math.ldexp(1.0, -537), 0.0)
add(math.ldexp(1.0, -537), -math.ldexp(1.0, -537), TINY)

# The edge of the overflow: a*b + c next to the largest double and the half of
# its last unit, from below and from above, with c bringing the sum back
for _ in range(8):
    add(random_double(500, 511), random_double(500, 511), -random_double(1000, 1023))
    add(random_double(500, 511), random_double(500, 511), 0.0)
add(MAX, 1.0, 0.0)
add(MAX, 1.0, math.ldexp(1.0, 970))
add(MAX, 1.0, math.ldexp(1.0, 969))
add(MAX, 1.0, -math.ldexp(1.0, 970))
add(MAX, 2.0, -MAX)
add(MAX, 2.0, MAX)
add(-MAX, 2.0, MAX)
add(math.ldexp(1.0, 1023), 2.0, 0.0)
add(math.ldexp(1.0, 1023), 1.0 + 2.0 ** -52, math.ldexp(1.0, 1023))
add(math.ldexp(1.0, 600), math.ldexp(1.0, 423), 0.0)
add(math.ldexp(1.0, 600), math.ldexp(1.0, 423), -MAX)

# Significands of a few bits set or cleared (one and a bit, all ones, a half),
# the product and c at every gap of 140 bits below and above one another: what
# the bits that fall below the 126 kept do to the rounding depends on the
# pattern of both operands at the gap where they begin to fall, and no random
# significand shows it at every gap (found by changing the gap at which
# gaol_fma() keeps only a sticky bit: 128 bits, then 64 and 90, which nothing
# else of this table saw)
SIGNIFICANDS = (1.0, 1.0 + 2.0 ** -52, 2.0 - 2.0 ** -52, 1.5, 1.0 + 2.0 ** -26,
                2.0 - 2.0 ** -26, 1.0 + 2.0 ** -51, 1.75)
for gap in range(-140, 141):
    a = rng.choice(SIGNIFICANDS) * rng.choice((1.0, -1.0))
    b = rng.choice(SIGNIFICANDS) * rng.choice((1.0, -1.0)) * 2.0 ** rng.randint(-3, 3)
    c = rng.choice(SIGNIFICANDS) * rng.choice((1.0, -1.0)) * 2.0 ** (7 + gap)
    add(a, b, c)

# A product whose dropped bits are just under a half of the last bit kept, and c
# the size of what is missing: the sum is then just under, at or just over the
# halfway point, whatever the bits that fall far below it, whose place a sticky
# bit takes: c is 2^t units of the last place of the product, t from 0 to 51,
# with a significand that is one, one and a bit, a bit less than one or one and
# a half (found by moving to 90 bits the limit above which gaol_fma() keeps
# only a sticky bit, which the significands above do not show)
MODULUS = 1 << 53
for t in range(0, 52):
    while True:
        ma = rng.getrandbits(53) | (1 << 52) | 1
        mb = ((MODULUS // 2 - (1 << t)) * pow(ma, -1, MODULUS)) % MODULUS
        if mb >> 52 and ma * mb >= 1 << 105:
            break
    a = ma * 2.0 ** -52
    b = mb * 2.0 ** -52
    # the product is ma*mb * 2^-104: 2^t units of its last place
    extra = (1.0, 1.0 + 2.0 ** -52, 1.0 - 2.0 ** -53, 1.5)[t % 4]
    c = extra * 2.0 ** (t - 104)
    for sign in (1.0, -1.0):
        add(sign * a, b, sign * c)
        add(sign * a, b, -sign * c)

# A product below the least subnormal number, from a half of it to 2^-130 of it,
# alone and added to the least number or its opposite: the result is 0, the
# least number or its opposite by the direction, and the rounding takes the
# branch of a sum below the unit kept (found by moving the limit of 128 bits of
# that branch, which nothing else of this table saw)
for k in range(1, 131):
    a = rng.choice(SIGNIFICANDS) * rng.choice((1.0, -1.0)) * 2.0 ** -k
    add(a, TINY, 0.0)
    if k % 3 == 0:
        add(a, rng.choice((1.0, 3.0)) * TINY, rng.choice((1.0, -1.0)) * TINY)

# Random exponents, all of them: the sum is then the product or c, with a
# sticky bit, and the overflows and the underflows come by themselves
for _ in range(20):
    add(random_double(-1022, 1023), random_double(-1022, 1023), random_double(-1022, 1023))

print("// Generated by tests/fma_values.py, which says what it is: do not edit")
print("//")
print("// Copyright (c) 2026 ENSTA, France")
print("//")
print("// By: Jordan NININ (created 2026-11-18)")
print("//")
print("// GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated")
print("// COPYING.LIB file for information.")
print()
print("// a*b + c rounded once: to nearest, upward, downward and toward zero")
print("struct FmaValue { double a, b, c; double rounded[4]; };")
print()
print("const FmaValue fma_values[] = {")
for a, b, c in CASES:
    x = Fraction(a) * Fraction(b) + Fraction(c)
    results = [rounded(x, d) for d in 'nudz']
    print("  {%s, %s, %s,\n   {%s}}," % (literal(a), literal(b), literal(c), ", ".join(literal(r) for r in results)))
print("};")
