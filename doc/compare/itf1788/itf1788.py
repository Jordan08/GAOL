#!/usr/bin/env python3
"""ITF1788, the test suite of the operations of IEEE 1788-2015, on GAOL v5.

ITF1788 (https://github.com/oheim/ITF1788, by Oliver Heimlich, Marco Nehmeier
and Maximilian Kiesner) writes its test cases once, in the .itl files of its
directory itl/, one case per line,

    add [1.0, 2.0] [3.0, 4.0] = [4.0, 6.0];

and converts them with plugins of its own into test programs for several
libraries. This script is the back end of GAOL v5. It reads the .itl files
itself rather than through ITF1788's converter, which needs PLY and
Boost.Test, and whose plugin for GAOL calls GAOL 4 by GAOL's own names: it
writes a C++ program per .itl file, which calls each operation by the name of
the standard that gaol/gaol_ieee1788.h gives it, and prints a line per case
(see itf1788_gaol.h); the report counts the lines and classifies the
failures.

    itf1788.py generate ITL_DIR OUT_DIR [--not-provided OP,OP...]
        writes OUT_DIR/<name>.cpp for each ITL_DIR/<name>.itl; the operations
        given after --not-provided are counted as not provided rather than run
    itf1788.py report REPORT.md OUT_DIR [--gaol LABEL] [--itf1788 COMMIT] BUILD=DIR...
        reads DIR/*.txt, what the programs of OUT_DIR printed with each build
        of GAOL v5 (sse=..., fpu=...), checks that each program went through
        all its cases, classifies the failures, and writes the tables between
        the markers of REPORT.md (the whole file when it has none); LABEL and
        COMMIT name the versions of GAOL v5 (the last commit of its sources) and
        of ITF1788 in the report

The literals of the cases are taken as ITF1788's C++ plugins take them: a
number is the double nearest to it (a C++ double literal), so that
[-2.0, -0.1] is the interval whose lower bound is the double nearest to -0.1,
and the intervals are built with the numeric constructor, interval(l, u), from
these doubles, which this script computes (float.fromhex, float) and writes as
decimal literals that convert back to the same doubles. The strings of
b-textToInterval are read by gaol_ieee1788::textToInterval, the reader of the
standard's literals.

Copyright (c) 2026 ENSTA, France

Created 2026-10-06 by Jordan NININ
"""

import math
import os
import re
import struct
import sys
from fractions import Fraction

# ---------------------------------------------------------------------------
# The operations

# The operations GAOL v5 provides, by their name in the .itl files: the C++
# expression of the call, {0}, {1}... being the inputs, and the kind of each
# output: I an interval, D a number, B a boolean
OPS = {}


def op(names, call, outputs="I"):
    """Each operation of names is called as call, @ standing for its name"""
    for name in names.split():
        OPS[name] = (call.replace("@", name), outputs)


# Forward elementary functions (Table 9.1). pos(x), which libieeep1788 has and
# IEEE 1788-2015 does not, is GAOL's unary +.
op("pos", "+{0}")
op("neg recip sqr sqrt exp exp2 exp10 log log2 log10 sin cos tan asin acos atan "
   "sinh cosh tanh asinh acosh atanh sign ceil floor trunc roundTiesToEven "
   "roundTiesToAway abs", "@({0})")
op("add sub mul div atan2 min max pow", "@({0}, {1})")
op("fma", "fma({0}, {1}, {2})")
op("pown rootn", "@({0}, {1})")
# Recommended forward functions (Table 10.5); cbrt, which ITF1788 takes from
# MPFI, is rootn(x, 3)
op("expm1 exp2m1 exp10m1 logp1 log2p1 log10p1 rSqrt sinPi cosPi tanPi asinPi acosPi atanPi", "@({0})")
op("hypot atan2Pi", "@({0}, {1})")
op("cbrt", "rootn({0}, 3)")
# Reverse functions (Table 10.1), which ITF1788 calls absRevBin... with their
# optional argument x
op("absRev sqrRev sinRev cosRev tanRev coshRev", "@({0})")
for _name in "absRev sqrRev sinRev cosRev tanRev coshRev".split():
    op(_name + "Bin", _name + "({0}, {1})")
op("pownRev", "pownRev({0}, {1})")
op("pownRevBin", "pownRev({0}, {1}, {2})")
op("mulRev", "mulRev({0}, {1})")
op("mulRevTen", "mulRev({0}, {1}, {2})")  # mulRev(b, c, x)
# Two-output division (10.5.5)
op("mulRevToPair", "mulRevToPair({0}, {1})", "II")
# Cancellative addition and subtraction (10.5.6), set operations (10.5.7)
op("cancelMinus cancelPlus intersection convexHull", "@({0}, {1})")
# Constructors (10.5.8, 12.12.7)
op("b-numsToInterval", "numsToInterval({0}, {1})")
op("b-textToInterval", "textToInterval({0})")
# Numeric functions (Table 10.2)
op("inf sup mid wid rad mag mig", "@({0})", "D")
op("midRad", "midRad({0})", "DD")
# Boolean functions (Table 10.3, 10.6.3)
op("isEmpty isEntire isCommonInterval isSingleton", "@({0})", "B")
op("equal subset less strictLess precedes strictPrecedes interior disjoint isMember", "@({0}, {1})", "B")

