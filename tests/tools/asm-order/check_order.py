#!/usr/bin/env python3
#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: reads the assembly of the library for the instructions
# that compute with the rounding direction (add, sub, mul, div, sqrt, fused
# multiply-add, conversions that round, rounding with the direction of the
# control register) placed on the wrong side of an instruction that writes
# the control register (ldmxcsr, fldcw, or a call of round_upward_if_needed()
# or fesetround(), which write it). GCC does not model the rounding
# direction (bug 34678): it may compute a result after the write that sets
# the direction back, when the result is only used after it, or before the
# write that sets it upward, when the operands are in registers. GAOL keeps
# its bounds out of the way of both by reading them from memory after the
# check (round_upward_if_needed()), by rnd_reread() and by rnd_keep()
# (gaol/gaol_fpu.h); this reads the code the compiler made of them.
#
#     python3 check_order.py [--build DIR] [--keep DIR] [--preserve]
#                            [--verbose] [FILE.s]...
#     python3 check_order.py --selftest
#
# Each FILE is the assembly of x86-64 of GCC or Clang (AT&T syntax, from -S).
# With --build DIR, the CMake build in DIR (configured with
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON) has the sources of the library
# (gaol/, not the tests) compiled again with -S and the flags it compiled
# them with, into DIR/asm-order (--keep DIR: or there). The checks, for each
# function that holds a write of the control register, on the paths of its
# control flow graph (calls are taken as they are: what a callee does is read
# in the callee, the code of which is read as well):
#
# - BEFORE: an instruction that rounds, other than the probe of the direction
#   (1 + (2^-1060 + 0), whose sums are meant to be computed in the direction
#   found: they follow the use of a static named subnormal or tiny, and end
#   with the jump that tells the direction), that the function runs before
#   the write on a path from its entry to that write, and before the jump of
#   the probe. Instructions that the function runs without a write, because
#   the probe found the direction upward, are not these: only those with a
#   write after them. A conversion of a 32-bit integer to a double is not one
#   either: it is exact. The edges that go back to a loop are left out, so
#   that the instructions of an iteration are not taken for the ones before the
#   write of the next.
# - AFTER (with --preserve, for the builds with GAOL_PRESERVE_ROUNDING, whose
#   operations write the register back): an instruction that rounds, after a
#   write with no other write after it, and that the entry cannot reach
#   without a write (a function whose writes are conditional, as the ones of
#   the registers of ARM and of the processors of 32-bit x86, reach all of its
#   code without one: its second check is by the tests, which compare the
#   results in every direction).
#
# Not seen: the code a callee runs, the instructions inside an asm statement,
# and the order of a computation and a write in two functions (the first
# calls the second between the two): the tests that call each operation in
# every direction, tests/rounding_direction.cpp, see the results.
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-27 by Jordan NININ (the check of the assembly of AArch64 and
# 32-bit ARM of #150 was a script of the work directory, which was lost)

import argparse
import collections
import json
import os
import re
import shlex
import subprocess
import sys

# The instructions that write the control register of the rounding direction
WRITES = re.compile(r'^(v?ldmxcsr|fldcw|fldenv|v?fxrstor(64)?|x?rstor(s|s64|64)?|xrstors?)$')

# The instructions whose result depends on the rounding direction
ARITHMETIC = re.compile(r'^v?(add|sub|mul|div|sqrt|hadd|hsub|addsub|dp)(ss|sd|ps|pd)$')
FUSED = re.compile(r'^v?f(n?m(add|sub)|m(add|sub)(add|sub))(132|213|231)(ss|sd|ps|pd)$')
CONVERSION = re.compile(r'^v?cvt(si2(sd|ss)[lq]?|usi2(sd|ss)[lq]?|sd2ss|pd2ps|sd2si[lq]?|ss2si[lq]?|pd2dq|ps2dq|dq2ps'
                        r'|(u?q?q2(pd|ps))|pd2(u?q?q|udq)|ps2(u?q?q|udq)|sd2usi|ss2usi)$')
ROUNDING = re.compile(r'^v?(round(ss|sd|ps|pd)|rndscale(ss|sd|ps|pd)|reduce(ss|sd|ps|pd))$')
X87 = re.compile(r'^f(i?(add|sub|subr|mul|div|divr)(p|l|s)?|sqrt|rndint|ist(p)?(ll)?|scale|prem1?|yl2x(p1)?|sin|cos|sincos|ptan|patan)$')
EMBEDDED_ROUNDING = re.compile(r'\{r[nduz]-sae\}')
INT32_TO_DOUBLE = re.compile(r'^v?cvtu?si2sd[lq]?$')

PROBE_SYMBOL = re.compile(r'(subnormal|tiny)', re.I)

# The findings that are right and harmless, read on the code of 2026-10-27 (GCC 15.2, -O3): kind, a regular
# expression for the name of the function (demangled), and why. A finding of the list is printed as "known" and
# does not make the check fail; an entry that matches nothing in a run over the whole library is reported
KNOWN = [
    ('AFTER', r'difference_at_least',
     'the halving 0.5 * a of the bounds of the recursive call is exact in every direction: the bounds are 2^970 '
     'or more in magnitude there, their difference being beyond the largest double'),
    ('BEFORE', r'pow_up_without_overflow_exception|pow_rounded_up<true>',
     'the write sets the exception masks of MXCSR and leaves the direction; what comes before it only decides '
     'whether the masks are needed, with margins far larger than the rounding of the estimate '
     '(1.4426950408889634 is rounded to nearest on purpose)'),
    ('BEFORE', r'pown_checked',
     'the path of the embedded rounding (GAOL_POWN_ER) writes MXCSR only to clear the modes that flush the '
     'subnormals, and sets no direction: its instructions carry theirs, and the bound K of the error is computed '
     'before, in the direction of the program. K = (8 m L + 16) 2^-104 is exact (m L below 2^53, a power of 2), '
     'and the cube of the inverse, m^3 2^-155, is within 2^-52 of its upward rounding where the proof of K has '
     'a factor of 16 (gaol/core/gaol_interval_core.cpp, pown_bound_inverse())'),
]


# The functions that write the control register, whose call is a write seen from the caller
WRITE_CALLS = re.compile(r'(fesetround|fesetenv|feupdateenv|round_upward_if_needed|round_upward|round_nearest|'
                         r'round_downward|set_rounding|set_flush_modes|clear_flush_to_zero|restore_rounding)')


def is_write(mn, ops=''):
    return WRITES.match(mn) is not None or (mn in ('call', 'callq') and WRITE_CALLS.search(ops) is not None)


def int32_source(mn, ops):
    """Whether cvtsi2sd takes a 32-bit integer: the suffix l (GCC), or a register of 32 bits (Clang writes none)"""
    if mn.endswith('l'):
        return True
    if mn.endswith('q'):
        return False
    source = ops.split(',')[0].strip()
    return re.match(r'^%(e[a-z]{2}|r\d+d)$', source) is not None


def rounds(mn, ops):
    """Whether the instruction mn with the operands ops computes with the rounding direction of the control register"""
    if EMBEDDED_ROUNDING.search(ops):
        return False
    if INT32_TO_DOUBLE.match(mn) and int32_source(mn, ops):
        return False             # every int32 and uint32 is a double
    if ARITHMETIC.match(mn) or FUSED.match(mn) or CONVERSION.match(mn) or X87.match(mn):
        return True
    if ROUNDING.match(mn):
        # roundsd and the others have the direction in the immediate, bit 2 set for the one of the register
        m = re.match(r'\s*\$(0x[0-9a-fA-F]+|\d+)', ops)
        if m is None:
            return True
        return (int(m.group(1), 0) & 4) != 0
    return False


class Insn:
    __slots__ = ('index', 'mnemonic', 'operands', 'text', 'line', 'run')

    def __init__(self, index, mnemonic, operands, text, line, run):
        self.index, self.mnemonic, self.operands, self.text, self.line, self.run = index, mnemonic, operands, text, line, run


class Assembly:
    """The instructions of one assembly file, their labels, and the functions"""

    def __init__(self, name, text):
        self.name = name
        self.insns = []
        self.labels = {}         # label -> index of the instruction that follows it
        self.functions = []      # (name, index of its entry)
        self.lines = text.splitlines()
        self._parse()

    def _parse(self):
        run = 0
        pending = []             # labels waiting for the next instruction
        function_types = set()
        for n, raw in enumerate(self.lines, 1):
            line = raw.split('#')[0].rstrip() if '#' in raw and not raw.lstrip().startswith('.') else raw.rstrip()
            s = line.strip()
            if not s:
                continue
            m = re.match(r'^\s*\.type\s+([^,\s]+)\s*,\s*[@%]function', s)
            if m:
                function_types.add(m.group(1))
                run += 1         # nothing falls through into a function, nor out of one (a call that does not return)
                continue
            if re.match(r'^\s*\.(cfi_endproc|size)\b', s):
                run += 1
                continue
            if re.match(r'^\s*\.(text|section|data|bss|pushsection|popsection|previous|subsection)\b', s):
                run += 1         # no instruction falls through a change of section
                continue
            m = re.match(r'^([A-Za-z_.$][\w.$@]*):', s)
            if m:
                label = m.group(1)
                pending.append(label)
                self.labels[label] = len(self.insns)
                if label in function_types:
                    self.functions.append((label, len(self.insns)))
                s = s[m.end():].strip()
                if not s:
                    continue
            if s.startswith('.'):
                continue         # a directive
            parts = s.split(None, 1)
            mn = parts[0]
            ops = parts[1] if len(parts) > 1 else ''
            if mn in ('rep', 'repz', 'repnz', 'lock', 'notrack', 'bnd', 'data16', 'rex.W') and ops:
                parts = ops.split(None, 1)
                mn = parts[0]
                ops = parts[1] if len(parts) > 1 else ''
            self.insns.append(Insn(len(self.insns), mn, ops, s, n, run))
            pending = []

    def target(self, ops):
        t = ops.strip().split()[0] if ops.strip() else ''
        return self.labels.get(t) if t and not t.startswith('*') else None

    def successors(self, i):
        """The indices that can follow instruction i (a call returns; a jump to a symbol that is no label leaves)"""
        ins = self.insns[i]
        mn = ins.mnemonic
        out = []
        if mn in ('ret', 'retq', 'ud2', 'hlt', 'iret', 'iretq') or mn.startswith('ret'):
            return out
        if mn == 'jmp' or mn == 'jmpq':
            t = self.target(ins.operands)
            if t is not None:
                out.append(t)
            return out
        if re.match(r'^j(n?[abceglopsz]|ae|be|ge|le|na|nb|nc|ne|ng|nl|no|np|ns|nz|pe|po|cxz|ecxz|rcxz)$', mn):
            t = self.target(ins.operands)
            if t is not None:
                out.append(t)
        if i + 1 < len(self.insns) and self.insns[i + 1].run == ins.run:
            out.append(i + 1)
        return out


def reachable(asm, entry):
    seen = {entry}
    stack = [entry]
    while stack:
        i = stack.pop()
        for j in asm.successors(i):
            if j not in seen:
                seen.add(j)
                stack.append(j)
    return seen


def back_edges(asm, entry, nodes):
    """The edges of the graph from entry that go to a node on the path being followed (loops)"""
    edges = set()
    state = {}
    stack = [(entry, iter(asm.successors(entry)))]
    state[entry] = 1
    while stack:
        i, it = stack[-1]
        for j in it:
            if state.get(j) == 1:
                edges.add((i, j))
            elif j not in state:
                state[j] = 1
                stack.append((j, iter(asm.successors(j))))
                break
        else:
            state[i] = 2
            stack.pop()
    return edges


def probe_nodes(asm, nodes):
    """(the instructions that round in a straight run from a use of a static named subnormal or tiny to the next
    jump, which are the probe of the direction (GCC loads the static, Clang adds it from memory), and the jumps
    that end these runs, which are where the check of the direction is made)"""
    probes = set()
    branches = set()
    for i in sorted(nodes):
        ins = asm.insns[i]
        if PROBE_SYMBOL.search(ins.operands) and not ins.mnemonic.startswith(('j', 'call', 'lea')):
            j = i
            while j < len(asm.insns) and asm.insns[j].run == ins.run:
                cur = asm.insns[j]
                if rounds(cur.mnemonic, cur.operands):
                    probes.add(j)
                if cur.mnemonic.startswith(('j', 'ret', 'call')):
                    if cur.mnemonic.startswith('j'):
                        branches.add(j)
                    break
                j += 1
    return probes, branches


def closure(starts, edges, barriers=()):
    """The nodes reached from starts along edges; a node of barriers is reached, and goes no further"""
    seen = set()
    stack = list(starts)
    while stack:
        i = stack.pop()
        if i in seen:
            continue
        seen.add(i)
        if i in barriers:
            continue
        stack.extend(edges.get(i, ()))
    return seen


def check_function(asm, name, entry, preserve):
    """(number of writes, number of instructions that round, number of probes, [(kind, index)])"""
    nodes = reachable(asm, entry)
    writes = [i for i in nodes if is_write(asm.insns[i].mnemonic, asm.insns[i].operands)]
    roundings = [i for i in nodes if rounds(asm.insns[i].mnemonic, asm.insns[i].operands)]
    if not writes:
        return 0, len(roundings), 0, []
    wset = set(writes)
    loops = back_edges(asm, entry, nodes)
    full = {i: list(asm.successors(i)) for i in nodes}
    dag = {i: [j for j in full[i] if (i, j) not in loops] for i in nodes}
    reverse = {}
    for graph, key in ((full, 'full'), (dag, 'dag')):
        pred = collections.defaultdict(list)
        for i, js in graph.items():
            for j in js:
                pred[j].append(i)
        reverse[key] = pred
    # BEFORE, on the graph without the edges that return to a loop: reached from the entry without a write
    # before it, and from which a write can be reached
    probes, branches = probe_nodes(asm, nodes)
    free_dag = closure([entry], dag, wset | branches)
    leads_dag = closure(writes, reverse['dag'])
    # AFTER, on the whole graph (the body of a loop between two writes leads to the second): reached from a
    # write without another write before it, not from the entry without a write, and from which no write
    # can be reached
    free_full = closure([entry], full, wset)
    leads_full = closure(writes, reverse['full'])
    after = closure([j for w in writes for j in full[w]], full, wset)
    found = []
    for i in sorted(roundings):
        if i in probes:
            continue
        if i in free_dag and i in leads_dag:
            found.append(('BEFORE', i))
        elif preserve and i in after and i not in leads_full and i not in free_full:
            found.append(('AFTER', i))
    return len(writes), len(roundings), len(probes & set(roundings)), found


def demangle(names):
    try:
        r = subprocess.run(['c++filt'], input='\n'.join(names), capture_output=True, text=True, check=True)
        out = r.stdout.splitlines()
        return dict(zip(names, out)) if len(out) == len(names) else {n: n for n in names}
    except (OSError, subprocess.CalledProcessError):
        return {n: n for n in names}


KNOWN_SEEN = set()


def known(kind, name):
    """Why a finding of this kind in this function is harmless, or None"""
    for k, pattern, why in KNOWN:
        if k == kind and re.search(pattern, name):
            return why
    return None


def check_file(path, text, preserve, verbose, out):
    asm = Assembly(path, text)
    names = [f for f, _ in asm.functions]
    pretty = demangle(names)
    stats = collections.Counter()
    seen_functions = set()
    bad = 0
    for name, entry in asm.functions:
        w, r, p, found = check_function(asm, name, entry, preserve)
        stats['functions'] += 1
        if w:
            stats['with writes'] += 1
            stats['writes'] += w
            stats['instructions that round (in them)'] += r
            stats['probes'] += p
        for kind, i in found:
            ins = asm.insns[i]
            fname = pretty.get(name, name)
            why = known(kind, fname)
            if why is not None:
                stats['known findings'] += 1
                KNOWN_SEEN.add(why)
                if verbose:
                    out.append('%s:%d: %s in %s: known' % (path, ins.line, kind, fname))
                continue
            bad += 1
            stats['functions with a finding'] += 1 if (name, 'f') not in seen_functions else 0
            seen_functions.add((name, 'f'))
            out.append('%s:%d: %s in %s' % (path, ins.line, kind, fname))
            out.append('    %s' % ins.text)
            if verbose:
                lo = max(ins.line - 6, 1)
                for k in range(lo, min(ins.line + 5, len(asm.lines) + 1)):
                    out.append('    %4d  %s' % (k, asm.lines[k - 1]))
    return stats, bad


def compile_commands(build, keep):
    """Compiles the sources of the library of the build with -S; the paths of the assembly files"""
    db = json.load(open(os.path.join(build, 'compile_commands.json')))
    out_dir = os.path.join(keep or build, 'asm-order') if keep is None else keep
    os.makedirs(out_dir, exist_ok=True)
    paths = []
    for entry in db:
        f = entry['file']
        if not f.endswith(('.cpp', '.c')) or '/gaol/' not in f or '/3rd/' in f:
            continue
        args = shlex.split(entry['command']) if 'command' in entry else list(entry['arguments'])
        cleaned, skip = [], False
        for a in args:
            if skip:
                skip = False
                continue
            if a == '-o':
                skip = True
                continue
            if a == '-c':
                continue
            cleaned.append(a)
        name = os.path.relpath(f, entry['directory']).replace(os.sep, '_').replace('..', '').strip('_') + '.s'
        target = os.path.join(out_dir, name)
        r = subprocess.run(cleaned + ['-S', '-o', target], cwd=entry['directory'], capture_output=True, text=True)
        if r.returncode != 0:
            sys.exit('check_order.py: %s does not compile with -S:\n%s' % (f, r.stderr[-2000:]))
        paths.append(target)
    return paths


SELFTEST = {
    'sum before the write': ('''
	.type	f, @function
f:
	addsd	%xmm1, %xmm0
	movl	$16, %eax
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'sum after the write': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	ldmxcsr	(%rsp)
	addsd	%xmm1, %xmm0
	ldmxcsr	4(%rsp)
	ret
''', None, False),
    'sum after the write that sets back': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	ldmxcsr	(%rsp)
	ldmxcsr	4(%rsp)
	mulsd	%xmm1, %xmm0
	ret
''', 'AFTER', True),
    'the probe, then the write on a slow path': ('''
	.type	f, @function
f:
	movsd	_ZZ17round_upward_if_neededvE9subnormal(%rip), %xmm2
	addsd	.LC1(%rip), %xmm2
	addsd	.LC0(%rip), %xmm2
	ucomisd	.LC0(%rip), %xmm2
	ja	.L2
	ldmxcsr	(%rsp)
.L2:
	addsd	%xmm1, %xmm0
	ret
''', None, False),
    'a sum hoisted above the probe': ('''
	.type	f, @function
f:
	addsd	%xmm1, %xmm0
	movsd	_ZZ17round_upward_if_neededvE9subnormal(%rip), %xmm2
	addsd	.LC0(%rip), %xmm2
	ucomisd	.LC0(%rip), %xmm2
	ja	.L2
	ldmxcsr	(%rsp)
.L2:
	addsd	%xmm1, %xmm0
	ret
''', 'BEFORE', False),
    'a sum with an embedded rounding': ('''
	.type	f, @function
f:
	vaddsd	{ru-sae}, %xmm1, %xmm0, %xmm0
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'a rounding of the immediate': ('''
	.type	f, @function
f:
	roundsd	$9, %xmm0, %xmm1
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'a rounding of the register': ('''
	.type	f, @function
f:
	roundsd	$12, %xmm0, %xmm1
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'the check is a call, the sum is after it': ('''
	.type	f, @function
f:
	call	fegetround@PLT
	call	_ZN9gaol_core22round_upward_if_neededEv
	mulsd	%xmm1, %xmm0
	call	fesetround@PLT
	ldmxcsr	(%rsp)
	ret
''', None, True),
    'the sum is before the call that checks': ('''
	.type	f, @function
f:
	call	fegetround@PLT
	mulsd	%xmm1, %xmm0
	call	_ZN9gaol_core22round_upward_if_neededEv
	call	fesetround@PLT
	ret
''', 'BEFORE', True),
    'a conversion of an int32 and of an int64': ('''
	.type	f, @function
f:
	cvtsi2sdl	%edi, %xmm0
	vcvtsi2sd	%ecx, %xmm1, %xmm2
	cvtsi2sdq	%rax, %xmm3
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a loop that writes each time': ('''
	.type	f, @function
f:
.L1:
	ldmxcsr	(%rsp)
	addsd	%xmm1, %xmm0
	ldmxcsr	4(%rsp)
	subl	$1, %edi
	jne	.L1
	ret
''', None, True),
}


def selftest():
    bad = 0
    for name, (text, want, preserve) in sorted(SELFTEST.items()):
        out = []
        _, found = check_file(name, text, preserve, False, out)
        kinds = sorted(set(l.split(': ')[1].split()[0] for l in out if ': ' in l and not l.startswith('    ')))
        ok = (kinds == []) if want is None else (kinds == [want])
        print('%-44s %s' % (name, 'ok' if ok else 'FAILED: %r wanted, %r found' % (want, kinds)))
        bad += 0 if ok else 1
    return bad


def main():
    p = argparse.ArgumentParser(description='Instructions that round, on the wrong side of a write of the control register.')
    p.add_argument('files', nargs='*', metavar='FILE.s', help='assembly files of x86-64')
    p.add_argument('--build', metavar='DIR', help='a CMake build (with compile_commands.json) whose library is compiled with -S')
    p.add_argument('--keep', metavar='DIR', help='where --build writes the assembly (DIR/asm-order of the build by default)')
    p.add_argument('--preserve', action='store_true', help='the build has GAOL_PRESERVE_ROUNDING: look after the writes too')
    p.add_argument('--verbose', action='store_true', help='print the lines around each finding')
    p.add_argument('--selftest', action='store_true', help='check the checks on assembly written for it, and stop')
    a = p.parse_args()
    if a.selftest:
        sys.exit(1 if selftest() else 0)
    files = list(a.files)
    if a.build:
        files += compile_commands(a.build, a.keep)
    if not files:
        p.error('no assembly file')
    total, bad = collections.Counter(), 0
    for f in files:
        out = []
        stats, n = check_file(f, open(f, encoding='utf-8', errors='replace').read(), a.preserve, a.verbose, out)
        total.update(stats)
        bad += n
        print('\n'.join(out))
    print('%d files; %s; findings: %d' % (len(files), ', '.join('%s %d' % kv for kv in sorted(total.items())), bad))
    if a.build:
        for k, pattern, why in KNOWN:
            if why not in KNOWN_SEEN:
                print('known finding not seen in this build (out of date, or in the other configurations): %s %s' % (k, pattern))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
