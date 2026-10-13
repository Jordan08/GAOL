#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: mutation testing of pow. Each mutant is GAOL with one
# deliberate error in pow_standard_upward(), pow_hybrid_upward(),
# gaol_pow_real(), pow_lo(), pow_hi() or gaol_ieee1788::pow
# (gaol/gaol_interval.cpp); the tests have to fail on it, unless it computes
# the same bounds (an equivalent mutant, which the list says).
#
#     python3 mutate.py [--sources DIR] [--work DIR] [--cmake-option=OPT]...
#                       [--test NAME]... [--jobs N] [--no-diffpow]
#                       [--diffpow-randoms N] [--mutants FILE] [--list]
#                       [--fresh] [PATTERN...]
#
# The sources of GAOL (--sources, $GAOL_SOURCES, or by default the tree this
# script is in) are copied to WORK/src (--work, $GAOL_POW_MUTANTS, by default
# gaol-pow-mutants in the temporary directory, /tmp/gaol-pow-mutants on
# Linux), which are configured with CMake in WORK/build
# (-DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON, and each --cmake-option, such as
# --cmake-option=-DGAOL_SIMD=OFF), so that the tests are built with the flags
# of the build, and the sources given are never written. The tests
# gaol_test_NAME of each --test (by default ieee1788, with the boxes of
# pow_on_boxes(), and elementary, with pow over 385 boxes and at points; also
# arithmetic, rounding_direction...) are built and have to pass, and diffpow.cpp is linked
# with that build and run (build.py, diffpow.py). Then, for each mutant whose
# name contains one of the PATTERNs (all of them without one), the copy of
# the file is edited, the tests are rebuilt with --jobs processes (2 by
# default) and run, diffpow is run again if no test failed, and the file is
# restored. The copy is rebuilt from the sources at the end.
#
# For each mutant: KILLED (a test failed, crashed or ran over 300 s, with the
# number of failed checks of each name), "expected to survive" (the list says
# why its bounds are the same, and diffpow finds the same bounds as without
# it), SURVIVED (no test failed, and the list does not say why, or diffpow
# finds other bounds: it prints the first box where they differ, a box the
# tests lack), NOT APPLIED (a text to replace is not in the file once exactly:
# the list is out of date for the sources), or COMPILE ERROR. A mutant listed
# for the same bounds can be killed all the same, by a test that sees more
# than the bounds (the flags raised). WORK/mutants/NAME/ keeps the diff, the
# build log and the outputs of the tests and of diffpow. The exit status is 1
# for a mutant that survives, but those the list expects to, or one that
# cannot be applied or compiled.
#
# --mutants FILE takes the list from a Python file defining MUTANTS, a list
# of dict(name=..., edits=[(old, new), ...], file='gaol/gaol_interval.cpp',
# survives='why it is equivalent'), file and survives being optional; --list
# prints the mutants and whether they apply to the sources.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ

import argparse
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

import build
import diffpow

HERE = os.path.dirname(os.path.abspath(__file__))
FILE = 'gaol/gaol_interval.cpp'

