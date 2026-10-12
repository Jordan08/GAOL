#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tests of GAOL v5: generates atan2_rel_values.h, the cases of atan2Rev1 and
# atan2Rev2, the reverse functions of atan2 of IEEE 1788-2015 (10.5.4,
# Table 10.1), which tests/reverse.cpp checks atan2_rel_y() and atan2_rel_x()
# with.
#
# For an interval of angles c and a box y x x, S is the set of the points
# (x, y) of the box, (0, 0) left out, whose angle atan2(y, x), in (-pi, pi],
# is in c, and IEEE 1788 defines
#     atan2Rev1(x, c, y) = hull{ y | (x, y) in S },
#     atan2Rev2(y, c, x) = hull{ x | (x, y) in S },
# which it asks (12.10.2) to be valid, enclosing the hull, to be accurate for
# inf-sup types, within nextOut(tightest(nextOut(arguments))) (12.10.1), and
# to be empty when an argument is. For each case, this script computes the
# tightest enclosures of both hulls from that definition, and that bound of
# the accurate results, with mpmath at 3000 bits, otherwise than GAOL does:
# the angles of c within (-pi, pi] are cut at the axes into pieces, each the
# angles of a closed convex cone K, and the hull of the points of K in the box
# is the hull of the vertices of that convex polygon, which are among the
# corners of the box within K (their angle computed by mpmath), the points
# where the two half-lines bounding K cross the lines of the sides of the box
# within it, and the apex (0, 0). An infinite bound of the box is taken as
# 2^4000, beyond which the hull of no case changes (the slopes of the cones
# are above 2^-1075 and below 2^1075): a hull reaching 2^1100 is unbounded.
# 3000 bits tell the tangent of the least subnormal angle, 2^-1074, from the
# angle itself, which it exceeds by 2^-2148 of it (2000 bits do not), and the
# angle of a point of doubles from a bound of c, a double, which it is never.
# The point (0, 0) is left out, and so is the half-line y = 0, x < 0 from the
# piece of the angles next to -pi, whose angle is pi: a piece whose vertices
# all lie in what is left out holds no point of S; otherwise S is dense in the
# polygon, which has the same hull.
#
# The cases: angles at, next to and around the multiples of pi/4, the axes
# and the half-line y = 0, x < 0 where atan2 jumps from pi to -pi, angles
# outside [-pi, pi], tiny ones, the whole line and the empty set, on boxes in
# each quadrant, across the axes and the half-line, with zero, tiny, huge and
# infinite bounds; and random cases.
#
#     python3 tests/atan2_rel_values.py > tests/atan2_rel_values.h
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
import struct
import sys

import mpmath

mpmath.mp.prec = 3000
INF = math.inf
MAX = sys.float_info.max
TINY = math.ldexp(1.0, -1074)
EMPTY = "empty"
W = (-INF, INF)

PI = +mpmath.pi
HALF_PI = PI / 2
ZERO = mpmath.mpf(0)
BIG = mpmath.mpf(2) ** 4000      # an infinite bound of the box
HUGE = mpmath.mpf(2) ** 1100     # a hull beyond it is unbounded


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


def round_down(v):
    """The greatest double at most v, an mpf"""
    if v >= HUGE:
        return MAX
    if v <= -HUGE:
        return -INF
    if v > MAX:
        return MAX
    if v < -MAX:
        return -INF
    d = float(v)
    return next_down(d) if mpmath.mpf(d) > v else d


def round_up(v):
    r = -round_down(-v)
    return 0.0 if r == 0.0 else r  # +0 rather than the -0 of -0.0


def next_out(a):
    """nextOut of IEEE 1788 (12.10.1): an interval widened by one double"""
    if a == EMPTY:
        return EMPTY
    return (next_down(a[0]), next_up(a[1]))


def mpf_bound(d):
    """A bound of the box as an mpf, an infinite one being +-2^4000"""
    if d == INF:
        return BIG
    if d == -INF:
        return -BIG
    return mpmath.mpf(d)


#---------------------------------------------------------------------------
# The angles
#---------------------------------------------------------------------------
directions = {}


def direction(theta):
    """The unit vector of the angle theta, a double"""
    if theta not in directions:
        t = mpmath.mpf(theta)
        directions[theta] = (mpmath.cos(t), mpmath.sin(t))
    return directions[theta]


# The ends of the quadrants, with their exact directions; -pi and pi are the
# same half-line
ENDS = [(-PI, (-1, 0)), (-HALF_PI, (0, -1)), (ZERO, (1, 0)), (HALF_PI, (0, 1)), (PI, (-1, 0))]


