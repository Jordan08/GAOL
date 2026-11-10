#!/usr/bin/env python3
# Tests of GAOL v5: the forward tests of IBEX (ibex-lib commit 65ed587,
# tests/TestInterval.cpp and tests/TestArith.cpp), as native GAOL tests.
#
# The case list lives here, with the result each IBEX test expects, and so does
# the oracle: for each case, the exact mathematical result of the operation on
# the exact inputs (the doubles the test writes), computed apart from GAOL and
# from IBEX, by exact rational arithmetic (fractions) for the operations that
# are rational and by mpmath with 2000 bits for the elementary functions, as
# tests/elementary_values.py does. The reference stored for an interval result
# is Ref{lo, hi}: lo the greatest double not above the exact lower bound, hi
# the least double not below the exact upper bound (-oo or +oo beyond the
# doubles), `ibex_empty` for the empty set; for a real result the same pair
# (equal when the result is a double); for a relation, the exact truth value.
#
# What is exact and what is a convention:
#   - the arithmetic, the sets, floor, ceil, integer, sqrt, the integer powers,
#     the roots, the elementary functions, atan2 (the hull of the angles of the
#     points of the box, (0, 0) left out, the half-line y = 0, x < 0 having the
#     angle pi): the exact image, the hull of the values over the interval;
#   - an operation on the interval an earlier operation of GAOL gave (pi - x,
#     x + 2 pi...) is replayed: the operations + - * / are exact in GAOL, so
#     the reference is the exact image of the interval of doubles it gives;
#   - the midpoint of an unbounded interval, -MAXREAL for [-oo, b], MAXREAL
#     for [a, +oo], 0 for [-oo, +oo]: a convention (the same in IBEX and in
#     GAOL), no exact value; the radius and the width of an unbounded interval
#     are +oo;
#   - contains(d) is lb <= d <= ub on the extended reals, so that [-oo, +oo]
#     contains +oo (IBEX: "d can also be an open bound"; IEEE 1788-2015 would
#     say no);
#   - is_bisectable: a double lies strictly between the bounds, the infinities
#     being next to the largest doubles.
#
# The result each IBEX test expects is a cross-check, not the reference: it is
# compared with the exact result with the tolerance the IBEX test uses (1e-10,
# absolute, for check(); none for ==), or taken as the double on either side of
# the exact result, and every disagreement is reported (--originals).
#
# The rows of the tables of tests/test_ibex_codac.cpp, the section ibex_ (the forward tests of IBEX),
# are written by this script between the lines
#
#     // BEGIN <table>
#     // END <table>
#
# (the text between them, those lines included, is what the script writes).
#
#     python3 -I ibex_forward_cases.py                     the tables, on the standard output
#     python3 -I ibex_forward_cases.py --write ../../test_ibex_codac.cpp    replaces the tables of the file
#     python3 -I ibex_forward_cases.py --check ../../test_ibex_codac.cpp    every table of the file is what is generated
#     python3 -I ibex_forward_cases.py --originals [-v]    the disagreements with IBEX's expectations
#     python3 -I ibex_forward_cases.py --selftest          checks the oracle against sampling
#
# The exit status is 1 when --check finds a difference, --originals a
# disagreement that is not listed in KNOWN, or --selftest a failure. Python 3.8
# or later, and mpmath (1.1 or later).
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-02)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

import argparse
import math
import random
import re
import struct
import sys
from fractions import Fraction

import mpmath

mpmath.mp.prec = 2000

INF = math.inf
DBL_MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)
ERROR = 1e-10      # the tolerance of check() of IBEX (tests/utils.cpp), absolute
BIG = Fraction(2) ** 4000     # an infinite bound of a box, for atan2
SENTINEL = Fraction(2) ** 1100    # a value far beyond the doubles


#---------------------------------------------------------------------------
# Extended numbers: a Fraction or an infinite float
#---------------------------------------------------------------------------
def is_inf(a):
    return isinstance(a, float)


def X(d):
    """The exact value of a double, or the infinity itself"""
    return d if math.isinf(d) else Fraction(d)


def e_add(a, b):
    if is_inf(a):
        return a
    if is_inf(b):
        return b
    return a + b


def e_sign(a):
    return (a > 0) - (a < 0)


def e_mul(a, b):
    """a*b with 0*oo = 0, the convention of the hull of the products of the bounds"""
    if a == 0 or b == 0:
        return Fraction(0)
    if is_inf(a) or is_inf(b):
        return INF if e_sign(a) * e_sign(b) > 0 else -INF
    return a * b


def e_recip(a):
    """1/a for a != 0, 1/oo = 0"""
    return Fraction(0) if is_inf(a) else 1 / a


def mpf_of(q):
    return mpmath.mpf(q.numerator) / mpmath.mpf(q.denominator)


def mpq(v):
    """An mpf as a Fraction, or an infinity; beyond 2^1100 in magnitude, as the sentinel 2^1100"""
    if mpmath.isinf(v):
        return INF if v > 0 else -INF
    s, m, e, bc = v._mpf_
    if m == 0:
        return Fraction(0)
    if e + bc > 1100:
        return -SENTINEL if s else SENTINEL
    q = Fraction(m) * (Fraction(2) ** e if e >= 0 else Fraction(1, 2 ** -e))
    return -q if s else q


#---------------------------------------------------------------------------
# Directed roundings to the doubles
#---------------------------------------------------------------------------
def _round_positive(q, up):
    """The double at most (or at least, with up) the Fraction q > 0, inf past the largest"""
    n, d = q.numerator, q.denominator
    e = n.bit_length() - d.bit_length()
    if (n < (d << e)) if e >= 0 else ((n << -e) < d):
        e -= 1
    # 2^e <= q < 2^(e+1)
    if e > 1023:
        return INF if up else DBL_MAX
    k = max(e, -1022) - 52
    m, r = divmod(n, d << k) if k >= 0 else divmod(n << -k, d)
    if up and r:
        m += 1
    try:
        return math.ldexp(m, k)
    except OverflowError:
        return INF


def rd(a):
    """The greatest double not above the extended number a"""
    if is_inf(a):
        return a
    if a == 0:
        return 0.0
    if a > 0:
        return _round_positive(a, False)
    return -_round_positive(-a, True)


def ru(a):
    """The least double not below the extended number a"""
    if is_inf(a):
        return a
    return -rd(-a)


def snap_bounds(q):
    """(rd, ru) of q, both the double q is when q is within 2^-1800 (relatively) of one: a function
    that is exactly a double (exp(0), sqrt(4), pow(0.25, 1.5)...) computed inexactly by mpmath"""
    lo, hi = rd(q), ru(q)
    if lo != hi and not is_inf(q):
        for d in (lo, hi):
            if d != 0.0 and not math.isinf(d) and abs(q - Fraction(d)) <= abs(Fraction(d)) / Fraction(2) ** 1800:
                return d, d
    return lo, hi


def nearest(q):
    """The double nearest the Fraction q, ties to even"""
    return float(q)


#---------------------------------------------------------------------------
# Intervals with exact bounds (Fractions or infinities); the empty set has lo None
#---------------------------------------------------------------------------
class Itv:
    __slots__ = ('lo', 'hi')

    def __init__(self, lo=None, hi=None):
        self.lo, self.hi = lo, hi

    @property
    def empty(self):
        return self.lo is None

    def __repr__(self):
        return 'empty' if self.empty else '[%s, %s]' % (self.lo, self.hi)

    def neg(self):
        return self if self.empty else Itv(-self.hi, -self.lo)

    def add(self, o):
        if self.empty or o.empty:
            return Itv()
        return Itv(e_add(self.lo, o.lo), e_add(self.hi, o.hi))

    def sub(self, o):
        return self.add(o.neg())

    def mul(self, o):
        if self.empty or o.empty:
            return Itv()
        p = [e_mul(a, b) for a in (self.lo, self.hi) for b in (o.lo, o.hi)]
        return Itv(min(p), max(p))

    def div(self, o):
        """The hull of the x/y for the x of self and the y of o other than 0"""
        if self.empty or o.empty:
            return Itv()
        pieces = []
        if o.lo < 0:     # the y of o below 0: 1/y from 1/lo down to -oo when 0 is in o
            pieces.append(Itv(-INF, e_recip(o.lo)) if o.hi >= 0 else Itv(e_recip(o.hi), e_recip(o.lo)))
        if o.hi > 0:
            pieces.append(Itv(e_recip(o.hi), INF) if o.lo <= 0 else Itv(e_recip(o.hi), e_recip(o.lo)))
        if not pieces:
            return Itv()
        results = [self.mul(p) for p in pieces]
        return Itv(min(r.lo for r in results), max(r.hi for r in results))

    def hull(self, o):
        if self.empty:
            return o
        if o.empty:
            return self
        return Itv(min(self.lo, o.lo), max(self.hi, o.hi))

    def inter(self, o):
        if self.empty or o.empty:
            return Itv()
        lo, hi = max(self.lo, o.lo), min(self.hi, o.hi)
        return Itv() if lo > hi else Itv(lo, hi)

    def out(self):
        """The tightest interval of doubles containing self (what the operations of GAOL give)"""
        if self.empty:
            return self
        return Itv(X(rd(self.lo)), X(ru(self.hi)))

    def refs(self):
        """(lo, hi) of the reference: the doubles around the exact bounds; None for the empty set"""
        if self.empty:
            return None
        return snap_bounds(self.lo)[0], snap_bounds(self.hi)[1]


def iv(a, b):
    """Interval(a, b) of IBEX: the empty set when a is +oo, b is -oo or a > b"""
    if a == INF or b == -INF or a > b:
        return Itv()
    return Itv(X(a), X(b))


def pt(a):
    """Interval(a) of IBEX: the empty set for an infinite a"""
    return Itv() if math.isinf(a) else iv(a, a)


EMPTY = Itv()
ALL = iv(-INF, INF)
POS = iv(0.0, INF)
NEG = iv(-INF, 0.0)
ZERO = iv(0.0, 0.0)
ONE = iv(1.0, 1.0)

