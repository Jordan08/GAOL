#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: compiles one of the programs of tests/tools/pow
# (diffpow.cpp, evalrows.cpp, bench.cpp) against a build of GAOL, with the
# compiler and the flags that build compiles its own tests with.
#
# The GAOL is given in one of three ways:
#
# - a CMake build directory (--build DIR), configured with
#   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON: the program is compiled with the
#   command compile_commands.json gives for tests/ieee1788.cpp (compiler,
#   include directories, definitions, C++ standard and the flags of interval
#   arithmetic), and linked with the line of
#   tests/CMakeFiles/gaol_test_ieee1788.dir/link.txt, the library of that
#   build in place of the test (the Unix Makefiles generator writes it). With
#   another generator, which writes no link.txt, the program is linked with
#   the compiler of the compile command and the libgaol of DIR;
# - an installed GAOL (--pkg-config), with the compiler $CXX (c++ by
#   default), the flags $CXXFLAGS (-O2 -std=c++17 by default) and
#   pkg-config --cflags --libs gaol ($PKG_CONFIG_PATH finds gaol.pc);
# - explicit flags (--cxx, --cxxflags, --ldflags).
#
#     python3 tests/tools/pow/build.py --build build-sse -o /tmp/pow/sse/bench tests/tools/pow/bench.cpp
#     PKG_CONFIG_PATH=/opt/gaol/lib/pkgconfig python3 tests/tools/pow/build.py --pkg-config \
#         -o /tmp/pow/installed/bench tests/tools/pow/bench.cpp
#
# With --build, the object file is written next to the program (PROGRAM.o).
# diffpow.py and mutate.py import it to build diffpow once per build of GAOL.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-04 by Jordan NININ

import argparse
import json
import os
import shlex
import subprocess
import sys

# The test whose compile command and link line the programs take: it is
# compiled as a program using gaol_ieee1788 and gaol, in C++17
REFERENCE = 'tests/ieee1788.cpp'
REFERENCE_TARGET = 'gaol_test_ieee1788'


# Whether the commands are printed (mutate.py runs them quietly)
VERBOSE = True


def run(cmd, cwd=None):
    """Runs cmd (a list), printing it, and stops on a failure"""
    line = ' '.join(shlex.quote(c) for c in cmd)
    if VERBOSE:
        print('+ ' + line, file=sys.stderr)
        r = subprocess.run(cmd, cwd=cwd)
    else:
        r = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if r.returncode != 0:
        if not VERBOSE:
            sys.stderr.write(line + '\n' + r.stdout.decode(errors='replace'))
        sys.exit('build.py: the command failed (exit %d)' % r.returncode)


def library_of(build):
    """The libgaol a CMake build directory made"""
    for name in sorted(os.listdir(build)):
        if name.startswith('libgaol.') and (name.endswith('.a') or '.so' in name or name.endswith('.dylib')):
            return os.path.join(build, name)
    if os.path.exists(os.path.join(build, 'gaol.lib')):
        return os.path.join(build, 'gaol.lib')
    sys.exit('build.py: no libgaol in %s' % build)


