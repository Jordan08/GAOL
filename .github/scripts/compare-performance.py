#!/usr/bin/env python3
# The measures of two builds of GAOL, one beside the other (GAOL v5): the
# median of each row over the runs of each build, their ratio and the spread
# of the runs, in Markdown, for the summary of a job of the continuous
# integration (see .github/scripts/performance-ab.sh, which measures the two
# builds in turn, and doc/continuous-integration.md).
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
#
# The ratio is the time of A over the time of B: above 1, B is faster. The
# spread is the largest ratio, over the runs of one build, of its slowest to
# its fastest, for the larger of the two builds: a ratio of A to B that stays
# within it is noise. The runners are shared virtual machines, whose speed
# changes from one moment to the next (x * y with Visual C++ took 7.9 and then
# 16.3 ns, sqrt with MSYS2 6.8 and then 170 ns, on code that had not changed):
# only a difference of several times, on rows that share their code, is one,
# and this program never fails because of what it measures.
import statistics
import sys

REGISTER_ROWS = ('x + y', 'x - y', 'x * y', 'x / y', 'sqr(x)', 'sqrt(x)', 'x.midpoint()', 'line of 15 +', 'line of 7 *', 'line of 13 + - * /')


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


def runs(paths):
    """Per row, the values of the runs, in the order of the rows of the first run."""
    tables = [table(p) for p in paths]
    names = []
    for t in tables:
        names += [k for k in t[1] if k not in names]
    return tables[0][0], tables[0][2], {k: [t[1][k] for t in tables if k in t[1]] for k in names}


def number(value):
    return '%.2f' % value if value < 100 else '%.1f' % value


def spread(values):
    return max(values) / min(values) if values and min(values) > 0 else 1.0


def main():
    args = sys.argv[1:]
    rows_shown = 'all'
    factor = 2.0
    while args and args[0].startswith('--') and args[0] != '--':
        if len(args) < 2:
            sys.exit('compare-performance.py: %s needs a value' % args[0])
        if args[0] == '--rows' and args[1] in ('all', 'register'):
            rows_shown = args[1]
        elif args[0] == '--factor':
            factor = float(args[1])
        else:
            sys.exit('compare-performance.py: unknown option or value: %s %s' % (args[0], args[1]))
        args = args[2:]
    if '--' not in args:
        sys.exit('usage: compare-performance.py [--rows all|register] [--factor F] "label A" a... -- "label B" b...')
    cut = args.index('--')
    a, b = args[:cut], args[cut + 1:]
    if len(a) < 2 or len(b) < 2:
        sys.exit('usage: compare-performance.py [--rows all|register] [--factor F] "label A" a... -- "label B" b...')
    title_a, kind_a, ra = runs(a[1:])
    title_b, kind_b, rb = runs(b[1:])
    kind = kind_a
    names = list(ra) + [k for k in rb if k not in ra]
    if kind == 'table' and rows_shown == 'register':
        names = [k for k in REGISTER_ROWS if k in names]

    first, unit = ('Operation', 'ns per operation') if kind == 'table' else ('Line', 'ns per evaluation of the line')
    print('| %s (%s) | %s | %s | %s / %s | spread of the runs |' % (first, unit, a[0], b[0], a[0], b[0]))
    print('|---|---:|---:|---:|---:|')
    different = []
    for k in names:
        va, vb = ra.get(k), rb.get(k)
        # A pipe in the name of a row (x | y) would end its cell
        cell = k.replace('|', '\\|')
        if va is None or vb is None:
            print('| %s | %s | %s | | |' % (cell, number(statistics.median(va)) if va else '', number(statistics.median(vb)) if vb else ''))
            continue
        ma, mb = statistics.median(va), statistics.median(vb)
        ratio = ma / mb if mb > 0 else float('inf')
        print('| %s | %s | %s | %.2f | %.2f |' % (cell, number(ma), number(mb), ratio, max(spread(va), spread(vb))))
        if ratio >= factor or ratio <= 1.0 / factor:
            different.append('`%s` (%.2f)' % (k, ratio))
    print('')
    print('Medians of %d run%s of %s and %d of %s, measured in turn in the same job. '
          'A ratio above 1 means that %s is faster.' % (len(a) - 1, '' if len(a) == 2 else 's', a[0], len(b) - 1, b[0], b[0]))
    if different:
        print('Rows %g times apart or more: %s.' % (factor, '; '.join(different)))
    else:
        print('No row is %g times apart or more.' % factor)
    print('The runner is a virtual machine shared with other jobs, whose speed changes from one moment to '
          'the next: only a difference of several times, on rows that share their code, means something '
          '(a ratio within the spread of the runs is noise). This table never fails the job.')
    for label, title in ((a[0], title_a), (b[0], title_b)):
        if title:
            print('')
            print('%s: %s' % (label, title))


if __name__ == '__main__':
    main()
