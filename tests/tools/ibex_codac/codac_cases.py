#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of GAOL v5: the cases of the interval tests of Codac, with their
# exact results, and the rows of the tables of the C++ section `codac.inc`
# of tests/codac_ibex.cpp.
#
# The unit tests of Codac 2 on its scalar Interval (commit 6419940 of the
# master, and the branch GAOL of the fork of Jordan Ninin, commit f0d06ff0):
#
#     tests/core/domains/interval/codac2_tests_Interval.cpp
#     tests/core/domains/interval/codac2_tests_Interval_operations.cpp
#     tests/core/domains/interval/codac2_tests_Interval_rounding.cpp
#
# are written here, case by case, with what the original expects. For each
# case the tool computes the EXACT result of the operation on the exact inputs
# (the doubles the test writes), apart from GAOL: with the rational numbers of
# Python for + - * / sqr sqrt of squares, the powers and the integer
# functions, and with mpmath on 2000 bits for the elementary functions. The
# reference of a result is the pair (lo, hi): lo the greatest double not above
# the exact lower bound, hi the least double not below the exact upper bound
# (-inf and +inf beyond the doubles), or `none` for the empty set. The C++ rows
# hold these pairs, which codac.inc compares GAOL's bounds with.
#
# The original expected values are a cross-check, not the reference: each is
# compared with the exact result, with the tolerance the original test uses
# (equal to the tightest bounds for an `==`, within the eps of an Approx), and
# every disagreement is printed ("DISAGREEMENT"), a wrong expectation of the
# original or a bug of GAOL.
#
# Where the original gives GAOL an argument computed with the interval pi
# (sin(pi - x), tan(3 pi / 4 | 5 pi / 4)...), the reference is the exact hull of
# the function over the interval of doubles that argument is, computed here by
# the exact operations rounded outward (pi being the two doubles around it, as
# GAOL's interval::pi() is): the tightest result the function can have on that
# argument. The exact hull over the real argument is computed too, to check
# the original's expectation and that the reference encloses it.
#
#     python3 -I codac_cases.py --check codac.inc     # the rows of codac.inc are those the cases give
#     python3 -I codac_cases.py --write codac.inc     # rewrites the rows between the BEGIN and END marks
#     python3 -I codac_cases.py --list                # the cases and their references, one per line
#     python3 -I codac_cases.py --selftest            # the exact oracle against brute force
#     python3 -I codac_cases.py --elementary-values tests/elementary_values.h
#                                                     # the points of the rounding test of Codac that
#                                                     # GAOL's tables already hold
#
# The exit status is 1 when a row differs (--check), or a self-test fails.
# Python 3.9 or later, and mpmath (https://mpmath.org).
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-11-02 by Jordan NININ

import argparse
import math
import re
import struct
import sys
from fractions import Fraction

import mpmath

mpmath.mp.prec = 2000

INF = math.inf
MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)
FMAX = Fraction(MAX)
BEYOND = Fraction(2)**2000      # stands for a value far beyond the doubles
SMALL = Fraction(1, 2**1200)    # stands for a value far below the subnormals
E = None                        # the empty set


#---------------------------------------------------------------------------
# Numbers: a bound is a Fraction, or -inf or +inf (floats)
#---------------------------------------------------------------------------

def is_inf(b):
    return isinstance(b, float) and math.isinf(b)


def mpf_of(b):
    """An exact Fraction (a double, or an mpf already) as an mpf of 2000 bits"""
    if isinstance(b, mpmath.mpf):
        return b
    return mpmath.mpf(b.numerator) / mpmath.mpf(b.denominator)


def frac_of(m):
    """An mpf as a Fraction; BEYOND and SMALL stand for what is far beyond or far below the doubles"""
    if m == 0:
        return Fraction(0)
    g = mpmath.mag(m)
    if g > 1100:
        return BEYOND if m > 0 else -BEYOND
    if g < -1200:
        return SMALL if m > 0 else -SMALL
    sign, man, exp, _ = m._mpf_            # m = (-1)^sign man 2^exp (man_exp has no sign)
    v = Fraction(int(man)) * Fraction(2)**exp
    return -v if sign else v


def snap(v):
    """v, or the double it is within 2^-1500 of relatively: an exact result that 2000 bits computed inexactly"""
    if v == 0 or abs(v) >= BEYOND / 2 or abs(v) <= SMALL * 2:
        return v
    try:
        d = Fraction(float(v))
    except OverflowError:
        return v
    if d != 0 and abs(v - d) <= abs(v) * Fraction(1, 2**1500):
        return d
    return v


def ev(fn, b):
    """fn(b) computed with 2000 bits, as a Fraction (an infinite bound is the caller's)"""
    return snap(frac_of(fn(mpf_of(b))))


def rd(v):
    """The greatest double not above v: a Fraction, or an infinity"""
    if is_inf(v):
        return v
    if v >= FMAX:
        return MAX
    if v < -FMAX:
        return -INF
    d = float(v)
    if Fraction(d) > v:
        d = math.nextafter(d, -INF)
    return d + 0.0


def ru(v):
    """The least double not below v"""
    if is_inf(v):
        return v
    return -rd(-v) + 0.0


def ex(x):
    """A double (or None for the empty set), as an exact bound"""
    return x if is_inf(x) else Fraction(x)


def exi(X):
    """An interval of doubles (a pair of floats, or E) as an exact interval"""
    return None if X is None else (ex(X[0]), ex(X[1]))


def ref_of(A):
    """The reference of an exact interval: the tightest doubles around it"""
    return None if A is None else (rd(A[0]), ru(A[1]))


def lit(x):
    """A double as a C++ literal of the section"""
    if x == INF:
        return 'codac_oo'
    if x == -INF:
        return '-codac_oo'
    return (x + 0.0).hex()


def num(x):
    """A double in an expression of the cases: a short integer, or hexadecimal"""
    if x == int(x) and abs(x) < 1e15:
        return '%d.0' % int(x)
    return lit(x)


def ref_lit(R):
    return 'codac_none' if R is None else '{ %s, %s }' % (lit(R[0]), lit(R[1]))


def itv_lit(X):
    return 'codac_none' if X is None else '{ %s, %s }' % (lit(X[0]), lit(X[1]))


def next_up(x):
    return math.nextafter(x, INF)


def next_down(x):
    return math.nextafter(x, -INF)


#---------------------------------------------------------------------------
# Exact interval arithmetic: an interval is a pair of bounds, or E
#---------------------------------------------------------------------------

def sign(b):
    return (b > 0) - (b < 0)


def badd(a, b):
    """a + b for two lower bounds or two upper bounds: an infinite one gives its infinity"""
    if is_inf(a):
        return a
    if is_inf(b):
        return b
    return a + b


def bmul(a, b):
    """The product of two bounds, 0 times an infinity being 0 (the hull of the products of points)"""
    if a == 0 or b == 0:
        return Fraction(0)
    if is_inf(a) or is_inf(b):
        return INF if sign(a) * sign(b) > 0 else -INF
    return a * b


def iadd(X, Y):
    if X is None or Y is None:
        return None
    return (badd(X[0], Y[0]), badd(X[1], Y[1]))


def ineg(X):
    return None if X is None else (-X[1], -X[0])


def isub(X, Y):
    return iadd(X, ineg(Y))


def imul(X, Y):
    if X is None or Y is None:
        return None
    c = [bmul(a, b) for a in X for b in Y]
    return (min(c), max(c))


def brecip(b):
    return Fraction(0) if is_inf(b) else (INF if b == 0 else 1 / b)


def recip_pieces(Y):
    """The hull of 1/y for the y of Y other than 0, on each side of 0: a list of intervals"""
    pieces = []
    if Y is None:
        return pieces
    if Y[1] > 0:                      # the y > 0 of Y: [max(lo, 0), hi]
        a, b = max(Y[0], Fraction(0)), Y[1]
        pieces.append((brecip(b), brecip(a)))
    if Y[0] < 0:                      # the y < 0 of Y
        a, b = Y[0], min(Y[1], Fraction(0))
        pieces.append((-brecip(-a), -brecip(-b)))
    return pieces


def idiv(X, Y):
    """The hull of the x/y for x in X and y in Y, y != 0"""
    if X is None:
        return None
    res = None
    for P in recip_pieces(Y):
        Q = imul(X, P)
        res = Q if res is None else (min(res[0], Q[0]), max(res[1], Q[1]))
    return res


def ihull(X, Y):
    if X is None:
        return Y
    if Y is None:
        return X
    return (min(X[0], Y[0]), max(X[1], Y[1]))


def imeet(X, Y):
    if X is None or Y is None:
        return None
    lo, hi = max(X[0], Y[0]), min(X[1], Y[1])
    return (lo, hi) if lo <= hi else None


def bpow(b, n):
    """b^n for a bound b and an int n > 0"""
    if is_inf(b):
        return INF if (b > 0 or n % 2 == 0) else -INF
    return b**n


def ipown(X, n):
    """The hull of x^n for the x of X, n an int (pown: any x, 0^0 = 1, no 0^n for n < 0)"""
    if X is None:
        return None
    if n == 0:
        return (Fraction(1), Fraction(1))
    if n < 0:
        return idiv((Fraction(1), Fraction(1)), ipown(X, -n))
    lo, hi = X
    if n % 2 == 1:
        return (bpow(lo, n), bpow(hi, n))
    ma, mb = abs(lo) if not is_inf(lo) else INF, abs(hi) if not is_inf(hi) else INF
    least = Fraction(0) if lo <= 0 <= hi else bpow(min(ma, mb), n)
    return (least, bpow(max(ma, mb), n))


def iint_floor(X):
    return None if X is None else tuple(b if is_inf(b) else Fraction(math.floor(b)) for b in X)


def iint_ceil(X):
    return None if X is None else tuple(b if is_inf(b) else Fraction(math.ceil(b)) for b in X)


def iint_integer(X):
    """The hull of the integers of X"""
    if X is None:
        return None
    lo = X[0] if is_inf(X[0]) else Fraction(math.ceil(X[0]))
    hi = X[1] if is_inf(X[1]) else Fraction(math.floor(X[1]))
    return (lo, hi) if lo <= hi else None


def sqrt_exact(q):
    """The square root of a Fraction q >= 0: exact for a square of rationals, else computed"""
    if q == 0:
        return Fraction(0)
    a, b = math.isqrt(q.numerator), math.isqrt(q.denominator)
    if a * a == q.numerator and b * b == q.denominator:
        return Fraction(a, b)
    return ev(mpmath.sqrt, q)


def isqrt_hull(X):
    """The square roots of the part of X in [0, +oo]"""
    if X is None or X[1] < 0:
        return None
    lo = max(X[0], Fraction(0))
    return (sqrt_exact(lo), INF if is_inf(X[1]) else sqrt_exact(X[1]))


