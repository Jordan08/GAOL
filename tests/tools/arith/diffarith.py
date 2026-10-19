#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: the differential test of the arithmetic. Builds
# diffarith.cpp against two builds of GAOL or more, runs it with each, and
# compares the outputs.
#
#     python3 diffarith.py run --out DIR LABEL=BUILD LABEL=BUILD...
#     python3 diffarith.py compare A.txt B.txt
#
# run compiles diffarith.cpp against each CMake build BUILD (see
# tests/tools/pow/build.py: configured with -DWITH_TESTS=ON and
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON) into DIR/LABEL/diffarith, writes its
# output to DIR/LABEL/diffarith.txt, and compares each output with the first
# one. compare compares two outputs of diffarith, however the programs were
# built.
#
# The comparison counts, for each operation, the lines that are identical,
# those that differ only in the sign of zero bounds (-0 for +0, which the
# FPU, SSE2 and NEON intervals do not all give, see doc/accuracy.md), and those
# that differ otherwise: a bound, an empty set, or an exception flag. It prints
# the first lines that differ otherwise, and exits with 1 when there is one: a
# difference in the sign of a zero alone is reported, not failed.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-19 by Jordan NININ

import argparse
import collections
import itertools
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
# build.py of the tools of pow, without leaving a __pycache__ in that directory
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.join(HERE, '..', 'pow'))
import build  # noqa: E402  (tests/tools/pow/build.py)

NEGATIVE_ZERO = '8000000000000000'


def unsigned_zero(token):
    return '0000000000000000' if token == NEGATIVE_ZERO else token


def compare(a_path, b_path, show=20):
    """Compares two outputs of diffarith, prints the counts, and returns whether they agree"""
    identical = collections.Counter()
    zero = collections.Counter()
    other = collections.Counter()
    shown = []
    with open(a_path) as fa, open(b_path) as fb:
        ha, hb = fa.readline(), fb.readline()
        print('%s: %s' % (a_path, ha.strip()))
        print('%s: %s' % (b_path, hb.strip()))
        if ha != hb:
            print('not the same diffarith (the boxes differ)')
            return False
        for n, (la, lb) in enumerate(itertools.zip_longest(fa, fb), 2):
            if la is None or lb is None:
                print('the outputs do not have the same number of lines')
                return False
            if la == lb:
                identical[la.split()[1]] += 1
                continue
            ta, tb = la.split(), lb.split()
            op = ta[1]
            if len(ta) == len(tb) and ta[:4] == tb[:4] \
                    and [unsigned_zero(t) for t in ta[4:]] == [unsigned_zero(t) for t in tb[4:]]:
                zero[op] += 1
            else:
                other[op] += 1
                if len(shown) < show:
                    shown.append('  line %d\n    %s\n    %s' % (n, la.strip(), lb.strip()))
    ops = sorted(set(identical) | set(zero) | set(other))
    print('%-10s %10s %10s %10s' % ('operation', 'identical', 'zero sign', 'different'))
    for op in ops:
        print('%-10s %10d %10d %10d' % (op, identical[op], zero[op], other[op]))
    print('%-10s %10d %10d %10d' % ('all', sum(identical.values()), sum(zero.values()), sum(other.values())))
    for s in shown:
        print(s)
    return not other


def run(out, specs):
    outputs = []
    for spec in specs:
        if '=' not in spec:
            sys.exit('diffarith.py: %s is not LABEL=BUILD' % spec)
        label, path = spec.split('=', 1)
        exe = os.path.join(out, label, 'diffarith')
        build.compile_tool(os.path.join(HERE, 'diffarith.cpp'), exe, build=path)
        txt = exe + '.txt'
        with open(txt, 'w') as f:
            subprocess.check_call([exe], stdout=f)
        outputs.append(txt)
    ok = True
    for txt in outputs[1:]:
        print()
        ok = compare(outputs[0], txt) and ok
    return ok


def main():
    p = argparse.ArgumentParser(description='The differential test of the arithmetic between builds of GAOL.')
    sub = p.add_subparsers(dest='command')
    r = sub.add_parser('run', help='build diffarith against each build, run it and compare the outputs')
    r.add_argument('--out', required=True, help='the directory of the programs and of their outputs')
    r.add_argument('builds', nargs='+', metavar='LABEL=BUILD', help='a name and a CMake build directory of GAOL')
    c = sub.add_parser('compare', help='compare two outputs of diffarith')
    c.add_argument('first')
    c.add_argument('second')
    c.add_argument('--show', type=int, default=20, help='the number of differing lines printed (20)')
    a = p.parse_args()
    if a.command == 'run':
        ok = run(a.out, a.builds)
    elif a.command == 'compare':
        ok = compare(a.first, a.second, a.show)
    else:
        p.print_help()
        sys.exit(2)
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
