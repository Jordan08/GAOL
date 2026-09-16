#!/usr/bin/env python3
"""Special cases of interval arithmetic in GAOL, libieeep1788 and Solaris Studio.

The cases come from the tests of GAOL (tests/*.cpp, and check/*.cpp for the
integer powers). Each one is written once, below, as an expression, from which
this script generates a program for each library; the results the programs
print are then compared with the results IEEE 1788-2015 defines, computed here
with mpmath and rounded outward.

    cases.py generate DIR
        writes DIR/cases_gaol.cpp, DIR/cases_p1788.cpp and DIR/cases_sun.f90
    cases.py report GAOL.txt P1788.txt SUN.txt REPORT.md
        reads what the three programs printed, and writes the table of the
        results between the markers of REPORT.md (the whole file when it does
        not exist)

Each program prints one line per case:
    id|I|lo|hi   an interval, the bounds in C hexadecimal (%a) or as the bits
                 of the double in hexadecimal, prefixed by b:
    id|E         the empty set
    id|R|v       a number
    id|X|text    an exception, or a string the library could not read
    id|NA        the library has no such operation
"""

import math
import struct
import sys

import mpmath

MAX = sys.float_info.max
TINY = 5e-324
INF = math.inf
NAN = math.nan

# ---------------------------------------------------------------------------
# Expressions


class Node:
    def __init__(self, op, *args):
        self.op = op
        self.args = args


def iv(lo, hi=None):
    """interval(lo, hi), or interval(lo) when hi is None"""
    return Node("iv", float(lo), None if hi is None else float(hi))


EMPTY = Node("empty")
ENTIRE = Node("entire")


def text(s):
    return Node("text", s)


def op(name, *args):
    return Node(name, *args)


class NotAvailable(Exception):
    pass


def is_neg_zero(x):
    return x == 0.0 and math.copysign(1.0, x) < 0


def cpp_double(x):
    if math.isnan(x):
        return "qnan"
    if x == INF:
        return "pinf"
    if x == -INF:
        return "(-pinf)"
    if x == MAX:
        return "dmax"
    if x == -MAX:
        return "(-dmax)"
    if x == TINY:
        return "dtiny"
    if is_neg_zero(x):
        return "(-0.0)"
    s = repr(x)
    return "(" + s + ")" if x < 0 else s


def f90_double(x):
    if math.isnan(x):
        return "qnan"
    if x == INF:
        return "pinf"
    if x == -INF:
        return "ninf"
    if x == MAX:
        return "dmax"
    if x == -MAX:
        return "(-dmax)"
    if x == TINY:
        return "dtiny"
    if is_neg_zero(x):
        return "mzero"
    s = repr(x)
    s = s.replace("e", "d") if "e" in s else s + "d0"
    return "(" + s + ")" if x < 0 else s


POWERS = {2**31: "2^31", 2**31 - 1: "2^31−1", 2**31 + 1: "2^31+1", 2**52 - 1: "2^52−1"}


def readable_double(x):
    """x for the tables: MAX, 2^-1074..., and the shortest decimal otherwise"""
    if math.isnan(x):
        return "NaN"
    if x == -INF:
        return "−∞"
    if x < 0 or is_neg_zero(x):
        s = readable_double(-x)
        return "−" + ("(" + s + ")" if "+" in s or "−" in s else s)
    names = {INF: "+∞", MAX: "MAX", MAX / 2: "MAX/2", 1e10: "1e10", 0.0: "0"}
    if x in names:
        return names[x]
    if x == next_down(MAX):
        return "pred(MAX)"
    if x < 2.2250738585072014e-308 and x / TINY <= 64:
        k = int(x / TINY)
        return "2^-1074" if k == 1 else f"{k}·2^-1074"
    if x == int(x) and x < 1e16:
        return POWERS.get(int(x), str(int(x)))
    return repr(x).replace("-", "−")


# The binary operators on intervals, in the notation of each library
INFIX = {"add": "+", "sub": "-", "mul": "*", "div": "/"}
FUNCTIONS = ["sin", "cos", "tan", "asin", "acos", "atan", "exp", "log", "sqrt", "sinh", "cosh", "tanh",
             "asinh", "acosh", "atanh", "abs", "sqr"]


def cpp(n, lib):
    """The C++ expression of n; lib is 'gaol' or 'p1788'"""
    g = lib == "gaol"
    a = n.args
    if n.op == "iv":
        lo, hi = a
        if hi is None:
            return f"interval({cpp_double(lo)})" if g else f"II({cpp_double(lo)}, {cpp_double(lo)})"
        return f"{'interval' if g else 'II'}({cpp_double(lo)}, {cpp_double(hi)})"
    if n.op == "empty":
        return "interval::emptyset()" if g else "II::empty()"
    if n.op == "entire":
        return "interval::universe()" if g else "II::entire()"
    if n.op == "text":
        s = a[0].replace("\\", "\\\\").replace('"', '\\"')
        return f'interval("{s}")' if g else f'II(std::string("{s}"))'
    if n.op in INFIX:
        return f"({cpp(a[0], lib)} {INFIX[n.op]} {cpp(a[1], lib)})"
    if n.op == "neg":
        return f"(-{cpp(a[0], lib)})"
    if n.op in ("addd", "subd", "muld", "divd"):  # interval op double
        o = INFIX[n.op[:-1]]
        return f"({cpp(a[0], lib)} {o} {cpp_double(a[1])})" if g else \
               f"({cpp(a[0], lib)} {o} II({cpp_double(a[1])}, {cpp_double(a[1])}))"
    if n.op == "dmul":  # double * interval
        return f"({cpp_double(a[0])} * {cpp(a[1], lib)})" if g else \
               f"(II({cpp_double(a[0])}, {cpp_double(a[0])}) * {cpp(a[1], lib)})"
    if n.op == "rdiv":  # GAOL's relational division x % y: mul_rev of IEEE 1788
        return f"({cpp(a[0], lib)} % {cpp(a[1], lib)})" if g else f"mul_rev({cpp(a[1], lib)}, {cpp(a[0], lib)})"
    if n.op in FUNCTIONS:
        return f"{n.op}({cpp(a[0], lib)})"
    if n.op == "inverse":
        return f"inverse({cpp(a[0], lib)})" if g else f"recip({cpp(a[0], lib)})"
    if n.op == "powi":
        return f"pow({cpp(a[0], lib)}, {a[1]})" if g else f"pown({cpp(a[0], lib)}, {a[1]})"
    if n.op == "powd":
        return f"pow({cpp(a[0], lib)}, {cpp_double(a[1])})" if g else \
               f"pow({cpp(a[0], lib)}, II({cpp_double(a[1])}, {cpp_double(a[1])}))"
    if n.op == "pow":
        return f"pow({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op == "nth_root":
        if not g:
            raise NotAvailable
        return f"nth_root({cpp(a[0], lib)}, {a[1]}u)"
    if n.op == "hull":
        return f"({cpp(a[0], lib)} | {cpp(a[1], lib)})" if g else f"convex_hull({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op == "inter":
        return f"({cpp(a[0], lib)} & {cpp(a[1], lib)})" if g else f"intersection({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op in ("min", "max"):
        return f"{n.op}({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op in ("mid", "wid", "mag", "mig"):
        method = {"mid": "midpoint", "wid": "width", "mag": "mag", "mig": "mig"}[n.op]
        return f"{cpp(a[0], lib)}.{method}()" if g else f"{n.op}({cpp(a[0], lib)})"
    raise ValueError(n.op)


