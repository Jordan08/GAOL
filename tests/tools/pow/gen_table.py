#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: the table of pow_on_boxes() in tests/ieee1788.cpp, read
# into a rows file and written back from one.
#
# A rows file has one box per line, "X | Y | S | H": the base X, the exponent
# Y, S = gaol_ieee1788::pow(X, Y) and H = gaol::pow(X, Y), each "empty" or two
# numbers "l u" (decimal or hexadecimal, inf, -inf); lines starting with '#'
# are the comments of the table. evalrows (evalrows.cpp) computes S and H with
# a build of GAOL, and checkrows.py checks them against mpmath.
#
#     python3 gen_table.py extract tests/ieee1788.cpp > expected.txt
#         the boxes of the table and their expected bounds; X and Y as the
#         table writes them, oo written inf
#     python3 gen_table.py cpp results.txt [--into tests/ieee1788.cpp]
#         the rows of the table, {x, y, gaol_ieee1788::pow(x, y)} and
#         {x, y, gaol_ieee1788::pow(x, y), gaol::pow(x, y)} where the two
#         differ, the bounds as integers, short decimals that are exact, or
#         hexadecimal literals; printed, or written in place of the table of
#         the file given with --into
#     python3 gen_table.py diff expected.txt results.txt
#         the boxes whose bounds differ between two rows files, a -0 being a
#         +0, with the distance of each bound in doubles and whether it is
#         tighter or looser
#
# extract followed by cpp gives the table back as it is written, which
# checks both. The table was written so by this script and evalrows from the
# bounds of the code before the pow of the standard was written once
# (a2ca992), then edited for the exact corners of #63.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ

import argparse
import math
import re
import struct
import sys
from fractions import Fraction

START = 'const Box boxes[] = {'
END = '};'


def number(t):
    """A number of a rows file or of the table: decimal, hexadecimal, inf, oo"""
    t = t.strip()
    if t in ('oo', '+oo', 'inf', '+inf'):
        return math.inf
    if t in ('-oo', '-inf'):
        return -math.inf
    if '0x' in t.lower():
        return float.fromhex(t)
    return float(t)


def parse_interval(field):
    """None for the empty set, else (l, u) as floats"""
    t = field.split()
    if t == ['empty']:
        return None
    if len(t) != 2:
        raise ValueError('not an interval: %r' % field)
    return (number(t[0]), number(t[1]))


def read_rows(path):
    """The rows of a rows file: ('#', text) for a comment, ('box', [X, Y, S, H]) the texts of the fields"""
    rows = []
    for n, line in enumerate(open(path), 1):
        line = line.strip()
        if not line:
            continue
        if line.startswith('#'):
            rows.append(('#', line[1:].strip()))
            continue
        fields = [f.strip() for f in line.split('|')]
        if len(fields) not in (2, 4):
            sys.exit('gen_table.py: %s:%d: not a box: %s' % (path, n, line))
        rows.append(('box', fields + [''] * (4 - len(fields))))
    return rows


# ---------------------------------------------------------------- extract

def split_cells(s):
    """The cells of {a, b, c}, split at the commas outside parentheses"""
    cells, depth, cur = [], 0, ''
    for ch in s:
        if ch == ',' and depth == 0:
            cells.append(cur.strip())
            cur = ''
            continue
        depth += (ch == '(') - (ch == ')')
        cur += ch
    if cur.strip():
        cells.append(cur.strip())
    return cells


def literal(tok):
    """A number of the table as a rows file writes it: oo as inf, the others as they are"""
    tok = tok.strip()
    if tok in ('oo', '-oo'):
        return tok.replace('oo', 'inf')
    number(tok)  # checks it
    return tok