def angle(px, py):
    """atan2(py, px) for a point other than (0, 0), exact on the axes, where it
    is one of the ENDS, so that it compares with them as it should"""
    if py == 0:
        return ZERO if px > 0 else PI
    if px == 0:
        return HALF_PI if py > 0 else -HALF_PI
    return mpmath.atan2(py, px)


def pieces(c):
    """The angles of c within (-pi, pi], cut at the axes: (lo, dlo, hi, dhi,
    open), lo and hi the angles, dlo and dhi their directions, open telling
    that the piece holds -pi, which is no angle"""
    cl, cu = mpf_bound(c[0]), mpf_bound(c[1])
    out = []
    for (q0, d0), (q1, d1) in zip(ENDS, ENDS[1:]):
        lo, dlo = (q0, d0) if cl <= q0 else (cl, direction(c[0]))
        hi, dhi = (q1, d1) if cu >= q1 else (cu, direction(c[1]))
        if lo <= hi:
            out.append((lo, dlo, hi, dhi, lo is ENDS[0][0]))
    return out


#---------------------------------------------------------------------------
# The hulls
#---------------------------------------------------------------------------
def in_cone(p, piece):
    lo, _, hi, _, open_piece = piece
    px, py = p
    if px == 0 and py == 0:
        return True
    if open_piece and py == 0 and px < 0:
        return True  # the half-line of -pi, which the closed cone holds
    a = angle(px, py)
    return lo <= a <= hi


def vertices(piece, box):
    """The points among which the vertices of the polygon of the points of the
    cone of piece in box = (xl, xu, yl, yu) are"""
    xl, xu, yl, yu = box
    out = []
    for p in [(xl, yl), (xl, yu), (xu, yl), (xu, yu)]:
        if in_cone(p, piece):
            out.append(p)
    if xl <= 0 <= xu and yl <= 0 <= yu:
        out.append((ZERO, ZERO))
    for dx, dy in (piece[1], piece[3]):
        for e in (xl, xu):
            if dx != 0:
                r = e / dx
                if r >= 0 and yl <= r * dy <= yu:
                    out.append((e, r * dy))
        for e in (yl, yu):
            if dy != 0:
                r = e / dy
                if r >= 0 and xl <= r * dx <= xu:
                    out.append((r * dx, e))
    return out


def tightest(c, y, x):
    """The tightest enclosures of the hulls of the y and of the x of S"""
    if EMPTY in (c, y, x):
        return EMPTY, EMPTY
    box = (mpf_bound(x[0]), mpf_bound(x[1]), mpf_bound(y[0]), mpf_bound(y[1]))
    ys, xs = [], []
    for piece in pieces(c):
        points = vertices(piece, box)
        # What is left out: (0, 0), and the half-line of -pi
        kept = [p for p in points if not (p[1] == 0 and (p[0] == 0 or (piece[4] and p[0] < 0)))]
        if not kept:
            continue
        xs += [p[0] for p in points]
        ys += [p[1] for p in points]
    if not xs:
        return EMPTY, EMPTY
    return (round_down(min(ys)), round_up(max(ys))), (round_down(min(xs)), round_up(max(xs)))


def accurate(c, y, x):
    """nextOut(tightest(nextOut(c), nextOut(y), nextOut(x))) of both hulls"""
    ty, tx = tightest(next_out(c), next_out(y), next_out(x))
    return next_out(ty), next_out(tx)


#---------------------------------------------------------------------------
# The cases
#---------------------------------------------------------------------------
def around(k):
    """The doubles below and above k pi/4"""
    v = k * PI / 4
    d = float(v)
    below = next_down(d) if mpmath.mpf(d) > v else d
    return below, next_up(below)


