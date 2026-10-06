"""The flags of interval arithmetic in the code of a meson project using GAOL
as a subproject (GAOL v5).

    python3 check_flags.py <build directory> <source directory of the project>

Fails unless each command of compile_commands.json that compiles a source of
the project, not of its subprojects, holds the flags of the Cflags of the
gaol.pc the subproject GAOL wrote in meson-private, its include directory
aside, and unless each command of build.ninja that links an executable of the
project holds the options of its Libs, -L and -l aside (-mno-daz-ftz, where
the compiler takes it; with a compiler that refuses it, there is none): the
code including GAOL's headers is to be compiled and linked with them (see
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
    lines = pc.read().splitlines()
cflags = [line for line in lines if line.startswith('Cflags:')][0]
libs = [line for line in lines if line.startswith('Libs:')][0]
flags = [f for f in shlex.split(cflags[len('Cflags:'):])
         if not f.startswith('-I')]
link_flags = [f for f in shlex.split(libs[len('Libs:'):])
              if not f.startswith(('-L', '-l'))]
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

# The links of the executables of the project: a statement "build <target>:
# cpp_LINKER ..." of build.ninja, whose variable LINK_ARGS follows on an
# indented line; those of the subprojects are under subprojects/
linked = 0
with open(os.path.join(build, 'build.ninja')) as ninja:
    statements = ninja.read().split('\nbuild ')
for statement in statements[1:]:
    head, _, rest = statement.partition('\n')
    target, _, rule = head.partition(': ')
    if not rule.startswith(('cpp_LINKER', 'c_LINKER')) or \
       target.startswith('subprojects/'):
        continue
    args = [line.strip()[len('LINK_ARGS = '):] for line in rest.split('\n')
            if line.startswith(' ') and line.strip().startswith('LINK_ARGS = ')]
    linked += 1
    missing = [f for f in link_flags
               if f not in shlex.split(args[0] if args else '')]
    if missing:
        failed += 1
        print('%s is linked without %s' % (target, ' '.join(missing)))
print('%d link(s) of the project checked for %s'
      % (linked, ' '.join(link_flags) or 'no option (gaol.pc gives none)'))
sys.exit(1 if failed or not checked or not flags or not linked else 0)