def cpp_body(n, lib):
    """The body of the lambda computing the case n"""
    g = lib == "gaol"
    a = n.args
    if n.op in ("iadd", "isub", "imul", "idiv", "irdiv"):  # x op= d
        x, d = a
        if g:
            o = {"iadd": "+=", "isub": "-=", "imul": "*=", "idiv": "/=", "irdiv": "%="}[n.op]
            return f"interval r = {cpp(x, lib)}; r {o} {cpp_double(d)}; return r;"
        if n.op == "irdiv":
            return f"return mul_rev(II({cpp_double(d)}, {cpp_double(d)}), {cpp(x, lib)});"
        o = INFIX[n.op[1:]]
        return f"return {cpp(x, lib)} {o} II({cpp_double(d)}, {cpp_double(d)});"
    if n.op == "iaddI":  # x += y
        x, y = a
        if g:
            return f"interval r = {cpp(x, lib)}; r += {cpp(y, lib)}; return r;"
        return f"return {cpp(x, lib)} + {cpp(y, lib)};"
    if n.op == "assign":  # x = d
        x, d = a
        if g:
            return f"interval r = {cpp(x, lib)}; r = {cpp_double(d)}; return r;"
        return f"return II({cpp_double(d)}, {cpp_double(d)});"
    return f"return {cpp(n, lib)};"


def f90(n):
    """The Fortran expression of n, for Solaris Studio's -xia"""
    a = n.args
    if n.op == "iv":
        lo, hi = a
        if hi is None:
            return f"interval({f90_double(lo)})"
        return f"interval({f90_double(lo)}, {f90_double(hi)})"
    if n.op == "empty":
        return "emptyi"
    if n.op == "entire":
        return "interval(ninf, pinf)"
    if n.op in INFIX:
        return f"({f90(a[0])} {INFIX[n.op]} {f90(a[1])})"
    if n.op == "neg":
        return f"(-{f90(a[0])})"
    if n.op in ("addd", "subd", "muld", "divd"):
        return f"({f90(a[0])} {INFIX[n.op[:-1]]} {f90_double(a[1])})"
    if n.op == "dmul":
        return f"({f90_double(a[0])} * {f90(a[1])})"
    if n.op in ("rdiv", "nth_root", "asinh", "acosh", "atanh"):
        raise NotAvailable
    if n.op == "sqr":
        return f"({f90(a[0])}**2)"
    if n.op in FUNCTIONS:
        return f"{n.op}({f90(a[0])})"
    if n.op == "inverse":
        return f"(onei / {f90(a[0])})"
    if n.op == "powi":
        return f"({f90(a[0])}**({a[1]}))"
    if n.op == "powd":
        return f"({f90(a[0])}**{f90_double(a[1])})"
    if n.op == "pow":
        return f"({f90(a[0])}**{f90(a[1])})"
    if n.op == "hull":
        return f"({f90(a[0])} .ih. {f90(a[1])})"
    if n.op == "inter":
        return f"({f90(a[0])} .ix. {f90(a[1])})"
    if n.op in ("min", "max", "mid", "wid", "mag", "mig"):
        return f"{n.op}({', '.join(f90(x) for x in a)})"
    raise ValueError(n.op)


def f90_statements(n, cid, kind):
    """The Fortran statements computing the case n and printing its result"""
    a = n.args
    if n.op == "text":
        s = a[0]
        return [f"s = '{s}'", "r = emptyi", "read(s, *, iostat=ios) r", "if (ios /= 0) then",
                f"  call showx('{cid}', ios)", "else", f"  call showi('{cid}', r)", "end if"]
    if n.op in ("iadd", "isub", "imul", "idiv"):
        x, d = a
        return [f"r = {f90(x)}", f"r = r {INFIX[n.op[1:]]} {f90_double(d)}", f"call showi('{cid}', r)"]
    if n.op == "irdiv":
        raise NotAvailable
    if n.op == "iaddI":
        x, y = a
        return [f"r = {f90(x)}", f"r = r + {f90(y)}", f"call showi('{cid}', r)"]
    if n.op == "assign":
        x, d = a
        return [f"r = {f90(x)}", f"r = {f90_double(d)}", f"call showi('{cid}', r)"]
    if kind == "R":
        return [f"v = {f90(n)}", f"call showr('{cid}', v)"]
    return [f"r = {f90(n)}", f"call showi('{cid}', r)"]