def isqr(X):
    return ipown(X, 2)


#---------------------------------------------------------------------------
# Elementary functions: the hull over an interval of bounds (Fractions, mpf
# or infinities), computed with 2000 bits
#---------------------------------------------------------------------------

def mp_bound(b):
    return b if is_inf(b) else mpf_of(b)


def trig_value(f, b):
    """f(b) for a sine, cosine or tangent: the argument is a double or a multiple of pi computed with 2000 bits,
    so that a value below 2^-1900, which no double argument gives, is the zero of a multiple of pi"""
    m = f(b)
    return Fraction(0) if abs(m) < mpmath.ldexp(1, -1900) else snap(frac_of(m))


def exp_hull(X):
    if X is None:
        return None
    lo = Fraction(0) if X[0] == -INF else ev(mpmath.exp, X[0])
    hi = INF if X[1] == INF else ev(mpmath.exp, X[1])
    return (lo, hi)


def log_hull(X):
    if X is None or X[1] <= 0:
        return None
    lo = -INF if X[0] <= 0 else ev(mpmath.log, X[0])
    hi = INF if X[1] == INF else ev(mpmath.log, X[1])
    return (lo, hi)


def atan_hull(X):
    if X is None:
        return None
    half_pi = snap(frac_of(mpmath.pi / 2))
    lo = -half_pi if X[0] == -INF else ev(mpmath.atan, X[0])
    hi = half_pi if X[1] == INF else ev(mpmath.atan, X[1])
    return (lo, hi)


def trig_hull(kind, X):
    """The hull of sin or cos over [a, b], whose bounds are Fractions, mpf or infinities"""
    if X is None:
        return None
    a, b = X
    if is_inf(a) or is_inf(b):
        return (Fraction(-1), Fraction(1))
    A, B = mp_bound(a), mp_bound(b)
    f = mpmath.sin if kind == 'sin' else mpmath.cos
    two_pi = 2 * mpmath.pi

    def has(point):             # an integer k with A <= point + 2 k pi <= B
        k = mpmath.ceil((A - point) / two_pi)
        return point + k * two_pi <= B

    top = mpmath.pi / 2 if kind == 'sin' else mpmath.mpf(0)
    values = [trig_value(f, A), trig_value(f, B)]
    hi = Fraction(1) if has(top) else max(values)
    lo = Fraction(-1) if has(top + mpmath.pi) else min(values)
    return (lo, hi)


def tan_hull(X):
    """tan over [a, b]: [-oo, +oo] if a pole pi/2 + k pi is in it, else the values at the bounds"""
    if X is None:
        return None
    a, b = X
    if is_inf(a) or is_inf(b):
        return (-INF, INF)
    A, B = mp_bound(a), mp_bound(b)
    k = mpmath.ceil((A - mpmath.pi / 2) / mpmath.pi)
    if mpmath.pi / 2 + k * mpmath.pi <= B:
        return (-INF, INF)
    return (trig_value(mpmath.tan, A), trig_value(mpmath.tan, B))


def quadrant_angles(xa, xb, ya, yb):
    """The least and the greatest angle atan2(y, x) over the points other than 0 of
    [xa, xb] x [ya, yb], 0 <= xa <= xb <= +oo, 0 <= ya <= yb <= +oo (limits included),
    as mpf; None if the box is {0}"""
    if xb == 0 and yb == 0:
        return None
    half_pi = mpmath.pi / 2
    zero = mpmath.mpf(0)
    # the greatest angle: at (xa, yb)
    if xa == 0 and yb == 0:
        top = zero                  # the box lies on the x-axis
    elif yb == INF or xa == 0:
        top = half_pi
    else:
        top = mpmath.atan2(mpf_of(yb), mpf_of(xa))
    # the least angle: at (xb, ya)
    if xb == 0 and ya == 0:
        bottom = half_pi            # the box lies on the y-axis
    elif xb == INF or ya == 0:
        bottom = zero
    elif xb == 0:
        bottom = half_pi
    else:
        bottom = mpmath.atan2(mpf_of(ya), mpf_of(xb))
    return bottom, top


def atan2_hull(Y, X):
    """The hull of atan2(y, x) over the (x, y) other than (0, 0) of X x Y, atan2(0, x) being pi for
    x < 0 (IEEE 1788-2015): the four closed quadrants, each reflected on the first"""
    if X is None or Y is None:
        return None
    pi = mpmath.pi
    angles = []
    for sx in (1, -1):
        for sy in (1, -1):
            # the part of the box in the quadrant: x of sign sx, y >= 0 for sy = 1, y < 0 for sy = -1
            xs = (max(X[0], Fraction(0)), X[1]) if sx > 0 else (X[0], min(X[1], Fraction(0)))
            ys = (max(Y[0], Fraction(0)), Y[1]) if sy > 0 else (Y[0], min(Y[1], Fraction(0)))
            if xs[0] > xs[1] or ys[0] > ys[1]:
                continue
            if sy < 0 and ys[0] == 0 == ys[1] or sy < 0 and ys[0] >= 0:
                continue                # y < 0 has no point
            # reflected on x >= 0, y >= 0
            xr = xs if sx > 0 else (-xs[1], -xs[0])
            yr = ys if sy > 0 else (-ys[1], -ys[0])
            xr = (xr[0] + 0, xr[1])
            q = quadrant_angles(xr[0], xr[1], yr[0], yr[1])
            if q is None:
                continue
            bottom, top = q
            if sx > 0 and sy > 0:
                angles.append((bottom, top))
            elif sx < 0 and sy > 0:
                angles.append((pi - top, pi - bottom))
            elif sx > 0 and sy < 0:
                angles.append((-top, -bottom))
            else:
                angles.append((bottom - pi, top - pi))
    if not angles:
        return None
    lo = min(a for a, _ in angles)
    hi = max(b for _, b in angles)
    return (snap(frac_of(lo)), snap(frac_of(hi)))


def root_hull(X, n):
    """rootn: the real n-th roots of the x of X (n > 0), for an odd n of the negative ones too; for n < 0,
    1/x^(1/|n|), x != 0"""
    if X is None:
        return None
    m = abs(n)

    def rt(b):
        if is_inf(b):
            return b
        if b == 0:
            return Fraction(0)
        r = ev(lambda t: mpmath.root(abs(t), m), b)
        return r if b > 0 else -r

    if m % 2 == 1:
        R = (rt(X[0]), rt(X[1]))
    else:
        if X[1] < 0:
            return None
        R = (rt(max(X[0], Fraction(0))), rt(X[1]))
    if n > 0:
        return R
    return idiv((Fraction(1), Fraction(1)), R)


def pow_hull(X, Y):
    """The pow of IEEE 1788-2015 (Table 9.1) over the box X x Y: the powers of the part of X in [0, +oo],
    0^y being 0 for y > 0 and no value for y <= 0; the integer power on all of X for a degenerate
    integer exponent Y = [n]"""
    if X is None or Y is None:
        return None
    if Y[0] == Y[1] and not is_inf(Y[0]) and Y[0].denominator == 1:
        return ipown(X, int(Y[0]))
    if is_inf(Y[0]) and Y[0] == Y[1]:
        return None
    base = (max(X[0], Fraction(0)), X[1])
    if base[0] > base[1]:
        return None
    values = []
    for x in base:
        if is_inf(x):
            raise NotImplementedError('infinite base')
        for y in Y:
            if is_inf(y):
                raise NotImplementedError('infinite exponent')
            if x == 0:
                continue
            values.append(snap(frac_of(mpmath.power(mpf_of(x), mpf_of(y)))))
    if base[0] == 0:
        # x = 0: 0^y = 0 for the y > 0 of Y; x -> 0+ gives +oo for the y < 0 of Y, 1 for y = 0
        if Y[1] > 0:
            values.append(Fraction(0))
        if Y[0] < 0:
            values.append(INF)
        if Y[0] <= 0 <= Y[1] and Y[1] > 0:
            values.append(Fraction(1))
    if not values:
        return None
    return (min(values), max(values))


#---------------------------------------------------------------------------
# The expressions with pi, evaluated on doubles: the tightest outward rounding
# of each operation, as GAOL gives it for + - * /, and the C++ they are
#---------------------------------------------------------------------------

class V:
    """An interval of doubles (a pair of floats, or E), the value of an expression of GAOL's
    operations rounded outward, with the C++ expression that computes it"""

    def __init__(self, iv, cpp):
        self.iv = iv
        self.cpp = cpp

    def exact(self):
        return exi(self.iv)

    @staticmethod
    def of(v):
        if isinstance(v, V):
            return v
        return V((float(v), float(v)), num(float(v)))      # a double operand

    @staticmethod
    def make(A, cpp):
        return V(ref_of(A), cpp)

    def _bin(self, other, op, f):
        o = V.of(other)
        return V.make(f(self.exact(), o.exact()), '(%s %s %s)' % (self.cpp, op, o.cpp))

    def __add__(self, o):
        return self._bin(o, '+', iadd)

    def __radd__(self, o):
        return V.of(o)._bin(self, '+', iadd)

    def __sub__(self, o):
        return self._bin(o, '-', isub)

    def __rsub__(self, o):
        return V.of(o)._bin(self, '-', isub)

    def __mul__(self, o):
        return self._bin(o, '*', imul)

    def __rmul__(self, o):
        return V.of(o)._bin(self, '*', imul)

    def __truediv__(self, o):
        return self._bin(o, '/', idiv)

    def __rtruediv__(self, o):
        return V.of(o)._bin(self, '/', idiv)

    def __neg__(self):
        return V.make(ineg(self.exact()), '(-%s)' % self.cpp)

    def __or__(self, o):
        return self._bin(o, '|', ihull)


def interval_constant(value, cpp):
    return V((rd(snap(frac_of(value))), ru(snap(frac_of(value)))), cpp)


PI = interval_constant(mpmath.pi, 'interval::pi()')
TWO_PI = interval_constant(2 * mpmath.pi, 'interval::two_pi()')
HALF_PI = interval_constant(mpmath.pi / 2, 'interval::half_pi()')
PI_LB, PI_UB = PI.iv
MP_PI = mpmath.pi


def VI(lo, hi):
    """interval(lo, hi) written in an expression"""
    return V((lo, hi) if lo <= hi else None, 'interval(%s, %s)' % (num(lo), num(hi)))


#---------------------------------------------------------------------------
# The comparison of the original's expectation with the exact result
#---------------------------------------------------------------------------

disagreements = []
checks_made = 0


def disagree(case, what):
    disagreements.append('DISAGREEMENT %s: %s' % (case, what))