def cell_to_field(cell):
    if cell == 'none':
        return 'empty'
    if cell == 'all':
        return '-inf inf'
    m = re.match(r'^P\((.*)\)$', cell)
    if m:
        v = literal(m.group(1))
        return '%s %s' % (v, v)
    m = re.match(r'^I\((.*)\)$', cell)
    if m:
        a = split_cells(m.group(1))
        if len(a) == 2:
            return '%s %s' % (literal(a[0]), literal(a[1]))
    raise ValueError('cell %r: write none, all, P(p) or I(l, u) with numbers' % cell)


def extract(cpp):
    text = open(cpp).read()
    i = text.find('void pow_on_boxes()')
    i = text.find(START, i)
    if i < 0:
        sys.exit('gen_table.py: no table "%s" after pow_on_boxes() in %s' % (START, cpp))
    j = text.find('\n' + ' ' * 4 + END, i)
    out = []
    for n, line in enumerate(text[i + len(START):j].splitlines(), 1):
        s = line.strip()
        if not s:
            continue
        if s.startswith('//'):
            out.append('# ' + s[2:].strip())
            continue
        if not (s.startswith('{') and s.endswith('},')):
            sys.exit('gen_table.py: line %r of the table is not a box {x, y, s[, h]},' % s)
        cells = split_cells(s[1:-2])
        if len(cells) not in (3, 4):
            sys.exit('gen_table.py: box %r has not 3 or 4 cells' % s)
        try:
            f = [cell_to_field(c) for c in cells]
        except ValueError as e:
            sys.exit('gen_table.py: %s' % e)
        out.append(' | '.join(f[:3] + [f[3] if len(f) == 4 else f[2]]))
    print('\n'.join(out))


# ---------------------------------------------------------------- cpp

def hexfloat(v):
    """v as a hexadecimal literal, normalized also for a subnormal, without trailing zeros"""
    sign = '-' if v < 0 else ''
    a = abs(v)
    if a >= 2.2250738585072014e-308:
        h = a.hex()
    else:
        n = int(Fraction(a) * (1 << 1074))  # a = n 2^-1074
        e = n.bit_length() - 1
        h = '0x1.%013xp%+d' % ((n - (1 << e)) << (52 - e), e - 1074)
    mant, exp = h.split('p')
    if '.' in mant:
        mant = mant.rstrip('0').rstrip('.')
    return sign + mant + 'p' + (exp if exp[0] in '+-' else '+' + exp)


def num(v):
    """A bound as the table writes it: an integer, a short decimal that is exact, or a hexadecimal literal"""
    if v == 0:
        return '0'
    if v == math.inf:
        return 'oo'
    if v == -math.inf:
        return '-oo'
    if v == int(v) and abs(v) < 1e15:
        return str(int(v))
    r = repr(v)
    if 'e' not in r and len(r) <= 9 and Fraction(r) == Fraction(v):
        return r
    return hexfloat(v)


def result_cpp(field):
    x = parse_interval(field)
    if x is None:
        return 'none'
    l, u = x
    if l == -math.inf and u == math.inf:
        return 'all'
    if l == u:
        return 'P(%s)' % num(l)
    return 'I(%s, %s)' % (num(l), num(u))


def input_cpp(field):
    t = field.split()
    if t == ['empty']:
        return 'none'
    c = [x.replace('inf', 'oo') for x in t]
    if c[0] == c[1]:
        return 'P(%s)' % c[0]
    return 'I(%s, %s)' % (c[0], c[1])


def same(a, b):
    """Whether two fields are the same interval, a -0 being a +0"""
    return parse_interval(a) == parse_interval(b)