PI_Q = mpq(mpmath.pi)
PI_LO, PI_HI = rd(PI_Q), ru(PI_Q)
PI = Itv(X(PI_LO), X(PI_HI))
TWO_PI = Itv(X(2 * PI_LO), X(2 * PI_HI))
HALF_PI = Itv(X(PI_LO / 2), X(PI_HI / 2))


def D(x):
    """A double that the Fraction x is exactly"""
    f = float(x)
    assert Fraction(f) == x, x
    return f


#---------------------------------------------------------------------------
# The double operations of the inputs of IBEX's tests, rounding upward as IBEX
# does (GAOL leaves the rounding upward)
#---------------------------------------------------------------------------
def up(q):
    return ru(q)


def mul_up(a, b):
    return up(Fraction(a) * Fraction(b))


def add_up(a, b):
    return up(Fraction(a) + Fraction(b))


def sub_up(a, b):
    return up(Fraction(a) - Fraction(b))


def div_up(a, b):
    return up(Fraction(a) / Fraction(b))


#---------------------------------------------------------------------------
# The exact functions
#---------------------------------------------------------------------------
def mp_value(f, q):
    """f(q) for a finite Fraction q, as an extended number"""
    return mpq(f(mpf_of(q)))


def monotone(x, f, lim_lo, lim_hi, lo_domain=None, strict=False):
    """The hull of f over x for an increasing f, with the limits lim_lo and lim_hi at -oo and +oo;
    lo_domain: f is defined on [lo_domain, +oo) (open with strict), its limit at lo_domain is lim_lo"""
    if x.empty:
        return x
    lo, hi = x.lo, x.hi
    if lo_domain is not None:
        if hi < lo_domain or (strict and hi <= lo_domain):
            return Itv()
        if lo < lo_domain or (strict and lo <= lo_domain):
            lo = lo_domain
            low = lim_lo
        else:
            low = None
    else:
        low = None
    if low is None:
        low = lim_lo if lo == -INF else (lim_hi if lo == INF else mp_value(f, lo))
    high = lim_hi if hi == INF else (lim_lo if hi == -INF else mp_value(f, hi))
    return Itv(low, high)


def f_exp(x):
    return monotone(x, mpmath.exp, Fraction(0), INF)


def f_log(x):
    return monotone(x, mpmath.log, -INF, INF, lo_domain=Fraction(0), strict=True)


def f_sqrt(x):
    return monotone(x, mpmath.sqrt, Fraction(0), INF, lo_domain=Fraction(0))


def f_sinh(x):
    return monotone(x, mpmath.sinh, -INF, INF)


def f_cosh(x):
    if x.empty:
        return x
    a, b = x.lo, x.hi
    cosh = lambda v: INF if is_inf(v) else mp_value(mpmath.cosh, v)
    if a >= 0:
        return Itv(cosh(a), cosh(b))
    if b <= 0:
        return Itv(cosh(b), cosh(a))
    return Itv(Fraction(1), max(cosh(a), cosh(b)))


def f_floor(x):
    return x if x.empty else Itv(*[v if is_inf(v) else Fraction(math.floor(v)) for v in (x.lo, x.hi)])


def f_ceil(x):
    return x if x.empty else Itv(*[v if is_inf(v) else Fraction(math.ceil(v)) for v in (x.lo, x.hi)])


def f_integer(x):
    """The integers of x: [ceil(lo), floor(hi)]"""
    if x.empty:
        return x
    r = Itv(f_ceil(Itv(x.lo, x.lo)).lo, f_floor(Itv(x.hi, x.hi)).hi)
    return Itv() if r.lo > r.hi else r


def _critical(a, b, offset, period):
    """Whether offset + k*period, an integer k, lies in [a, b] (finite Fractions)"""
    ma, mb = mpf_of(a), mpf_of(b)
    return mpmath.ceil((ma - offset) / period) <= mpmath.floor((mb - offset) / period)


def f_sincos(x, cosine):
    """The hull of sin (or cos) over x"""
    if x.empty:
        return x
    if is_inf(x.lo) or is_inf(x.hi):
        return Itv(Fraction(-1), Fraction(1))
    f = mpmath.cos if cosine else mpmath.sin
    a, b = x.lo, x.hi
    values = [mp_value(f, a), mp_value(f, b)]
    shift = 0 if cosine else mpmath.pi / 2
    if _critical(a, b, shift, 2 * mpmath.pi):
        values.append(Fraction(1))
    if _critical(a, b, shift + mpmath.pi, 2 * mpmath.pi):
        values.append(Fraction(-1))
    return Itv(min(values), max(values))


def f_tan(x):
    if x.empty:
        return x
    if is_inf(x.lo) or is_inf(x.hi) or _critical(x.lo, x.hi, mpmath.pi / 2, mpmath.pi):
        return Itv(-INF, INF)
    return Itv(mp_value(mpmath.tan, x.lo), mp_value(mpmath.tan, x.hi))


def f_atan2(y, x):
    """The hull of the angles atan2(y, x), in (-pi, pi], of the points of the box y x x, (0, 0) left out;
    the angle of the half-line y = 0, x < 0 is pi, and -pi is approached from below it"""
    if y.empty or x.empty:
        return Itv()

    def finite(v):
        return (BIG if v > 0 else -BIG) if is_inf(v) else v
    xl, xu, yl, yu = finite(x.lo), finite(x.hi), finite(y.lo), finite(y.hi)
    pi = mpq(mpmath.pi)
    half = mpq(mpmath.pi / 2)
    c = []
    for a in (xl, xu):
        for b in (yl, yu):
            if (a, b) != (0, 0):
                c.append(mpq(mpmath.atan2(mpf_of(Fraction(b)), mpf_of(Fraction(a)))))
    if yl <= 0 <= yu and xu > 0:
        c.append(Fraction(0))
    if xl <= 0 <= xu and yu > 0:
        c.append(half)
    if xl <= 0 <= xu and yl < 0:
        c.append(-half)
    if yl <= 0 <= yu and xl < 0:
        c.append(pi)
    if yl < 0 <= yu and xl < 0:
        c.append(-pi)
    return Itv(min(c), max(c)) if c else Itv()


def _root_base(m, odd):
    """v -> v^(1/m) for the root of order m >= 1, increasing on its domain"""
    def g(v):
        if v == 0:
            return Fraction(0)
        r = mpmath.root(mpf_of(abs(v)), m)
        q = mpq(r)
        return -q if v < 0 else q
    return g


def f_pown(x, n):
    """The hull of x^n for an integer n, the pown of IEEE 1788: n < 0 leaves 0 out"""
    if x.empty:
        return x
    if n == 0:
        return Itv(Fraction(1), Fraction(1))
    m = abs(n)

    def p(v):
        if is_inf(v):
            return INF if (v > 0 or m % 2 == 0) else -INF
        return v ** m

    def image(a, b):    # x^m over [a, b]
        if m % 2 == 1:
            return Itv(p(a), p(b))
        if a >= 0:
            return Itv(p(a), p(b))
        if b <= 0:
            return Itv(p(b), p(a))
        return Itv(Fraction(0), max(p(a), p(b)))
    if n > 0:
        return image(x.lo, x.hi)
    parts = []
    if x.lo < 0:
        parts.append(image(x.lo, min(x.hi, Fraction(0))))
    if x.hi > 0:
        parts.append(image(max(x.lo, Fraction(0)), x.hi))
    if not parts:
        return Itv()
    one = Itv(Fraction(1), Fraction(1))
    result = Itv()
    for part in parts:
        result = result.hull(one.div(part))
    return result


def f_root(x, n):
    """rootn(x, n) of IEEE 1788: the real root of order n, 1/x^(1/|n|) for n < 0"""
    if x.empty:
        return x
    m = abs(n)
    odd = m % 2 == 1
    g = _root_base(m, odd)

    def root(v):
        return v if is_inf(v) else g(v)
    dom = x if odd else x.inter(Itv(Fraction(0), INF))
    if dom.empty:
        return dom
    base = Itv(root(dom.lo), root(dom.hi))
    if n > 0:
        return base
    parts = []
    if base.lo < 0:
        parts.append(Itv(base.lo, min(base.hi, Fraction(0))))
    if base.hi > 0:
        parts.append(Itv(max(base.lo, Fraction(0)), base.hi))
    if not parts:
        return Itv()
    one = Itv(Fraction(1), Fraction(1))
    result = Itv()
    for part in parts:
        result = result.hull(one.div(part))
    return result


#---------------------------------------------------------------------------
# C++ text
#---------------------------------------------------------------------------
def dec(d):
    """A double as a decimal literal, the shortest the compiler reads back as d: '0.1', '1e-10', '5e-324'"""
    if math.isinf(d):
        return 'ibex_oo' if d > 0 else '-ibex_oo'
    if d == 0.0:
        return '0.0'
    return repr(d)


def ref_text(refs):
    """{ lo, hi } for the bounds of a reference, ibex_empty for the empty set"""
    if refs is None:
        return 'ibex_empty'
    return '{ %s, %s }' % (dec(refs[0]), dec(refs[1]))


def itv_ref(i):
    """The reference of a given interval of doubles (an input, or an interval IBEX expects)"""
    return None if i.empty else (D(i.lo) if not is_inf(i.lo) else i.lo, D(i.hi) if not is_inf(i.hi) else i.hi)


def real_ref(q):
    """The reference of a real result: the doubles around the exact value q (an extended number)"""
    return snap_bounds(q)


#---------------------------------------------------------------------------
# The cases. Each group builds its rows and the cross-checks with IBEX's expectations
#---------------------------------------------------------------------------
def split_cells(inner):
    """The cells of a row, split at the commas outside braces and quotes"""
    cells, depth, cur, quote = [], 0, '', False
    for ch in inner:
        if quote:
            quote = ch != '"'
        elif ch == '"':
            quote = True
        elif ch in '{(':
            depth += 1
        elif ch in '})':
            depth -= 1
        elif ch == ',' and depth == 0:
            cells.append(cur.strip())
            cur = ''
            continue
        cur += ch
    cells.append(cur.strip())
    return cells