def approx_eq(o, v, eps):
    """Codac's Approx on doubles: equal, or closer than eps (v exact, a Fraction or an infinity)"""
    if o == v:
        return True
    if is_inf(o) or is_inf(v):
        return False
    return abs(Fraction(o) - v) < Fraction(eps)


def agrees(case, A, orig, tol):
    """Whether the original's expected interval (pair of floats, or E) is the exact result A:
    for tol = 0 its bounds are the tightest doubles around A (an `==`), else each is equal to
    the exact bound or within tol of it (an Approx)"""
    global checks_made
    checks_made += 1
    if A is None or orig is None:
        if (A is None) != (orig is None):
            disagree(case, 'the original expects %s, the exact result is %s' % (orig, A and ref_of(A)))
        return
    R = ref_of(A)
    for k in (0, 1):
        o = orig[k]
        ok = (o == R[k]) if tol == 0 else (o == R[k] or approx_eq(o, A[k], tol))
        if not ok:
            disagree(case, 'bound %d: the original expects %s, the exact bound is %s (tightest double %s)'
                     % (k, lit(o), float(A[k]) if not is_inf(A[k]) else A[k], lit(R[k])))


def agrees_value(case, v, orig, tol=0):
    """The same for a result that is one number (a double): the original's value is within the doubles
    around the exact v, or within tol of it"""
    global checks_made
    checks_made += 1
    R = (rd(v), ru(v))
    if tol == 0:
        ok = R[0] <= orig <= R[1]
    else:
        ok = R[0] <= orig <= R[1] or approx_eq(orig, v, tol)
    if not ok:
        disagree(case, 'the original expects %s, the exact value is %s (doubles around it %s %s)'
                 % (lit(orig), float(v) if not is_inf(v) else v, lit(R[0]), lit(R[1])))


def holds(case, ok, what):
    global checks_made
    checks_made += 1
    if not ok:
        disagree(case, what)


EPS = Fraction(math.ulp(1.0))              # std::numeric_limits<double>::epsilon()
DEFAULT_EPS = 10 * EPS                     # Codac's DEFAULT_EPS


#---------------------------------------------------------------------------
# The rows. A group is a list of rows (strings); the .inc holds them between
# the marks "// BEGIN <group>" and "// END <group>"
#---------------------------------------------------------------------------

F = 'Interval'
O = 'Interval_operations'
R_ = 'Interval_rounding'


def name_of(file, line, text=''):
    return 'Codac %s:%d%s' % (file, line, (' ' + text) if text else '')


GROUPS = {}


def group(fn):
    GROUPS[fn.__name__[len('group_'):]] = fn
    return fn


# The intervals the originals write, with the infinities and the largest double
oo = INF


@group
def group_constructors():
    """interval(), interval(a) and interval(a, b)"""
    cases = [
        # line, number of arguments, a, b, expected
        (26, 0, 0.0, 0.0, (-oo, oo)),
        (27, 1, 1.0, 0.0, (1.0, 1.0)),
        (28, 1, -oo, 0.0, E),
        (29, 1, oo, 0.0, E),
        (30, 2, 1.0, 0.0, E),
        (216, 1, oo, 0.0, E),
    ]
    rows = []
    for line, n, a, b, expected in cases:
        # IBEX's rules, which IEEE 1788-2015 leaves free: [-oo, +oo] for no argument, the empty set for an
        # infinity as the one argument, and for bounds in the wrong order (and for [+oo, +oo] and [-oo, -oo])
        if n == 0:
            A = (-INF, INF)
        elif n == 1:
            A = None if math.isinf(a) else (Fraction(a), Fraction(a))
        else:
            A = (ex(a), ex(b)) if a <= b and a != INF and b != -INF else None
        agrees(name_of(F, line), A, expected, 0)
        rows.append('{ "%s", %d, %s, %s, %s }' % (name_of(F, line), n, lit(a), lit(b), ref_lit(ref_of(A))))
    return rows


def binary_rows(cases, op, file, with_y_double=False):
    rows = []
    for line, x, y, expected in cases:
        X = exi(x)
        if with_y_double:
            # an infinite double is no real number: the sum, the product... have no value
            A = None if math.isinf(y) else op(X, (Fraction(y), Fraction(y)))
            ylit = lit(y)
        else:
            A = op(X, exi(y))
            ylit = itv_lit(y)
        agrees(name_of(file, line), A, expected, 0)
        rows.append('{ "%s", %s, %s, %s }' % (name_of(file, line), itv_lit(x), ylit, ref_lit(ref_of(A))))
    return rows



@group
def group_additions():
    cases = [
        (109, E, (0.0, 1.0), E),
        (110, (0.0, 1.0), E, E),
        (111, (-oo, 1.0), (0.0, 1.0), (-oo, 2.0)),
        (112, (1.0, oo), (0.0, 1.0), (1.0, oo)),
        (113, (-oo, oo), (0.0, 1.0), (-oo, oo)),
        (114, (MAX, oo), (1.0, 1.0), (MAX, oo)),
        (115, (MAX, oo), (-1.0, -1.0), (next_down(MAX), oo)),
        (116, (MAX, oo), (MAX, oo), (MAX, oo)),
        (117, (MAX, oo), E, E),                 # Interval(-oo) is the empty set
        (118, (MAX, oo), E, E),                 # Interval(oo) is the empty set
        (119, (-oo, -MAX), (1.0, 1.0), (-oo, next_up(-MAX))),
        (120, (-oo, -MAX), (-1.0, -1.0), (-oo, -MAX)),
        (121, (-oo, -MAX), (-oo, -MAX), (-oo, -MAX)),
        (270, (3.0, 8.0), E, E),                 # [3, 8] + ([1, 3] & [6, 7]), the empty set
    ]
    return binary_rows([(l, x, y, e) for l, x, y, e in cases[:-1]], iadd, O) + \
        binary_rows([(l, x, y, e) for l, x, y, e in cases[-1:]], iadd, F)


@group
def group_scalar_additions():
    cases = [
        (123, E, oo, E),
        (124, E, 0.0, E),
        (125, (0.0, 1.0), 1.0, (1.0, 2.0)),
        (126, (0.0, 1.0), -oo, E),
        (127, (0.0, 1.0), oo, E),
        (128, (-oo, 1.0), 1.0, (-oo, 2.0)),
    ]
    return binary_rows(cases, iadd, O, True)


@group
def group_products():
    cases = [
        (130, E, (0.0, 1.0), E),
        (131, (0.0, 0.0), (-oo, oo), (0.0, 0.0)),
        (132, (-1.0, 1.0), (-oo, 0.0), (-oo, oo)),
        (133, (-oo, -1.0), (-1.0, 0.0), (0.0, oo)),
        (134, (-oo, 1.0), (-1.0, 0.0), (-1.0, oo)),
        (135, (0.0, 1.0), (1.0, oo), (0.0, oo)),
        (136, (0.0, 1.0), (-1.0, oo), (-1.0, oo)),
        (137, (-oo, -1.0), (0.0, 1.0), (-oo, 0.0)),
        (138, (-oo, 1.0), (0.0, 1.0), (-oo, 1.0)),
        (139, (0.0, 1.0), (-oo, -1.0), (-oo, 0.0)),
        (140, (0.0, 1.0), (-oo, 1.0), (-oo, 1.0)),
        (141, (1.0, oo), (0.0, 1.0), (0.0, oo)),
        (142, (-1.0, oo), (0.0, 1.0), (-1.0, oo)),
        (143, (1.0, 2.0), (1.0, 2.0), (1.0, 4.0)),
        (144, (1.0, 2.0), (-2.0, 3.0), (-4.0, 6.0)),
        (145, (-1.0, 1.0), (-2.0, 3.0), (-3.0, 3.0)),
    ]
    return binary_rows(cases, imul, O)


@group
def group_scalar_products():
    cases = [
        (146, (1.0, 2.0), -oo, E),
        (147, (1.0, 2.0), oo, E),
        (148, (1.0, 2.0), -1.0, (-2.0, -1.0)),
    ]
    return binary_rows(cases, imul, O, True)


@group
def group_quotients():
    cases = [
        (150, E, (0.0, 1.0), E),
        (151, (0.0, 0.0), (0.0, 0.0), E),
        (152, (1.0, 2.0), (0.0, 0.0), E),
        (153, (-oo, oo), (0.0, 0.0), E),
        (154, (0.0, 0.0), (0.0, 1.0), (0.0, 0.0)),
        (155, (0.0, 0.0), (-oo, oo), (0.0, 0.0)),
        (156, (6.0, 12.0), (2.0, 3.0), (2.0, 6.0)),
        (157, (6.0, 12.0), (-3.0, -2.0), (-6.0, -2.0)),
        (158, (6.0, 12.0), (-2.0, 3.0), (-oo, oo)),
        (159, (-oo, -1.0), (-1.0, 0.0), (1.0, oo)),
        (160, (-oo, -1.0), (0.0, 1.0), (-oo, -1.0)),
        (161, (1.0, oo), (-1.0, 0.0), (-oo, -1.0)),
        (162, (1.0, oo), (0.0, 1.0), (1.0, oo)),
        (163, (-1.0, 1.0), (-1.0, 1.0), (-oo, oo)),
    ]
    return binary_rows(cases, idiv, O)


@group
def group_scalar_quotients():
    cases = [
        (164, (1.0, 2.0), -oo, E),
        (165, (1.0, 2.0), oo, E),
        (166, (1.0, 2.0), -1.0, (-2.0, -1.0)),
    ]
    return binary_rows(cases, idiv, O, True)


@group
def group_hulls():
    cases = [
        (77, (0.0, 1.0), (2.0, 3.0), (0.0, 3.0)),
        (81, (0.0, 1.0), E, (0.0, 1.0)),
        (85, (-1.0, -1.0), (0.0, oo), (-1.0, oo)),
    ]
    return binary_rows(cases, ihull, F)


@group
def group_intersections():
    cases = [
        (89, (0.0, 1.0), (2.0, 3.0), E),
        (93, (0.0, 1.0), E, E),
        (97, (0.0, 2.0), (-1.0, 1.0), (0.0, 1.0)),
        (101, (0.0, 1.0), (0.0, oo), (0.0, 1.0)),
        (105, (0.0, 1.0), (-oo, 0.0), (0.0, 0.0)),
        (109, (0.0, 1.0), (1.0, 2.0), (1.0, 1.0)),
        (269, (1.0, 3.0), (6.0, 7.0), E),
    ]
    return binary_rows(cases, imeet, F)


def exact_mid(X):
    """The midpoint of an interval, GAOL's convention for the unbounded ones (IEEE 1788-2015: the middle
    of [-oo, +oo] is 0, that of [a, +oo] and [-oo, b] the largest double and its opposite)"""
    lo, hi = X
    if lo == -INF and hi == INF:
        return Fraction(0)
    if hi == INF:
        return FMAX
    if lo == -INF:
        return -FMAX
    return (lo + hi) / 2


