#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of GAOL v5: generates reverse_values.h, the cases of the
# reverse functions of IEEE 1788-2015 (10.5.4, Table 10.1) tests/reverse.cpp
# checks GAOL's relational functions with.
#
# IEEE 1788 defines, x being the whole line when not given,
#     fRev(c, x) = hull{ x in x | f(x) is defined and in c },
#     mulRev(b, c, x) = hull{ x in x | x*b is in c for some b in b },
# and asks of them (12.10.2) to be valid, to enclose the hull, and, for
# inf-sup types, to be accurate:
#     fRev(c, x) is within nextOut(tightest(nextOut(c), nextOut(x))),
# the tightest interval of doubles enclosing the hull for arguments widened by
# one double, widened by one double; and to be empty when an argument is.
# For each case, this script computes from these definitions the tightest
# enclosure of the hull, and that bound of the accurate results:
#  - sqrRev, absRev, pownRev (p > 0) and coshRev: the preimage of c, one or
#    two intervals bounded by roots (Fractions where they are doubles, which
#    rational arithmetic checks, irrational otherwise) or acosh, intersected
#    with x;
#  - sinRev, cosRev, tanRev: the first and the last point of the preimage in
#    x, from the pieces asin(c) + 2k pi, pi - asin(c) + 2k pi and the like next
#    to the bounds of x, with mpmath at 2000 bits;
#  - mulRev: the union of the c/b over the negative and the positive b of b,
#    in rational arithmetic, a bound that no b reaches (0, when b reaches 0 or
#    +-oo) being open, intersected with x.
#
# The cases:
#  - those of the "minimal" tests of libieeep1788
#    (https://github.com/nehmeier/libieeep1788, Marco Nehmeier, Apache License
#    2.0), which became the reverse functions of the ITF1788 test suite: the
#    empty set, the whole line, infinite and signed zero bounds, the edges of
#    the domains, the periods, divisors containing 0, with and without x.
#    Their results in libieeep1788 are checked to enclose the tightest ones:
#    all do, and a few are wider (cosRev([-1], [3.14, 3.1406]) is one double
#    wider than [pi] on each side). Left out: the decorated and mixed types,
#    the invalid arguments, and pownRev for p <= 0, which GAOL does not
#    provide;
#  - random cases of each function, whose bounds are of many magnitudes, zero
#    or infinite, with and without x.
#
#     python3 tests/reverse_values.py \
#       <libieeep1788>/test/p1788/flavor/infsup/setbased/test_mpfr_bin_ieee754_flavor_rev_func.cpp \
#       > tests/reverse_values.h
#
# (libieeep1788 at commit 1f10b89, which doc/compare/code/setup.sh clones.)
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
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
EMPTY, ENTIRE = "empty", "entire"


#---------------------------------------------------------------------------
# Doubles
#---------------------------------------------------------------------------
def next_up(x):
    """The least double above x (math.nextafter(x, inf) of Python 3.9)"""
    if x != x or x == INF:
        return x
    if x == 0.0:
        return math.ldexp(1.0, -1074)
    n = struct.unpack("<q", struct.pack("<d", x))[0]
    n = n + 1 if x > 0 else n - 1
    return struct.unpack("<d", struct.pack("<q", n))[0]


def next_down(x):
    return -next_up(-x)


def to_mpf(v):
    """A real of this script (+-inf, a double, a Fraction or an mpf) as an mpf"""
    if isinstance(v, Fraction):
        return mpmath.mpf(v.numerator) / v.denominator
    if isinstance(v, float):
        return mpmath.mpf(v) if abs(v) != INF else (mpmath.inf if v > 0 else -mpmath.inf)
    return v


def less(a, b):
    """a < b, for reals of this script: Fractions compare exactly with doubles,
    and the mpfs this script compares are irrational, or exact"""
    if isinstance(a, (Fraction, float)) and isinstance(b, (Fraction, float)):
        return a < b
    return to_mpf(a) < to_mpf(b)


def round_down(v):
    """The greatest double at most v"""
    if isinstance(v, float):
        return v
    if isinstance(v, Fraction):
        if v > Fraction(MAX):
            return MAX
        if v < -Fraction(MAX):
            return -INF
        d = float(v)  # the double nearest v: Python divides integers correctly rounded
        return next_down(d) if Fraction(d) > v else d
    if v == mpmath.inf or v == -mpmath.inf:
        return INF if v > 0 else -INF
    d = float(v)
    if abs(d) == INF:
        return MAX if d > 0 else -INF
    return next_down(d) if mpmath.mpf(d) > v else d


