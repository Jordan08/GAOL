#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# The outputs the manual of GAOL v5 shows, against those of the library
# (GAOL v5): each example of manual/v5/gaol.tex that shows what it prints,
# in its lines @outputs^...~, is compiled with an installed GAOL and run, and
# what it prints is compared, line by line, with what the manual shows. Until
# #55, the examples printed 1 and 0 where the manual showed true and false,
# and chi() of the empty set printed -nan with GCC where it showed nan.
#
# An example holding a main() is compiled as it is. The others are put in a
# main() of their own, after the headers of GAOL (<gaol/gaol> and
# <gaol/gaol_expression.h>) and of the standard library they use, with
# using namespace std and using namespace gaol, or gaol_ieee1788 in the
# chapter of its names, which the manual's examples take for granted. In the
# code, @rem^...~ is the comment it holds; @textasciitilde is a tilde; in an
# output, @version is the version of the installed GAOL, which gaol::version
# prints. A line printed that is too long for the page may be shown on
# several lines of output, each break standing for a blank. Each example is a
# program of its own: a format or a precision one sets does not reach the
# next. A compiler error points to the line of the example in gaol.tex. Every
# line @outputs^...~ of the manual has to be in an example the script reads.
# What the manual shows otherwise, in a block onscreen after a program or in
# a comment "Prints ...", is not checked.
#
# The compiler is $CXX, c++ otherwise, with the flags pkg-config gives for
# gaol, so that PKG_CONFIG_PATH has to lead to the gaol.pc of the GAOL to
# check, in <prefix>/lib/pkgconfig:
#
#     PKG_CONFIG_PATH=<prefix>/lib/pkgconfig python3 manual/check_examples.py [-j 4] [--std c++11]
#
# The exit status is 1 when an example does not compile, fails, or prints
# something else than the manual shows.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

import argparse
import concurrent.futures
import os
import re
import shlex
import subprocess
import sys
import tempfile

# The commands of the examples: @name^argument~ (commandchars=\@\^\~ of the
# environment examplevb, manual/v5/manual.cls)
COMMAND = re.compile(r'@(\w+)\^([^~]*)~')
OUTPUT = re.compile(r'\s*@outputs\^(.*)~\s*')
# A tilde, which would end a command: '\0' while the commands are read
TILDE = '@textasciitilde'

PROLOGUE = '''#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <gaol/gaol>
#include <gaol/gaol_expression.h>
using namespace std;
using namespace {namespace};

int main()
{{
#line {line} "{tex}"
'''

EPILOGUE = '''
  gaol::cleanup();
  return 0;
}
'''


class Example:
    def __init__(self, line, code, outputs, namespace):
        self.line = line            # The line of the first line of code in gaol.tex
        self.code = code            # The code, a line of output left empty
        self.outputs = outputs      # The lines of output the manual shows
        self.namespace = namespace  # gaol, or gaol_ieee1788


def code_line(text, tex, line):
    """The C++ of a line of an example: the comment of @rem^...~."""
    def command(m):
        if m.group(1) != 'rem':
            raise ValueError('%s:%d: @%s^...~ in an example that shows an output'
                             % (tex, line, m.group(1)))
        return m.group(2)
    code = COMMAND.sub(command, text)
    if '@' in code:
        raise ValueError('%s:%d: a command the script does not know: %s' % (tex, line, text.strip()))
    return code


def examples(tex, text):
    """The examples of text that show what they print, in their order."""
    # The examples of the chapter of the names of IEEE 1788-2015 open gaol_ieee1788
    chapters = [m.start() for m in re.finditer(r'\\chapter\b', text)] + [len(text)]
    label = text.find(r'\label{chap:ieee1788}')
    ieee = [(a, b) for a, b in zip(chapters, chapters[1:]) if a <= label < b]
    found = []
    for m in re.finditer(r'\\begin\{example\}[^\n]*\n(.*?)\\end\{example\}', text, re.S):
        if '@outputs^' not in m.group(1):
            continue
        first = text.count('\n', 0, m.start(1)) + 1
        code, outputs = [], []
        for i, line in enumerate(m.group(1).replace(TILDE, '\0').split('\n')[:-1]):
            output = OUTPUT.fullmatch(line)
            if output:
                outputs.append(output.group(1).rstrip().replace('\0', '~'))
                code.append('')
            elif '@outputs^' in line:
                raise ValueError('%s:%d: an output that is not alone on its line' % (tex, first + i))
            else:
                code.append(code_line(line, tex, first + i).replace('\0', '~'))
        namespace = 'gaol_ieee1788' if ieee and ieee[0][0] <= m.start() < ieee[0][1] else 'gaol'
        found.append(Example(first, '\n'.join(code), outputs, namespace))
    # An output the script does not read, in an example it does not
    # recognize, would not be checked: none may be left out
    read = sum(len(example.outputs) for example in found)
    if not found or read != text.count('@outputs^'):
        raise ValueError('%s: %d lines @outputs^...~, of which %d in the %d examples read'
                         % (tex, text.count('@outputs^'), read, len(found)))
    return found