@group
def group_accessors():
    # line, interval, then the original's lb, ub, mid, rad, diam, mag, mig, smig
    cases = [
        (113, (0.0, 2.0), (0.0, 2.0, 1.0, 1.0, 2.0, 2.0, 0.0, 0.0)),
        (125, (-3.0, -1.0), (-3.0, -1.0, -2.0, 1.0, 2.0, 3.0, 1.0, -1.0)),
        (137, (-3.0, 1.0), (-3.0, 1.0, -1.0, 2.0, 4.0, 3.0, 0.0, 0.0)),
        (149, (-oo, 0.0), (-oo, 0.0, -MAX, oo, oo, oo, 0.0, 0.0)),
        (161, (-oo, oo), (-oo, oo, 0.0, oo, oo, oo, 0.0, 0.0)),
        (173, (MAX, oo), (MAX, oo, MAX, oo, oo, oo, MAX, MAX)),
    ]
    rows = []
    for line, x, orig in cases:
        X = exi(x)
        lo, hi = X
        finite = not (is_inf(lo) or is_inf(hi))
        mid = exact_mid(X)
        # the radius: the smallest r with x inside [m - r, m + r] for the midpoint m, which is exact for these
        rad = (hi - lo) / 2 if finite else INF
        width = (hi - lo) if finite else INF
        mag = INF if (is_inf(lo) or is_inf(hi)) else max(abs(lo), abs(hi))
        mig = Fraction(0) if lo <= 0 <= hi else min(abs(lo), abs(hi))
        smig = Fraction(0) if lo <= 0 <= hi else (lo if lo > 0 else hi)
        values = [lo, hi, mid, rad, width, mag, mig, smig]
        for k, (v, o) in enumerate(zip(values, orig)):
            what = ('lb', 'ub', 'mid', 'rad', 'diam', 'mag', 'mig', 'smig')[k]
            agrees_value(name_of(F, line, what), v, o)
        cols = [mid, rad, width, mag, mig, smig]
        rows.append('{ "%s", %s, { %s } }' % (name_of(F, line), itv_lit(x), ', '.join(ref_lit((rd(v), ru(v))) for v in cols)))
    return rows


@group
def group_midpoints():
    """mid() of the intervals of lines 235 to 241"""
    cases = [
        (235, (0.0, 2.0), lambda v: v == 1.0),
        (236, (-oo, oo), lambda v: v == 0),
        (237, (0.0, oo), lambda v: v == FMAX),
        (238, (-oo, 0.0), lambda v: v == -FMAX),
        (240, (MAX, oo), lambda v: v in (FMAX, -FMAX)),
        (241, (0.0, 0.0), lambda v: v == 0),
    ]
    rows = []
    for line, x, pred in cases:
        v = exact_mid(exi(x))
        holds(name_of(F, line, 'mid'), pred(v), 'the midpoint %s is not the one the original expects' % float(v))
        rows.append('{ "%s", %s, %s }' % (name_of(F, line), itv_lit(x), ref_lit((rd(v), ru(v)))))
    return rows


@group
def group_relations():
    """predicates of two intervals: line, operation, x, y, what the original expects of the predicate"""
    cases = [
        (34, 'set_eq', (-oo, oo), (-oo, oo), True),
        (35, 'set_neq', (-oo, oo), (0.0, oo), True),
        (36, 'set_eq', (0.0, 1.0), (0.0, 1.0), True),
        (37, 'set_neq', (0.0, 2.0), (0.0, 1.0), True),
        (38, 'set_neq', (0.0, 2.0), (1.0, 3.0), True),
        (39, 'set_eq', (0.0, oo), (0.0, oo), True),
        (40, 'set_eq', E, E, True),
        (41, 'set_eq', (0.0, 0.0), (0.0, 0.0), True),
        (200, 'set_leq', (0.0, 1.0), (0.0, 2.0), True),
        (201, 'set_leq', (0.0, 1.0), (1.0, 2.0), False),
        (202, 'set_le && set_neq', (0.0, 1.0), (0.0, 2.0), False),
        (203, 'set_le && set_neq', (-oo, 0.0), (-oo, oo), True),
        (204, 'set_le && set_neq', E, (0.0, 1.0), True),
        (205, 'set_geq', (0.0, 2.0), (0.0, 1.0), True),
        (206, 'set_geq', (1.0, 2.0), (0.0, 1.0), False),
        (207, 'set_geq && set_neq', (0.0, 2.0), (0.0, 2.0), False),
        (208, 'set_geq && set_neq', (-oo, oo), (-oo, 0.0), True),
        (209, 'set_geq && set_neq', E, E, False),
        (214, 'set_disjoint', (-oo, 0.0), (0.0, oo), False),
        (215, 'set_disjoint', (0.0, 1.0), (2.0, 3.0), True),
    ]
    rows = []
    for line, op, x, y, orig in cases:
        X, Y = exi(x), exi(y)
        same = (X is None and Y is None) or X == Y

        def subset(a, b):          # a is a subset of b
            return a is None or (b is not None and b[0] <= a[0] and a[1] <= b[1])

        def interior(a, b):        # a is in the interior of b (IEEE 1788-2015: the infinite bounds of b are beyond a's)
            return a is None or (b is not None and (b[0] < a[0] or b[0] == -INF) and (a[1] < b[1] or b[1] == INF))

        if op == 'set_eq':
            truth = same
        elif op == 'set_neq':
            truth = not same
        elif op == 'set_disjoint':
            truth = X is None or Y is None or imeet(X, Y) is None
        elif op == 'set_leq':
            truth = subset(X, Y)
        elif op == 'set_geq':
            truth = subset(Y, X)
        elif op == 'set_le && set_neq':
            truth = interior(X, Y) and not same
        else:                      # strict_superset
            truth = subset(Y, X) and not same
        holds(name_of(F, line, op), truth == orig, 'the original expects %s, the exact predicate is %s' % (orig, truth))
        rows.append('{ "%s", "%s", %s, %s, %s }' % (name_of(F, line), op, itv_lit(x), itv_lit(y), 'true' if truth else 'false'))
    return rows


@group
def group_point_relations():
    """predicates of an interval and a double"""
    cases = [
        (210, 'set_contains', (0.0, 2.0), 0.0, True),
        (211, 'set_contains', (-oo, oo), oo, True),
        (212, 'set_strictly_contains', (0.0, 2.0), 0.0, False),
        (213, 'set_strictly_contains', (0.0, 2.0), 1.0, True),
    ]
    rows = []
    for line, op, x, d, orig in cases:
        X = exi(x)
        # the extended real line: an infinite bound holds its infinity, which a closed interval of reals does not
        # (IEEE 1788-2015 leaves the point +oo out): the convention of GAOL and of the original
        if op == 'set_contains':
            truth = X is not None and X[0] <= ex(d) <= X[1]
        else:
            truth = X is not None and X[0] < ex(d) < X[1]
        holds(name_of(F, line, op), truth == orig, 'the original expects %s, the predicate is %s' % (orig, truth))
        rows.append('{ "%s", "%s", %s, %s, %s }' % (name_of(F, line), op, itv_lit(x), lit(d), 'true' if truth else 'false'))
    return rows


def finite_double_inside(X):
    """Whether a finite double lies strictly between the bounds"""
    if X is None:
        return False
    lo, hi = X
    lo = float(lo) if not is_inf(lo) else -INF
    hi = float(hi) if not is_inf(hi) else INF
    d = next_up(lo)
    return not math.isinf(d) and d < hi


@group
def group_predicates():
    """predicates of one interval"""
    nd = TINY
    cases = [
        (217, 'is_empty', (0.0, 1.0), False),
        (218, 'is_a_double', (0.0, 0.0), True),
        (220, 'is_finite', E, True),
        (221, 'is_finite', (0.0, 1.0), True),
        (222, 'is_finite', (-oo, 0.0), False),
        (223, 'is_finite', (0.0, oo), False),
        (224, 'is_bisectable', E, False),
        (225, 'is_bisectable', (MAX, oo), False),
        (226, 'is_bisectable', (-oo, -MAX), False),
        (227, 'is_bisectable', (0.0, nd), False),
        (228, 'is_bisectable', (0.0, next_up(nd)), True),
        (229, 'is_bisectable', (10.0, next_up(next_up(10.0))), True),
        (230, 'is_bisectable', (next_down(-nd), 0.0), True),
        (231, 'is_bisectable', (next_down(next_down(10.0)), 10.0), True),
        (232, 'is_bisectable', (10.0, next_up(10.0)), False),
        (233, 'is_bisectable', (-nd, 0.0), False),
    ]
    rows = []
    for line, op, x, orig in cases:
        X = exi(x)
        if op == 'is_empty':
            truth = X is None
        elif op == 'is_a_double':
            truth = X is not None and X[0] == X[1] and not is_inf(X[0])
        elif op == 'is_finite':
            truth = X is None or not (is_inf(X[0]) or is_inf(X[1]))
        else:
            truth = finite_double_inside(X)
        holds(name_of(F, line, op), truth == orig, 'the original expects %s, the predicate is %s' % (orig, truth))
        rows.append('{ "%s", "%s", %s, %s }' % (name_of(F, line), op, itv_lit(x), 'true' if truth else 'false'))
    return rows


def comparison_rows():
    """x < y and x > y of Codac, which tell TRUE, FALSE, UNKNOWN or EMPTY, by the two relations of GAOL
    that tell them apart: for <, certainly_le (TRUE) and certainly_geq (FALSE); for >, certainly_ge (TRUE)
    and certainly_leq (FALSE); both hold, vacuously, for an empty interval (EMPTY)"""
    T, Fl, U, Em = 'TRUE', 'FALSE', 'UNKNOWN', 'EMPTY'
    less = [
        (300, (0.0, 1.0), (2.0, 3.0), T), (301, (0.0, 1.0), (1.0, 2.0), U), (302, (0.0, 1.0), (0.0, 1.0), U),
        (303, (2.0, 3.0), (0.0, 1.0), Fl), (304, (2.0, 3.0), (-oo, 2.0), Fl), (305, (-oo, 2.0), (3.0, oo), T),
        (306, E, (0.0, 1.0), Em), (307, (0.0, 1.0), E, Em),
    ]
    greater = [
        (309, (2.0, 3.0), (0.0, 1.0), T), (310, (1.0, 2.0), (0.0, 1.0), U), (311, (0.0, 1.0), (0.0, 1.0), U),
        (312, (0.0, 1.0), (2.0, 3.0), Fl), (313, (-oo, 2.0), (2.0, 3.0), Fl), (314, (3.0, oo), (-oo, 2.0), T),
        (315, E, (0.0, 1.0), Em), (316, (0.0, 1.0), E, Em),
    ]
    expected = {T: (True, False), Fl: (False, True), U: (False, False), Em: (True, True)}

    def forall(X, Y, cmp):
        # for all x of X, y of Y: x cmp y, which is true for an empty interval
        if X is None or Y is None:
            return True
        return cmp(X, Y)

    rows = []
    for table, kind in ((less, 'less'), (greater, 'greater')):
        for line, x, y, orig in table:
            X, Y = exi(x), exi(y)
            if kind == 'less':
                first = forall(X, Y, lambda a, b: a[1] < b[0])       # x < y for all x, y
                second = forall(X, Y, lambda a, b: a[0] >= b[1])     # x >= y for all x, y
            else:
                first = forall(X, Y, lambda a, b: a[0] > b[1])       # x > y
                second = forall(X, Y, lambda a, b: a[1] <= b[0])     # x <= y
            holds(name_of(F, line, kind), (first, second) == expected[orig],
                  'the original expects %s, the relations give %s' % (orig, (first, second)))
            rows.append('{ "%s", %s, %s, %s, %s }' % (name_of(F, line), itv_lit(x), itv_lit(y),
                                                      'true' if first else 'false', 'true' if second else 'false'))
    return rows[:8], rows[8:]