def label(n):
    """GAOL's notation of n"""
    a = n.args
    if n.op == "iv":
        lo, hi = a
        if hi is None:
            return f"[{readable_double(lo)}]" if math.isfinite(lo) else f"interval({readable_double(lo)})"
        if lo <= hi and not (lo == INF or hi == -INF):
            return f"[{readable_double(lo)}, {readable_double(hi)}]" if lo != hi else f"[{readable_double(lo)}]"
        return f"interval({readable_double(lo)}, {readable_double(hi)})"
    if n.op == "empty":
        return "∅"
    if n.op == "entire":
        return "[−∞, +∞]"
    if n.op == "text":
        return f'interval("{a[0]}")'
    if n.op in INFIX:
        return f"{label(a[0])} {INFIX[n.op]} {label(a[1])}"
    if n.op == "neg":
        return f"−{label(a[0])}"
    if n.op in ("addd", "subd", "muld", "divd"):
        return f"{label(a[0])} {INFIX[n.op[:-1]]} {readable_double(a[1])}"
    if n.op == "dmul":
        return f"{readable_double(a[0])} * {label(a[1])}"
    if n.op == "rdiv":
        return f"{label(a[0])} % {label(a[1])}"
    if n.op in ("iadd", "isub", "imul", "idiv", "irdiv"):
        o = {"iadd": "+=", "isub": "-=", "imul": "*=", "idiv": "/=", "irdiv": "%="}[n.op]
        return f"x = {label(a[0])}; x {o} {readable_double(a[1])}"
    if n.op == "iaddI":
        return f"x = {label(a[0])}; x += {label(a[1])}"
    if n.op == "assign":
        return f"x = {label(a[0])}; x = {readable_double(a[1])}"
    if n.op in ("powi", "nth_root"):
        return f"{'pow' if n.op == 'powi' else n.op}({label(a[0])}, {readable_double(a[1])})"
    if n.op == "powd":
        return f"pow({label(a[0])}, {readable_double(a[1])})"
    if n.op == "hull":
        return f"{label(a[0])} | {label(a[1])}"
    if n.op == "inter":
        return f"{label(a[0])} & {label(a[1])}"
    if n.op in ("mid", "wid", "mag", "mig"):
        method = {"mid": "midpoint", "wid": "width", "mag": "mag", "mig": "mig"}[n.op]
        return f"{label(a[0])}.{method}()"
    return f"{n.op}({', '.join(label(x) for x in a)})"


# ---------------------------------------------------------------------------
# Results of IEEE 1788-2015: X(lo, hi) is the interval hull of [lo, hi], its
# bounds being numbers or expressions evaluated by mpmath and rounded outward;
# XR(v, rounding) a number; EMPTYSET the empty set; None, no result defined


class X:
    def __init__(self, lo, hi=None):
        self.lo = lo
        self.hi = lo if hi is None else hi


class XR:
    def __init__(self, v, rounding="nearest"):
        self.v = v
        self.rounding = rounding


EMPTYSET = "empty"

mpmath.mp.prec = 400
MP_NAMES = {name: getattr(mpmath, name) for name in
            ("sin", "cos", "tan", "asin", "acos", "atan", "exp", "log", "sqrt", "sinh", "cosh", "tanh",
             "asinh", "acosh", "atanh", "pi", "mpf")}
MP_NAMES.update({"MAX": mpmath.mpf(MAX), "TINY": mpmath.mpf(TINY), "inf": mpmath.inf})


def next_up(x):
    if math.isnan(x) or x == INF:
        return x
    if x == 0.0:
        return TINY
    bits = struct.unpack("<q", struct.pack("<d", x))[0]
    bits += 1 if x > 0 else -1
    return struct.unpack("<d", struct.pack("<q", bits))[0]


def next_down(x):
    return -next_up(-x)


def mp_value(e):
    if isinstance(e, str):
        return eval(e, {"__builtins__": {}}, MP_NAMES)
    return mpmath.mpf(e)


def to_double(e, rounding):
    v = mp_value(e)
    if mpmath.isinf(v):
        return INF if v > 0 else -INF
    f = float(v) if abs(v) <= MAX else math.copysign(INF, v)
    if rounding == "down" and mpmath.mpf(f) > v:
        f = next_down(f)
    elif rounding == "up" and mpmath.mpf(f) < v:
        f = next_up(f)
    return f


def expected(x):
    """(kind, lo, hi) of an expected result, as the programs give theirs"""
    if x is None:
        return None
    if x == EMPTYSET:
        return ("E",)
    if isinstance(x, XR):
        if isinstance(x.v, float) and math.isnan(x.v):
            return ("R", NAN)
        return ("R", to_double(x.v, x.rounding))
    return ("I", to_double(x.lo, "down"), to_double(x.hi, "up"))


# ---------------------------------------------------------------------------
# The cases


class Case:
    def __init__(self, group, node, ieee, kind="I", note="", name=None):
        self.group = group
        self.node = node
        self.ieee = ieee
        self.kind = kind
        self.note = note
        self.name = name or label(node)
        self.id = None


CASES = []
GROUPS = []


def group(title, source):
    GROUPS.append((title, source))


def case(node, ieee, note="", kind="I", name=None):
    CASES.append(Case(len(GROUPS) - 1, node, ieee, kind, note, name))


I12 = iv(1, 2)

