#!/usr/bin/env python3
"""Special cases of interval arithmetic in GAOL, libieeep1788, filib++, PROFIL/BIAS and Solaris Studio.

The cases come from the tests of GAOL (tests/*.cpp, and check/*.cpp for the
integer powers). Each one is written once, below, as an expression, from which
this script generates a program for each library; the results the programs
print are then compared with the results IEEE 1788-2015 defines, computed here
with mpmath and rounded outward.

    cases.py generate DIR
        writes DIR/cases_gaol.cpp, DIR/cases_p1788.cpp, DIR/cases_filib.cpp, DIR/cases_profil.cpp
        and DIR/cases_sun.f90
    cases.py report DIR REPORT.md
        reads what the five programs printed, DIR/gaol.txt, DIR/p1788.txt,
        DIR/filib.txt, DIR/profil.txt and DIR/sun.txt, and writes the table of the results
        between the markers of REPORT.md (the whole file when it does not
        exist)

Each program prints one line per case:
    id|I|lo|hi   an interval, the bounds in C hexadecimal (%a) or as the bits
                 of the double in hexadecimal, prefixed by b:
    id|E         the empty set
    id|R|v       a number
    id|B|b       a boolean, 0 or 1
    id|X|text    an exception, or a string the library could not read
    id|NA        the library has no such operation
"""

# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ

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
        # −(2^31+1), but −9.852612533569336e+19
        return "−" + ("(" + s + ")" if ("+" in s or "−" in s) and "e" not in s else s)
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


# The C++ libraries: the name of their interval type, and whether they have
# operators of an interval and a double, and compound assignments. PROFIL/BIAS
# has no empty set: the cases with one are not available to it
CPP_TYPE = {"gaol": "interval", "p1788": "II", "filib": "FI", "profil": "INTERVAL"}
MIXED = {"gaol": True, "p1788": False, "filib": True, "profil": True}

# The functions of PROFIL/BIAS (Functions.h)
PROFIL_FUNCTIONS = {"sin": "Sin", "cos": "Cos", "tan": "Tan", "asin": "ArcSin", "acos": "ArcCos", "atan": "ArcTan",
                    "exp": "Exp", "log": "Log", "sqrt": "Sqrt", "sinh": "Sinh", "cosh": "Cosh", "tanh": "Tanh",
                    "asinh": "ArSinh", "acosh": "ArCosh", "atanh": "ArTanh", "abs": "IAbs", "sqr": "Sqr"}


