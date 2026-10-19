"""Writes a C++ file which, preprocessed with the flags of a build, shows the configuration GAOL sees."""

# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
# The macros of the configuration of GAOL: those of gaol/gaol_configuration.h,
# the same with the three builds (see doc/building.md), and those
# gaol/gaol_config.h and gaol/gaol_fpu.h derive from the compiler (GAOL v5)
config = ['GAOL_AARCH64_LINUX', 'GAOL_ARM_MACOSX', 'GAOL_IX86_LINUX', 'GAOL_IX86_MACOSX', 'GAOL_DEBUGGING', 'GAOL_EXCEPTIONS_ENABLED', 'GAOL_FLOAT_INTERVALS', 'GAOL_PRESERVE_ROUNDING', 'GAOL_VERBOSE_MODE',
          'GAOL_HAVE_FENV_H',
          'GAOL_HAVE_VISIBILITY_OPTIONS', 'GAOL_SIZEOF_INT', 'GAOL_SIZEOF_LONG_LONG_INT',
          'GAOL_USING_SSE2_INSTRUCTIONS', 'GAOL_USING_SSE3_INSTRUCTIONS', 'GAOL_USING_NEON_INSTRUCTIONS', 'GAOL_WORDS_BIGENDIAN', 'GAOL_RND_PROBE', 'GAOL_RND_SSE_REGISTER',
          'GAOL_HAVE_AVX512_TARGET', 'GAOL_PREFER_AVX512', 'GAOL_AVX512_ARITHMETIC']
# Compared by value only (GAOL_VERSION is a string, which #if cannot read)
text = ['GAOL_INLINE', 'GAOL_PUBLIC', 'GAOL_MAJOR_VERSION', 'GAOL_MINOR_VERSION', 'GAOL_MICRO_VERSION', 'GAOL_VERSION']
compiler = ['__cplusplus', '__STRICT_ANSI__', '__OPTIMIZE__', 'NDEBUG', '_GLIBCXX_ASSERTIONS', '__FAST_MATH__', '__FINITE_MATH_ONLY__', 'FLT_EVAL_METHOD',
            '__SSE2__', '__SSE3__', '__SSE2_MATH__', '__PIC__', '__x86_64__', '__i386__', '__aarch64__', '__arm__', '__APPLE__',
            '__linux__', '_WIN32', '__MINGW64_VERSION_MAJOR']
lines = ['#include <cfloat>', '#include "gaol/gaol.h"']
for n in config + text + compiler:
    lines += ['#ifdef %s' % n, 'GAOLPROBE_%s = %s' % (n, n), '#else', 'GAOLPROBE_%s undefined' % n, '#endif']
for n in config + [c for c in compiler if c not in ('__cplusplus',)]:
    lines += ['#if defined(%s) && (%s + 0)' % (n, n), 'GAOLPROBEIF_%s true' % n, '#else', 'GAOLPROBEIF_%s false' % n, '#endif']
print('\n'.join(lines))