group("Constructors and assignment", "tests/numbers.cpp (constructors)")
case(iv(INF), EMPTYSET, "numsToInterval(+∞, +∞) fails (12.12.7)")
case(iv(-INF), EMPTYSET)
case(iv(INF, INF), EMPTYSET)
case(iv(-INF, -INF), EMPTYSET)
case(iv(INF, 1), EMPTYSET)
case(iv(1, -INF), EMPTYSET)
case(iv(2, 1), EMPTYSET, "l > u: the constructor fails")
case(iv(NAN), EMPTYSET)
case(iv(NAN, 1), EMPTYSET)
case(iv(1, NAN), EMPTYSET)
case(op("assign", I12, INF), EMPTYSET, "libieeep1788 has no assignment of a double: II(d, d)")
case(op("assign", I12, -INF), EMPTYSET)
case(iv(-INF, 1), X("-inf", 1))
case(iv(1, INF), X(1, "inf"))
case(iv(-INF, INF), X("-inf", "inf"))
case(iv(-0.0, 0.0), X(0))
case(iv(-MAX, MAX), X("-MAX", "MAX"))
case(EMPTY, EMPTYSET, "Solaris Studio has no empty constant: [1, 2] .ix. [3, 4]", name="empty set")

group("Reading intervals from text", "tests/numbers.cpp (numbers)")
case(text("0.1"), None, "not an interval literal (9.7.4): ∅, unless the implementation extends the literals "
     "(9.7.1), as GAOL does; Solaris Studio reads 0.1 ± 0.1")
case(text("[0.1]"), X("mpf(1)/10"))
case(text("[0.1, 0.3]"), X("mpf(1)/10", "mpf(3)/10"))
case(text("[1/3, 0.3]"), EMPTYSET, "rational literal, l > u")
case(text("[0.3, 0.1]"), EMPTYSET, "l > u")
case(text("[1e309]"), X("MAX", "inf"), "the real number 1e309, beyond the doubles")
case(text("[1e-400]"), X(0, "TINY"))
case(text("[1, inf]"), X(1, "inf"), "a literal of the set-based flavor (10.5.1)")
case(text("[empty]"), EMPTYSET, "a literal of the set-based flavor (10.5.1)")
case(text("[entire]"), X("-inf", "inf"))
case(text("3.56?1"), X("mpf(355)/100", "mpf(357)/100"), "uncertain form (9.7.4)")

group("Division by an interval containing zero", "tests/arithmetic.cpp (divisions_by_zero)")
for x, y, ieee in [((1, 2), (0, 1), X(1, "inf")), ((1, 2), (-1, 0), X("-inf", -1)),
                   ((-2, -1), (0, 1), X("-inf", -1)), ((-2, -1), (-1, 0), X(1, "inf")),
                   ((1, 2), (-1, 1), X("-inf", "inf")), ((-1, 2), (0, 1), X("-inf", "inf")),
                   ((0, 2), (0, 1), X(0, "inf")), ((-2, 0), (0, 1), X("-inf", 0)),
                   ((0, 0), (-1, 1), X(0)), ((1, 2), (0, 0), EMPTYSET), ((0, 0), (0, 0), EMPTYSET)]:
    case(op("div", iv(*x), iv(*y)), ieee)
case(op("inverse", iv(0, 1)), X(1, "inf"), "recip in IEEE 1788, 1/x in Solaris Studio")
case(op("inverse", iv(0, 0)), EMPTYSET)
case(op("inverse", iv(-1, 1)), X("-inf", "inf"))

group("Relational division (GAOL's %, mulRev of IEEE 1788)", "tests/arithmetic.cpp (divisions_by_zero)")
for x, y, ieee in [((1, 2), (0, 1), X(1, "inf")), ((1, 2), (-1, 0), X("-inf", -1)),
                   ((1, 2), (-1, 1), X("-inf", "inf")), ((0, 0), (-1, 1), X("-inf", "inf")),
                   ((-1, 2), (0, 0), X("-inf", "inf")), ((1, 2), (0, 0), EMPTYSET)]:
    case(op("rdiv", iv(*x), iv(*y)), ieee, "mul_rev(y, x) in libieeep1788" if x == (1, 2) and y == (0, 1) else "")

group("Products with zero and infinite bounds", "tests/arithmetic.cpp (products_with_infinite_bounds)")
for x, y, ieee in [((0, 1), (1, INF), X(0, "inf")), ((-1, 0), (1, INF), X("-inf", 0)),
                   ((0, 0), (1, INF), X(0)), ((0, 0), (-INF, INF), X(0)),
                   ((0, 1), (-INF, INF), X("-inf", "inf")), ((1, INF), (1, INF), X(1, "inf")),
                   ((-INF, -1), (1, INF), X("-inf", -1)), ((-INF, 0), (-INF, 0), X(0, "inf")),
                   ((-INF, INF), (-INF, INF), X("-inf", "inf"))]:
    case(op("mul", iv(*x), iv(*y)), ieee)
case(op("mul", I12, EMPTY), EMPTYSET)
case(op("mul", iv(0, 1), iv(INF)), EMPTYSET, "interval(+∞) is empty")

group("Operators of an interval and a double (+=, -=, *=, /=)",
      "tests/arithmetic.cpp (operations_with_special_doubles)")
NOTE_D = "libieeep1788 has neither mixed operators nor op=: x op II(d, d); Solaris Studio: x = x op d"
case(op("iadd", I12, INF), EMPTYSET, NOTE_D)
case(op("isub", I12, INF), EMPTYSET)
case(op("imul", I12, INF), EMPTYSET)
case(op("idiv", I12, INF), EMPTYSET)
case(op("imul", iv(0, 0), INF), EMPTYSET)
case(op("imul", iv(0, 1), -INF), EMPTYSET)
case(op("iadd", I12, NAN), EMPTYSET)
case(op("idiv", I12, 0.0), EMPTYSET)
case(op("idiv", I12, -0.0), EMPTYSET)
case(op("idiv", iv(-1, 2), 0.0), EMPTYSET)
case(op("irdiv", I12, 0.0), EMPTYSET, "GAOL's %=, mul_rev in libieeep1788")
case(op("iadd", I12, -0.0), X(1, 2))
case(op("imul", I12, -0.0), X(0))
case(op("imul", iv(1, INF), 0.0), X(0))
case(op("imul", iv(-INF, INF), 0.0), X(0))
case(op("isub", iv(1, INF), MAX), X("1-MAX", "inf"))
case(op("iadd", iv(MAX, MAX), MAX), X("MAX", "inf"))
case(op("iadd", EMPTY, 1.0), EMPTYSET)
case(op("iaddI", I12, iv(INF)), EMPTYSET)
case(op("addd", I12, INF), EMPTYSET)
case(op("muld", iv(0, 1), INF), EMPTYSET)
case(op("dmul", 0.0, iv(1, INF)), X(0))
case(op("divd", iv(1, 2), 0.0), EMPTYSET)

