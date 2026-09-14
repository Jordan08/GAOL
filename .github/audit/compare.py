"""compare.py [--check] JSON...

For each machine and compiler audited (audit.sh, probe_build.py), prints the
flags of each build and the configuration that differs between the builds.
With --check, exits with 1 when a build did not configure, or when the builds
differ on a macro GAOL reads or on a flag that bears on the results or the
speed (see MACROS and FLAGS).
"""
import json
import re
import sys

BUILDS = ('cmake', 'autotools', 'meson')

# The macros GAOL reads, compared by whether they are defined and true
MACROS = ('AARCH64_LINUX', 'ARM_MACOSX', 'IX86_LINUX', 'IX86_MACOSX', 'GAOL_CERTAINLY_RELATIONS',
          'GAOL_POSSIBLY_RELATIONS', 'GAOL_SET_RELATIONS', 'GAOL_DEBUGGING', 'GAOL_EXCEPTIONS_ENABLED',
          'GAOL_PRESERVE_ROUNDING', 'GAOL_USING_APMATHLIB', 'GAOL_USING_CRLIBM', 'GAOL_USING_ASM',
          'GAOL_VERBOSE_MODE', 'GETRUSAGE_IN_HEADER', 'HAVE_CASSERT', 'HAVE_CLOCK', 'HAVE_FENV_H',
          'HAVE_GETRUSAGE', 'HAVE_LIMITS', 'HAVE_ROUNDING_MATH_OPTION', 'HAVE_VISIBILITY_OPTIONS',
          'USING_SSE2_INSTRUCTIONS', 'USING_SSE3_INSTRUCTIONS', 'WORDS_BIGENDIAN', 'GAOL_RND_PROBE',
          'NDEBUG', '_GLIBCXX_ASSERTIONS', '__FAST_MATH__', '__OPTIMIZE__', '__SSE2_MATH__', '__STRICT_ANSI__')
# The macros compared by value
VALUES = ('SIZEOF_INT', 'SIZEOF_LONG_LONG_INT', 'FLT_EVAL_METHOD', '__cplusplus')
# The flags compared by presence
FLAGS = ('-frounding-math', '-ffp-contract=off', '-fno-fast-math', '-ffloat-store', '-mfpmath=sse', '-msse2',
         '-msse3', '-fvisibility=hidden', '-fvisibility-inlines-hidden', '-funroll-loops',
         '-fomit-frame-pointer', '-fexpensive-optimizations', '-Wall', '-Wconversion')


def truth(entry, name):
    if entry['macros'].get(name) is None:
        return 'undefined'
    return 'true' if entry['ifs'].get(name) else 'false'


def optimization(flags):
    levels = [f for f in flags if re.match(r'-O(\d|s|g|fast|z)?$', f)]
    return levels[-1] if levels else '-O0'


def standard(flags):
    stds = [f for f in flags if f.startswith('-std=')]
    return stds[-1] if stds else '(default of the compiler)'


def main():
    check = '--check' in sys.argv
    paths = [a for a in sys.argv[1:] if a != '--check']
    failed = False
    for path in paths:
        r = json.load(open(path))
        print('######## %s' % r['label'])
        ok = []
        for b in BUILDS:
            e = r[b]
            if e.get('error') or 'macros' not in e:
                print('  %-9s NOT CONFIGURED: %s' % (b, e.get('error', '?')))
                failed = True
            else:
                print('  %-9s %s: %s' % (b, e.get('version', ''), ' '.join(e['flags'])))
                ok.append(b)
        rows = []
        for n in MACROS:
            vals = [truth(r[b], n) for b in ok]
            if len(set(vals)) > 1:
                rows.append((n, vals))
        for n in VALUES:
            vals = [str(r[b]['macros'].get(n)) for b in ok]
            if len(set(vals)) > 1:
                rows.append((n, vals))
        for f in FLAGS:
            vals = ['yes' if f in r[b]['flags'] else 'no' for b in ok]
            if len(set(vals)) > 1:
                rows.append((f, vals))
        for name, fn in (('optimization', optimization), ('standard', standard)):
            vals = [fn(r[b]['flags']) for b in ok]
            if len(set(vals)) > 1:
                rows.append((name, vals))
        if rows:
            failed = True
            print('  %-28s ' % 'differs' + ' '.join('%-24s' % b for b in ok))
            for n, vals in rows:
                print('  %-28s ' % n + ' '.join('%-24s' % v[:24] for v in vals))
        elif ok:
            print('  the same configuration in: ' + ', '.join(ok))
    if check and failed:
        print('The builds do not agree, or one of them did not configure')
        sys.exit(1)


main()