def wrap(text, indent, width, trailer=','):
    """The lines of a row (or of a cell that is a list) of at most width columns, breaking between cells"""
    if len(indent) + len(text) + len(trailer) <= width or not text.startswith('{ '):
        return [indent + text + trailer]
    cells = split_cells(text[2:-2])
    lines, cont = [], indent + '  '
    cur = indent + '{ '
    for i, cell in enumerate(cells):
        last = i == len(cells) - 1
        sep = ' }' + trailer if last else ','
        piece = cell + sep
        if len(cur) + len(piece) <= width:
            cur += piece + ('' if last else ' ')
            continue
        if cur.strip() != '{':
            lines.append(cur.rstrip())
            cur = cont
        if len(cont) + len(piece) > width and cell.startswith('{ '):
            sub = wrap(cell, cont, width, sep)
            lines.extend(sub[:-1])
            cur = sub[-1] + ('' if last else ' ')
        else:
            cur += piece + ('' if last else ' ')
    lines.append(cur.rstrip())
    return lines


class Table:
    def __init__(self, name, struct, fields, comment):
        self.name, self.struct, self.fields, self.comment = name, struct, fields, comment
        self.rows = []

    def add(self, text):
        self.rows.append(text)

    def text(self):
        out = ['  // BEGIN %s (written by ibex_forward_cases.py: do not edit)' % self.name]
        out.append('  static const %s %s[] = {' % (self.struct, self.name))
        for r in self.rows:
            out.extend(wrap(r, '    ', 120))
        out.append('  };')
        out.append('  // END %s' % self.name)
        return '\n'.join(out)


TABLES = []
FINDINGS = []     # (case, text): a disagreement of IBEX's expectation with the exact result
CHECKS = [0]      # the number of IBEX's checks compared
ORIGINAL_CHECKS = {}   # per IBEX test, its checks
NOTES = []        # (case, text): an expectation of IBEX within the tolerance of the exact result, but not enclosing it


def table(name, struct, fields, comment=''):
    t = Table(name, struct, fields, comment)
    TABLES.append(t)
    return t


def row(*cells):
    return '{ ' + ', '.join(cells) + ' }'


def quoted(name):
    return '"' + name + '"'


def agree_value(orig, exact, tol):
    """Whether the number orig (a double, or an infinity) is the exact value within tol, or one of the two
    doubles around it (the bound of the doubles IBEX expects of an operation that rounds, which the
    tolerance cannot tell from the exact value past 2^-33 or so)"""
    if is_inf(exact):
        return orig == exact
    if orig in snap_bounds(exact):
        return True
    return not math.isinf(orig) and abs(Fraction(orig) - exact) <= Fraction(tol)


def tally(case, times):
    ORIGINAL_CHECKS[case] = ORIGINAL_CHECKS.get(case, 0) + times
    CHECKS[0] += times


def compare_interval(case, what, orig, exact, tol=ERROR, times=1):
    """The interval IBEX expects against the exact one; times: the number of checks of IBEX it stands for"""
    tally(case, times)
    if orig.empty or exact.empty:
        if orig.empty != exact.empty:
            FINDINGS.append((case, '%s: IBEX expects %s, the exact result is %s' % (what, fmt(orig), fmt(exact))))
        return
    for which, o, e in (('lower', orig.lo, exact.lo), ('upper', orig.hi, exact.hi)):
        if not agree_value(float(o) if not is_inf(o) else o, e, tol):
            FINDINGS.append((case, '%s: IBEX expects the %s bound of %s, the exact result is %s' % (what, which, fmt(orig), fmt(exact))))
            return
    # within the tolerance, but not an enclosure at an integer bound (0, 1...) that IBEX writes as exact
    for o, e, inside in ((orig.lo, exact.lo, orig.lo > rd(exact.lo)), (orig.hi, exact.hi, orig.hi < ru(exact.hi))):
        if tol > 0 and inside and not is_inf(o) and o == math.floor(o):
            NOTES.append((case, '%s: IBEX expects %s, whose bound %s is not an enclosure of the exact result %s'
                          % (what, fmt(orig), fnum(o), fmt(exact))))


def compare_number(case, what, orig, exact, tol=ERROR, times=1):
    tally(case, times)
    if not agree_value(orig, exact, tol):
        FINDINGS.append((case, '%s: IBEX expects %r, the exact value is %s' % (what, orig, fnum(exact))))


def compare_bool(case, what, orig, exact, times=1):
    tally(case, times)
    if orig != exact:
        FINDINGS.append((case, '%s: IBEX expects %s, the exact truth value is %s' % (what, orig, exact)))


def compare_predicate(case, what, ok, times=1):
    tally(case, times)
    if not ok:
        FINDINGS.append((case, '%s: the exact result does not satisfy it' % what))


def fnum(q):
    if is_inf(q):
        return str(q)
    return '%.17g' % float(q) if abs(q) < Fraction(DBL_MAX) else '%s(beyond the doubles)' % ('+' if q > 0 else '-')


def fmt(i):
    return 'empty' if i.empty else '[%s, %s]' % (fnum(i.lo), fnum(i.hi))


def itv_cell(i):
    return ref_text(itv_ref(i))


def ref_cell(i):
    """The reference of an exact result: the doubles around its bounds"""
    return ref_text(i.refs())


def next_float(d):
    """The least double above d (math.nextafter(d, inf) from Python 3.9)"""
    if d != d or d == INF:
        return d
    if d == 0.0:
        return TINY
    (bits,) = struct.unpack('<Q', struct.pack('<d', d))
    bits = bits + 1 if d > 0.0 else bits - 1
    return struct.unpack('<d', struct.pack('<Q', bits))[0]


def previous_float(d):
    return -next_float(-d)


#---- TestInterval ---------------------------------------------------------
def group_constructors():
    t = table('ibex_construction_cases', 'ibex_construction_case', 'name, form, a, b, result')
    # form: 'd' interval(), 's' interval(a), 'p' interval(a, b)
    cases = [
        ('cons01', 'd', 0.0, 0.0, ALL, 2),
        ('cons02', 's', 1.0, 0.0, ONE, 2),
        ('cons03', 's', -INF, 0.0, EMPTY, 2),
        ('cons04', 's', INF, 0.0, EMPTY, 2),
        ('cons05', 'p', 1.0, 0.0, EMPTY, 2),
    ]
    for name, form, a, b, orig, _ in cases:
        if form == 'd':
            exact = ALL
        elif form == 's':
            exact = Itv() if math.isinf(a) else iv(a, a)
        else:
            exact = Itv() if (a == INF or b == -INF or a > b) else iv(a, b)
        compare_interval('IBEX TestInterval::' + name, 'constructor', orig, exact)
        compare_interval('IBEX TestInterval::' + name, 'assignment', orig, exact)
        t.add(row(quoted('IBEX TestInterval::' + name), "'%s'" % form, dec(a), dec(b), ref_cell(exact)))


def group_equality():
    t = table('ibex_equality_cases', 'ibex_equality_case', 'name, x, y, equal')
    cases = [('eq01', ALL, ALL, True), ('eq02', ALL, POS, False), ('eq03', iv(0, 1), iv(0, 1), True),
             ('eq04', iv(0, 2), iv(0, 1), False), ('eq05', iv(0, 2), iv(1, 3), False), ('eq06', POS, POS, True),
             ('eq07', EMPTY, EMPTY, True), ('eq08', iv(0, 0), iv(0, 0), True)]
    for name, x, y, orig in cases:
        exact = (x.empty and y.empty) or (not x.empty and not y.empty and x.lo == y.lo and x.hi == y.hi)
        compare_bool('IBEX TestInterval::' + name, 'x == y', orig, exact, times=4)
        t.add(row(quoted('IBEX TestInterval::' + name), itv_cell(x), itv_cell(y), 'true' if exact else 'false'))


def group_set_operations():
    t = table('ibex_set_operation_cases', 'ibex_set_operation_case', 'name, operation, x, z, result')
    hulls = [('hull01', iv(0, 1), iv(2, 3), iv(0, 3)), ('hull02', iv(0, 1), EMPTY, iv(0, 1)),
             ('hull03', pt(-1), POS, iv(-1, INF))]
    inters = [('inter01', iv(0, 1), iv(2, 3), EMPTY), ('inter02', iv(0, 1), EMPTY, EMPTY),
              ('inter03', iv(0, 2), iv(-1, 1), iv(0, 1)), ('inter04', iv(0, 1), POS, iv(0, 1)),
              ('inter05', iv(0, 1), NEG, ZERO), ('inter06', iv(0, 1), iv(1, 2), iv(1, 1))]
    for op, cases in (('|', hulls), ('&', inters)):
        for name, x, z, orig in cases:
            exact = x.hull(z) if op == '|' else x.inter(z)
            compare_interval('IBEX TestInterval::' + name, 'x %s z' % op, orig, exact, 0.0, times=4)
            t.add(row(quoted('IBEX TestInterval::' + name), "'%s'" % op, itv_cell(x), itv_cell(z), ref_cell(exact)))


def midpoint_of(x):
    """The midpoint of an interval, as an extended number or the convention for the unbounded ones (the
    same in IBEX and GAOL: [-oo, b] -> -DBL_MAX, [a, +oo] -> DBL_MAX, [-oo, +oo] -> 0)"""
    if x.lo == -INF:
        return Fraction(0) if x.hi == INF else -Fraction(DBL_MAX)
    if x.hi == INF:
        return Fraction(DBL_MAX)
    return (x.lo + x.hi) / 2


def radius_of(x):
    """rad of IEEE 1788: the smallest r with x inside [m - r, m + r], m the midpoint rounded to nearest"""
    if is_inf(x.lo) or is_inf(x.hi):
        return INF
    m = Fraction(nearest(midpoint_of(x)))
    return max(m - x.lo, x.hi - m)


def width_of(x):
    return INF if (is_inf(x.lo) or is_inf(x.hi)) else x.hi - x.lo


def mignitude(x):
    return x.lo if x.lo > 0 else (-x.hi if x.hi < 0 else Fraction(0))


