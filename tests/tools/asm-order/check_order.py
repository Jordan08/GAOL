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
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON) has the sources of gaol/ compiled again
# with -S and the flags it compiled them with, and inline.cpp of this
# directory, which calls the inline functions of the headers that make their
# own check (width(), rad(), mid_rad()), with the flags of gaol_interval.cpp;
# the assembly is written into DIR/asm-order (or into --keep). The checks, for
# each function that holds a write of the control register, on the paths of
# its control flow graph (the jump tables of the switch statements are
# followed; calls are taken as they are: what a callee does is read in the
# callee, the code of which is read as well):
#
# - BEFORE: an instruction that rounds, other than the probe of the direction
#   (1 + (2^-1060 + 0), whose sums are meant to be computed in the direction
#   found: the instructions that round with a register that holds the static
#   named subnormal or tiny, or a sum of it, up to the jump that tells the
#   direction), that the function runs before the write on a path from its
#   entry to that write, and before the jump of the probe. Instructions that
#   the function runs without a write, because the probe found the direction
#   upward, are not these: only those with a write after them. Where the
#   writes are conditional (the code after one is reached without it), the
#   jump that goes around the write, after a read of the register (stmxcsr,
#   fnstcw, a call of fegetround()), is where the check is made, like the jump
#   of the probe: the instructions before it are the ones that precede the
#   check, and past it the register holds what the write would have put there. A conversion
#   of a 32-bit integer to a double is not one either: it is exact. The edges
#   that go back to a loop are left out, so that the instructions of an
#   iteration are not taken for the ones before the write of the next.
# - AFTER (with --preserve, for the builds with GAOL_PRESERVE_ROUNDING, whose
#   operations write the register back): an instruction that rounds, after a
#   write with no other write after it, and that the entry cannot reach
#   without a write. A function whose writes are conditional (the ones of
#   round_upward_if_needed(), of flush_guard, of the registers of ARM and of
#   the processors of 32-bit x86) reaches all of its code without one, and
#   the check cannot tell a result computed after the write from one computed
#   before it there.
# - BLIND: an instruction that rounds or writes the register that no function
#   of the file reaches (a jump table the check could not follow). The
#   indirect jumps whose table is not found are counted.
#
# Not seen: the code a callee runs; the order of a computation and a write in
# two functions (the first calls the second between the two); the sections
# between the first write and the last (the evaluations in nearest of
# GAOL_RND_NEAREST_ENTER(), which only the tests that compare the results in
# every direction see: tests/rounding_direction.cpp); the instructions with an
# embedded rounding ({ru-sae}, the AVX-512 path), which carry their direction
# and do not read the register, but read the flush-to-zero modes that
# flush_guard clears and gives back; the comparisons, which read
# denormals-are-zero too; and a value that is computed in the direction of the
# program and used where upward is meant, which takes a data flow analysis
# (difference_at_least() computes the halves of its bounds after the last
# write, and the check sees it with some compilers only). The text of an asm
# statement is read as code (fldcw is written by one).
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
from fractions import Fraction
import json
import os
import re
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

# The instructions that write the control register of the rounding direction
WRITES = re.compile(r'^(v?ldmxcsr|fldcw|fldenv|frstor|fninit|v?fxrstor(64)?|x?rstor(s|s64|64)?|xrstors?)$')

# The instructions whose result depends on the rounding direction
ARITHMETIC = re.compile(r'^v?(add|sub|mul|div|sqrt|hadd|hsub|addsub|dp)(ss|sd|ps|pd)$')
FUSED = re.compile(r'^v?f(n?m(add|sub)|m(add|sub)(add|sub))(132|213|231)(ss|sd|ps|pd)$')
CONVERSION = re.compile(r'^v?cvt(si2(sd|ss)[lq]?|usi2(sd|ss)[lq]?|sd2ss|pd2ps[xy]?|sd2si[lq]?|ss2si[lq]?|pd2dq[xy]?|ps2dq'
                        r'|dq2ps|udq2ps|u?q?q2(pd|ps)[xy]?|pd2(u?q?q|udq)[xy]?|ps2(u?q?q|udq)|sd2usi|ss2usi|ps2ph)$')
ROUNDING = re.compile(r'^v?(round(ss|sd|ps|pd)|rndscale(ss|sd|ps|pd)|reduce(ss|sd|ps|pd)|scalef(ss|sd|ps|pd))$')
X87 = re.compile(r'^f(i?(add|sub|subr|mul|div|divr)(p|l|s)?|sqrt|rndint|i?st(p)?(l|s|ll)?|scale|prem1?|yl2x(p1)?|sin|cos|sincos|ptan|patan)$')
EMBEDDED_ROUNDING = re.compile(r'\{r[nduz]-sae\}')
INT32_TO_DOUBLE = re.compile(r'^v?cvtu?si2sd[lq]?$')
VECTOR_REGISTER = re.compile(r'%[xyz]mm\d+')

PROBE_SYMBOL = re.compile(r'(subnormal|tiny)', re.I)
# The instructions and the calls that read the control register, which a jump around a write follows
READ_REGISTER = re.compile(r'^(v?stmxcsr|fnstcw|fstcw|fnstenv|v?fxsave(64)?)$')
READ_CALLS = re.compile(r'(fegetround|fegetenv|get_rounding|get_flush_modes)')