def cpp(n, lib):
    """The C++ expression of n; lib is 'gaol', 'p1788', 'filib' or 'profil'"""
    t = CPP_TYPE[lib]
    a = n.args

    def point(d):  # the interval of a double
        return f"{t}({cpp_double(d)})" if lib != "p1788" else f"II({cpp_double(d)}, {cpp_double(d)})"

    if n.op == "iv":
        lo, hi = a
        return point(lo) if hi is None else f"{t}({cpp_double(lo)}, {cpp_double(hi)})"
    if n.op == "empty":
        if lib == "profil":
            raise NotAvailable
        return {"gaol": "interval::emptyset()", "p1788": "II::empty()", "filib": "FI::EMPTY()"}[lib]
    if n.op == "entire":
        return {"gaol": "interval::universe()", "p1788": "II::entire()", "filib": "FI::ENTIRE()",
                "profil": "INTERVAL(-pinf, pinf)"}[lib]
    if n.op == "text":
        s = a[0].replace("\\", "\\\\").replace('"', '\\"')
        return {"gaol": f'interval("{s}")', "p1788": f'II(std::string("{s}"))', "filib": f'read_interval("{s}")',
                "profil": f'read_interval("{s}")'}[lib]
    if n.op in INFIX:
        return f"({cpp(a[0], lib)} {INFIX[n.op]} {cpp(a[1], lib)})"
    if n.op == "neg":
        return f"(-{cpp(a[0], lib)})"
    if n.op in ("addd", "subd", "muld", "divd"):  # interval op double
        d = cpp_double(a[1]) if MIXED[lib] else point(a[1])
        return f"({cpp(a[0], lib)} {INFIX[n.op[:-1]]} {d})"
    if n.op == "dmul":  # double * interval
        d = cpp_double(a[0]) if MIXED[lib] else point(a[0])
        return f"({d} * {cpp(a[1], lib)})"
    if n.op == "rdiv":  # GAOL's relational division x % y: mul_rev of IEEE 1788
        if lib in ("filib", "profil"):
            raise NotAvailable
        return f"({cpp(a[0], lib)} % {cpp(a[1], lib)})" if lib == "gaol" else f"mul_rev({cpp(a[1], lib)}, {cpp(a[0], lib)})"
    if n.op in FUNCTIONS:
        if lib == "profil":
            return f"{PROFIL_FUNCTIONS[n.op]}({cpp(a[0], lib)})"
        return f"{n.op}({cpp(a[0], lib)})"
    if n.op == "inverse":
        return {"gaol": "inverse({})", "p1788": "recip({})", "filib": "(1.0 / {})",
                "profil": "(1.0 / {})"}[lib].format(cpp(a[0], lib))
    if n.op == "powi":
        return {"gaol": "pow({}, {})", "p1788": "pown({}, {})", "filib": "power({}, {})",
                "profil": "Power({}, {})"}[lib].format(cpp(a[0], lib), a[1])
    if n.op == "powd":
        if lib == "gaol":
            return f"pow({cpp(a[0], lib)}, {cpp_double(a[1])})"
        if lib == "profil":
            return f"Power({cpp(a[0], lib)}, {point(a[1])})"
        return f"pow({cpp(a[0], lib)}, {point(a[1])})"
    if n.op == "pow":
        if lib == "profil":
            return f"Power({cpp(a[0], lib)}, {cpp(a[1], lib)})"
        return f"pow({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op == "atan2":  # filib++ and PROFIL/BIAS have none
        if lib in ("filib", "profil"):
            raise NotAvailable
        return f"atan2({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op == "nth_root":
        if lib == "profil":
            return f"Root({cpp(a[0], lib)}, {a[1]})"
        if lib != "gaol":
            raise NotAvailable
        return f"nth_root({cpp(a[0], lib)}, {a[1]}u)"
    if n.op == "hull":
        return {"gaol": "({} | {})", "p1788": "convex_hull({}, {})", "filib": "hull({}, {})",
                "profil": "Hull({}, {})"}[lib].format(cpp(a[0], lib), cpp(a[1], lib))
    if n.op == "inter":
        return {"gaol": "({} & {})", "p1788": "intersection({}, {})", "filib": "intersect({}, {})",
                "profil": "intersect({}, {})"}[lib].format(cpp(a[0], lib), cpp(a[1], lib))
    if n.op in ("min", "max"):
        if lib == "profil":
            raise NotAvailable
        name = "i" + n.op if lib == "filib" else n.op
        return f"{name}({cpp(a[0], lib)}, {cpp(a[1], lib)})"
    if n.op in ("mid", "wid", "mag", "mig", "rad"):
        if lib == "p1788":
            return f"{n.op}({cpp(a[0], lib)})"
        if lib == "profil":
            if n.op == "rad":
                raise NotAvailable
            return {"mid": "Mid", "wid": "Diam", "mag": "Abs", "mig": "Mig"}[n.op] + f"({cpp(a[0], lib)})"
        methods = {"gaol": {"mid": "midpoint", "wid": "width"}, "filib": {"wid": "diam"}}[lib]
        return f"{cpp(a[0], lib)}.{methods.get(n.op, n.op)}()"
    if n.op in RELATIONS:
        x, y = cpp(a[0], lib), cpp(a[1], lib)
        name = RELATIONS[n.op][lib]
        if name is None:
            raise NotAvailable
        if lib == "gaol":
            # GAOL's methods: x.certainly_leq(y), and y.set_contains(x) for subset
            return f"{y}.{name[1:]}({x})" if name.startswith("~") else f"{x}.{name}({y})"
        return f"{name}({x}, {y})"
    raise ValueError(n.op)


