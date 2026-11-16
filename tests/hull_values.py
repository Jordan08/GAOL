#!/usr/bin/env python3
# Tests of GAOL v5: writes the statements TEST_HULL() of tests/trigonometric.cpp
# and tests/reverse_mappings.cpp, the unit tests of GAOL 4 whose references were
# decimal numbers compared at a Hausdorff distance of 1e-8 (TEST_EQ), which an
# enclosure that missed the exact value by less than that passed.
#
# For each call, this script computes the hull of the values the function takes
# on the interval(s) it is called with (for the relational functions, the hull
# of the x of the last interval that the relation of IEEE 1788-2015 (10.5.4,
# Table 10.1) selects), exactly, with 2000 bits by mpmath (https://mpmath.org)
# and rational arithmetic, and writes TEST_HULL(call, lo, hi), lo being the
# greatest double at most the lower bound of the hull, and hi the least double at
# least the upper one: GAOL's bounds have to enclose them, and be one double from
# them at most (tests/unit_tests.h). A bound of the hull that is a double itself,
# a finite one, is GAOL's too: the statement is TEST_HULL_EXACT_LO or
# TEST_HULL_EXACT_HI when one of the two is, and TEST_SEQ when both are. The
# relational functions come from reverse_values.py, which writes the cases of
# tests/reverse.cpp.
#
#     python3 tests/hull_values.py            # the statements, by file
#     python3 tests/hull_values.py --check    # the statements are in the files
#
# The statements are in the two files as written here. A difference is an error:
# --check exits with 1 and names the statements that are missing.
#
# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-15)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

import math
import os
import re
import sys
import mpmath

sys.dont_write_bytecode = True  # no tests/__pycache__ for reverse_values
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import reverse_values as rv  # noqa: E402  (sets mpmath's precision to 2000 bits)

INF = math.inf
MAX = sys.float_info.max


def m(x):
    """x, a double or +-inf, as an mpf"""
    if x == INF:
        return mpmath.inf
    if x == -INF:
        return -mpmath.inf
    return mpmath.mpf(x)


def pair(v):
    """The greatest double at most the real v and the least double at least it,
    which are v when it is a double (v is a double, +-inf, a Fraction or an mpf)"""
    return rv.round_down(v), rv.round_up(v)


def bounds(low, high):
    """The enclosure of a hull whose lower and upper bounds have the pairs low and
    high (pair()): (lo, hi, lo_exact, hi_exact), lo being the greatest double at most
    the lower bound and hi the least double at least the upper one, and lo_exact
    telling that the lower bound is a finite double, so that GAOL's has to be it.
    The infinite bounds are not told: TEST_HULL asks for them either way"""
    lo, hi = low[0], high[1]
    return (lo, hi, low[0] == low[1] and abs(lo) != INF, high[0] == high[1] and abs(hi) != INF)


#---------------------------------------------------------------------------
# The text of a call: the arguments are written from the numbers they hold, so
# that the statement and the value cannot differ
#---------------------------------------------------------------------------
def number(x):
    if x == INF:
        return "GAOL_INFINITY"
    if x == -INF:
        return "-GAOL_INFINITY"
    if x == int(x) and abs(x) < 2**31:
        return str(int(x))  # an int, as the tests of GAOL 4 write the small ones
    return repr(x)


class Arg:
    """An interval argument: its C++ text, and its bounds"""
    def __init__(self, text, lo, hi):
        self.text, self.lo, self.hi = text, lo, hi


def iv(lo, hi=None):
    lo = float(lo)
    hi = lo if hi is None else float(hi)
    if lo == hi:
        return Arg("interval(%s)" % number(lo), lo, hi)
    return Arg("interval(%s,%s)" % (number(lo), number(hi)), lo, hi)


UNIVERSE = Arg("interval::universe()", -INF, INF)
PI = Arg("interval::pi()", math.pi, rv.next_up(math.pi))
MAX_INF = Arg("max_inf", MAX, INF)  # tests/unit_tests.h
M_INF_M_MAX = Arg("m_inf_m_max", -INF, -MAX)


