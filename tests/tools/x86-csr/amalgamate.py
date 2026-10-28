#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: one source file made of three units of GAOL (the core of
# the intervals, the common code and the test of the rounding direction), for a
# compiler that is not here and takes one file, such as the ones of Compiler
# Explorer (https://godbolt.org): Visual C++ for x86 and x64, clang-cl, a GCC
# or a Clang for another target. The file is meant to be compiled, not linked
# (the parser, the port, CORE-MATH... are not in it), with a configuration
# written by hand. Nothing is run by this script.
#
#     python3 amalgamate.py [--root DIR] [--define NAME[=VALUE]]...
#                           [--output FILE] [UNIT...]
#
# Each UNIT (by default gaol/core/gaol_interval_core.cpp, gaol/gaol_common.cpp
# and tests/rounding_direction.cpp, relative to the root of the sources, by
# default the tree this script is in) is copied in turn, and each
#
#     #include "path"
#
# is replaced by the file, found relative to the root, to gaol/, to tests/ or
# to the including file. A file that guards itself against a second inclusion
# (#ifndef and #define, or #pragma once at its start) is put in once, the
# first time it is met outside every #if: met under an #if that may be false
# (the intervals of floats, an inactive path), it would be missing where it is
# needed next, so each such occurrence has its copy, and the guard of the copy
# keeps the compiler from reading it twice. The includes of the system (<...>)
# are kept. The file
# gaol/gaol_configuration.h, which the builds write, is written from
# cmake/gaol_configuration.h.in: each macro the template names with
# #cmakedefine is defined to 1 if a --define says so (--define
# GAOL_USING_SSE2_INSTRUCTIONS --define GAOL_USING_SSE3_INSTRUCTIONS for the
# SSE2 intervals, --define GAOL_PRESERVE_ROUNDING, --define GAOL_HAVE_FENV_H,
# and --define NAME=VALUE for another value), and left undefined otherwise; a
# name that the template does not know is an error.
# GAOL_HAVE_FENV_H is defined, as the three builds do, unless --define
# GAOL_HAVE_FENV_H=0 says otherwise.
#
# A Compiler Explorer compiles for the target the compiler is for; give the
# flags of the builds (gaol/gaol_config.h refuses a target that does not
# compute its doubles in the rounding direction GAOL asks for, and a compiler
# that is not told so): -std=c++17 (the test writes hexadecimal floating
# literals) -O2 -frounding-math -ffp-contract=off -msse2 -mfpmath=sse for GCC
# and Clang on x86, /std:c++17 /O2 /fp:strict with Visual C++; and
# -faligned-new=8 for GCC and Clang on 32-bit Windows with the SSE2 intervals
# (gaol/gaol_config.h refuses them without it). The result is large (the three
# units, 33 000 lines), and a Compiler Explorer stops a compilation at about 20
# seconds.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-27 by Jordan NININ (the work of #156 had such a script,
# which was lost, rewritten)

import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
UNITS = ['gaol/core/gaol_interval_core.cpp', 'gaol/gaol_common.cpp', 'tests/rounding_direction.cpp']
INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"')


def configuration(root, defines):
    """gaol/gaol_configuration.h as the template of CMake gives it for the macros defined"""
    version = open(os.path.join(root, 'VERSION.txt')).read().strip()
    major, minor, patch = (version.split('.') + ['0', '0'])[:3]
    text = open(os.path.join(root, 'cmake', 'gaol_configuration.h.in')).read()
    text = (text.replace('@PROJECT_VERSION_MAJOR@', major).replace('@PROJECT_VERSION_MINOR@', minor)
                .replace('@PROJECT_VERSION_PATCH@', patch).replace('@PROJECT_VERSION@', version))

    def define(m):
        name = m.group(1)
        if name in defines:
            return '#define %s %s' % (name, defines[name])
        return '/* #undef %s */' % name
    return re.sub(r'^#cmakedefine ([A-Za-z0-9_]+) 1$', define, text, flags=re.M)