# The mutants of the code of configure-clean on 2026-10-04 (after #61 and
# #63), their texts brought to the code of the pull requests of B: the
# integer powers in pown_upward(), the bounds in pow_rounded_dn() and
# pow_rounded_up(), the corners of every box, the limits of x^y where the
# base is 0 or +oo or the exponent infinite (pow_is_limit(), pow_limit()).
# Each edit replaces a text that has to be once in the file; survives says
# why the bounds of an equivalent mutant are those of the sources. B.1 of
# TODO.md removed the code that std-beyond-no-one-upper edited, an
# equivalent mutant, and kept the guard of [+oo] and [-oo], which an exponent
# built from an SSE2 register reaches; B.2 removed the conditions of the
# corners, their branch of a base from 0 and exp(y*log(base)), whose
# mutants (std-corners-*, std-corner-from-zero-upper, std-exp-log-of-x) went
# with them, and added the limits, whose mutants are corner-lower-no-limit,
# corner-upper-no-limit and corner-limit-*; the degenerate exponents beyond
# the ints take the corners, their block and its mutants (std-beyond-*)
# gone, and a base holding 1 inside takes an exponent of the other sign as 0
# (std-corner-around-1-*clamp*), and its limits where J holds 0 inside
# (std-corner-around-1-*skip*); a degenerate exponent takes one corner for
# each bound (std-degenerate-*), a base from 0 with exponents above 0 has a
# branch again (std-zero-base-*), and an exponent [+oo] or [-oo], which only
# an SSE2 register holds, gives the empty set (std-infinite-exponent-*).
MUTANTS = [
    # pow_standard_upward(): the pow of Table 9.1
    dict(name='std-empty-y-unchecked',
         edits=[('if (x.is_empty() || y.is_empty() || !(y.left() < GAOL_INFINITY',
                 'if (x.is_empty() || !(y.left() < GAOL_INFINITY')],
         survives='the NaN bounds of an empty y fail the test of an infinite exponent too, which gives the '
                  'empty set: the same bounds, but rounding_direction finds the invalid-operation flag that '
                  'comparing them raises'),
    dict(name='std-infinite-exponent-unchecked',
         edits=[('y.is_empty() || !(y.left() < GAOL_INFINITY && y.right() > -GAOL_INFINITY)) {',
                 'y.is_empty()) {')]),
    dict(name='std-no-cut-to-positive',
         edits=[('const interval base = x & interval::positive();', 'const interval base = x;')]),
    dict(name='std-empty-base-unchecked',
         edits=[('if (base.is_empty()) {\n      return interval::emptyset();\n    }\n    // pow(0,y)',
                 '// pow(0,y)')]),
    dict(name='std-no-zero-case',
         edits=[('if (base.right() == 0.0) {\n      return y.right() > 0.0',
                 'if (false) {\n      return y.right() > 0.0')]),
    dict(name='std-zero-case-y-ge-0',
         edits=[('return y.right() > 0.0 ? interval(0.0)', 'return y.right() >= 0.0 ? interval(0.0)')]),
    dict(name='std-zero-case-minus-zero',
         edits=[('return y.right() > 0.0 ? interval(0.0)', 'return y.right() > 0.0 ? interval::zero()')],
         survives='the lower bound is -0 rather than +0 with SSE2 intervals: the tests compare bounds with ==, '
                  'which a -0 passes'),
    dict(name='std-int-within-ints-by-corners',
         edits=[('if (y.is_an_int()) {\n      return pown_upward(', 'if (false) {\n      return pown_upward(')]),
    dict(name='std-int-within-ints-on-whole-x',
         edits=[('return pown_upward(base, static_cast<int>(y.left()));',
                 'return pown_upward(x, static_cast<int>(y.left()));')]),
    dict(name='std-degenerate-by-corners',
         edits=[('if (yl == yu) {', 'if (false) {')],
         survives='the corners of a degenerate exponent are those of the branch: the one at xl for the lower '
                  'bound and xu for the upper one for y > 0, and the reverse for y < 0, the clamped exponent 0 of '
                  'a base holding 1 giving 1, which changes neither'),
    dict(name='std-degenerate-lower-wrong-monotony',
         edits=[('l = pow_lo((yl > 0.0) ? xl : xu, yl);', 'l = pow_lo(xl, yl);')]),
    dict(name='std-degenerate-upper-wrong-monotony',
         edits=[('r = pow_hi((yl > 0.0) ? xu : xl, yl);', 'r = pow_hi(xu, yl);')]),
    dict(name='std-zero-base-by-corners',
         edits=[('if (xl == 0.0 && yl > 0.0) {', 'if (false) {')],
         survives='the corners give a base from 0 and exponents above 0 the bounds of the branch: the limit 0 '
                  'at (0, y), and the power at xu with yu for xu >= 1 and yl below'),
    dict(name='std-zero-base-upper-yu',
         edits=[('r = pow_hi(xu, (xu >= 1.0) ? yu : yl);', 'r = pow_hi(xu, yu);')]),
    dict(name='std-corner-above-1-lower',
         edits=[('l = pow_lo((yl >= 0.0) ? xl : xu, yl);', 'l = pow_lo(xl, yl);')]),
    dict(name='std-corner-above-1-upper',
         edits=[('r = pow_hi((yu >= 0.0) ? xu : xl, yu);', 'r = pow_hi(xu, yu);')]),
    dict(name='std-corner-below-1-lower',
         edits=[('l = pow_lo((yu >= 0.0) ? xl : xu, yu);', 'l = pow_lo(xl, yu);')]),
    dict(name='std-corner-below-1-upper',
         edits=[('r = pow_hi((yl >= 0.0) ? xu : xl, yl);', 'r = pow_hi(xu, yl);')]),
    dict(name='std-corner-around-1-lower-a',
         edits=[('minimum(pow_lo(xl, yu_or_0), pow_lo(xu, yl_or_0));', 'pow_lo(xl, yu_or_0);')]),
    dict(name='std-corner-around-1-lower-b',
         edits=[('minimum(pow_lo(xl, yu_or_0), pow_lo(xu, yl_or_0));', 'pow_lo(xu, yl_or_0);')]),
    dict(name='std-corner-around-1-upper-a',
         edits=[('maximum(pow_hi(xl, yl_or_0), pow_hi(xu, yu_or_0));', 'pow_hi(xl, yl_or_0);')]),
    dict(name='std-corner-around-1-upper-b',
         edits=[('maximum(pow_hi(xl, yl_or_0), pow_hi(xu, yu_or_0));', 'pow_hi(xu, yu_or_0);')]),
    dict(name='std-corner-around-1-no-skip',
         edits=[('const bool around_0 = yl < 0.0 && yu > 0.0;', 'const bool around_0 = false;')],
         survives='the limit 0 or +oo that bounds the box is also the bound computed at its corner: two calls '
                  'of CORE-MATH more, the same bounds'),
    dict(name='std-corner-around-1-lower-skip-always',
         edits=[('(around_0 && (pow_is_limit(xl, yu) || pow_is_limit(xu, yl)))', 'around_0')]),
    dict(name='std-corner-around-1-upper-skip-always',
         edits=[('(around_0 && (pow_is_limit(xl, yl) || pow_is_limit(xu, yu)))', 'around_0')]),
    dict(name='std-corner-around-1-lower-skip-one-sign',
         edits=[('(around_0 && (pow_is_limit(xl, yu) || pow_is_limit(xu, yl)))',
                 '(pow_is_limit(xl, yu) || pow_is_limit(xu, yl))')]),
    dict(name='std-corner-around-1-upper-skip-one-sign',
         edits=[('(around_0 && (pow_is_limit(xl, yl) || pow_is_limit(xu, yu)))',
                 '(pow_is_limit(xl, yl) || pow_is_limit(xu, yu))')]),
    dict(name='std-corner-around-1-no-clamp',
         edits=[('const double yu_or_0 = (yu > 0.0) ? yu : 0.0, yl_or_0 = (yl < 0.0) ? yl : 0.0;',
                 'const double yu_or_0 = yu, yl_or_0 = yl;')],
         survives='the four corners, whose least is at most 1 and greatest at least 1, I holding 1: the corner '
                  'of an exponent of the other sign, which the clamp takes as 1, changes neither; two calls of '
                  'CORE-MATH more where J has one sign'),
    dict(name='std-corner-around-1-clamp-upper-inverted',
         edits=[('(yu > 0.0) ? yu : 0.0', '(yu > 0.0) ? 0.0 : yu')]),
    dict(name='std-corner-around-1-clamp-lower-inverted',
         edits=[('(yl < 0.0) ? yl : 0.0', '(yl < 0.0) ? 0.0 : yl')]),
    # pow_lo() and pow_hi(): the bounds at a corner
    dict(name='corner-lower-not-exact',
         edits=[('return pow_is_double(x, y, d) ? d : maximum(pow_rounded_dn(x,y), 0.0);',
                 'return maximum(pow_rounded_dn(x,y), 0.0);')]),
    dict(name='corner-lower-no-one',
         edits=[('if (x == 1.0 || y == 0.0) {\n      return 1.0;\n    }\n    if (pow_is_limit(x, y)) {',
                 'if (pow_is_limit(x, y)) {')]),
    dict(name='corner-upper-no-one',
         edits=[('if (x == 1.0 || y == 0.0) {\n      return 1.0;\n    }\n    return pow_is_limit(x, y)',
                 'return pow_is_limit(x, y)')]),
    dict(name='corner-lower-no-limit',
         edits=[('    if (pow_is_limit(x, y)) {\n      return pow_limit(x, y);\n    }\n    return pow_is_double',
                 '    return pow_is_double')]),
    dict(name='corner-upper-no-limit',
         edits=[('return pow_is_limit(x, y) ? pow_limit(x, y) : pow_rounded_up(x,y);',
                 'return pow_rounded_up(x,y);')]),
    dict(name='corner-limit-inverted',
         edits=[('return ((x > 1.0) == (y > 0.0)) ? GAOL_INFINITY : 0.0;',
                 'return ((x > 1.0) == (y > 0.0)) ? 0.0 : GAOL_INFINITY;')]),
    dict(name='corner-limit-zero-base-finite',
         edits=[('return !(x > 0.0 && x < GAOL_INFINITY && std::fabs(y) < GAOL_INFINITY);',
                 'return !(x >= 0.0 && x < GAOL_INFINITY && std::fabs(y) < GAOL_INFINITY);')]),
    dict(name='corner-limit-infinite-base-finite',
         edits=[('return !(x > 0.0 && x < GAOL_INFINITY && std::fabs(y) < GAOL_INFINITY);',
                 'return !(x > 0.0 && std::fabs(y) < GAOL_INFINITY);')]),
    dict(name='corner-limit-infinite-exponent-finite',
         edits=[('return !(x > 0.0 && x < GAOL_INFINITY && std::fabs(y) < GAOL_INFINITY);',
                 'return !(x > 0.0 && x < GAOL_INFINITY);')],
         survives='pow_rounded_up() takes an infinite y apart: +oo, or 2^-1074 whose double below is the limit 0 '
                  'for pow_lo(); a pow_hi() of 2^-1074 rather than 0 bounds no box from above, and the double '
                  'below +oo, which pow_lo() then takes for a limit +oo, is a lower bound only for an exponent '
                  '[+oo] or [-oo], which only an SSE2 register holds'),
    dict(name='corner-power-of-two-not-integer',
         edits=[('if (t % 1024 != 0 || t < -1074*1024 || t > 1023*1024) {',
                 'if (t < -1074*1024 || t > 1023*1024) {')]),
    # pow_hybrid_upward(): gaol::pow(x, y)
    dict(name='gaol-empty-unchecked',
         edits=[('if (I.is_empty() || J.is_empty()) {\n      return interval::emptyset();\n    }\n'
                 '    // [+oo] and [-oo]',
                 '// [+oo] and [-oo]')]),
    dict(name='gaol-infinite-guard-dropped',
         edits=[('if (J.left() == J.right() && !(std::fabs(J.left()) <= (std::numeric_limits<double>::max)())) {',
                 'if (false) {')],
         survives='the tests reach it with the SSE2 intervals only (elementary: an exponent [+oo] built from an '
                  'SSE2 register); interval(+oo) and interval(-oo) are the empty set'),
    dict(name='gaol-no-pown',
         edits=[('return pown_upward(I,int(J.left()));', 'return pow_standard_upward(I, J);')]),
    dict(name='gaol-pown-on-positive',
         edits=[('return pown_upward(I,int(J.left()));', 'return pown_upward(I & interval::positive(),int(J.left()));')]),
    dict(name='gaol-beyond-ints-standard',
         edits=[('return interval::universe();\n    }\n    return pow_standard_upward(I, J);',
                 'return pow_standard_upward(I, J);\n    }\n    return pow_standard_upward(I, J);')]),
    # gaol_pow_real(): gaol::pow(x, p) for a double p
    dict(name='real-non-integer-standard',
         edits=[('res = pow_hybrid_upward(I, interval(p));', 'res = pow_standard_upward(I, interval(p));')]),
    dict(name='real-integer-standard',
         edits=[('res = pown_upward(I, static_cast<int>(p));', 'res = pow_standard_upward(I, interval(p));')]),
    # gaol_ieee1788::pow
    dict(name='ieee-is-gaol',
         edits=[('return ::gaol_core::pow_standard(x, y);', 'return ::gaol_core::gaol_pow_hybrid(x, y);')]),
]