def round_up(v):
    if isinstance(v, float):
        return v
    return -round_down(-v)


def next_out(a):
    """nextOut of IEEE 1788 (12.10.1): an interval widened by one double"""
    if a == EMPTY:
        return EMPTY
    l, u = a
    return (next_down(l), next_up(u))


def root(v, p):
    """The real p-th root of a double v, p > 0 (v >= 0 for an even p): a Fraction
    when it is a double, an mpf otherwise (it is then irrational)"""
    if abs(v) == INF:
        return v
    if v == 0:
        return Fraction(0)
    s = -1 if v < 0 else 1
    r = mpmath.root(mpmath.mpf(abs(v)), p)
    d = float(r)
    if Fraction(d) ** p == Fraction(abs(v)):
        return s * Fraction(d)
    return s * r


#---------------------------------------------------------------------------
# The functions whose preimage of c is a list of closed intervals
#---------------------------------------------------------------------------
def hull_within(pieces, x):
    """The tightest enclosure of the hull of the pieces, closed intervals of
    reals, within x, a pair of doubles"""
    lo, hi = None, None
    for a, b in pieces:
        l = a if less(x[0], a) else x[0]
        u = b if less(b, x[1]) else x[1]
        if less(u, l):
            continue
        lo = l if lo is None or less(l, lo) else lo
        hi = u if hi is None or less(hi, u) else hi
    return EMPTY if lo is None else (round_down(lo), round_up(hi))


def symmetric(lo, hi):
    """[-hi, -lo] and [lo, hi]"""
    return [(-hi, -lo), (lo, hi)]


def sqr_rev(c, x):
    l, u = max(c[0], 0.0), c[1]
    return EMPTY if l > u else hull_within(symmetric(root(l, 2), root(u, 2)), x)


def abs_rev(c, x):
    l, u = max(c[0], 0.0), c[1]
    return EMPTY if l > u else hull_within(symmetric(Fraction(l), u if u == INF else Fraction(u)), x)


def pown_rev(c, x, p):
    if p % 2 == 1:
        return hull_within([(root(c[0], p), root(c[1], p))], x)
    l, u = max(c[0], 0.0), c[1]
    return EMPTY if l > u else hull_within(symmetric(root(l, p), root(u, p)), x)


def cosh_rev(c, x):
    l, u = max(c[0], 1.0), c[1]
    if l > u:
        return EMPTY
    lo = Fraction(0) if l == 1.0 else mpmath.acosh(mpmath.mpf(l))
    hi = INF if u == INF else (Fraction(0) if u == 1.0 else mpmath.acosh(mpmath.mpf(u)))
    return hull_within(symmetric(lo, hi), x)


#---------------------------------------------------------------------------
# The periodic functions
#---------------------------------------------------------------------------
def periodic_rev(x, in_preimage, starts, ends, period):
    """The tightest enclosure of the x of x in the preimage, the union over k
    of the intervals [s + k period, e + k period], s from starts and e from ends
    (irrational, but for 0): its first point from the lower bound of x, its
    last one from the upper bound"""
    xl, xh = x
    if xl == -INF:
        lo = -INF
    elif in_preimage(xl):
        lo = xl
    else:
        k = mpmath.floor((mpmath.mpf(xl) - max(starts)) / period)
        lo = min(v for v in (s + (k + j) * period for s in starts for j in range(-1, 4)) if v > mpmath.mpf(xl))
    if xh == INF:
        hi = INF
    elif in_preimage(xh):
        hi = xh
    else:
        k = mpmath.floor((mpmath.mpf(xh) - min(ends)) / period)
        hi = max(v for v in (e + (k + j) * period for e in ends for j in range(-3, 2)) if v < mpmath.mpf(xh))
    if less(xh, lo) or less(hi, xl):
        return EMPTY
    return (round_down(lo), round_up(hi))


def sin_rev(c, x):
    l, u = max(c[0], -1.0), min(c[1], 1.0)
    if l > u:
        return EMPTY
    a1, a2 = mpmath.asin(l), mpmath.asin(u)
    return periodic_rev(x, lambda t: l <= mpmath.sin(mpmath.mpf(t)) <= u,
                        [a1, mpmath.pi - a2], [a2, mpmath.pi - a1], 2 * mpmath.pi)


