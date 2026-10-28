#!/usr/bin/env python3
# The test of the tools that compare the speed of two builds of GAOL (GAOL v5):
# .github/scripts/compare-performance.py, on tables made for the purpose, and
# processor_name() of doc/compare/code/run_perf.sh, which names the processor
# whatever /proc/cpuinfo holds. Python alone, a fraction of a second, no build:
# .github/workflows/performance.yml runs it before the builds, in every job.
#
#   python3 .github/scripts/test-performance-tools.py
#
# COMPARE_PERFORMANCE and RUN_PERF give other copies of the two programs to
# test (to check that a test fails on a copy with a deliberate error).
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
COMPARE = os.environ.get('COMPARE_PERFORMANCE', os.path.join(HERE, 'compare-performance.py'))
RUN_PERF = os.environ.get('RUN_PERF', os.path.join(HERE, '..', '..', 'doc', 'compare', 'code', 'run_perf.sh'))


def table(rows, title='GAOL performance: test'):
    """The output of gaol_performance with rows [(name, ns)]."""
    text = title + '\n\n| Operation | Interval (ns) | Double (ns) | Interval / double |\n|---|---:|---:|---:|\n'
    for name, ns in rows:
        text += '| %s | %.2f | 1.00 | 1.0 |\n' % (name, ns)
    return text


def lines_csv(rows):
    """The output of gaol_bench with the lines [(name, ns)]."""
    return ''.join('gaol5,%s,200000,3,0.009,%.3f,1.0,2.0\n' % (name, ns) for name, ns in rows)