def from_cmake(build, source, output):
    """Compiles and links source as the CMake build of GAOL in build compiles its tests"""
    build = os.path.abspath(build)
    db = os.path.join(build, 'compile_commands.json')
    if not os.path.exists(db):
        sys.exit('build.py: %s is missing: configure the build with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON' % db)
    entry = None
    for e in json.load(open(db)):
        if os.path.normpath(e['file']).replace('\\', '/').endswith(REFERENCE):
            entry = e
    if entry is None:
        sys.exit('build.py: no command for %s in %s: configure the build with -DWITH_TESTS=ON' % (REFERENCE, db))
    args = entry['arguments'] if 'arguments' in entry else shlex.split(entry['command'])
    flags = []
    skip = False
    for a in args[1:]:
        if skip:
            skip = False
            continue
        if a == '-o':
            skip = True
            continue
        if a == '-c' or os.path.normpath(os.path.join(entry['directory'], a)) == os.path.normpath(entry['file']):
            continue
        flags.append(a)
    obj = os.path.abspath(output) + '.o'
    run([args[0]] + flags + ['-c', os.path.abspath(source), '-o', obj], cwd=entry['directory'])

    link_txt = os.path.join(build, 'tests', 'CMakeFiles', REFERENCE_TARGET + '.dir', 'link.txt')
    if os.path.exists(link_txt):
        # Run from tests/ of the build, where its relative paths start
        tokens = shlex.split(open(link_txt).read().strip().splitlines()[0])
        cmd = []
        skip = False
        for i, t in enumerate(tokens):
            if skip:
                skip = False
                continue
            if t == '-o':
                cmd += ['-o', os.path.abspath(output)]
                skip = True
            elif t.endswith('.o') or t.endswith('.obj'):
                if obj not in cmd:
                    cmd.append(obj)
            else:
                cmd.append(t)
        run(cmd, cwd=os.path.join(build, 'tests'))
    else:
        lib = library_of(build)
        extra = ['-Wl,-rpath,' + build] if ('.so' in lib or lib.endswith('.dylib')) else []
        run([args[0]] + [f for f in flags if f.startswith(('-O', '-m', '-f', '-std'))]
            + [obj, '-o', os.path.abspath(output), lib] + extra)


def from_pkg_config(source, output, name='gaol'):
    """Compiles source with $CXX, $CXXFLAGS and the flags pkg-config gives for an installed GAOL"""
    cxx = shlex.split(os.environ.get('CXX', 'c++'))
    cxxflags = shlex.split(os.environ.get('CXXFLAGS', '-O2 -std=c++17'))
    try:
        cflags = shlex.split(subprocess.check_output(['pkg-config', '--cflags', name]).decode())
        libs = shlex.split(subprocess.check_output(['pkg-config', '--libs', name]).decode())
    except (OSError, subprocess.CalledProcessError):
        sys.exit('build.py: pkg-config does not find %s (set PKG_CONFIG_PATH)' % name)
    run(cxx + cxxflags + cflags + [source, '-o', output] + libs)


def from_flags(source, output, cxx, cxxflags, ldflags):
    run(shlex.split(cxx) + shlex.split(cxxflags) + [source, '-o', output] + shlex.split(ldflags))


def compile_tool(source, output, build=None, pkg_config=False, cxx=None, cxxflags='', ldflags=''):
    """Compiles source into the program output, against the GAOL given by one of build, pkg_config, cxx"""
    d = os.path.dirname(os.path.abspath(output))
    if not os.path.isdir(d):
        os.makedirs(d)
    if build:
        from_cmake(build, source, output)
    elif pkg_config:
        from_pkg_config(source, output)
    elif cxx:
        from_flags(source, output, cxx, cxxflags, ldflags)
    else:
        sys.exit('build.py: give a CMake build (--build), --pkg-config, or --cxx')


def add_arguments(p):
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument('--build', metavar='DIR', help='a CMake build of GAOL, configured with -DWITH_TESTS=ON '
                   'and -DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
    g.add_argument('--pkg-config', action='store_true', help='an installed GAOL that pkg-config finds')
    g.add_argument('--cxx', help='the compiler, with --cxxflags and --ldflags')
    p.add_argument('--cxxflags', default='', help='with --cxx: the flags, -I of GAOL\'s headers included')
    p.add_argument('--ldflags', default='', help='with --cxx: the libraries, libgaol included')


def main():
    p = argparse.ArgumentParser(description='Compiles a program of tests/tools/pow against a build of GAOL.')
    add_arguments(p)
    p.add_argument('-o', dest='output', required=True, help='the program to write')
    p.add_argument('source', help='diffpow.cpp, evalrows.cpp or bench.cpp')
    a = p.parse_args()
    compile_tool(a.source, a.output, a.build, a.pkg_config, a.cxx, a.cxxflags, a.ldflags)


if __name__ == '__main__':
    main()