def magnitude(x):
    return INF if (is_inf(x.lo) or is_inf(x.hi)) else max(abs(x.lo), abs(x.hi))


def group_getters():
    t = table('ibex_getter_cases', 'ibex_getter_case', 'name, x, lb, ub, mid, rad, diam, mig, mag')
    oo, dm = INF, DBL_MAX
    # lb, ub, mid, rad, diam, mig, mag as the IBEX tests expect them
    cases = [('getters01', iv(0, 2), (0, 2, 1, 1, 2, 0, 2)),
             ('getters02', iv(-3, -1), (-3, -1, -2, 1, 2, 1, 3)),
             ('getters03', iv(-3, 1), (-3, 1, -1, 2, 4, 0, 3)),
             ('getters04', NEG, (-oo, 0, -dm, oo, oo, 0, oo)),
             ('getters05', ALL, (-oo, oo, 0, oo, oo, 0, oo)),
             ('getters06', iv(dm, INF), (dm, oo, dm, oo, oo, dm, oo))]
    for name, x, orig in cases:
        case = 'IBEX TestInterval::' + name
        exact = [x.lo, x.hi, midpoint_of(x), radius_of(x), width_of(x), mignitude(x), magnitude(x)]
        for what, o, e in zip(('lb', 'ub', 'mid', 'rad', 'diam', 'mig', 'mag'), orig, exact):
            compare_number(case, what, float(o), e)
        lb, ub = D(x.lo) if not is_inf(x.lo) else x.lo, D(x.hi) if not is_inf(x.hi) else x.hi
        mig, mag = real_ref(exact[5]), real_ref(exact[6])
        assert mig[0] == mig[1] and mag[0] == mag[1]
        t.add(row(quoted(case), itv_cell(x), dec(lb), dec(ub), ref_text(real_ref(exact[2])),
                  ref_text(real_ref(exact[3])), ref_text(real_ref(exact[4])), dec(mig[0]), dec(mag[0])))


def group_midpoints():
    t = table('ibex_midpoint_cases', 'ibex_midpoint_case', 'name, x, mid')
    dm = DBL_MAX
    cases = [('mid01', iv(0, 2), [1.0]), ('mid02', iv(-INF, INF), [0.0]), ('mid03', POS, [dm]),
             ('mid04', NEG, [-dm]), ('mid05', iv(dm, INF), [dm, -dm]), ('mid06', ZERO, [0.0])]
    for name, x, accepted in cases:
        case = 'IBEX TestInterval::' + name
        exact = midpoint_of(x)
        compare_predicate(case, 'mid() is one of %s' % accepted, any(Fraction(a) == exact for a in accepted))
        t.add(row(quoted(case), itv_cell(x), ref_text(real_ref(exact))))
    # mid07 and mid08: (x + m) - m contains x, m the midpoint of x
    t = table('ibex_midpoint_roundtrip_cases', 'ibex_midpoint_case', 'name, x, mid')
    boxes = [(2.7629416257693772963, 2.7629416257693835135, 'mid07')] + [(a, b, 'mid08 [%d]' % i) for i, (a, b) in enumerate([
        (3.4836061062102725572, 3.4836061062102761099), (3.5754039600153961764, 3.5754039600154006173),
        (3.2401050837195057852, 3.2401050837195093379), (3.5187713541601914891, 3.5187713541601937095),
        (3.4109008703180077049, 3.410900870318013034), (3.336209585763303842, 3.3362095857633069507),
        (3.3970878856415889224, 3.3970878856415973601), (3.0865914745327103219, 3.0865914745327134305),
        (3.1673998478272404888, 3.1673998478272467061), (3.2795769799080982665, 3.2795769799081013751),
        (2.9966040612287390132, 2.996604061228742566), (3.4621387483049854694, 3.4621387483049890221),
        (0.39999999999999980016, 0.40000000000000074385), (1.9595917942265419498, 1.959591794226542838),
        (2.4033959387712586775, 2.4033959387712604538), (-1.5025469540784459621, -1.5025469540784437417),
        (3.4474903944979735648, 3.4474903944979780057), (2.8417250066344248438, 2.8417250066344270643),
        (3.6343497218046629804, 3.6343497218046700858), (3.0078080936676636625, 3.0078080936676689916),
        (3.8716115579677374825, 3.8716115579677432557), (2.9290247128026156709, 2.929024712802626329),
        (3.6847522306610471787, 3.6847522306610511755), (2.7629416257693772963, 2.7629416257693835135),
        (3.4978929033543559868, 3.4978929033543604277), (2.5968585387361389216, 2.5968585387361411421),
        (5, 5), (0, 0), (0, 0), (0, 0)])]
    for a, b, name in boxes:
        case = 'IBEX TestInterval::' + name.split(' ')[0]
        x = iv(float(a), float(b))
        m = nearest(midpoint_of(x))
        # x2 = x + m and x3 = x2 - m, with the outward roundings of GAOL: x3 contains x
        x3 = x.add(pt(m)).out().sub(pt(m)).out()
        compare_bool(case, 'x3 contains x', True, x3.lo <= x.lo and x3.hi >= x.hi,
                     times=1 if name in ('mid07', 'mid08 [0]') else 0)
        t.add(row(quoted('IBEX TestInterval::' + name), itv_cell(x), ref_text(real_ref(midpoint_of(x)))))


def box_relation(rel, x, y=None, d=None):
    """The exact truth value of the relation, from the definition of the sets"""
    if rel == 'subset':
        return x.empty or (not y.empty and y.lo <= x.lo and x.hi <= y.hi)
    if rel == 'superset':
        return box_relation('subset', y, x)
    if rel == 'interior':       # x inside the interior of y
        return x.empty or (not y.empty and (y.lo == -INF or y.lo < x.lo) and (y.hi == INF or x.hi < y.hi))
    same = (x.empty and y.empty) or (not x.empty and not y.empty and x.lo == y.lo and x.hi == y.hi) if y is not None else None
    if rel == 'strict_interior_subset':
        return box_relation('interior', x, y) and not same
    if rel == 'strict_superset':
        return box_relation('subset', y, x) and not same
    if rel == 'disjoint':
        return x.empty or y.empty or x.lo > y.hi or x.hi < y.lo
    if rel == 'contains':
        return not x.empty and x.lo <= d <= x.hi
    if rel == 'interior_contains':
        return not x.empty and x.lo < d < x.hi
    if rel == 'empty':
        return x.empty
    if rel == 'degenerated':
        return x.empty or x.lo == x.hi
    if rel == 'unbounded':
        return not x.empty and (x.lo == -INF or x.hi == INF)
    if rel == 'bisectable':
        # a double lies strictly between the bounds (the infinities next to the largest doubles)
        if x.empty:
            return False

        return ordinal(float(x.hi)) - ordinal(float(x.lo)) >= 2
    raise ValueError(rel)


def ordinal(d):
    """A monotone integer image of the doubles, -0 and +0 alike, the infinities next to the largest doubles"""
    if d == 0.0:
        return 0
    (b,) = struct.unpack('<q', struct.pack('<d', d))
    return b if b >= 0 else -(b & 0x7fffffffffffffff)


def group_relations():
    t1 = table('ibex_relation_cases', 'ibex_relation_case', 'name, relation, x, y, result')
    t2 = table('ibex_point_relation_cases', 'ibex_point_relation_case', 'name, relation, x, d, result')
    t3 = table('ibex_predicate_cases', 'ibex_predicate_case', 'name, predicate, x, result')
    dm = DBL_MAX
    # IBEX's relation -> the GAOL expression in the C++ (the key of the table)
    two = {'subset': 'set_leq', 'superset': 'set_geq', 'strict_interior_subset': 'set_le_and_neq',
           'strict_superset': 'set_geq_and_neq', 'disjoint': 'set_disjoint'}
    cases2 = [('is_subset01', 'subset', iv(0, 1), iv(0, 2), True), ('is_subset02', 'subset', iv(0, 1), iv(1, 2), False),
              ('is_strict_interior_subset01', 'strict_interior_subset', iv(0, 1), iv(0, 2), False),
              ('is_strict_interior_subset02', 'strict_interior_subset', NEG, ALL, True),
              ('is_strict_interior_subset03', 'strict_interior_subset', EMPTY, iv(0, 1), True),
              ('is_superset01', 'superset', iv(0, 2), iv(0, 1), True), ('is_superset02', 'superset', iv(1, 2), iv(0, 1), False),
              ('is_strict_superset01', 'strict_superset', iv(0, 2), iv(0, 2), False),
              ('is_strict_superset02', 'strict_superset', ALL, NEG, True),
              ('is_strict_superset03', 'strict_superset', EMPTY, EMPTY, False),
              ('is_disjoint01', 'disjoint', NEG, POS, False), ('is_disjoint02', 'disjoint', iv(0, 1), iv(2, 3), True)]
    for name, rel, x, y, orig in cases2:
        exact = box_relation(rel, x, y)
        compare_bool('IBEX TestInterval::' + name, rel, orig, exact)
        t1.add(row(quoted('IBEX TestInterval::' + name), quoted(two[rel]), itv_cell(x), itv_cell(y), 'true' if exact else 'false'))
    pts = {'contains': 'set_contains', 'interior_contains': 'set_strictly_contains'}
    cases3 = [('contains01', 'contains', iv(0, 2), 0.0, True), ('contains02', 'contains', ALL, INF, True),
              ('interior_contains01', 'interior_contains', iv(0, 2), 0.0, False),
              ('interior_contains02', 'interior_contains', iv(0, 2), 1.0, True)]
    for name, rel, x, d, orig in cases3:
        exact = box_relation(rel, x, d=d)
        compare_bool('IBEX TestInterval::' + name, rel, orig, exact)
        t2.add(row(quoted('IBEX TestInterval::' + name), quoted(pts[rel]), itv_cell(x), dec(d), 'true' if exact else 'false'))
    preds = {'empty': 'is_empty', 'degenerated': 'is_empty_or_a_double', 'unbounded': 'is_not_finite',
             'bisectable': 'is_bisectable'}
    cases1 = [('is_empty01', 'empty', EMPTY, True), ('is_empty02', 'empty', iv(0, 1), False),
              ('is_degenerated01', 'degenerated', ZERO, True), ('is_degenerated02', 'degenerated', EMPTY, True),
              ('is_unbounded01', 'unbounded', EMPTY, False), ('is_unbounded02', 'unbounded', iv(0, 1), False),
              ('is_unbounded03', 'unbounded', NEG, True), ('is_unbounded04', 'unbounded', POS, True),
              ('is_bisectable01', 'bisectable', EMPTY, False), ('is_bisectable02', 'bisectable', iv(dm, INF), False),
              ('is_bisectable03', 'bisectable', iv(-INF, -dm), False),
              ('is_bisectable04', 'bisectable', iv(0, next_float(0.0)), False),
              ('is_bisectable05', 'bisectable', iv(0, next_float(next_float(0.0))), True),
              ('is_bisectable06', 'bisectable', iv(10, next_float(next_float(10.0))), True),
              ('is_bisectable07', 'bisectable', iv(previous_float(previous_float(0.0)), 0), True),
              ('is_bisectable08', 'bisectable', iv(previous_float(previous_float(10.0)), 10), True),
              ('is_bisectable09', 'bisectable', iv(10, next_float(10.0)), False),
              ('is_bisectable10', 'bisectable', iv(previous_float(0.0), 0), False)]
    for name, rel, x, orig in cases1:
        exact = box_relation(rel, x)
        compare_bool('IBEX TestInterval::' + name, rel, orig, exact)
        t3.add(row(quoted('IBEX TestInterval::' + name), quoted(preds[rel]), itv_cell(x), 'true' if exact else 'false'))


