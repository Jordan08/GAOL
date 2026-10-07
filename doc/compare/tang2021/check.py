#!/usr/bin/env python3
"""Checks the queries of the benchmark of Tang et al. (2021) for GAOL v5.

The benchmark (see ../tang2021.md) prints, for each input of each expression
and each library, a query

    GAOL: lo <= expression <= hi

whose bounds lo and hi are the library's interval, written as exact
rationals, and whose expression is the expression of the benchmark at the
input, written with exact rationals too; the paper checks the queries with
Mathematica (mathmetica_notebooks/verify_queries.wls). This script checks them
without Mathematica, and twice:

  (a) the query as the benchmark prints it: its decimal constants, such as
      0.954929658551372 in sineOrder3, are the exact decimals;
  (b) the expression as the program computes it (src/methods.hpp and
      src/FPBench_methods.hpp of the benchmark), on the inputs that the line
      INPUT before the queries gives exactly: its constants are the doubles
      the program computes, the literals rounded to nearest when compiling.
      This is what the library was asked to enclose. The operations between
      constants, (3.14159265359) / 180.0 in polarToCarthesian, are computed
      when the program runs (-frounding-math, which GAOL's flags give the
      benchmark, keeps them from being folded), once, where the compiler
      puts them (GCC does not model the rounding direction, GCC bug 34678),
      to nearest or upward: both are tried, and the query is right if it
      holds with either (three expressions only have such a constant, and
      how many queries hold with one only is counted).

A query is right when the exact value is in [lo, hi], wrong when it is not,
and undefined when the exact expression has no real value at the input (a
square root or a logarithm of a negative number, a division by zero), which
Mathematica's script does not count as wrong either. The exact value is
computed with rational arithmetic as long as the expression is rational, and
then enclosed with MPFR (gmpy2), each bound rounded in its direction, at 256
bits, and at 2000 bits for the few queries 256 bits do not decide; a query
that 2000 bits do not decide is counted as undecided. For the 8 single
operations (+, -, *, /, sqrt, exp, sin, cos), the script also checks that the
interval is the tightest, [RD(v), RU(v)]. And for every input, it compares the
width of GAOL's interval with that of filib's C version (FILIB C), the
paper's reference for the sizes.

    query_bin | check.py check --sources DIR --out shard.json
    check.py report shard.json... > results.md

Copyright (c) 2026 ENSTA, France

Created 2026-10-06 by Jordan NININ
"""

import argparse
import ast
import json
import math
import re
import struct
import sys
from fractions import Fraction

import gmpy2
from gmpy2 import mpfr, mpq, mpz

SINGLE_OPERATIONS = ('addition', 'subtraction', 'multiplication', 'division',
                     'square_root', 'exponential', 'sin', 'cos')
FUNCTIONS = {'sqrt': 'sqrt', 'Sqrt': 'sqrt', 'exp': 'exp', 'Exp': 'exp', 'log': 'log',
             'sin': 'sin', 'cos': 'cos', 'tan': 'tan', 'atan': 'atan'}
REFERENCE_WIDTH = 'FILIB C'
EXAMPLES = 5


class Undefined(Exception):
    """The exact expression has no real value."""


class Undecided(Exception):
    """The precision does not decide."""


# ----------------------------------------------------------------------------
# The exact values: an mpq while the expression is rational, then a pair
# (lo, hi) of mpfr enclosing it, each bound rounded in its direction
# ----------------------------------------------------------------------------

CONTEXTS = {bits: tuple(gmpy2.context(precision=bits, round=r, emax=gmpy2.get_emax_max(),
                                       emin=gmpy2.get_emin_min())
                         for r in (gmpy2.RoundDown, gmpy2.RoundUp))
            for bits in (256, 2000)}
DN, UP = CONTEXTS[256]
D64 = gmpy2.ieee(64)
D64.round = gmpy2.RoundDown
U64 = gmpy2.ieee(64)
U64.round = gmpy2.RoundUp