def default_sources():
    root = os.path.normpath(os.path.join(HERE, '..', '..', '..'))
    return os.environ.get('GAOL_SOURCES', root)


def apply(text, edits):
    """The text with the edits made, or None and the first old text that is not in it once"""
    for old, new in edits:
        n = text.count(old)
        if n != 1:
            return None, '%r is in the file %d times' % (old[:70], n)
        text = text.replace(old, new)
    return text, None


def copy_sources(src, dst, work):
    """Copies the sources, without .git, the CMake build directories and the work directory. Only the files
    that differ from those of the copy are written, with the time of now: the build of the copy remakes what
    changed in the sources since the run before, and nothing else"""
    work = os.path.abspath(work)
    for d, dirs, files in os.walk(src):
        dirs[:] = [n for n in dirs if n != '.git' and os.path.abspath(os.path.join(d, n)) != work
                   and not os.path.exists(os.path.join(d, n, 'CMakeCache.txt'))]
        target = os.path.join(dst, os.path.relpath(d, src))
        os.makedirs(target, exist_ok=True)
        for n in files:
            a, b = os.path.join(d, n), os.path.join(target, n)
            if os.path.islink(a):
                continue
            if os.path.exists(b) and os.path.getsize(a) == os.path.getsize(b):
                with open(a, 'rb') as fa, open(b, 'rb') as fb:
                    if fa.read() == fb.read():
                        continue
            shutil.copyfile(a, b)
            shutil.copymode(a, b)