def cos_rev(c, x):
    l, u = max(c[0], -1.0), min(c[1], 1.0)
    if l > u:
        return EMPTY
    a1, a2 = mpmath.acos(u), mpmath.acos(l)
    return periodic_rev(x, lambda t: l <= mpmath.cos(mpmath.mpf(t)) <= u, [a1, -a2], [a2, -a1], 2 * mpmath.pi)


def tan_rev(c, x):
    l, u = c
    a1 = -mpmath.pi / 2 if l == -INF else mpmath.atan(l)
    a2 = mpmath.pi / 2 if u == INF else mpmath.atan(u)
    return periodic_rev(x, lambda t: to_mpf(l) <= mpmath.tan(mpmath.mpf(t)) <= to_mpf(u), [a1], [a2], mpmath.pi)


#---------------------------------------------------------------------------
# mulRev, in rational arithmetic, with the bounds no b reaches left open
#---------------------------------------------------------------------------
def quotient(a, b):
    """a/b for b > 0 finite, or b = +oo with a finite (0, as a limit)"""
    if b == INF:
        return Fraction(0)
    if abs(a) == INF:
        return a
    return a / b


def over_positive(c, b0, b0_reached, b1):
    """{ t | t*b in c for some b, b0 < b <= b1 or b0 <= b <= b1 (b0_reached) },
    0 <= b0 <= b1, b1 reached when finite: the union of the c/b, an interval,
    as (lo, lo_reached, hi, hi_reached)"""
    cl, ch = c
    if cl == -INF:
        lo, lo_r = -INF, False
    elif cl < 0:  # the least of cl/b, at the least b
        lo, lo_r = (-INF, False) if b0 == 0 else (quotient(cl, b0), b0_reached)
    elif cl == 0:
        lo, lo_r = Fraction(0), True
    else:  # at the greatest b
        lo, lo_r = quotient(cl, b1), b1 != INF
    if ch == INF:
        hi, hi_r = INF, False
    elif ch > 0:
        hi, hi_r = (INF, False) if b0 == 0 else (quotient(ch, b0), b0_reached)
    elif ch == 0:
        hi, hi_r = Fraction(0), True
    else:
        hi, hi_r = quotient(ch, b1), b1 != INF
    return lo, lo_r, hi, hi_r


def frac(v):
    return v if abs(v) == INF else Fraction(v)


def mul_rev(b, c, x):
    bl, bh = frac(b[0]), frac(b[1])
    c = (frac(c[0]), frac(c[1]))
    xl, xh = frac(x[0]), frac(x[1])
    if bl <= 0 <= bh and c[0] <= 0 <= c[1]:
        return x  # t*0 = 0 is in c, for every t
    pieces = []
    if bh > 0:  # the b > 0 of b
        pieces.append(over_positive(c, max(bl, Fraction(0)), bl > 0, bh))
    if bl < 0:  # b = -g, g > 0: t*b in c is (-t)*g in c
        lo, lo_r, hi, hi_r = over_positive(c, max(-bh, Fraction(0)), bh < 0, -bl)
        pieces.append((-hi, hi_r, -lo, lo_r))
    lo_all, hi_all = None, None
    for lo, lo_r, hi, hi_r in pieces:
        # Intersected with [xl, xh]
        if less(xl, lo) or (xl == lo and not lo_r):
            l, l_r = lo, lo_r
        else:
            l, l_r = xl, True
        if less(hi, xh) or (xh == hi and not hi_r):
            u, u_r = hi, hi_r
        else:
            u, u_r = xh, True
        if less(u, l) or (l == u and not (l_r and u_r)):
            continue
        lo_all = l if lo_all is None or less(l, lo_all) else lo_all
        hi_all = u if hi_all is None or less(hi_all, u) else hi_all
    return EMPTY if lo_all is None else (round_down(lo_all), round_up(hi_all))


#---------------------------------------------------------------------------
# The tightest and the accurate results of a case
#---------------------------------------------------------------------------
def whole(a):
    return (-INF, INF) if a == ENTIRE else a


