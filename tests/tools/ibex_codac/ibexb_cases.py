#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of GAOL v5: the backward tests of IBEX (ibex-lib commit 65ed587,
# tests/TestArith.cpp: bwd_mul, bwd_div, bwd_maxmin, bwd_sqr, bwd_log, bwd_sin
# (and bwd_cos), bwd_pow, bwd_atan2, bwd_sqrt, bwd_atan, bwd_add, bwd_sub,
# bwd_imod, bwd_floor, bwd_ceil, and div2; tests/TestInterval.cpp: distance and
# rel_distance), as native GAOL tests.
#
# This script goes with ibex_forward_cases.py, the forward tests of IBEX, whose
# oracle it imports (exact intervals, the roundings to the doubles, the
# comparison with the expectations of IBEX) and whose conventions it keeps.
# The case list lives here, with what each IBEX test expects, and so does the
# oracle: for each case, the exact mathematical result of the contraction on the
# exact inputs (the doubles the test writes), computed apart from GAOL and from
# IBEX, by exact rational arithmetic (fractions) for the contractions that are
# rational, and by mpmath with 2000 bits for the roots, the exponential, the
# sine, the cosine, the tangent and the angles. The reference stored for an
# interval result is Ref{lo, hi}: lo the greatest double not above the exact
# lower bound, hi the least double not below the exact upper bound (-oo or +oo
# beyond the doubles), `ibexb_empty` for the empty set; for a real result the
# same pair; for a yes or no, the exact truth value.
#
# What the contractors of IBEX are in GAOL: the native expression each case runs,
# and whether IBEX (src/arithmetic/ibex_Interval.h, interval_lib_wrapper/gaol) computes the
# same:
#   bwd_add(y, x1, x2)   x1 & (y - x2), then x2 & (y - x1)                    IBEX: the same
#   bwd_sub(y, x1, x2)   x1 & (y + x2), then x2 & (x1 - y)                    IBEX: the same
#   bwd_mul(y, x1, x2)   div_rel(y, x2, x1), then div_rel(y, x1, x2)          IBEX: the same
#   bwd_div(y, x1, x2)   x1 & (y * x2); t = div_rel(x1, x2, y); then          IBEX: the same (t does not change
#                        div_rel(x1, t, x2)                                   the second contraction)
#   bwd_max, bwd_min     max_rel(z, y, x) then max_rel(z, x, y) (min_rel)     IBEX: by cases, the same hull
#   bwd_sqr(y, x)        sqrt_rel(y, x)                                       IBEX: the same
#   bwd_sqrt(y, x)       x & sqr(y & [0, +oo])                                IBEX: the same, by cases
#   bwd_log(y, x)        x & exp(y)                                           IBEX: the same
#   bwd_sin, bwd_cos     asin_rel(y, x), acos_rel(y, x)                       IBEX: the same (its & is redundant)
#   bwd_pow(y, p, x)     nth_root_rel(y, p, x) for p > 0; for p < 0 the        IBEX: through root() for every p
#                        roots nth_root(y, p) of y, and their opposites       (nth_root_rel was wrong in GAOL 4):
#                        for an even p, meet x                                the same hull
#   bwd_atan2(c, y, x)   atan2_rel_y(c, x, y), then atan2_rel_x(c, y, x)      IBEX: by quadrants (bwd_atan and
#                                                                             bwd_div), the same hull
#   bwd_atan(y, x)       x & [tan of the lower bound of y, tan of its upper   IBEX: the same, but the double
#                        bound], infinite at a pole, empty beyond it          below pi/2 is a pole for it
#   bwd_floor(y, x)      floor_rel(y, x)                                      IBEX: [ceil(inf y), floor(sup y) + 1]
#                                                                             closed at the end: floor_rel is open
#   bwd_ceil(y, x)       -floor_rel(-y, -x)                                   IBEX: [ceil(inf y) - 1, floor(sup y)]
#                                                                             closed at the end: open for GAOL
#   bwd_imod(x, y, p)    for each integer k of integer((x - y) / p): x & (y + k p)   IBEX: the same for the integers
#                        and y & (x - k p), the hulls over k                         of the tests, it gives up on
#                                                                             wide boxes and on more than two k
#   div2(num, div)       gaol_ieee1788::mulRevToPair(div, num)                IBEX: by cases, the same hull but for
#                                                                             0/0 (IBEX: num = [0] gives ([0], empty),
#                                                                             div = [0] empty, mulRev: the whole line)
#   distance(x, y)       hausdorff(x, y)                                      IBEX: the same (it settles the infinite
#                                                                             bounds, GAOL's hausdorff does too),
#                                                                             the empty set apart
#   rel_distance(x, y)   hausdorff(x, y) / width(x)                           IBEX: the same, plus its conventions
#
# What is exact and what is a convention:
#   - the references of the contractions are the hulls of the sets they stand
#     for, computed from the definition: the x of X with a y of Y such that
#     x + y, x - y, x y, x/y, max(x, y), x^2, x^n, exp, sin, cos, atan,
#     atan2... is in the image, with the same sequence of contractions as the
#     native code: the interval a contraction gives is replayed as the interval
#     of doubles GAOL gives, the tightest enclosure of the exact hull (the
#     second contraction of bwd_add runs on the first one's result);
#   - the y of the sine and cosine tests, sin(Interval(0.5, 1.5)) in IBEX, is the
#     tightest enclosure of the sine of [0.5, 1.5], two doubles of the table:
#     the references are computed from those doubles, so as not to depend on
#     the sine of GAOL (a bound of x near pi/2 moves by several doubles for one
#     double of y);
#   - the doubles that IBEX computes before the contraction (5*piU/2.0, M_PI/6.,
#     -sqrt(3.)/2...) are rounded upward, as GAOL leaves the direction: a choice,
#     not what a build of IBEX does (it is compiled with -frounding-math only with
#     its other interval libraries, so that a compiler folds these constants to
#     the nearest double, one double away in 37 of them); they are in the tables,
#     as the other inputs;
#   - the distance of IBEX to the empty set is the radius of the other interval,
#     the distance of GAOL a NaN: no such case is ported (distance20 and 21);
#   - rel_distance is the distance over the width; IBEX settles the infinite
#     distances (1) and a null or infinite width (0) by convention; the
#     quotient is checked where it is a quotient of reals or of a finite number
#     by +oo, and the distance and the width are checked in every case.
#
# The result each IBEX test expects is a cross-check, not the reference: it is
# compared with the exact result with the tolerance the IBEX test uses (1e-10,
# absolute, for check() and almost_eq(); none for ==), and every disagreement
# is reported (--originals).
#
# The rows of the tables of tests/test_ibex_codac.cpp, the section ibexb_ (the backward tests of IBEX),
# are written by this script between the lines
#
#     // BEGIN <table>
#     // END <table>
#
# (the text between them, those lines included, is what the script writes).
#
#     python3 -I ibexb_cases.py                     the tables, on the standard output
#     python3 -I ibexb_cases.py --write ../../test_ibex_codac.cpp   replaces the tables of the file
#     python3 -I ibexb_cases.py --check ../../test_ibex_codac.cpp   every table of the file is what is generated
#     python3 -I ibexb_cases.py --originals [-v]    the disagreements with IBEX's expectations
#     python3 -I ibexb_cases.py --selftest          checks the oracles against sampling
#
# ibex_forward_cases.py has to be in the directory of this script. The exit
# status is 1 when --check finds a difference, --originals a disagreement that
# is not listed in KNOWN, or --selftest a failure. Python 3.8 or later, and
# mpmath (1.1 or later).
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
import os
import random
import re
import sys
from fractions import Fraction

# python3 -I does not put the directory of the script in the path
sys.dont_write_bytecode = True     # no __pycache__ in the directory of the tools
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import mpmath                       # noqa: E402
try:
    import ibex_forward_cases as fc     # noqa: E402
except ImportError:
    sys.exit('ibexb_cases.py needs ibex_forward_cases.py, the forward tests of IBEX, in its directory')

INF = fc.INF
DBL_MAX = fc.DBL_MAX
ERROR = fc.ERROR
Itv, iv, pt, X = fc.Itv, fc.iv, fc.pt, fc.X
EMPTY, ALL, POS, NEG, ZERO, ONE = fc.EMPTY, fc.ALL, fc.POS, fc.NEG, fc.ZERO, fc.ONE
is_inf = fc.is_inf
Q0 = Fraction(0)
PI_LO, PI_HI = fc.PI_LO, fc.PI_HI          # Interval::pi().lb() and ub(): piL and piU
HALF_PI = fc.HALF_PI                       # Interval::half_pi(): [piL/2, piU/2]
M_PI = math.pi                             # the double of M_PI


#---------------------------------------------------------------------------
# Exact numbers: roots, mpmath values as fractions
#---------------------------------------------------------------------------
def mpf_ext(q):
    """An extended number (a Fraction or an infinite float) as an mpf"""
    if is_inf(q):
        return mpmath.inf if q > 0 else -mpmath.inf
    return fc.mpf_of(q)


def snapq(v):
    """An mpf as a Fraction: the double it is within 2^-1800 of (a root, an exponential that is exact), or
    the Fraction of its bits; an infinity stays"""
    q = fc.mpq(v)
    if is_inf(q):
        return q
    if abs(q) < Fraction(1, 2 ** 1800):
        return Q0
    lo, hi = fc.snap_bounds(q)
    return Fraction(lo) if lo == hi else q