def set_precision(bits):
    global DN, UP
    DN, UP = CONTEXTS[bits]


def iv(x):
    """x as a pair of mpfr."""
    if isinstance(x, tuple):
        return x
    return (DN.div(x.numerator, x.denominator), UP.div(x.numerator, x.denominator))


def NEG(x):
    if isinstance(x, tuple):
        return (-x[1], -x[0])
    return -x


def ADD(x, y):
    if not isinstance(x, tuple) and not isinstance(y, tuple):
        return x + y
    x, y = iv(x), iv(y)
    return (DN.add(x[0], y[0]), UP.add(x[1], y[1]))


def SUB(x, y):
    if not isinstance(x, tuple) and not isinstance(y, tuple):
        return x - y
    x, y = iv(x), iv(y)
    return (DN.sub(x[0], y[1]), UP.sub(x[1], y[0]))


def MUL(x, y):
    if not isinstance(x, tuple) and not isinstance(y, tuple):
        return x * y
    (a, b), (c, d) = iv(x), iv(y)
    return (min(DN.mul(a, c), DN.mul(a, d), DN.mul(b, c), DN.mul(b, d)),
            max(UP.mul(a, c), UP.mul(a, d), UP.mul(b, c), UP.mul(b, d)))


def DIV(x, y):
    if not isinstance(y, tuple):
        if y == 0:
            raise Undefined
        if not isinstance(x, tuple):
            return x / y
    (a, b), (c, d) = iv(x), iv(y)
    if c <= 0 <= d:
        raise Undecided
    return (min(DN.div(a, c), DN.div(a, d), DN.div(b, c), DN.div(b, d)),
            max(UP.div(a, c), UP.div(a, d), UP.div(b, c), UP.div(b, d)))


def F_sqrt(x):
    if not isinstance(x, tuple):
        if x < 0:
            raise Undefined
        n, d = x.numerator, x.denominator
        if gmpy2.is_square(n) and gmpy2.is_square(d):
            return mpq(gmpy2.isqrt(n), gmpy2.isqrt(d))
        x = iv(x)
    if x[1] < 0:
        raise Undefined
    if x[0] < 0:
        raise Undecided
    return (DN.sqrt(x[0]), UP.sqrt(x[1]))


def F_exp(x):
    if not isinstance(x, tuple):
        if x == 0:
            return mpq(1)
        x = iv(x)
    return (DN.exp(x[0]), UP.exp(x[1]))


def F_log(x):
    if not isinstance(x, tuple):
        if x <= 0:
            raise Undefined
        if x == 1:
            return mpq(0)
        x = iv(x)
    if x[1] <= 0:
        raise Undefined
    if x[0] <= 0:
        raise Undecided
    return (DN.log(x[0]), UP.log(x[1]))


def F_atan(x):
    if not isinstance(x, tuple):
        if x == 0:
            return mpq(0)
        x = iv(x)
    return (DN.atan(x[0]), UP.atan(x[1]))


# sin, cos and tan of a pair: monotonic on it when the derivative keeps its
# sign at both ends of a pair narrower than pi (the zeros of the derivative are
# simple, and pi apart)
def F_sin(x):
    if not isinstance(x, tuple):
        if x == 0:
            return mpq(0)
        x = iv(x)
    a, b = x
    if a == b:
        return (DN.sin(a), UP.sin(a))
    if b - a >= 1:
        raise Undecided
    if DN.cos(a) > 0 and DN.cos(b) > 0:
        return (DN.sin(a), UP.sin(b))
    if UP.cos(a) < 0 and UP.cos(b) < 0:
        return (DN.sin(b), UP.sin(a))
    raise Undecided


def F_cos(x):
    if not isinstance(x, tuple):
        if x == 0:
            return mpq(1)
        x = iv(x)
    a, b = x
    if a == b:
        return (DN.cos(a), UP.cos(a))
    if b - a >= 1:
        raise Undecided
    if DN.sin(a) > 0 and DN.sin(b) > 0:
        return (DN.cos(b), UP.cos(a))
    if UP.sin(a) < 0 and UP.sin(b) < 0:
        return (DN.cos(a), UP.cos(b))
    raise Undecided


