#!/usr/bin/env python3
# The tables of gaol_performance of two builds of GAOL, one beside the other
# (GAOL v5): the median of each row over the runs of each build, and their
# ratio, in Markdown, for the summary of a job of the continuous integration.
#
#   python3 .github/scripts/compare-performance.py "label A" a1.md a2.md a3.md -- "label B" b1.md b2.md b3.md
#
# Only the rows that the control register of the rounding direction can change
# are shown (the arithmetic, the square root, midpoint() and the long lines),
# the others being the same code in both builds. The medians of three runs
# still carry the noise of a shared runner: a ratio within 10% of 1 is not a
# difference.
import statistics
import sys

ROWS = ('x + y', 'x - y', 'x * y', 'x / y', 'sqr(x)', 'sqrt(x)', 'x.midpoint()', 'line of 15 +', 'line of 7 *', 'line of 13 + - * /')


def table(path):
    rows = {}
    title = ''
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
    return title, rows


def median(paths):
    tables = [table(p) for p in paths]
    return tables[0][0], {k: statistics.median(t[1][k] for t in tables if k in t[1]) for k in tables[0][1]}


def main():
    args = sys.argv[1:]
    if '--' not in args:
        sys.exit('usage: compare-performance.py "label A" a.md... -- "label B" b.md...')
    cut = args.index('--')
    a, b = args[:cut], args[cut + 1:]
    title_a, ra = median(a[1:])
    title_b, rb = median(b[1:])
    print('| ns per operation | %s | %s | speed-up, A / B |' % (a[0], b[0]))
    print('|---|---:|---:|---:|')
    for k in ROWS:
        if k in ra and k in rb:
            print('| %s | %.2f | %.2f | %.2f |' % (k, ra[k], rb[k], ra[k] / rb[k]))
    print('')
    print('%s' % title_a)
    print('')
    print('%s' % title_b)


if __name__ == '__main__':
    main()
