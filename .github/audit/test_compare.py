"""test_compare.py: compare.py --check on made-up results of the audit: it has to pass for three builds that agree,
and fail for a build whose probe of the C sources failed (probe_build.py), and for builds that differ."""

# Copyright (c) 2026 ENSTA, France
#
# By: Jordan NININ (created 2026-11-14)
#
# GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING.LIB file for information.

import copy, json, os, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
FLAGS = ['-O3', '-std=c++11', '-frounding-math', '-ffp-contract=off', '-fno-fast-math', '-msse2']
C_FLAGS = ['-O3', '-std=gnu99', '-frounding-math', '-ffp-contract=off', '-fno-fast-math', '-msse2']


def build():
    return {
        'compiler': 'g++', 'version': 'g++ 15.2.0', 'flags': list(FLAGS),
        'macros': {'GAOL_VERSION': '"5.0.0"', '__cplusplus': '201103L'},
        'ifs': {'GAOL_PRESERVE_ROUNDING': False, 'GAOL_USING_SSE2_INSTRUCTIONS': True},
        'configuration': {'GAOL_USING_SSE2_INSTRUCTIONS': '1'},
        'c': {'compiler': 'gcc', 'flags': list(C_FLAGS), 'macros': {'__STDC_VERSION__': '199901L'},
              'ifs': {'__SSE2__': True}},
    }


def result():
    return {'label': 'test', 'cmake': build(), 'autotools': build(), 'meson': build()}


def run(r):
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, 'test.json')
        json.dump(r, open(path, 'w'))
        p = subprocess.run([sys.executable, os.path.join(here, 'compare.py'), '--check', path],
                           capture_output=True, text=True)
    return p.returncode, p.stdout


failures = 0


def expect(title, r, status):
    global failures
    got, out = run(r)
    if got != status:
        failures += 1
        print('FAILED: %s: exit status %d, expected %d\n%s' % (title, got, status, out))
    else:
        print('ok: %s' % title)


expect('three builds that agree', result(), 0)

# The probe of the C sources failed: probe_build.py leaves the error and no macros...
r = result()
r['autotools']['c'] = {'compiler': 'gcc', 'flags': list(C_FLAGS), 'error': 'probe: probe.c: error: unknown type name'}
expect('a failed probe of the C sources, no macros', r, 1)
# ... and an error next to the macros that an older probe_build.py read from nothing, the same
# nothing for the three builds, which then agreed
r = result()
for b in ('cmake', 'autotools', 'meson'):
    r[b]['c'] = {'compiler': 'gcc', 'flags': list(C_FLAGS), 'error': 'probe failed', 'macros': {}, 'ifs': {}}
expect('failed probes of the C sources of the three builds, empty macros', r, 1)

r = result()
r['meson']['error'] = 'probe failed'
expect('a failed probe of the C++ sources', r, 1)

r = result()
r['meson']['ifs']['GAOL_PRESERVE_ROUNDING'] = True
expect('a macro that differs', r, 1)

r = result()
r['cmake']['c']['flags'].remove('-msse2')
expect('a flag of the C sources that differs', r, 1)

r = result()
r['cmake'] = {'error': 'CMake Error: no compiler'}
expect('a build that did not configure', r, 1)

sys.exit(1 if failures else 0)