def iroot(n, m):
    """The greatest integer r with r^m <= n, for an integer n >= 0"""
    if n < 2:
        return n
    lo, hi = 0, 1 << (n.bit_length() // m + 1)
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if mid ** m <= n:
            lo = mid
        else:
            hi = mid - 1
    return lo


def root_ext(q, m):
    """q^(1/m) for q >= 0: a Fraction when q is a perfect power, else the mpmath value; +oo stays"""
    if is_inf(q):
        return q
    if q == 0:
        return Q0
    rn, rd_ = iroot(q.numerator, m), iroot(q.denominator, m)
    if rn ** m == q.numerator and rd_ ** m == q.denominator:
        return Fraction(rn, rd_)
    return snapq(mpmath.root(fc.mpf_of(q), m))


def sroot(q, m):
    """The real root of order m of any q (the real root of a negative number for an odd m)"""
    return -root_ext(-q, m) if q < 0 else root_ext(q, m)


def sq(q):
    return INF if is_inf(q) else q * q


def sqr_itv(x):
    """The exact square of an interval"""
    if x.empty:
        return x
    a, b = abs(x.lo), abs(x.hi)
    if x.lo <= 0 <= x.hi:
        return Itv(Q0, sq(max(a, b)))
    return Itv(sq(min(a, b)), sq(max(a, b)))


def neg_if(i, negative):
    return i.neg() if negative else i


#---------------------------------------------------------------------------
# The exact reverse functions: the hulls of the sets the contractors stand for
#---------------------------------------------------------------------------
def recip_pieces(j):
    """The sets {1/y : y in j, y != 0}, one per side of 0 (fc.Itv.div, kept apart)"""
    pieces = []
    if j.lo < 0:
        pieces.append(Itv(-INF, fc.e_recip(j.lo)) if j.hi >= 0 else Itv(fc.e_recip(j.hi), fc.e_recip(j.lo)))
    if j.hi > 0:
        pieces.append(Itv(fc.e_recip(j.hi), INF) if j.lo <= 0 else Itv(fc.e_recip(j.hi), fc.e_recip(j.lo)))
    return pieces


def rel_div(k, j, i):
    """div_rel(K, J, I) = hull{x in I | exists z in K, y in J: z = x y}"""
    if k.empty or j.empty or i.empty:
        return EMPTY
    if k.lo <= 0 <= k.hi and j.lo <= 0 <= j.hi:     # 0 = x * 0: every x
        return i
    result = EMPTY
    for p in recip_pieces(j):
        part = i.inter(k.mul(p))
        # the set of the quotients z / y is open at 0 where y is unbounded (z / y is never 0 for z != 0):
        # an I that only touches it there, in {0}, holds no solution
        if not part.empty and part.lo == 0 and part.hi == 0 and not k.lo <= 0 <= k.hi:
            continue
        result = result.hull(part)
    return result


def rel_sqr(y, x):
    """sqrt_rel(Y, X) = hull{x in X | x^2 in Y}"""
    yp = y.inter(POS)
    if yp.empty or x.empty:
        return EMPTY
    a, b = root_ext(yp.lo, 2), root_ext(yp.hi, 2)
    return x.inter(Itv(-b, -a)).hull(x.inter(Itv(a, b)))


def rel_pown(y, n, x):
    """nth_root_rel(Y, n, X) = hull{x in X | x^n in Y}, n >= 1"""
    if y.empty or x.empty:
        return EMPTY
    if n % 2 == 1:
        return x.inter(Itv(sroot(y.lo, n), sroot(y.hi, n)))
    yp = y.inter(POS)
    if yp.empty:
        return EMPTY
    a, b = root_ext(yp.lo, n), root_ext(yp.hi, n)
    return x.inter(Itv(-b, -a)).hull(x.inter(Itv(a, b)))


def rel_pow(y, p, x):
    """hull{x in X | x^p in Y}, p an integer; for p < 0, an even one: x^p = 1/x^m, m = -p, in Y (0 is no
    solution), that is x^m in 1/(Y > 0)"""
    if p > 0:
        return rel_pown(y, p, x)
    m = -p
    assert m % 2 == 0, 'only the even negative exponents are written'
    yp = y.inter(POS)
    if yp.empty or yp.hi == 0 or x.empty:
        return EMPTY
    t = Itv(fc.e_recip(yp.hi), INF if yp.lo == 0 else fc.e_recip(yp.lo))
    return rel_pown(t, m, x)


def rel_exp(y, x):
    """The x of X in exp(Y), the reverse of the logarithm"""
    return x.inter(fc.f_exp(y))


def periodic_hull(x, y, cosine):
    """The hull of the x of X with sin(x) (cos(x) with cosine) in Y, over the pieces of the preimage in each
    period [asin(a), asin(b)] and [pi - asin(b), pi - asin(a)] (cosine: [acos(b), acos(a)] and
    [-acos(a), -acos(b)]), shifted by 2 k pi"""
    if x.empty or y.empty:
        return EMPTY
    yc = y.inter(Itv(Fraction(-1), Fraction(1)))
    if yc.empty:
        return EMPTY
    pi = mpmath.pi
    a, b = fc.mpf_of(yc.lo), fc.mpf_of(yc.hi)
    if cosine:
        base = [(mpmath.acos(b), mpmath.acos(a)), (-mpmath.acos(a), -mpmath.acos(b))]
    else:
        base = [(mpmath.asin(a), mpmath.asin(b)), (pi - mpmath.asin(b), pi - mpmath.asin(a))]
    period = 2 * pi
    lo_inf, hi_inf = x.lo == -INF, x.hi == INF
    if lo_inf and hi_inf:
        return ALL
    # the window where the pieces are met: X itself, or four periods from its finite bound
    wl = mpf_ext(x.lo) if not lo_inf else mpf_ext(x.hi) - 4 * period
    wh = mpf_ext(x.hi) if not hi_inf else mpf_ext(x.lo) + 4 * period
    kmin, kmax = int(mpmath.floor(wl / period)) - 2, int(mpmath.ceil(wh / period)) + 2
    assert kmax - kmin < 10000, 'a box too large for the periodic preimage'
    low = high = None
    for k in range(kmin, kmax + 1):
        for plo, phi in base:
            lo_, hi_ = max(wl, plo + k * period), min(wh, phi + k * period)
            if lo_ <= hi_:
                low = lo_ if low is None else min(low, lo_)
                high = hi_ if high is None else max(high, hi_)
    if low is None:
        return EMPTY
    return Itv(-INF if lo_inf else snapq(low), INF if hi_inf else snapq(high))


def rel_sin(y, x):
    return periodic_hull(x, y, False)


def rel_cos(y, x):
    return periodic_hull(x, y, True)


def rel_atan(y, x):
    """The x of X with atan(x) in Y: tan of Y inside (-pi/2, pi/2), infinite at a pole and empty beyond it"""
    if y.empty or x.empty:
        return EMPTY
    half = mpmath.pi / 2
    ylo, yhi = mpf_ext(y.lo), mpf_ext(y.hi)
    if yhi <= -half or ylo >= half:
        return EMPTY
    lo = -INF if ylo <= -half else snapq(mpmath.tan(ylo))
    hi = INF if yhi >= half else snapq(mpmath.tan(yhi))
    return x.inter(Itv(lo, hi))


def _clip(poly, f, slack):
    """The polygon (list of mp points) inside f >= 0, a linear form (Sutherland-Hodgman); a point is inside
    where f(p) + slack(p) >= 0, a tolerance for the points on the line that the products of the form
    leave a hair negative, which does not move the points cut on the segments"""
    def cut(p, q):
        fp, fq = f(p), f(q)
        if fp == fq:
            return p
        if abs(fq) < abs(fp):         # from the end nearer to the line, to lose no digit to a parameter near 1
            p, q, fp, fq = q, p, fq, fp
        t = min(max(fp / (fp - fq), 0), 1)
        return (p[0] + t * (q[0] - p[0]), p[1] + t * (q[1] - p[1]))
    out = []
    for i in range(len(poly)):
        p, q = poly[i - 1], poly[i]
        p_in, q_in = f(p) + slack(p) >= 0, f(q) + slack(q) >= 0
        if q_in:
            if not p_in:
                out.append(cut(p, q))
            out.append(q)
        elif p_in:
            out.append(cut(p, q))
    return out


def cone_projections(theta, x, y):
    """(the hull of the y, the hull of the x) of the points (x, y) of the box X x Y whose angle atan2(y, x)
    is in theta, the point (0, 0), which has no angle, left out, the angles being those of (-pi, pi]: the
    wedges of theta cut at the axes, each clipped to the box. An infinite bound of the box is 2^2200: the
    clipping cancels 2200 bits of the coordinates, hence the 5000 bits"""
    if theta.empty or x.empty or y.empty:
        return EMPTY, EMPTY
    with mpmath.workprec(5000):
        return _cone_projections(theta, x, y)


def _cone_projections(theta, x, y):
    pi = mpmath.pi
    a, b = max(mpf_ext(theta.lo), -pi), min(mpf_ext(theta.hi), pi)
    if a > b or b <= -pi:
        return EMPTY, EMPTY
    axes = [(-pi, (-1, 0)), (-pi / 2, (0, -1)), (mpmath.mpf(0), (1, 0)), (pi / 2, (0, 1)), (pi, (-1, 0))]

    def direction(t):
        for angle, vec in axes:
            if t == angle:
                return (mpmath.mpf(vec[0]), mpmath.mpf(vec[1]))
        return (mpmath.cos(t), mpmath.sin(t))
    breaks = [a] + [angle for angle, _ in axes if a < angle < b] + [b]
    big = mpmath.mpf(2) ** 2200

    def finite(v):
        return (big if v > 0 else -big) if is_inf(v) else fc.mpf_of(v)
    xl, xu, yl, yu = finite(x.lo), finite(x.hi), finite(y.lo), finite(y.hi)
    eps = mpmath.mpf(2) ** -4500      # above the noise of the products (2^-5000), far below any distance of doubles
    tiny = mpmath.mpf(2) ** -1800     # a coordinate below it is 0
    xs, ys = [], []
    for t0, t1 in zip(breaks, breaks[1:]):
        d0, d1 = direction(t0), direction(t1)
        poly = [(xl, yl), (xu, yl), (xu, yu), (xl, yu)]
        forms = [lambda p: d0[0] * p[1] - d0[1] * p[0], lambda p: p[0] * d1[1] - p[1] * d1[0],
                 lambda p: (d0[0] + d1[0]) * p[0] + (d0[1] + d1[1]) * p[1]]
        for form in forms:
            # a point on the line may come out of the product a hair negative: the tolerance is relative
            poly = _clip(poly, form, lambda p: eps * (1 + abs(p[0]) + abs(p[1])))
            if not poly:
                break
        if t0 == -pi and poly and min(p[1] for p in poly) >= -tiny:
            continue    # the half-line y = 0, x < 0 has the angle pi, not -pi: a box that only meets it holds no point
        if poly and any(abs(p[0]) > tiny or abs(p[1]) > tiny for p in poly):
            xs.extend(p[0] for p in poly)
            ys.extend(p[1] for p in poly)
    if not xs:
        return EMPTY, EMPTY

    def bound(v):
        return INF if v >= big / 2 else (-INF if v <= -big / 2 else snapq(v))
    return Itv(bound(min(ys)), bound(max(ys))), Itv(bound(min(xs)), bound(max(xs)))


def cone_projections_float(theta, x, y):
    """The same hulls by another way, in floats, for a bounded box: for each wedge, the corners of the box in
    the wedge, the cuts of the sides of the wedge with the sides of the box, and the origin when it is in the
    box. None when empty"""
    if theta.empty or x.empty or y.empty:
        return None
    xl, xu, yl, yu = float(x.lo), float(x.hi), float(y.lo), float(y.hi)
    pi = math.pi
    a, b = max(float(theta.lo), -pi), min(float(theta.hi), pi)
    if a > b or b <= -pi:
        return None
    axes = {-pi: (-1.0, 0.0), -pi / 2: (0.0, -1.0), 0.0: (1.0, 0.0), pi / 2: (0.0, 1.0), pi: (-1.0, 0.0)}

    def vec(t):
        return axes.get(t, (math.cos(t), math.sin(t)))
    cuts = [a] + [t for t in sorted(axes) if a < t < b] + [b]
    points = []
    for t0, t1 in zip(cuts, cuts[1:]):
        r0, r1 = vec(t0), vec(t1)
        mid = (r0[0] + r1[0], r0[1] + r1[1])

        def inside(p):
            tol = 1e-9 * (1 + abs(p[0]) + abs(p[1]))
            return (r0[0] * p[1] - r0[1] * p[0] >= -tol and p[0] * r1[1] - p[1] * r1[0] >= -tol
                    and mid[0] * p[0] + mid[1] * p[1] >= -tol)
        cand = [(cx, cy) for cx in (xl, xu) for cy in (yl, yu) if inside((cx, cy))]
        for r in (r0, r1):
            for cx in (xl, xu):
                if r[0] != 0 and cx / r[0] >= 0:
                    cy = cx / r[0] * r[1]
                    if yl - 1e-9 <= cy <= yu + 1e-9:
                        cand.append((cx, min(max(cy, yl), yu)))
            for cy in (yl, yu):
                if r[1] != 0 and cy / r[1] >= 0:
                    cx = cy / r[1] * r[0]
                    if xl - 1e-9 <= cx <= xu + 1e-9:
                        cand.append((min(max(cx, xl), xu), cy))
        if xl <= 0 <= xu and yl <= 0 <= yu and cand:
            cand.append((0.0, 0.0))
        if t0 == -pi and cand and min(c[1] for c in cand) >= -1e-9:
            continue
        if any(abs(c[0]) > 1e-12 or abs(c[1]) > 1e-12 for c in cand):
            points.extend(cand)
    if not points:
        return None
    return (min(p[1] for p in points), max(p[1] for p in points)), (min(p[0] for p in points), max(p[0] for p in points))


def rel_atan2_y(c, x, y):
    return cone_projections(c, x, y)[0]


def rel_atan2_x(c, y, x):
    return cone_projections(c, x, y)[1]


def rel_max(z, y, x):
    """max_rel(Z, Y, X) = hull{x in X | exists y in Y: max(x, y) in Z}"""
    if z.empty or y.empty or x.empty:
        return EMPTY
    yc = y.inter(Itv(-INF, z.hi))                  # the y of Y at most sup Z
    if yc.empty:
        return EMPTY
    if yc.hi >= z.lo:                              # a y in Z: every x up to sup Z
        return x.inter(Itv(-INF, z.hi))
    return x.inter(z)                              # Y below Z: the maximum is x


def rel_min(z, y, x):
    return rel_max(z.neg(), y.neg(), x.neg()).neg()


def rel_floor(z, x):
    """floor_rel(Z, X) = hull{x in X | floor(x) in Z}: X meets [ceil(inf Z), floor(sup Z) + 1)"""
    if z.empty or x.empty:
        return EMPTY
    lo = z.lo if is_inf(z.lo) else Fraction(math.ceil(z.lo))
    hi = z.hi if is_inf(z.hi) else Fraction(math.floor(z.hi))
    if lo > hi:
        return EMPTY
    top = hi if is_inf(hi) else hi + 1                 # the open end
    low = max(x.lo, lo)
    if low > x.hi or low >= top:
        return EMPTY
    return Itv(low, min(x.hi, top))


def rel_ceil(z, x):
    return rel_floor(z.neg(), x.neg()).neg()


def rel_imod(x, y, p):
    """(x, y) of the pairs with x - y = k p, an integer k, p > 0 a double: the hulls of the x and of the y"""
    if x.empty or y.empty:
        return EMPTY, EMPTY
    d = x.sub(y)
    assert not is_inf(d.lo) and not is_inf(d.hi)
    p = Fraction(p)
    xs, ys = EMPTY, EMPTY
    for k in range(math.ceil(d.lo / p), math.floor(d.hi / p) + 1):
        kp = Itv(k * p, k * p)
        xs = xs.hull(x.inter(y.add(kp)))
        ys = ys.hull(y.inter(x.sub(kp)))
    return xs, ys


def mul_rev_pair(b, c):
    """The two parts of {x : b' x = c' for a b' of B and a c' of C}: (u, v), the lower first, (u, empty)
    for one part, 0 being strictly inside B and not in C for two, the entire line (u) when 0 is in B and C"""
    if b.empty or c.empty:
        return EMPTY, EMPTY
    if b.lo <= 0 <= b.hi and c.lo <= 0 <= c.hi:
        return ALL, EMPTY
    parts = [c.mul(p) for p in recip_pieces(b)]
    parts = [p for p in parts if not p.empty]
    if not parts:
        return EMPTY, EMPTY
    if len(parts) == 1:
        return parts[0], EMPTY
    parts.sort(key=lambda p: p.lo)
    return parts[0], parts[1]


def hausdorff(a, b):
    """The Hausdorff distance of two intervals, max(|a.lo - b.lo|, |a.hi - b.hi|), the distance of equal
    infinite bounds being 0"""
    def d(p, q):
        if is_inf(p) or is_inf(q):
            return Q0 if (is_inf(p) and p == q) else INF
        return abs(p - q)
    return max(d(a.lo, b.lo), d(a.hi, b.hi))


#---------------------------------------------------------------------------
# The roundings the original performs before a contraction
#---------------------------------------------------------------------------
def usqrt(v):
    """sqrt(v) rounded upward, as the C library does in the direction GAOL leaves"""
    return fc.ru(fc.mpq(mpmath.sqrt(mpmath.mpf(v))))


def nearest_sin(v):
    return float(mpmath.sin(mpmath.mpf(v)))


def pi_times(num, den):
    """num*M_PI/den as the original evaluates it, upward"""
    return fc.div_up(fc.mul_up(float(num), M_PI), float(den))


#---------------------------------------------------------------------------
# C++ text
#---------------------------------------------------------------------------
def dec(d):
    return fc.dec(d).replace('ibex_oo', 'ibexb_oo')


def ref_text(refs):
    """{ lo, hi } for the bounds of a reference, ibexb_empty for the empty set"""
    if refs is None:
        return 'ibexb_empty'
    return '{ %s, %s }' % (dec(refs[0]), dec(refs[1]))


def ref_cell(i):
    """The reference of an exact result: the doubles around its bounds"""
    return ref_text(i.refs())


def itv_cell(i):
    """An interval of doubles (an input)"""
    return ref_text(fc.itv_ref(i))


def real_cell(q):
    return ref_text(fc.snap_bounds(q))


class Table(fc.Table):
    def text(self):
        out = ['  // BEGIN %s (written by ibexb_cases.py: do not edit)' % self.name]
        out.append('  static const %s %s[] = {' % (self.struct, self.name))
        for r in self.rows:
            out.extend(fc.wrap(r, '    ', 120))
        out.append('  };')
        out.append('  // END %s' % self.name)
        return '\n'.join(out)


TABLES = []
CONVENTIONS = []     # (case, text): a result of IBEX that is a convention of IBEX, no result of GAOL to check
LEFT_OUT = []        # (test, text): a test not ported


def table(name, struct, fields):
    t = Table(name, struct, fields, '')
    TABLES.append(t)
    return t


row, quoted = fc.row, fc.quoted


def doubles_of(*results, default='limit'):
    """The tolerance of a row, in doubles: 0 where every finite bound of the exact results is a double,
    `default` (limit) where one is not (a quotient, a root, an exponential, an angle...)"""
    for r in results:
        if r.empty:
            continue
        for b in (r.lo, r.hi):
            if not is_inf(b):
                lo, hi = fc.snap_bounds(b)
                if lo != hi:
                    return default
    return '0'


def same_ref(a, b):
    return a.refs() == b.refs()


def check_same(case, what, a, b):
    """The variants of an IBEX helper give the same exact result (up to the signs), so that one reference
    serves them all"""
    if not same_ref(a, b):
        raise SystemExit('%s: %s differs between the variants: %s and %s' % (case, what, fc.fmt(a), fc.fmt(b)))


#---------------------------------------------------------------------------
# TestInterval: distance, rel_distance
#---------------------------------------------------------------------------
def width_of(i):
    return fc.width_of(i)


def group_distance():
    t = table('ibexb_distance_cases', 'ibexb_distance_case', 'name, x, y, result')
    oo = INF
    cases = [
        ('distance01', iv(0, 10), iv(-5, 5), 5), ('distance02', iv(0, 10), iv(5, 10), 5),
        ('distance03', iv(0, 10), iv(5, 15), 5), ('distance04', iv(0, 10), iv(1, 2), 8),
        ('distance05', iv(0, 10), iv(0, 10), 0), ('distance06', iv(0, 10), iv(-10, 20), 10),
        ('distance07', POS, iv(0, 10), oo), ('distance08', ALL, iv(0, 10), oo), ('distance09', ALL, ALL, 0),
        ('distance10', ALL, NEG, oo), ('distance11', ALL, POS, oo), ('distance12', POS, iv(-oo, 1), oo),
        ('distance13', POS, iv(0, 1), oo), ('distance14', NEG, iv(-1, oo), oo), ('distance15', NEG, iv(-oo, -1), 1),
        ('distance15b', POS, iv(1, oo), 1),
        ('distance16', iv(0, 10), iv(5, 5), 5), ('distance17', iv(0, 10), iv(0, 0), 10),
        ('distance18', iv(0, 10), iv(10, 10), 10), ('distance19', iv(0, 0), iv(0, 0), 0),
        ('distance22', iv(0, 10), iv(11, 20), 11), ('distance23', iv(0, 10), iv(0, 5), 5)]
    for name, x, y, orig in cases:
        case = 'IBEX TestInterval::' + name
        exact = hausdorff(x, y)
        fc.compare_number(case, 'distance', float(orig), exact)
        t.add(row(quoted(case), itv_cell(x), itv_cell(y), real_cell(exact)))
    for name in ('distance20', 'distance21'):
        LEFT_OUT.append(('IBEX TestInterval::' + name, 'the distance to the empty set: the radius of the other interval in '
                         'IBEX (5 and 0), a NaN in GAOL (hausdorff), no value of the Hausdorff distance both agree on'))
    fc.tally('IBEX TestInterval::distance20', 1)
    fc.tally('IBEX TestInterval::distance21', 1)


def group_relative_distance():
    t = table('ibexb_relative_distance_cases', 'ibexb_relative_distance_case', 'name, x, y, distance, width, quotient, relative')
    oo = INF
    cases = [
        ('rel_distance01', iv(0, 10), iv(-5, 5), 0.5), ('rel_distance02', iv(0, 10), iv(5, 10), 0.5),
        ('rel_distance03', iv(0, 10), iv(5, 15), 0.5), ('rel_distance04', iv(0, 10), iv(1, 2), 0.8),
        ('rel_distance05', iv(0, 10), iv(0, 10), 0), ('rel_distance06', iv(0, 10), iv(-10, 20), 1),
        ('rel_distance07', POS, iv(0, 10), 1), ('rel_distance08', ALL, iv(0, 10), 1),
        ('rel_distance09', ALL, ALL, 0), ('rel_distance10', ALL, NEG, 1), ('rel_distance11', ALL, POS, 1),
        ('rel_distance12', POS, iv(-oo, 1), 1), ('rel_distance13', POS, iv(0, 1), 1),
        ('rel_distance14', NEG, iv(-1, oo), 1), ('rel_distance15', NEG, iv(-oo, -1), 0),
        ('rel_distance16', iv(0, 10), iv(5, 5), 0.5), ('rel_distance17', iv(0, 10), iv(0, 0), 1),
        ('rel_distance18', iv(0, 10), iv(10, 10), 1), ('rel_distance19', iv(0, 0), iv(0, 0), 0),
        ('rel_distance20', iv(1, 4), iv(1.5, 3), 1.0 / 3.0), ('rel_distance21', iv(0, 10), iv(0, 5), 0.5)]
    for name, x, y, orig in cases:
        case = 'IBEX TestInterval::' + name
        d, w = hausdorff(x, y), width_of(x)
        # the quotient is a quotient of reals when the distance is finite and the width not 0 (a finite
        # distance over an infinite width is 0); IBEX's own rules give the others, 1 for an infinite distance
        # and 0 for a null width, which are no result of GAOL
        if not is_inf(d) and w != 0:
            q = Q0 if is_inf(w) else d / w
            fc.compare_number(case, 'rel_distance', float(orig), q)
            t.add(row(quoted(case), itv_cell(x), itv_cell(y), real_cell(d), real_cell(w), 'true', real_cell(q)))
        else:
            fc.tally(case, 1)
            why = ('an infinite distance' if is_inf(d) else 'a finite distance') + ' over ' + (
                'a null width' if w == 0 else ('an infinite width' if is_inf(w) else 'a finite width'))
            CONVENTIONS.append((case, 'IBEX returns %s for %s, by its convention (1 for an infinite distance, 0 for a null '
                                'width): not a quotient of reals, no result of GAOL; the distance and the width are checked'
                                % (orig, why)))
            t.add(row(quoted(case), itv_cell(x), itv_cell(y), real_cell(d), real_cell(w), 'false', 'ibexb_empty'))


#---------------------------------------------------------------------------
# TestArith: div2
#---------------------------------------------------------------------------
def x_div2_inter3(this, pair):
    """Interval::div2_inter(num, div, out2) on the exact parts: (the interval, out2)"""
    u, v = pair
    first = this.inter(u)
    if first.empty:
        return this.inter(v), EMPTY
    return first, this.inter(v)


def x_div2_inter2(this, pair):
    a, b = x_div2_inter3(this, pair)
    return a.hull(b)


def group_div2():
    t = table('ibexb_div2_cases', 'ibexb_div2_case', 'name, num, div, out1, out2, out1bis, out2bis')
    oo = INF
    cases = [
        ('div2_01', iv(6, 12), iv(0, 0), EMPTY, EMPTY), ('div2_02', iv(6, 12), iv(2, 3), iv(2, 6), EMPTY),
        ('div2_03', iv(-2, 3), iv(-1, 1), ALL, EMPTY), ('div2_04', iv(2, 3), iv(-1, 1), iv(-oo, -2), iv(2, oo)),
        ('div2_05', iv(-3, -2), iv(-1, 1), iv(-oo, -2), iv(2, oo)),
        ('div2_06', iv(2, 3), iv(-oo, 1), NEG, iv(2, oo)), ('div2_07', iv(2, 3), iv(-1, oo), iv(-oo, -2), POS),
        ('div2_08', iv(-3, -2), iv(-oo, 1), iv(-oo, -2), POS), ('div2_09', iv(-3, -2), iv(-1, oo), NEG, iv(2, oo)),
        ('div2_10', iv(2, 3), NEG, NEG, EMPTY), ('div2_11', iv(2, 3), POS, POS, EMPTY),
        ('div2_12', iv(-3, -2), NEG, POS, EMPTY), ('div2_13', iv(-3, -2), POS, NEG, EMPTY)]
    for name, num, div, o1, o2 in cases:
        case = 'IBEX TestArith::' + name
        u, v = mul_rev_pair(div, num)
        fc.compare_interval(case, 'div2 out1', o1, u)
        fc.compare_interval(case, 'div2 out2', o2, v)
        pair = (u, v)
        # the variants of check_div2, with the exact parts
        for what, start, want in (('out1', u, u), ('out2', v, v), ('all_reals', ALL, u.hull(v))):
            fc.compare_interval(case, '%s.div2_inter(x,y)' % what, want, x_div2_inter2(start, pair))
        for what, start, want1, want2 in (('out1', u, u, EMPTY), ('out2', v, v, EMPTY), ('all_reals', ALL, u, v)):
            got1, got2 = x_div2_inter3(start, pair)
            fc.compare_interval(case, '%s.div2_inter(x,y,out2)' % what, want1, got1)
            fc.compare_interval(case, '%s.div2_inter(x,y,out2) out2' % what, want2, got2)
        bis1 = bis2 = EMPTY
        # out1bis = [out1.mid(), out1.ub()] where out1 is not a point, out2bis = [out2.lb(), out2.mid()]
        if not u.empty and u.lo < u.hi:
            bis1 = Itv(Fraction(fc.nearest(fc.midpoint_of(u))), u.hi)
            start = Itv(bis1.lo, Fraction(0.5) * (u.hi + v.lo)) if (not v.empty and v.lo > u.hi) else bis1
            fc.compare_interval(case, 'out1bis.div2_inter(x,y)', bis1, x_div2_inter2(start, pair))
            got1, got2 = x_div2_inter3(bis1, pair)
            fc.compare_interval(case, 'out1bis.div2_inter(x,y,out2)', bis1, got1)
            fc.compare_interval(case, 'out1bis.div2_inter(x,y,out2) out2', EMPTY, got2)
            if not v.empty and v.lo < v.hi:
                bis2 = Itv(v.lo, Fraction(fc.nearest(fc.midpoint_of(v))))
                both = bis1.hull(bis2)
                fc.compare_interval(case, 'out1bis|out2bis.div2_inter(x,y)', both, x_div2_inter2(both, pair))
                got1, got2 = x_div2_inter3(both, pair)
                fc.compare_interval(case, 'out1bis|out2bis.div2_inter(x,y,out2)', bis1, got1)
                fc.compare_interval(case, 'out1bis|out2bis.div2_inter(x,y,out2) out2', bis2, got2)
        t.add(row(quoted(case), itv_cell(num), itv_cell(div), ref_cell(u), ref_cell(v),
                  ref_cell(bis1), ref_cell(bis2)))


#---------------------------------------------------------------------------
# TestArith: the contractions of two variables: bwd_add, bwd_sub, bwd_mul, bwd_div
#---------------------------------------------------------------------------
# The exact contractions, as the native code runs them: the second step runs on the interval of doubles
# the first one gives (.out()); the intervals returned are the exact results of the steps
def x_bwd_add(y, a, b):
    a = a.inter(y.sub(b).out())
    b = b.inter(y.sub(a).out())
    return a, b


def x_bwd_sub(y, a, b):
    a = a.inter(y.add(b).out())
    b = b.inter(a.sub(y).out())
    return a, b


def x_bwd_mul(y, a, b):
    a_exact = rel_div(y, b, a)
    b_exact = rel_div(y, a_exact.out(), b)
    return a_exact, b_exact


def x_bwd_div(y, a, b):
    a = a.inter(y.mul(b).out())
    t = rel_div(a, b, y).out()
    return a, rel_div(a, t, b)


# the calls of the checking helper of IBEX: (sign of y, sign of x1, sign of x2, order swapped)
VARIANTS = {
    'bwd_add': [(1, 1, 1, False), (1, 1, 1, True), (-1, -1, -1, False), (-1, -1, -1, True)],
    'bwd_sub': [(1, 1, 1, False), (-1, -1, -1, False), (-1, 1, 1, True), (1, -1, -1, True)],
    'bwd_mul': [(1, 1, 1, False), (1, 1, 1, True), (-1, -1, 1, True), (-1, 1, -1, True)],
    'bwd_div': [(1, 1, 1, False), (1, -1, -1, False), (-1, -1, 1, False), (-1, 1, -1, False)],
}
EXACT_BINARY = {'bwd_add': x_bwd_add, 'bwd_sub': x_bwd_sub, 'bwd_mul': x_bwd_mul, 'bwd_div': x_bwd_div}


def group_binary(fn, cases, tolerance):
    t = table('ibexb_%s_cases' % fn[4:], 'ibexb_binary_case', 'name, y, x1, x2, x1_after, x2_after, doubles')
    f = EXACT_BINARY[fn]
    for name, y, x1, x2, a1, a2 in cases:
        case = 'IBEX TestArith::' + name
        results = []
        for sy, s1, s2, swapped in VARIANTS[fn]:
            yy, b1, b2 = neg_if(y, sy < 0), neg_if(x1, s1 < 0), neg_if(x2, s2 < 0)
            if swapped:
                r2, r1 = f(yy, b2, b1)
            else:
                r1, r2 = f(yy, b1, b2)
            fc.compare_interval(case, '%s variant x1' % fn, neg_if(a1, s1 < 0), r1)
            fc.compare_interval(case, '%s variant x2' % fn, neg_if(a2, s2 < 0), r2)
            results.append((neg_if(r1, s1 < 0), neg_if(r2, s2 < 0)))
        for r in results[1:]:
            check_same(case, 'x1', results[0][0], r[0])
            check_same(case, 'x2', results[0][1], r[1])
        r1, r2 = results[0]
        t.add(row(quoted(case), itv_cell(y), itv_cell(x1), itv_cell(x2), ref_cell(r1), ref_cell(r2),
                  '0' if tolerance == 0 else doubles_of(r1, r2)))


def group_add():
    group_binary('bwd_add', [
        ('bwd_add01', iv(1, 3), iv(1, 2), iv(-10, 5), iv(1, 2), iv(-1, 2)),
        ('bwd_add02', EMPTY, iv(0.1, 2.0), iv(-10, 2), EMPTY, EMPTY),
        ('bwd_add03', iv(0, 0), iv(0, 0), iv(.1, 5), EMPTY, EMPTY),
        ('bwd_add04', iv(0, 0), iv(-1, 1), iv(-1, 1), iv(-1, 1), iv(-1, 1)),
        ('bwd_add05', iv(-1, 1), iv(1, 2), iv(-10, 5), iv(1, 2), iv(-3, 0))], 0)


def group_sub():
    group_binary('bwd_sub', [
        ('bwd_sub01', iv(1, 3), iv(1, 2), iv(-10, 5), iv(1, 2), iv(-2, 1)),
        ('bwd_sub02', EMPTY, iv(0.1, 2.0), iv(-10, 2), EMPTY, EMPTY),
        ('bwd_sub03', iv(0, 0), iv(0, 0), iv(.1, 5), EMPTY, EMPTY),
        ('bwd_sub04', iv(0, 0), iv(-1, 1), iv(-1, 1), iv(-1, 1), iv(-1, 1)),
        ('bwd_sub05', iv(-1, 1), iv(1, 2), iv(-10, 5), iv(1, 2), iv(0, 3))], 0)


def group_mul():
    group_binary('bwd_mul', [
        ('bwd_mul01', iv(1, 2), iv(0.1, 2.0), iv(-10, 2), iv(0.5, 2.0), iv(0.5, 2)),
        ('bwd_mul02', EMPTY, iv(0.1, 2.0), iv(-10, 2), EMPTY, EMPTY),
        ('bwd_mul03', iv(0, 0), iv(0.1, 2.0), iv(-10, 2), iv(0.1, 2.0), iv(0, 0)),
        ('bwd_mul04', iv(0, 0), iv(-1, 1), iv(-1, 1), iv(-1, 1), iv(-1, 1)),
        ('bwd_mul05', iv(1, 1), iv(0, 10), iv(0, 10), iv(0.1, 10.0), iv(0.1, 10.0))], None)


def group_div():
    nf = fc.next_float(0.0)
    group_binary('bwd_div', [
        ('bwd_div01', iv(1, 2), iv(0, 1), iv(2, 3), EMPTY, EMPTY),
        ('bwd_div02', iv(1, 2), iv(0, 1), iv(1, 3), iv(1, 1), iv(1, 1)),
        ('bwd_div03', iv(1, 2), iv(1, 3), iv(0, 1), iv(1, 2), iv(0.5, 1)),
        ('bwd_div04', iv(-1, 1), iv(-2, 2), iv(0, 1), iv(-1, 1), iv(0, 1)),
        ('bwd_div05', iv(-1, 1), iv(-2, 2), ZERO, ZERO, ZERO),
        ('bwd_div06', ZERO, iv(-2, 2), iv(-2, 2), ZERO, iv(-2, 2)),
        ('bwd_div07', POS, iv(0, 1), iv(-1, 0), ZERO, iv(-1, 0)),
        ('bwd_div08', iv(nf, INF), iv(0, 1), iv(-1, 0), ZERO, ZERO),
        ('bwd_div09', POS, iv(nf, 1), iv(-1, 0), EMPTY, EMPTY)], None)


#---------------------------------------------------------------------------
# TestArith: bwd_maxmin
#---------------------------------------------------------------------------
def group_maxmin():
    t = table('ibexb_maxmin_cases', 'ibexb_maxmin_case', 'name, z, x, y, x_after, y_after')
    cases = [
        ('bwd_maxmin01', EMPTY, iv(-2, -1), iv(-2, 3), EMPTY, EMPTY),
        ('bwd_maxmin02', iv(0, 1), iv(-2, -1), iv(-2, 3), iv(-2, -1), iv(0, 1)),
        ('bwd_maxmin03', iv(0, 1), iv(-2, -1), iv(2, 3), EMPTY, EMPTY),
        ('bwd_maxmin04', iv(0, 1), iv(-2, 0), iv(-2, 3), iv(-2, 0), iv(-2, 1)),
        ('bwd_maxmin05', iv(0, 1), iv(-2, 2), iv(-2, 3), iv(-2, 1), iv(-2, 1))]

    def contract(rel, z, a, b):
        a = rel(z, b, a)
        b = rel(z, a, b)
        return a, b
    for name, z, x, y, ex, ey in cases:
        case = 'IBEX TestArith::' + name
        # bwd_max(z, x, y), bwd_max(z, y, x), then bwd_min(-z, -x, -y) and bwd_min(-z, -y, -x), == compared
        r = []
        r.append(contract(rel_max, z, x, y))
        b, a = contract(rel_max, z, y, x)
        r.append((a, b))
        a, b = contract(rel_min, z.neg(), x.neg(), y.neg())
        r.append((a.neg(), b.neg()))
        b, a = contract(rel_min, z.neg(), y.neg(), x.neg())
        r.append((a.neg(), b.neg()))
        for rx, ry in r:
            fc.compare_interval(case, 'bwd_max, bwd_min x', ex, rx, 0.0)
            fc.compare_interval(case, 'bwd_max, bwd_min y', ey, ry, 0.0)
            check_same(case, 'x', r[0][0], rx)
            check_same(case, 'y', r[0][1], ry)
        t.add(row(quoted(case), itv_cell(z), itv_cell(x), itv_cell(y), ref_cell(r[0][0]), ref_cell(r[0][1])))


#---------------------------------------------------------------------------
# TestArith: bwd_sqr, bwd_log, bwd_sqrt, bwd_floor, bwd_ceil: one contraction of x by y
#---------------------------------------------------------------------------
def x_bwd_sqrt(y, x):
    return x.inter(sqr_itv(y.inter(POS)))


EXACT_PROJECTION = {'sqr': (rel_sqr, None), 'sqrt': (x_bwd_sqrt, '0'), 'log': (rel_exp, None),
                    'floor': (rel_floor, '0'), 'ceil': (rel_ceil, '0')}


def group_projections():
    t = table('ibexb_projection_cases', 'ibexb_projection_case', 'name, function, y, x, result, doubles')
    oo = INF
    e = math.e
    sqr_cases = [
        ('bwd_sqr01', iv(1, 9), iv(0, 4), iv(1, 3)), ('bwd_sqr02', iv(1, 9), iv(0, 2), iv(1, 2)),
        ('bwd_sqr03', iv(1, 9), iv(-4, 2), iv(-3, 2)), ('bwd_sqr04', iv(1, 9), iv(-4, -3), iv(-3, -3)),
        ('bwd_sqr05', iv(-oo, 9), iv(-4, 1), iv(-3, 1)), ('bwd_sqr06', iv(4, 9), iv(-1, 5), iv(2, 3)),
        ('bwd_sqr07', iv(-4, -2), ALL, EMPTY)]
    log_cases = [
        ('bwd_log01', ALL, ALL, POS), ('bwd_log02', NEG, ALL, iv(0, 1)), ('bwd_log03', POS, iv(0, 1), iv(1, 1)),
        ('bwd_log04', iv(0, 1), iv(-1, 3), iv(1, e)), ('bwd_log05', iv(-oo, 1), iv(-1, 3), iv(0, e)),
        ('bwd_log06', iv(-oo, 1), iv(-1, 2), iv(0, 2)), ('bwd_log07', iv(-oo, 1), iv(3, 4), EMPTY),
        ('bwd_log08', iv(-1, 1), iv(-0.01, 0.01), EMPTY)]
    sqrt_cases = [
        ('bwd_sqrt01', iv(1, 3), iv(0, 15), iv(1, 9)), ('bwd_sqrt02', iv(1, 3), iv(0, 2), iv(1, 2)),
        ('bwd_sqrt03', iv(1, 3), iv(-4, 2), iv(1, 2)), ('bwd_sqrt04', iv(1, 3), iv(-4, -3), EMPTY),
        ('bwd_sqrt05', iv(-oo, 9), iv(-4, 1), iv(0, 1)), ('bwd_sqrt06', iv(2, 5), iv(-1, 5), iv(4, 5)),
        ('bwd_sqrt07', iv(-4, -2), ALL, EMPTY)]
    floor_cases = [
        ('bwd_floor_01', EMPTY, ALL, EMPTY), ('bwd_floor_02', iv(-oo, -1), iv(-oo, -0.000001), iv(-oo, -0.000001)),
        ('bwd_floor_03', iv(-oo, -1), iv(-oo, 0.000001), NEG), ('bwd_floor_04', iv(-oo, -0.000001), iv(-oo, 0.000001), NEG),
        ('bwd_floor_05', iv(1, oo), iv(0.000001, oo), iv(1, oo)), ('bwd_floor_06', iv(1, oo), iv(-0.000001, oo), iv(1, oo)),
        ('bwd_floor_07', iv(0.000001, oo), iv(-0.000001, oo), iv(1, oo)), ('bwd_floor_08', iv(1, 2), iv(0.01, 2.99), iv(1, 2.99)),
        ('bwd_floor_09', iv(1, 2), iv(2.99, 3.01), iv(2.99, 3)), ('bwd_floor_10', iv(1, 2), iv(3.01, 3.99), EMPTY),
        ('bwd_floor_11', iv(0.01, 2.99), iv(0.01, 2.99), iv(1, 2.99))]
    ceil_cases = [
        ('bwd_ceil_01', EMPTY, ALL, EMPTY), ('bwd_ceil_02', iv(-oo, -1), iv(-oo, -0.000001), iv(-oo, -1)),
        ('bwd_ceil_03', iv(-oo, -1), iv(-oo, 0.000001), iv(-oo, -1)), ('bwd_ceil_04', iv(-oo, -0.000001), iv(-oo, 0.000001), iv(-oo, -1)),
        ('bwd_ceil_05', iv(1, oo), iv(0.000001, oo), iv(0.000001, oo)), ('bwd_ceil_06', iv(1, oo), iv(-0.000001, oo), POS),
        ('bwd_ceil_07', iv(0.000001, oo), iv(-0.000001, oo), POS), ('bwd_ceil_08', iv(1, 2), iv(0.01, 2.99), iv(0.01, 2)),
        ('bwd_ceil_09', iv(1, 2), iv(-0.01, 0.01), iv(0, 0.01)), ('bwd_ceil_10', iv(1, 2), iv(-1.01, -0.01), EMPTY),
        ('bwd_ceil_11', iv(0.01, 2.99), iv(0.01, 2.99), iv(0.01, 2))]
    for function, cases in (('sqr', sqr_cases), ('log', log_cases), ('sqrt', sqrt_cases), ('floor', floor_cases), ('ceil', ceil_cases)):
        f, forced = EXACT_PROJECTION[function]
        for name, y, x, orig in cases:
            case = 'IBEX TestArith::' + name
            exact = f(y, x)
            fc.compare_interval(case, 'bwd_' + function, orig, exact, 0.0 if function in ('floor', 'ceil') else ERROR)
            t.add(row(quoted(case), quoted(function), itv_cell(y), itv_cell(x), ref_cell(exact),
                      forced if forced else doubles_of(exact)))
    for name in ('bwd_floor_12', 'bwd_floor_13', 'bwd_ceil_12', 'bwd_ceil_13'):
        LEFT_OUT.append(('IBEX TestArith::' + name, 'empty body: the only statement is a comment'))


#---------------------------------------------------------------------------
# TestArith: bwd_sin (and bwd_cos)
#---------------------------------------------------------------------------
def group_trig():
    t = table('ibexb_trig_cases', 'ibexb_trig_case', 'name, y, x, result, doubles')
    pU = Fraction(PI_HI)
    h = Fraction(1, 2)
    y_sin = fc.f_sincos(iv(0.5, 1.5), False).out()         # sin(Interval(0.5,1.5)): the tightest doubles
    s05 = nearest_sin(0.5)
    y_tail = iv(s05, 1.0)
    pu, pl = PI_HI, PI_LO
    two_pu = 2 * pu
    five_half = fc.div_up(fc.mul_up(5.0, pu), 2.0)
    minus_three_half = fc.div_up(fc.mul_up(-3.0, pu), 2.0)

    def Q(a, b):
        return Itv(Fraction(a), Fraction(b))
    cases = [
        ('bwd_sin01', y_sin, iv(0, pu / 2.0), Q(h, 3 * h)),
        ('bwd_sin02', y_sin, iv(0, five_half), Itv(h, 2 * pU + 3 * h)),
        ('bwd_sin03', y_sin, iv(-two_pu, pu / 2.0), Itv(-2 * pU + h, 3 * h)),
        ('bwd_sin04', y_sin, iv(-two_pu, five_half), Itv(-2 * pU + h, 2 * pU + 3 * h)),
        ('bwd_sin05', y_sin, iv(fc.sub_up(-pl, 0.4), pu / 2.0), Q(h, 3 * h)),
        ('bwd_sin06', y_sin, iv(fc.sub_up(-pu, 0.5), pu / 2.0), Itv(-pU - h, 3 * h)),
        ('bwd_sin07', y_sin, iv(minus_three_half, pu / 2.0), Itv(-pU - 3 * h, 3 * h)),
        ('bwd_sin08', y_sin, iv(0.5, fc.sub_up(pl, 1.6)), Q(h, 3 * h)),
        ('bwd_sin09', y_sin, iv(0.5, fc.sub_up(pu, 1.5)), Itv(h, pU - 3 * h)),
        ('bwd_sin10', y_sin, iv(0.5, pu), Itv(h, pU - h)),
        ('bwd_sin11', y_sin, iv(fc.sub_up(-pu, 0.5), fc.sub_up(pu, 1.5)), Itv(-pU - h, pU - 3 * h)),
        ('bwd_sin12', y_sin, iv(minus_three_half, pu), Itv(-pU - 3 * h, pU - h)),
        ('bwd_sin13', y_sin, iv(-pu / 2.0, pu / 2.0), Q(h, 3 * h)),
        ('bwd_sin14', iv(2, 3), ALL, EMPTY),
        ('bwd_sin15', y_tail, iv(0, pu / 2.0), Itv(h, pU / 2)),
        ('bwd_sin16', y_tail, iv(0, five_half), Itv(h, 5 * pU / 2)),
        ('bwd_sin17', y_tail, iv(-two_pu, pu / 2.0), Itv(-2 * pU + h, pU / 2)),
        ('bwd_sin18', y_tail, iv(-two_pu, five_half), Itv(-2 * pU + h, 5 * pU / 2)),
        ('bwd_sin19', y_tail, iv(fc.sub_up(-pl, 0.4), pu / 2.0), Itv(h, pU / 2)),
        ('bwd_sin20', y_tail, iv(fc.sub_up(-pu, 0.5), pu / 2.0), Itv(-pU - h, pU / 2)),
        ('bwd_sin21', y_tail, iv(minus_three_half, pu / 2.0), Itv(-3 * pU / 2, pU / 2)),
        ('bwd_sin22', y_tail, iv(0.5, pu), Itv(h, pU - h)),
        ('bwd_sin23', y_tail, iv(minus_three_half, pu), Itv(-3 * pU / 2, pU - h)),
        ('bwd_sin24', iv(1.0, 1.0), iv(4.0, 6.0), EMPTY)]
    for name, y, xb, orig in cases:
        case = 'IBEX TestArith::' + name
        x_minus = xb.sub(HALF_PI).out()          # xbefore - Interval::half_pi()
        x_plus = xb.add(HALF_PI).out()
        r = [rel_sin(y, xb), rel_sin(y.neg(), xb.neg()), rel_cos(y, x_minus), rel_cos(y.neg(), x_plus)]
        # the original: sin(x) in y -> xafter; sin(-x) in -y -> -xafter; cos(x - pi/2) in y and
        # cos(x + pi/2) in -y -> xafter -/+ pi/2
        wants = [orig, orig.neg(), orig.sub(HALF_PI), orig.add(HALF_PI)]
        for what, want, got in zip(('sin(x)', 'sin(-x)', 'cos(x-pi/2)', 'cos(x+pi/2)'), wants, r):
            fc.compare_interval(case, 'bwd_sin, ' + what, want, got)
        check_same(case, 'sin(-x)', r[0].neg(), r[1])
        t.add('{ %s, %s, %s, { %s }, %s }' % (quoted(case), itv_cell(y), itv_cell(xb), ', '.join(ref_cell(i) for i in r),
                                                 doubles_of(*r)))


#---------------------------------------------------------------------------
# TestArith: bwd_pow
#---------------------------------------------------------------------------
def group_pow():
    t = table('ibexb_pow_cases', 'ibexb_pow_case', 'name, y, x, p, result, doubles')
    cases = [
        ('bwd_pow01', iv(16, 81), iv(1, 4), iv(2, 3), 4), ('bwd_pow02', iv(16, 81), iv(3, 4), iv(3, 3), 4),
        ('bwd_pow03', iv(16, 81), iv(-1, 4), iv(2, 3), 4), ('bwd_pow04', iv(16, 81), iv(-2, 4), iv(-2, 3), 4),
        ('bwd_pow05', iv(16, 81), iv(-5, 4), iv(-3, 3), 4), ('bwd_pow06', iv(16, 81), iv(1, 1), EMPTY, 4),
        ('bwd_pow07', iv(16, 81), iv(4, 4), EMPTY, 4), ('bwd_pow08', iv(8, 27), iv(1, 4), iv(2, 3), 3),
        ('bwd_pow09', iv(8, 27), iv(3, 4), iv(3, 3), 3), ('bwd_pow10', iv(8, 27), iv(-5, 4), iv(2, 3), 3),
        ('bwd_pow11', iv(-8, 27), iv(-5, 4), iv(-2, 3), 3), ('bwd_pow12', iv(-27, 27), iv(-5, 4), iv(-3, 3), 3),
        ('bwd_pow13', iv(8, 27), iv(1, 1), EMPTY, 3), ('bwd_pow14', iv(8, 27), iv(4, 4), EMPTY, 3),
        ('bwd_pow15', iv(-27, -8), iv(-1, -1), EMPTY, 3), ('bwd_pow16', iv(-27, -8), iv(-4, -4), EMPTY, 3),
        ('bwd_pow17', iv(0, 1), iv(-10, 10), iv(-10, 10), -2)]
    for name, y, x, orig, p in cases:
        case = 'IBEX TestArith::' + name
        r1 = rel_pow(y, p, x)
        r2 = rel_pow(y if p % 2 == 0 else y.neg(), p, x.neg())
        fc.compare_interval(case, 'bwd_pow(y,p,x)', orig, r1)
        fc.compare_interval(case, 'bwd_pow(-y,p,-x)', orig.neg(), r2)
        check_same(case, 'x', r1.neg(), r2)
        t.add(row(quoted(case), itv_cell(y), itv_cell(x), str(p), ref_cell(r1), doubles_of(r1, r2)))


#---------------------------------------------------------------------------
# TestArith: bwd_atan2, bwd_atan, bwd_imod
#---------------------------------------------------------------------------
def group_atan2():
    t = table('ibexb_atan2_cases', 'ibexb_atan2_case', 'name, theta, y, x, y_after, x_after, doubles')
    pi_4 = M_PI / 4.0
    s3, s2 = usqrt(3.0), usqrt(2.0)          # sqrt(3.) and sqrt(2.), upward
    r3, r2 = math.sqrt(3.), math.sqrt(2.)    # in the expected values, which are only compared

    def angle(num, den):                     # num*M_PI/den
        return pi_times(num, den)
    third = fc.div_up(-M_PI, 3.)             # -M_PI/3.
    cases = [
        ('bwd_atan2_01', iv(angle(1, 6), angle(1, 3)), iv(.5, 10.), iv(.5, 2.), iv(.5, 2. * r3), iv(.5, 2.)),
        ('bwd_atan2_02', iv(angle(1, 6), angle(1, 3)), iv(.5, 2.), iv(.5, 10.), iv(.5, 2.), iv(.5, 2. * r3)),
        ('bwd_atan2_03', iv(-pi_4, pi_4), iv(1., 2.), iv(.5, 2.), iv(1., 2.), iv(1., 2.)),
        ('bwd_atan2_04', iv(-M_PI / 2., 0.), iv(.5, 2.), iv(.5, 10.), EMPTY, EMPTY),
        ('bwd_atan2_05', iv(2. * M_PI, fc.mul_up(3., M_PI)), iv(-.5, 2.), iv(.5, 10.), EMPTY, EMPTY),
        ('bwd_atan2_06', iv(angle(2, 3), angle(5, 6)), iv(0., 100.), iv(-20., -s3 / 2.), iv(.5, 20. * r3), iv(-20., -r3 / 2)),
        ('bwd_atan2_07', iv(angle(-3, 4), angle(-2, 3)), iv(-s3 / 2., 2.), iv(-s2 / 2., 0.), iv(-r3 / 2., 0.), iv(-r2 / 2., 0.)),
        ('bwd_atan2_08', iv(angle(-3, 4), angle(-2, 3)), iv(-s3 / 2., 2.), iv(-1., -.5), iv(-r3 / 2., -.5), iv(-r3 / 2., -.5)),
        ('bwd_atan2_09', iv(angle(-3, 4), -pi_4), iv(-5., -.5), ALL, iv(-5., -.5), iv(-5., 5.)),
        ('bwd_atan2_10', iv(third, pi_4), ALL, pt(s3 / 2.), iv(-1.5, r3 / 2.), pt(r3 / 2.)),
        ('bwd_atan2_issue134', iv(PI_LO / 2, PI_HI / 2), ONE, ZERO, ONE, ZERO)]
    for name, theta, y, x, ay, ax in cases:
        case = 'IBEX TestArith::' + name
        # the first contraction on the box, the second on the box with the doubles of the first
        y_exact = rel_atan2_y(theta, x, y)
        y1 = y_exact.out()
        x_exact = rel_atan2_x(theta, y1, x)
        fc.compare_interval(case, 'bwd_atan2 y', ay, y_exact)
        fc.compare_interval(case, 'bwd_atan2 x', ax, x_exact)
        t.add(row(quoted(case), itv_cell(theta), itv_cell(y), itv_cell(x), ref_cell(y_exact), ref_cell(x_exact),
                  'ibexb_atan2_limit' if doubles_of(y_exact, x_exact) != '0' else '0'))


def group_atan():
    t = table('ibexb_atan_cases', 'ibexb_atan_case', 'name, y, x, result, mirror, doubles')
    pi2 = M_PI / 2.0
    tan01 = fc.f_tan(iv(0., fc.div_up(M_PI, 6.))).out()
    cases = [
        ('bwd_atan01', iv(0., fc.div_up(M_PI, 6.)), iv(-1., 3.), tan01, True),
        ('bwd_atan02', iv(-M_PI, 1.5), iv(0, fc.div_up(fc.mul_up(5., M_PI), 2.0)), iv(0, fc.div_up(fc.mul_up(5., M_PI), 2.0)), True),
        ('bwd_atan03', iv(0., fc.div_up(M_PI, 6.)), iv(.2, .5), iv(.2, .5), True),
        ('bwd_atan04', iv(fc.sub_up(-pi2, 0.1), fc.add_up(pi2, 0.1)), iv(-100, 100), iv(-100, 100), True),
        ('bwd_atan05', iv(fc.add_up(pi2, 0.1), M_PI), iv(-100, 100), EMPTY, True),
        ('bwd_atan06', iv(-M_PI, fc.sub_up(-pi2, 0.1)), iv(-100, 100), EMPTY, True),
        ('bwd_atan07', iv(-M_PI / 4, pi2), ALL, None, False),
        ('bwd_atan08', iv(-pi2, M_PI / 4.), ALL, None, False)]
    for name, y, x, orig, mirror in cases:
        case = 'IBEX TestArith::' + name
        r = rel_atan(y, x)
        if name == 'bwd_atan07':
            fc.compare_number(case, 'x.lb()', -1.0, r.lo)
            fc.compare_predicate(case, 'x.ub() > 1000', r.hi > 1000)
        elif name == 'bwd_atan08':
            fc.compare_number(case, 'x.ub()', 1.0, r.hi)
            fc.compare_predicate(case, 'x.lb() < -1000', r.lo < -1000)
        else:
            fc.compare_interval(case, 'bwd_atan(y,x)', orig, r)
            m = rel_atan(y.neg(), x.neg())
            fc.compare_interval(case, 'bwd_atan(-y,-x)', orig.neg(), m)
            check_same(case, 'mirror', r.neg(), m)
        t.add(row(quoted(case), itv_cell(y), itv_cell(x), ref_cell(r), 'true' if mirror else 'false', doubles_of(r)))


def group_imod():
    t = table('ibexb_imod_cases', 'ibexb_imod_case', 'name, period, x, y, x_after, y_after')
    pi = mpmath.pi

    def mp_itv(a, b):
        return Itv(fc.mpq(a), fc.mpq(b))
    twopi = 2. * M_PI
    cases = [
        ('bwd_imod_01', 3., iv(3., 5.), iv(1., 2.), iv(4., 5.), iv(1., 2.)),
        ('bwd_imod_02', 2., iv(7., 8.), iv(.5, 2.), iv(7., 8.), iv(1., 2.)),
        ('bwd_imod_03', 2., iv(7., 8.), iv(0., 2.), iv(7., 8.), iv(0., 2.)),
        ('bwd_imod_04', twopi, iv(twopi, fc.mul_up(3., M_PI)), iv(pi_times(1, 6), M_PI / 2.),
         mp_itv(13 * pi / 6, 5 * pi / 2), iv(pi_times(1, 6), M_PI / 2.)),
        ('bwd_imod_05', twopi, iv(fc.mul_up(3., M_PI), fc.mul_up(4., M_PI)), iv(pi_times(1, 3), M_PI / 2.), EMPTY, EMPTY),
        ('bwd_imod_06', twopi, iv(fc.mul_up(3., M_PI), fc.mul_up(4., M_PI)), iv(0., M_PI / 2.), pt(4 * M_PI), pt(0.)),
        ('bwd_imod_07', twopi, iv(twopi, fc.mul_up(4., M_PI)), iv(pi_times(-1, 6), M_PI / 2.), iv(twopi, fc.mul_up(4., M_PI)), iv(pi_times(-1, 6), M_PI / 2.)),
        ('bwd_imod_08', twopi, iv(fc.div_up(fc.mul_up(7., M_PI), 4.), fc.div_up(fc.mul_up(8., M_PI), 3)), iv(-M_PI / 2, M_PI / 2.),
         mp_itv(7 * pi / 4, 5 * pi / 2), mp_itv(-pi / 4, pi / 2))]
    for name, p, x, y, ax, ay in cases:
        case = 'IBEX TestArith::' + name
        rx, ry = rel_imod(x, y, p)
        fc.compare_interval(case, 'bwd_imod x', ax, rx)
        fc.compare_interval(case, 'bwd_imod y', ay, ry)
        t.add(row(quoted(case), dec(p), itv_cell(x), itv_cell(y), ref_cell(rx), ref_cell(ry)))


GROUPS = [group_distance, group_relative_distance, group_div2, group_mul, group_div, group_maxmin, group_projections,
          group_trig, group_pow, group_atan2, group_atan, group_add, group_sub, group_imod]


def build():
    del TABLES[:]
    del fc.FINDINGS[:]
    del fc.NOTES[:]
    del CONVENTIONS[:]
    del LEFT_OUT[:]
    fc.CHECKS[0] = 0
    fc.ORIGINAL_CHECKS.clear()
    for g in GROUPS:
        g()
    return TABLES


#---------------------------------------------------------------------------
# The oracles against sampling
#---------------------------------------------------------------------------
WINDOW = 40.0       # the samples of an unbounded interval go from -WINDOW to WINDOW


def selftest():
    rng = random.Random(20261103)
    failures = [0]

    def fail(what, detail):
        failures[0] += 1
        if failures[0] <= 30:
            print('SELFTEST FAILED %s: %s' % (what, detail))

    def grid(i, n):
        """The points of an interval, the bounds first and last, an infinite bound being +-WINDOW"""
        lo = -WINDOW if is_inf(i.lo) else float(i.lo)
        hi = WINDOW if is_inf(i.hi) else float(i.hi)
        if lo == hi:
            return [lo], 1.0
        return [lo + (hi - lo) * k / (n - 1) for k in range(n)], (hi - lo) / (n - 1)

    def short(i):
        if i.empty:
            return 'empty'
        f = lambda v: str(v) if is_inf(v) else ('%.6g' % float(v) if abs(v) < 2 ** 1000 else ('big' if v > 0 else '-big'))
        return '[%s, %s]' % (f(i.lo), f(i.hi))

    def hull_close(got, members, step, what, args):
        """got is the hull of the members of a grid of step: it holds them, and the members come within a step
        and a half of its bounds, those of a window (+-WINDOW) for an infinite one"""
        if not members:
            return          # the set may fall between the grid points (a point, a thin piece)
        if got.empty:
            fail(what, ('empty result, members from %s to %s' % (min(members), max(members)), args))
            return
        lo, hi = min(members), max(members)
        glo = -math.inf if is_inf(got.lo) else float(got.lo)
        ghi = math.inf if is_inf(got.hi) else float(got.hi)
        if lo < glo - 1e-9 or hi > ghi + 1e-9:
            fail(what, ('a member outside the hull', args, short(got), lo, hi))
        edge = WINDOW - 1.5 * step
        if (glo == -math.inf and lo > -edge) or (glo > -edge and lo - glo > 1.5 * step + 1e-9):
            fail(what, ('the hull reaches below the members', args, short(got), lo))
        if (ghi == math.inf and hi < edge) or (ghi < edge and ghi - hi > 1.5 * step + 1e-9):
            fail(what, ('the hull reaches above the members', args, short(got), hi))

    def rand_itv(vals):
        while True:
            a, b = sorted(rng.choice(vals) for _ in range(2))
            if not iv(a, b).empty:
                return iv(a, b)
    pal = [-4.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0, 5.0, INF, -INF]
    # 1. div_rel: x is a member when 0 is in K and some y of J is any (x = 0 too), or K/x meets J
    for _ in range(300):
        k, j, i = rand_itv(pal), rand_itv(pal), rand_itv(pal)
        pts, step = grid(i, 401)

        def member(x):
            q = Fraction(x)
            if q == 0:
                return k.lo <= 0 <= k.hi
            kx = k.div(Itv(q, q)) if q != 0 else EMPTY
            return not kx.inter(j).empty
        hull_close(rel_div(k, j, i), [x for x in pts if member(x)], step, 'div_rel', (short(k), short(j), short(i)))
        # the zero case: 0 = x * 0
        if k.lo <= 0 <= k.hi and j.lo <= 0 <= j.hi and not i.empty and rel_div(k, j, i).refs() != i.refs():
            fail('div_rel zero', (short(k), short(j), short(i)))
    # 2. sqrt_rel, nth_root_rel, the powers
    for _ in range(300):
        y, x = rand_itv(pal), rand_itv(pal)
        pts, step = grid(x, 401)
        for n in (2, 3, 4):
            hull_close(rel_pown(y, n, x), [v for v in pts if not y.inter(Itv(Fraction(v) ** n, Fraction(v) ** n)).empty], step,
                       'nth_root_rel', (short(y), short(x), n))
        hull_close(rel_sqr(y, x), [v for v in pts if not y.inter(Itv(Fraction(v) ** 2, Fraction(v) ** 2)).empty], step, 'sqrt_rel',
                   (short(y), short(x)))
        hull_close(rel_pow(y, -2, x), [v for v in pts if v != 0 and not y.inter(Itv(1 / Fraction(v) ** 2, 1 / Fraction(v) ** 2)).empty],
                   step, 'pow -2', (short(y), short(x)))
    # 3. max and min: some y of Y with max(x, y) in Z, the values max(x, y) filling [max(x, inf Y), max(x, sup Y)]
    for _ in range(300):
        z, y, x = rand_itv(pal), rand_itv(pal), rand_itv(pal)
        pts, step = grid(x, 401)

        def reach(f, v):
            q = Fraction(v)
            return Itv(f(q, y.lo), f(q, y.hi))
        mx = [v for v in pts if not y.empty and not reach(max, v).inter(z).empty]
        mn = [v for v in pts if not y.empty and not reach(min, v).inter(z).empty]
        hull_close(rel_max(z, y, x), mx, step, 'max_rel', (short(z), short(y), short(x)))
        hull_close(rel_min(z, y, x), mn, step, 'min_rel', (short(z), short(y), short(x)))
    # 4. the periodic preimages
    for _ in range(150):
        y = rand_itv([-1.5, -1.0, -0.5, 0.0, 0.3, 0.5, 0.9, 1.0, 1.5])
        a = rng.uniform(-12, 12)
        x = iv(a, a + rng.choice([0.5, 2.0, 5.0, 9.0]))
        pts, step = grid(x, 4001)
        for name, rel, f in (('sin', rel_sin, math.sin), ('cos', rel_cos, math.cos)):
            hull_close(rel(y, x), [v for v in pts if float(y.lo) - 1e-12 <= f(v) <= float(y.hi) + 1e-12], step, 'rel_' + name, (short(y), short(x)))
    # 5. the preimage of atan
    for _ in range(100):
        y = rand_itv([-4.0, -1.6, -1.5, -0.5, 0.0, 0.7, 1.5, 1.6, 4.0])
        x = rand_itv([-30.0, -3.0, -1.0, 0.0, 1.0, 3.0, 30.0, INF, -INF])
        pts, step = grid(x, 4001)
        hull_close(rel_atan(y, x), [v for v in pts if float(y.lo) <= math.atan(v) <= float(y.hi)], step, 'rel_atan', (short(y), short(x)))
    # 6. the angles: the hulls are those of a second computation, in floats (bounded boxes, up to 1e7), and
    # the points of a grid of the box in the wedge are in them
    angles = [-3.5, -3.14159, -2.5, -1.5707, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5707, 2.5, 3.14159, 4.0]
    boxes = [-1e7, -4.0, -2.0, -1.0, 0.0, 1.0, 2.0, 4.0, 1e7]
    for _ in range(150):
        c, x, y = rand_itv(angles), rand_itv(boxes), rand_itv(boxes)
        gy, gx = cone_projections(c, x, y)
        other = cone_projections_float(c, x, y)
        args = (short(c), short(x), short(y))
        if (other is None) != gy.empty or (gy.empty != gx.empty):
            fail('cone emptiness', (args, short(gy), short(gx), other))
            continue
        if other is not None:
            for got, want in ((gy, other[0]), (gx, other[1])):
                for bound, ref in ((got.lo, want[0]), (got.hi, want[1])):
                    if abs(float(bound) - ref) > 1e-7 * (1 + abs(ref)):
                        fail('cone', (args, short(gy), short(gx), other))
        if c.lo != c.hi:
            xs, step_x = grid(x, 121)
            ys, step_y = grid(y, 121)
            for vx in xs:
                for vy in ys:
                    if (vx, vy) == (0.0, 0.0) or gy.empty or gx.empty:
                        continue
                    angle = math.atan2(vy, vx)
                    if (float(c.lo) <= angle <= float(c.hi) or (vy == 0.0 and vx < 0 and float(c.lo) <= math.pi <= float(c.hi))) and not (
                            float(gy.lo) - 1e-6 <= vy <= float(gy.hi) + 1e-6 and float(gx.lo) - 1e-6 <= vx <= float(gx.hi) + 1e-6):
                        fail('cone member', (args, vx, vy, short(gy), short(gx)))
    # 7. the two-output division: the members are in u or in v, none between them
    for _ in range(120):
        b, c = rand_itv(pal), rand_itv(pal)
        u, v = mul_rev_pair(b, c)
        step = 2 * WINDOW / 1600
        members = []
        for k in range(1601):
            x = -WINDOW + k * step
            q = Fraction(x)
            # b * x in c for a b of b: b in c / x
            if (c.lo <= 0 <= c.hi) if q == 0 else not c.div(Itv(q, q)).inter(b).empty:
                members.append(x)
        hull_close(u.hull(v), members, step, 'mulRevToPair hull', (short(b), short(c)))
        parts = [part for part in (u, v) if not part.empty]
        for x in members:
            if not any(float(part.lo) - 1e-9 <= x <= float(part.hi) + 1e-9 for part in parts):
                fail('mulRevToPair gap', (short(b), short(c), short(u), short(v), x))
                break
        if u.empty and not v.empty:
            fail('mulRevToPair second without first', (short(b), short(c), short(u), short(v)))
        if not u.empty and not v.empty and u.hi > v.lo:
            fail('mulRevToPair order', (short(b), short(c), short(u), short(v)))
    # 8. floor, ceil
    halves = [-2.5, -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.5]
    for _ in range(300):
        z, x = rand_itv(halves), rand_itv(halves + [INF, -INF])
        pts, step = grid(x, 801)
        hull_close(rel_floor(z, x), [v for v in pts if float(z.lo) <= math.floor(v) <= float(z.hi)], step, 'floor_rel', (short(z), short(x)))
        hull_close(rel_ceil(z, x), [v for v in pts if float(z.lo) <= math.ceil(v) <= float(z.hi)], step, 'ceil_rel', (short(z), short(x)))
    # 9. the modulo, on grids of quarters, a period of 2, 3 or 6
    for _ in range(200):
        p = rng.choice([2.0, 3.0, 6.0])
        x, y = rand_itv([0.0, 1.0, 2.5, 4.0, 7.0, 9.0, 12.0]), rand_itv([-1.0, 0.0, 0.5, 2.0, 3.0, 5.0])
        gx, gy = rel_imod(x, y, p)
        xs = [float(x.lo) + 0.25 * k for k in range(int((float(x.hi) - float(x.lo)) / 0.25) + 1)]
        ys = [float(y.lo) + 0.25 * k for k in range(int((float(y.hi) - float(y.lo)) / 0.25) + 1)]
        pts = [(vx, vy) for vx in xs for vy in ys if (vx - vy) % p == 0.0]
        hull_close(gx, [q[0] for q in pts], 0.25, 'imod x', (p, short(x), short(y)))
        hull_close(gy, [q[1] for q in pts], 0.25, 'imod y', (p, short(x), short(y)))
    print('selftest: %d failures' % failures[0])
    return 1 if failures[0] else 0


#---------------------------------------------------------------------------
# The command line
#---------------------------------------------------------------------------
def write(path):
    text = open(path).read()
    for t in build():
        pattern = re.compile(r'  // BEGIN %s\b.*?  // END %s\b' % (t.name, t.name), re.S)
        if not pattern.search(text):
            sys.exit('ibexb_cases.py: %s: no BEGIN/END of %s' % (path, t.name))
        text = pattern.sub(lambda m: t.text(), text, count=1)
    open(path, 'w').write(text)
    print('%s: %d tables, %d rows written' % (path, len(TABLES), sum(len(t.rows) for t in TABLES)))


def check(path):
    found = fc.blocks_of(path)
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
        if not name.startswith('ibexb_'):
            continue  # a table of another section of the file, checked by its own script
        print('%s: the table %s is not generated' % (path, name))
        bad += 1
    print('%s: %d tables, %d rows checked, %s' % (path, len(TABLES), sum(len(t.rows) for t in TABLES),
                                                  'all as generated' if not bad else '%d tables differ' % bad))
    return 1 if bad else 0


# Disagreements known and explained: (IBEX test, text up to the colon) -> why
KNOWN = {}


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
    print('%d checks of IBEX compared with the exact results (tolerance %g, absolute; none for ==)' % (fc.CHECKS[0], ERROR))
    unknown = 0
    for case, text in fc.distinct(fc.FINDINGS):
        why = KNOWN.get((case, text.split(':')[0]))
        print('DISAGREEMENT %s: %s%s' % (case, text, ('  [known: %s]' % why) if why else ''))
        unknown += why is None
    notes = fc.distinct(fc.NOTES)
    print('%d disagreements, %d expectations within the tolerance that do not enclose the exact result, '
          '%d conventions of IBEX, %d tests left out' % (len(fc.distinct(fc.FINDINGS)), len(notes), len(CONVENTIONS), len(LEFT_OUT)))
    if verbose:
        for case, text in notes:
            print('note %s: %s' % (case, text))
        for case, text in CONVENTIONS:
            print('convention %s: %s' % (case, text))
        for case, text in LEFT_OUT:
            print('left out %s: %s' % (case, text))
        rows = row_names()
        print('%-34s %7s %6s' % ('IBEX test', 'checks', 'rows'))
        for test in sorted(set(rows) | set(fc.ORIGINAL_CHECKS), key=lambda n: (n.split('::')[0], n)):
            print('%-34s %7d %6d' % (test, fc.ORIGINAL_CHECKS.get(test, 0), rows.get(test, 0)))
    return 1 if unknown else 0


def main():
    p = argparse.ArgumentParser(description='The backward tests of IBEX as native GAOL tests: rows, references, checks.')
    p.add_argument('--write', metavar='FILE', help='replace the generated tables of FILE')
    p.add_argument('--check', metavar='FILE', help='check that the generated tables of FILE are what is generated')
    p.add_argument('--originals', action='store_true', help='report the expectations of IBEX that differ from the exact results')
    p.add_argument('-v', '--verbose', action='store_true', help='with --originals: the notes, the conventions, and the checks and rows of each test')
    p.add_argument('--selftest', action='store_true', help='check the oracles against sampling')
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
