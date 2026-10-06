"""The flags of interval arithmetic in the code of a meson project using GAOL
as a subproject (GAOL v5).

    python3 check_flags.py <build directory> <source directory of the project>

Fails unless each command of compile_commands.json that compiles a source of
the project, not of its subprojects, holds the flags of the Cflags of the
gaol.pc the subproject GAOL wrote in meson-private, its include directory
aside: the code including GAOL's headers is to be compiled with them (see
doc/using.md), and gaol_dep alone gives them, the arguments of the project
GAOL reaching none of the targets of another project.

Copyright (c) 2026 ENSTA, France

Created 2026-10-06 by Jordan NININ
"""
import json
import os
import shlex
import sys

build, source = sys.argv[1], sys.argv[2]
with open(os.path.join(build, 'meson-private', 'gaol.pc')) as pc:
    cflags = [line for line in pc if line.startswith('Cflags:')][0]
flags = [f for f in shlex.split(cflags[len('Cflags:'):])
         if not f.startswith('-I')]
with open(os.path.join(build, 'compile_commands.json')) as db:
    commands = json.load(db)
subprojects = os.path.join(os.path.normpath(source), 'subprojects') + os.sep
checked = 0
failed = 0
for command in commands:
    path = os.path.normpath(os.path.join(command['directory'], command['file']))
    if path.startswith(subprojects):
        continue
    checked += 1
    missing = [f for f in flags if f not in shlex.split(command['command'])]
    if missing:
        failed += 1
        print('%s is compiled without %s'
              % (command['file'], ' '.join(missing)))
print('%d command(s) of the project checked for %s'
      % (checked, ' '.join(flags)))
sys.exit(1 if failed or not checked or not flags else 0)
