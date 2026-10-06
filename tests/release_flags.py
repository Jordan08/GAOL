"""GAOL built in release by default, brought into another project (GAOL v5).

    python3 release_flags.py <build directory> <sources of GAOL>

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

Copyright (c) 2026 ENSTA, France

Created 2026-10-06 by Jordan NININ
"""
import json
import os
import re
import shlex
import sys

build, sources = sys.argv[1], os.path.normpath(sys.argv[2])
roots = tuple(os.path.join(sources, d) + os.sep for d in ('gaol', '3rd'))
with open(os.path.join(build, 'compile_commands.json')) as db:
    commands = json.load(db)
checked = 0
failed = 0
for command in commands:
    path = os.path.normpath(os.path.join(command['directory'], command['file']))
    if not path.startswith(roots):
        continue
    checked += 1
    tokens = shlex.split(command['command'])
    levels = [t for t in tokens if re.match(r'^[-/]O([0-3sgdxz]|fast)?$', t)]
    problems = []
    if not levels or levels[-1] not in ('-O3', '/O2'):
        problems.append('optimization %s' % (levels[-1] if levels else 'none'))
    if not ({'-DNDEBUG', '/DNDEBUG', '-DNDEBUG=1', '/DNDEBUG=1'} & set(tokens)):
        problems.append('no NDEBUG')
    if problems:
        failed += 1
        print('%s: %s' % (command['file'], ', '.join(problems)))
headers = [os.path.join(d, f) for d, _, files in os.walk(build) for f in files
           if f == 'gaol_configuration.h']
for header in headers:
    with open(header) as h:
        if re.search(r'^\s*#\s*define\s+GAOL_DEBUGGING\b', h.read(), re.M):
            failed += 1
            print('%s defines GAOL_DEBUGGING' % header)
print('%d command(s) of GAOL and CORE-MATH checked, %d gaol_configuration.h'
      % (checked, len(headers)))
sys.exit(1 if failed or not checked or not headers else 0)