def program(example, tex):
    """The program of an example, its lines at their line in tex."""
    if re.search(r'\bmain\s*\(', example.code):
        return '#line %d "%s"\n%s\n' % (example.line, tex, example.code)
    return (PROLOGUE.format(namespace=example.namespace, line=example.line, tex=tex)
            + example.code + EPILOGUE)


def lines(text):
    """The lines of text, without the blanks that end them, nor the empty lines at its end."""
    result = [line.rstrip() for line in text.split('\n')]
    while result and not result[-1]:
        result.pop()
    return result


def shows(expected, printed):
    """Whether the lines printed are those the manual shows, where a line
    printed may be shown on several, each break standing for a blank."""
    i = 0
    for line in printed:
        if i == len(expected):
            return False
        shown = expected[i]
        i += 1
        while shown != line and len(shown) < len(line) and i < len(expected):
            shown += ' ' + expected[i]
            i += 1
        if shown != line:
            return False
    return i == len(expected)


def check(example, number, tex, directory, compiler, flags, libs, version):
    """None if the example prints what the manual shows, what goes wrong otherwise."""
    source = os.path.join(directory, 'example%02d.cpp' % number)
    executable = os.path.join(directory, 'example%02d' % number)
    with open(source, 'w') as f:
        f.write(program(example, tex))
    build = subprocess.run(compiler + flags + [source, '-o', executable] + libs,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    if build.returncode != 0:
        return 'does not compile:\n' + build.stdout
    try:
        run = subprocess.run([executable], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True, timeout=60)
    except subprocess.TimeoutExpired:
        return 'did not end within 60 s'
    if run.returncode != 0:
        return 'ended with the status %d:\n%s%s' % (run.returncode, run.stdout, run.stderr)
    expected = [output.replace('@version', version) for output in example.outputs]
    printed = lines(run.stdout)
    if not shows(expected, printed) or run.stderr:
        return ('prints something else than the manual shows\n  manual:  %s\n  printed: %s%s'
                % ('\n           '.join(expected), '\n           '.join(printed),
                   '\n  on the error output: ' + run.stderr if run.stderr else ''))
    return None


def pkg_config(*arguments):
    try:
        return subprocess.run(['pkg-config'] + list(arguments) + ['gaol'], stdout=subprocess.PIPE,
                              universal_newlines=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        sys.exit('check_examples.py: pkg-config does not find gaol: '
                 'set PKG_CONFIG_PATH to the directory of its gaol.pc')


def main():
    p = argparse.ArgumentParser(description='Compiles the examples of the manual of GAOL v5 that show what '
                                'they print, runs them, and compares what they print with the manual.')
    p.add_argument('tex', nargs='?', default=os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                                         'v5', 'gaol.tex'),
                   help='the manual (manual/v5/gaol.tex by default)')
    p.add_argument('-j', '--jobs', type=int, default=1, help='the examples compiled at once (1)')
    p.add_argument('--std', help='the standard of C++, as c++11 (that of the compiler by default)')
    p.add_argument('--keep', metavar='DIRECTORY', help='writes the programs there, and keeps them')
    p.add_argument('-v', '--verbose', action='store_true', help='names every example checked')
    args = p.parse_args()

    with open(args.tex, encoding='utf-8') as f:
        try:
            found = examples(args.tex, f.read())
        except ValueError as e:
            sys.exit('check_examples.py: %s' % e)
    compiler = shlex.split(os.environ.get('CXX', 'c++'))
    flags = (['-std=' + args.std] if args.std else []) + shlex.split(pkg_config('--cflags'))
    libs = shlex.split(pkg_config('--libs'))
    if sys.platform != 'win32':
        libs.append('-Wl,-rpath,' + pkg_config('--variable=libdir'))
    version = pkg_config('--modversion')

    def run_all(directory):
        with concurrent.futures.ThreadPoolExecutor(max(1, args.jobs)) as pool:
            return list(pool.map(lambda n: check(found[n], n + 1, args.tex, directory, compiler, flags,
                                                 libs, version), range(len(found))))
    if args.keep:
        os.makedirs(args.keep, exist_ok=True)
        results = run_all(args.keep)
    else:
        with tempfile.TemporaryDirectory() as directory:
            results = run_all(directory)

    failed = 0
    for example, result in zip(found, results):
        if result:
            failed += 1
            print('%s:%d: the example %s' % (args.tex, example.line, result))
        elif args.verbose:
            print('%s:%d: as the manual shows' % (args.tex, example.line))
    print('%d examples showing an output: %d print it, %d do not' % (len(found), len(found) - failed, failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