# The operations GAOL v5 does not provide, and why (gaol/gaol_ieee1788.h,
# "Not provided")
NOT_IN_STANDARD = "not an operation of IEEE 1788-2015, which GAOL v5 does not provide"
NOT_PROVIDED = {
    "powRev1": "powRev1 (Table 10.1): GAOL v5 has no reverse of pow(x, y)",
    "powRev2": "powRev2 (Table 10.1): GAOL v5 has no reverse of pow(x, y)",
    "atan2Rev1": "atan2Rev1 (Table 10.1): GAOL v5 has no reverse of atan2",
    "atan2Rev2": "atan2Rev2 (Table 10.1): GAOL v5 has no reverse of atan2",
    "compoundm1": "compoundm1 (Table 10.5): CORE-MATH has none",
    "overlap": "overlap (10.6.4): GAOL v5 has no interval overlapping",
}
for _name in "sum dot sum_abs sum_sqr".split():
    for _mode in "nearest up down zero".split():
        NOT_PROVIDED[_name + "_" + _mode] = "the reduction operations (12.12.12): GAOL v5 has none"
for _name in "cot coth acot acoth sec sech csc csch".split():
    NOT_PROVIDED[_name] = _name + ": " + NOT_IN_STANDARD

# The operations of the decorated intervals (Clause 11, 12.12.11): GAOL v5 has
# none, and no NaI
DECORATION_OPS = {"intervalPart", "decorationPart", "newDec", "setDec", "isNaI"}

# The message gaol_ieee1788::pownRev throws for p <= 0, which GAOL v5 does not
# provide
POWNREV_NOT_PROVIDED = "not provided by GAOL v5"


# ---------------------------------------------------------------------------
# Reading the .itl files

TOKEN = re.compile(r"""
    (?P<space>\s+)
  | (?P<comment>/\*.*?\*/ | //[^\n]* | \#\*.*?\*\# | \#[^\n]*)
  | (?P<string>"(?:[^"\\\n]|\\.)*")
  | (?P<number>[+-]?(?:0[xX](?:[0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.?)[pP][+-]?[0-9]+
                    | (?:[0-9]*\.[0-9]+|[0-9]+\.)(?:[eE][+-]?[0-9]+)?
                    | [0-9]+[eE][+-]?[0-9]+
                    | 0[xX][0-9a-fA-F]+[uUlL]*
                    | [0-9]+[uUlL]*))
  | (?P<signedinf>[+-]infinity)
  | (?P<word>[a-zA-Z][a-zA-Z0-9_-]*)
  | (?P<punct>[{};=<\[\],_.])
""", re.S | re.X)

DECORATIONS = {"com", "dac", "def", "trv", "ill"}
OVERLAPS = {"bothEmpty", "firstEmpty", "secondEmpty", "before", "meets", "overlaps", "starts",
            "containedBy", "finishes", "equals", "finishedBy", "contains", "startedBy",
            "overlappedBy", "metBy", "after"}
EXCEPTIONS = {"UndefinedOperation", "PossiblyUndefinedOperation", "InvalidOperand",
              "IntvlPartOfNaI", "IntvlOverflow"}


class ItlError(Exception):
    pass


def tokens(text, path):
    """The tokens of an .itl file: (kind, text, line), comments left out"""
    pos, line = 0, 1
    while pos < len(text):
        m = TOKEN.match(text, pos)
        if not m:
            raise ItlError("%s:%d: cannot read %r" % (path, line, text[pos:pos + 20]))
        kind, value = m.lastgroup, m.group()
        if kind not in ("space", "comment"):
            yield kind, value, line
        line += value.count("\n")
        pos = m.end()


def number(text):
    """The double nearest to a number literal, as a C++ double literal is"""
    t = text.rstrip("uUlL")
    if re.match(r"[+-]?0[xX]", t):
        if "p" in t.lower():
            return float.fromhex(t)
        return float(int(t, 16))
    return float(t)


class Literal:
    """A literal of a case: kind is interval, empty, entire, nai, number, int,
    nan, string, bool, overlap, decoration or array"""

    def __init__(self, kind, value=None, dec=None):
        self.kind, self.value, self.dec = kind, value, dec

    def is_interval(self):
        return self.kind in ("interval", "empty", "entire", "nai")


class Case:
    def __init__(self, file, testcase, line, op, inputs, outputs, accurate, signal, source):
        self.file, self.testcase, self.line, self.op = file, testcase, line, op
        self.inputs, self.outputs, self.accurate, self.signal = inputs, outputs, accurate, signal
        self.source = source


