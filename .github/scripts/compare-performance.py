#!/usr/bin/env python3
# The measures of two builds of GAOL, one beside the other (GAOL v5): the
# median of each row over the runs of each build, their ratio and the spread
# of the runs, in Markdown, for the summary of a job of the continuous
# integration (see .github/scripts/performance-ab.sh, which measures the two
# builds in turn, and doc/continuous-integration.md). Its test is
# .github/scripts/test-performance-tools.py.
#
#   python3 .github/scripts/compare-performance.py [options] "label A" a1 a2 a3 -- "label B" b1 b2 b3
#
# The files are the outputs of gaol_performance (a Markdown table, whose
# rows are in nanoseconds per operation) or of gaol_bench, the program of
# doc/compare/code (CSV, the rows being the long lines of run_lines.sh, in
# nanoseconds per evaluation of the line); the kind is read from each file.
#
#   --rows all|register   the rows of a table of gaol_performance to show: all of
#                         them (default), or only those that the control register
#                         of the rounding direction can change (the arithmetic,
#                         the square root, midpoint() and the long lines), the
#                         others being the same code in builds that differ by
#                         that register alone
#   --factor F            a row is listed as different from F times (default 2)
#   --optional            a table that cannot be made is a notice, not an error
#                         (the long lines, which are an addition)
#
# The ratio is the time of A over the time of B: above 1, B is faster (n/a for
# two times of 0). The spread is the largest ratio, over the runs of one
# build, of its slowest to its fastest (inf if a run took no time), for the
# larger of the two builds. The runners are shared virtual machines, whose
# speed changes from one moment to the next (x * y with Visual C++ took 7.9 and
# then 16.3 ns, sqrt with MSYS2 6.8 and then 170 ns, on code that had not
# changed): only a difference of several times, on rows that share their code,
# is one. A row is listed under the table as different when its ratio is F
# times from 1 or more and beyond the spread: a ratio within the spread is
# noise.
#
# Times never make it fail. It fails (status 1, with the reason) when the
# tables cannot be compared: a file with no row, or no row common to the two
# builds, which is what a change of the format of the output of
# gaol_performance gives; with --optional it says so and succeeds.
import statistics
import sys

REGISTER_ROWS = ('x + y', 'x - y', 'x * y', 'x / y', 'sqr(x)', 'sqrt(x)', 'x.midpoint()', 'line of 15 +', 'line of 7 *', 'line of 13 + - * /')
USAGE = 'usage: compare-performance.py [--rows all|register] [--factor F] [--optional] "label A" a... -- "label B" b...'


def table(path):
    """The title, the rows (name: ns) and the kind ('table' or 'lines') of one output."""
    rows = {}
    title = ''
    kind = 'table'
    with open(path, errors='replace') as f:
        for line in f:
            line = line.rstrip('\n')
            if line.startswith('GAOL performance'):
                title = line
            if line.startswith('|') and not line.startswith('|---') and 'Operation' not in line:
                cells = [c.strip() for c in line.strip('|').rsplit('|', 3)]
                try:
                    rows[cells[0]] = float(cells[1])
                except (ValueError, IndexError):
                    pass
                continue
            # A line of gaol_bench: library,line,n,repeats,best (s),ns,sum of the midpoints,sum of the widths
            fields = line.split(',')
            if len(fields) == 8:
                try:
                    int(fields[2])
                    rows[fields[1].strip()] = float(fields[5])
                    kind = 'lines'
                except ValueError:
                    pass
    return title, rows, kind


def runs(tables):
    """Per row, the values of the runs, in the order of the rows of the first run."""
    names = []
    for t in tables:
        names += [k for k in t[1] if k not in names]
    return {k: [t[1][k] for t in tables if k in t[1]] for k in names}


def number(value):
    return '%.2f' % value if value < 100 else '%.1f' % value


def spread(values):
    """The slowest run over the fastest: 1 for equal runs, inf if a run took no time."""
    low, high = min(values), max(values)
    if high == low:
        return 1.0
    return high / low if low > 0 else float('inf')


def ratio(a, b):
    """a over b: None for two times of 0, inf for a time over 0."""
    if a == 0 and b == 0:
        return None
    return a / b if b > 0 else float('inf')