#---- TestArith ------------------------------------------------------------
def group_floats():
    t = table('ibex_float_cases', 'ibex_float_case', 'name, direction, x, result')
    cases = [('float01', 'n', 0.0, lambda v: 0 < v), ('float02', 'p', 0.0, lambda v: 0 > v),
             ('float03', 'n', 1.0, lambda v: 1 < v), ('float04', 'p', 1.0, lambda v: 1 > v),
             ('float05', 'n', INF, lambda v: INF == v), ('float06', 'n', -INF, lambda v: -DBL_MAX == v),
             ('float07', 'p', INF, lambda v: DBL_MAX == v), ('float08', 'p', -INF, lambda v: -INF == v),
             ('float09', 'n', DBL_MAX, lambda v: INF == v),
             # float10 is in a comment of TestArith.h ("disabled temporarily due to bug between Gaol and elibc"): not run in IBEX
             ('float10', 'p', -DBL_MAX, lambda v: -INF == v)]
    for name, direction, x, predicate in cases:
        v = next_float(x) if direction == 'n' else previous_float(x)
        compare_predicate('IBEX TestArith::' + name, 'the relation to next_float or previous_float', predicate(v))
        t.add(row(quoted('IBEX TestArith::' + name), "'%s'" % direction, dec(x), ref_text((v, v))))


def group_binary():
    t = table('ibex_binary_cases', 'ibex_binary_case', 'name, operation, x, z, scalar, result')
    dm = DBL_MAX
    # (name, x, z (an interval, or a double for the scalar forms), IBEX's result)
    add = [('add01', EMPTY, iv(0, 1), EMPTY), ('add02', iv(0, 1), EMPTY, EMPTY),
           ('add03', iv(-INF, 1), iv(0, 1), iv(-INF, 2)), ('add04', iv(1, INF), iv(0, 1), iv(1, INF)),
           ('add05', ALL, iv(0, 1), ALL), ('add06', EMPTY, INF, EMPTY), ('add07', EMPTY, 0.0, EMPTY),
           ('add08', iv(0, 1), 1.0, iv(1, 2)), ('add09', iv(0, 1), -INF, EMPTY), ('add10', iv(0, 1), INF, EMPTY),
           ('add11', iv(-INF, 1), 1.0, iv(-INF, 2)),
           ('add12', iv(dm, INF), pt(1.0), iv(dm, INF)), ('add13', iv(dm, INF), pt(-1.0), iv(previous_float(dm), INF)),
           ('add14', iv(dm, INF), iv(dm, INF), iv(dm, INF)), ('add15', iv(dm, INF), pt(-INF), EMPTY),
           ('add16', iv(dm, INF), pt(INF), EMPTY), ('add17', iv(-INF, -dm), pt(1.0), iv(-INF, next_float(-dm))),
           ('add18', iv(-INF, -dm), pt(-1.0), iv(-INF, -dm)), ('add19', iv(-INF, -dm), iv(-INF, -dm), iv(-INF, -dm))]
    mul = [('mul01', EMPTY, iv(0, 1), EMPTY), ('mul02', ZERO, ALL, ZERO), ('mul03', iv(-1, 1), NEG, ALL),
           ('mul04', iv(-INF, -1), iv(-1, 0), POS), ('mul05', iv(-INF, 1), iv(-1, 0), iv(-1, INF)),
           ('mul06', iv(0, 1), iv(1, INF), POS), ('mul07', iv(0, 1), iv(-1, INF), iv(-1, INF)),
           ('mul08', iv(-INF, -1), iv(0, 1), NEG), ('mul09', iv(-INF, 1), iv(0, 1), iv(-INF, 1)),
           ('mul10', iv(0, 1), iv(-INF, -1), NEG), ('mul11', iv(0, 1), iv(-INF, 1), iv(-INF, 1)),
           ('mul12', iv(1, INF), iv(0, 1), POS), ('mul13', iv(-1, INF), iv(0, 1), iv(-1, INF)),
           ('mul14', iv(1, 2), iv(1, 2), iv(1, 4)), ('mul15', iv(1, 2), iv(-2, 3), iv(-4, 6)),
           ('mul16', iv(-1, 1), iv(-2, 3), iv(-3, 3)), ('mul17', iv(1, 2), -INF, EMPTY),
           ('mul18', iv(1, 2), INF, EMPTY), ('mul19', iv(1, 2), -1.0, iv(-2, -1))]
    div = [('div01', EMPTY, iv(0, 1), EMPTY), ('div02', ZERO, ZERO, EMPTY), ('div03', iv(1, 2), ZERO, EMPTY),
           ('div04', ALL, ZERO, EMPTY), ('div05', ZERO, iv(0, 1), ZERO), ('div06', ZERO, ALL, ZERO),
           ('div07', iv(6, 12), iv(2, 3), iv(2, 6)), ('div08', iv(6, 12), iv(-3, -2), iv(-6, -2)),
           ('div09', iv(6, 12), iv(-2, 3), ALL), ('div10', iv(-INF, -1), iv(-1, 0), iv(1, INF)),
           ('div11', iv(-INF, -1), iv(0, 1), iv(-INF, -1)), ('div12', iv(1, INF), iv(-1, 0), iv(-INF, -1)),
           ('div13', iv(1, INF), iv(0, 1), iv(1, INF)), ('div14', iv(-1, 1), iv(-1, 1), ALL),
           ('div15', iv(1, 2), -INF, EMPTY), ('div16', iv(1, 2), INF, EMPTY), ('div17', iv(1, 2), -1.0, iv(-2, -1))]
    # the number of checks of IBEX's helper: check_add 8, check_add_scal 6, check_mul 3, check_mul_scal 3,
    # check_div 2, check_div_scal 2
    checks = {('+', False): 8, ('+', True): 6, ('*', False): 3, ('*', True): 3, ('/', False): 2, ('/', True): 2}
    for op, cases in (('+', add), ('*', mul), ('/', div)):
        for name, x, z, orig in cases:
            scalar = isinstance(z, float)
            zi = pt(z) if scalar else z
            exact = {'+': x.add, '*': x.mul, '/': x.div}[op](zi)
            compare_interval('IBEX TestArith::' + name, 'x %s z' % op, orig, exact, times=checks[(op, scalar)])
            zcell = ref_text((z, z)) if scalar else itv_cell(z)
            t.add(row(quoted('IBEX TestArith::' + name), "'%s'" % op, itv_cell(x), zcell,
                      'true' if scalar else 'false', ref_cell(exact)))