@group
def group_less():
    return comparison_rows()[0]


@group
def group_greater():
    return comparison_rows()[1]


@group
def group_neighbours():
    """next_float and previous_float"""
    # line, function, x, what the original compares the result with, as a predicate of the result
    cases = [
        (94, 'next_float', 0.0, lambda r: 0 < r),
        (95, 'previous_float', 0.0, lambda r: 0 > r),
        (96, 'next_float', 1.0, lambda r: 1 < r),
        (97, 'previous_float', 1.0, lambda r: 1 > r),
        (98, 'next_float', oo, lambda r: oo == r),
        (99, 'previous_float', -oo, lambda r: -oo == r),
        (100, 'next_float', -oo, lambda r: -MAX >= r),
        (101, 'previous_float', oo, lambda r: MAX <= r),
    ]
    rows = []
    for line, fn, x, pred in cases:
        # the double next to x in the direction, the infinities being next to the largest doubles
        if fn == 'next_float':
            r = x if x == INF else (-MAX if x == -INF else next_up(x))
        else:
            r = x if x == -INF else (MAX if x == INF else next_down(x))
        holds(name_of(O, line, fn), pred(r), 'the result %s does not satisfy the original' % lit(r))
        rows.append('{ "%s", "%s", %s, { %s, %s } }' % (name_of(O, line), fn, lit(x), lit(r), lit(r)))
    return rows


def unary_exact(fn, X):
    if fn == 'neg':
        return ineg(X)
    if fn == 'sqr':
        return isqr(X)
    if fn == 'sqrt':
        return isqrt_hull(X)
    if fn == 'exp':
        return exp_hull(X)
    if fn == 'log':
        return log_hull(X)
    if fn == 'floor':
        return iint_floor(X)
    if fn == 'ceil':
        return iint_ceil(X)
    if fn == 'integer':
        return iint_integer(X)
    if fn == 'sin' or fn == 'cos':
        return trig_hull(fn, X)
    if fn == 'tan':
        return tan_hull(X)
    if fn == 'atan':
        return atan_hull(X)
    raise ValueError(fn)


def unary_rows(cases, file):
    """line, function, x, then the original's expected interval and its tolerance (0: equal), or a
    predicate of the exact result"""
    rows = []
    for line, fn, x, orig, tol in cases:
        A = unary_exact(fn, exi(x))
        c = name_of(file, line, fn)
        if callable(orig):
            holds(c, orig(A), 'the exact result %s does not satisfy the original' % (A and ref_of(A),))
        else:
            agrees(c, A, orig, tol)
        rows.append('{ "%s", "%s", %s, %s }' % (name_of(file, line), fn, itv_lit(x), ref_lit(ref_of(A))))
    return rows


@group
def group_negations():
    cases = [
        (104, 'neg', (0.0, 1.0), (-1.0, 0.0), 0),
        (105, 'neg', (-oo, oo), (-oo, oo), 0),
        (106, 'neg', (-oo, 0.0), (0.0, oo), 0),
        (107, 'neg', (-oo, 1.0), (-1.0, oo), 0),
    ]
    return unary_rows(cases, O)


@group
def group_roots_of_squares():
    cases = [
        (168, 'sqrt', (-oo, oo), (0.0, oo), 0),
        (169, 'sqrt', (-oo, 0.0), (0.0, 0.0), 0),
        (170, 'sqrt', (-9.0, 4.0), (0.0, 2.0), 0),
        (171, 'sqrt', (4.0, 9.0), (2.0, 3.0), 0),
        (172, 'sqrt', (-9.0, -4.0), E, 0),
        (173, 'sqrt', (-9.0, oo), (0.0, oo), 0),
    ]
    return unary_rows(cases, O)


@group
def group_logarithms():
    ln2 = math.log(2.0)
    cases = [
        (175, 'log', E, E, 0),
        (176, 'log', (-oo, oo), (-oo, oo), 0),
        (177, 'log', (0.0, oo), (-oo, oo), 0),
        (178, 'log', (-oo, 0.0), E, 0),
        (179, 'log', (1.0, 2.0), (0.0, ln2), DEFAULT_EPS),
        (180, 'log', (-1.0, 1.0), (-oo, 0.0), DEFAULT_EPS),
        (181, 'log', (0.0, 1.0), (-oo, 0.0), DEFAULT_EPS),
        (182, 'log', (1.0, oo), (0.0, oo), DEFAULT_EPS),
        (183, 'log', (0.0, 0.0), E, 0),
        (184, 'log', (-2.0, -1.0), E, 0),
        (185, 'log', (0.0, TINY), lambda A: ru(A[1]) > -744.5, 0),
    ]
    return unary_rows(cases, O)


@group
def group_exponentials():
    cases = [
        (187, 'exp', E, E, 0),
        (188, 'exp', (-oo, oo), (0.0, oo), 0),
        (189, 'exp', (0.0, oo), (1.0, oo), DEFAULT_EPS),
        (190, 'exp', (-oo, 0.0), (0.0, 1.0), DEFAULT_EPS),
        (191, 'exp', (0.0, 2.0), (1.0, math.exp(2.0)), DEFAULT_EPS),
        (192, 'exp', (-1.0, 1.0), (math.exp(-1.0), math.exp(1.0)), DEFAULT_EPS),
        (193, 'exp', (1.e100, 1.e111), (MAX, oo), 0),
        (194, 'exp', (MAX, oo), (MAX, oo), 0),
        (195, 'exp', (0.0, MAX), (1.0, oo), DEFAULT_EPS),
    ]
    return unary_rows(cases, O)


@group
def group_integer_functions():
    m = 1e8
    cases = [
        (214, 'floor', E, E, 0),
        (215, 'floor', (-oo, -0.000001), (-oo, -1.0), 0),
        (216, 'floor', (0.00000001, oo), (0.0, oo), 0),
        (217, 'floor', (-oo, oo), (-oo, oo), 0),
        (218, 'floor', (0.01, 2.99), (0.0, 2.0), 0),
        (219, 'floor', (-0.01, 2.99), (-1.0, 2.0), 0),
        (220, 'floor', (1.000000001, 1.9999999999), (1.0, 1.0), 0),
        (221, 'floor', (m, MAX), (m, MAX), 0),
        (223, 'ceil', E, E, 0),
        (224, 'ceil', (-oo, -0.000001), (-oo, 0.0), 0),
        (225, 'ceil', (0.00000001, oo), (1.0, oo), 0),
        (226, 'ceil', (-oo, oo), (-oo, oo), 0),
        (227, 'ceil', (0.01, 2.99), (1.0, 3.0), 0),
        (228, 'ceil', (-0.01, 2.99), (0.0, 3.0), 0),
        (229, 'ceil', (1.000000001, 1.9999999999), (2.0, 2.0), 0),
        (230, 'ceil', (m, MAX), (m, MAX), 0),
        (232, 'integer', E, E, 0),
        (233, 'integer', (-oo, -0.000001), (-oo, -1.0), 0),
        (234, 'integer', (0.00000001, oo), (1.0, oo), 0),
        (235, 'integer', (-oo, oo), (-oo, oo), 0),
        (236, 'integer', (0.01, 2.99), (1.0, 2.0), 0),
        (237, 'integer', (-0.01, 2.99), (0.0, 2.0), 0),
        (238, 'integer', (1.000000001, 1.9999999999), E, 0),
        (239, 'integer', (m, MAX), (m, MAX), 0),
    ]
    return unary_rows(cases, O)


@group
def group_tangents():
    x1 = (PI_LB / 4.0, (1 - 1e-10) * PI_LB / 2.0)
    x2 = (-(1 - 1e-10) * PI_LB / 2.0, PI_LB / 4.0)
    cases = [
        (249, 'tan', (-oo, oo), (-oo, oo), 0),
        (254, 'tan', x1, lambda A: abs(A[0] - 1) < DEFAULT_EPS and A[1] > 1000, 0),
        (261, 'tan', x2, lambda A: A[0] < -1000 and ref_of(A)[1] == 1.0, 0),
    ]
    return unary_rows(cases, O)


@group
def group_rounded_values():
    """The points of the rounding test of Codac that GAOL's tables (tests/elementary_values.h) do not hold"""
    return rounded_points(only_new=True)