def structured_angles():
    out = [W, (0.0, 0.0), (-0.0, 0.0), EMPTY]
    for k in range(-4, 5):
        if k == 0:
            continue
        below, above = around(k)
        out += [(below, below), (above, above), (below, above)]
    pi_dn, pi_up = around(4)
    hpi_dn, hpi_up = around(2)
    qpi_dn, qpi_up = around(1)
    out += [
        (0.0, qpi_up), (qpi_dn, hpi_up), (hpi_dn, pi_up), (-pi_up, -hpi_dn), (-hpi_up, 0.0),
        (-qpi_up, qpi_up), (hpi_up, pi_dn), (-pi_dn, -hpi_up), (0.0, hpi_dn), (-hpi_dn, 0.0),
        (3.0, 4.0), (-4.0, -3.0), (-4.0, 4.0), (4.0, 5.0), (-5.0, -4.0), (pi_up, 10.0), (-10.0, -pi_up),
        (-INF, -pi_up), (pi_up, INF), (-INF, 0.0), (0.0, INF), (1.0, 2.0), (-2.0, -1.0), (0.5, 0.6),
        (-3.0, 3.0), (2.5, 3.5), (-3.5, -2.5), (-pi_up, pi_dn), (-pi_dn, pi_dn), (-INF, -3.0), (3.0, INF),
        (TINY, 2 * TINY), (-2 * TINY, -TINY), (math.ldexp(1.0, -30), math.ldexp(1.0, -29)),
        (next_down(hpi_dn), hpi_dn), (hpi_up, next_up(hpi_up)), (next_down(pi_dn), pi_dn),
        (-pi_dn, next_up(-pi_dn)), (-TINY, TINY),
    ]
    return out


def structured_boxes():
    """(y, x) pairs"""
    b = [
        ((1.0, 2.0), (1.0, 2.0)), ((1.0, 2.0), (-2.0, -1.0)), ((-2.0, -1.0), (-2.0, -1.0)),
        ((-2.0, -1.0), (1.0, 2.0)), ((-1.0, 1.0), (1.0, 2.0)), ((-1.0, 1.0), (-2.0, -1.0)),
        ((1.0, 2.0), (-1.0, 1.0)), ((-2.0, -1.0), (-1.0, 1.0)), ((-1.0, 1.0), (-1.0, 1.0)),
        ((-1.0, 2.0), (-3.0, 1.0)), ((0.0, 0.0), (0.0, 0.0)), ((0.0, 0.0), (-1.0, 1.0)),
        ((0.0, 0.0), (-2.0, -1.0)), ((0.0, 0.0), (1.0, 2.0)), ((-1.0, 1.0), (0.0, 0.0)),
        ((0.0, 1.0), (0.0, 0.0)), ((-1.0, 0.0), (0.0, 0.0)), ((0.0, 1.0), (0.0, 1.0)),
        ((-1.0, 0.0), (-1.0, 0.0)), ((0.0, 1.0), (-1.0, 0.0)), ((-1.0, 0.0), (0.0, 1.0)),
        ((-1.0, 0.0), (-2.0, -1.0)), ((0.0, 1.0), (-2.0, -1.0)), ((-1.0, -0.0), (-2.0, -0.0)),
        (W, W), ((0.0, INF), (0.0, INF)), ((1.0, INF), (1.0, INF)), ((-INF, -1.0), (-INF, 0.0)),
        (W, (1.0, 2.0)), ((1.0, 2.0), W), ((0.0, INF), (-INF, 0.0)), (W, (-INF, -1.0)),
        ((-INF, 0.0), W), ((-INF, 0.0), (-INF, 0.0)), ((0.0, 0.0), (-INF, 0.0)), ((0.0, 0.0), W),
        ((TINY, 2 * TINY), (1e300, 1e308)), ((1e300, MAX), (TINY, math.ldexp(1.0, -1060))),
        ((-math.ldexp(1.0, -1060), math.ldexp(1.0, -1060)), (-1e300, -1e299)), ((MAX, INF), (1.0, 2.0)),
        ((3.0, 3.0), (4.0, 4.0)), ((1.0, 1.0), (1.0, 1.0)), ((-1.0, -1.0), (-1.0, -1.0)),
        ((1.0, 1.0), (-1.0, -1.0)), ((-TINY, TINY), (-TINY, TINY)),
        # Bounds that are powers of two, whose products and quotients with a
        # subnormal angle are exact
        (W, (1.0, math.ldexp(1.0, 20))), ((math.ldexp(1.0, -1000), 1.0), (0.0, INF)),
        ((-1.0, -math.ldexp(1.0, -1000)), (0.0, INF)), ((-1.0, 1.0), EMPTY),
        (EMPTY, (1.0, 2.0)),
    ]
    return b


rng = random.Random(1788)


def random_number(lo_exp, hi_exp, special):
    r = rng.random()
    if r < special / 3:
        return rng.choice([0.0, -0.0, 1.0, -1.0])
    if r < special:
        return rng.choice([INF, -INF])
    return rng.choice([-1.0, 1.0]) * math.ldexp(rng.uniform(1.0, 2.0), rng.randint(lo_exp, hi_exp))


def random_interval(lo_exp, hi_exp, special=0.2):
    a, b = sorted([random_number(lo_exp, hi_exp, special), random_number(lo_exp, hi_exp, special)])
    return (-1.0, 1.0) if a == INF or b == -INF else (a, b)