class Parser:
    def __init__(self, path):
        self.path = path
        self.file = os.path.basename(path)
        with open(path, encoding="utf-8") as f:
            self.text = f.read()
        self.lines = self.text.split("\n")
        self.toks = list(tokens(self.text, path))
        self.i = 0

    def peek(self, k=0):
        return self.toks[self.i + k] if self.i + k < len(self.toks) else (None, None, None)

    def next(self):
        t = self.peek()
        if t[0] is None:
            raise ItlError("%s: unexpected end of the file" % self.path)
        self.i += 1
        return t

    def expect(self, value):
        kind, text, line = self.next()
        if text != value:
            raise ItlError("%s:%d: %r expected, %r found" % (self.path, line, value, text))
        return line

    def cases(self):
        while self.peek()[0] is not None:
            kind, text, line = self.next()
            if text != "testcase":
                raise ItlError("%s:%d: testcase expected, %r found" % (self.path, line, text))
            name = self.next()[1]
            while self.peek()[1] == ".":
                # a qualified name: minimal.powRev1_test, IEEE1788.a
                self.next()
                name += "." + self.next()[1]
            self.expect("{")
            while self.peek()[1] != "}":
                yield self.case(name)
            self.expect("}")

    def case(self, testcase):
        kind, op, line = self.next()
        if kind != "word":
            raise ItlError("%s:%d: an operation expected, %r found" % (self.path, line, op))
        inputs, outputs, accurate, signal = [], None, None, None
        while self.peek()[1] not in ("=", "<", ";", "signal"):
            inputs.append(self.literal())
        if self.peek()[1] == "=":
            self.next()
            outputs = []
            while self.peek()[1] not in ("<", ";", "signal"):
                outputs.append(self.literal())
        if self.peek()[1] == "<":
            self.next()
            self.expect("=")
            accurate = []
            while self.peek()[1] not in (";", "signal"):
                accurate.append(self.literal())
        if self.peek()[1] == "signal":
            self.next()
            signal = self.next()[1]
            if signal not in EXCEPTIONS:
                raise ItlError("%s:%d: unknown exception %r" % (self.path, line, signal))
        end = self.expect(";")
        source = " ".join(l.strip() for l in self.lines[line - 1:end])
        source = re.sub(r"\s+", " ", source)
        return Case(self.file, testcase, line, op, inputs, outputs, accurate, signal, source)

    def literal(self):
        kind, text, line = self.next()
        if text == "[":
            k2, t2, _ = self.next()
            if t2 in ("empty", "entire", "nai"):
                self.expect("]")
                lit = Literal(t2)
            else:
                lo = self.bound(k2, t2, line)
                self.expect(",")
                k3, t3, _ = self.next()
                hi = self.bound(k3, t3, line)
                self.expect("]")
                lit = Literal("interval", (lo, hi))
            if self.peek()[1] == "_":
                self.next()
                lit.dec = self.next()[1]
                if lit.dec not in DECORATIONS:
                    raise ItlError("%s:%d: unknown decoration %r" % (self.path, line, lit.dec))
            return lit
        if text == "{":
            values = []
            while self.peek()[1] != "}":
                k2, t2, _ = self.next()
                values.append(self.bound(k2, t2, line))
                if self.peek()[1] == ",":
                    self.next()
            self.next()
            return Literal("array", values)
        if kind == "string":
            return Literal("string", text)
        if kind == "signedinf":
            return Literal("number", math.inf if text[0] == "+" else -math.inf)
        if kind == "number":
            if re.fullmatch(r"[+-]?(0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*", text):
                return Literal("int", int(text.rstrip("uUlL"), 0))
            return Literal("number", number(text))
        if kind == "word":
            if text == "infinity":
                return Literal("number", math.inf)
            if text == "NaN":
                return Literal("nan", math.nan)
            if text in ("true", "false"):
                return Literal("bool", text == "true")
            if text in OVERLAPS:
                return Literal("overlap", text)
            if text in DECORATIONS:
                return Literal("decoration", text)
        raise ItlError("%s:%d: cannot read the literal %r" % (self.path, line, text))

    def bound(self, kind, text, line):
        if kind == "signedinf":
            return math.inf if text[0] == "+" else -math.inf
        if kind == "word" and text == "infinity":
            return math.inf
        if kind == "word" and text == "NaN":
            return math.nan
        if kind == "number":
            return number(text)
        raise ItlError("%s:%d: a number expected, %r found" % (self.path, line, text))


# ---------------------------------------------------------------------------
# Writing the C++ programs

def cpp_double(x):
    """A C++ literal of the double x, which converts back to x"""
    if math.isnan(x):
        return "itf::nan"
    if math.isinf(x):
        return "itf::inf" if x > 0 else "-itf::inf"
    r = repr(x)
    if not any(c in r for c in ".en"):
        r += ".0"
    return r