def build_targets(work, targets, jobs, log):
    with open(log, 'w') as f:
        r = subprocess.run(['cmake', '--build', os.path.join(work, 'build'), '-j', str(jobs), '--target'] + targets,
                           stdout=f, stderr=subprocess.STDOUT)
    return r.returncode == 0


def run_test(work, test, out):
    """(passed, {check name: failures}, total line) of one run of gaol_test_test"""
    exe = os.path.join(work, 'build', 'tests', 'gaol_test_' + test)
    try:
        r = subprocess.run([exe], cwd=os.path.dirname(exe), stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           timeout=300)
        text = r.stdout.decode(errors='replace')
        code = r.returncode
    except subprocess.TimeoutExpired:
        text, code = 'over 300 s\n', 'timeout'
    open(out, 'w').write(text)
    failed = {}
    for l in text.splitlines():
        m = re.match(r'^(.*?)\s+(\d+) checks, (\d+) failed$', l)  # the line of each name of check
        if m and int(m.group(3)):
            failed[m.group(1)] = int(m.group(3))
    total = [l for l in text.splitlines() if re.match(r'^\d+ checks, \d+ failed$', l)]
    summary = total[-1] if total else ('exit %s' % code)
    return code == 0, failed, summary


def run_diffpow(work, out, randoms):
    """Links diffpow.cpp with the library of the work build as it is, and runs it into out"""
    build.VERBOSE = False
    exe = os.path.join(os.path.dirname(out), 'diffpow')
    build.compile_tool(os.path.join(HERE, 'diffpow.cpp'), exe, build=os.path.join(work, 'build'))
    with open(out, 'w') as f:
        subprocess.check_call([exe, str(randoms)], stdout=f)