# The doubles immediately below and above the exact value of each function at x, as the rounding test of Codac
# writes them: function, x, below, above (line 299-334 of codac2_tests_Interval_rounding.cpp)
ROUNDING_POINTS = [
    (299, 'exp', '0x1.0000000000000p+0', '0x1.5bf0a8b145769p+1', '0x1.5bf0a8b14576ap+1'),
    (300, 'exp', '0x1.0000000000000p-1', '0x1.a61298e1e069bp+0', '0x1.a61298e1e069cp+0'),
    (301, 'exp', '-0x1.0000000000000p+0', '0x1.78b56362cef37p-2', '0x1.78b56362cef38p-2'),
    (302, 'exp', '0x1.4000000000000p+3', '0x1.5829dcf95055fp+14', '0x1.5829dcf950560p+14'),
    (303, 'exp', '0x1.0624dd2f1a9fcp-10', '0x1.0041919b7ee33p+0', '0x1.0041919b7ee34p+0'),
    (304, 'exp', '-0x1.4000000000000p+4', '0x1.1b48655f37266p-29', '0x1.1b48655f37267p-29'),
    (305, 'log', '0x1.0000000000000p+1', '0x1.62e42fefa39efp-1', '0x1.62e42fefa39f0p-1'),
    (306, 'log', '0x1.4000000000000p+3', '0x1.26bb1bbb55515p+1', '0x1.26bb1bbb55516p+1'),
    (307, 'log', '0x1.999999999999ap-4', '-0x1.26bb1bbb55516p+1', '-0x1.26bb1bbb55515p+1'),
    (308, 'log', '0x1.8000000000000p+0', '0x1.9f323ecbf984bp-2', '0x1.9f323ecbf984cp-2'),
    (309, 'log', '0x1.4f8b588e368f1p-17', '-0x1.7069e2aa2aa5bp+3', '-0x1.7069e2aa2aa5ap+3'),
    (310, 'log', '0x1.2a05f20000000p+33', '0x1.7069e2aa2aa5ap+4', '0x1.7069e2aa2aa5bp+4'),
    (311, 'sin', '0x1.0000000000000p+0', '0x1.aed548f090ceep-1', '0x1.aed548f090cefp-1'),
    (312, 'sin', '0x1.0000000000000p-1', '0x1.eaee8744b05efp-2', '0x1.eaee8744b05f0p-2'),
    (313, 'sin', '0x1.8000000000000p+1', '0x1.210386db6d55bp-3', '0x1.210386db6d55cp-3'),
    (314, 'sin', '-0x1.0000000000000p+1', '-0x1.d18f6ead1b446p-1', '-0x1.d18f6ead1b445p-1'),
    (315, 'sin', '0x1.9000000000000p+6', '-0x1.03425b78c4db9p-1', '-0x1.03425b78c4db8p-1'),
    (316, 'sin', '0x1.0624dd2f1a9fcp-10', '0x1.0624da5218a62p-10', '0x1.0624da5218a63p-10'),
    (317, 'cos', '0x1.0000000000000p+0', '0x1.14a280fb5068bp-1', '0x1.14a280fb5068cp-1'),
    (318, 'cos', '0x1.0000000000000p-1', '0x1.c1528065b7d4fp-1', '0x1.c1528065b7d50p-1'),
    (319, 'cos', '0x1.8000000000000p+1', '-0x1.fae04be85e5d3p-1', '-0x1.fae04be85e5d2p-1'),
    (320, 'cos', '-0x1.0000000000000p+1', '-0x1.aa22657537205p-2', '-0x1.aa22657537204p-2'),
    (321, 'cos', '0x1.9000000000000p+6', '0x1.b981dbf665fdfp-1', '0x1.b981dbf665fe0p-1'),
    (322, 'cos', '0x1.8000000000000p+0', '0x1.21bd54fc5f9a7p-4', '0x1.21bd54fc5f9a8p-4'),
    (323, 'tan', '0x1.0000000000000p+0', '0x1.8eb245cbee3a5p+0', '0x1.8eb245cbee3a6p+0'),
    (324, 'tan', '0x1.0000000000000p-1', '0x1.17b4f5bf3474ap-1', '0x1.17b4f5bf3474bp-1'),
    (325, 'tan', '-0x1.3333333333333p+0', '-0x1.493c43acb164dp+1', '-0x1.493c43acb164cp+1'),
    (326, 'tan', '0x1.8000000000000p+1', '-0x1.23ef71254b870p-3', '-0x1.23ef71254b86fp-3'),
    (327, 'atan', '0x1.0000000000000p+0', '0x1.921fb54442d18p-1', '0x1.921fb54442d19p-1'),
    (328, 'atan', '0x1.0000000000000p-1', '0x1.dac670561bb4fp-2', '0x1.dac670561bb50p-2'),
    (329, 'atan', '-0x1.0000000000000p+1', '-0x1.1b6e192ebbe45p+0', '-0x1.1b6e192ebbe44p+0'),
    (330, 'atan', '0x1.f400000000000p+9', '0x1.91de2c0e658bcp+0', '0x1.91de2c0e658bdp+0'),
    (331, 'sqrt', '0x1.0000000000000p+1', '0x1.6a09e667f3bccp+0', '0x1.6a09e667f3bcdp+0'),
    (332, 'sqrt', '0x1.8000000000000p+1', '0x1.bb67ae8584caap+0', '0x1.bb67ae8584cabp+0'),
    (333, 'sqrt', '0x1.999999999999ap-4', '0x1.43d136248490fp-2', '0x1.43d1362484910p-2'),
    (334, 'sqrt', '0x1.ad7f29abcaf48p-24', '0x1.4b96be9c2da2bp-12', '0x1.4b96be9c2da2cp-12'),
]

# The points GAOL's tests/elementary_values.h does not hold (the other 30 are there, same function, argument
# and doubles around the value: --elementary-values checks it)
NOT_IN_ELEMENTARY_VALUES = {303, 309, 310, 322, 325, 334}


def rounded_points(only_new):
    rows = []
    for line, fn, x, below, above in ROUNDING_POINTS:
        xd, bd, ad = float.fromhex(x), float.fromhex(below), float.fromhex(above)
        A = unary_exact(fn, exi((xd, xd)))
        R = ref_of(A)
        c = name_of(R_, line, fn)
        # the original's doubles are the ones immediately below and above the exact value
        holds(c, (bd, ad) == R and next_up(bd) == ad, 'the original has %s %s, the doubles around the exact value are %s %s'
              % (lit(bd), lit(ad), lit(R[0]), lit(R[1])))
        if (not only_new) or line in NOT_IN_ELEMENTARY_VALUES:
            rows.append('{ "%s", "%s", %s, %s }' % (name_of(R_, line), fn, itv_lit((xd, xd)), ref_lit(R)))
    return rows


def parse_elementary_values(path):
    text = open(path).read()
    rows = re.findall(r'\{ "(\w+)", (-?0x[0-9a-fp.+-]+), (-?0x[0-9a-fp.+-]+|-?gaol_tests::inf), '
                      r'(-?0x[0-9a-fp.+-]+|-?gaol_tests::inf) \}', text)
    table = {}
    for fn, x, below, above in rows:
        conv = lambda s: math.copysign(INF, -1 if s.startswith('-') else 1) if 'inf' in s else float.fromhex(s)
        table.setdefault((fn, float.fromhex(x)), []).append((conv(below), conv(above)))
    return table


def check_elementary_values(path):
    table = parse_elementary_values(path)
    bad = 0
    for line, fn, x, below, above in ROUNDING_POINTS:
        found = (float.fromhex(below), float.fromhex(above)) in table.get((fn, float.fromhex(x)), [])
        expected = line not in NOT_IN_ELEMENTARY_VALUES
        if found != expected:
            bad += 1
            print('%s: %s(%s) %s in %s' % (name_of(R_, line), fn, x, 'is not' if expected else 'is', path))
    print('rounding test of Codac: %d points, %d in %s, %d not; %s' % (
        len(ROUNDING_POINTS), len(ROUNDING_POINTS) - len(NOT_IN_ELEMENTARY_VALUES), path, len(NOT_IN_ELEMENTARY_VALUES),
        'all as the tool says' if not bad else '%d DIFFER' % bad))
    return bad == 0


@group
def group_atan2():
    cases = [
        # line, y, x, the original's expected value (a V, or an interval), tolerance
        (266, (-oo, oo), E, E, 0),
        (267, (1.0, 1.0), (1.0, 1.0), PI / 4.0, DEFAULT_EPS),
        (268, (-1.0, -1.0), (-1.0, -1.0), (-3.0) * PI / 4.0, DEFAULT_EPS),
        (269, (-1.0, -1.0), (1.0, 1.0), -PI / 4.0, DEFAULT_EPS),
        (270, (1.0, 1.0), (-1.0, -1.0), 3.0 * PI / 4.0, DEFAULT_EPS),
        (271, (0.0, oo), (0.0, oo), VI(0.0, 1.0) * HALF_PI, DEFAULT_EPS),
        (272, (-oo, -TINY), (-oo, 0.0), -(HALF_PI | PI), DEFAULT_EPS),
        (273, (-oo, 0.0), (0.0, oo), VI(-1.0, 0.0) * HALF_PI, DEFAULT_EPS),
        (274, (0.0, oo), (-oo, 0.0), (HALF_PI | PI), DEFAULT_EPS),
        (275, (1.0, oo), (-1.0, 1.0), (PI / 4.0 | 3.0 * PI / 4.0), DEFAULT_EPS),
        (276, (-oo, -1.0), (-1.0, 1.0), -(PI / 4.0 | 3.0 * PI / 4.0), DEFAULT_EPS),
        (277, (-1.0, 1.0), (1.0, oo), VI(-1.0, 1.0) * PI / 4.0, 0),
        (278, (-1.0, 1.0), (-oo, -1.0), VI(-1.0, 1.0) * PI, 0),
        (279, (-oo, oo), (-oo, oo), VI(-1.0, 1.0) * PI, 0),
        (280, (0.0, 0.0), (0.0, 0.0), E, 0),
    ]
    rows = []
    for line, y, x, orig, tol in cases:
        A = atan2_hull(exi(y), exi(x))
        o = orig.iv if isinstance(orig, V) else orig
        agrees(name_of(O, line, 'atan2'), A, o, tol)
        rows.append('{ "%s", %s, %s, %s }' % (name_of(O, line), itv_lit(y), itv_lit(x), ref_lit(ref_of(A))))
    return rows


# CHECK_trigo: the eight ways the original gives sin and cos an argument and an expectation:
# name, function, the argument as an expression of x (a V), the argument over the reals (mpf),
# and whether the expectation is the opposite of the one given
def shift_real(X, s):
    return None if X is None else tuple(b if is_inf(b) else mp_bound(b) + s for b in X)


def reverse_real(X, s):
    return None if X is None else (-INF if is_inf(X[1]) else s - mp_bound(X[1]), INF if is_inf(X[0]) else s - mp_bound(X[0]))


def neg_real(X):
    return reverse_real(X, mpmath.mpf(0))


TRIG_VARIANTS = [
    ('sin(x)', 'sin', lambda x: x, lambda X: X, False),
    ('sin(pi-x)', 'sin', lambda x: PI - x, lambda X: reverse_real(X, MP_PI), False),
    ('sin(x+2pi)', 'sin', lambda x: x + TWO_PI, lambda X: shift_real(X, 2 * MP_PI), False),
    ('sin(-x)', 'sin', lambda x: -x, neg_real, True),
    ('cos(x-pi/2)', 'cos', lambda x: x - HALF_PI, lambda X: shift_real(X, -MP_PI / 2), False),
    ('cos(pi/2-x)', 'cos', lambda x: HALF_PI - x, lambda X: reverse_real(X, MP_PI / 2), False),
    ('cos(x+pi/2)', 'cos', lambda x: x + HALF_PI, lambda X: shift_real(X, MP_PI / 2), True),
    ('cos(x+2pi-pi/2)', 'cos', lambda x: x + TWO_PI - HALF_PI, lambda X: shift_real(X, 3 * MP_PI / 2), False),
]