# The comparisons of IEEE 1788-2015 (Table 10.3), in each library; ~ marks a
# GAOL method called on the second interval
# PROFIL/BIAS: x <= y is the inclusion of x in y, x < y in its interior, x == y
# the equality; the others are written with Inf and Sup in the program
# (precedes, strict_precedes, disjoint)
RELATIONS = {
    "precedes": {"gaol": "certainly_leq", "p1788": "precedes", "filib": "cle", "sun": ".cle.", "profil": "precedes"},
    "strict_precedes": {"gaol": "certainly_le", "p1788": "strictly_precedes", "filib": "clt", "sun": ".clt.",
                        "profil": "strict_precedes"},
    "interior": {"gaol": "~set_strictly_contains", "p1788": "interior", "filib": "interior", "sun": ".int.",
                 "profil": "interior"},
    "subset": {"gaol": "~set_contains", "p1788": "subset", "filib": "subset", "sun": ".sb.", "profil": "subset"},
    "equal": {"gaol": "set_eq", "p1788": "equal", "filib": "seq", "sun": ".seq.", "profil": "equal"},
    "disjoint": {"gaol": "set_disjoint", "p1788": "disjoint", "filib": "disjoint", "sun": ".dj.", "profil": "disjoint"},
}


def cpp_body(n, lib):
    """The body of the lambda computing the case n"""
    t = CPP_TYPE[lib]
    a = n.args
    if n.op in ("iadd", "isub", "imul", "idiv", "irdiv"):  # x op= d
        x, d = a
        if n.op == "irdiv" and lib in ("filib", "profil"):
            raise NotAvailable
        if MIXED[lib]:
            o = {"iadd": "+=", "isub": "-=", "imul": "*=", "idiv": "/=", "irdiv": "%="}[n.op]
            return f"{t} r = {cpp(x, lib)}; r {o} {cpp_double(d)}; return r;"
        if n.op == "irdiv":
            return f"return mul_rev(II({cpp_double(d)}, {cpp_double(d)}), {cpp(x, lib)});"
        return f"return {cpp(x, lib)} {INFIX[n.op[1:]]} II({cpp_double(d)}, {cpp_double(d)});"
    if n.op == "iaddI":  # x += y
        x, y = a
        if MIXED[lib]:
            return f"{t} r = {cpp(x, lib)}; r += {cpp(y, lib)}; return r;"
        return f"return {cpp(x, lib)} + {cpp(y, lib)};"
    if n.op == "assign":  # x = d
        x, d = a
        if MIXED[lib]:
            return f"{t} r = {cpp(x, lib)}; r = {cpp_double(d)}; return r;"
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
    if n.op == "atan2":
        return f"atan2({f90(a[0])}, {f90(a[1])})"
    if n.op == "hull":
        return f"({f90(a[0])} .ih. {f90(a[1])})"
    if n.op == "inter":
        return f"({f90(a[0])} .ix. {f90(a[1])})"
    if n.op in ("min", "max", "mid", "wid", "mag", "mig"):
        return f"{n.op}({', '.join(f90(x) for x in a)})"
    if n.op == "rad":
        raise NotAvailable
    if n.op in RELATIONS:
        return f"({f90(a[0])} {RELATIONS[n.op]['sun']} {f90(a[1])})"
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
    if kind == "B":
        return [f"t = {f90(n)}", f"call showb('{cid}', t)"]
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
    if n.op in ("mid", "wid", "mag", "mig", "rad"):
        method = {"mid": "midpoint", "wid": "width", "mag": "mag", "mig": "mig", "rad": "rad"}[n.op]
        return f"{label(a[0])}.{method}()"
    if n.op in RELATIONS:
        name = {"precedes": "precedes", "strict_precedes": "strictPrecedes"}.get(n.op, n.op)
        return f"{name}({label(a[0])}, {label(a[1])})"
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


class XB:
    def __init__(self, v):
        self.v = v


EMPTYSET = "empty"