#---------------------------------------------------------------------------
# The hull of the values of the functions of trigonometric.cpp
#---------------------------------------------------------------------------
def double_value(f, x):
    """The greatest double at most f(x) and the least double at least f(x), for
    an increasing f. A value that 2000 bits cannot tell from 1 or -1, as tanh(MAX)
    which is 1 - 2^-2000 or so, is not them: it is inside (-1, 1) at a finite x"""
    v = f(m(x))
    below, above = pair(v)
    if abs(x) != INF and abs(v) == 1:
        below, above = (rv.next_down(1.0), 1.0) if v > 0 else (-1.0, rv.next_up(-1.0))
    return below, above


def trig_hull(name, a, b):
    """The least and the greatest value of cos or tan over [a, b], as mpfs: the
    values at a and b, and -1 or 1 where cos has an extremum within [a, b], that
    is a multiple of pi; for tan, -inf and +inf where there is a pole within
    [a, b], that is pi/2 plus a multiple of pi"""
    shift = mpmath.mpf(0) if name == "cos" else mpmath.mpf(1) / 2
    k_first = mpmath.ceil(m(a) / mpmath.pi - shift)
    k_last = mpmath.floor(m(b) / mpmath.pi - shift)
    if name == "tan":
        if k_first <= k_last:
            return -mpmath.inf, mpmath.inf
        return mpmath.tan(m(a)), mpmath.tan(m(b))
    values = [mpmath.cos(m(a)), mpmath.cos(m(b))]
    if k_first <= k_last:
        values.append(mpmath.mpf(1) if int(k_first) % 2 == 0 else mpmath.mpf(-1))
        if k_first < k_last:
            values.append(mpmath.mpf(-1) if int(k_first) % 2 == 0 else mpmath.mpf(1))
    return min(values), max(values)


def hull(name, a, b):
    """The tightest enclosure of the hull of the function name over [a, b], with
    the bounds that are doubles (bounds())"""
    if name in ("cos", "tan"):
        lo, hi = trig_hull(name, a, b)
        return bounds(pair(lo), pair(hi))
    if name == "cosh":  # even: the least value at 0 if it is within [a, b]
        least = mpmath.mpf(1) if a <= 0 <= b else min(mpmath.cosh(m(a)), mpmath.cosh(m(b)))
        return bounds(pair(least), pair(max(mpmath.cosh(m(a)), mpmath.cosh(m(b)))))
    # The increasing functions, and acos, which is decreasing: over their domain
    domains = {"asin": (-1.0, 1.0), "acos": (-1.0, 1.0), "acosh": (1.0, INF), "atanh": (-1.0, 1.0)}
    low, high = domains.get(name, (-INF, INF))
    a, b = max(a, low), min(b, high)
    if a > b:
        raise ValueError("%s over an interval outside its domain" % name)
    if name == "acos":
        return bounds(pair(mpmath.acos(m(b))), pair(mpmath.acos(m(a))))
    if name == "atanh":  # +-oo at +-1, which atanh does not reach
        f = lambda v: -mpmath.inf if v == -1 else (mpmath.inf if v == 1 else mpmath.atanh(v))
        return bounds(pair(f(m(a))), pair(f(m(b))))
    functions = {"asin": mpmath.asin, "atan": mpmath.atan, "sinh": mpmath.sinh, "tanh": mpmath.tanh,
                 "asinh": mpmath.asinh, "acosh": mpmath.acosh}
    f = functions[name]
    return bounds(double_value(f, a), double_value(f, b))


# The calls of trigonometric.cpp: function, argument
TRIGONOMETRIC = [
    ("cos", iv(-1, -0.5)), ("cos", PI), ("cos", iv(3, 3.5)), ("cos", iv(-3.5, -3)), ("cos", iv(-3.5, 3)),
    ("cos", iv(10, 12)), ("cos", iv(13, 14)), ("cos", iv(10, 14)), ("cos", iv(14, 16)),
    ("cos", iv(-11, -10)), ("cos", iv(-14, -13)), ("cos", iv(-16, -14)), ("cos", iv(-102, -100)),
    ("cos", iv(4.6e15, 4.7e15)), ("cos", iv(4503599627370495, 4503599627370496)),
    ("tan", iv(0, 1)), ("tan", iv(-1, 0)), ("tan", iv(-2, -1)), ("tan", iv(202, 203)),
    ("acos", iv(-1.5, -.5)), ("acos", iv(-0.5, 0.5)), ("acos", iv(0.1)),
    ("asin", iv(-1.5, -.5)), ("asin", iv(-0.5, 0.5)), ("asin", iv(0.1)),
    ("atan", MAX_INF), ("atan", M_INF_M_MAX), ("atan", iv(-10, 3)),
    ("cosh", iv(-5, -3)), ("cosh", iv(-3, 5)), ("cosh", iv(-5, 3)), ("cosh", iv(3, 5)),
    ("sinh", iv(-3, 5)), ("sinh", UNIVERSE),
    ("tanh", M_INF_M_MAX), ("tanh", UNIVERSE), ("tanh", iv(-3, 5)), ("tanh", iv(-1, .5)),
    ("acosh", iv(0, 1)), ("acosh", iv(2, 4)), ("acosh", iv(100.1)),
    ("asinh", iv(-3, 2)), ("atanh", iv(-2, -.5)), ("atanh", iv(0, 3)), ("atanh", iv(-0.5, 0.25)),
]