def tightest(f, c, b, x, p):
    if EMPTY in (c, b, x):
        return EMPTY
    c, b, x = whole(c), whole(b), whole(x)
    functions = {"sqrRev": lambda: sqr_rev(c, x), "absRev": lambda: abs_rev(c, x),
                 "pownRev": lambda: pown_rev(c, x, p), "coshRev": lambda: cosh_rev(c, x),
                 "sinRev": lambda: sin_rev(c, x), "cosRev": lambda: cos_rev(c, x),
                 "tanRev": lambda: tan_rev(c, x), "mulRev": lambda: mul_rev(b, c, x)}
    return functions[f]()


def accurate(f, c, b, x, p):
    widen = lambda a: next_out(whole(a))
    return next_out(tightest(f, widen(c), widen(b), widen(x), p))


def encloses(a, t):
    if t == EMPTY:
        return True
    if a == EMPTY:
        return False
    a = whole(a)
    return a[0] <= t[0] and t[1] <= a[1]


#---------------------------------------------------------------------------
# The cases of libieeep1788
#---------------------------------------------------------------------------
BLOCKS = {
    "minimal_sqr_rev_test": "sqrRev", "minimal_sqr_rev_bin_test": "sqrRev",
    "minimal_abs_rev_test": "absRev", "minimal_abs_rev_bin_test": "absRev",
    "minimal_pown_rev_test": "pownRev", "minimal_pown_rev_bin_test": "pownRev",
    "minimal_sin_rev_test": "sinRev", "minimal_sin_rev_bin_test": "sinRev",
    "minimal_cos_rev_test": "cosRev", "minimal_cos_rev_bin_test": "cosRev",
    "minimal_tan_rev_test": "tanRev", "minimal_tan_rev_bin_test": "tanRev",
    "minimal_cosh_rev_test": "coshRev", "minimal_cosh_rev_bin_test": "coshRev",
    "minimal_mul_rev_test": "mulRev", "minimal_mul_rev_ten_test": "mulRev",
}
CONSTANTS = {"INF_D": INF, "MAX_D": MAX, "MIN_D": sys.float_info.min, "DNORM_MIN_D": math.ldexp(1.0, -1074)}


class Parser:
    """The arguments of a call of libieeep1788's tests: intervals
    (REP<double>(l, u), F<double>::empty(), F<double>::entire()) and numbers"""

    def __init__(self, text):
        self.s = re.sub(r"\s+", "", text)
        self.i = 0

    def eat(self, token):
        if self.s.startswith(token, self.i):
            self.i += len(token)
            return True
        return False

    def expect(self, token):
        if not self.eat(token):
            raise ValueError("expected %s at %r" % (token, self.s[self.i:self.i + 40]))

    def number(self):
        sign = -1.0 if self.eat("-") else 1.0
        if self.eat('std::stod("'):
            j = self.s.index('"', self.i)
            # As std::stod, the longest prefix that is a number: two cases of
            # tanRev give "0X1.D02967C31+53", without the p of the exponent,
            # which is then 0X1.D02967C31, the value their results are for
            v = float.fromhex(re.match(r"[-+]?0x[0-9a-f]*\.?[0-9a-f]*(p[-+]?[0-9]+)?", self.s[self.i:j], re.I).group(0))
            self.i = j
            self.expect('")')
            return sign * v
        for name, v in CONSTANTS.items():
            if self.eat(name):
                return sign * v
        m = re.match(r"[0-9.]+(e[-+]?[0-9]+)?", self.s[self.i:], re.I)
        if not m:
            raise ValueError("no number at %r" % self.s[self.i:self.i + 40])
        self.i += m.end()
        return sign * float(m.group(0))  # the double nearest the decimal, as the C++ compiler reads it

    def argument(self):
        if self.eat("F<double>::empty()"):
            return EMPTY
        if self.eat("F<double>::entire()"):
            return ENTIRE
        if self.eat("REP<double>("):
            l = self.number()
            self.expect(",")
            u = self.number()
            self.expect(")")
            return (l, u)
        return self.number()

    def arguments(self):
        args = [self.argument()]
        while self.eat(","):
            args.append(self.argument())
        return args


def valid(a):
    """Whether a is a number, or an interval of IEEE 1788 (10.2)"""
    if not isinstance(a, tuple):
        return True
    l, u = a
    return l <= u and l < INF and u > -INF