# The findings that are right and harmless, read on the code of 2026-10-27 and 2026-10-29: kind, a regular expression
# for the name of the function (demangled), why, and for some a regular expression for the mnemonic of the
# instruction. A finding of the list is printed as "known" and does not make the check fail. It is for the function
# and the kind (and the mnemonic when it is given): another finding in these functions is hidden.
KNOWN = [
    ('AFTER', r'difference_at_least',
     'the halving 0.5 * a of the bounds of the recursive call is exact in every direction: the bounds are 2^970 '
     'or more in magnitude there, their difference being beyond the largest double (seen with Clang 18 and 21 '
     'and the direction preserved; GCC turns the recursion into a loop that leads back to a write)'),
    ('BEFORE', r'pow_up_without_overflow_exception|pow_rounded_up<true>',
     'the write sets the exception masks of MXCSR and leaves the direction; what comes before it only decides '
     'whether the masks are needed, with margins far larger than the rounding of the estimate '
     '(1.4426950408889634 is rounded to nearest on purpose)'),
    # GCC inlines pow_up_without_overflow_exception() (and pow_rounded_up<true>()) into the callers below instead of
    # keeping the function that the entry above names, and the same estimates come before its write: the products
    # |y| |log(x)| 1.44... and the quotient 1000/(e + 1) that decide whether the masks are needed (point PG, 2026-10-29)
    ('BEFORE', r'pow_lo<true>|pow_hi<true>|log_ratio_(up|dn)\(double, double\)::|'
               r'root_of_power_(up|dn)\(double, double, double\)::|pow_rel_beyond|root_end|log_ratio_end',
     'the estimates of the inlined pow_up_without_overflow_exception(): the products and the quotient that decide '
     'whether the exceptions are masked, with margins (1000/(e + 1), 1023.999999999, 1024.000000001) far larger '
     'than their rounding', r'^v?(mul|div|sub)sd$'),
    # The hints of the search for the tightest bound of c^(1/y) and of log(c)/log(x): they only say where to start
    ('BEFORE', r'root_end|log_ratio_end',
     'root_of_power_near() and log_ratio_near() compute a hint, from which last_key_holding() and '
     'first_key_holding() look for the bound by gallop and bisection of a monotone proof: the bound is the same '
     'for any hint (the tests pass with the two hints replaced by constants, 2026-10-29), only the number of '
     'powers computed changes', r'^v?(mul|div|sub)sd$'),
]

# The functions that write the control register, whose call is a write seen from the caller
WRITE_CALLS = re.compile(r'(fesetround|fesetenv|feupdateenv|round_upward_if_needed|round_upward|round_nearest|'
                         r'round_downward|set_rounding|set_flush_modes|clear_flush_to_zero|restore_rounding)')


def is_write(mn, ops=''):
    if WRITES.match(mn) is not None:
        return True
    if mn in ('call', 'callq', 'jmp', 'jmpq') and not ops.strip().startswith(('*', '.L', '%')):
        return WRITE_CALLS.search(ops) is not None       # a call, or a tail call
    return False


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


def extended_is_double(words):
    """Whether the x87 constant of three 32-bit words (the low and the high half of the significand, then the sign
    and the exponent) is a double, so that a store of it as a double rounds nothing"""
    if len(words) < 3:
        return False
    low, high, sign_exponent = (w & 0xffffffff for w in words[:3])
    exponent = sign_exponent & 0x7fff
    significand = (high << 32) | low
    if exponent == 0x7fff:
        return False
    if significand == 0:
        return True
    value = Fraction(significand) * Fraction(2)**(max(exponent, 1) - 16383 - 63)
    try:
        return Fraction(float(value)) == value
    except OverflowError:
        return False


class Insn:
    __slots__ = ('mnemonic', 'operands', 'text', 'line', 'run', 'rounds')

    def __init__(self, mnemonic, operands, text, line, run, rounds_):
        self.mnemonic, self.operands, self.text, self.line, self.run, self.rounds = mnemonic, operands, text, line, run, rounds_


JUMP_CONDITIONAL = re.compile(r'^j(n?[abceglopsz]|ae|be|ge|le|na|nb|nc|ne|ng|nl|no|np|ns|nz|pe|po|cxz|ecxz|rcxz)$')
LABEL_REFERENCE = re.compile(r'\.L[\w.$]+')


