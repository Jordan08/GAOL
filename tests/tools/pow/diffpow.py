#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: the differential test of pow. Builds diffpow.cpp against
# two builds of GAOL or more, runs it with each, and compares the outputs.
#
#     python3 diffpow.py run --out DIR LABEL=BUILD LABEL=BUILD...
#     python3 diffpow.py compare A.txt B.txt
#
# run compiles diffpow.cpp against each CMake build BUILD (see build.py:
# configured with -DWITH_TESTS=ON and -DCMAKE_EXPORT_COMPILE_COMMANDS=ON) into
# DIR/LABEL/diffpow, writes its output to DIR/LABEL/diffpow.txt, and compares
# each output with the first one. compare compares two outputs of diffpow,
# however the programs were built (build.py --pkg-config for an installed
# GAOL, for instance).
#
# The comparison counts the lines that are identical, those that differ only
# in the sign of zero bounds (-0 for +0), and those that differ otherwise, for
# each of the functions diffpow calls (std: gaol_ieee1788::pow(x, y), hyb:
# gaol::pow(x, y), real: gaol::pow(x, p) and gaol_ieee1788::pow(x, p)), and the
# lines of each output whose result depended on the rounding direction at the
# call ("DIFF"). It prints the first lines that differ otherwise, and exits
# with 1 when there is one, or a result that depended on the rounding
# direction: a difference in the sign of a zero alone is reported, not failed.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-04 by Jordan NININ

import argparse
import os
import re
import subprocess
import sys

import build

HERE = os.path.dirname(os.path.abspath(__file__))
LINE = re.compile(r'^(\S+) (\d+) x=(.*?) y=(.*?) std=(.*?) hyb=(.*?) real=(.*?)((?: same| DIFF\{[^}]*\})*)$')
FIELDS = ('std', 'hyb', 'real', 'modes')


def read(path):
    """The lines of an output of diffpow, by (tag, index): (x, y, {field: text})"""
    out = {}
    header = ''
    for n, l in enumerate(open(path), 1):
        l = l.rstrip('\n')
        if l.startswith('#'):
            header = header or l
            continue
        m = LINE.match(l)
        if not m:
            sys.exit('diffpow.py: %s:%d is not a line of diffpow: %s' % (path, n, l[:100]))
        out[(m.group(1), int(m.group(2)))] = (m.group(3), m.group(4), dict(zip(FIELDS, m.group(5, 6, 7, 8))))
    return header, out


def number(t):
    try:
        return float.fromhex(t)
    except ValueError:
        return None


def zero_sign_only(a, b):
    """Whether the texts a and b differ only in the sign of zeros"""
    ta, tb = a.replace('{', ' ').replace('}', ' ').split(), b.replace('{', ' ').replace('}', ' ').split()
    if len(ta) != len(tb):
        return False
    for u, v in zip(ta, tb):
        if u != v:
            x, y = number(u), number(v)
            if x is None or y is None or x != 0.0 or y != 0.0:
                return False
    return True


def differences(a_path, b_path):
    """The comparison of two outputs of diffpow, as a dict: the lines (a and b), the number of identical
    lines, the lines that differ in the sign of zero bounds only and the others, and the number of results
    that depend on the rounding direction in each output; None where the boxes are not the same"""
    ha, a = read(a_path)
    hb, b = read(b_path)
    out = dict(a=a, b=b, headers=(ha, hb), identical=0, zero=dict((f, 0) for f in FIELDS), zero_lines=[],
               other=dict((f, 0) for f in FIELDS), other_lines=[])
    if set(a) != set(b) or any(a[k][:2] != b[k][:2] for k in a):
        return None
    for k in sorted(a):
        ra, rb = a[k][2], b[k][2]
        if ra == rb:
            out['identical'] += 1
            continue
        only_zero = True
        for f in FIELDS:
            if ra[f] != rb[f]:
                if zero_sign_only(ra[f], rb[f]):
                    out['zero'][f] += 1
                else:
                    out['other'][f] += 1
                    only_zero = False
        out['zero_lines' if only_zero else 'other_lines'].append(k)
    out['depends'] = tuple(sum(1 for v in d.values() if 'DIFF' in v[2]['modes']) for d in (a, b))
    return out


def describe(d, k):
    """The line k of the two outputs, where they differ"""
    xa, ya, ra = d['a'][k]
    rb = d['b'][k][2]
    text = ['  %s %d x=%s y=%s' % (k + (xa, ya))]
    for f in FIELDS:
        if ra[f] != rb[f]:
            text.append('    %-5s %s\n          %s' % (f, ra[f].strip(), rb[f].strip()))
    return '\n'.join(text)


def compare(a_path, b_path, show=20):
    """Compares two outputs of diffpow, prints the counts, and returns whether they agree"""
    d = differences(a_path, b_path)
    if d is None:
        print('%s and %s do not have the same boxes: not the same diffpow' % (a_path, b_path))
        return False
    for path, h, n in ((a_path, d['headers'][0], len(d['a'])), (b_path, d['headers'][1], len(d['b']))):
        print('%s: %d lines (%s)' % (path, n, h.lstrip('# ') or 'no header'))
    z, o = d['zero'], d['other']
    print('identical: %d' % d['identical'])
    print('different in the sign of zero bounds only: %d (std %d, hyb %d, real %d, other rounding directions %d)'
          % (len(d['zero_lines']), z['std'], z['hyb'], z['real'], z['modes']))
    print('different otherwise: %d (std %d, hyb %d, real %d, other rounding directions %d)'
          % (len(d['other_lines']), o['std'], o['hyb'], o['real'], o['modes']))
    print('results that depend on the rounding direction at the call: %d in the first, %d in the second'
          % d['depends'])
    for k in d['other_lines'][:show]:
        print(describe(d, k))
    if len(d['other_lines']) > show:
        print('  ... and %d more' % (len(d['other_lines']) - show))
    return not d['other_lines'] and not any(d['depends'])


def run(out, specs, randoms):
    outputs = []
    for spec in specs:
        if '=' not in spec:
            sys.exit('diffpow.py: %s is not LABEL=BUILD' % spec)
        label, path = spec.split('=', 1)
        exe = os.path.join(out, label, 'diffpow')
        build.compile_tool(os.path.join(HERE, 'diffpow.cpp'), exe, build=path)
        txt = exe + '.txt'
        with open(txt, 'w') as f:
            subprocess.check_call([exe, str(randoms)], stdout=f)
        outputs.append(txt)
    ok = True
    for txt in outputs[1:]:
        print()
        ok = compare(outputs[0], txt) and ok
    return ok


def main():
    p = argparse.ArgumentParser(description='The differential test of pow between builds of GAOL.')
    sub = p.add_subparsers(dest='command')
    r = sub.add_parser('run', help='build diffpow against each build, run it and compare the outputs')
    r.add_argument('--out', required=True, help='the directory of the programs and of their outputs')
    r.add_argument('--randoms', type=int, default=60000, help='the number of random boxes (60000)')
    r.add_argument('builds', nargs='+', metavar='LABEL=BUILD', help='a name and a CMake build directory of GAOL')
    c = sub.add_parser('compare', help='compare two outputs of diffpow')
    c.add_argument('first')
    c.add_argument('second')
    c.add_argument('--show', type=int, default=20, help='the number of differing lines printed (20)')
    a = p.parse_args()
    if a.command == 'run':
        ok = run(a.out, a.builds, a.randoms)
    elif a.command == 'compare':
        ok = compare(a.first, a.second, a.show)
    else:
        p.print_help()
        sys.exit(2)
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