@group
def group_trigonometric():
    up = PI_UB
    lo = PI_LB
    cases = [
        (200, (-oo, oo), (-1.0, 1.0)),
        (201, E, E),
        (202, (0.0, up / 2.0), (0.0, 1.0)),
        (203, (0.0, up), (0.0, 1.0)),
        (204, (0.0, 3 * up / 2.0), (-1.0, 1.0)),
        (205, (lo, 3 * up / 2.0), (-1.0, 0.0)),
        (206, (0.5, 1.5), (math.sin(0.5), math.sin(1.5))),
        (207, (1.5, 3.0), (math.sin(3.0), 1.0)),
        (208, (3.0, 4.0), (math.sin(4.0), math.sin(3.0))),
        (209, (3.0, 5.0), (-1.0, math.sin(3.0))),
        (210, (3.0, 2 * up + 1.5), (-1.0, math.sin(1.5))),
        (211, (5.0, 2 * up + 1.5), (math.sin(5.0), math.sin(1.5))),
        (212, (5.0, 2 * up + 3.0), (math.sin(5.0), 1.0)),
    ]
    rows = []
    for line, x, expected in cases:
        refs = []
        for vname, fn, arg, real, opposite in TRIG_VARIANTS:
            c = name_of(O, line, vname)
            xv = V(x, 'x')
            # the original's expectation of the result, and the exact result over the real argument
            orig = None if expected is None else ((-expected[1], -expected[0]) if opposite else expected)
            real_arg = real(exi(x))
            A = trig_hull(fn, real_arg)
            agrees(c, A, orig, Fraction(1, 10**9))
            # the reference: the hull over the argument of doubles that GAOL computes
            B = trig_hull(fn, arg(xv).exact())
            if A is not None and B is not None:
                holds(c, B[0] <= A[0] and A[1] <= B[1], 'the reference does not enclose the exact result')
            refs.append(ref_of(B))
        rows.append('{ "%s", %s, { %s } }' % (name_of(O, line), itv_lit(x), ', '.join(ref_lit(r) for r in refs)))
    return rows


@group
def group_tangent_arguments():
    """tan of an argument computed with pi: line, the argument (a V), the original's expectation, the
    tolerance, and the predicates the original checks"""
    cases = [
        (241, PI / 2.0, (-oo, oo), 0, False),
        (243, -PI, lambda A: A is not None and A[0] <= 0 <= A[1] and A[1] - A[0] < Fraction(1, 10**8), 0, True),
        (248, (3.0 * PI / 4.0) | (5.0 * PI / 4.0), (-1.0, 1.0), DEFAULT_EPS, False),
        (250, ((-PI) / 4.0) | (PI / 4.0), (-1.0, 1.0), DEFAULT_EPS, False),
    ]
    rows = []
    for line, arg, orig, tol, holds_zero in cases:
        # the exact result over the real argument
        real = {
            241: (mpmath.pi / 2, mpmath.pi / 2),
            243: (-mpmath.pi, -mpmath.pi),
            248: (3 * mpmath.pi / 4, 5 * mpmath.pi / 4),
            250: (-mpmath.pi / 4, mpmath.pi / 4),
        }[line]
        A = tan_hull(real)
        B = tan_hull(arg.exact())
        c = name_of(O, line, 'tan')
        if callable(orig):
            holds(c, orig(A), 'the exact result over the real argument does not satisfy the original')
        else:
            agrees(c, A, orig, tol)
        holds(c, A[0] >= B[0] and A[1] <= B[1], 'the reference does not enclose the exact result')
        rows.append('{ "%s", [] { return %s; }, %s, %s }' % (name_of(O, line), arg.cpp, ref_lit(ref_of(B)), 'true' if holds_zero else 'false'))
    return rows


@group
def group_integer_powers():
    # line, x, p, the original's expected interval, the eps of the original
    cases = [
        (282, (-oo, oo), 4, (0.0, oo), 0), (283, E, 4, E, 0), (284, (2.0, 3.0), 4, (16.0, 81.0), 0),
        (285, (-2.0, 3.0), 4, (0.0, 81.0), 0), (286, (-3.0, 2.0), 4, (0.0, 81.0), 0), (287, (2.0, oo), 4, (16.0, oo), 0),
        (288, (-oo, oo), 3, (-oo, oo), 0), (289, E, 3, E, 0), (290, (2.0, 3.0), 3, (8.0, 27.0), 0),
        (291, (-2.0, 3.0), 3, (-8.0, 27.0), 0), (292, (-3.0, 2.0), 3, (-27.0, 8.0), 0), (293, (2.0, oo), 3, (8.0, oo), 0),
        (294, (-10.0, 10.0), -2, (1.0 / 100, oo), EPS),
    ]
    rows = []
    for line, x, p, orig, eps in cases:
        X = exi(x)
        A = ipown(X, p)
        agrees(name_of(O, line, 'pow(x,%d)' % p), A, orig, eps)
        # pow(-x, p): the same for an even p, the opposite for an odd one
        An = ipown(ineg(X), p)
        o2 = orig if p % 2 == 0 else (None if orig is None else (-orig[1], -orig[0]))
        agrees(name_of(O, line, 'pow(-x,%d)' % p), An, o2, eps)
        rows.append('{ "%s", %s, %d, %s }' % (name_of(O, line), itv_lit(x), p, ref_lit(ref_of(A))))
    return rows


@group
def group_powers():
    # line, x, the exponent as an interval, the original's expected interval, tolerance
    cases = [
        (300, (4.0, 4.0), (0.5, 0.5), (2.0, 2.0), DEFAULT_EPS),
        (301, (4.0, 4.0), (1.5, 1.5), (8.0, 8.0), DEFAULT_EPS),
        (302, (4.0, 9.0), (0.5, 0.5), (2.0, 3.0), DEFAULT_EPS),
        (303, (4.0, 4.0), (-0.5, -0.5), (0.5, 0.5), DEFAULT_EPS),
        (304, (0.0, 4.0), (0.5, 0.5), (0.0, 2.0), DEFAULT_EPS),
        (305, (2.0, 3.0), (1.5, 1.5), (2.0**1.5, 3.0**1.5), DEFAULT_EPS),
        (310, (-2.0, 3.0), (3.0, 3.0), (-8.0, 27.0), 0),
        (311, (-2.0, -2.0), (2.0, 2.0), (4.0, 4.0), 0),
        (312, (2.0, 3.0), (4.0, 4.0), (16.0, 81.0), 0),
        (313, (4.0, 4.0), (0.0, 0.0), (1.0, 1.0), 0),
        (314, (-10.0, 10.0), (-2.0, -2.0), (1.0 / 100, oo), EPS),
        (318, (-4.0, -1.0), (0.5, 0.5), E, 0),
        (319, (-4.0, -1.0), (0.5, 0.5), E, 0),
        (321, (-4.0, 9.0), (0.5, 0.5), (0.0, 3.0), DEFAULT_EPS),
        (323, (-2.0, 3.0), (1.0, 2.0), (0.0, 9.0), DEFAULT_EPS),
        (327, (-2.0, 3.0), (3.0, 3.0), (-8.0, 27.0), 0),
        (329, (-2.0, 3.0), (2.0, 2.0), (0.0, 9.0), 0),
        (332, (4.0, 4.0), (oo, oo), E, 0),
        (333, (4.0, 4.0), (-oo, -oo), E, 0),
        (334, E, (1.5, 1.5), E, 0),
    ]
    rows = []
    for line, x, y, orig, tol in cases:
        A = pow_hull(exi(x), exi(y))
        agrees(name_of(O, line, 'pow'), A, orig, tol)
        rows.append('{ "%s", %s, %s, %s }' % (name_of(O, line), itv_lit(x), itv_lit(y), ref_lit(ref_of(A))))
    return rows


@group
def group_lower_bounds():
    """pow(x, y).lb() >= 0 where the negative part of x is out of the domain"""
    cases = [(322, (-4.0, 9.0), (0.5, 0.5))]
    rows = []
    for line, x, y in cases:
        A = pow_hull(exi(x), exi(y))
        holds(name_of(O, line, 'pow'), A[0] >= 0, 'the exact lower bound %s is below 0' % A[0])
        rows.append('{ "%s", %s, %s, %s }' % (name_of(O, line), itv_lit(x), itv_lit(y), ref_lit((rd(A[0]), rd(A[0])))))
    return rows


@group
def group_roots():
    cases = [
        (336, (0.0, 1.0), -1, (1.0, oo), 0),
        (337, (-27.0, -8.0), 3, (-3.0, -2.0), DEFAULT_EPS),
        (338, (-4.0, 1.0), 2, (0.0, 1.0), 0),
        (339, (-8.0, 1.0), 3, (-2.0, 1.0), DEFAULT_EPS),
    ]
    rows = []
    for line, x, n, orig, tol in cases:
        A = root_hull(exi(x), n)
        agrees(name_of(O, line, 'root'), A, orig, tol)
        rows.append('{ "%s", %s, %d, %s }' % (name_of(O, line), itv_lit(x), n, ref_lit(ref_of(A))))
    return rows


@group
def group_midpoint_property():
    """(x + mid(x)) - mid(x) contains x: its exact value is x"""
    x = (float('2.7629416257693772963'), float('2.7629416257693835135'))
    A = exi(x)
    holds(name_of(F, 248), A is not None, '')
    return ['{ "%s", %s, %s }' % (name_of(F, 248), itv_lit(x), ref_lit(ref_of(A)))]


@group
def group_bisections():
    ratio02 = struct.unpack('<f', struct.pack('<f', 0.2))[0]    # 0.2 as the float that bisect(float ratio) of the original gets
    cases = [
        # line, x, ratio, the original's two parts, tolerance
        (278, (-oo, oo), 0.5, ((-oo, 0.0), (0.0, oo)), 0),
        (282, (0.0, oo), 0.5, ((0.0, MAX), (MAX, oo)), 0),
        (287, (-oo, 0.0), 0.5, ((-oo, -MAX), (-MAX, 0.0)), 0),
        (292, (0.0, 10.0), ratio02, ((0.0, 2.0), (2.0, 10.0)), Fraction(1, 10**7)),
    ]
    rows = []
    for line, x, ratio, orig, tol in cases:
        X = exi(x)
        lo, hi = X
        if is_inf(lo) or is_inf(hi):
            # the cut of a half-line or of the whole line is a convention: the largest double or its opposite, 0
            cut = orig[0][1]
            cut_ref = (cut, cut)
            holds(name_of(F, line, 'bisect'), orig[0] == (x[0], cut) and orig[1] == (cut, x[1]), 'the parts are not cut at the point')
        else:
            c = lo + Fraction(ratio) * (hi - lo)
            cut_ref = (rd(c), ru(c))
            for part, o in zip(((lo, c), (c, hi)), orig):
                agrees(name_of(F, line, 'bisect'), part, o, tol)
        rows.append('{ "%s", %s, %s, %s }' % (name_of(F, line), itv_lit(x), lit(ratio), ref_lit(cut_ref)))
    return rows