group("Elementary functions at the edges of their domains", "tests/elementary.cpp (at_known_intervals), check/")
case(op("sin", iv(1, 2)), X("sin(1)", 1))
case(op("sin", iv(4, 5)), X(-1, "sin(4)"))
case(op("sin", iv(0, 7)), X(-1, 1))
case(op("sin", ENTIRE), X(-1, 1))
case(op("cos", iv(3, 4)), X(-1, "cos(4)"))
case(op("cos", iv(-1, 1)), X("cos(1)", 1))
case(op("cos", iv(0, 7)), X(-1, 1))
case(op("cos", iv(2**52 - 1)), X("cos(2**52-1)"), "a wrong mathlib gives −0.4855 (see manual)")
case(op("tan", iv(1, 2)), X("-inf", "inf"))
case(op("tan", iv(-1, 1)), X("tan(-1)", "tan(1)"))
case(op("tan", ENTIRE), X("-inf", "inf"))
case(op("log", iv(-1, 1)), X("-inf", 0))
case(op("log", iv(0, 1)), X("-inf", 0))
case(op("log", iv(-4, 0)), EMPTYSET, "no x > 0 in [−4, 0]: GAOL and Solaris Studio take log(0) = −∞")
case(op("log", iv(0, 0)), EMPTYSET)
case(op("log", iv(-2, -1)), EMPTYSET)
case(op("log", iv(1, INF)), X(0, "inf"))
case(op("exp", iv(-INF, 0)), X(0, 1))
case(op("exp", iv(740)), X("MAX", "inf"))
case(op("exp", iv(-800)), X(0, "TINY"))
case(op("exp", ENTIRE), X(0, "inf"))
case(op("exp", EMPTY), EMPTYSET)
case(op("sqrt", iv(-4, 4)), X(0, 2))
case(op("sqrt", iv(-4, -1)), EMPTYSET)
case(op("sqrt", ENTIRE), X(0, "inf"))
case(op("sqr", ENTIRE), X(0, "inf"), "x**2 in Solaris Studio")
case(op("sqr", iv(-INF, -MAX)), X("MAX", "inf"))
case(op("asin", iv(-2, 2)), X("-pi/2", "pi/2"))
case(op("asin", iv(2, 3)), EMPTYSET)
case(op("acos", iv(-2, 2)), X(0, "pi"))
case(op("acos", iv(1, 3)), X(0))
case(op("acos", iv(-3, -1)), X("pi"))
case(op("acos", iv(-3, -2)), EMPTYSET)
case(op("atan", ENTIRE), X("-pi/2", "pi/2"))
case(op("sinh", ENTIRE), X("-inf", "inf"))
case(op("cosh", ENTIRE), X(1, "inf"))
case(op("tanh", ENTIRE), X(-1, 1))
case(op("asinh", ENTIRE), X("-inf", "inf"), "Solaris Studio has no asinh, acosh, atanh")
case(op("acosh", iv(0, 1)), X(0))
case(op("acosh", iv(-1, 0.5)), EMPTYSET)
case(op("atanh", iv(-1, 1)), X("-inf", "inf"))
case(op("atanh", iv(2, 3)), EMPTYSET)
case(op("abs", iv(-INF, -1)), X(1, "inf"))
case(op("abs", iv(-INF, 1)), X(0, "inf"))

group("Integer powers (GAOL's pow(x, n), pown of IEEE 1788, x**n of Fortran)",
      "check/non_arithmetic.cpp (test_pow_int), tests/arithmetic.cpp")
case(op("powi", iv(0), 0), X(1), "pown(x, 0) = 1 for every x (Table 9.1, b)")
case(op("powi", EMPTY, 0), EMPTYSET)
case(op("powi", ENTIRE, 0), X(1))
case(op("powi", iv(-3, 5), 2), X(0, 25))
case(op("powi", iv(-2, 3), 3), X(-8, 27))
case(op("powi", iv(-3, 2), -1), X("-inf", "inf"))
case(op("powi", iv(-3, 2), -2), X("mpf(1)/9", "inf"))
case(op("powi", iv(-3, 2), -3), X("-inf", "inf"))
case(op("powi", iv(-3, -2), -3), X("-mpf(1)/8", "-mpf(1)/27"))
case(op("powi", iv(2, 3), -4), X("mpf(1)/81", "mpf(1)/16"))
case(op("powi", iv(0), -1), EMPTYSET, "pown(0, p) has no value for p < 0")
case(op("powi", iv(0, 2), -1), X(0.5, "inf"))
case(op("powi", iv(-2, 0), -1), X("-inf", -0.5))
case(op("powi", iv(-2, 0), -2), X(0.25, "inf"))
case(op("powi", iv(0, INF), -2), X(0, "inf"))
case(op("powi", iv(-INF, -MAX), 3), X("-inf", "-MAX"))
case(op("powi", iv(-INF, -MAX), 4), X("MAX", "inf"))
case(op("powi", iv(MAX, INF), 3), X("MAX", "inf"))
case(op("powi", iv(-15), 17), X("-mpf(15)**17"))
case(op("powi", iv(10), 400), X("MAX", "inf"))
case(op("powi", iv(10), -400), X(0, "TINY"))

group("Real powers (GAOL's pow(x, d) and pow(x, y), pow of IEEE 1788, x**y of Fortran)",
      "tests/elementary.cpp (powers)")