def different(r, factor, noise):
    """Whether a ratio is factor times from 1 or more, and beyond the spread of the runs."""
    return (r >= factor and r > noise) or (r <= 1.0 / factor and r < 1.0 / noise)


def fixed(value):
    return 'n/a' if value is None else '%.2f' % value


def main():
    args = sys.argv[1:]
    rows_shown = 'all'
    factor = 2.0
    optional = False
    while args and args[0].startswith('--') and args[0] != '--':
        if args[0] == '--optional':
            optional = True
            args = args[1:]
            continue
        if len(args) < 2:
            sys.exit('compare-performance.py: %s needs a value' % args[0])
        if args[0] == '--rows' and args[1] in ('all', 'register'):
            rows_shown = args[1]
        elif args[0] == '--factor':
            try:
                factor = float(args[1])
            except ValueError:
                sys.exit('compare-performance.py: --factor needs a number: %s' % args[1])
            if not factor > 1:
                sys.exit('compare-performance.py: --factor needs a number above 1: %s' % args[1])
        else:
            sys.exit('compare-performance.py: unknown option or value: %s %s' % (args[0], args[1]))
        args = args[2:]
    if '--' not in args:
        sys.exit(USAGE)
    cut = args.index('--')
    a, b = args[:cut], args[cut + 1:]
    if len(a) < 2 or len(b) < 2:
        sys.exit(USAGE)
    tables_a = [table(p) for p in a[1:]]
    tables_b = [table(p) for p in b[1:]]
    ra, rb = runs(tables_a), runs(tables_b)
    title_a, kind = tables_a[0][0], tables_a[0][2]
    title_b = tables_b[0][0]
    names = list(ra) + [k for k in rb if k not in ra]
    if kind == 'table' and rows_shown == 'register':
        names = [k for k in REGISTER_ROWS if k in names]

    # What cannot be compared: an output with no row, and no row in common
    problems = ['%s has no row' % path for paths, tables in ((a[1:], tables_a), (b[1:], tables_b))
                for path, t in zip(paths, tables) if not t[1]]
    if not [k for k in names if k in ra and k in rb]:
        problems.append('no row is common to %s and %s' % (a[0], b[0]))
    if problems:
        message = 'Cannot compare %s with %s: %s.' % (a[0], b[0], '; '.join(problems))
        print(message)
        if optional:
            return
        sys.exit('compare-performance.py: ' + message)

    first, unit = ('Operation', 'ns per operation') if kind == 'table' else ('Line', 'ns per evaluation of the line')
    print('| %s (%s) | %s | %s | %s / %s | spread of the runs |' % (first, unit, a[0], b[0], a[0], b[0]))
    print('|---|---:|---:|---:|---:|')
    listed = []
    for k in names:
        va, vb = ra.get(k), rb.get(k)
        # A pipe in the name of a row (x | y) would end its cell
        cell = k.replace('|', '\\|')
        if va is None or vb is None:
            print('| %s | %s | %s | | |' % (cell, number(statistics.median(va)) if va else '', number(statistics.median(vb)) if vb else ''))
            continue
        ma, mb = statistics.median(va), statistics.median(vb)
        r = ratio(ma, mb)
        noise = max(spread(va), spread(vb))
        print('| %s | %s | %s | %s | %s |' % (cell, number(ma), number(mb), fixed(r), fixed(noise)))
        if r is not None and different(r, factor, noise):
            listed.append('`%s` (%s)' % (k, fixed(r)))
    print('')
    print('Medians of %d run%s of %s and %d of %s, measured in turn in the same job. '
          'A ratio above 1 means that %s is faster.' % (len(a) - 1, '' if len(a) == 2 else 's', a[0], len(b) - 1, b[0], b[0]))
    if listed:
        print('Rows %g times apart or more, beyond the spread of the runs: %s.' % (factor, '; '.join(listed)))
    else:
        print('No row is %g times apart or more beyond the spread of the runs.' % factor)
    print('The runner is a virtual machine shared with other jobs, whose speed changes from one moment to '
          'the next: only a difference of several times, on rows that share their code, means something '
          '(a ratio within the spread of the runs is noise). No time fails the job.')
    for label, title in ((a[0], title_a), (b[0], title_b)):
        if title:
            print('')
            print('%s: %s' % (label, title))


if __name__ == '__main__':
    main()