class Assembly:
    """The instructions of one assembly file, their labels, the jump tables and the functions"""

    def __init__(self, text):
        self.insns = []
        self.labels = {}         # label -> index of the instruction that follows it
        self.functions = []      # (name, index of its entry)
        self.entries = set()     # the indices of the entries of the functions
        self.tables = {}         # label of a jump table -> the labels it holds
        self.unresolved = set()  # the indirect jumps whose table is not known
        self.pads = set()        # the labels the tables of exceptions name, where the unwinder jumps to
        self.longs = {}          # label of a constant -> the 32-bit numbers of its .long directives
        self.stores = []         # (instruction index, label) of the fst* that follows a fldt of a constant
        self.lines = text.splitlines()
        self._parse()
        self._exact_constants()

    def _parse(self):
        run = 0
        function_types = set()
        last_label = None
        for n, raw in enumerate(self.lines, 1):
            s = raw.strip()
            if not s or s.startswith('#'):
                continue
            if '#' in s and not s.startswith('.'):
                s = s.split('#')[0].strip()
            m = re.match(r'^\.type\s+([^,\s]+)\s*,\s*[@%]function', s)
            if m:
                function_types.add(m.group(1))
                run += 1         # nothing falls through into a function, nor out of one (a call that does not return)
                continue
            if re.match(r'^\.(cfi_endproc|size)\b', s):
                run += 1
                continue
            if re.match(r'^\.(text|section|data|bss|pushsection|popsection|previous|subsection)\b', s):
                run += 1         # no instruction falls through a change of section
                continue
            m = re.match(r'^([A-Za-z_.$][\w.$@]*):', s)
            if m:
                label = m.group(1)
                self.labels[label] = len(self.insns)
                last_label = label
                if label in function_types:
                    self.functions.append((label, len(self.insns)))
                    self.entries.add(len(self.insns))
                s = s[m.end():].strip()
                if not s:
                    continue
            m = re.match(r'^\.uleb128\s+(\.L[\w.$]+)-(\.Lfunc_begin\d+|\.LFB\d+)\b', s)
            if m:
                self.pads.add(m.group(1))        # a call site or a landing pad of a table of exceptions
                continue
            if s.startswith('.'):
                # a jump table: the .long .L5-.L4 or .quad .L5 that follow its label
                if last_label is not None and re.match(r'^\.(long|quad|word|4byte|8byte)\b', s):
                    targets = [t for t in LABEL_REFERENCE.findall(s) if t != last_label]
                    if targets:
                        self.tables.setdefault(last_label, []).extend(targets)
                    number = re.match(r'^\.long\s+(-?\d+)\s*$', s)
                    if number:
                        self.longs.setdefault(last_label, []).append(int(number.group(1)))
                continue
            parts = s.split(None, 1)
            mn = parts[0]
            ops = parts[1] if len(parts) > 1 else ''
            if mn in ('rep', 'repz', 'repnz', 'lock', 'notrack', 'bnd', 'data16', 'rex.W') and ops:
                parts = ops.split(None, 1)
                mn = parts[0]
                ops = parts[1] if len(parts) > 1 else ''
            rounding = rounds(mn, ops)
            if rounding and mn.startswith('fst'):
                # the store of 1 or of 0 is exact (GCC loads a constant so): the x87 instruction before it, in
                # the few instructions that come before, is fld1 or fldz
                for previous in reversed(self.insns[-6:]):
                    if previous.run != run:
                        break
                    if previous.mnemonic.startswith('f'):
                        rounding = previous.mnemonic not in ('fld1', 'fldz')
                        constant = re.match(r'^(\.LC\d+)\(%rip\)$', previous.operands.strip()) if previous.mnemonic == 'fldt' else None
                        if constant:
                            self.stores.append((len(self.insns), constant.group(1)))
                        break
            self.insns.append(Insn(mn, ops, s, n, run, rounding))
            last_label = None

    def _exact_constants(self):
        """The store of a double after the load of an extended constant that is a double rounds nothing (GCC loads
        std::numeric_limits<double>::denorm_min() so: fldt .LC15(%rip), fstpl)"""
        for index, label in self.stores:
            if extended_is_double(self.longs.get(label, [])):
                self.insns[index].rounds = False

    def table_of(self, i):
        """The labels of the jump table that the indirect jump i reads, or None"""
        ins = self.insns[i]
        for label in LABEL_REFERENCE.findall(ins.operands):     # Clang: jmpq *.LJTI0_0(,%rax,8)
            if label in self.tables:
                return self.tables[label]
        j = i - 1                                               # GCC: leaq .L4(%rip), %rdx ... jmp *%rax
        while j >= 0 and i - j <= 12 and self.insns[j].run == ins.run:
            prev = self.insns[j]
            if prev.mnemonic.startswith('lea'):
                for label in LABEL_REFERENCE.findall(prev.operands):
                    if label in self.tables:
                        return self.tables[label]
            j -= 1
        return None

    def successors(self, i, entry=None):
        """The indices that can follow instruction i (a call returns; a jump to a symbol that is no label of the
        function leaves it; an indirect jump goes to the labels of its table)"""
        ins = self.insns[i]
        mn = ins.mnemonic
        out = []
        if mn.startswith('ret') or mn in ('ud2', 'hlt', 'iret', 'iretq'):
            return out
        if mn in ('jmp', 'jmpq'):
            if ins.operands.strip().startswith(('*', '%')):
                labels = self.table_of(i)
                if labels is None:
                    self.unresolved.add(i)
                    return out
                return [self.labels[l] for l in labels if l in self.labels]
            t = self.jump_target(ins.operands, entry)
            return [t] if t is not None else out
        if JUMP_CONDITIONAL.match(mn):
            t = self.jump_target(ins.operands, entry)
            if t is not None:
                out.append(t)
        if i + 1 < len(self.insns) and self.insns[i + 1].run == ins.run:
            out.append(i + 1)
        return out

    def jump_target(self, ops, entry):
        t = ops.strip().split()[0] if ops.strip() else ''
        index = self.labels.get(t)
        if index is None or (index in self.entries and index != entry):
            return None          # a tail call to another function
        return index