NOTE_POWN = "GAOL: pown for an integer exponent (choice of the fork), IEEE 1788's pow: x > 0 only"
case(op("powd", iv(4), 0.5), X(2), "libieeep1788: pow(x, II(d, d))")
case(op("powd", iv(4), 1.5), X(8))
case(op("powd", iv(4, 9), 0.5), X(2, 3))
case(op("powd", iv(4), -0.5), X(0.5))
case(op("powd", iv(0, 4), 0.5), X(0, 2))
case(op("powd", iv(-4, 9), 0.5), X(0, 3))
case(op("pow", iv(-2, 3), iv(1, 2)), X(0, 9))
case(op("powd", iv(-10, 10), -2.0), X("mpf(1)/100", "inf"))
case(op("powd", iv(-2, 3), 3.0), X(0, 27), NOTE_POWN)
case(op("powd", iv(-2), 2.0), EMPTYSET, NOTE_POWN)
case(op("powd", iv(2, 3), 4.0), X(16, 81))
case(op("powd", iv(4), 0.0), X(1))
case(op("pow", iv(-2, 3), iv(3)), X(0, 27), NOTE_POWN)
case(op("pow", iv(-2, 3), iv(2)), X(0, 9))
case(op("pow", iv(3, 4), iv(2, 3)), X(9, 64))
case(op("powd", iv(-4, -1), 0.5), EMPTYSET)
case(op("pow", iv(-4, -1), iv(0.5)), EMPTYSET)
case(op("powd", iv(4), INF), EMPTYSET, "the exponent interval(+∞) is empty")
case(op("powd", iv(4), -INF), EMPTYSET)
case(op("powd", iv(4), NAN), EMPTYSET)
case(op("pow", iv(4), iv(INF)), EMPTYSET)
case(op("pow", iv(4), iv(-INF)), EMPTYSET)
case(op("powd", EMPTY, 1.5), EMPTYSET)
case(op("pow", iv(4), EMPTY), EMPTYSET)
case(op("pow", iv(-2, -1), iv(1e10)), EMPTYSET, "GAOL: pown, [−∞, +∞] for an integer beyond the ints")
case(op("pow", iv(2, 3), iv(2.0**31)), X("MAX", "inf"), "GAOL: [−∞, +∞] for an integer beyond the ints")
case(op("pow", iv(0.25, 0.5), iv(-2.0**31 - 1)), X("MAX", "inf"))
case(op("powd", iv(-2, 3), 1e10), X(0, "inf"))
case(op("pow", EMPTY, iv(1e10)), EMPTYSET)
case(op("pow", iv(-1), iv(2.0**31 - 1)), EMPTYSET, NOTE_POWN)
case(op("pow", iv(0), iv(0)), EMPTYSET, "pow(0, y) has no value for y ≤ 0 (Table 9.1, c); GAOL: pown(0, 0) = 1")
case(op("powd", iv(0), 0.0), EMPTYSET)
case(op("pow", ENTIRE, iv(0)), X(1))
case(op("pow", iv(0, 2), iv(-1)), X(0.5, "inf"))
case(op("pow", iv(-2, 0), iv(-1)), EMPTYSET, NOTE_POWN)
case(op("pow", iv(0), iv(0.5)), X(0), "pow(0, y) = 0 for y > 0")
case(op("powd", iv(0), 0.5), X(0))
case(op("pow", iv(-2, 0), iv(2.5)), X(0))
case(op("pow", iv(0), iv(0, 1)), X(0))
case(op("pow", iv(0), iv(-1, 1)), X(0))
case(op("pow", iv(0), iv(-1)), EMPTYSET)
case(op("pow", iv(0), iv(-0.5)), EMPTYSET)
case(op("powd", iv(0), -0.5), EMPTYSET)
case(op("pow", iv(-2, 0), iv(-0.5)), EMPTYSET)
case(op("pow", iv(0), iv(-1, 0)), EMPTYSET)
case(op("pow", iv(0.5), ENTIRE), X(0, "inf"))
case(op("pow", iv(1), ENTIRE), X(1))
case(op("pow", iv(0, 1), iv(1, INF)), X(0, 1))
case(op("pow", iv(2, INF), iv(-1, 1)), X(0, "inf"))

group("n-th roots (GAOL's nth_root, rootn of IEEE 1788)", "check/non_arithmetic.cpp, tests/arithmetic.cpp")
NOTE_ROOT = "neither libieeep1788 nor Solaris Studio has rootn"
case(op("nth_root", iv(-8, 27), 3), X(-2, 3), NOTE_ROOT + "; GAOL takes the roots of x ∩ [0, +∞] for every n "
     "(check/non_arithmetic.cpp)")
case(op("nth_root", iv(-4, 9), 2), X(0, 3))
case(op("nth_root", iv(-4, -1), 2), EMPTYSET)
case(op("nth_root", ENTIRE, 0), None, "rootn(x, q) is for q ≠ 0 only (Table 10.5)")
case(op("nth_root", iv(0, INF), 2), X(0, "inf"))