def main():
    p = argparse.ArgumentParser(description='Mutation testing of the pow of GAOL.')
    p.add_argument('--sources', default=default_sources(), help='the sources of GAOL, copied (%(default)s)')
    p.add_argument('--work', default=os.environ.get('GAOL_POW_MUTANTS',
                                                    os.path.join(tempfile.gettempdir(), 'gaol-pow-mutants')),
                   help='the work directory: the copy, its build and the logs (%(default)s)')
    p.add_argument('--cmake-option', action='append', default=[], metavar='OPT',
                   help='an option of the CMake configuration, written --cmake-option=-DGAOL_SIMD=OFF')
    p.add_argument('--test', action='append', metavar='NAME',
                   help='a test of tests/ to run, in place of ieee1788 and elementary')
    p.add_argument('--jobs', type=int, default=2, help='the processes of each build (2)')
    p.add_argument('--no-diffpow', action='store_true', help='do not run diffpow on the mutants that survive')
    p.add_argument('--diffpow-randoms', type=int, default=60000, help='the random boxes of diffpow (60000)')
    p.add_argument('--mutants', metavar='FILE', help='a Python file defining MUTANTS, in place of the list here')
    p.add_argument('--list', action='store_true', help='print the mutants and whether they apply, and stop')
    p.add_argument('--fresh', action='store_true', help='remove the work directory first')
    p.add_argument('patterns', nargs='*', metavar='PATTERN', help='the mutants whose name contains one of them')
    a = p.parse_args()

    mutants = MUTANTS
    if a.mutants:
        scope = {}
        exec(compile(open(a.mutants).read(), a.mutants, 'exec'), scope)
        mutants = scope['MUTANTS']
    if a.patterns:
        mutants = [m for m in mutants if any(s in m['name'] for s in a.patterns)]
    tests = a.test or ['ieee1788', 'elementary']
    targets = ['gaol_test_' + t for t in tests]
    if not os.path.exists(os.path.join(a.sources, FILE)):
        sys.exit('mutate.py: %s is not the sources of GAOL (no %s)' % (a.sources, FILE))

    if a.list:
        for m in mutants:
            text = open(os.path.join(a.sources, m.get('file', FILE))).read()
            _, why = apply(text, m['edits'])
            print('%-34s %s%s' % (m['name'], 'applies' if why is None else 'NOT APPLIED: ' + why,
                                  ' (expected to survive)' if m.get('survives') else ''))
        return

    work = os.path.abspath(a.work)
    if a.fresh and os.path.exists(work):
        shutil.rmtree(work)
    src = os.path.join(work, 'src')
    os.makedirs(os.path.join(work, 'mutants'), exist_ok=True)
    copy_sources(a.sources, src, work)
    files = sorted(set(m.get('file', FILE) for m in mutants))
    originals = dict((f, open(os.path.join(src, f)).read()) for f in files)
    for f in files:
        os.utime(os.path.join(src, f))  # rebuilt from the sources, whatever a run before left
    cmd = ['cmake', '-S', src, '-B', os.path.join(work, 'build'), '-DCMAKE_BUILD_TYPE=Release', '-DWITH_TESTS=ON',
           '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'] + a.cmake_option
    with open(os.path.join(work, 'configure.log'), 'w') as f:
        if subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT).returncode != 0:
            sys.exit('mutate.py: the configuration failed, see %s/configure.log' % work)
    if not build_targets(work, targets, a.jobs, os.path.join(work, 'build.log')):
        sys.exit('mutate.py: the build of the sources failed, see %s/build.log' % work)
    for t in tests:
        ok, failed, summary = run_test(work, t, os.path.join(work, 'test-%s.txt' % t))
        if not ok:
            sys.exit('mutate.py: %s fails without a mutant (%s), see %s/test-%s.txt' % (t, summary, work, t))
        print('without a mutant: %s passes (%s)' % (t, summary))
    base = os.path.join(work, 'diffpow', 'diffpow.txt')
    if not a.no_diffpow:
        os.makedirs(os.path.dirname(base), exist_ok=True)
        run_diffpow(work, base, a.diffpow_randoms)

    counts = {}
    bad = []
    start = time.time()
    try:
        for m in mutants:
            f = m.get('file', FILE)
            d = os.path.join(work, 'mutants', m['name'])
            os.makedirs(d, exist_ok=True)
            text, why = apply(originals[f], m['edits'])
            more = ''
            if text is None:
                outcome, detail = 'NOT APPLIED', why
            else:
                open(os.path.join(d, 'diff'), 'w').writelines(difflib.unified_diff(
                    originals[f].splitlines(True), text.splitlines(True), 'a/' + f, 'b/' + f))
                open(os.path.join(src, f), 'w').write(text)
                if not build_targets(work, targets, a.jobs, os.path.join(d, 'build.log')):
                    outcome, detail = 'COMPILE ERROR', 'see ' + os.path.join(d, 'build.log')
                else:
                    killed, parts = False, []
                    for t in tests:
                        ok, failed, summary = run_test(work, t, os.path.join(d, 'test-%s.txt' % t))
                        if not ok:
                            killed = True
                            names = '; '.join('%s: %d' % (k[:60], v) for k, v in sorted(failed.items()))
                            parts.append('%s %s%s' % (t, summary, (' (' + names + ')') if names else ''))
                    if killed:
                        outcome, detail = 'KILLED', ', '.join(parts)
                        if m.get('survives'):
                            detail += '; listed for the same bounds: ' + m['survives']
                    else:
                        outcome = 'expected to survive' if m.get('survives') else 'SURVIVED'
                        detail = m.get('survives') or 'no test failed'
                        if not a.no_diffpow:
                            run_diffpow(work, os.path.join(d, 'diffpow.txt'), a.diffpow_randoms)
                            r = diffpow.differences(base, os.path.join(d, 'diffpow.txt'))
                            n = len(r['a'])
                            if r['other_lines']:
                                outcome = 'SURVIVED'
                                detail += '; diffpow: other bounds than the sources on %d of %d lines, first:' % (
                                    len(r['other_lines']), n)
                                more = diffpow.describe(r, r['other_lines'][0])
                            elif r['zero_lines']:
                                detail += '; diffpow: the same bounds on its %d lines, but for the sign of ' \
                                          'zero bounds on %d' % (n, len(r['zero_lines']))
                            else:
                                detail += '; diffpow: the same bounds on its %d lines' % n
                open(os.path.join(src, f), 'w').write(originals[f])
            counts[outcome] = counts.get(outcome, 0) + 1
            if outcome in ('NOT APPLIED', 'COMPILE ERROR', 'SURVIVED'):
                bad.append(m['name'])
            print('%-34s %-20s %s' % (m['name'], outcome, detail), flush=True)
            if more:
                print(more, flush=True)
    finally:
        for f in files:
            open(os.path.join(src, f), 'w').write(originals[f])
        build_targets(work, targets, a.jobs, os.path.join(work, 'build.log'))
    print('mutants: %d in %.0f s; %s' % (len(mutants), time.time() - start,
                                          ', '.join('%s %d' % kv for kv in sorted(counts.items()))))
    if bad:
        print('to look at: ' + ', '.join(bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