def statements(body):
    """The BOOST_CHECK... statements of a test case, each on one line"""
    out = []
    i = body.find("BOOST_CHECK")
    while i >= 0:
        depth, j = 0, body.index("(", i)
        while True:
            if body[j] == "(":
                depth += 1
            elif body[j] == ")":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        out.append(re.sub(r"\s+", " ", body[i:j + 1]) + ";")
        i = body.find("BOOST_CHECK", j)
    return out


def split_call(s):
    """'call(...), expected);' -> ('call(...)', 'expected')"""
    depth = 0
    for i, ch in enumerate(s):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == "," and depth == 0:
            return s[:i].strip(), s[i + 1:].strip()[:-2].strip()
    raise ValueError("no top-level comma in " + s)


def libieeep1788_cases(path):
    """(f, c, b, x, p, x_given, result of libieeep1788)"""
    text = open(path).read()
    out = []
    call = r"F<double>::(\w+)\((.*)\)"
    for m in re.finditer(r"BOOST_AUTO_TEST_CASE\((\w+)\)\s*\{(.*?)\n\}", text, re.S):
        name, body = m.group(1), m.group(2)
        if name not in BLOCKS:
            continue
        f = BLOCKS[name]
        for line in statements(body):
            # The cases of invalid arguments follow the first use of p1788::exception
            if "p1788::exception" in line:
                break
            empty = re.match(r"BOOST_CHECK\(\s*F<double>::is_empty\(\s*" + call + r"\s*\)\s*\);$", line)
            if empty:
                args, expected = Parser(empty.group(2)).arguments(), EMPTY
            elif line.startswith("BOOST_CHECK_EQUAL("):
                inner, rest = split_call(line[len("BOOST_CHECK_EQUAL("):])
                args, expected = Parser(re.match(call, inner).group(2)).arguments(), Parser(rest).argument()
            else:
                raise ValueError("line not understood: " + line)
            if not all(valid(a) for a in args + [expected]):
                continue  # an interval with bounds in the wrong order: GAOL cannot build it
            p = 0
            if f == "pownRev":
                p = int(args.pop())
                if p <= 0:
                    continue  # pownRev for p <= 0: not provided by GAOL
            if f == "mulRev":
                b, c = args[0], args[1]
                x = args[2] if len(args) == 3 else ENTIRE
                out.append((f, c, b, x, p, len(args) == 3, expected))
            else:
                c = args[0]
                x = args[1] if len(args) == 2 else ENTIRE
                out.append((f, c, ENTIRE, x, p, len(args) == 2, expected))
    return out


#---------------------------------------------------------------------------
# Random cases
#---------------------------------------------------------------------------
rng = random.Random(1788)


def random_number(lo_exp, hi_exp, special):
    """A double of magnitude 2^u, u from lo_exp to hi_exp, of either sign, or
    now and then 0, +-1 or an infinity"""
    r = rng.random()
    if r < special / 3:
        return rng.choice([0.0, -0.0, 1.0, -1.0])
    if r < special:
        return rng.choice([INF, -INF])
    return rng.choice([-1.0, 1.0]) * math.ldexp(rng.uniform(1.0, 2.0), rng.randint(lo_exp, hi_exp))


def random_interval(lo_exp, hi_exp, special=0.15):
    a, b = sorted([random_number(lo_exp, hi_exp, special), random_number(lo_exp, hi_exp, special)])
    return (-1.0, 1.0) if a == INF or b == -INF else (a, b)


def random_cases(n):
    out = []
    for f in ["sqrRev", "absRev", "pownRev", "coshRev", "mulRev"]:
        for _ in range(n):
            c, x = random_interval(-20, 20), random_interval(-20, 20)
            b = random_interval(-20, 20) if f == "mulRev" else ENTIRE
            p = rng.randint(1, 7) if f == "pownRev" else 0
            x_given = rng.random() < 0.7
            out.append((f, c, b, x if x_given else ENTIRE, p, x_given))
    for f in ["sinRev", "cosRev", "tanRev"]:
        for _ in range(n):
            if f == "tanRev":
                c = random_interval(-10, 10)
            else:
                c = tuple(sorted([rng.uniform(-1.5, 1.5), rng.uniform(-1.5, 1.5)]))
            # x about 2^e, e from -5 to 60, of width 2^w, or any
            centre = rng.choice([-1.0, 1.0]) * math.ldexp(rng.uniform(1.0, 2.0), rng.randint(-5, 60))
            width = math.ldexp(1.0, rng.randint(-40, 4))
            x = (centre - width, centre + width) if rng.random() < 0.85 else random_interval(-5, 60, 0.3)
            out.append((f, c, ENTIRE, x, 0, True))
    return out