#---------------------------------------------------------------------------
# The .inc: the rows between the marks
#---------------------------------------------------------------------------

def generated():
    out = {}
    for name, fn in GROUPS.items():
        out[name] = fn()
    return out


def marks(text, name):
    begin = re.search(r'^[ ]*// BEGIN %s\n' % re.escape(name), text, re.M)
    end = re.search(r'^[ ]*// END %s\n' % re.escape(name), text, re.M)
    return begin, end


def rows_text(rows, indent):
    return ''.join('%s%s,\n' % (' ' * indent, r) for r in rows)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--check', metavar='INC', help='the rows of INC are those the cases give')
    ap.add_argument('--write', metavar='INC', help='rewrite the rows of INC')
    ap.add_argument('--list', action='store_true', help='print the rows of every group')
    ap.add_argument('--selftest', action='store_true')
    ap.add_argument('--elementary-values', metavar='H', help='tests/elementary_values.h')
    ap.add_argument('--quiet', action='store_true', help='no list of the disagreements')
    args = ap.parse_args()
    status = 0
    if args.selftest:
        status |= 0 if selftest() else 1
    if args.elementary_values:
        status |= 0 if check_elementary_values(args.elementary_values) else 1
    if not (args.check or args.write or args.list):
        return status
    rows = generated()
    total = sum(len(r) for r in rows.values())
    print('%d rows in %d groups; %d originals compared with the exact results, %d disagreements'
          % (total, len(rows), checks_made, len(disagreements)))
    if not args.quiet:
        for d in disagreements:
            print(d)
    if args.list:
        for name, rs in rows.items():
            print('// %s' % name)
            for r in rs:
                print('  ' + r)
    path = args.check or args.write
    if path:
        text = open(path).read()
        new = text
        for name, rs in rows.items():
            begin, end = marks(new, name)
            if not begin or not end:
                print('%s: no marks for the group %s' % (path, name))
                status = 1
                continue
            indent = len(re.match(r"[ ]*", new[begin.start():]).group(0))
            body = rows_text(rs, indent)
            if args.check:
                if new[begin.end():end.start()] != body:
                    status = 1
                    old = new[begin.end():end.start()].splitlines()
                    now = body.splitlines()
                    print('%s: the rows of %s differ from the cases (%d rows in the file, %d from the cases)' % (path, name, len(old), len(now)))
                    for i, (a, b) in enumerate(zip(old, now)):
                        if a != b:
                            print('  row %d:\n    file:  %s\n    cases: %s' % (i + 1, a.strip(), b.strip()))
                            break
            else:
                new = new[:begin.end()] + body + new[end.start():]
        if args.write:
            open(path, 'w').write(new)
            print('%s: rows written' % path)
        elif status == 0:
            print('%s: every row is the one the cases give' % path)
    return status


def selftest():
    """The exact oracle against brute force: sampled points of the operands give results inside the hulls,
    and the hulls are reached by the samples (or by points ever nearer to a pole or an infinity)"""
    import random
    rng = random.Random(20261102)
    failures = []

    def fail(what):
        failures.append(what)

    big = Fraction(10**9)

    def samples(X):
        """Points of an interval: its bounds, inner points, and for an infinite bound large points"""
        lo, hi = X
        a = -big * 1000 if lo == -INF else lo
        b = big * 1000 if hi == INF else hi
        pts = {a, b, (a + b) / 2}
        for k in range(1, 8):
            pts.add(a + (b - a) * k / 8)
        if lo == -INF:
            pts.update({-big, Fraction(-10**6)})
        if hi == INF:
            pts.update({big, Fraction(10**6)})
        return pts

    def random_interval(values):
        while True:
            a, b = rng.choice(values), rng.choice(values)
            if a > b:
                a, b = b, a
            if a != INF and b != -INF:         # [+oo, +oo] and [-oo, -oo] are no intervals
                return (a, b)

    # + - * / over small integers and infinities
    values = [-INF, -3, -2, -1, 0, 1, 2, 5, INF]
    values = [v if is_inf(v) else Fraction(v) for v in values]
    tiny = Fraction(1, 10**9)
    for _ in range(600):
        X, Y = random_interval(values), random_interval(values)
        for name, op in (('+', iadd), ('-', isub), ('*', imul), ('/', idiv)):
            A = op(X, Y)
            xs, ys = samples(X), samples(Y)
            if name == '/':
                ys = ys | ({tiny, -tiny} if Y[0] <= 0 <= Y[1] else set())
                ys = {y for y in ys if y != 0 and Y[0] <= y <= Y[1]}
                if not ys:
                    if A is not None:
                        fail('%s %s %s should be empty' % (X, name, Y))
                    continue
            vals = []
            for x in xs:
                for y in ys:
                    vals.append({'+': x + y, '-': x - y, '*': x * y, '/': (x / y if y != 0 else None)}[name])
            vals = [v for v in vals if v is not None]
            lo, hi = min(vals), max(vals)
            if A is None or lo < A[0] or hi > A[1]:
                fail('%s %s %s: samples %s..%s outside %s' % (X, name, Y, float(lo), float(hi), A))
                continue
            for k, ext in ((0, lo), (1, hi)):
                if is_inf(A[k]):
                    if abs(ext) < big // 10:
                        fail('%s %s %s: the infinite bound %d is not approached by the samples (%s)' % (X, name, Y, k, float(ext)))
                elif A[k] != ext and not (any(is_inf(b) for b in X + Y) and abs(A[k] - ext) < Fraction(1, 10**6)):
                    # (with an infinite bound, a sample next to it only comes near the limit)
                    fail('%s %s %s: the bound %d is %s, the samples reach %s' % (X, name, Y, k, A[k], ext))

    # integer powers
    for _ in range(200):
        X = random_interval(values)
        n = rng.choice([1, 2, 3, 4, 5, -1, -2, -3])
        A = ipown(X, n)
        pts = [p for p in samples(X) if not (n < 0 and p == 0)]
        if n < 0 and X[0] <= 0 <= X[1]:
            pts += [tiny, -tiny] if True else []
            pts = [p for p in pts if X[0] <= p <= X[1]]
        if not pts:
            if A is not None:
                fail('pown %s %d should be empty' % (X, n))
            continue
        vals = [p**n for p in pts]
        if A is None or min(vals) < A[0] or max(vals) > A[1]:
            fail('pown %s %d: %s outside %s' % (X, n, (float(min(vals)), float(max(vals))), A))

    # sin, cos, tan, atan over doubles
    for _ in range(120):
        a = rng.uniform(-12, 12)
        b = a + rng.choice([0.01, 0.5, 2.0, 4.0, 7.0])
        X = (Fraction(a), Fraction(b))
        pts = [a + (b - a) * k / 4000 for k in range(4001)]
        for fn, f in (('sin', math.sin), ('cos', math.cos)):
            A = trig_hull(fn, X)
            vals = [f(p) for p in pts]
            if min(vals) < float(A[0]) - 1e-12 or max(vals) > float(A[1]) + 1e-12 or \
               float(A[0]) < min(vals) - 2e-5 or float(A[1]) > max(vals) + 2e-5:
                fail('%s hull over [%s, %s] is %s, samples %s..%s' % (fn, a, b, (float(A[0]), float(A[1])), min(vals), max(vals)))
        A = tan_hull(X)
        has_pole = any(math.floor((p - math.pi / 2) / math.pi) != math.floor((q - math.pi / 2) / math.pi)
                       for p, q in zip(pts, pts[1:]))
        if has_pole != (A == (-INF, INF)):
            fail('tan over [%s, %s]: pole %s, hull %s' % (a, b, has_pole, A))
        if not has_pole and not (float(A[0]) <= math.tan(a) + 1e-9 and math.tan(b) <= float(A[1]) + 1e-9):
            fail('tan over [%s, %s]: %s' % (a, b, A))

    # atan2 over boxes: the angles of sampled points are inside, and the hull is reached to within 1e-6
    bounds = [-INF, -2, -1, 0, 1, 2, INF]
    bounds = [b if is_inf(b) else Fraction(b) for b in bounds]
    eps = Fraction(1, 10**12)
    for _ in range(600):
        X, Y = random_interval(bounds), random_interval(bounds)
        A = atan2_hull(Y, X)
        xs, ys = samples(X), samples(Y)
        # near the axes: points next to 0 on each side, to see the limits of the angles
        for B, S in ((X, xs), (Y, ys)):
            if B[0] <= 0 <= B[1]:
                S.update({eps, -eps})
        xs = {x for x in xs if X[0] <= x <= X[1]}
        ys = {y for y in ys if Y[0] <= y <= Y[1]}
        angles = []
        for x in xs:
            for y in ys:
                if x == 0 and y == 0:
                    continue
                angles.append(float(mpmath.atan2(mpf_of(y), mpf_of(x))) if y != 0 or x > 0 else math.pi)
        if not angles:
            if A is not None:
                fail('atan2 over %s x %s should be empty' % (Y, X))
            continue
        lo, hi = min(angles), max(angles)
        if A is None:
            fail('atan2 over %s x %s: no hull, samples %s..%s' % (Y, X, lo, hi))
        elif lo < float(A[0]) - 1e-9 or hi > float(A[1]) + 1e-9 or float(A[0]) < lo - 1e-6 or float(A[1]) > hi + 1e-6:
            fail('atan2 over %s x %s: hull %s, samples %s..%s' % (Y, X, (float(A[0]), float(A[1])), lo, hi))

    # the doubles around a rational: rd and ru, normal and subnormal
    for _ in range(2000):
        q = Fraction(rng.getrandbits(80) + 1, rng.getrandbits(70) + 1) * Fraction(2)**rng.randint(-1090, 960)
        if rng.getrandbits(1):
            q = -q
        lo, hi = rd(q), ru(q)
        if not (Fraction(lo) <= q <= Fraction(hi)) or (lo != hi and next_up(lo) != hi):
            fail('rd/ru of %s: %s %s' % (q, lo, hi))

    for f in failures[:20]:
        print('SELFTEST FAILED: ' + f)
    print('selftest: %s (%d failures)' % ('ok' if not failures else 'FAILED', len(failures)))
    return not failures


if __name__ == '__main__':
    sys.exit(main())