class Compare(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.count = 0

    def tearDown(self):
        shutil.rmtree(self.dir)

    def write(self, text):
        self.count += 1
        path = os.path.join(self.dir, 'run%d.txt' % self.count)
        with open(path, 'w') as f:
            f.write(text)
        return path

    def compare(self, a, b, *options, labels=('base', 'PR')):
        """Runs the program on the texts of the runs of A and of B."""
        command = [sys.executable, COMPARE] + list(options) + [labels[0]] + [self.write(t) for t in a] \
            + ['--', labels[1]] + [self.write(t) for t in b]
        r = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        return r.returncode, r.stdout, r.stderr

    def listed(self, out):
        """The names under 'Rows ... apart', or [] if none."""
        m = re.search(r'^Rows .* apart or more, beyond the spread of the runs: (.*)\.$', out, re.M)
        return re.findall(r'`([^`]*)`', m.group(1)) if m else []

    def test_median_and_ratio(self):
        # The median of 10, 12 and 50 is 12 (their mean is 24)
        rc, out, _ = self.compare([table([('x + y', v)]) for v in (10, 12, 50)], [table([('x + y', 6)])] * 3)
        self.assertEqual(rc, 0)
        self.assertIn('| x + y | 12.00 | 6.00 | 2.00 | 5.00 |', out)
        self.assertIn('| Operation (ns per operation) | base | PR | base / PR | spread of the runs |', out)
        self.assertIn('Medians of 3 runs of base and 3 of PR', out)

    def test_threshold(self):
        # By default a row is listed from 1.5 times apart: 1.49 and 0.67 (above
        # 2/3) are not, 1.5 and 0.6 are
        rows_a = [('just under', 1.49), ('one and a half', 1.5), ('three fifths', 0.6), ('just over two thirds', 0.67), ('same', 1.0)]
        rows_b = [(name, 1.0) for name, _ in rows_a]
        rc, out, _ = self.compare([table(rows_a)], [table(rows_b)])
        self.assertEqual(rc, 0)
        self.assertEqual(self.listed(out), ['one and a half', 'three fifths'])
        # --factor gives another one
        rows_a = [('just under', 1.99), ('twice', 2.0), ('half', 0.5), ('just over half', 0.51), ('same', 1.0)]
        rows_b = [(name, 1.0) for name, _ in rows_a]
        rc, out, _ = self.compare([table(rows_a)], [table(rows_b)], '--factor', '2')
        self.assertEqual(self.listed(out), ['twice', 'half'])
        rc, out, _ = self.compare([table(rows_a)], [table(rows_b)])
        self.assertEqual(self.listed(out), ['just under', 'twice', 'half', 'just over half'])

    def test_ratio_within_the_spread_is_noise(self):
        # A's runs are 3, 1 and 1 (spread 3): a ratio of 0.5 is noise, one of 0.25 is not
        runs_a = [table([('half', 3), ('quarter', 3)]), table([('half', 1), ('quarter', 1)]), table([('half', 1), ('quarter', 1)])]
        rc, out, _ = self.compare(runs_a, [table([('half', 2), ('quarter', 4)])] * 3)
        self.assertEqual(rc, 0)
        self.assertIn('| half | 1.00 | 2.00 | 0.50 | 3.00 |', out)
        self.assertEqual(self.listed(out), ['quarter'])
        # The same on the side of B, and above 1: 5 is beyond a spread of 3, 2.5 is not
        runs_b = [table([('five', 3), ('two and a half', 3)]), table([('five', 1), ('two and a half', 1)]), table([('five', 1), ('two and a half', 1)])]
        rc, out, _ = self.compare([table([('five', 5), ('two and a half', 2.5)])] * 3, runs_b)
        self.assertEqual(self.listed(out), ['five'])

    def test_no_row_listed(self):
        rc, out, _ = self.compare([table([('x', 1.4)])], [table([('x', 1.0)])])
        self.assertEqual(rc, 0)
        self.assertIn('No row is 1.5 times apart or more beyond the spread of the runs.', out)

    def test_row_on_one_side_only(self):
        rc, out, _ = self.compare([table([('x', 2), ('only base', 3)])], [table([('x', 2), ('only PR', 4)])])
        self.assertEqual(rc, 0)
        self.assertIn('| only base | 3.00 |  | | |', out)
        self.assertIn('| only PR |  | 4.00 | | |', out)
        self.assertEqual(self.listed(out), [])

    def test_pipe_in_a_name(self):
        rc, out, _ = self.compare([table([('x | y', 2)])], [table([('x | y', 1)])])
        self.assertEqual(rc, 0)
        self.assertIn('| x \\| y | 2.00 | 1.00 | 2.00 | 1.00 |', out)
        self.assertEqual(self.listed(out), ['x | y'])

    def test_one_run_each(self):
        rc, out, _ = self.compare([table([('x', 3)])], [table([('x', 1)])])
        self.assertEqual(rc, 0)
        self.assertIn('| x | 3.00 | 1.00 | 3.00 | 1.00 |', out)
        self.assertIn('Medians of 1 run of base and 1 of PR', out)

    def test_zeros(self):
        runs_a = [table([('both 0', 0), ('over 0', 5), ('run of 0', v)]) for v in (0, 1, 2)]
        runs_b = [table([('both 0', 0), ('over 0', 0), ('run of 0', 1)])] * 3
        rc, out, _ = self.compare(runs_a, runs_b)
        self.assertEqual(rc, 0)
        self.assertIn('| both 0 | 0.00 | 0.00 | n/a | 1.00 |', out)
        self.assertIn('| over 0 | 5.00 | 0.00 | inf | 1.00 |', out)
        # A run of 0 and a run of 2: the spread is infinite
        self.assertIn('| run of 0 | 1.00 | 1.00 | 1.00 | inf |', out)
        self.assertEqual(self.listed(out), ['over 0'])

    def test_no_comparable_table_fails(self):
        for a, b, reason in (
                ([''], [table([('x', 1)])], 'has no row'),
                ([table([('x', 1)])], [''], 'has no row'),
                ([''], [''], 'has no row'),
                ([table([('x', 1)]), '', table([('x', 1)])], [table([('x', 1)])], 'has no row'),
                ([table([('x', 1)])], [table([('y', 1)])], 'no row is common'),
                # The output of gaol_performance in another format
                ([table([('x', 1)])], ['| Operation | ns |\n|---|---|\n| x | fast |\n'], 'has no row')):
            with self.subTest(a=a, b=b):
                rc, out, err = self.compare(a, b)
                self.assertNotEqual(rc, 0)
                self.assertIn(reason, out)
                self.assertIn(reason, err)
                self.assertNotIn('No row is', out)

    def test_optional_table_is_a_notice(self):
        rc, out, err = self.compare([''], [table([('x', 1)])], '--optional')
        self.assertEqual(rc, 0)
        self.assertIn('Cannot compare base with PR', out)
        rc, out, _ = self.compare([table([('x', 2)])], [table([('x', 1)])], '--optional')
        self.assertEqual(rc, 0)
        self.assertIn('| x | 2.00 | 1.00 | 2.00 | 1.00 |', out)

    def test_register_rows(self):
        rows = [('x + y', 1), ('pow(x, 3)', 5), ('x.midpoint()', 4)]
        rc, out, _ = self.compare([table(rows)], [table(rows)], '--rows', 'register')
        self.assertEqual(rc, 0)
        self.assertIn('| x + y |', out)
        self.assertIn('| x.midpoint() |', out)
        self.assertNotIn('pow(x, 3)', out)
        rc, out, _ = self.compare([table(rows)], [table(rows)])
        self.assertIn('pow(x, 3)', out)

    def test_long_lines(self):
        rows = [('sum16', 48.686), ('mul8', 43.2)]
        rc, out, _ = self.compare([lines_csv(rows)] * 3, [lines_csv([('sum16', 9.1), ('mul8', 43.0)])] * 3)
        self.assertEqual(rc, 0)
        self.assertIn('| Line (ns per evaluation of the line) | base | PR | base / PR | spread of the runs |', out)
        self.assertIn('| sum16 | 48.69 | 9.10 | 5.35 | 1.00 |', out)
        self.assertEqual(self.listed(out), ['sum16'])

    def test_line_ends_of_windows(self):
        # The output of a program of Windows has CRLF line ends
        path = os.path.join(self.dir, 'crlf.txt')
        with open(path, 'wb') as f:
            f.write(table([('x', 4)]).replace('\n', '\r\n').encode())
        r = subprocess.run([sys.executable, COMPARE, 'base', path, '--', 'PR', self.write(table([('x', 1)]))],
                           stdout=subprocess.PIPE, universal_newlines=True)
        self.assertEqual(r.returncode, 0)
        self.assertIn('| x | 4.00 | 1.00 | 4.00 | 1.00 |', r.stdout)

    def test_titles(self):
        rc, out, _ = self.compare([table([('x', 1)], 'GAOL performance: A')], [table([('x', 1)], 'GAOL performance: B')])
        self.assertIn('base: GAOL performance: A', out)
        self.assertIn('PR: GAOL performance: B', out)

    def test_usage(self):
        for options in (['base', 'x'], ['--rows', 'some', 'a', 'x', '--', 'b', 'y'], ['--factor', '1', 'a', 'x', '--', 'b', 'y'],
                        ['--factor', 'two', 'a', 'x', '--', 'b', 'y'], ['--unknown', '1', 'a', 'x', '--', 'b', 'y'], ['a', '--', 'b', 'y']):
            with self.subTest(options=options):
                r = subprocess.run([sys.executable, COMPARE] + options, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                self.assertNotEqual(r.returncode, 0)


@unittest.skipUnless(os.name == 'posix' and shutil.which('bash'), 'bash only')
class ProcessorName(unittest.TestCase):
    """processor_name() of run_perf.sh, which has to name a processor under set -euo pipefail."""

    def setUp(self):
        self.dir = tempfile.mkdtemp()
        with open(RUN_PERF) as f:
            m = re.search(r'^processor_name\(\) \{\n.*?^\}\n', f.read(), re.M | re.S)
        self.assertTrue(m, 'processor_name() is not in run_perf.sh')
        self.function = m.group(0)
        # A PATH with the tools the function uses and, apart from sysctl, no other
        self.bin = os.path.join(self.dir, 'bin')
        os.mkdir(self.bin)
        for tool in ('grep', 'cut', 'sed'):
            os.symlink(shutil.which(tool), os.path.join(self.bin, tool))

    def tearDown(self):
        shutil.rmtree(self.dir)

    def sysctl(self, text, status=0):
        path = os.path.join(self.bin, 'sysctl')
        with open(path, 'w') as f:
            f.write('#!/bin/sh\n' + ('echo "%s"\n' % text if text else '') + 'exit %d\n' % status)
        os.chmod(path, 0o755)

    def cpuinfo(self, text):
        path = os.path.join(self.dir, 'cpuinfo')
        with open(path, 'w') as f:
            f.write(text)
        return path

    def name(self, cpuinfo):
        r = subprocess.run([shutil.which('bash'), '-euo', 'pipefail', '-c', self.function + 'processor_name'],
                           env={'PATH': self.bin, 'CPUINFO': cpuinfo}, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                           universal_newlines=True)
        return r.returncode, r.stdout.strip()

    def test_model_name(self):
        self.assertEqual(self.name(self.cpuinfo('processor\t: 0\nmodel name\t: Test CPU 9000\nmodel name\t: Second\n')), (0, 'Test CPU 9000'))

    def test_no_model_name_takes_sysctl(self):
        self.sysctl('Brand of sysctl')
        self.assertEqual(self.name(self.cpuinfo('processor\t: 0\nCPU part\t: 0xd40\n')), (0, 'Brand of sysctl'))

    def test_no_cpuinfo_takes_sysctl(self):
        self.sysctl('Brand of sysctl')
        self.assertEqual(self.name(os.path.join(self.dir, 'missing')), (0, 'Brand of sysctl'))

    def test_nothing_to_name_it(self):
        self.assertEqual(self.name(os.path.join(self.dir, 'missing')), (0, 'an unknown processor'))
        self.assertEqual(self.name(self.cpuinfo('processor\t: 0\n')), (0, 'an unknown processor'))
        self.sysctl('', status=1)
        self.assertEqual(self.name(self.cpuinfo('')), (0, 'an unknown processor'))


if __name__ == '__main__':
    unittest.main()