def reachable(asm, entry, roots=()):
    """The nodes reached from entry, and from the landing pads in roots"""
    seen = {entry} | set(roots)
    stack = list(seen)
    while stack:
        i = stack.pop()
        for j in asm.successors(i, entry):
            if j not in seen:
                seen.add(j)
                stack.append(j)
    return seen


def back_edges(asm, entry):
    """The edges of the graph from entry that go to a node on the path being followed (loops)"""
    edges = set()
    state = {}
    stack = [(entry, iter(asm.successors(entry, entry)))]
    state[entry] = 1
    while stack:
        i, it = stack[-1]
        for j in it:
            if state.get(j) == 1:
                edges.add((i, j))
            elif j not in state:
                state[j] = 1
                stack.append((j, iter(asm.successors(j, entry))))
                break
        else:
            state[i] = 2
            stack.pop()
    return edges


def registers(operands):
    """(the registers read, the register written) of a vector instruction, AT&T order: the last operand is written"""
    regs = VECTOR_REGISTER.findall(operands)
    if not regs:
        return [], None
    return regs[:-1], regs[-1]


def probe_nodes(asm, nodes):
    """(the instructions that round in a straight run from a use of a static named subnormal or tiny to the next
    jump with a register that holds the static or a sum of it, which are the probe of the direction (GCC loads
    the static, Clang adds it from memory), and the jumps that end these runs, which are where the direction is
    told)"""
    probes = set()
    branches = set()
    for i in sorted(nodes):
        ins = asm.insns[i]
        if not PROBE_SYMBOL.search(ins.operands) or ins.mnemonic.startswith(('j', 'call', 'lea')):
            continue
        tainted = set()
        j = i
        while j < len(asm.insns) and asm.insns[j].run == ins.run:
            cur = asm.insns[j]
            sources, destination = registers(cur.operands)
            if not cur.mnemonic.startswith('v') and destination is not None:
                sources = sources + [destination]       # the two-operand form reads the register it writes
            if j == i or any(r in tainted for r in sources):
                if rounds(cur.mnemonic, cur.operands):
                    probes.add(j)
                if destination is not None:
                    tainted.add(destination)
            elif destination is not None:
                tainted.discard(destination)
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


def check_function(asm, entry, pads, preserve):
    """(the set of the nodes of the function, number of writes, number of instructions that round, number of
    probes, [(kind, index)])"""
    nodes = reachable(asm, entry, pads)
    writes = [i for i in nodes if is_write(asm.insns[i].mnemonic, asm.insns[i].operands)]
    roundings = [i for i in nodes if asm.insns[i].rounds]
    if not writes:
        return nodes, 0, len(roundings), 0, []
    wset = set(writes)
    loops = back_edges(asm, entry)
    full = {i: list(asm.successors(i, entry)) for i in nodes}
    dag = {i: [j for j in full[i] if (i, j) not in loops] for i in nodes}
    reverse = {}
    for graph, key in ((full, 'full'), (dag, 'dag')):
        pred = collections.defaultdict(list)
        for i, js in graph.items():
            for j in js:
                pred[j].append(i)
        reverse[key] = pred
    # BEFORE, on the graph without the edges that return to a loop: reached from the entry without a write
    # before it, and from which a write can be reached. A write is conditional when the code after it can
    # be reached from the entry without it (the start and the end of an operation write only if the
    # register has to change). The jump that goes around the write, after a read of the register, is then
    # where the check is made, as the jump of the probe is: past it the register holds what the write
    # would have put there, and the edge of that jump is left out; the instructions before the jump are
    # the ones that precede the check. A jump that no read precedes checks nothing
    probes, branches = probe_nodes(asm, nodes)
    reads = {i for i in nodes if READ_REGISTER.match(asm.insns[i].mnemonic)
             or (asm.insns[i].mnemonic in ('call', 'callq') and READ_CALLS.search(asm.insns[i].operands))}
    before_read = closure([entry], dag, reads) - reads
    around_edges = set()
    for w in writes:
        around = closure([entry], dag, {w})
        around.discard(w)
        merge = around & closure(dag[w], dag)
        if not merge:
            continue
        to_write = closure([w], reverse['dag'])
        to_merge = closure(merge, reverse['dag'])
        for p in around - before_read:
            around_edges.update((p, j) for j in dag[p] if j not in to_write and j in to_merge)
    free_edges = {i: [j for j in js if (i, j) not in around_edges] for i, js in dag.items()}
    free_dag = closure([entry], free_edges, wset | branches)
    free_full = closure([entry], full, wset)
    leads_dag = closure(writes, reverse['dag'])
    # AFTER, on the whole graph (the body of a loop between two writes leads to the second): reached from a
    # write without another write before it, not from the entry without a write, and from which no write
    # can be reached
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
    return nodes, len(writes), len(roundings), len(probes & set(roundings)), found


def demangle(names):
    try:
        r = subprocess.run(['c++filt'], input='\n'.join(names), capture_output=True, text=True, check=True)
        out = r.stdout.splitlines()
        return dict(zip(names, out)) if len(out) == len(names) else {n: n for n in names}
    except (OSError, subprocess.CalledProcessError):
        return {n: n for n in names}