def F_tan(x):
    if not isinstance(x, tuple):
        if x == 0:
            return mpq(0)
        x = iv(x)
    a, b = x
    if a == b:
        return (DN.tan(a), UP.tan(a))
    if b - a >= 1:
        raise Undecided
    if (DN.cos(a) > 0 and DN.cos(b) > 0) or (UP.cos(a) < 0 and UP.cos(b) < 0):
        return (DN.tan(a), UP.tan(b))
    raise Undecided


def subscript(node):
    """i of x[i] (an ast.Index around it before Python 3.9)."""
    sl = node.slice.value if isinstance(node.slice, ast.Index) else node.slice
    return sl.value


NAMESPACE = {'NEG': NEG, 'ADD': ADD, 'SUB': SUB, 'MUL': MUL, 'DIV': DIV,
             'F_sqrt': F_sqrt, 'F_exp': F_exp, 'F_log': F_log, 'F_sin': F_sin,
             'F_cos': F_cos, 'F_tan': F_tan, 'F_atan': F_atan}
BINARY = {ast.Add: 'ADD', ast.Sub: 'SUB', ast.Mult: 'MUL', ast.Div: 'DIV'}


# ----------------------------------------------------------------------------
# (a): the query as printed. Its integers become n[0], n[1]..., the expression
# a function of them, compiled once for each shape of query
# ----------------------------------------------------------------------------

def compile_query(shape):
    k = [0]

    def number(_):
        k[0] += 1
        return 'n[%d]' % (k[0] - 1)
    text = re.sub(r'#', number, shape)

    def tr(node):
        if isinstance(node, ast.BinOp) and type(node.op) in BINARY:
            return '%s(%s, %s)' % (BINARY[type(node.op)], tr(node.left), tr(node.right))
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
            return 'NEG(%s)' % tr(node.operand)
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.UAdd):
            return tr(node.operand)
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id in FUNCTIONS \
           and len(node.args) == 1:
            return 'F_%s(%s)' % (FUNCTIONS[node.func.id], tr(node.args[0]))
        if isinstance(node, ast.Subscript):
            return 'n[%d]' % subscript(node)
        raise ValueError('unexpected in a query: %s' % ast.dump(node))
    code = tr(ast.parse(text, mode='eval').body)
    return eval('lambda n: ' + code, dict(NAMESPACE))


# ----------------------------------------------------------------------------
# (b): the expression as the program computes it, from its C++ source. The
# operations between constants are folded here as C++ computes them: integers
# as integers, doubles to nearest or upward
# ----------------------------------------------------------------------------

def next_up(f):
    if f != f or f == math.inf:
        return f
    if f == 0.0:
        return 5e-324
    b = struct.unpack('<q', struct.pack('<d', f))[0]
    b = b + 1 if f > 0 else b - 1
    return struct.unpack('<d', struct.pack('<q', b))[0]


def double_op(op, a, b, upward):
    a, b = float(a), float(b)
    if not upward:
        return {ast.Add: a + b, ast.Sub: a - b, ast.Mult: a * b}[op] if op is not ast.Div else a / b
    exact = {ast.Add: Fraction(a) + Fraction(b), ast.Sub: Fraction(a) - Fraction(b),
             ast.Mult: Fraction(a) * Fraction(b), ast.Div: Fraction(a) / Fraction(b)}[op]
    f = float(exact)
    if Fraction(f) < exact:
        f = next_up(f)
    return f