def group_unary():
    t = table('ibex_unary_cases', 'ibex_unary_case', 'name, function, x, result')
    dm = DBL_MAX
    exact_of = {'neg': lambda x: x.neg(), 'sqrt': f_sqrt, 'log': f_log, 'exp': f_exp, 'floor': f_floor, 'ceil': f_ceil,
                'integer': f_integer, 'tan': f_tan}
    # (name, function, x, IBEX's result; or a function of the exact result that makes IBEX's checks; tolerance)
    cases = [('minus01', 'neg', iv(0, 1), iv(-1, 0)), ('minus02', 'neg', ALL, ALL), ('minus03', 'neg', NEG, POS),
             ('minus04', 'neg', iv(-INF, 1), iv(-1, INF)),
             ('sqrt01', 'sqrt', ALL, POS), ('sqrt02', 'sqrt', NEG, ZERO), ('sqrt03', 'sqrt', iv(-9, 4), iv(0, 2)),
             ('sqrt04', 'sqrt', iv(4, 9), iv(2, 3)), ('sqrt05', 'sqrt', iv(-9, -4), EMPTY),
             # sqrt06 is defined in TestArith.cpp and not registered in TestArith.h: not run in IBEX
             ('sqrt06', 'sqrt', iv(-9, INF), POS),
             ('log01', 'log', EMPTY, EMPTY), ('log02', 'log', ALL, ALL), ('log03', 'log', POS, ALL),
             ('log04', 'log', NEG, EMPTY), ('log05', 'log', iv(1, 2), iv(0, math.log(2))),
             ('log06', 'log', iv(-1, 1), iv(-INF, 0)),
             ('log07', 'log', iv(0, next_float(0.0)), lambda ex: [('ub() > -744.5', ex.hi > Fraction(-744.5))]),
             ('log08', 'log', iv(0, 1), iv(-INF, 0)), ('log09', 'log', iv(1, INF), POS),
             ('log10', 'log', pt(0.0), EMPTY), ('log11', 'log', iv(-2, -1), EMPTY),
             ('exp01', 'exp', EMPTY, EMPTY), ('exp02', 'exp', ALL, POS), ('exp03', 'exp', POS, iv(1, INF)),
             ('exp04', 'exp', NEG, iv(0, 1)), ('exp05', 'exp', iv(0, 2), iv(1, math.exp(2))),
             ('exp06', 'exp', iv(-1, 1), iv(math.exp(-1), math.exp(1))), ('exp07', 'exp', iv(1.e100, 1.e111), iv(dm, INF)),
             ('exp08', 'exp', iv(dm, INF), iv(dm, INF)), ('exp09', 'exp', iv(0, dm), iv(1, INF)),
             ('floor01', 'floor', EMPTY, EMPTY), ('floor02', 'floor', iv(-INF, -0.000001), iv(-INF, -1)),
             ('floor03', 'floor', iv(0.00000001, INF), POS), ('floor04', 'floor', ALL, ALL),
             ('floor05', 'floor', iv(0.01, 2.99), iv(0, 2)), ('floor06', 'floor', iv(-0.01, 2.99), iv(-1, 2)),
             ('floor07', 'floor', iv(1.000000001, 1.9999999999), ONE), ('floor08', 'floor', iv(1e8, dm), iv(1e8, dm)),
             ('ceil01', 'ceil', EMPTY, EMPTY), ('ceil02', 'ceil', iv(-INF, -0.000001), NEG),
             ('ceil03', 'ceil', iv(0.00000001, INF), iv(1.0, INF)), ('ceil04', 'ceil', ALL, ALL),
             ('ceil05', 'ceil', iv(0.01, 2.99), iv(1, 3)), ('ceil06', 'ceil', iv(-0.01, 2.99), iv(0, 3)),
             ('ceil07', 'ceil', iv(1.000000001, 1.9999999999), iv(2, 2)), ('ceil08', 'ceil', iv(1e8, dm), iv(1e8, dm)),
             ('integer01', 'integer', EMPTY, EMPTY), ('integer02', 'integer', iv(-INF, -0.000001), iv(-INF, -1)),
             ('integer03', 'integer', iv(0.00000001, INF), iv(1.0, INF)), ('integer04', 'integer', ALL, ALL),
             ('integer05', 'integer', iv(0.01, 2.99), iv(1, 2)), ('integer06', 'integer', iv(-0.01, 2.99), iv(0, 2)),
             ('integer07', 'integer', iv(1.000000001, 1.9999999999), EMPTY),
             ('integer08', 'integer', iv(1e8, dm), iv(1e8, dm)),
             ('tan01', 'tan', ALL, ALL)]
    # tan03 and tan04: the argument is an expression of the lower bound of pi, rounding upward as IBEX does
    below = mul_up(sub_up(1.0, 1e-10), PI_LO)
    tan03_x = iv(div_up(PI_LO, 4.0), div_up(below, 2.0))
    tan04_x = iv(div_up(mul_up(-sub_up(1.0, 1e-10), PI_LO), 2.0), div_up(PI_LO, 4.0))
    cases.append(('tan03', 'tan', tan03_x, lambda ex: [('y.lb() is 1', agree_value(1.0, ex.lo, ERROR)),
                                                        ('y.ub() > 1000', ex.hi > 1000)]))
    cases.append(('tan04', 'tan', tan04_x, lambda ex: [('y.lb() < -1000', ex.lo < -1000),
                                                        ('y.ub() is 1', agree_value(1.0, ex.hi, ERROR))]))
    cases.append(('tan_issue248', 'tan', iv(-1.57079632679489678, 1.1780972450961728626),
                  lambda ex: [('tan(itv) is not empty', not ex.empty)]))
    exact_tol = {'floor': 0.0, 'ceil': 0.0, 'integer': 0.0}
    for name, function, x, orig in cases:
        case = 'IBEX TestArith::' + name
        exact = exact_of[function](x)
        if callable(orig):
            for what, ok in orig(exact):
                compare_predicate(case, what, ok)
        else:
            compare_interval(case, function, orig, exact, exact_tol.get(function, ERROR))
        t.add(row(quoted(case), quoted(function), itv_cell(x), ref_cell(exact)))


def group_expressions():
    t = table('ibex_expression_cases', 'ibex_expression_case', 'name, expression, result')
    quarter = PI.div(pt(4.0)).out()
    arg02 = PI.neg().div(pt(4.0)).out().hull(quarter)
    arg05 = PI.div(pt(2.0)).out()
    # tan07 is defined in TestArith.cpp and not registered in TestArith.h: not run in IBEX
    arg07 = pt(3.0).mul(PI).out().div(pt(4.0)).out().hull(pt(5.0).mul(PI).out().div(pt(4.0)).out())
    for name, expression, arg, orig in (('tan02', 'tan((-pi/4)|(pi/4))', arg02, iv(-1, 1)),
                                        ('tan05', 'tan(pi/2)', arg05, ALL),
                                        ('tan07', 'tan(3*pi/4|5*pi/4)', arg07, iv(-1, 1))):
        case = 'IBEX TestArith::' + name
        exact = f_tan(arg)
        compare_interval(case, expression, orig, exact)
        t.add(row(quoted(case), quoted(expression), ref_cell(exact)))


def group_roots():
    t = table('ibex_root_cases', 'ibex_root_case', 'name, x, n, result')
    cases = [('root01', iv(0, 1), -1, iv(1.0, INF)), ('root02', iv(-27, -8), 3, iv(-3, -2)),
             ('root03', iv(-4, 1), 2, iv(0, 1)), ('root04', iv(-8, 1), 3, iv(-2, 1))]
    for name, x, n, orig in cases:
        case = 'IBEX TestArith::' + name
        exact = f_root(x, n)
        compare_interval(case, 'root', orig, exact)
        t.add(row(quoted(case), itv_cell(x), str(n), ref_cell(exact)))


def group_powers():
    t = table('ibex_pow_cases', 'ibex_pow_case', 'name, x, p, result, result_of_opposite')
    cases = [('pow01', ALL, 4, POS), ('pow02', EMPTY, 4, EMPTY), ('pow03', iv(2, 3), 4, iv(16, 81)),
             ('pow04', iv(-2, 3), 4, iv(0, 81)), ('pow05', iv(-3, 2), 4, iv(0, 81)),
             ('pow06', iv(2, INF), 4, iv(16, INF)), ('pow07', ALL, 3, ALL), ('pow08', EMPTY, 3, EMPTY),
             ('pow09', iv(2, 3), 3, iv(8, 27)), ('pow10', iv(-2, 3), 3, iv(-8, 27)),
             ('pow11', iv(-3, 2), 3, iv(-27, 8)), ('pow12', iv(2, INF), 3, iv(8, INF)),
             ('pow13', iv(-10, 10), -2, iv(1.0 / 100, INF))]
    for name, x, p, orig in cases:
        case = 'IBEX TestArith::' + name
        exact, exact_opposite = f_pown(x, p), f_pown(x.neg(), p)
        compare_interval(case, 'pow(x,p)', orig, exact)
        compare_interval(case, 'pow(-x,p)', orig if p % 2 == 0 else orig.neg(), exact_opposite)
        t.add(row(quoted(case), itv_cell(x), str(p), ref_cell(exact), ref_cell(exact_opposite)))


def group_atan2():
    t = table('ibex_atan2_cases', 'ibex_atan2_case', 'name, y, x, result')
    quarter, half = PI.div(pt(4.0)).out(), PI.div(pt(2.0)).out()

    def times(a, b):
        return a.mul(b).out()

    def over(a, d):
        return a.div(pt(d)).out()
    cases = [('atan2_01', ALL, EMPTY, EMPTY), ('atan2_02', ONE, ONE, quarter),
             ('atan2_03', ONE.neg(), ONE.neg(), over(times(pt(-3.0), PI), 4.0)),
             ('atan2_04', ONE.neg(), ONE, quarter.neg()), ('atan2_05', ONE, ONE.neg(), over(times(pt(3.0), PI), 4.0)),
             ('atan2_06', POS, POS, times(iv(0, 1), HALF_PI)),
             ('atan2_07', iv(-INF, previous_float(0.0)), NEG, HALF_PI.hull(PI).neg()),
             ('atan2_08', NEG, POS, times(iv(-1, 0), HALF_PI)), ('atan2_09', POS, NEG, HALF_PI.hull(PI)),
             ('atan2_10', iv(1, INF), iv(-1, 1), quarter.hull(over(times(pt(3.0), PI), 4.0))),
             ('atan2_11', iv(-INF, -1), iv(-1, 1), quarter.hull(over(times(pt(3.0), PI), 4.0)).neg()),
             ('atan2_12', iv(-1, 1), iv(1, INF), times(iv(-1, 1), PI).div(pt(4.0)).out()),
             ('atan2_13', iv(-1, 1), iv(-INF, -1), times(iv(-1, 1), PI)),
             ('atan2_14', ALL, ALL, times(iv(-1, 1), PI)), ('atan2_15', ZERO, ZERO, EMPTY)]
    for name, y, x, orig in cases:
        case = 'IBEX TestArith::' + name
        exact = f_atan2(y, x)
        compare_interval(case, 'atan2', orig, exact)
        t.add(row(quoted(case), itv_cell(y), itv_cell(x), ref_cell(exact)))


def trig_arguments(x):
    """The eight arguments of check_trigo() of IBEX, with the outward roundings of GAOL's operations, and
    whether the function is the cosine"""
    two, half = TWO_PI, HALF_PI
    plus_two = x.add(two).out()
    return [(x, False), (PI.sub(x).out(), False), (plus_two, False), (x.neg(), False),
            (x.sub(half).out(), True), (half.sub(x).out(), True), (x.add(half).out(), True),
            (plus_two.sub(half).out(), True)]