#---------------------------------------------------------------------------
# The header
#---------------------------------------------------------------------------
def double(x):
    if x == INF:
        return "inf"
    if x == -INF:
        return "-inf"
    return x.hex() if x != 0.0 else ("-0x0.0p+0" if math.copysign(1.0, x) < 0 else "0x0.0p+0")


def interval(a):
    if a == EMPTY:
        return "E"
    if a == ENTIRE:
        return "W"
    return "I(%s, %s)" % (double(a[0]), double(a[1]))


def main():
    table, wider = [], 0
    for f, c, b, x, p, x_given, expected in libieeep1788_cases(sys.argv[1]):
        t = tightest(f, c, b, x, p)
        if not encloses(expected, t):
            raise ValueError("libieeep1788's %s(c=%r, b=%r, x=%r, p=%d) = %r does not enclose %r"
                             % (f, c, b, x, p, expected, t))
        if whole(expected) != t:
            wider += 1
            print("libieeep1788 wider than the tightest: %s(c=%r, b=%r, x=%r, p=%d) = %r, tightest %r"
                  % (f, c, b, x, p, expected, t), file=sys.stderr)
        table.append((f, c, b, x, p, x_given, t, accurate(f, c, b, x, p), "libieeep1788"))
    n_libieeep1788 = len(table)
    for f, c, b, x, p, x_given in random_cases(150):
        table.append((f, c, b, x, p, x_given, tightest(f, c, b, x, p), accurate(f, c, b, x, p), "random"))

    print("// Generated by tests/reverse_values.py: the reverse functions of IEEE 1788-2015")
    print("// on doubles, in the cases of the minimal tests of libieeep1788 (Apache License")
    print("// 2.0) and in random ones, with the tightest enclosure of the hull the standard")
    print("// defines and the bound of the accurate results. Do not edit.")
    print("//")
    print("// Copyright (c) 2026 ENSTA, France")
    print("//")
    print("// Created 2026-09-20 by Jordan NININ")
    print("#ifndef GAOL_TESTS_REVERSE_VALUES_H")
    print("#define GAOL_TESTS_REVERSE_VALUES_H")
    print("namespace reverse_values")
    print("{")
    print("  const double inf = gaol_tests::inf;")
    print("  // An interval of the table: E the empty set, W the whole line")
    print("  struct I { double l, u; bool empty; I(double a, double b) : l(a), u(b), empty(false) {} "
          "I(bool e) : l(0.0), u(0.0), empty(e) {} };")
    print("  const I E(true), W(-inf, inf);")
    print("  // f, the IEEE 1788 function: fRev(c, x), mulRev(b, c, x), pownRev(c, x, p);")
    print("  // b is W for the functions other than mulRev, and x when the case does not")
    print("  // give it (x_given false); tightest, the tightest interval enclosing the hull;")
    print("  // accurate, nextOut(tightest(nextOut(c), nextOut(b), nextOut(x))), which the")
    print("  // accurate results are within (12.10.1); from, libieeep1788 or random")
    print("  struct Case { const char* f; I c, b, x; int p; bool x_given; I tightest, accurate; const char* from; };")
    print("  const Case cases[] = {")
    for f, c, b, x, p, x_given, t, a, source in table:
        print("    { \"%s\", %s, %s, %s, %d, %s, %s, %s, \"%s\" }," % (
            f, interval(c), interval(b), interval(x), p, "true" if x_given else "false", interval(t), interval(a), source))
    print("  };")
    print("}")
    print("#endif // GAOL_TESTS_REVERSE_VALUES_H")
    counts = {}
    for row in table:
        counts[row[0]] = counts.get(row[0], 0) + 1
    print("reverse_values.py: %d cases of libieeep1788 (%d of whose results are wider than the tightest), "
          "%d random; %s" % (n_libieeep1788, wider, len(table) - n_libieeep1788,
                             ", ".join("%s %d" % kv for kv in sorted(counts.items()))), file=sys.stderr)


if __name__ == "__main__":
    main()