def compile_source(cpp, upward):
    """The C++ expression as a function of the inputs v, and its constants."""
    text = re.sub(r'value_array\[(\d+)\]', r'v[\1]', cpp)
    constants = []

    def const(value):
        constants.append(mpq(value))
        return ('code', 'K[%d]' % (len(constants) - 1))

    def code(x):
        return const(x[2])[1] if x[0] == 'const' else x[1]

    def tr(node):
        if isinstance(node, ast.Constant) and type(node.value) in (int, float):
            return ('const', type(node.value), node.value)
        if isinstance(node, ast.Subscript):
            return ('code', 'v[%d]' % subscript(node))
        if isinstance(node, ast.UnaryOp) and type(node.op) in (ast.USub, ast.UAdd):
            x = tr(node.operand)
            sign = -1 if isinstance(node.op, ast.USub) else 1
            if x[0] == 'const':
                return ('const', x[1], sign * x[2])
            return ('code', 'NEG(%s)' % x[1]) if sign < 0 else x
        if isinstance(node, ast.BinOp) and type(node.op) in BINARY:
            x, y = tr(node.left), tr(node.right)
            if x[0] == 'const' and y[0] == 'const':
                if x[1] is int and y[1] is int:
                    a, b = x[2], y[2]
                    if isinstance(node.op, ast.Div):
                        q = abs(a) // abs(b)
                        return ('const', int, q if (a < 0) == (b < 0) else -q)
                    return ('const', int, {ast.Add: a + b, ast.Sub: a - b, ast.Mult: a * b}[type(node.op)])
                return ('const', float, double_op(type(node.op), x[2], y[2], upward))
            return ('code', '%s(%s, %s)' % (BINARY[type(node.op)], code(x), code(y)))
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id in FUNCTIONS \
           and len(node.args) == 1:
            x = tr(node.args[0])
            if x[0] == 'const':
                raise ValueError('a function of a constant, computed by the libm: not supported')
            return ('code', 'F_%s(%s)' % (FUNCTIONS[node.func.id], x[1]))
        raise ValueError('unexpected in the source: %s' % ast.dump(node))
    body = tr(ast.parse(text, mode='eval').body)
    if body[0] == 'const':
        raise ValueError('a constant expression')
    return eval('lambda v, K: ' + body[1], dict(NAMESPACE)), constants


def read_sources(directory):
    text = ''
    for name in ('methods.hpp', 'FPBench_methods.hpp'):
        with open('%s/src/%s' % (directory, name)) as f:
            text += f.read()
    bodies = dict(re.findall(r'inline T (\w+)\(const std::vector<T> &value_array\)\s*\{\s*return (.*?);\s*\}',
                             text, re.S))
    return {k: ' '.join(v.split()) for k, v in bodies.items()}


# ----------------------------------------------------------------------------
# Checking
# ----------------------------------------------------------------------------

def bound(text):
    """A bound of a query: an mpq, or None for an infinite one."""
    if text in ('Infinity', '-Infinity'):
        return None
    return mpq(text)


def verdict(value, lo, hi):
    if not isinstance(value, tuple):
        return 'right' if (lo is None or lo <= value) and (hi is None or value <= hi) else 'wrong'
    a, b = value
    if (lo is None or lo <= a) and (hi is None or b <= hi):
        return 'right'
    if (hi is not None and hi < a) or (lo is not None and lo > b):
        return 'wrong'
    raise Undecided


def decide(evaluate, lo, hi):
    """right, wrong, undefined or undecided, at 256 bits, then at 2000."""
    for bits in (256, 2000):
        set_precision(bits)
        try:
            return verdict(evaluate(), lo, hi), bits
        except Undefined:
            return 'undefined', bits
        except Undecided:
            continue
        finally:
            set_precision(256)
    return 'undecided', 2000


def tightest(value):
    """[RD(v), RU(v)] in doubles, or None when the precision does not decide."""
    if not isinstance(value, tuple):
        n, d = value.numerator, value.denominator
        return D64.div(n, d), U64.div(n, d)
    a, b = value
    lo, lo2 = D64.plus(a), D64.plus(b)
    hi, hi2 = U64.plus(a), U64.plus(b)
    return (lo, hi) if lo == lo2 and hi == hi2 else None