#---------------------------------------------------------------------------
# The relational functions of reverse_mappings.cpp: the name of GAOL, the
# function of reverse_values.py, and the order of the arguments of GAOL
#---------------------------------------------------------------------------
def tanh_rev(c, x):
    """x within x with tanh(x) in c, tanh taking its values in (-1, 1): the
    bounds at +-1 are the limits +-oo"""
    l, u = max(c[0], -1.0), min(c[1], 1.0)
    if l > u or (l == u and abs(l) == 1):
        return rv.EMPTY
    f = lambda v: -mpmath.inf if v == -1 else (mpmath.inf if v == 1 else mpmath.atanh(m(v)))
    return rv.hull_within([(f(l), f(u))], x)


def with_exact_bounds(compute):
    """The enclosure that compute() finds through rv.hull_within(), with the
    bounds that are doubles (bounds()), which rv.last_hull has before they are
    rounded"""
    rv.last_hull = None
    result = compute()
    if result == rv.EMPTY:
        return result
    low, high = rv.last_hull
    assert result == (rv.round_down(low), rv.round_up(high)), (result, rv.last_hull)
    return bounds(pair(low), pair(high))


def relational(name, args):
    """The tightest enclosure of the hull of the relational function name"""
    ranges = [(a.lo, a.hi) for a in args]
    if name == "acos_rel":
        return with_exact_bounds(lambda: rv.tightest("cosRev", ranges[0], None, ranges[1], 0))
    if name == "atan_rel":
        return with_exact_bounds(lambda: rv.tightest("tanRev", ranges[0], None, ranges[1], 0))
    if name == "div_rel":  # div_rel(c, b, x): the x with x*b in c for some b
        return with_exact_bounds(lambda: rv.tightest("mulRev", ranges[0], ranges[1], ranges[2], 0))
    if name == "atanh_rel":
        return with_exact_bounds(lambda: tanh_rev(ranges[0], ranges[1]))
    raise ValueError(name)


def pown(c, n, x):
    """nth_root_rel(c, n, x)"""
    return with_exact_bounds(lambda: rv.tightest("pownRev", (c.lo, c.hi), None, (x.lo, x.hi), n))