mpmath.mp.prec = 400
MP_NAMES = {name: getattr(mpmath, name) for name in
            ("sin", "cos", "tan", "asin", "acos", "atan", "exp", "log", "sqrt", "sinh", "cosh", "tanh",
             "asinh", "acosh", "atanh", "atan2", "pi", "mpf")}
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
    if isinstance(x, XB):
        return ("B", 1 if x.v else 0)
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
case(text("[0.1]"), X("mpf(1)/10"), "filib++: operator>>, which reads [l, u] only, and rounds the bounds to nearest")
case(text("[0.1, 0.3]"), X("mpf(1)/10", "mpf(3)/10"))
case(text("[1/3, 0.3]"), EMPTYSET, "rational literal, l > u")
case(text("[0.3, 0.1]"), EMPTYSET, "l > u")
case(text("[1e309]"), X("MAX", "inf"), "the real number 1e309, beyond the doubles")
case(text("[1e-400]"), X(0, "TINY"))
case(text("[1, inf]"), X(1, "inf"), "a literal of the set-based flavor (10.5.1)")
case(text("[empty]"), EMPTYSET, "a literal of the set-based flavor (10.5.1); filib++ reads [ EMPTY ]")
case(text("[entire]"), X("-inf", "inf"))
case(text("3.56?1"), X("mpf(355)/100", "mpf(357)/100"), "uncertain form (9.7.4)")

group("Division by an interval containing zero", "tests/arithmetic.cpp (divisions_by_zero)")
for x, y, ieee in [((1, 2), (0, 1), X(1, "inf")), ((1, 2), (-1, 0), X("-inf", -1)),
                   ((-2, -1), (0, 1), X("-inf", -1)), ((-2, -1), (-1, 0), X(1, "inf")),
                   ((1, 2), (-1, 1), X("-inf", "inf")), ((-1, 2), (0, 1), X("-inf", "inf")),
                   ((0, 2), (0, 1), X(0, "inf")), ((-2, 0), (0, 1), X("-inf", 0)),
                   ((0, 0), (-1, 1), X(0)), ((1, 2), (0, 0), EMPTYSET), ((0, 0), (0, 0), EMPTYSET)]:
    case(op("div", iv(*x), iv(*y)), ieee)
case(op("inverse", iv(0, 1)), X(1, "inf"), "recip in IEEE 1788, 1/x in filib++ and Solaris Studio")
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
case(op("cos", iv(2**52 - 1)), X("cos(2**52-1)"))
case(op("tan", iv(1, 2)), X("-inf", "inf"))
case(op("tan", iv(-1, 1)), X("tan(-1)", "tan(1)"))
case(op("tan", ENTIRE), X("-inf", "inf"))
case(op("log", iv(-1, 1)), X("-inf", 0))
case(op("log", iv(0, 1)), X("-inf", 0))
case(op("log", iv(-4, 0)), EMPTYSET, "no x > 0 in [−4, 0]: Solaris Studio takes log(0) = −∞")
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

