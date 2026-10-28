#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: mutation testing of the port of cos.c to gaol_u128. Puts
# one deliberate error at a time in a line of 3rd/math-core/src/binary64/cos/
# cos.c that carries /* GAOL */ (or in the macro U128 of the halves), rebuilds
# the test core_math, runs it, and reports whether the checks of
# cos_accurate_path() (tests/core_math.cpp) fail: they have to.
#
#     python3 mutate.py --src DIR --build DIR [--jobs 2] [--only M01 M05 ...]
#
# DIR of --src is a COPY of the tree (never the working tree: the file is
# edited and put back), DIR of --build is a build of that copy with
# -DWITH_TESTS=ON, with -DGAOL_U128_EMULATION=ON to run the code of Visual C++
# and of the 32-bit targets. A mutant that survives is not always a gap of the
# test: see README.md for the two kinds.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-27 by Jordan NININ

import argparse
import os
import re
import subprocess
import sys

COS = os.path.join('3rd', 'math-core', 'src', 'binary64', 'cos', 'cos.c')

# name, description, text, replacement (the text has to be found once)
MUTANTS = [
    ('M01', 'the factor 8 of r: shl(r, 3) -> shl(r, 2)', 'gaol_u128_shl(r, 3)', 'gaol_u128_shl(r, 2)'),
    ('M02', 'exceptions: + 5 -> + 4', 'gaol_u128_lo(c1u) + 5', 'gaol_u128_lo(c1u) + 4'),
    ('M03', 'the sum of the combination made a difference',
     'c1u = gaol_u128_add(c1u, s1u); /* GAOL */', 'c1u = gaol_u128_sub(c1u, s1u); /* GAOL */'),
    ('M04', 'the comparison of c1u and s1u inverted', 'gaol_u128_lt(c1u, s1u)', 'gaol_u128_lt(s1u, c1u)'),
    ('M05', 'u128_tod(): the count of zeros of the low half', '__builtin_clzll (gaol_u128_hi(r))',
     '__builtin_clzll (gaol_u128_lo(r) | 1)'),
    ('M06', 'reduce_large_acc(): shr(v, 64) -> shr(v, 63)', 'gaol_u128_shr(v, 64)', 'gaol_u128_shr(v, 63)'),
    ('M07', 'reduce_large_acc(): the 1 of mask + 1 dropped', 'gaol_u128_sub(gaol_u128_add64(mask, 1), u)',
     'gaol_u128_sub(mask, u)'),
    ('M08', 's1u: the sum and the difference swapped',
     's1u = (i1 < 32) ? gaol_u128_add(s1u, t) : gaol_u128_sub(s1u, t);',
     's1u = (i1 < 32) ? gaol_u128_sub(s1u, t) : gaol_u128_add(s1u, t);'),
    ('M09', 'reduce_large_acc(): the mask of k + 1 dropped', 'k = (k+1) & ((1ull<<13)-1);', 'k = (k+1);'),
    ('M10', 'reduce_large_acc(): the halves of gaol_u128_make(V0, V1) swapped', 'gaol_u128_make(V0, V1)',
     'gaol_u128_make(V1, V0)'),
    ('M11', 'u128_tod(): the round bit, 74 - sh -> 73 - sh', '(int)(74 - sh)', '(int)(73 - sh)'),
    ('M12', 'reduce_large_acc(): the 13 leading bits not cleared', 'u = gaol_u128_and(u, mask);',
     'u = gaol_u128_and(u, gaol_u128_make(~0ull, ~0ull));'),
    ('M13', 'reduce_large_acc(): V2 takes bits of _T[i+2] rather than _T[i+3]', '(_T[i+3] >> (64-f))',
     '(_T[i+2] >> (64-f))'),
    ('M14', 'c1u: the sum and the difference swapped',
     'c1u = (i1 < 32) ? gaol_u128_sub(c1u, t) : gaol_u128_add(c1u, t);',
     'c1u = (i1 < 32) ? gaol_u128_add(c1u, t) : gaol_u128_sub(c1u, t);'),
    ('M15', 'u128_tod(): the shift of the 53 bits, 75 - sh -> 76 - sh', '(int)(75 - sh)', '(int)(76 - sh)'),
    ('M16', 'reduce_large_acc(): the shift by f always made', 'if (f != 0) {', 'if (1) {'),
    ('M17', 'the sign of the result not changed for i1 >= 32', 'sbit ^ (i1 >= 32)', 'sbit'),
    ('E01', 'the macro U128 of the halves: lo and hi swapped (-DGAOL_U128_EMULATION=ON only)',
     '#define U128(lo,hi) {.l = (lo), .h = (hi)}', '#define U128(lo,hi) {.l = (hi), .h = (lo)}'),
]


def build_and_run(build, jobs):
    r = subprocess.run(['cmake', '--build', build, '-j%d' % jobs, '--target', 'gaol_test_core_math'],
                       capture_output=True, text=True)
    if r.returncode != 0:
        return None, (r.stdout + r.stderr)[-400:]
    r = subprocess.run([os.path.join(build, 'tests', 'gaol_test_core_math')], capture_output=True, text=True)
    failed = {}
    for m in re.finditer(r'^(.*?)\s+(\d+) checks, (\d+) failed', r.stdout + r.stderr, re.M):
        if int(m.group(3)) > 0:
            failed[m.group(1).strip()] = int(m.group(3))
    return failed, ''


def main():
    parser = argparse.ArgumentParser(description='Mutation testing of the port of cos.c')
    parser.add_argument('--src', required=True, help='a COPY of the tree')
    parser.add_argument('--build', required=True, help='a build of that copy, with the tests')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--only', nargs='+', metavar='NAME')
    args = parser.parse_args()
    path = os.path.join(args.src, COS)
    original = open(path).read()
    survivors = 0
    try:
        for name, what, old, new in MUTANTS:
            if args.only and name not in args.only:
                continue
            if original.count(old) < 1:
                print('%s: the text to change is not in cos.c: %s' % (name, old))
                return 2
            open(path, 'w').write(original.replace(old, new, 1))
            failed, error = build_and_run(args.build, args.jobs)
            if failed is None:
                print('%s: the build failed: %s' % (name, error))
                continue
            caught = sum(v for k, v in failed.items() if k.startswith('cos on its accurate path'))
            others = sum(1 for k in failed if not k.startswith('cos on its accurate path'))
            if caught == 0:
                survivors += 1
            print('%s %-9s %s (%d checks of cos_accurate_path() failed, %d other kinds of checks)'
                  % (name, 'killed' if caught else 'SURVIVED', what, caught, others))
            sys.stdout.flush()
    finally:
        open(path, 'w').write(original)
        subprocess.run(['cmake', '--build', args.build, '-j%d' % args.jobs, '--target', 'gaol_test_core_math'],
                       capture_output=True)
    return 1 if survivors else 0


if __name__ == '__main__':
    sys.exit(main())