# The calls of reverse_mappings.cpp whose result is a literal: the function and
# its arguments, in the order of GAOL
RELATIONAL = [
    ("acos_rel", [iv(.25, .5), iv(1.02, 1.32)]),
    ("acos_rel", [iv(.5, .75), iv(-6.3, 6.3)]),
    ("acos_rel", [iv(.5, .75), iv(-2, 6.3)]),
    ("acos_rel", [iv(.5, .75), iv(-6.3, 2)]),
    ("acos_rel", [iv(.9, .95), iv(-0.32, 0.2)]),
    ("acos_rel", [UNIVERSE, iv(-3, 2)]),
    ("acos_rel", [UNIVERSE, iv(0.5, 0.6)]),
    ("atan_rel", [iv(-5, 5), UNIVERSE]),
    ("atan_rel", [UNIVERSE, iv(4.0, 5.5)]),
    ("atan_rel", [UNIVERSE, iv(4.0, 6.5)]),
    ("atan_rel", [iv(.5, .6), iv(3.7, 7)]),
    ("atan_rel", [iv(.5, .6), iv(3.5, 7)]),
    ("atan_rel", [iv(.5, .6), iv(3.62, 7)]),
    ("atan_rel", [iv(.5, .6), iv(-7, -3.62)]),
    ("nth_root_rel", [iv(-3, 2), 2, UNIVERSE]),
    ("nth_root_rel", [iv(2, 3), 2, UNIVERSE]),
    ("nth_root_rel", [iv(2, 3), 2, iv(-2, -1.5)]),
    ("nth_root_rel", [iv(2, 3), 2, iv(1.5, 2)]),
    ("nth_root_rel", [iv(2, 3), 5, UNIVERSE]),
    ("nth_root_rel", [iv(0.5, 3), 5, UNIVERSE]),
    ("nth_root_rel", [iv(-.2, 3), 5, UNIVERSE]),
    ("nth_root_rel", [iv(-3, -2), 5, UNIVERSE]),
    ("nth_root_rel", [iv(0.2, 0.5), 5, UNIVERSE]),
    ("nth_root_rel", [iv(-3, -.2), 5, UNIVERSE]),
    ("div_rel", [iv(-6, 0), iv(3.0), iv(-2, 0)]),
    ("div_rel", [iv(-4.7, -1.5), iv(3.0), iv(-2, 0)]),
    ("div_rel", [iv(-1.5, 0), iv(3.0), iv(-2, 0)]),
    ("div_rel", [iv(-3, 0), iv(3.0), iv(-2, 0)]),
    ("atanh_rel", [iv(0.5, 1.0), iv(0.0, 100.0)]),
    ("atanh_rel", [iv(-1.0, 1.0), iv(1.0, 2.0)]),
    ("atanh_rel", [iv(0.5, 1.0), iv(0.0, INF)]),
    ("atanh_rel", [iv(-1.0, -0.5), iv(-INF, 0.0)]),
]


#---------------------------------------------------------------------------
# The statements
#---------------------------------------------------------------------------
def literal(x):
    if x == INF:
        return "GAOL_INFINITY"
    if x == -INF:
        return "-GAOL_INFINITY"
    return (0.0 if x == 0 else x).hex()


def exact_literal(x):
    """A bound that GAOL's has to be: a whole number as one, as the tests of GAOL
    4 write them, and the others in hexadecimal"""
    return str(int(x)) if x == int(x) and abs(x) < 2**31 else literal(x)


def statement(call, enclosure):
    if enclosure == rv.EMPTY:
        return "TEST_EMPTY(%s);" % call
    lo, hi, lo_exact, hi_exact = enclosure
    if lo_exact and hi_exact:
        reference = exact_literal(lo) if lo == hi else "%s, %s" % (exact_literal(lo), exact_literal(hi))
        return "TEST_SEQ(%s, interval(%s));" % (call, reference)
    macro = "TEST_HULL_EXACT_LO" if lo_exact else "TEST_HULL_EXACT_HI" if hi_exact else "TEST_HULL"
    return "%s(%s, %s, %s);" % (macro, call, literal(lo), literal(hi))


def trigonometric_statements():
    return [statement("%s(%s)" % (name, arg.text), hull(name, arg.lo, arg.hi)) for name, arg in TRIGONOMETRIC]


def reverse_mappings_statements():
    out = []
    for name, args in RELATIONAL:
        if name == "nth_root_rel":
            c, n, x = args
            call = "nth_root_rel(%s,%d,%s)" % (c.text, n, x.text)
            out.append(statement(call, pown(c, n, x)))
        else:
            call = "%s(%s)" % (name, ",".join(a.text for a in args))
            out.append(statement(call, relational(name, args)))
    return out


FILES = {"trigonometric.cpp": trigonometric_statements, "reverse_mappings.cpp": reverse_mappings_statements}


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--check":
        directory = os.path.dirname(os.path.abspath(__file__))
        missing = 0
        for name, statements in FILES.items():
            with open(os.path.join(directory, name)) as f:
                text = re.sub(r"\s+", "", f.read())
            for s in statements():
                if re.sub(r"\s+", "", s) not in text:
                    print("%s: missing %s" % (name, s))
                    missing += 1
        if missing:
            sys.exit(1)
        print("every statement is in tests/trigonometric.cpp and tests/reverse_mappings.cpp")
        return
    for name, statements in FILES.items():
        print("// " + name)
        for s in statements():
            print(s)


if __name__ == "__main__":
    main()