def new_stats():
    return {'queries': 0, 'empty': 0, 'infinite': 0,
            'a': {'right': 0, 'wrong': 0, 'undefined': 0, 'undecided': 0},
            'b': {'right': 0, 'wrong': 0, 'undefined': 0, 'undecided': 0},
            'b_nearest_only': 0, 'b_upward_only': 0,
            'tightest': 0, 'not_tightest': 0, 'tightest_undecided': 0,
            'narrower': 0, 'same': 0, 'wider': 0, 'ratios': {},
            'examples': []}


def check(args):
    sources = read_sources(args.sources)
    libraries = set(args.libs.split(','))
    set_precision(256)
    stats = {}
    query_cache = {}
    expression = None
    source = {}
    inputs = None
    widths = {}
    for line in sys.stdin:
        if line.startswith('EXPRESSION, '):
            expression = line[12:].strip()
            source = {}
            continue
        if line.startswith('INPUT'):
            inputs = [mpq(float.fromhex(x)) for x in line.rstrip('\n').split(', ')[1:]]
            widths = {}
            continue
        library, sep, query = line.partition(': ')
        if not sep or expression is None:
            continue
        first = query.index(' <= ')
        last = query.rindex(' <= ')
        lo, hi = bound(query[:first]), bound(query[last + 4:].strip())
        if lo is not None and hi is not None and lo > hi:
            width = -1
        elif lo is None or hi is None:
            width = None
        else:
            width = hi - lo
        widths[library] = width
        if library not in libraries:
            continue
        st = stats.setdefault(library, {}).setdefault(expression, new_stats())
        st['queries'] += 1
        if width == -1:
            st['empty'] += 1
        elif width is None:
            st['infinite'] += 1
        # (a) the query as printed
        text = query[first + 4:last]
        numbers = re.findall(r'\d+', text)
        shape = re.sub(r'\d+', '#', text)
        f = query_cache.get(shape)
        if f is None:
            f = query_cache[shape] = compile_query(shape)
        n = [mpq(int(x)) for x in numbers]
        va, _ = decide(lambda: f(n), lo, hi)
        st['a'][va] += 1
        # (b) the expression as the program computes it, with its constants
        # computed to nearest, and upward when they differ
        if not source:
            nearest, upward = (compile_source(sources[expression], u) for u in (False, True))
            source['variants'] = [nearest] if nearest[1] == upward[1] else [nearest, upward]
        verdicts = [decide(lambda: g(inputs, K), lo, hi)[0] for g, K in source['variants']]
        vb = next((v for v in ('right', 'undefined', 'undecided') if v in verdicts), 'wrong')
        st['b'][vb] += 1
        if len(verdicts) == 2 and verdicts.count('right') == 1:
            st['b_nearest_only' if verdicts[0] == 'right' else 'b_upward_only'] += 1
        g, K = source['variants'][0]
        # the tightest interval, for the single operations
        if expression in SINGLE_OPERATIONS and vb == 'right':
            for bits in (256, 2000):
                set_precision(bits)
                t = tightest(g(inputs, K))
                set_precision(256)
                if t is not None:
                    break
            if t is None:
                st['tightest_undecided'] += 1
            elif lo is not None and hi is not None and lo == t[0] and hi == t[1]:
                st['tightest'] += 1
            else:
                st['not_tightest'] += 1
        # the width, against the reference's at the same input
        ref = widths.get(REFERENCE_WIDTH)
        if library != REFERENCE_WIDTH and ref is not None and ref != -1 and width is not None and width != -1:
            if width < ref:
                st['narrower'] += 1
            elif width == ref:
                st['same'] += 1
            else:
                st['wider'] += 1
            if width > 0 and ref > 0:
                key = '%.6g' % (width / ref)
                st['ratios'][key] = st['ratios'].get(key, 0) + 1
        if (va == 'wrong' or vb != 'right' and vb != 'undefined') and len(st['examples']) < EXAMPLES:
            st['examples'].append({'inputs': [float(x).hex() for x in inputs],
                                   'lo': str(lo), 'hi': str(hi), 'a': va, 'b': vb})
    with open(args.out, 'w') as out:
        json.dump(stats, out)