def random_angles():
    r = rng.random()
    if r < 0.6:
        a, b = sorted([rng.uniform(-4.0, 4.0), rng.uniform(-4.0, 4.0)])
        return (a, b)
    if r < 0.8:  # next to a multiple of pi/4, or a point
        below, above = around(rng.randint(-4, 4))
        a = rng.choice([below, above])
        return (a, a) if rng.random() < 0.5 else tuple(sorted([a, rng.uniform(-4.0, 4.0)]))
    return random_interval(-40, 3, 0.4)


def cases():
    out = []
    for c in structured_angles():
        for y, x in structured_boxes():
            out.append((c, y, x, "structured"))
    for _ in range(400):
        if rng.random() < 0.5:
            y, x = random_interval(-20, 20), random_interval(-20, 20)
        else:  # bounds of any magnitude
            y, x = random_interval(-1074, 1023, 0.3), random_interval(-1074, 1023, 0.3)
        out.append((random_angles(), y, x, "random"))
    return out


#---------------------------------------------------------------------------
# The header
#---------------------------------------------------------------------------
def double(x):
    """x as a hexadecimal literal, without the trailing zeros of its digits"""
    if x == INF:
        return "inf"
    if x == -INF:
        return "-inf"
    if x == 0.0:
        return "-0x0p+0" if math.copysign(1.0, x) < 0 else "0x0p+0"
    digits, exponent = x.hex().split("p")
    return digits.rstrip("0").rstrip(".") + "p" + exponent


def interval(a):
    if a == EMPTY:
        return "E"
    if a == W:
        return "W"
    return "i(%s, %s)" % (double(a[0]), double(a[1]))


def main():
    print("// Generated by tests/atan2_rel_values.py: atan2Rev1 and atan2Rev2 of IEEE 1788-2015")
    print("// on doubles, with the tightest enclosures of the hulls the standard defines and")
    print("// the bounds of the accurate results. Do not edit.")
    print("//")
    print("// Copyright (c) 2026 ENSTA, France")
    print("//")
    print("// Created 2026-10-06 by Jordan NININ")
    print("#ifndef GAOL_TESTS_ATAN2_REL_VALUES_H")
    print("#define GAOL_TESTS_ATAN2_REL_VALUES_H")
    print("namespace atan2_rel_values")
    print("{")
    print("  // Constant expressions, which the compiler writes as data rather than")
    print("  // initializing the table at run time, at length: with this table and")
    print("  // reverse_values.h initialized at run time, tests/reverse.cpp took GCC 12")
    print("  // for 32-bit ARM beyond its 3 GB (Debian 12 armhf)")
    print("  constexpr double inf = std::numeric_limits<double>::infinity();")
    print("  // An interval of the table: i(l, u), E the empty set, W the whole line")
    print("  struct I { double l, u; bool empty; };")
    print("  constexpr I E = { 0.0, 0.0, true }, W = { -inf, inf, false };")
    print("  constexpr I i(double l, double u) { return I{ l, u, false }; }")
    print("  // c, the angles, and the box y x x; the tightest enclosures of the hulls of the")
    print("  // y and of the x of its points (0, 0) left out whose angle is in c,")
    print("  // atan2Rev1(x, c, y) and atan2Rev2(y, c, x), and the bounds of their accurate")
    print("  // results, nextOut(tightest(nextOut(c), nextOut(y), nextOut(x))) (12.10.1);")
    print("  // from, structured or random")
    print("  struct Case { I c, y, x, y_tightest, y_accurate, x_tightest, x_accurate; const char* from; };")
    print("  constexpr Case cases[] = {")
    n, nonempty, empty = 0, 0, 0
    for c, y, x, source in cases():
        ty, tx = tightest(c, y, x)
        if ty == EMPTY and source == "structured" and EMPTY not in (c, y, x):
            # One structured case of five without points, the box and the
            # angles not meeting, is kept: they are the most of them
            empty += 1
            if empty % 5 != 1:
                continue
        ay, ax = accurate(c, y, x)
        n += 1
        nonempty += ty != EMPTY
        print("    { %s, %s, %s, %s, %s, %s, %s, \"%s\" }," % (
            interval(c), interval(y), interval(x), interval(ty), interval(ay), interval(tx), interval(ax), source))
    print("  };")
    print("}")
    print("#endif // GAOL_TESTS_ATAN2_REL_VALUES_H")
    print("atan2_rel_values.py: %d cases, %d of them with points" % (n, nonempty), file=sys.stderr)


if __name__ == "__main__":
    main()