def group_trig():
    t = table('ibex_trig_cases', 'ibex_trig_case', 'name, x, result')
    piu, pil = PI_HI, PI_LO
    s = math.sin
    two_pi_up = mul_up(2.0, piu)
    three_half = div_up(mul_up(3.0, piu), 2.0)
    cases = [('sin01', ALL, iv(-1, 1)), ('sin02', EMPTY, EMPTY),
             ('sin03', iv(0, div_up(piu, 2.0)), iv(0, 1)), ('sin04', iv(0, piu), iv(0, 1)),
             ('sin05', iv(0, three_half), iv(-1, 1)), ('sin06', iv(pil, three_half), iv(-1, 0)),
             ('sin07', iv(0.5, 1.5), iv(s(0.5), s(1.5))), ('sin08', iv(1.5, 3), iv(s(3.0), 1)),
             ('sin09', iv(3, 4), iv(s(4.0), s(3.0))), ('sin10', iv(3, 5), iv(-1, s(3.0))),
             ('sin11', iv(3, add_up(two_pi_up, 1.5)), iv(-1, s(1.5))),
             ('sin12', iv(5, add_up(two_pi_up, 1.5)), iv(s(5.0), s(1.5))),
             ('sin13', iv(5, add_up(two_pi_up, 3.0)), iv(s(5.0), 1))]
    for name, x, orig in cases:
        case = 'IBEX TestArith::' + name
        # sin(x), sin(pi-x), sin(x+2pi) and cos(x-pi/2), cos(pi/2-x), cos(x+2pi-pi/2) expect sin(x); sin(-x) and cos(x+pi/2) -sin(x)
        expected = [orig, orig, orig, orig.neg(), orig, orig, orig.neg(), orig]
        refs = []
        for (arg, cosine), want in zip(trig_arguments(x), expected):
            exact = f_sincos(arg, cosine)
            compare_interval(case, 'the variant', want, exact)
            refs.append(ref_cell(exact))
        t.add('{ %s, %s, { %s } }' % (quoted(case), itv_cell(x), ', '.join(refs)))


def group_hyperbolic():
    ts = table('ibex_sinh_cases', 'ibex_hyperbolic_case', 'name, x, result, result_of_opposite')
    tc = table('ibex_cosh_cases', 'ibex_hyperbolic_case', 'name, x, result, result_of_opposite')
    xs = [ALL, POS, iv(0, 1), iv(1, INF), iv(1, 1), iv(2, 3), iv(4, 5)]

    def ibex_sinh(x):
        yl = -INF if x.lo == -INF else 0.5 * (math.exp(float(x.lo)) - math.exp(-float(x.lo)))
        yu = INF if x.hi == INF else 0.5 * (math.exp(float(x.hi)) - math.exp(-float(x.hi)))
        return yl, yu

    def ibex_cosh(x):
        lb, ub = float(x.lo), float(x.hi)
        if ub == INF:
            return iv(1, INF) if lb <= 0 else iv(math.cosh(lb), INF)
        if lb == -INF:
            return iv(1, INF) if ub >= 0 else iv(math.cosh(ub), INF)
        if lb >= 0:
            return iv(math.cosh(lb), math.cosh(ub))
        if ub <= 0:
            return iv(math.cosh(ub), math.cosh(lb))
        return iv(1, math.cosh(lb)) if abs(lb) > abs(ub) else iv(1, math.cosh(ub))
    for i, x in enumerate(xs, 1):
        yl, yu = ibex_sinh(x)
        case = 'IBEX TestArith::sinh%02d' % i
        exact, exact_opposite = f_sinh(x), f_sinh(x.neg())
        compare_interval(case, 'sinh(x)', iv(yl, yu), exact)
        compare_interval(case, 'sinh(-x)', iv(-yu, -yl), exact_opposite)
        compare_interval(case, 'asinh(sinh(x))', x, x)        # the identity asinh(sinh(x)) = x
        compare_interval(case, 'asinh(sinh(-x))', x.neg(), x.neg())
        ts.add(row(quoted(case), itv_cell(x), ref_cell(exact), ref_cell(exact_opposite)))
        case = 'IBEX TestArith::cosh%02d' % i
        y = ibex_cosh(x)
        exact, exact_opposite = f_cosh(x), f_cosh(x.neg())
        compare_interval(case, 'cosh(x)', y, exact)
        compare_interval(case, 'cosh(-x)', y, exact_opposite)
        tc.add(row(quoted(case), itv_cell(x), ref_cell(exact), ref_cell(exact_opposite)))


def group_constants():
    t = table('ibex_constant_cases', 'ibex_constant_case', 'name, constant, result')
    for name, constant, i in (('empty_set', 'emptyset', EMPTY), ('all_reals', 'universe', ALL), ('pos_reals', 'positive', POS),
                              ('neg_reals', 'negative', NEG), ('zero', 'zero', ZERO), ('one', 'one', ONE),
                              ('pi', 'pi', PI), ('two_pi', 'two_pi', TWO_PI), ('half_pi', 'half_pi', HALF_PI)):
        value = i if name not in ('pi', 'two_pi', 'half_pi') else None
        if value is None:
            q = {'pi': PI_Q, 'two_pi': 2 * PI_Q, 'half_pi': PI_Q / 2}[name]
            exact = Itv(q, q)
            assert exact.refs() == itv_ref(i), name
        t.add(row(quoted('IBEX Interval::' + name + '()'), quoted(constant),
                  itv_cell(i) if value is not None else ref_text(Itv(q, q).refs())))


def group_issue_463():
    """[3, 8] + ([1, 3] & [6, 7]): written by hand in tests/test_ibex_codac.cpp (the chain of two operations); IBEX's one check"""
    a, b = iv(3, 8), iv(1, 3).inter(iv(6, 7))
    compare_predicate('IBEX TestInterval::issue_463', 'the sum is empty', b.empty and a.add(b).empty)


GROUPS = [group_issue_463, group_constants, group_constructors, group_equality, group_set_operations, group_getters, group_midpoints,
          group_relations, group_floats, group_binary, group_unary, group_expressions, group_roots, group_powers,
          group_atan2, group_trig, group_hyperbolic]


def build():
    del TABLES[:]
    del FINDINGS[:]
    CHECKS[0] = 0
    ORIGINAL_CHECKS.clear()
    del NOTES[:]
    for g in GROUPS:
        g()
    return TABLES


#---------------------------------------------------------------------------
# The oracle against sampling
#---------------------------------------------------------------------------
def samples_of(lo, hi):
    """Points of [lo, hi] (floats, possibly infinite): the bounds, points between, 0 and values next to it,
    and large values for an infinite bound"""
    pts = set()
    for b in (lo, hi):
        if not math.isinf(b):
            pts.add(b)
    finite_lo = lo if not math.isinf(lo) else (hi - 1e12 if not math.isinf(hi) else -1e12)
    finite_hi = hi if not math.isinf(hi) else (lo + 1e12 if not math.isinf(lo) else 1e12)
    for k in range(0, 41):
        pts.add(finite_lo + (finite_hi - finite_lo) * k / 40.0)
    if math.isinf(lo):
        pts.update([hi - 1e3, hi - 1e12, hi - 1e30] if not math.isinf(hi) else [-1e3, -1e12, -1e30])
    if math.isinf(hi):
        pts.update([lo + 1e3, lo + 1e12, lo + 1e30] if not math.isinf(lo) else [1e3, 1e12, 1e30])
    if lo <= 0 <= hi:
        pts.update([0.0, 1e-12, -1e-12, 1e-30, -1e-30])
    for b in (lo, hi):
        if not math.isinf(b):
            pts.update([b + 1e-12, b - 1e-12])
    return sorted(p for p in pts if lo <= p <= hi)


