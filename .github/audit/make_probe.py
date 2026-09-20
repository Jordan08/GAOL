"""Writes a C++ file which, preprocessed with the flags of a build, shows the configuration GAOL sees."""
config = ['AARCH64_LINUX', 'ARM_MACOSX', 'IX86_LINUX', 'IX86_MACOSX', 'GAOL_CERTAINLY_RELATIONS', 'GAOL_POSSIBLY_RELATIONS',
          'GAOL_SET_RELATIONS', 'GAOL_DEBUGGING', 'GAOL_EXCEPTIONS_ENABLED', 'GAOL_FLOAT_INTERVALS', 'GAOL_PRESERVE_ROUNDING', 'GAOL_USING_ASM', 'GAOL_VERBOSE_MODE', 'GETRUSAGE_IN_HEADER', 'HAVE_CASSERT', 'HAVE_CLOCK',
          'HAVE_FENV_H', 'HAVE_FINITE', 'HAVE_GETRUSAGE', 'HAVE_ISNAN', 'HAVE_LIMITS', 'HAVE_NEXTAFTER',
          'HAVE_ROUNDING_MATH_OPTION', 'HAVE_VISIBILITY_OPTIONS', 'SIZEOF_INT', 'SIZEOF_LONG_INT', 'SIZEOF_LONG_LONG_INT',
          'USING_SSE2_INSTRUCTIONS', 'USING_SSE3_INSTRUCTIONS', 'WORDS_BIGENDIAN', 'GAOL_RND_PROBE', 'GAOL_RND_SSE_REGISTER']
text = ['_MATHLIB_DLL_', 'INLINE', '__GAOL_PUBLIC__']
compiler = ['__cplusplus', '__STRICT_ANSI__', '__OPTIMIZE__', 'NDEBUG', '_GLIBCXX_ASSERTIONS', '__FAST_MATH__', 'FLT_EVAL_METHOD',
            '__SSE2__', '__SSE3__', '__SSE2_MATH__', '__PIC__', '__x86_64__', '__i386__', '__aarch64__', '__arm__', '__APPLE__',
            '__linux__', '_WIN32', '__MINGW64_VERSION_MAJOR']
lines = ['#include <cfloat>', '#include "gaol/gaol.h"']
for n in config + text + compiler:
    lines += ['#ifdef %s' % n, 'GAOLPROBE_%s = %s' % (n, n), '#else', 'GAOLPROBE_%s undefined' % n, '#endif']
for n in config + [c for c in compiler if c not in ('__cplusplus',)]:
    lines += ['#if defined(%s) && (%s + 0)' % (n, n), 'GAOLPROBEIF_%s true' % n, '#else', 'GAOLPROBEIF_%s false' % n, '#endif']
print('\n'.join(lines))
