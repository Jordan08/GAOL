#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: the mutants of the rounding direction of a 32-bit x86
# processor, for tests/tools/pow/mutate.py (see README.md). Each is
# gaol/gaol_fpu.h with one deliberate error in the functions gaol_core::x86_*(),
# which decide what to write in the control word of the x87 unit and in MXCSR,
# and which tests/rounding_direction.cpp runs on every platform, on registers
# simulated by a type of its own (x86_registers_simulated()); the test has to
# fail on it, unless it makes the same decisions (an equivalent mutant, which
# the list says).
#
#     python3 tests/tools/pow/mutate.py --mutants tests/tools/x86-csr/mutants.py \
#             --test rounding_direction --no-diffpow
#
# The constants x86_flush_mask, sse_rc_mask and sse_rc_upward are not mutated:
# a static_assert compares them with those of <xmmintrin.h>, and a mutant
# that fails to compile is no mutant. The processor itself (x86_registers,
# gaol_read_x86_control()) is not here either: only a 32-bit x86 processor runs
# it (x86_registers_on_the_processor() of the test).
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-27 by Jordan NININ (from the 46 mutants of the work of #156,
# whose file was lost, rewritten for the code of configure-clean)

F = 'gaol/gaol_fpu.h'


def m(name, old, new, survives=None):
    d = dict(name=name, file=F, edits=[(old, new)])
    if survives:
        d['survives'] = survives
    return d


MUTANTS = [
    # The fields
    m('x87-mask-too-narrow', 'const unsigned int x87_rc_mask = 0x0c00u;', 'const unsigned int x87_rc_mask = 0x0800u;'),
    m('x87-mask-too-wide', 'const unsigned int x87_rc_mask = 0x0c00u;', 'const unsigned int x87_rc_mask = 0x0e00u;'),
    m('x87-upward-wrong', 'const unsigned int x87_rc_upward = 0x0800u;', 'const unsigned int x87_rc_upward = 0x0400u;'),
    m('x87-upward-is-FE_UPWARD', 'const unsigned int x87_rc_upward = 0x0800u;',
      'const unsigned int x87_rc_upward = static_cast<unsigned int>(FE_UPWARD);',
      survives='FE_UPWARD is 0x0800 with glibc: only a host whose <fenv.h> says otherwise (macOS, Windows) kills it'),

    # x86_is_upward()
    m('is-upward-or', 'return (x87_cw & x87_rc_mask) == x87_rc_upward\n        && (mxcsr',
      'return (x87_cw & x87_rc_mask) == x87_rc_upward\n        || (mxcsr'),
    m('is-upward-ignores-sse', '''return (x87_cw & x87_rc_mask) == x87_rc_upward
        && (mxcsr & (sse_rc_mask | x86_flush_mask)) == sse_rc_upward;''',
      'return (x87_cw & x87_rc_mask) == x87_rc_upward;'),
    m('is-upward-ignores-x87', '''return (x87_cw & x87_rc_mask) == x87_rc_upward
        && (mxcsr & (sse_rc_mask | x86_flush_mask)) == sse_rc_upward;''',
      'return (mxcsr & (sse_rc_mask | x86_flush_mask)) == sse_rc_upward;'),
    m('is-upward-ignores-flush', '(mxcsr & (sse_rc_mask | x86_flush_mask)) == sse_rc_upward;',
      '(mxcsr & sse_rc_mask) == sse_rc_upward;'),

    # x86_set_modes()
    m('set-modes-x87-always', 'if ((cw & x87_rc_mask) != x87_rc) {', 'if (true) {'),
    m('set-modes-x87-never', 'if ((cw & x87_rc_mask) != x87_rc) {', 'if (false) {'),
    m('set-modes-sse-always', 'if ((csr & sse_mask) != sse_bits) {', 'if (true) {'),
    m('set-modes-sse-never', 'if ((csr & sse_mask) != sse_bits) {', 'if (false) {'),
    m('set-modes-x87-compares-the-whole-word', 'if ((cw & x87_rc_mask) != x87_rc) {', 'if (cw != x87_rc) {'),
    m('set-modes-sse-compares-the-whole-register', 'if ((csr & sse_mask) != sse_bits) {', 'if (csr != sse_bits) {'),
    m('set-modes-x87-overwritten', 'r.write_x87_control((cw & ~x87_rc_mask) | x87_rc);', 'r.write_x87_control(x87_rc);'),
    m('set-modes-sse-overwritten', 'r.write_mxcsr((csr & ~sse_mask) | sse_bits);', 'r.write_mxcsr(sse_bits);'),
    m('set-modes-flags-cleared', 'r.write_mxcsr((csr & ~sse_mask) | sse_bits);',
      'r.write_mxcsr((csr & ~sse_mask & ~0x3fu) | sse_bits);'),
    m('set-modes-x87-write-wide', 'r.write_x87_control((cw & ~x87_rc_mask) | x87_rc);',
      'r.write_x87_control((cw & ~x87_rc_mask) | x87_rc | 0x10000u);'),

    # x86_set_upward()
    m('set-upward-x87-nearest', 'x86_set_modes(r, x87_rc_upward, sse_rc_mask | x86_flush_mask, sse_rc_upward);',
      'x86_set_modes(r, 0u, sse_rc_mask | x86_flush_mask, sse_rc_upward);'),
    m('set-upward-sse-nearest', 'x86_set_modes(r, x87_rc_upward, sse_rc_mask | x86_flush_mask, sse_rc_upward);',
      'x86_set_modes(r, x87_rc_upward, sse_rc_mask | x86_flush_mask, 0u);'),
    m('set-upward-flush-kept', 'x86_set_modes(r, x87_rc_upward, sse_rc_mask | x86_flush_mask, sse_rc_upward);',
      'x86_set_modes(r, x87_rc_upward, sse_rc_mask, sse_rc_upward);'),

    # x86_enter_registers()
    m('enter-always', 'if (GAOL_UNLIKELY(!x86_is_upward(x87_cw, mxcsr))) {', 'if (true) {'),
    m('enter-never', 'if (GAOL_UNLIKELY(!x86_is_upward(x87_cw, mxcsr))) {', 'if (false) {'),
    m('enter-inverted', 'if (GAOL_UNLIKELY(!x86_is_upward(x87_cw, mxcsr))) {',
      'if (GAOL_UNLIKELY(x86_is_upward(x87_cw, mxcsr))) {'),

    # x86_get_state()
    m('get-state-x87-unmasked', 's.x87 = cw & x87_rc_mask;', 's.x87 = cw;'),
    m('get-state-x87-from-sse', 's.x87 = cw & x87_rc_mask;', 's.x87 = (csr & sse_rc_mask) >> 3;'),
    m('get-state-sse-from-x87', 's.sse = csr & sse_rc_mask;', 's.sse = (cw & x87_rc_mask) << 3;'),
    m('get-state-sse-unmasked', 's.sse = csr & sse_rc_mask;', 's.sse = csr;'),
    m('get-state-flush-lost', 's.flush = csr & x86_flush_mask;', 's.flush = 0u;'),
    m('get-state-daz-lost', 's.flush = csr & x86_flush_mask;', 's.flush = csr & 0x8000u;'),
    m('get-state-ftz-lost', 's.flush = csr & x86_flush_mask;', 's.flush = csr & 0x0040u;'),

    # x86_enter_upward()
    m('enter-upward-ignores-flush', 'x86_enter_registers(r, s.x87, s.sse | s.flush);',
      'x86_enter_registers(r, s.x87, s.sse);'),
    m('enter-upward-ignores-x87', 'x86_enter_registers(r, s.x87, s.sse | s.flush);',
      'x86_enter_registers(r, x87_rc_upward, s.sse | s.flush);'),
    m('enter-upward-ignores-sse', 'x86_enter_registers(r, s.x87, s.sse | s.flush);',
      'x86_enter_registers(r, s.x87, sse_rc_upward | s.flush);'),

    # x86_set_rounding(): the directions, not the modes
    m('set-rounding-flush-too', 'x86_set_modes(r, s.x87, sse_rc_mask, s.sse);',
      'x86_set_modes(r, s.x87, sse_rc_mask | x86_flush_mask, s.sse);'),
    m('set-rounding-x87-upward', 'x86_set_modes(r, s.x87, sse_rc_mask, s.sse);',
      'x86_set_modes(r, x87_rc_upward, sse_rc_mask, s.sse);'),
    m('set-rounding-sse-upward', 'x86_set_modes(r, s.x87, sse_rc_mask, s.sse);',
      'x86_set_modes(r, s.x87, sse_rc_mask, sse_rc_upward);'),
    m('set-rounding-sse-ignored', 'x86_set_modes(r, s.x87, sse_rc_mask, s.sse);',
      'x86_set_modes(r, s.x87, 0u, 0u);'),

    # x86_set_rounding_and_flush_modes()
    m('set-all-flush-lost', 'x86_set_modes(r, s.x87, sse_rc_mask | x86_flush_mask, s.sse | s.flush);',
      'x86_set_modes(r, s.x87, sse_rc_mask | x86_flush_mask, s.sse);'),
    m('set-all-flush-not-written', 'x86_set_modes(r, s.x87, sse_rc_mask | x86_flush_mask, s.sse | s.flush);',
      'x86_set_modes(r, s.x87, sse_rc_mask, s.sse);'),
    m('set-all-x87-upward', 'x86_set_modes(r, s.x87, sse_rc_mask | x86_flush_mask, s.sse | s.flush);',
      'x86_set_modes(r, x87_rc_upward, sse_rc_mask | x86_flush_mask, s.sse | s.flush);'),

    # x86_leave()
    m('leave-always', 'if (GAOL_UNLIKELY(!x86_is_upward(s.x87, s.sse | s.flush))) {', 'if (true) {'),
    m('leave-never', 'if (GAOL_UNLIKELY(!x86_is_upward(s.x87, s.sse | s.flush))) {', 'if (false) {'),
    m('leave-ignores-x87', 'if (GAOL_UNLIKELY(!x86_is_upward(s.x87, s.sse | s.flush))) {',
      'if (GAOL_UNLIKELY(!x86_is_upward(x87_rc_upward, s.sse | s.flush))) {'),
    m('leave-ignores-flush', 'if (GAOL_UNLIKELY(!x86_is_upward(s.x87, s.sse | s.flush))) {',
      'if (GAOL_UNLIKELY(!x86_is_upward(s.x87, s.sse))) {'),
    m('leave-restores-directions-only', 'x86_set_rounding_and_flush_modes(r, s);\n    }\n  }\n\n  #if GAOL_RND_X86_DIRECT || GAOL_RND_MINGW64_X87',
      'x86_set_rounding(r, s);\n    }\n  }\n\n  #if GAOL_RND_X86_DIRECT || GAOL_RND_MINGW64_X87'),
]