# ----------------------------------------------------------------------------
# The report: the shards merged, as Markdown tables
# ----------------------------------------------------------------------------

def merge(files):
    total = {}
    for name in files:
        with open(name) as f:
            for library, expressions in json.load(f).items():
                for expression, st in expressions.items():
                    t = total.setdefault(library, {}).setdefault(expression, new_stats())
                    for k, v in st.items():
                        if isinstance(v, dict):
                            for kk, vv in v.items():
                                t[k][kk] = t[k].get(kk, 0) + vv
                        elif isinstance(v, list):
                            t[k].extend(v)
                        else:
                            t[k] += v
    return total


def median(counts):
    """The median of values counted in a dictionary {value: count}."""
    values = sorted((float(v), n) for v, n in counts.items())
    half = sum(n for _, n in values) // 2
    for v, n in values:
        half -= n
        if half < 0:
            return v
    return float('nan')


def report(args):
    total = merge(args.files)
    order = args.order.split(',') if args.order else None
    for library in sorted(total):
        expressions = total[library]
        names = [e for e in order if e in expressions] if order else sorted(expressions)
        q = sum(expressions[e]['queries'] for e in names)
        print('### %s: %d queries, %d expressions\n' % (library, q, len(names)))
        for crit in ('a', 'b'):
            s = {k: sum(expressions[e][crit][k] for e in names) for k in ('right', 'wrong', 'undefined', 'undecided')}
            print('- (%s): %d right, %d wrong, %d undefined, %d undecided' % (crit, s['right'], s['wrong'], s['undefined'], s['undecided']))
        print('- empty results: %d, unbounded results: %d' % (sum(expressions[e]['empty'] for e in names),
                                                             sum(expressions[e]['infinite'] for e in names)))
        for e in names:
            st = expressions[e]
            if st['b_nearest_only'] or st['b_upward_only']:
                print('- `%s`: (b) right with the constants to nearest only: %d, upward only: %d'
                      % (e, st['b_nearest_only'], st['b_upward_only']))
        print()
        print('| Expression | Queries | (a) wrong | (b) wrong | Undefined | Undecided | Tightest | Narrower / same / wider than %s | Median width ratio |' % REFERENCE_WIDTH)
        print('|---|---|---|---|---|---|---|---|---|')
        for e in names:
            st = expressions[e]
            tight = ('%d of %d' % (st['tightest'], st['tightest'] + st['not_tightest'] + st['tightest_undecided'])
                     if e in SINGLE_OPERATIONS else '')
            ratio = median(st['ratios'])
            print('| `%s` | %d | %d | %d | %d | %d | %s | %d / %d / %d | %s |' % (
                e, st['queries'], st['a']['wrong'], st['b']['wrong'],
                st['b']['undefined'], st['a']['undecided'] + st['b']['undecided'], tight,
                st['narrower'], st['same'], st['wider'],
                '' if ratio != ratio else '%.3g' % ratio))
        print()
        for e in names:
            for ex in expressions[e]['examples']:
                print('- `%s` (a) %s, (b) %s: inputs %s, interval [%s, %s]' % (
                    e, ex['a'], ex['b'], ', '.join(ex['inputs']), ex['lo'], ex['hi']))
        print()