def cpp_string(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def cpp_literal(lit, kind=None):
    """The C++ expression of a literal; kind D asks for a double"""
    if lit.kind == "interval":
        return "itf::iv(%s, %s)" % (cpp_double(lit.value[0]), cpp_double(lit.value[1]))
    if lit.kind == "empty":
        return "interval::emptyset()"
    if lit.kind == "entire":
        return "interval::universe()"
    if lit.kind in ("number", "nan"):
        return cpp_double(lit.value)
    if lit.kind == "int":
        if kind == "D":
            return cpp_double(float(lit.value))
        if -2**31 <= lit.value < 2**31:
            return str(lit.value)
        return str(lit.value) + "LL"
    if lit.kind == "bool":
        return "true" if lit.value else "false"
    if lit.kind == "string":
        # the .itl string, a C string literal already
        return "std::string(%s)" % lit.value
    raise ValueError(lit.kind)


def classify(case, not_provided):
    """(status, why) for a case that is not run, None for one that is"""
    lits = case.inputs + (case.outputs or []) + (case.accurate or [])
    if case.op.startswith("d-") or case.op in DECORATION_OPS:
        return "notapplicable", "an operation of the decorated intervals (Clause 11): GAOL v5 has no decorations"
    if any(l.kind == "nai" for l in lits):
        return "notapplicable", "NaI, the decorated ill-formed interval (Clause 11): GAOL v5 has no decorations"
    if any(l.dec is not None for l in lits):
        return "notapplicable", "a decorated interval (Clause 11): GAOL v5 has no decorations"
    if case.op in not_provided:
        return "notprovided", not_provided[case.op]
    if case.op not in OPS:
        return "notprovided", "an operation this generator does not know"
    return None


def midpoint_rounds_to_zero(case):
    """Whether the exact midpoint of the input of mid or midRad is not 0 but
    the expected result is: the sign of its zero is then not checked"""
    x, out = case.inputs[0], case.outputs[0]
    if x.kind != "interval" or out.kind not in ("number", "int") or out.value != 0:
        return False
    lo, hi = x.value
    return math.isfinite(lo) and math.isfinite(hi) and Fraction(lo) + Fraction(hi) != 0


def cpp_case(case, not_provided):
    """The C++ statements of a case"""
    signal = case.signal or "-"
    head = "%d, %s, %s, %s" % (case.line, cpp_string(case.op), cpp_string(signal), cpp_string(case.source))
    skipped = classify(case, not_provided)
    if skipped:
        return ["r.skip(%s, %s, %s);" % (head, cpp_string(skipped[0]), cpp_string(skipped[1]))]
    call, kinds = OPS[case.op]
    args = [cpp_literal(l) for l in case.inputs]
    expr = call.format(*args)
    outs = case.outputs
    if case.accurate is not None or outs is None:
        # "= tightest <= accurate", which the grammar of ITF1788 allows and no
        # case of it uses
        raise ItlError("%s:%d: a case with an accurate result (<=) or without a result, which this "
                       "generator does not read" % (case.file, case.line))
    if len(outs) != len(kinds):
        raise ItlError("%s:%d: %d outputs, %d expected for %s" % (case.file, case.line, len(outs), len(kinds), case.op))
    for lit, k in zip(outs, kinds):
        ok = {"I": lit.is_interval(), "D": lit.kind in ("number", "nan", "int"), "B": lit.kind == "bool"}[k]
        if not ok:
            raise ItlError("%s:%d: output %s where %s is expected" % (case.file, case.line, lit.kind, k))
    exp = [cpp_literal(l, k) for l, k in zip(outs, kinds)]
    # The midpoint whose exact value is not 0 but rounds to it may be -0 or +0
    # (see itf1788_gaol.h)
    eq_mid = "r.eq_any_zero" if case.op in ("mid", "midRad") and midpoint_rounds_to_zero(case) else "r.eq"
    if kinds == "I":
        body = "r.eq(interval(%s), %s);" % (expr, exp[0])
    elif kinds == "D":
        body = "%s(double(%s), %s);" % (eq_mid, expr, exp[0])
    elif kinds == "B":
        body = "r.eq(bool(%s), %s);" % (expr, exp[0])
    elif kinds == "II":
        body = ("const std::pair<interval, interval> p = %s; r.eq(p.first, interval(%s)); "
                "r.eq(p.second, interval(%s));" % (expr, exp[0], exp[1]))
    elif kinds == "DD":
        body = ("double m = 0.0, d = 0.0; midRad(%s, m, d); %s(m, %s); r.eq(d, %s);"
                % (args[0], eq_mid, exp[0], exp[1]))
    else:
        raise ValueError(kinds)
    return ["r.begin(%s);" % head,
            "try { %s } catch (...) { r.threw(); }" % body,
            "r.end();"]


def generate(itl_dir, out_dir, not_provided):
    os.makedirs(out_dir, exist_ok=True)
    files = sorted(f for f in os.listdir(itl_dir) if f.endswith(".itl"))
    if not files:
        raise ItlError("no .itl file in %s" % itl_dir)
    manifest = []
    for f in files:
        cases = list(Parser(os.path.join(itl_dir, f)).cases())
        name = f[:-4]
        lines = ["// Generated by doc/compare/itf1788/itf1788.py from %s of ITF1788: do not edit" % f,
                 '#include "itf1788_gaol.h"',
                 "",
                 "using namespace gaol_ieee1788;",
                 ""]
        # A function per run of 100 cases at most, so that the compiler does
        # not get functions of thousands of statements
        chunks, current = [], []
        for c in cases:
            if current and (len(current) == 100 or current[-1].testcase != c.testcase):
                chunks.append(current)
                current = []
            current.append(c)
        if current:
            chunks.append(current)
        for n, chunk in enumerate(chunks):
            lines.append("static void cases_%d(itf::recorder& r)" % n)
            lines.append("{")
            lines.append("  r.testcase(%s);" % cpp_string(chunk[0].testcase))
            for c in chunk:
                lines.extend("  " + s for s in cpp_case(c, not_provided))
            lines.append("}")
            lines.append("")
        lines.append("int main()")
        lines.append("{")
        lines.append("  itf::recorder r(%s);" % cpp_string(f))
        for n in range(len(chunks)):
            lines.append("  cases_%d(r);" % n)
        lines.append("  const int status = r.finish();")
        lines.append("  gaol::cleanup();")
        lines.append("  return status;")
        lines.append("}")
        with open(os.path.join(out_dir, name + ".cpp"), "w", encoding="utf-8") as out:
            out.write("\n".join(lines) + "\n")
        manifest.append("%s\t%d\n" % (f, len(cases)))
        print("%s: %d cases" % (f, len(cases)))
    # The number of cases of each file, which the report checks the results against
    with open(os.path.join(out_dir, "cases.txt"), "w", encoding="utf-8") as out:
        out.write("".join(manifest))




# ---------------------------------------------------------------------------
# The report

BEGIN = "<!-- BEGIN GENERATED TABLES (doc/compare/itf1788/itf1788.py) -->"
END = "<!-- END GENERATED TABLES -->"

# The operations IEEE 1788-2015 requires to be tightest (12.10.2: the basic
# operations, the integer and the absmax functions; 12.12.5 for the
# cancellative ones, 10.5.7 and 12.12.7 for the set operations and the
# constructors); the others are to be valid, and should be accurate
TIGHTEST = set("pos neg add sub mul div recip sqr sqrt fma sign ceil floor trunc roundTiesToEven "
               "roundTiesToAway abs min max cancelMinus cancelPlus intersection convexHull "
               "b-numsToInterval b-textToInterval".split())

# The classes of the failures: key, title, verdict
CLASSES = [
    ("unsound", "GAOL v5's result misses part of the exact one (checked with mpmath)", "bug of GAOL v5"),
    ("not-tightest", "Not the tightest result, which IEEE 1788-2015 requires of this operation (12.10.2)",
     "bug of GAOL v5"),
    ("pair-order", "mulRevToPair: the two pieces, or the one piece and the empty set, in the wrong order "
     "(10.5.5: (u, v) with u < v, (u, ∅) for a single piece)", "bug of GAOL v5"),
    ("number", "Another number", "bug of GAOL v5"),
    ("zero-sign", "The sign of a zero result (12.12.8: −0 for inf, +0 for the other numeric functions)",
     "bug of GAOL v5"),
    ("boolean", "The other boolean", "bug of GAOL v5"),
    ("threw", "An exception", "bug of GAOL v5"),
    ("valid", "Valid, not the tightest: IEEE 1788-2015 requires of this operation a valid result, and "
     "recommends an accurate one (12.10.2)", "conforming"),
    ("itf-not-tightest", "ITF1788 expects a result that is not the tightest: GAOL v5's is narrower, and "
     "encloses the exact result (checked with mpmath, whose tightest result the last column gives)",
     "defect of the test case"),
    ("unverified", "GAOL v5's result misses part of the expected one, and no reference was computed",
     "to be examined"),
]


def ordinal(x):
    """The rank of the double x among the doubles, -0 and +0 being the same"""
    b = struct.unpack("<q", struct.pack("<d", x))[0]
    return b if b >= 0 else -(b & 0x7fffffffffffffff)


def double_of(n):
    b = n if n >= 0 else (-n) | -0x8000000000000000
    return struct.unpack("<d", struct.pack("<q", b))[0]


def next_up(x):
    return x if x != x or x == math.inf else double_of(ordinal(x) + 1)


def next_down(x):
    return x if x != x or x == -math.inf else double_of(ordinal(x) - 1)


OUTPUT = re.compile(r"\[empty\]|\[([^,\]]+), ([^\]]+)\]|(\S+)")


def outputs(text):
    """The outputs a program printed: None for the empty set, (l, u) for an
    interval, a float or a bool"""
    out = []
    for m in OUTPUT.finditer(text):
        if m.group() == "[empty]":
            out.append(None)
        elif m.group(1) is not None:
            out.append((float.fromhex(m.group(1)), float.fromhex(m.group(2))))
        elif m.group(3) in ("true", "false"):
            out.append(m.group(3) == "true")
        else:
            out.append(float.fromhex(m.group(3)))
    return out


def contains(a, b):
    """Whether the interval a contains b (None being the empty set)"""
    return b is None or (a is not None and a[0] <= b[0] and b[1] <= a[1])


def distance(a, b):
    """The number of doubles between the bounds of two intervals, the larger"""
    if a is None or b is None:
        return 0
    return max(abs(ordinal(a[0]) - ordinal(b[0])), abs(ordinal(a[1]) - ordinal(b[1])))


def parse_case(source, file="-"):
    """The Case of the text of a case, as the programs print it"""
    p = Parser.__new__(Parser)
    p.path, p.file = file, file
    p.text = "testcase t { %s }" % source
    p.lines = [p.text]
    p.toks = list(tokens(p.text, file))
    p.i = 0
    return next(p.cases())


def interval_of(lit):
    if lit.kind == "empty":
        return None
    if lit.kind == "entire":
        return (-math.inf, math.inf)
    return lit.value


def reverse_trig(fn, c, x):
    """The tightest enclosure of {t in x : fn(t) in c}, fn being sin, cos or
    tan, computed with mpmath at 2000 bits; None for an unbounded x. The
    solution set is a union of closed pieces whose ends are the bounds of x,
    the points where fn takes the value of a bound of c, and, for tan, the
    poles it approaches when c is unbounded: its hull is the least and the
    greatest of these points that are in it."""
    import mpmath
    if c is None or x is None:
        return None, True
    xl, xu = x
    if not (math.isfinite(xl) and math.isfinite(xu)):
        return None, False
    cl, ch = c
    if fn != "tan":
        cl, ch = max(cl, -1.0), min(ch, 1.0)
        if cl > ch:
            return None, True
    with mpmath.workprec(2000):
        pi = mpmath.pi
        f = {"sin": mpmath.sin, "cos": mpmath.cos, "tan": mpmath.tan}[fn]
        XL, XU = mpmath.mpf(xl), mpmath.mpf(xu)
        period = pi if fn == "tan" else 2 * pi
        k0, k1 = int(mpmath.floor(XL / period)) - 2, int(mpmath.ceil(XU / period)) + 2
        if k1 - k0 > 100000:
            return None, False
        bases = []
        for v in (cl, ch):
            if math.isfinite(v):
                V = mpmath.mpf(v)
                if fn == "sin":
                    bases += [mpmath.asin(V), pi - mpmath.asin(V)]
                elif fn == "cos":
                    bases += [mpmath.acos(V), -mpmath.acos(V)]
                else:
                    bases += [mpmath.atan(V)]
        tol = mpmath.mpf(2) ** -1500

        def inside(t):
            if fn == "tan" and abs(mpmath.cos(t)) < tol:
                return False
            y = f(t)
            return (cl == -math.inf or y >= cl - tol) and (ch == math.inf or y <= ch + tol)

        points = [t for t in [XL, XU] + [b + k * period for b in bases for k in range(k0, k1 + 1)]
                  if XL <= t <= XU and inside(t)]
        if fn == "tan":
            for k in range(k0, k1 + 1):
                p = pi / 2 + k * pi
                if XL <= p <= XU and ((ch == math.inf and p > XL) or (cl == -math.inf and p < XU)):
                    points.append(p)
        if not points:
            return None, True
        lo, hi = min(points), max(points)
        dl, du = float(lo), float(hi)
        if mpmath.mpf(dl) > lo:
            dl = next_down(dl)
        if mpmath.mpf(du) < hi:
            du = next_up(du)
        return (dl, du), True


def accurate(case, got, expected):
    """Whether the result got of a case whose tightest result is expected is
    proved accurate in the sense of 12.10.1 of IEEE 1788-2015, (28):
        got ⊆ nextOut(f_tightest(nextOut(x))),
    x being the box of the inputs. A result within one double of the
    tightest is. Beyond, for pow and pown, f_tightest(nextOut(x)) holds the
    tightest result and the values of f at the corners of nextOut(x) that are
    within the domain, finite and nonzero, computed with mpmath: got is
    accurate when it lies within one double outside of their hull. False
    means that the accuracy is not proved, not that it fails."""
    if got is None or expected is None:
        return got == expected
    if distance(got, expected) <= 1:
        return True
    if case.op not in ("pow", "pown"):
        return False
    import mpmath
    x = interval_of(case.inputs[0])
    if x is None:
        return False
    xs = [v for v in (next_down(x[0]), next_up(x[1])) if math.isfinite(v) and v != 0]
    if case.op == "pow":
        y = interval_of(case.inputs[1])
        if y is None:
            return False
        xs = [v for v in xs if v > 0]
        ys = [v for v in (next_down(y[0]), next_up(y[1])) if math.isfinite(v)]
        corners = [(a, b) for a in xs for b in ys]
    else:
        corners = [(a, case.inputs[1].value) for a in xs]
    lo, hi = expected
    with mpmath.workprec(2000):
        for a, b in corners:
            v = mpmath.power(mpmath.mpf(a), b if isinstance(b, int) else mpmath.mpf(b))
            dl, du = float(v), float(v)
            if mpmath.mpf(dl) > v:
                dl = next_down(dl)
            if mpmath.mpf(du) < v:
                du = next_up(du)
            lo, hi = min(lo, dl), max(hi, du)
    return next_down(lo) <= got[0] and got[1] <= next_up(hi)


def reference(case):
    """(the tightest result, True) for the operations whose failures need it,
    computed apart from GAOL; (None, False) where none is computed"""
    fn = {"sinRev": "sin", "cosRev": "cos", "tanRev": "tan"}.get(case.op.replace("Bin", ""))
    if fn is None:
        return None, False
    c = interval_of(case.inputs[0])
    x = interval_of(case.inputs[1]) if case.op.endswith("Bin") else (-math.inf, math.inf)
    return reverse_trig(fn, c, x)


class Row:
    def __init__(self, fields):
        (self.status, self.file, self.testcase, self.line, self.op, self.signal, self.got, self.expected,
         self.source) = fields
        self.line = int(self.line)
        self.category, self.why = None, None

    def key(self):
        return (self.file, self.line)


def read_results(directory, manifest):
    rows, ends = [], {}
    for name in sorted(os.listdir(directory)):
        if not name.endswith(".txt"):
            continue
        with open(os.path.join(directory, name), encoding="utf-8") as f:
            for line in f:
                fields = line.rstrip("\n").split("\t")
                if fields[0] == "end":
                    ends[fields[1]] = int(fields[2])
                else:
                    rows.append(Row(fields))
    for file, n in manifest.items():
        found = sum(1 for r in rows if r.file == file)
        if ends.get(file) != n or found != n:
            raise SystemExit("%s: the program of %s printed %d cases of %d%s" %
                             (directory, file, found, n, "" if file in ends else ", and stopped before its end"))
    return rows


def classify_row(row):
    """Sets row.category (passed, failed, notprovided, notapplicable) and
    row.why (the class of a failure, the reason of a case not run)"""
    if row.status == "pass":
        row.category = "passed"
    elif row.status in ("notprovided", "notapplicable"):
        row.category, row.why = row.status, row.got
    elif row.status == "threw" and POWNREV_NOT_PROVIDED in row.got:
        row.category = "notprovided"
        row.why = "pownRev for p ≤ 0 (Table 10.1): gaol_ieee1788::pownRev throws std::invalid_argument"
    elif row.status == "threw":
        row.category, row.why = "failed", "threw"
    else:
        row.category, row.why = "failed", failure_class(row)
    return row


def failure_class(row):
    got, exp = outputs(row.got), outputs(row.expected)
    row.note = ""
    if any(isinstance(e, bool) for e in exp):
        return "boolean"
    if any(isinstance(e, float) for e in exp):
        same = all((g != g and e != e) or g == e for g, e in zip(got, exp))
        return "zero-sign" if same else "number"
    if row.op == "mulRevToPair" and got[::-1] == exp:
        return "pair-order"
    if all(contains(g, e) for g, e in zip(got, exp)):
        d = max(distance(g, e) for g, e in zip(got, exp))
        row.note = "%d double%s beyond" % (d, "" if d == 1 else "s")
        if row.op in TIGHTEST:
            return "not-tightest"
        if len(got) == 1 and accurate(parse_case(row.source, row.file), got[0], exp[0]):
            row.note += ", accurate"
        return "valid"
    ref, known = reference(parse_case(row.source, row.file))
    if not known:
        return "unverified"
    row.note = "tightest: " + ("[empty]" if ref is None else "[%s, %s]" % (ref[0].hex(), ref[1].hex()))
    if len(got) == 1 and contains(got[0], ref):
        return "itf-not-tightest"
    return "unsound"


def md(text):
    return text.replace("|", "\\|")


BUILD_NAMES = {"sse": "SSE2", "fpu": "FPU"}


def report(out, programs, builds, gaol_label, itf_label):
    manifest = {}
    with open(os.path.join(programs, "cases.txt"), encoding="utf-8") as f:
        for line in f:
            file, n = line.split("\t")
            manifest[file] = int(n)
    results = [(name, [classify_row(r) for r in read_results(d, manifest)]) for name, d in builds]
    base_name, rows = results[0]
    total = len(rows)
    lines = []
    w = lines.append

    # The builds against one another
    differences = []
    by_key = {r.key(): r for r in rows}
    for name, other in results[1:]:
        for r in other:
            b = by_key[r.key()]
            if (r.status, r.got) != (b.status, b.got):
                differences.append((name, b, r))
    shown = [BUILD_NAMES.get(n, n) for n, _ in results]
    names = shown[0] if len(shown) == 1 else ", ".join(shown[:-1]) + " and " + shown[-1]
    w("ITF1788 at commit `%s`: %d cases in %d files, run with GAOL v5 at `%s` (the last commit of its "
      "sources, gaol/ and 3rd/), in its %s build%s" %
      (itf_label, total, len(manifest), gaol_label, names, "s" if len(results) > 1 else ""))
    if len(results) > 1:
        w(", which give the same result in every case.\n\n" if not differences else
          ", which differ in %d cases, listed at the end.\n\n" % len(differences))
    else:
        w(".\n\n")

    count = {}
    for r in rows:
        count[r.category] = count.get(r.category, 0) + 1
    fails = [r for r in rows if r.category == "failed"]
    w("| | Cases |\n|---|---:|\n")
    w("| Passed | %d |\n" % count.get("passed", 0))
    w("| Failed | %d |\n" % len(fails))
    for verdict in ("bug of GAOL v5", "conforming", "defect of the test case", "to be examined"):
        n = sum(1 for r in fails if dict((k, v) for k, _, v in CLASSES)[r.why] == verdict)
        w("| … %s | %d |\n" % (verdict, n))
    w("| Not provided by GAOL v5 | %d |\n" % count.get("notprovided", 0))
    w("| Not applicable: decorations | %d |\n" % count.get("notapplicable", 0))
    w("| Total | %d |\n\n" % total)

    # Per operation, in the order of the files
    w("### Per operation\n\n")
    w("| Operation (.itl) | GAOL v5 | Cases | Passed | Failed | Not provided | Not applicable |\n")
    w("|---|---|---:|---:|---:|---:|---:|\n")
    ops = []
    for r in rows:
        if r.op not in ops:
            ops.append(r.op)
    for op_name in sorted(ops, key=lambda o: o.lower()):
        rs = [r for r in rows if r.op == op_name]
        c = {k: sum(1 for r in rs if r.category == k) for k in ("passed", "failed", "notprovided", "notapplicable")}
        if op_name in OPS and c["passed"] + c["failed"] > 0:
            call = OPS[op_name][0].format("a", "b", "c")
            call = "`%s`" % md(call)
        else:
            call = "—"
        w("| `%s` | %s | %d | %d | %d | %d | %d |\n" % (op_name, call, len(rs), c["passed"], c["failed"],
                                                       c["notprovided"], c["notapplicable"]))
    w("| Total | | %d | %d | %d | %d | %d |\n\n" % (total, count.get("passed", 0), len(fails),
                                                    count.get("notprovided", 0), count.get("notapplicable", 0)))

    # Per file
    w("### Per file\n\n")
    w("| File | Cases | Passed | Failed | Not provided | Not applicable |\n|---|---:|---:|---:|---:|---:|\n")
    for file in manifest:
        rs = [r for r in rows if r.file == file]
        c = {k: sum(1 for r in rs if r.category == k) for k in ("passed", "failed", "notprovided", "notapplicable")}
        w("| `%s` | %d | %d | %d | %d | %d |\n" % (file, len(rs), c["passed"], c["failed"], c["notprovided"],
                                                  c["notapplicable"]))
    w("\n")

    # The failures, by class
    w("### The failures\n\n")
    if not fails:
        w("None.\n\n")
    for key, title, verdict in CLASSES:
        rs = [r for r in fails if r.why == key]
        if not rs:
            continue
        ops_of = sorted(set(r.op for r in rs))
        w("#### %s: %d case%s (%s)\n\n" % (title, len(rs), "" if len(rs) == 1 else "s", verdict))
        w("Operations: %s.\n\n" % ", ".join("`%s`" % o for o in ops_of))
        w("| Case | Line | GAOL v5 | |\n|---|---|---|---|\n")
        for r in rs:
            w("| `%s` | %s:%d | %s | %s |\n" % (md(r.source), r.file, r.line, md(r.got), md(getattr(r, "note", ""))))
        w("\n")

    # The cases not run
    w("### The cases not run\n\n")
    w("| Why | Operations | Cases |\n|---|---|---:|\n")
    for category in ("notprovided", "notapplicable"):
        whys = []
        for r in rows:
            if r.category == category and r.why not in whys:
                whys.append(r.why)
        for why in whys:
            rs = [r for r in rows if r.category == category and r.why == why]
            ops_of = sorted(set(r.op for r in rs), key=lambda o: o.lower())
            w("| %s: %s | %s | %d |\n" % ("Not provided" if category == "notprovided" else "Not applicable",
                                          md(why), ", ".join("`%s`" % o for o in ops_of), len(rs)))
    w("\n")

    signals = [r for r in rows if r.signal != "-" and r.category in ("passed", "failed")]
    if signals:
        w("%d cases run expect an exception of IEEE 1788-2015 besides their result (%s): their result is "
          "checked, %d of them passing; GAOL v5 signals none of these exceptions.\n\n" %
          (len(signals), ", ".join(sorted(set(r.signal for r in signals))),
           sum(1 for r in signals if r.category == "passed")))

    if differences:
        w("### The differences between the builds\n\n| Case | Line | %s | Build | Result |\n|---|---|---|---|---|\n"
          % base_name)
        for name, b, r in differences:
            w("| `%s` | %s:%d | %s %s | %s | %s %s |\n" % (md(b.source), b.file, b.line, b.status, md(b.got), name,
                                                         r.status, md(r.got)))
        w("\n")

    block = BEGIN + "\n\n" + "".join(lines) + END + "\n"
    try:
        with open(out, encoding="utf-8") as f:
            content = f.read()
    except FileNotFoundError:
        content = ""
    if BEGIN in content and END in content:
        head, rest = content.split(BEGIN, 1)
        tail = rest.split(END, 1)[1]
        content = head + block + tail.lstrip("\n")
    else:
        content = block
    with open(out, "w", encoding="utf-8") as f:
        f.write(content)
    print("%d cases: %d passed, %d failed, %d not provided, %d not applicable" %
          (total, count.get("passed", 0), len(fails), count.get("notprovided", 0), count.get("notapplicable", 0)))
    for key, title, verdict in CLASSES:
        n = sum(1 for r in fails if r.why == key)
        if n:
            print("  %4d %s (%s)" % (n, key, verdict))
    if differences:
        print("%d differences between the builds" % len(differences))


# ---------------------------------------------------------------------------

def main(argv):
    if len(argv) >= 3 and argv[0] == "generate":
        not_provided = dict(NOT_PROVIDED)
        rest = argv[3:]
        if rest[:1] == ["--not-provided"] and len(rest) >= 2:
            for name in filter(None, rest[1].split(",")):
                not_provided[name] = ("%s: counted as not provided by the runner (NOT_PROVIDED in run.sh)" % name)
            rest = rest[2:]
        if rest:
            raise SystemExit(__doc__)
        generate(argv[1], argv[2], not_provided)
    elif len(argv) >= 4 and argv[0] == "report":
        out, programs, rest = argv[1], argv[2], argv[3:]
        labels = {"--gaol": "?", "--itf1788": "?"}
        while rest and rest[0] in labels and len(rest) >= 2:
            labels[rest[0]] = rest[1]
            rest = rest[2:]
        builds = [tuple(b.split("=", 1)) for b in rest]
        if not builds or any(len(b) != 2 for b in builds):
            raise SystemExit(__doc__)
        report(out, programs, builds, labels["--gaol"], labels["--itf1788"])
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main(sys.argv[1:])