KNOWN_SEEN = set()


def known(kind, name, mnemonic=''):
    """Why a finding of this kind in this function (on this instruction, for the entries that name some) is
    harmless, or None"""
    for entry in KNOWN:
        k, pattern, why = entry[:3]
        if k == kind and re.search(pattern, name) and (len(entry) < 4 or re.search(entry[3], mnemonic)):
            return why
    return None


def looks_like_x86_64(text):
    return re.search(r'%(rsp|rax|rbp|xmm\d+)\b', text) is not None


def check_file(path, text, preserve, verbose, out):
    """(statistics, number of findings) of one assembly file; the findings are appended to out"""
    if not looks_like_x86_64(text):
        sys.exit('check_order.py: %s is no assembly of x86-64 (no %%rsp, %%rax, %%rbp or %%xmm)' % path)
    asm = Assembly(text)
    names = [f for f, _ in asm.functions]
    pretty = demangle(names)
    # the landing pads of each function: the labels of the tables of exceptions within its text, which only the
    # unwinder jumps to (they are in its graph so that what they write is seen, and are not reached from its entry)
    starts = sorted(entry for _, entry in asm.functions)
    pad_indices = sorted(asm.labels[l] for l in asm.pads if l in asm.labels)
    stats = collections.Counter()
    seen_functions = set()
    covered = set()
    bad = 0
    for name, entry in asm.functions:
        following = [s for s in starts if s > entry]
        end = following[0] if following else len(asm.insns)
        nodes, w, r, p, found = check_function(asm, entry, [i for i in pad_indices if entry < i < end], preserve)
        covered |= nodes
        stats['functions'] += 1
        if w:
            stats['with writes'] += 1
            stats['writes'] += w
            stats['instructions that round (in them)'] += r
            stats['probes'] += p
        for kind, i in found:
            ins = asm.insns[i]
            fname = pretty.get(name, name)
            why = known(kind, fname, ins.mnemonic)
            if why is not None:
                stats['known findings'] += 1
                KNOWN_SEEN.add(why)
                if verbose:
                    out.append('%s:%d: %s in %s: known' % (path, ins.line, kind, fname))
                continue
            bad += 1
            if name not in seen_functions:
                seen_functions.add(name)
                stats['functions with a finding'] += 1
            out.append('%s:%d: %s in %s' % (path, ins.line, kind, fname))
            out.append('    %s' % ins.text)
            if verbose:
                for k in range(max(ins.line - 6, 1), min(ins.line + 5, len(asm.lines) + 1)):
                    out.append('    %4d  %s' % (k, asm.lines[k - 1]))
    # what no function reaches: a jump table that could not be followed hides it
    for i, ins in enumerate(asm.insns):
        if i not in covered and (ins.rounds or is_write(ins.mnemonic, ins.operands)):
            bad += 1
            stats['instructions no function reaches'] += 1
            out.append('%s:%d: BLIND in no function, nothing reaches it' % (path, ins.line))
            out.append('    %s' % ins.text)
    stats['indirect jumps without a table'] += len(asm.unresolved)
    return stats, bad


