"""GAOL built in release by default, brought into another project (GAOL v5).

    python3 release_flags.py [--debug] <build directory> <sources of GAOL>

Fails unless each command of the compile_commands.json of the build directory
that compiles a source of GAOL or of CORE-MATH (<sources>/gaol/,
<sources>/3rd/) optimizes as the release build does (its last -O is -O3, or
/O2 with Visual C++) and defines NDEBUG, and unless the gaol_configuration.h
of the build leaves GAOL_DEBUGGING undefined: a project that brings GAOL in
without choosing a build type, as a CMake project without CMAKE_BUILD_TYPE
(add_subdirectory, FetchContent) or a meson project in the default build type
of meson (a subproject), gets GAOL in release, as configure builds it.
tests/meson_subproject runs it, and the continuous integration on
tests/fetch_content (.github/workflows/linux.yml).

With --debug, the other way round, for GAOL_DEBUG of CMake and enable-debug of
meson: no optimization (no -O, or a last -O0 or /Od), NDEBUG not defined in
the end (no -DNDEBUG, or a -UNDEBUG after it), and GAOL_DEBUGGING defined, by
gaol_configuration.h or on the command line.

Copyright (c) 2026 ENSTA, France

Created 2026-10-06 by Jordan NININ
"""
import json
import os
import re
import shlex
import sys

args = sys.argv[1:]
debug = args[:1] == ['--debug']
if debug:
    args = args[1:]
build, sources = args[0], os.path.normpath(args[1])
roots = tuple(os.path.join(sources, d) + os.sep for d in ('gaol', '3rd'))
with open(os.path.join(build, 'compile_commands.json')) as db:
    commands = json.load(db)
headers = [os.path.join(d, f) for d, _, files in os.walk(build) for f in files
           if f == 'gaol_configuration.h']
header_debugging = False
for header in headers:
    with open(header) as h:
        if re.search(r'^\s*#\s*define\s+GAOL_DEBUGGING\b', h.read(), re.M):
            header_debugging = True
checked = 0
failed = 0
for command in commands:
    path = os.path.normpath(os.path.join(command['directory'], command['file']))
    if not path.startswith(roots):
        continue
    checked += 1
    tokens = shlex.split(command['command'])
    levels = [t for t in tokens if re.match(r'^[-/]O([0-3sgdxz]|fast)?$', t)]
    level = levels[-1] if levels else None
    ndebug = [t for t in tokens if re.match(r'^[-/][DU]NDEBUG(=1)?$', t)]
    ndebug = bool(ndebug) and ndebug[-1][1] == 'D'
    debugging = header_debugging or any(
        re.match(r'^[-/]DGAOL_DEBUGGING(=1)?$', t) for t in tokens)
    problems = []
    if debug:
        if level not in (None, '-O0', '/Od'):
            problems.append('optimization %s' % level)
        if ndebug:
            problems.append('NDEBUG')
        if not debugging:
            problems.append('no GAOL_DEBUGGING')
    else:
        if level not in ('-O3', '/O2'):
            problems.append('optimization %s' % (level or 'none'))
        if not ndebug:
            problems.append('no NDEBUG')
        if debugging:
            problems.append('GAOL_DEBUGGING')
    if problems:
        failed += 1
        print('%s: %s' % (command['file'], ', '.join(problems)))
print('%d command(s) of GAOL and CORE-MATH checked for %s, '
      '%d gaol_configuration.h'
      % (checked, 'debug' if debug else 'release', len(headers)))
sys.exit(1 if failed or not checked or not headers else 0)
