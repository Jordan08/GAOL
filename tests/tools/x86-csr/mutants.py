#---------------------------------------------------------------------------
# gaol -- NOT Just Another Interval Library
#---------------------------------------------------------------------------
# Tools of GAOL v5: the mutants of the rounding direction of a 32-bit x86
# processor, for tests/tools/pow/mutate.py (see README.md). Each is
# gaol/gaol_fpu.h with one deliberate error in the functions gaol_core::x86_*(),
# which decide what to write in the control word of the x87 unit and in MXCSR,
# and in gaol_core::sse_enter_upward() and sse_leave(), which decide what the
# operations with GAOL_PRESERVE_ROUNDING write in MXCSR alone; they are run by
# tests/rounding_direction.cpp on every platform, on registers simulated by a
# type of its own (x86_registers_simulated(), sse_register_simulated()); the
# test has to fail on it, unless it makes the same decisions (an equivalent
# mutant, which the list says).
#
#     python3 tests/tools/pow/mutate.py --mutants tests/tools/x86-csr/mutants.py \
#             --test rounding_direction --no-diffpow
#
# The constants x86_flush_mask, sse_rc_mask and sse_rc_upward are not mutated:
# a static_assert compares them with those of <xmmintrin.h>, and a mutant
# that fails to compile is no mutant. The processor itself (x86_registers,
# gaol_read_x86_control()) and the functions that give it to x86_*()
# (get_rounding(), set_rounding()...) are here since point PG.10 of TODO.md:
# an x86-64 processor runs them (rounding_state_on_the_processor() of the test).
#---------------------------------------------------------------------------
# gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
# COPYING file for information.
#---------------------------------------------------------------------------
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-27 by Jordan NININ (from the 46 mutants of the work of #156,
# whose file was lost, rewritten for the code of configure-clean; the 17 of
# sse_enter_upward() and sse_leave() added for point 3 of PF, GAOL v5)

F = 'gaol/gaol_fpu.h'
# The constants of the fields and x86_set_modes(), which gaol_set_rounding_x86() of this header needs before it
# (point 3 of PF, GAOL v5)
FENV = 'gaol/gaol_fpu_fenv.h'


def m(name, old, new, survives=None, file=F):
    d = dict(name=name, file=file, edits=[(old, new)])
    if survives:
        d['survives'] = survives
    return d


def mf(name, old, new, survives=None):
    return m(name, old, new, survives, FENV)


# The two conditions of sse_enter_upward() and sse_leave()
SSE_ENTER = '''    if (GAOL_UNLIKELY((saved & modes) != sse_rc_upward)) {
      r.write((saved & ~modes) | sse_rc_upward);'''
SSE_LEAVE = '''    if (GAOL_UNLIKELY((saved & modes) != sse_rc_upward)) {
      const unsigned int csr = r.read();'''
SSE_IF = 'if (GAOL_UNLIKELY((saved & modes) != sse_rc_upward)) {'
SSE_WANTED = 'const unsigned int wanted = (csr & ~modes) | (saved & modes);'


MUTANTS = [
    # The fields
    mf('x87-mask-too-narrow', 'const unsigned int x87_rc_mask = 0x0c00u;', 'const unsigned int x87_rc_mask = 0x0800u;'),
    mf('x87-mask-too-wide', 'const unsigned int x87_rc_mask = 0x0c00u;', 'const unsigned int x87_rc_mask = 0x0e00u;'),
    mf('x87-upward-wrong', 'const unsigned int x87_rc_upward = 0x0800u;', 'const unsigned int x87_rc_upward = 0x0400u;'),
    mf('x87-upward-is-FE_UPWARD', 'const unsigned int x87_rc_upward = 0x0800u;',
      'const unsigned int x87_rc_upward = static_cast<unsigned int>(FE_UPWARD);',
      survives='FE_UPWARD is 0x0800 with glibc: only a host whose <fenv.h> says otherwise (macOS on arm64, Windows) kills it'),

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
    mf('set-modes-x87-always', 'if ((cw & x87_rc_mask) != x87_rc) {', 'if (true) {'),
    mf('set-modes-x87-never', 'if ((cw & x87_rc_mask) != x87_rc) {', 'if (false) {'),
    mf('set-modes-sse-always', 'if ((csr & sse_mask) != sse_bits) {', 'if (true) {'),
    mf('set-modes-sse-never', 'if ((csr & sse_mask) != sse_bits) {', 'if (false) {'),
    mf('set-modes-x87-compares-the-whole-word', 'if ((cw & x87_rc_mask) != x87_rc) {', 'if (cw != x87_rc) {'),
    mf('set-modes-sse-compares-the-whole-register', 'if ((csr & sse_mask) != sse_bits) {', 'if (csr != sse_bits) {'),
    mf('set-modes-x87-overwritten', 'r.write_x87_control((cw & ~x87_rc_mask) | x87_rc);', 'r.write_x87_control(x87_rc);'),
    mf('set-modes-sse-overwritten', 'r.write_mxcsr((csr & ~sse_mask) | sse_bits);', 'r.write_mxcsr(sse_bits);'),
    mf('set-modes-flags-cleared', 'r.write_mxcsr((csr & ~sse_mask) | sse_bits);',
      'r.write_mxcsr((csr & ~sse_mask & ~0x3fu) | sse_bits);'),
    mf('set-modes-x87-write-wide', 'r.write_x87_control((cw & ~x87_rc_mask) | x87_rc);',
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
    m('leave-restores-directions-only', 'x86_set_rounding_and_flush_modes(r, s);\n    }\n  }\n\n\n  /*!\n  \\brief The rounding direction GAOL_RND_ENTER()',
      'x86_set_rounding(r, s);\n    }\n  }\n\n\n  /*!\n  \\brief The rounding direction GAOL_RND_ENTER()'),

    # sse_enter_upward(): the write at the start of an operation, only if the direction is not upward with the
    # modes that flush the subnormals clear
    m('sse-enter-always', SSE_ENTER, SSE_ENTER.replace(SSE_IF, 'if (true) {')),
    m('sse-enter-never', SSE_ENTER, SSE_ENTER.replace(SSE_IF, 'if (false) {')),
    m('sse-enter-inverted', SSE_ENTER, SSE_ENTER.replace('!= sse_rc_upward', '== sse_rc_upward')),
    m('sse-enter-ignores-flush', SSE_ENTER, SSE_ENTER.replace('(saved & modes) != sse_rc_upward', '(saved & sse_rc_mask) != sse_rc_upward')),
    m('sse-enter-keeps-flush', SSE_ENTER,
      SSE_ENTER.replace('r.write((saved & ~modes) | sse_rc_upward);', 'r.write((saved & ~sse_rc_mask) | sse_rc_upward);')),
    m('sse-enter-writes-nearest', SSE_ENTER, SSE_ENTER.replace('| sse_rc_upward);', '| 0u);')),
    m('sse-enter-overwrites', SSE_ENTER, SSE_ENTER.replace('r.write((saved & ~modes) | sse_rc_upward);', 'r.write(sse_rc_upward);')),

    # sse_leave(): nothing if the operation found upward with the modes clear, else the read that keeps the exception
    # flags, and a write only if MXCSR differs from what the operation found. 'sse-leave-always' writes the same
    # bytes, as nothing was changed: the test counts the reads, which the mutant makes always
    m('sse-leave-always', SSE_LEAVE, SSE_LEAVE.replace(SSE_IF, 'if (true) {')),
    m('sse-leave-never', SSE_LEAVE, SSE_LEAVE.replace(SSE_IF, 'if (false) {')),
    m('sse-leave-inverted', SSE_LEAVE, SSE_LEAVE.replace('!= sse_rc_upward', '== sse_rc_upward')),
    m('sse-leave-ignores-flush', SSE_LEAVE, SSE_LEAVE.replace('(saved & modes) != sse_rc_upward', '(saved & sse_rc_mask) != sse_rc_upward')),
    m('sse-leave-no-compare', 'if (csr != wanted) {', 'if (true) {'),
    m('sse-leave-never-writes', 'if (csr != wanted) {', 'if (false) {'),
    m('sse-leave-flags-lost', SSE_WANTED, 'const unsigned int wanted = (csr & ~modes & ~0x3fu) | (saved & modes);'),
    m('sse-leave-writes-saved', SSE_WANTED, 'const unsigned int wanted = saved;'),
    m('sse-leave-keeps-the-modes', SSE_WANTED, 'const unsigned int wanted = csr;'),
    m('sse-leave-direction-only', SSE_WANTED, 'const unsigned int wanted = (csr & ~sse_rc_mask) | (saved & sse_rc_mask);'),

    # get_rounding(), set_rounding() and set_rounding_and_flush_modes() on the processor, and the registers they give
    # to the functions above (x86-64 and 32-bit x86, point PG.10): rounding_state_on_the_processor() of the test, which
    # reads and writes the registers apart from the library, kills them on x86-64
    m('get-rounding-reads-nothing', 'x86_get_state(hw, s);', 's.x87 = 0u; s.sse = 0u; s.flush = 0u;'),
    m('set-rounding-also-sets-the-modes', 'x86_set_rounding(hw, s);', 'x86_set_rounding_and_flush_modes(hw, s);'),
    m('set-rounding-and-flush-modes-sets-no-mode', 'x86_set_rounding_and_flush_modes(hw, s);', 'x86_set_rounding(hw, s);'),
    mf('registers-read-swapped',
      'void read_control(unsigned int& x87_cw, unsigned int& mxcsr) const { gaol_read_x86_control(x87_cw, mxcsr); }',
      'void read_control(unsigned int& x87_cw, unsigned int& mxcsr) const { gaol_read_x86_control(mxcsr, x87_cw); }'),
    mf('registers-read-x87-bit-lost', '''    x87_cw = cw;
    mxcsr = csr;
  }

  //! Writes the whole control word of the x87 unit
  GAOL_INLINE void gaol_write_x87_control(unsigned int x87_cw)
  {
    const unsigned short cw''', '''    x87_cw = cw & ~0x0400u;
    mxcsr = csr;
  }

  //! Writes the whole control word of the x87 unit
  GAOL_INLINE void gaol_write_x87_control(unsigned int x87_cw)
  {
    const unsigned short cw'''),
    mf('registers-write-x87-direction-lost',
      'void write_x87_control(unsigned int x87_cw) const { gaol_write_x87_control(x87_cw); }',
      'void write_x87_control(unsigned int x87_cw) const { gaol_write_x87_control(x87_cw & ~0x0c00u); }'),
    mf('registers-write-mxcsr-flags-lost', '''    void write_mxcsr(unsigned int mxcsr) const { _mm_setcsr(mxcsr); }
  };

  /*
    The fields RC''', '''    void write_mxcsr(unsigned int mxcsr) const { _mm_setcsr(mxcsr & ~0x3fu); }
  };

  /*
    The fields RC'''),
]