def selftest():
    rng = random.Random(20261102)
    failures = [0]

    def fail(what, detail):
        failures[0] += 1
        print('SELFTEST FAILED %s: %s' % (what, detail))
    # 1. the roundings
    for _ in range(20000):
        e = rng.choice([-1074, -1070, -1030, -1022, -1000, -60, -1, 0, 1, 52, 53, 100, 1000, 1023, 1024])
        q = Fraction(rng.getrandbits(80) + 1, 1 << rng.randint(0, 120)) * (Fraction(2) ** e) * rng.choice([1, -1])
        lo, hi = rd(q), ru(q)
        if (lo != -INF and Fraction(lo) > q) or (hi != INF and Fraction(hi) < q) or lo > hi:
            fail('rounding', q)
            continue
        if lo != hi:
            if next_float(lo) != hi:
                fail('rounding, two doubles apart', q)
        elif Fraction(lo) != q:
            fail('rounding, equal bounds of a non-double', q)
        if abs(q) < Fraction(DBL_MAX) and abs(float(q)) != INF and float(q) not in (lo, hi):
            fail('rounding, the nearest double', q)
    # 2. the products and quotients
    pal = [-INF, -3.0, -1.0, -0.5, 0.0, 1.0 / 3.0, 1.0, 2.0, 7.0, INF]
    for a in range(len(pal)):
        for b in range(a, len(pal)):
            for c in range(len(pal)):
                for d in range(c, len(pal)):
                    X_, Z_ = iv(pal[a], pal[b]), iv(pal[c], pal[d])
                    if X_.empty or Z_.empty:
                        continue
                    xs, zs = samples_of(pal[a], pal[b]), samples_of(pal[c], pal[d])
                    prod = [x * z for x in xs for z in zs]
                    quot = [x / z for x in xs for z in zs if z != 0.0]
                    for name, got, vals in (('mul', X_.mul(Z_), prod), ('div', X_.div(Z_), quot)):
                        if not vals:
                            if not got.empty:
                                fail(name, (pal[a], pal[b], pal[c], pal[d], got))
                            continue
                        lo, hi = min(vals), max(vals)
                        glo, ghi = float(got.lo) if not is_inf(got.lo) else got.lo, float(got.hi) if not is_inf(got.hi) else got.hi
                        tol = 1e-9 * (1 + max(abs(lo), abs(hi)))
                        ok = glo <= lo + tol and ghi >= hi - tol
                        # a finite bound of the hull is attained; an infinite one is reached by large samples
                        ok = ok and (glo == -INF or glo >= lo - tol or (lo < -1e9 and False)) and (ghi == INF or ghi <= hi + tol)
                        ok = ok and (glo != -INF or lo < -1e9) and (ghi != INF or hi > 1e9)
                        if not ok:
                            fail(name, (pal[a], pal[b], pal[c], pal[d], got, lo, hi))
    # 3. atan2
    for a in range(len(pal)):
        for b in range(a, len(pal)):
            for c in range(len(pal)):
                for d in range(c, len(pal)):
                    Y_, X_ = iv(pal[a], pal[b]), iv(pal[c], pal[d])
                    got = f_atan2(Y_, X_)
                    ys, xs = samples_of(pal[a], pal[b]), samples_of(pal[c], pal[d])
                    vals = [math.atan2(y, x) for y in ys for x in xs if (x, y) != (0.0, 0.0)]
                    if not vals:
                        if not got.empty:
                            fail('atan2', (pal[a], pal[b], pal[c], pal[d], got))
                        continue
                    glo, ghi = float(got.lo), float(got.hi)
                    if not (abs(glo - min(vals)) < 1e-9 and abs(ghi - max(vals)) < 1e-9):
                        fail('atan2', ('y', pal[a], pal[b], 'x', pal[c], pal[d], glo, ghi, min(vals), max(vals)))
    # 4. sin, cos, tan
    for _ in range(400):
        a = rng.uniform(-30, 30)
        b = a + rng.choice([0.0, 1e-3, 0.5, 1.5, 3.0, 4.0, 6.0, 7.0, 20.0]) * rng.random()
        for cosine in (False, True):
            got = f_sincos(iv(a, b), cosine)
            f = math.cos if cosine else math.sin
            vals = [f(a + (b - a) * k / 4000.0) for k in range(4001)]
            if abs(float(got.lo) - min(vals)) > 1e-5 or abs(float(got.hi) - max(vals)) > 1e-5:
                fail('sincos', (a, b, cosine, got))
        got = f_tan(iv(a, b))
        poles = [k for k in range(-30, 30) if a <= math.pi / 2 + k * math.pi <= b]
        if bool(poles) != (got.lo == -INF):
            fail('tan', (a, b, got))
        elif not poles and abs(float(got.lo) - math.tan(a)) > 1e-9 * (1 + abs(math.tan(a))):
            fail('tan', (a, b, got))
    # 5. the integer powers and the roots
    for a in range(len(pal)):
        for b in range(a, len(pal)):
            x = iv(pal[a], pal[b])
            xs = samples_of(pal[a], pal[b])
            for n in range(-5, 6):
                got = f_pown(x, n)
                vals = [(v ** n) for v in xs if not (v == 0.0 and n < 0) and abs(v) < 1e30 or n >= 0 and abs(v) < 1e30]
                vals = [v ** n for v in xs if abs(v) < 1e30 and not (v == 0.0 and n < 0)]
                big = any(math.isinf(v) for v in (pal[a], pal[b]))
                if not vals:
                    if not got.empty and not big:
                        fail('pown', (pal[a], pal[b], n, got))
                    continue
                glo, ghi = got.lo, got.hi
                glo, ghi = (glo if is_inf(glo) else float(glo)), (ghi if is_inf(ghi) else float(ghi))
                tol = 1e-9 * (1 + max(abs(min(vals)), abs(max(vals))))
                if glo > min(vals) + tol or ghi < max(vals) - tol:
                    fail('pown', (pal[a], pal[b], n, got, min(vals), max(vals)))
                if not big and (n >= 0 or 0.0 not in (pal[a], pal[b])) and (glo < min(vals) - tol and glo != -INF):
                    fail('pown, the lower bound is below every sample', (pal[a], pal[b], n, got))
            for n in range(-4, 5):
                if n == 0:
                    continue
                got = f_root(x, n)
                m = abs(n)
                vals = []
                for v in xs:
                    if abs(v) > 1e30 or (v == 0.0 and n < 0) or (v < 0 and m % 2 == 0):
                        continue
                    r = math.copysign(abs(v) ** (1.0 / m), v)
                    vals.append(r if n > 0 else 1.0 / r)
                if not vals:
                    if not got.empty:
                        fail('root', (pal[a], pal[b], n, got))
                    continue
                glo, ghi = (got.lo if is_inf(got.lo) else float(got.lo)), (got.hi if is_inf(got.hi) else float(got.hi))
                tol = 1e-6 * (1 + max(abs(min(vals)), abs(max(vals))))
                if glo > min(vals) + tol or ghi < max(vals) - tol:
                    fail('root', (pal[a], pal[b], n, got, min(vals), max(vals)))
    # 6. floor, ceil, integer on a few intervals
    for lo, hi in ((0.2, 0.7), (0.2, 1.0), (-0.5, 0.5), (1.0, 1.0), (-2.5, -0.5), (3.5, 9.25)):
        x = iv(lo, hi)
        ints = [k for k in range(-5, 12) if lo <= k <= hi]
        got = f_integer(x)
        if (not ints) != got.empty or (ints and (got.lo != min(ints) or got.hi != max(ints))):
            fail('integer', (lo, hi, got))
        if f_floor(x).lo != math.floor(lo) or f_floor(x).hi != math.floor(hi) or f_ceil(x).lo != math.ceil(lo) or f_ceil(x).hi != math.ceil(hi):
            fail('floor or ceil', (lo, hi))
    print('selftest: %d failures' % failures[0])
    return 1 if failures[0] else 0


#---------------------------------------------------------------------------
# The command line
#---------------------------------------------------------------------------
MARKER = re.compile(r'^  // (BEGIN|END) (\w+)')


def blocks_of(path):
    """The generated blocks of a file: {table: its lines, from the BEGIN line to the END line}"""
    found, current = {}, None
    for line in open(path).read().split('\n'):
        m = MARKER.match(line)
        if m and m.group(1) == 'BEGIN':
            current = m.group(2)
            found[current] = [line]
        elif current:
            found[current].append(line)
            if m and m.group(1) == 'END':
                current = None
    return found


def write(path):
    text = open(path).read()
    for t in build():
        pattern = re.compile(r'  // BEGIN %s\b.*?  // END %s\b' % (t.name, t.name), re.S)
        if not pattern.search(text):
            sys.exit('ibex_forward_cases.py: %s: no BEGIN/END of %s' % (path, t.name))
        text = pattern.sub(lambda m: t.text(), text, count=1)
    open(path, 'w').write(text)
    print('%s: %d tables, %d rows written' % (path, len(TABLES), sum(len(t.rows) for t in TABLES)))


def check(path):
    found = blocks_of(path)
    bad = 0
    for t in build():
        want = t.text().split('\n')
        got = found.pop(t.name, None)
        if got is None:
            print('%s: the table %s is missing' % (path, t.name))
            bad += 1
            continue
        if got != want:
            bad += 1
            for i in range(max(len(got), len(want))):
                a = got[i] if i < len(got) else '(no line)'
                b = want[i] if i < len(want) else '(no line)'
                if a != b:
                    print('%s: %s, line %d of the table:\n  file:      %s\n  generated: %s' % (path, t.name, i, a, b))
    for name in found:
        if not name.startswith('ibex_'):
            continue  # a table of another section of the file, checked by its own script
        print('%s: the table %s is not generated' % (path, name))
        bad += 1
    print('%s: %d tables, %d rows checked, %s' % (path, len(TABLES), sum(len(t.rows) for t in TABLES),
                                                  'all as generated' if not bad else '%d tables differ' % bad))
    return 1 if bad else 0


# Disagreements known and explained: (IBEX test, text up to the colon) -> why
KNOWN = {}


def distinct(items):
    seen, out = set(), []
    for case, text in items:
        if (case, text) not in seen:
            seen.add((case, text))
            out.append((case, text))
    return out


def row_names():
    """The IBEX test of each row written: {test: number of rows}"""
    rows = {}
    for t in TABLES:
        for r in t.rows:
            key = 'IBEX ' + re.match(r'\{ "IBEX ([^" \[]+)', r).group(1)
            rows[key] = rows.get(key, 0) + 1
    return rows


def originals(verbose):
    build()
    print('%d checks of IBEX compared with the exact results (tolerance %g, absolute; none for ==)' % (CHECKS[0], ERROR))
    unknown = 0
    for case, text in distinct(FINDINGS):
        why = KNOWN.get((case, text.split(':')[0]))
        print('DISAGREEMENT %s: %s%s' % (case, text, ('  [known: %s]' % why) if why else ''))
        unknown += why is None
    notes = distinct(NOTES)
    print('%d disagreements, %d expectations within the tolerance that do not enclose the exact result' % (len(distinct(FINDINGS)), len(notes)))
    if verbose:
        for case, text in notes:
            print('note %s: %s' % (case, text))
        rows = row_names()
        rows['IBEX TestInterval::issue_463'] = 1      # written by hand
        print('%-34s %7s %6s' % ('IBEX test', 'checks', 'rows'))
        for test in sorted(set(rows) | set(ORIGINAL_CHECKS), key=lambda n: (n.split('::')[0], n)):
            print('%-34s %7d %6d' % (test, ORIGINAL_CHECKS.get(test, 0), rows.get(test, 0)))
    return 1 if unknown else 0


def main():
    p = argparse.ArgumentParser(description='The forward tests of IBEX as native GAOL tests: rows, references, checks.')
    p.add_argument('--write', metavar='FILE', help='replace the generated tables of FILE')
    p.add_argument('--check', metavar='FILE', help='check that the generated tables of FILE are what is generated')
    p.add_argument('--originals', action='store_true', help='report the expectations of IBEX that differ from the exact results')
    p.add_argument('-v', '--verbose', action='store_true', help='with --originals: the notes, and the checks and rows of each test')
    p.add_argument('--selftest', action='store_true', help='check the oracle against sampling')
    a = p.parse_args()
    if a.write:
        write(a.write)
    elif a.check:
        sys.exit(check(a.check))
    elif a.originals:
        sys.exit(originals(a.verbose))
    elif a.selftest:
        sys.exit(selftest())
    else:
        for t in build():
            print(t.text())
            print()


if __name__ == '__main__':
    main()