group("Numeric and set functions", "tests/other_functions.cpp, tests/numbers.cpp")
case(op("mid", ENTIRE), XR(0.0), "mid (12.12.8)", kind="R")
case(op("mid", iv(-INF, 1)), XR(-MAX), kind="R")
case(op("mid", iv(1, INF)), XR(MAX), kind="R")
case(op("mid", EMPTY), XR(NAN), kind="R")
case(op("mid", iv(MAX / 2, MAX)), XR("(MAX/2 + MAX)/2"), kind="R")
case(op("mid", iv(0, TINY)), XR(0.0), "the tie rounded to even", kind="R")
case(op("mid", iv(-TINY, TINY * 4)), XR(TINY * 2), "1.5·2^-1074, to even", kind="R")
case(op("wid", iv(-INF, 1)), XR(INF), "wid (12.12.8)", kind="R")
case(op("wid", iv(-MAX, MAX)), XR(INF), kind="R")
case(op("wid", EMPTY), XR(NAN), "GAOL returns −1 for the empty set", kind="R")
case(op("mag", iv(-3, 2)), XR(3.0), kind="R")
case(op("mag", EMPTY), XR(NAN), kind="R")
case(op("mig", iv(-3, 2)), XR(0.0), kind="R")
case(op("mig", iv(-3, -2)), XR(2.0), kind="R")
case(op("mig", EMPTY), XR(NAN), kind="R")
case(op("hull", I12, EMPTY), X(1, 2), "convex_hull in libieeep1788, .ih. in Solaris Studio")
case(op("inter", I12, iv(3, 4)), EMPTYSET, "intersection in libieeep1788, .ix. in Solaris Studio")
case(op("inter", I12, iv(2, 3)), X(2))
case(op("min", iv(-INF, 1), iv(0, 2)), X("-inf", 1))
case(op("max", I12, EMPTY), EMPTYSET)
case(op("neg", iv(-INF, 1)), X(-1, "inf"))


for i, c in enumerate(CASES):
    c.id = f"c{i + 1:03d}"


# ---------------------------------------------------------------------------
# The programs

GAOL_HEAD = r'''// Generated by doc/compare/code/cases.py: the special cases, computed by GAOL
#include <gaol/gaol.h>
#include <cstdio>
#include <exception>
#include <limits>
#include <string>

using namespace gaol;

static const double pinf = std::numeric_limits<double>::infinity();
static const double qnan = std::numeric_limits<double>::quiet_NaN();
static const double dmax = std::numeric_limits<double>::max();
static const double dtiny = std::numeric_limits<double>::denorm_min();

template<class F>
static void show(const char *id, F f)
{
  try {
    const interval r = f();
    if (r.is_empty()) {
      std::printf("%s|E\n", id);
    } else {
      std::printf("%s|I|%a|%a\n", id, r.left(), r.right());
    }
  } catch (input_format_error&) {
    std::printf("%s|X|exception input_format_error\n", id);
  } catch (unavailable_feature_error&) {
    std::printf("%s|X|exception unavailable_feature_error\n", id);
  } catch (std::exception&) {
    std::printf("%s|X|exception\n", id);
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

template<class F>
static void show_real(const char *id, F f)
{
  try {
    std::printf("%s|R|%a\n", id, f());
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

int main()
{
  gaol::init();
'''

GAOL_TAIL = r'''  gaol::cleanup();
  return 0;
}
'''

P1788_HEAD = r'''// Generated by doc/compare/code/cases.py: the special cases, computed by libieeep1788
#include <p1788/p1788.hpp>
#include <cstdio>
#include <limits>
#include <string>

typedef p1788::infsup::interval<double, p1788::flavor::infsup::setbased::mpfr_bin_ieee754_flavor> II;

static const double pinf = std::numeric_limits<double>::infinity();
static const double qnan = std::numeric_limits<double>::quiet_NaN();
static const double dmax = std::numeric_limits<double>::max();
static const double dtiny = std::numeric_limits<double>::denorm_min();

template<class F>
static void show(const char *id, F f)
{
  try {
    const II r = f();
    if (is_empty(r)) {
      std::printf("%s|E\n", id);
    } else {
      std::printf("%s|I|%a|%a\n", id, inf(r), sup(r));
    }
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

template<class F>
static void show_real(const char *id, F f)
{
  try {
    std::printf("%s|R|%a\n", id, f());
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

int main()
{
'''

P1788_TAIL = r'''  return 0;
}
'''

SUN_HEAD = '''! Generated by doc/compare/code/cases.py: the special cases, computed by the
! intervals of Solaris Studio's Fortran (f90 -xia)
module show_results
  implicit none
contains
  subroutine showi(id, r)
    character(len=*), intent(in) :: id
    interval(8), intent(in) :: r
    if (isempty(r)) then
      write(*, '(A,A)') id, '|E'
    else
      write(*, '(A,A,Z16.16,A,Z16.16)') id, '|I|b:', inf(r), '|b:', sup(r)
    end if
  end subroutine showi

  subroutine showr(id, v)
    character(len=*), intent(in) :: id
    real(8), intent(in) :: v
    write(*, '(A,A,Z16.16)') id, '|R|b:', v
  end subroutine showr

  subroutine showx(id, ios)
    character(len=*), intent(in) :: id
    integer, intent(in) :: ios
    write(*, '(A,A,I0)') id, '|X|read error, iostat ', ios
  end subroutine showx

  subroutine showna(id)
    character(len=*), intent(in) :: id
    write(*, '(A,A)') id, '|NA'
  end subroutine showna
end module show_results

program cases
  use show_results
  implicit none
  interval(8) :: r, emptyi, onei
  real(8) :: v, pinf, ninf, qnan, dmax, dtiny, mzero
  integer :: ios
  character(len=64) :: s

  ! The special values, computed at run time
  dmax = huge(1d0)
  pinf = dmax
  pinf = pinf*2d0
  ninf = -pinf
  qnan = pinf - pinf
  dtiny = tiny(1d0)
  dtiny = dtiny*(2d0**(-52))
  mzero = sign(0d0, -1d0)
  emptyi = interval(1d0, 2d0) .ix. interval(3d0, 4d0)
  onei = interval(1d0)
'''

SUN_TAIL = '''end program cases
'''