group("Integer powers (GAOL's pow(x, n), pown of IEEE 1788, filib++'s power(x, n), x**n of Fortran)",
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

group("Real powers (GAOL's pow(x, d) and pow(x, y), pow of IEEE 1788 and of filib++, x**y of Fortran)",
      "tests/elementary.cpp (powers)")
NOTE_POWN = "GAOL: pown for an integer exponent (choice of GAOL v5), IEEE 1788's pow: x > 0 only"
case(op("powd", iv(4), 0.5), X(2), "libieeep1788 and filib++: pow(x, [d])")
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
NOTE_ROOT = "libieeep1788, filib++ and Solaris Studio have no rootn; pown_rev([−8, 27], 3) of libieeep1788 is [−2, 3]"
case(op("nth_root", iv(-8, 27), 3), X(-2, 3), NOTE_ROOT)
case(op("nth_root", iv(-4, 9), 2), X(0, 3))
case(op("nth_root", iv(-4, -1), 2), EMPTYSET)
case(op("nth_root", iv(-8, -1), 3), X(-2, -1))
case(op("nth_root", iv(0), 5), X(0))
case(op("nth_root", ENTIRE, 3), X("-inf", "inf"))
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
case(op("wid", iv(-INF, 1)), XR(INF), "wid (12.12.8), diam in filib++", kind="R")
case(op("wid", iv(-MAX, MAX)), XR(INF), kind="R")
case(op("wid", EMPTY), XR(NAN), kind="R")
case(op("mag", iv(-3, 2)), XR(3.0), kind="R")
case(op("mag", EMPTY), XR(NAN), kind="R")
case(op("mig", iv(-3, 2)), XR(0.0), kind="R")
case(op("mig", iv(-3, -2)), XR(2.0), kind="R")
case(op("mig", EMPTY), XR(NAN), kind="R")
case(op("hull", I12, EMPTY), X(1, 2), "convex_hull in libieeep1788, hull in filib++, .ih. in Solaris Studio")
case(op("inter", I12, iv(3, 4)), EMPTYSET, "intersection in libieeep1788, intersect in filib++, .ix. in Solaris Studio")
case(op("inter", I12, iv(2, 3)), X(2))
case(op("min", iv(-INF, 1), iv(0, 2)), X("-inf", 1), "imin and imax in filib++")
case(op("max", I12, EMPTY), EMPTYSET)
case(op("neg", iv(-INF, 1)), X(-1, "inf"))
case(op("rad", iv(1, 2)), XR(0.5), "rad (12.12.8): the smallest r with x in [m − r, m + r]; Solaris Studio has none",
     kind="R")
case(op("rad", iv(1, 1 + 3 * 2.0**-52)), XR(2 * 2.0**-52), "m = 1 + 2^-51, the tie rounded to even", kind="R")
case(op("rad", iv(0, TINY)), XR(TINY), kind="R")
case(op("rad", iv(-MAX, MAX)), XR(MAX), kind="R")
case(op("rad", iv(-INF, 1)), XR(INF), kind="R")
case(op("rad", EMPTY), XR(NAN), kind="R")

group("Interval literals of the set-based flavor", "tests/numbers.cpp (ieee_literals)")
case(text("[ ]"), EMPTYSET, "12.11.3")
case(text("[Empty]"), EMPTYSET, "the case of the letters is ignored (9.7.1)")
case(text("[,]"), X("-inf", "inf"), "bounds left out are infinite")
case(text("[1,]"), X(1, "inf"))
case(text("[-Inf, 2/3]"), X("-inf", "mpf(2)/3"))
case(text("[0x1.3p-1, 2/3]"), X("mpf(19)/32", "mpf(2)/3"), "hexadecimal number (9.7.2)")
case(text("[0x1.00000000000001p0]"), X("1 + mpf(2)**-56"))
case(text("-10??u"), X(-10, "inf"), "uncertain form with an infinite radius")
case(text("-10?12"), X(-22, 2))
case(text("[inf]"), EMPTYSET, "not a literal: numsToInterval(+∞, +∞) has no value")
case(text("[inf, inf]"), EMPTYSET)

group("Comparisons (Tables 10.3 and 10.4)", "tests/other_functions.cpp (comparisons)")
NOTE_REL = ("GAOL: certainly_leq, certainly_le, set_strictly_contains, set_contains, set_eq, set_disjoint; "
            "filib++: cle, clt, interior, subset, seq, disjoint; Solaris Studio: .cle., .clt., .int., .sb., .seq., .dj.")
case(op("precedes", I12, iv(2, 3)), XB(True), NOTE_REL, kind="B")
case(op("precedes", I12, EMPTY), XB(True), "true when either interval is empty (Table 10.4)", kind="B")
case(op("precedes", EMPTY, I12), XB(True), kind="B")
case(op("strict_precedes", I12, iv(2, 3)), XB(False), kind="B")
case(op("strict_precedes", I12, EMPTY), XB(True), kind="B")
case(op("interior", iv(1.5), I12), XB(True), kind="B")
case(op("interior", iv(1, 2), iv(1, 3)), XB(False), kind="B")
case(op("interior", ENTIRE, ENTIRE), XB(True), "−∞ <0 −∞ and +∞ <0 +∞ (Table 10.3)", kind="B")
case(op("interior", iv(2, INF), iv(1, INF)), XB(True), kind="B")
case(op("interior", EMPTY, EMPTY), XB(True), kind="B")
case(op("subset", EMPTY, I12), XB(True), kind="B")
case(op("subset", I12, EMPTY), XB(False), kind="B")
case(op("equal", EMPTY, EMPTY), XB(True), kind="B")
case(op("equal", iv(1, INF), iv(1, INF)), XB(True), kind="B")
case(op("disjoint", I12, EMPTY), XB(True), kind="B")
case(op("disjoint", I12, iv(2, 3)), XB(False), kind="B")

# After the other groups, whose cases keep their numbers
group("atan2(y, x), defined on the plane but (0, 0), with values in (−π, π] (Table 9.1)",
      "tests/elementary.cpp (atan2_of_boxes)")
NOTE_CUT = "points on the half-line y = 0, x < 0, where atan2 is π, and points below it, where it is next to −π"
case(op("atan2", iv(0), iv(0)), EMPTYSET, "atan2(0, 0) has no value; filib++ has no atan2")
case(op("atan2", EMPTY, iv(1)), EMPTYSET)
case(op("atan2", iv(1), EMPTY), EMPTYSET)
case(op("atan2", iv(1), iv(1)), X("pi/4"))
case(op("atan2", iv(1, 2), iv(1, 2)), X("atan2(1, 2)", "atan2(2, 1)"))
case(op("atan2", iv(1, 2), iv(-2, -1)), X("atan2(2, -1)", "atan2(1, -2)"))
case(op("atan2", iv(-2, -1), iv(-2, -1)), X("atan2(-1, -2)", "atan2(-2, -1)"))
case(op("atan2", iv(-2, -1), iv(1, 2)), X("atan2(-2, 1)", "atan2(-1, 2)"))
case(op("atan2", iv(1, 2), iv(-1, 1)), X("pi/4", "3*pi/4"))
case(op("atan2", iv(-1, 1), iv(1, 2)), X("-pi/4", "pi/4"))
case(op("atan2", iv(0), iv(1, 2)), X(0))
case(op("atan2", iv(0), iv(-2, -1)), X("pi"))
case(op("atan2", iv(0), iv(-1, 1)), X(0, "pi"))
case(op("atan2", iv(0), iv(-1, 0)), X("pi"))
case(op("atan2", iv(1, 2), iv(0)), X("pi/2"))
case(op("atan2", iv(-2, -1), iv(0)), X("-pi/2"))
case(op("atan2", iv(-1, 1), iv(0)), X("-pi/2", "pi/2"))
case(op("atan2", iv(0, 1), iv(0)), X("pi/2"))
case(op("atan2", iv(0, 1), iv(-2, -1)), X("atan2(1, -1)", "pi"))
case(op("atan2", iv(-1, 0), iv(-2, -1)), X("-pi", "pi"), NOTE_CUT)
case(op("atan2", iv(-1, 1), iv(-2, -1)), X("-pi", "pi"), NOTE_CUT)
case(op("atan2", iv(-1, 0), iv(1, 2)), X("-pi/4", 0))
case(op("atan2", iv(-1, 1), iv(-1, 1)), X("-pi", "pi"))
case(op("atan2", ENTIRE, ENTIRE), X("-pi", "pi"))
case(op("atan2", iv(1, INF), iv(1, INF)), X(0, "pi/2"))
case(op("atan2", iv(1, INF), iv(-INF, -1)), X("pi/2", "pi"))
case(op("atan2", iv(-INF, -1), iv(-INF, -1)), X("-pi", "-pi/2"))
case(op("atan2", ENTIRE, iv(1, 2)), X("-pi/2", "pi/2"))
case(op("atan2", iv(1, 2), ENTIRE), X(0, "pi"))


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

template<class F>
static void show_bool(const char *id, F f)
{
  try {
    std::printf("%s|B|%d\n", id, f() ? 1 : 0);
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

template<class F>
static void show_bool(const char *id, F f)
{
  try {
    std::printf("%s|B|%d\n", id, f() ? 1 : 0);
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

FILIB_HEAD = r'''// Generated by doc/compare/code/cases.py: the special cases, computed by filib++
#include <interval/interval.hpp>
#include <cstdio>
#include <exception>
#include <limits>
#include <sstream>
#include <string>

// The intervals IBEX computes with when it is built with filib++
typedef filib::interval<double, filib::native_switched, filib::i_mode_extended_flag> FI;

static const double pinf = std::numeric_limits<double>::infinity();
static const double qnan = std::numeric_limits<double>::quiet_NaN();
static const double dmax = std::numeric_limits<double>::max();
static const double dtiny = std::numeric_limits<double>::denorm_min();

// An interval read by operator>>, which reads [l, u] and throws otherwise
static FI read_interval(const char *s)
{
  std::istringstream in(s);
  FI x;
  in >> x;
  return x;
}

template<class F>
static void show(const char *id, F f)
{
  try {
    const FI r = f();
    if (r.isEmpty()) {
      std::printf("%s|E\n", id);
    } else {
      std::printf("%s|I|%a|%a\n", id, r.inf(), r.sup());
    }
  } catch (filib::interval_io_exception&) {
    std::printf("%s|X|exception interval_io_exception\n", id);
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

template<class F>
static void show_bool(const char *id, F f)
{
  try {
    std::printf("%s|B|%d\n", id, f() ? 1 : 0);
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

int main()
{
  filib::fp_traits<double, filib::native_switched>::setup();
'''

FILIB_TAIL = r'''  return 0;
}
'''

PROFIL_HEAD = r'''// Generated by doc/compare/code/cases.py: the special cases, computed by PROFIL/BIAS
#include <Interval.h>
#include <Functions.h>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

// BIAS reports an error (a division by an interval holding 0, a function
// outside its domain) on the standard error and calls abort(): SIGABRT is
// caught, and the case shown as an error
static sigjmp_buf bias_abort;
static void on_abort(int) { siglongjmp(bias_abort, 1); }

static const double pinf = std::numeric_limits<double>::infinity();
static const double qnan = std::numeric_limits<double>::quiet_NaN();
static const double dmax = std::numeric_limits<double>::max();
static const double dtiny = std::numeric_limits<double>::denorm_min();

// PROFIL/BIAS has no empty set: an intersection that is empty, which
// Intersection() reports, is shown as one
struct empty_set {};

static INTERVAL intersect(const INTERVAL& x, const INTERVAL& y)
{
  INTERVAL r;
  if (!Intersection(r, x, y)) {
    throw empty_set();
  }
  return r;
}

// PROFIL/BIAS reads an interval as two numbers, "l u", with the >> of doubles
// (rounded to nearest), and has no literal: [x], [l, u], l or "l u" are read
// here with strtod, rounded to nearest as well; anything else is an error
static INTERVAL read_interval(const char *s)
{
  std::string t(s);
  for (char& c : t) {
    if (c == '[' || c == ']' || c == ',') {
      c = ' ';
    }
  }
  double v[2];
  int n = 0;
  const char *p = t.c_str();
  while (*p != '\0') {
    while (*p == ' ') {
      ++p;
    }
    if (*p == '\0') {
      break;
    }
    char *end;
    v[n < 2 ? n : 1] = std::strtod(p, &end);
    if (end == p || n == 2) {
      throw std::invalid_argument(s);
    }
    ++n;
    p = end;
  }
  if (n == 0) {
    throw std::invalid_argument(s);
  }
  return (n == 1) ? INTERVAL(v[0]) : INTERVAL(v[0], v[1]);
}

// The comparisons of IEEE 1788-2015 PROFIL/BIAS has no operator for
static bool precedes(const INTERVAL& x, const INTERVAL& y) { return Sup(x) <= Inf(y); }
static bool strict_precedes(const INTERVAL& x, const INTERVAL& y) { return Sup(x) < Inf(y); }
static bool interior(const INTERVAL& x, const INTERVAL& y) { return x < y; }
static bool subset(const INTERVAL& x, const INTERVAL& y) { return x <= y; }
static bool equal(const INTERVAL& x, const INTERVAL& y) { return x == y; }
static bool disjoint(const INTERVAL& x, const INTERVAL& y) { return Sup(x) < Inf(y) || Sup(y) < Inf(x); }

template<class F>
static void show(const char *id, F f)
{
  if (sigsetjmp(bias_abort, 1) != 0) {
    std::printf("%s|X|BIAS error, abort\n", id);
    return;
  }
  try {
    const INTERVAL r = f();
    std::printf("%s|I|%a|%a\n", id, Inf(r), Sup(r));
  } catch (empty_set&) {
    std::printf("%s|E\n", id);
  } catch (std::invalid_argument&) {
    std::printf("%s|X|not read\n", id);
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

template<class F>
static void show_real(const char *id, F f)
{
  if (sigsetjmp(bias_abort, 1) != 0) {
    std::printf("%s|X|BIAS error, abort\n", id);
    return;
  }
  try {
    std::printf("%s|R|%a\n", id, f());
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

template<class F>
static void show_bool(const char *id, F f)
{
  if (sigsetjmp(bias_abort, 1) != 0) {
    std::printf("%s|X|BIAS error, abort\n", id);
    return;
  }
  try {
    std::printf("%s|B|%d\n", id, f() ? 1 : 0);
  } catch (...) {
    std::printf("%s|X|exception\n", id);
  }
}

int main()
{
  std::setvbuf(stdout, NULL, _IOLBF, 0);
  std::signal(SIGABRT, on_abort);
'''

PROFIL_TAIL = r'''  return 0;
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

  subroutine showb(id, t)
    character(len=*), intent(in) :: id
    logical, intent(in) :: t
    if (t) then
      write(*, '(A,A)') id, '|B|1'
    else
      write(*, '(A,A)') id, '|B|0'
    end if
  end subroutine showb

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
  logical :: t
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
                                  ("p1788", P1788_HEAD, P1788_TAIL, "cases_p1788.cpp"),
                                  ("filib", FILIB_HEAD, FILIB_TAIL, "cases_filib.cpp"),
                                  ("profil", PROFIL_HEAD, PROFIL_TAIL, "cases_profil.cpp")):
        lines = [head]
        for c in CASES:
            lines.append(f"  // {c.name}\n")
            try:
                body = cpp_body(c.node, lib)
            except NotAvailable:
                lines.append(f'  std::printf("{c.id}|NA\\n");\n')
                continue
            result = {"R": "double", "B": "bool"}.get(c.kind, CPP_TYPE[lib])
            show = {"R": "show_real", "B": "show_bool"}.get(c.kind, "show")
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
            elif kind == "B":
                results[cid] = ("B", int(parts[2]))
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
    if r[0] == "B":
        return "true" if r[1] else "false"
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
    if e[0] == "B":
        return "=" if r[0] == "B" and r[1] == e[1] else "✗"
    if r[0] != "I":
        return "✗"
    if r[1] == e[1] and r[2] == e[2]:
        return "="
    if r[1] <= e[1] and r[2] >= e[2]:
        return "⊃"
    return "✗"


BEGIN = "<!-- BEGIN GENERATED TABLES (doc/compare/code/cases.py) -->"
END = "<!-- END GENERATED TABLES -->"


LIBRARIES = [("p1788", "libieeep1788"), ("gaol", "GAOL"), ("filib", "filib++"), ("sun", "Solaris Studio"),
             ("profil", "PROFIL/BIAS")]


def report(directory, out):
    import os
    names = [name for _, name in LIBRARIES]
    results = [read_results(os.path.join(directory, key + ".txt")) for key, _ in LIBRARIES]
    counts = [{"=": 0, "⊃": 0, "✗": 0, "n/a": 0} for _ in names]
    lines = []
    for g, (title, source) in enumerate(GROUPS):
        lines.append(f"### {g + 1}. {title}\n\n")
        lines.append(f"From {source}.\n\n")
        lines.append("| # | Operation | IEEE 1788 | " + " | ".join(names) + " | Notes |\n")
        lines.append("|---" * (len(names) + 4) + "|\n")
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
    elif len(sys.argv) == 4 and sys.argv[1] == "report":
        report(sys.argv[2], sys.argv[3])
    else:
        sys.exit(__doc__)