class Amalgam:
    def __init__(self, root, defines):
        self.root = root
        self.defines = defines
        self.done = set()
        self.out = []

    def find(self, name, including):
        for d in (os.path.dirname(including), self.root, os.path.join(self.root, 'gaol'),
                  os.path.join(self.root, 'tests')):
            p = os.path.normpath(os.path.join(d, name))
            if os.path.isfile(p):
                return p
        return None

    @staticmethod
    def is_pragma_once(text):
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
        text = re.sub(r'//[^\n]*', '', text)
        lines = [l.strip() for l in text.splitlines() if l.strip()]
        return lines[:1] == ['#pragma once']

    @staticmethod
    def guarded(text):
        """Whether the file starts with #pragma once, or #ifndef X followed by #define X"""
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
        text = re.sub(r'//[^\n]*', '', text)
        lines = [l.strip() for l in text.splitlines() if l.strip()]
        if lines[:1] == ['#pragma once']:
            return True
        return (len(lines) > 1 and re.match(r'#\s*ifndef\s+(\w+)$', lines[0]) is not None
                and re.sub(r'\s+', ' ', lines[1]).replace('# ', '#') == '#define ' + lines[0].split()[-1])

    def add(self, path, certain=True):
        """The file with its quoted includes replaced; certain is whether it is read for sure (outside every #if)"""
        path = os.path.normpath(path)
        text = open(path, encoding='utf-8').read()
        guarded = self.guarded(text)
        pragma_once = guarded and self.is_pragma_once(text)
        if guarded and path in self.done:
            return
        if guarded and certain:
            self.done.add(path)
        self.out.append('// ---- %s\n' % os.path.relpath(path, self.root))
        depth = 0           # the #if that are open, the guard of the file itself apart
        guard_open = guarded and not pragma_once    # the first #ifndef of a file with a guard is the guard
        for line in text.splitlines(True):
            d = re.match(r'^\s*#\s*(if|ifdef|ifndef|endif)\b', line)
            if d:
                if d.group(1) == 'endif':
                    depth = max(depth - 1, 0)
                elif guard_open:
                    guard_open = False      # the first #ifndef of a guarded file is its guard
                else:
                    depth += 1
            m = INCLUDE.match(line)
            if not m:
                self.out.append(line)
                continue
            name = m.group(1)
            if os.path.normpath(name) == os.path.normpath('gaol/gaol_configuration.h'):
                key = '<configuration>'
                if key not in self.done:
                    self.done.add(key)
                    self.out.append('// ---- gaol/gaol_configuration.h, from cmake/gaol_configuration.h.in\n')
                    self.out.append(configuration(self.root, self.defines) + '\n')
                continue
            found = self.find(name, path)
            if found is None:
                sys.exit('amalgamate.py: %s includes "%s", which is nowhere' % (path, name))
            self.add(found, certain and depth == 0)
        if self.out and not self.out[-1].endswith('\n'):
            self.out.append('\n')


def main():
    p = argparse.ArgumentParser(description='One source file from the sources of GAOL.')
    p.add_argument('--root', default=os.path.normpath(os.path.join(HERE, '..', '..', '..')),
                   help='the sources of GAOL (%(default)s)')
    p.add_argument('--define', action='append', default=[], metavar='NAME[=VALUE]',
                   help='a macro of gaol_configuration.h to define (to 1 without a value)')
    p.add_argument('--output', metavar='FILE', help='where to write it (the standard output)')
    p.add_argument('units', nargs='*', metavar='UNIT', help='the sources to put in it (%s)' % ', '.join(UNITS))
    a = p.parse_args()

    defines = {'GAOL_HAVE_FENV_H': '1'}
    known = set(re.findall(r'^#cmakedefine ([A-Za-z0-9_]+) 1$',
                           open(os.path.join(a.root, 'cmake', 'gaol_configuration.h.in')).read(), flags=re.M))
    for d in a.define:
        name, _, value = d.partition('=')
        if name not in known:
            sys.exit('amalgamate.py: %s is no macro of cmake/gaol_configuration.h.in (%s)' % (name, ', '.join(sorted(known))))
        defines[name] = value or '1'
    amalgam = Amalgam(os.path.abspath(a.root), defines)
    for unit in a.units or UNITS:
        amalgam.add(os.path.join(amalgam.root, unit))
    text = ''.join(amalgam.out)
    if a.output:
        open(a.output, 'w', encoding='utf-8').write(text)
    else:
        sys.stdout.write(text)


if __name__ == '__main__':
    main()