def generate(directory):
    import os
    os.makedirs(directory, exist_ok=True)
    for lib, head, tail, name in (("gaol", GAOL_HEAD, GAOL_TAIL, "cases_gaol.cpp"),
                                  ("p1788", P1788_HEAD, P1788_TAIL, "cases_p1788.cpp")):
        lines = [head]
        for c in CASES:
            lines.append(f"  // {c.name}\n")
            try:
                body = cpp_body(c.node, lib)
            except NotAvailable:
                lines.append(f'  std::printf("{c.id}|NA\\n");\n')
                continue
            result = "double" if c.kind == "R" else ("interval" if lib == "gaol" else "II")
            show = "show_real" if c.kind == "R" else "show"
            lines.append(f'  {show}("{c.id}", []() -> {result} {{ {body} }});\n')
        lines.append(tail)
        with open(os.path.join(directory, name), "w") as f:
            f.write("".join(lines))

    lines = [SUN_HEAD]
    for c in CASES:
        lines.append(f"\n  ! {c.id}: {c.name}\n")
        try:
            statements = f90_statements(c.node, c.id, c.kind)
        except NotAvailable:
            statements = [f"call showna('{c.id}')"]
        for s in statements:
            lines.append(f"  {s}\n")
    lines.append(SUN_TAIL)
    with open(os.path.join(directory, "cases_sun.f90"), "w") as f:
        f.write("".join(lines))


# ---------------------------------------------------------------------------
# The report


def parse_double(s):
    if s.startswith("b:"):
        return struct.unpack(">d", bytes.fromhex(s[2:]))[0]
    return float.fromhex(s)


def read_results(path):
    results = {}
    with open(path) as f:
        for line in f:
            parts = line.rstrip("\n").split("|")
            if len(parts) < 2 or not parts[0].startswith("c"):
                continue
            cid, kind = parts[0].strip(), parts[1]
            if kind == "I":
                results[cid] = ("I", parse_double(parts[2]), parse_double(parts[3]))
            elif kind == "R":
                results[cid] = ("R", parse_double(parts[2]))
            elif kind == "X":
                results[cid] = ("X", "|".join(parts[2:]))
            else:
                results[cid] = (kind,)
    return results


def show_result(r):
    if r is None:
        return "?"
    if r[0] == "E":
        return "∅"
    if r[0] == "NA":
        return "n/a"
    if r[0] == "X":
        return r[1]
    if r[0] == "R":
        return readable_double(r[1])
    lo, hi = r[1], r[2]
    if lo == hi:
        return f"[{readable_double(lo)}]"
    return f"[{readable_double(lo)}, {readable_double(hi)}]"


def same_double(a, b):
    return (math.isnan(a) and math.isnan(b)) or a == b


def verdict(r, e):
    """'=' when r is the result of IEEE 1788, '⊃' when r encloses it, '✗' otherwise"""
    if e is None or r is None or r[0] in ("NA",):
        return ""
    if r[0] == "X":
        return "✗"
    if e[0] == "E":
        return "=" if r[0] == "E" else "✗"
    if e[0] == "R":
        return "=" if r[0] == "R" and same_double(r[1], e[1]) else "✗"
    if r[0] != "I":
        return "✗"
    if r[1] == e[1] and r[2] == e[2]:
        return "="
    if r[1] <= e[1] and r[2] >= e[2]:
        return "⊃"
    return "✗"


BEGIN = "<!-- BEGIN GENERATED TABLES (doc/compare/code/cases.py) -->"
END = "<!-- END GENERATED TABLES -->"


def report(paths, out):
    names = ["GAOL", "libieeep1788", "Solaris Studio"]
    results = [read_results(p) for p in paths]
    counts = [{"=": 0, "⊃": 0, "✗": 0, "n/a": 0} for _ in names]
    lines = []
    for g, (title, source) in enumerate(GROUPS):
        lines.append(f"### {g + 1}. {title}\n\n")
        lines.append(f"From {source}.\n\n")
        lines.append("| # | Operation | IEEE 1788 | GAOL | libieeep1788 | Solaris Studio | Notes |\n")
        lines.append("|---|---|---|---|---|---|---|\n")
        for c in CASES:
            if c.group != g:
                continue
            e = expected(c.ieee)
            cells = []
            for k, res in enumerate(results):
                r = res.get(c.id)
                v = verdict(r, e)
                if r is not None and r[0] == "NA":
                    counts[k]["n/a"] += 1
                elif v:
                    counts[k][v] += 1
                text_ = show_result(r).replace("|", "\\|")
                cells.append(f"{text_} {v}".strip() if v != "=" else f"{text_} ✓")
            ieee = show_result(e) if e else "—"
            name = c.name.replace("|", "\\|")
            lines.append(f"| {c.id[1:]} | `{name}` | {ieee} | {' | '.join(cells)} | {c.note} |\n")
        lines.append("\n")
    summary = ["| | " + " | ".join(names) + " |\n", "|---|" + "---|" * len(names) + "\n"]
    for key, meaning in (("=", "✓ the result of IEEE 1788"), ("⊃", "⊃ encloses it, wider"),
                         ("✗", "✗ differs"), ("n/a", "n/a no such operation")):
        summary.append(f"| {meaning} | " + " | ".join(str(c[key]) for c in counts) + " |\n")
    table = (f"{len(CASES)} cases. Each result is marked against the result IEEE 1788-2015 defines "
             "(the tightest interval of doubles, computed with mpmath):\n\n" + "".join(summary) + "\n" + "".join(lines))
    block = BEGIN + "\n\n" + table + END + "\n"
    try:
        with open(out) as f:
            content = f.read()
    except FileNotFoundError:
        content = ""
    if BEGIN in content and END in content:
        head, rest = content.split(BEGIN, 1)
        tail = rest.split(END, 1)[1]
        content = head + block + tail.lstrip("\n")
    else:
        content = block
    with open(out, "w") as f:
        f.write(content)
    for name, c in zip(names, counts):
        print(f"{name}: {c['=']} as IEEE 1788, {c['⊃']} wider, {c['✗']} different, {c['n/a']} n/a")


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "generate":
        generate(sys.argv[2])
    elif len(sys.argv) == 6 and sys.argv[1] == "report":
        report(sys.argv[2:5], sys.argv[5])
    else:
        sys.exit(__doc__)