def cpp(rows_path, into=None):
    out = []
    for kind, r in read_rows(rows_path):
        if kind == '#':
            out.append(' ' * 6 + '// ' + r)
            continue
        if not r[2] or not r[3]:
            sys.exit('gen_table.py: box %s | %s has no bounds: run evalrows first' % (r[0], r[1]))
        cells = [input_cpp(r[0]), input_cpp(r[1]), result_cpp(r[2])]
        if not same(r[2], r[3]):
            cells.append(result_cpp(r[3]))
        out.append(' ' * 6 + '{' + ', '.join(cells) + '},')
    if into is None:
        print('\n'.join(out))
        return
    text = open(into).read()
    i = text.find(START, text.find('void pow_on_boxes()'))
    j = text.find('\n' + ' ' * 4 + END, i)
    if i < 0 or j < 0:
        sys.exit('gen_table.py: no table "%s" after pow_on_boxes() in %s' % (START, into))
    open(into, 'w').write(text[:i + len(START)] + '\n' + '\n'.join(out) + text[j:])


# ---------------------------------------------------------------- diff

def ordinal(d):
    """A monotone integer image of the doubles, -0 and +0 both 0, the infinities next to DBL_MAX"""
    if d == 0.0:
        return 0
    (b,) = struct.unpack('<q', struct.pack('<d', d))
    return b if b >= 0 else -(b & 0x7fffffffffffffff)


def bound_change(old, new, lower):
    if old == new:
        return 'same'
    d = abs(ordinal(new) - ordinal(old))
    tighter = (new > old) if lower else (new < old)
    return '%s by %d double%s' % ('tighter' if tighter else 'LOOSER', d, '' if d == 1 else 's')


def diff(a_path, b_path):
    def boxes(path):
        seen = {}
        out = []
        for kind, r in read_rows(path):
            if kind == 'box':
                key = (' '.join(r[0].split()), ' '.join(r[1].split()))
                seen[key] = seen.get(key, 0) + 1
                out.append((key + (seen[key],), r))
        return out
    a, b = boxes(a_path), boxes(b_path)
    bd = dict(b)
    changed = looser = 0
    for n, (key, r) in enumerate(a, 1):
        if key not in bd:
            print('box %3d %s | %s: only in %s' % (n, r[0], r[1], a_path))
            continue
        s = bd[key]
        lines = []
        for i, name in ((2, 'std'), (3, 'hyb')):
            if same(r[i], s[i]):
                continue
            x, y = parse_interval(r[i]), parse_interval(s[i])
            if x is None or y is None:
                what = 'empty set changed'
                looser += 1
            else:
                lo, up = bound_change(x[0], y[0], True), bound_change(x[1], y[1], False)
                what = 'lower %s, upper %s' % (lo, up)
                looser += ('LOOSER' in what)
            lines.append('    %s %s -> %s: %s' % (name, r[i], s[i], what))
        if lines:
            changed += 1
            print('box %3d %s | %s' % (n, r[0], r[1]))
            print('\n'.join(lines))
    keys = set(k for k, _ in a)
    for key, r in b:
        if key not in keys:
            print('        %s | %s: only in %s' % (r[0], r[1], b_path))
    print('boxes: %d, changed: %d, results with a looser bound or another empty set: %d' % (len(a), changed, looser))
    return changed == 0


def main():
    p = argparse.ArgumentParser(description='The table of pow_on_boxes() in tests/ieee1788.cpp and rows files.')
    sub = p.add_subparsers(dest='command')
    e = sub.add_parser('extract', help='the boxes of the table of a tests/ieee1788.cpp as a rows file')
    e.add_argument('cpp')
    c = sub.add_parser('cpp', help='the rows of the table from a rows file')
    c.add_argument('rows')
    c.add_argument('--into', metavar='CPP', help='write them in place of the table of this tests/ieee1788.cpp')
    d = sub.add_parser('diff', help='the boxes whose bounds differ between two rows files')
    d.add_argument('first')
    d.add_argument('second')
    a = p.parse_args()
    if a.command == 'extract':
        extract(a.cpp)
    elif a.command == 'cpp':
        cpp(a.rows, a.into)
    elif a.command == 'diff':
        sys.exit(0 if diff(a.first, a.second) else 1)
    else:
        p.print_help()
        sys.exit(2)


if __name__ == '__main__':
    main()