def speed(args):
    """The table of the speed test: the time of each library, in ms."""
    times = {}
    libraries = []
    for line in open(args.file):
        m = re.match(r'^TIME, (.*), ([A-Z][A-Z ]*), ([0-9.]+) us, ', line)
        if m:
            times.setdefault(m.group(1), {})[m.group(2)] = float(m.group(3)) / 1000
            if m.group(2) not in libraries:
                libraries.append(m.group(2))
    others = [lib for lib in libraries if lib != REFERENCE_WIDTH]
    print('| Expression | %s | %s |' % (' | '.join('%s (ms)' % lib for lib in libraries),
                                      ' | '.join('%s / %s' % (lib, REFERENCE_WIDTH) for lib in others)))
    print('|---|%s|' % '|'.join('---' for _ in libraries + others))
    for e, t in times.items():
        print('| %s | %s | %s |' % (e, ' | '.join('%.1f' % t[lib] for lib in libraries),
                                    ' | '.join('%.2f' % (t[lib] / t[REFERENCE_WIDTH]) for lib in others)))


def selftest(_):
    """The checker against mpmath at 2000 bits and exact rational arithmetic."""
    import random
    import mpmath
    mpmath.mp.prec = 2000
    rng = random.Random(1)
    failures = 0

    def mp(x):  # an mpfr or an mpq as an mpmath number, exactly for an mpfr
        n, d = (x.numerator, x.denominator) if isinstance(x, type(mpq())) else x.as_integer_ratio()
        return mpmath.mpf(int(n)) / int(d)
    set_precision(256)
    functions = {'sqrt': (F_sqrt, mpmath.sqrt, 0, 1e6), 'exp': (F_exp, mpmath.exp, -700, 700),
                 'log': (F_log, mpmath.log, 1e-300, 1e300), 'sin': (F_sin, mpmath.sin, -1e6, 1e6),
                 'cos': (F_cos, mpmath.cos, -1e6, 1e6), 'tan': (F_tan, mpmath.tan, -1e6, 1e6),
                 'atan': (F_atan, mpmath.atan, -1e6, 1e6)}
    for name, (f, ref, a, b) in functions.items():
        for _ in range(2000):
            x = rng.uniform(a, b) if name != 'log' else 10 ** rng.uniform(-300, 300)
            v = f(mpq(x))
            exact = ref(mpmath.mpf(x))
            lo, hi = (v, v) if not isinstance(v, tuple) else v
            # the enclosure holds mpmath's value, and is narrower than 2^-250 relatively
            if not (mp(lo) <= exact <= mp(hi)) or \
               mp(hi) - mp(lo) > abs(exact) * mpmath.mpf(2) ** -250:
                print('FAIL', name, x.hex(), lo, hi, exact)
                failures += 1
        # of a pair around the point too, as inside an expression (sin and cos
        # of the pairs on both sides of an extremum are undecided, not wrong)
        for _ in range(500):
            x = rng.uniform(a, b) if name != 'log' else 10 ** rng.uniform(-300, 300)
            eps = UP.mul(mpfr(2) ** -200, abs(x))
            pair = (DN.sub(iv(mpq(x))[0], eps), UP.add(iv(mpq(x))[1], eps))
            try:
                lo, hi = f(pair)
            except Undecided:
                continue
            for y in (pair[0], pair[1], DN.div(DN.add(pair[0], pair[1]), 2)):
                if not (mp(lo) <= ref(mp(y)) <= mp(hi)):
                    print('FAIL pair', name, x.hex())
                    failures += 1
    # pairs around the extrema of sin and cos, and the poles of tan: undecided,
    # or enclosing the extremum
    pi = mpmath.pi
    for k in range(-50, 50):
        for name, f, centre, extremum in (('sin', F_sin, (k + mpmath.mpf(1) / 2) * pi, (-1) ** k),
                                          ('cos', F_cos, k * pi, (-1) ** k),
                                          ('tan', F_tan, (k + mpmath.mpf(1) / 2) * pi, None)):
            c = mpfr(mpmath.nstr(centre, 120), 256)
            # 2^-60 from it: the function moves by 2^-121 there, which 256 bits see
            pair = (DN.sub(c, mpfr(2) ** -60), UP.add(c, mpfr(2) ** -60))
            try:
                lo, hi = f(pair)
            except Undecided:
                continue
            if extremum is None or not (mp(lo) <= extremum <= mp(hi)):
                print('FAIL extremum', name, k)
                failures += 1
    # the rational operations, against fractions.Fraction
    for _ in range(2000):
        xs = [rng.uniform(-1e6, 1e6) for _ in range(4)]
        q = [mpq(x) for x in xs]
        fr = [Fraction(x) for x in xs]
        got = DIV(SUB(MUL(q[0], q[1]), q[2]), ADD(q[3], mpq(1, 3)))
        want = (fr[0] * fr[1] - fr[2]) / (fr[3] + Fraction(1, 3))
        if Fraction(int(got.numerator), int(got.denominator)) != want:
            print('FAIL rational', xs)
            failures += 1
    # the decisions
    q = compile_query(re.sub(r'\d+', '#', 'Sqrt(2) * (1/3)'))
    cases = [
        (lambda: q([mpq(2), mpq(1), mpq(3)]), mpq(0), mpq(1), 'right'),
        (lambda: q([mpq(2), mpq(1), mpq(3)]), mpq(1, 2), mpq(1), 'wrong'),
        (lambda: compile_query('#/#')([mpq(1), mpq(3)]), mpq(1, 3), mpq(1, 3), 'right'),
        (lambda: compile_query('#/#')([mpq(1), mpq(3)]), mpq(1, 3) + mpq(1, 10 ** 30), None, 'wrong'),
        (lambda: compile_query('Sqrt(-#)')([mpq(1)]), None, None, 'undefined'),
        (lambda: compile_query('#/(#-#)')([mpq(1), mpq(2), mpq(2)]), None, None, 'undefined'),
        (lambda: compile_query('log(#)')([mpq(1)]), mpq(0), mpq(0), 'right'),
        (lambda: compile_query('sin(#)-sin(#)')([mpq(1), mpq(1)]), mpq(0), mpq(0), 'undecided'),
    ]
    for evaluate, lo, hi, want in cases:
        got, _ = decide(evaluate, lo, hi)
        if got != want:
            print('FAIL decision', want, got)
            failures += 1
    # the constants folded upward: (3.14159265359) / 180.0 of polarToCarthesian
    g, K = compile_source('value_array[0] * ((3.14159265359) / 180.0)', True)
    exact = Fraction(3.14159265359) / 180
    if not (exact <= Fraction(float(K[0])) < exact + Fraction(1, 2 ** 58)):  # the ulp of 0.017
        print('FAIL upward constant', float(K[0]))
        failures += 1
    g, K = compile_source('value_array[0] * ((3.14159265359) / 180.0)', False)
    if float(K[0]) != 3.14159265359 / 180.0:
        print('FAIL nearest constant', float(K[0]))
        failures += 1
    g, K = compile_source('value_array[0] * (1 / 2) + 7 / 2', False)
    if [int(k) for k in K] != [0, 3]:
        print('FAIL integer division', K)
        failures += 1
    print('selftest: %d failures' % failures)
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command')
    c = sub.add_parser('check', help='check the queries read on the standard input')
    c.add_argument('--sources', required=True, help='the directory of the benchmark')
    c.add_argument('--libs', default='GAOL', help='the libraries checked, separated by commas (default: GAOL)')
    c.add_argument('--out', required=True, help='the JSON file of the results')
    r = sub.add_parser('report', help='merge the results of the shards into Markdown tables')
    r.add_argument('--order', default='', help='the expressions, in the order of the benchmark')
    r.add_argument('files', nargs='+')
    t = sub.add_parser('speed', help='the table of the output of speed_bin')
    t.add_argument('file')
    sub.add_parser('selftest', help='check the checker against mpmath and fractions')
    args = parser.parse_args()
    if args.command == 'check':
        check(args)
    elif args.command == 'report':
        report(args)
    elif args.command == 'speed':
        speed(args)
    elif args.command == 'selftest':
        sys.exit(1 if selftest(args) else 0)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