def compile_commands(build, keep, root):
    """Compiles the sources of gaol/ of the build with -S, and inline.cpp; the paths of the assembly files"""
    db = json.load(open(os.path.join(build, 'compile_commands.json')))
    out_dir = keep or os.path.join(build, 'asm-order')
    os.makedirs(out_dir, exist_ok=True)
    paths = []
    first = None

    def compile(entry, source, target):
        args = shlex.split(entry['command']) if 'command' in entry else list(entry['arguments'])
        cleaned, skip = [], False
        for a in args:
            if skip:
                skip = False
            elif a == '-o':
                skip = True
            elif a != '-c':
                cleaned.append(source if a == entry['file'] else a)
        r = subprocess.run(cleaned + ['-S', '-o', target], cwd=entry['directory'], capture_output=True, text=True)
        if r.returncode != 0:
            sys.exit('check_order.py: %s does not compile with -S:\n%s' % (source, r.stderr[-2000:]))
        paths.append(target)

    for entry in db:
        f = entry['file']
        rel = os.path.relpath(f, root)
        if not f.endswith(('.cpp', '.c')) or not rel.startswith('gaol' + os.sep):
            continue
        first = first or entry
        compile(entry, f, os.path.join(out_dir, rel.replace(os.sep, '_') + '.s'))
    if first is None:
        sys.exit('check_order.py: no source of gaol/ under %s in the compile commands of %s' % (root, build))
    inline = os.path.join(HERE, 'inline.cpp')
    if os.path.exists(inline):
        # the flags of gaol_interval.cpp (or the first source of the library), for the file of this directory
        entry = next((e for e in db if e['file'].endswith('gaol_interval.cpp')), first)
        compile(entry, inline, os.path.join(out_dir, 'inline.cpp.s'))
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
    'the probe of Clang, with the static added from memory': ('''
	.type	f, @function
f:
	vxorpd	%xmm4, %xmm4, %xmm4
	vaddsd	_ZZ17round_upward_if_neededvE9subnormal(%rip), %xmm4, %xmm0
	vmovsd	.LCPI0_4(%rip), %xmm3
	vaddsd	%xmm0, %xmm3, %xmm0
	vcomisd	%xmm3, %xmm0
	jbe	.LBB0_5
.LBB0_6:
	vaddsd	%xmm2, %xmm1, %xmm1
	retq
.LBB0_5:
	ldmxcsr	(%rsp)
	jmp	.LBB0_6
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
    'a sum of the program in the window of the probe': ('''
	.type	f, @function
f:
	movsd	_ZZ17round_upward_if_neededvE9subnormal(%rip), %xmm2
	addsd	.LC0(%rip), %xmm2
	mulsd	%xmm1, %xmm0
	ucomisd	.LC0(%rip), %xmm2
	ja	.L2
	ldmxcsr	(%rsp)
.L2:
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
    'a sum, then a tail call that writes': ('''
	.type	f, @function
f:
	divsd	%xmm1, %xmm0
	jmp	fesetround@PLT
''', 'BEFORE', False),
    'a conversion of an int32 and of an int64': ('''
	.type	f, @function
f:
	cvtsi2sdl	%edi, %xmm0
	vcvtsi2sd	%ecx, %xmm1, %xmm2
	cvtsi2sdq	%rax, %xmm3
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'an int32 alone': ('''
	.type	f, @function
f:
	cvtsi2sdl	%edi, %xmm0
	vcvtsi2sd	%ecx, %xmm1, %xmm2
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'a subtraction': ('''
	.type	f, @function
f:
	subsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a square root': ('''
	.type	f, @function
f:
	vsqrtsd	%xmm1, %xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a packed division': ('''
	.type	f, @function
f:
	divpd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a fused multiply-add': ('''
	.type	f, @function
f:
	vfmadd132pd	%xmm1, %xmm2, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a store of an extended double, then a write of the x87 unit': ('''
	.type	f, @function
f:
	fstpl	8(%rsp)
	fldcw	(%rsp)
	ret
''', 'BEFORE', False),
    'a jump table of GCC, a sum in a case before the write': ('''
	.type	f, @function
f:
	cmpl	$1, %edi
	ja	.L9
	leaq	.L4(%rip), %rdx
	movslq	(%rdx,%rdi,4), %rax
	addq	%rdx, %rax
	notrack jmp	*%rax
	.section	.rodata
	.align 4
	.align 4
.L4:
	.long	.L5-.L4
	.long	.L6-.L4
	.text
.L5:
	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
.L6:
	ret
.L9:
	ret
''', 'BEFORE', False),
    'a jump table of Clang, a sum in a case before the write': ('''
	.type	f, @function
f:
	cmpl	$1, %edi
	ja	.LBB0_4
	jmpq	*.LJTI0_0(,%rax,8)
.LBB0_1:                                # %bb.1
	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	retq
.LBB0_2:
	retq
.LBB0_4:
	retq
	.section	.rodata,"a",@progbits
	.p2align	3, 0x0
.LJTI0_0:
	.quad	.LBB0_1
	.quad	.LBB0_2
''', 'BEFORE', False),
    'an indirect jump without a table, a sum behind it': ('''
	.type	f, @function
f:
	jmp	*%rax
.L5:
	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BLIND', False),
    'a part of the function in a cold section': ('''
	.type	f, @function
f:
	addsd	%xmm1, %xmm0
	jne	.L3
	ret
	.section	.text.unlikely,"ax",@progbits
.L3:
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a store of 1 through the x87 unit is exact': ('''
	.type	f, @function
f:
	fld1
	movapd	%xmm4, %xmm15
	fstpl	(%rsp)
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'a store of an extended constant that is a double is exact': ('''
	.type	f, @function
f:
	fldt	.LC15(%rip)
	movapd	%xmm4, %xmm15
	fstpl	(%rsp)
	ldmxcsr	(%rsp)
	ret
	.section	.rodata
.LC15:
	.long	0
	.long	-2147483648
	.long	15309
	.long	0
''', None, False),
    'a store of an extended constant that is not a double is not': ('''
	.type	f, @function
f:
	fldt	.LC16(%rip)
	movapd	%xmm4, %xmm15
	fstpl	(%rsp)
	ldmxcsr	(%rsp)
	ret
	.section	.rodata
.LC16:
	.long	1
	.long	-2147483648
	.long	16383
	.long	0
''', 'BEFORE', False),
    'a store of a computed x87 value is not': ('''
	.type	f, @function
f:
	fldl	8(%rsp)
	faddl	16(%rsp)
	fstpl	(%rsp)
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a landing pad that writes is no blind spot': ('''
	.type	f, @function
f:
.Lfunc_begin0:
	call	g
	ret
.Ltmp1:
	ldmxcsr	(%rsp)
	call	_Unwind_Resume@PLT
.Lfunc_end0:
	.size	f, .Lfunc_end0-f
	.section	.gcc_except_table,"a",@progbits
GCC_except_table0:
	.uleb128 .Ltmp1-.Lfunc_begin0
''', None, False),
    'a function that does not return does not fall into the next': ('''
	.type	f, @function
f:
	addsd	%xmm1, %xmm0
	call	abort@PLT
	.cfi_endproc
	.size	f, .-f
	.type	g, @function
g:
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'code in another section does not follow': ('''
	.type	f, @function
f:
	addsd	%xmm1, %xmm0
	call	abort@PLT
	.section	.text.unlikely,"ax",@progbits
	ldmxcsr	(%rsp)
	ret
''', 'BLIND', False),
    'code after a return that nothing reaches': ('''
	.type	f, @function
f:
	ret
	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BLIND', False),
    'a conditional write in a loop': ('''
	.type	f, @function
f:
.L1:
	testl	%edi, %edi
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	addsd	%xmm1, %xmm0
	subl	$1, %esi
	jne	.L1
	ret
''', None, False),
    'the probe, a body, and a write after': ('''
	.type	f, @function
f:
	movsd	_ZZ17round_upward_if_neededvE9subnormal(%rip), %xmm2
	addsd	.LC1(%rip), %xmm2
	ucomisd	.LC0(%rip), %xmm2
	ja	.L2
	ldmxcsr	(%rsp)
.L2:
	addsd	%xmm1, %xmm0
	ldmxcsr	4(%rsp)
	ret
''', None, False),
    'the probe with the static tiny': ('''
	.type	f, @function
f:
	movsd	_ZZ17round_upward_if_neededvE4tiny(%rip), %xmm2
	addsd	.LC1(%rip), %xmm2
	ucomisd	.LC0(%rip), %xmm2
	jne	.L2
	ldmxcsr	(%rsp)
.L2:
	ret
''', None, False),
    'a sum on a path that never writes': ('''
	.type	f, @function
f:
	testl	%edi, %edi
	je	.L2
	addsd	%xmm1, %xmm0
	ret
.L2:
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'a conditional write, then a sum, with the direction preserved': ('''
	.type	f, @function
f:
	testl	%edi, %edi
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	addsd	%xmm1, %xmm0
	ret
''', None, True),
    'the sum after the write that sets back, the direction not preserved': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	ldmxcsr	(%rsp)
	ldmxcsr	4(%rsp)
	mulsd	%xmm1, %xmm0
	ret
''', None, False),
    'a label and an instruction on a line': ('''
	.type	f, @function
f:
.L5:	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a known finding is not one': ('''
	.type	_ZN9gaol_coreL19difference_at_leastEdddd, @function
_ZN9gaol_coreL19difference_at_leastEdddd:
	ldmxcsr	(%rsp)
	ldmxcsr	4(%rsp)
	mulsd	%xmm1, %xmm0
	ret
''', None, True),
    'a finding of a known function on a mnemonic that its entry does not name is one': ('''
	.type	_ZN9gaol_coreL8root_endEddb, @function
_ZN9gaol_coreL8root_endEddb:
	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', False),
    'a finding of a known function on a mnemonic that its entry names is known': ('''
	.type	_ZN9gaol_coreL8root_endEddb, @function
_ZN9gaol_coreL8root_endEddb:
	mulsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', None, False),
    'conditional writes, the check first, the body between': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	movl	4(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	mulsd	%xmm1, %xmm0
	movl	%eax, %edx
	testl	%edx, %edx
	je	.L3
	stmxcsr	8(%rsp)
	ldmxcsr	12(%rsp)
.L3:
	ret
''', None, True),
    'conditional writes, a sum before the check': ('''
	.type	f, @function
f:
	mulsd	%xmm1, %xmm0
	stmxcsr	4(%rsp)
	movl	4(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	movl	%eax, %edx
	testl	%edx, %edx
	je	.L3
	stmxcsr	8(%rsp)
	ldmxcsr	12(%rsp)
.L3:
	ret
''', 'BEFORE', True),
    'conditional writes, fegetround() first, a sum before it': ('''
	.type	f, @function
f:
	mulsd	%xmm1, %xmm0
	call	fegetround@PLT
	testl	%eax, %eax
	je	.L2
	movl	$0x800, %edi
	call	fesetround@PLT
.L2:
	ret
''', 'BEFORE', True),
    'conditional writes, fegetround() first, the body after it': ('''
	.type	f, @function
f:
	call	fegetround@PLT
	testl	%eax, %eax
	je	.L2
	movl	$0x800, %edi
	call	fesetround@PLT
.L2:
	mulsd	%xmm1, %xmm0
	movl	%eax, %edi
	testl	%edi, %edi
	je	.L3
	call	fesetround@PLT
.L3:
	ret
''', None, True),
    'a conditional start, an unconditional end': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	movl	4(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	mulsd	%xmm1, %xmm0
	call	fesetround@PLT
	ret
''', None, True),
    'a conditional start, a sum before the read, an unconditional end': ('''
	.type	f, @function
f:
	mulsd	%xmm1, %xmm0
	stmxcsr	4(%rsp)
	movl	4(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	call	fesetround@PLT
	ret
''', 'BEFORE', True),
    'a read, a sum, an unconditional write': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	mulsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', 'BEFORE', True),
    'a read, a sum, an unconditional write, then a conditional one': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	mulsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	stmxcsr	8(%rsp)
	movl	8(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	ldmxcsr	12(%rsp)
.L2:
	ret
''', 'BEFORE', True),
    'a conditional start, a sum between the read and the jump': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	mulsd	%xmm1, %xmm0
	movl	4(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	ldmxcsr	(%rsp)
.L2:
	call	fesetround@PLT
	ret
''', 'BEFORE', True),
    'a conditional start, a sum in the write block': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	movl	4(%rsp), %eax
	testl	%eax, %eax
	je	.L2
	mulsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
.L2:
	call	fesetround@PLT
	ret
''', 'BEFORE', True),
    'the probe, a skip path that has its own write': ('''
	.type	f, @function
f:
	movsd	_ZZ17round_upward_if_neededvE9subnormal(%rip), %xmm2
	addsd	.LC1(%rip), %xmm2
	ucomisd	.LC0(%rip), %xmm2
	ja	.L2
	ldmxcsr	(%rsp)
	ret
.L2:
	addsd	%xmm1, %xmm0
	ldmxcsr	4(%rsp)
	ret
''', None, False),
    'a conditional write, and a path to another write': ('''
	.type	f, @function
f:
	stmxcsr	4(%rsp)
	testl	%edi, %edi
	je	.L2
	testl	%esi, %esi
	je	.L3
	ldmxcsr	(%rsp)
.L3:
	ret
.L2:
	addsd	%xmm1, %xmm0
	ldmxcsr	4(%rsp)
	ret
''', 'BEFORE', True),
    'a jump around the write that no read precedes': ('''
	.type	f, @function
f:
	testl	%edi, %edi
	je	.LJ
	stmxcsr	4(%rsp)
	movl	4(%rsp), %eax
	cmpl	$0x4000, %eax
	je	.LJ
	ldmxcsr	(%rsp)
.LJ:
	mulsd	%xmm1, %xmm0
	movl	%eax, %edx
	testl	%edx, %edx
	je	.L3
	stmxcsr	8(%rsp)
	ldmxcsr	12(%rsp)
.L3:
	ret
''', 'BEFORE', True),
    'a jump around the write after a read of fegetround()': ('''
	.type	f, @function
f:
	call	fegetround@PLT
	testl	%eax, %eax
	je	.LJ
	movl	$0x800, %edi
	call	fesetround@PLT
.LJ:
	mulsd	%xmm1, %xmm0
	movl	%eax, %edx
	testl	%edx, %edx
	je	.L3
	call	fesetround@PLT
.L3:
	ret
''', None, True),
    'a tail call to another function is no arc': ('''
	.type	f, @function
f:
	jmp	g
	.type	g, @function
g:
	addsd	%xmm1, %xmm0
	ldmxcsr	(%rsp)
	ret
''', ('BEFORE', 'g'), False),
}


def selftest():
    """Runs the checks on the assembly of SELFTEST: the kinds of the findings must be the one wanted, or the pair
    (kind, function) where the case names the function"""
    bad = 0
    for name, (text, want, preserve) in sorted(SELFTEST.items()):
        out = []
        check_file(name, text, preserve, False, out)
        found = sorted(set(tuple(l.split(': ')[1].replace(',', '').split()[0:3:2]) for l in out
                           if ': ' in l and not l.startswith('    ')))
        if isinstance(want, tuple):
            ok = found == [want]
        elif want is None:
            ok = found == []
        else:
            ok = [kind for kind, _ in found] == [want]
        print('%-60s %s' % (name, 'ok' if ok else 'FAILED: %r wanted, %r found' % (want, found)))
        bad += 0 if ok else 1
    return bad


def main():
    p = argparse.ArgumentParser(description='Instructions that round, on the wrong side of a write of the control register.')
    p.add_argument('files', nargs='*', metavar='FILE.s', help='assembly files of x86-64')
    p.add_argument('--build', metavar='DIR', help='a CMake build (with compile_commands.json) whose library is compiled with -S')
    p.add_argument('--root', default=os.path.normpath(os.path.join(HERE, '..', '..', '..')),
                   help='the sources of GAOL, for --build (%(default)s)')
    p.add_argument('--keep', metavar='DIR', help='where --build writes the assembly (DIR/asm-order of the build by default)')
    p.add_argument('--preserve', action='store_true', help='the build has GAOL_PRESERVE_ROUNDING: look after the writes too')
    p.add_argument('--verbose', action='store_true', help='print the lines around each finding, and the known findings')
    p.add_argument('--selftest', action='store_true', help='check the checks on assembly written for it, and stop')
    a = p.parse_args()
    if a.selftest:
        sys.exit(1 if selftest() else 0)
    files = list(a.files)
    if a.build:
        files += compile_commands(a.build, a.keep, os.path.abspath(a.root))
    if not files:
        p.error('no assembly file')
    total, bad = collections.Counter(), 0
    for f in files:
        out = []
        try:
            text = open(f, encoding='utf-8', errors='replace').read()
        except OSError as e:
            sys.exit('check_order.py: %s' % e)
        stats, n = check_file(f, text, a.preserve, a.verbose, out)
        total.update(stats)
        bad += n
        if out:
            print('\n'.join(out))
    if a.build and total['with writes'] == 0:
        sys.exit('check_order.py: no function of the build writes the control register: nothing was checked')
    print('%d files; %s; findings: %d' % (len(files), ', '.join('%s %d' % kv for kv in sorted(total.items())), bad))
    if a.build and a.verbose:
        for entry in KNOWN:
            k, pattern, why = entry[:3]
            if why not in KNOWN_SEEN:
                print('known finding not seen in this build (in the other configurations, or out of date): %s %s' % (k, pattern))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
