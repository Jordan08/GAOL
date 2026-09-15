project(
  'GAOL',
  'c',
  'cpp',
  version: '4.2.3',
  # Optimized by default, as configure builds GAOL (meson's own default, debug,
  # does not optimize), with NDEBUG as configure and the Release build of CMake
  # define it: meson setup --buildtype=debug builds it for debugging
  default_options: ['cpp_std=c++11', 'c_std=c99', 'buildtype=release', 'b_ndebug=if-release'],
)
major_version = 4
minor_version = 2
micro_version = 3

## Main compiler and linker configs

cpp_comp = meson.get_compiler('cpp')
c_comp = meson.get_compiler('c')

# The mingw-w64 CMakeLists.txt refuses: before version 12, whose math library is
# not accurate enough for the hyperbolic functions GAOL takes from it, and
# version 12, whose fesetround() runs the instruction cpuid at each call (see
# CMakeLists.txt)
gaol_compiler = cpp_comp.get_id() + ' ' + cpp_comp.version()
if not cpp_comp.compiles('''
#include <stdlib.h>
#if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 12
#error mingw-w64 before version 12
#endif
int main() { return 0; }
''', name: 'mingw-w64, if used, is of version 12 or later')
  error('GAOL cannot be built with '
    + gaol_compiler
    + ', whose mingw-w64 is older than version 12.\nThe math library of mingw-w64 before version 12 is not accurate enough for the hyperbolic functions GAOL takes from it: acosh() near 1 was up to 25 million floats away from the exact value, and asinh() of large negative numbers NaN, so that the intervals GAOL computes did not enclose the values they should (with the MinGW-w64 GCC 11.2.0, 12.2.0 and 13.2.0 of Chocolatey).\nBuild GAOL with a MinGW-w64 of version 13 or later (version 12 is refused too, its fesetround() being slow), such as the MinGW-w64 GCC 15.2.0 (rt_v13) of MinGW-Builds, which Chocolatey installs (choco install mingw --version=15.2.0), or the one of MSYS2, or with CMake and Visual Studio.')
endif
if not cpp_comp.compiles('''
#include <stdlib.h>
#if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 13
#error mingw-w64 before version 13
#endif
int main() { return 0; }
''', name: 'mingw-w64, if used, is of version 13 or later')
  error('GAOL cannot be built with '
    + gaol_compiler
    + ', whose mingw-w64 is version 12.\nfesetround() of mingw-w64 12 runs the instruction cpuid at each call, which takes microseconds in a virtual machine: 3.5 microseconds per call in GitHub Actions, against 130 nanoseconds with mingw-w64 13. GAOL changes the rounding direction four times for each exp(), log(), sin() or cos() of an interval, which then took 10.6 to 13.8 microseconds rather than 0.5 to 0.7 (with the MinGW-w64 GCC 14.2.0 of Chocolatey, rt_v12): the intervals GAOL computes are right, but some 20 times slower.\nBuild GAOL with a MinGW-w64 of version 13 or later, such as the MinGW-w64 GCC 15.2.0 (rt_v13) of MinGW-Builds, which Chocolatey installs (choco install mingw --version=15.2.0), or the one of MSYS2, or with CMake and Visual Studio.')
endif

# Options
debug = get_option('enable-debug')
simd = get_option('enable-simd')
asm = get_option('enable-asm')
float_intervals = get_option('enable-float-intervals')
preserve_rounding = get_option('enable-preserve-rounding')
relations = get_option('enable-relations')
exception = get_option('enable-exception')
mathlib = get_option('with-mathlib')
# The math library of the system, whose results GAOL widens slightly
# (gaol/gaol_double_op_m.h) without certifying its bounds: a choice left to
# the user, who is warned
if mathlib == 'default'
  warning('with-mathlib=default: GAOL computes its elementary functions with the math library of the system, and does not certify their bounds: '
    + 'its tests find bounds that do not enclose the exact values with it (near the overflow of exp, sinh and cosh with glibc 2.31). '
    + 'Use with-mathlib=apmathlib (the default) or crlibm for certified bounds.')
endif
doc = get_option('with-doc')
build_tests = get_option('with-test')

## dependencies
#apmathlib_dep = cpp_comp.find_library('ultim', required : false)
#crmlibm_dep = cpp_comp.find_library('crlibm', required : false)
#mathlib_dep=apmathlib_dep
#if not apmathlib_dep.found() and crlibm_dep.found()
#  mathlib_dep = crlibm_dep
#elif not apmathlib_dep.found() and not crlibm_dep.found()
#  error('Missing Mathematical library! Expecting APMathlib or Crlibm')
#endif
# Where the mathematical library is, when it is not in the usual paths (or on
# CPATH and LIBRARY_PATH): with-mathlib-include and with-mathlib-lib, as
# --with-mathlib-include and --with-mathlib-lib of configure. Its directory
# then goes into gaol.pc too.
mathlib_include = get_option('with-mathlib-include')
mathlib_libdir = get_option('with-mathlib-lib')
mathlib_dirs = []
mathlib_pc_libs = []
if mathlib_include != ''
  add_global_arguments(['-I' + mathlib_include], language: 'cpp')
  add_global_arguments(['-I' + mathlib_include], language: 'c')
endif
if mathlib_libdir != ''
  mathlib_dirs = [mathlib_libdir]
  mathlib_pc_libs = ['-L' + mathlib_libdir]
endif
mathlib_dep = dependency('', required: false)
if mathlib == 'apmathlib'
  mathlib_dep = cpp_comp.find_library('ultim', required: true, dirs: mathlib_dirs)
  mathlib_pc_libs += ['-lultim']
elif mathlib == 'crlibm'
  mathlib_dep = cpp_comp.find_library('crlibm', required: true, dirs: mathlib_dirs)
  mathlib_pc_libs += ['-lcrlibm']
  # else
  #   mathlib_dep = cpp_comp.find_library('m', required: true)
endif
m_dep = cpp_comp.find_library('m', required: true)

deps = [mathlib_dep, m_dep]

## Config
os_unix = false
os_linux = false
os_win = false
os_darwin = false
os_cygwin = false
os_sun = false

if host_machine.system() == 'darwin'
  os_darwin = true
elif host_machine.system() == 'windows'
  os_win = true
elif host_machine.system() == 'linux'
  os_linux = true
elif host_machine.system() == 'cygwin'
  os_cygwin = true
elif host_machine.system() == 'sunos'
  os_sun = true
else
  warning('Unknown host system!')
endif
os_unix = not os_win

message('CPU: ' + host_machine.cpu_family() + ' (' + host_machine.cpu() + ')')
message('OS: ' + host_machine.system())

# The processor and the system: gaol/gaol_config.h names them from the macros
# of the compiler, for the three builds alike, and every processor is accepted
# (meson.build refused every cpu() other than aarch64, x86 and x86_64, i686
# and armv7l included)

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
conf_data.set('project_name', meson.project_name())
conf_data.set('project_full_name', meson.project_name())
conf_data.set('bug_report', 'frederic.goualard@univ-nantes.fr')
conf_data.set('project_url', 'https://github.com/goualard-f/GAOL')

conf_data.set('GAOL_MAJOR_VERSION', major_version)
conf_data.set('GAOL_MINOR_VERSION', minor_version)
conf_data.set('GAOL_MICRO_VERSION', micro_version)
conf_data.set('version', meson.project_version())

if mathlib == 'apmathlib'
  conf_data.set('GAOL_USING_APMATHLIB', 1)
elif mathlib == 'crlibm'
  conf_data.set('GAOL_USING_CRLIBM', 1)
endif
if relations == 'certainly'
  conf_data.set('GAOL_CERTAINLY_RELATIONS', 1)
elif relations == 'possibly'
  conf_data.set('GAOL_POSSIBLY_RELATIONS', 1)
elif relations == 'set'
  conf_data.set('GAOL_SET_RELATIONS', 1)
else
  error(
    'Wrong value for relations, expecting set, possibly or certainly, but get: '
    + relations,
  )
endif
if exception
  conf_data.set('GAOL_EXCEPTIONS_ENABLED', 1)
endif
if debug
  conf_data.set('GAOL_DEBUGGING', 1)
endif
if preserve_rounding
  conf_data.set('GAOL_PRESERVE_ROUNDING', 1)
endif
if asm
  conf_data.set('GAOL_USING_ASM', 1)
endif
if float_intervals
  conf_data.set('GAOL_FLOAT_INTERVALS', 1)
endif
if get_option('enable-verbose-mode')
  conf_data.set('GAOL_VERBOSE_MODE', 1)
endif

if host_machine.endian() == 'big'
  conf_data.set('WORDS_BIGENDIAN', 1)
endif

conf_data.set('STDC_HEADERS', 1)
#conf_data.set('const',0)

check_headers = [
  'dlfcn.h',
  'inttypes.h',
  'stdint.h',
  'stdio.h',
  'stdlib.h',
  'strings.h',
  'string.h',
  'sys/stat.h',
  'sys/types.h',
  'unistd.h',
  'cassert',
  'fenv.h',
  'float.h',
  'limits',
  'stddef.h',
  'sys/resource.h',
  'sys/time.h',
  'time.h',
  'stdbool.h',
]

foreach h : check_headers
  if cpp_comp.has_header(h)
    conf_data.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

if mathlib == 'apmathlib'
  if cpp_comp.has_header('MathLib.h')
    conf_data.set('HAVE_' + 'MathLib.h'.underscorify().to_upper(), 1)
  endif
elif mathlib == 'crlibm'
  if cpp_comp.has_header('crlibm.h')
    conf_data.set('HAVE_' + 'crlibm.h'.underscorify().to_upper(), 1)
  endif
endif

check_functions = [
  'clock',
  'fesetround',
  'finite',
  'floor',
  'getrusage',
  'localeconv',
  'malloc',
  'memset',
  'nextafter',
  'pow',
  'realloc',
  'rint',
  'round',
  'sqrt',
  '__signbit',
]

foreach f : check_functions
  if cpp_comp.has_function(f)
    conf_data.set('HAVE_' + f.underscorify().to_upper(), 1)
  endif
endforeach

# As configure.ac does: gaol_profile.cpp declares getrusage() itself when
# sys/resource.h does not, which conflicts with the declaration of glibc
conf_data.set10('GETRUSAGE_IN_HEADER', cpp_comp.has_header_symbol('sys/resource.h', 'getrusage'))

conf_data.set('HAVE__BOOL', cpp_comp.has_type('_Bool'))
conf_data.set('SIZEOF_INT', cpp_comp.sizeof('int'))
conf_data.set('SIZEOF_LONG', cpp_comp.sizeof('long'))
conf_data.set('SIZEOF_LONG_LONG_INT', cpp_comp.sizeof('long long int'))

# The compilers CMakeLists.txt and configure refuse, for the same reasons (see
# CMakeLists.txt): Clang for 32-bit ARM processors, which does not honour the
# rounding direction, and a compiler that takes -frounding-math and says it
# does not honour the rounding direction on the target ("overriding currently
# unsupported rounding mode on this target"), as Clang 14 for 64-bit ARM
if not cpp_comp.compiles('''
#if defined(__arm__) && !defined(__aarch64__) && defined(__clang__)
#error Clang for a 32-bit ARM processor
#endif
int main() { return 0; }
''', name: 'the C++ compiler is not Clang for a 32-bit ARM processor')
  error('GAOL cannot be built with ' + gaol_compiler + ', Clang for a 32-bit ARM processor.\n'
    + 'Clang does not honour the rounding direction on 32-bit ARM processors: it does not support -frounding-math there (overriding currently unsupported rounding mode on this target) and optimizes floating-point operations as if they were always rounded to nearest. GAOL computes with the rounding direction set upward, so that the intervals it computes would not enclose the values they should (built by Clang 21, 4556 of 16000 random products, squares and cubes did not).\n'
    + 'Build GAOL with GCC, which honours the rounding direction on 32-bit ARM processors: CXX=g++ CC=gcc meson setup ...')
endif
if cpp_comp.has_argument('-frounding-math')
  conf_data.set('HAVE_ROUNDING_MATH_OPTION', 1)
  if not cpp_comp.compiles('int main() { return 0; }', args: ['-frounding-math', '-Werror'],
                           name: 'the C++ compiler honours the rounding direction on this target')
    error('GAOL cannot be built with ' + gaol_compiler + ' for this target.\n'
      + 'This compiler does not honour the rounding direction on this target, and says so of -frounding-math (overriding currently unsupported rounding mode on this target). GAOL computes with the rounding direction set upward, so that the intervals it computes would not enclose the values they should (built by Clang 14 for 64-bit ARM processors, bounds of pow() and nth_root() did not).\n'
      + 'Build GAOL with a compiler that honours the rounding direction on this target, such as GCC or a later version of Clang (Clang 18 does on 64-bit ARM processors).')
  endif
endif

# The flags of interval arithmetic, each where the compiler accepts it, as
# CMakeLists.txt and configure give them: -frounding-math; -ffp-contract=off
# (without it, compilers contract a multiplication and an addition into a fused
# one as if rounding to nearest: on 64-bit ARM, -(.5*l - .5*r) and -.5*l + .5*r
# both became fnma(.5, l, .5*r), and mid() a single double); -fno-fast-math
pc_cflags = []  # the flags of interval arithmetic, which gaol.pc gives the code using GAOL
foreach flag : ['-frounding-math', '-ffp-contract=off', '-fno-fast-math']
  if cpp_comp.has_argument(flag)
    add_global_arguments([flag], language: 'cpp')
    pc_cflags += [flag]
  endif
  if c_comp.has_argument(flag)
    add_global_arguments([flag], language: 'c')
  endif
endforeach
# Visual C++: /fp:strict, as CMakeLists.txt gives it, without which
# gaol/gaol_config.h refuses to compile
if cpp_comp.get_argument_syntax() == 'msvc'
  add_global_arguments(['/fp:strict'], language: 'cpp')
endif

# The library compiled with hidden visibility, as configure builds it: GAOL's
# public declarations carry __attribute__ ((visibility("default")))
if cpp_comp.has_multi_arguments('-fvisibility-inlines-hidden', '-fvisibility=hidden')
  conf_data.set('HAVE_VISIBILITY_OPTIONS', 1)
  add_global_arguments(['-fvisibility-inlines-hidden', '-fvisibility=hidden'], language: 'cpp')
endif
if cpp_comp.has_argument('-Wconversion')
  add_global_arguments(['-Wconversion'], language: 'cpp')
endif
if c_comp.has_argument('-Wconversion')
  add_global_arguments(['-Wconversion'], language: 'c')
endif
# enable-optimize: the optimizations configure adds for g++, where the compiler
# takes them, on top of the build type
if get_option('enable-optimize')
  foreach flag : ['-funroll-loops', '-fomit-frame-pointer', '-fexpensive-optimizations']
    if cpp_comp.has_argument(flag)
      add_global_arguments([flag], language: 'cpp')
    endif
    if c_comp.has_argument(flag)
      add_global_arguments([flag], language: 'c')
    endif
  endforeach
endif

# On x86 processors (the target of the compiler, as CMakeLists.txt and
# configure): the intervals are computed with SSE2 instructions
# (gaol_interval_sse.cpp), SSE3 instructions serve gaol::interval2f
# (gaol_interval2f.cpp), and on 32-bit x86 the doubles are computed with SSE2
# (-mfpmath=sse) rather than on the x87 unit, where GAOL's bounds and mathlib's
# results are wrong (see gaol/gaol_config.h). Not the SSE2 intervals on 32-bit
# Windows, where a std::vector of them crashes (see CMakeLists.txt).
use_sse2 = false
use_sse3 = false
x86_args = []
target_x86 = cpp_comp.compiles('''
#if !(defined(__i386__) || defined(__x86_64__))
#error not an x86 processor
#endif
int main() { return 0; }
''', name: 'the target is an x86 processor')
target_x86_32 = cpp_comp.compiles('''
#if !defined(__i386__)
#error not a 32-bit x86 processor
#endif
int main() { return 0; }
''', name: 'the target is a 32-bit x86 processor')
if target_x86
  if target_x86_32 and cpp_comp.has_multi_arguments('-msse2', '-mfpmath=sse')
    x86_args = ['-msse2', '-mfpmath=sse']
    add_global_arguments(x86_args, language: 'cpp')
    add_global_arguments(x86_args, language: 'c')
    pc_cflags += x86_args
  endif
  if simd and not (target_x86_32 and os_win) and cpp_comp.has_argument('-msse2')
    use_sse2 = true
    conf_data.set('USING_SSE2_INSTRUCTIONS', 1)
    add_global_arguments(['-msse2'], language: 'cpp')
    pc_cflags += ['-msse2']
    if cpp_comp.has_argument('-msse3')
      use_sse3 = true
      conf_data.set('USING_SSE3_INSTRUCTIONS', 1)
      add_global_arguments(['-msse3'], language: 'cpp')
      pc_cflags += ['-msse3']
    endif
  endif
endif

# -ffloat-store only where doubles are still computed on the x87 unit
# (FLT_EVAL_METHOD not 0), whose 80-bit registers keep more digits than a
# double (as CMakeLists.txt): checked with the flags of 32-bit x86 above, which
# the checks of the compiler do not get from add_global_arguments()
float_store_needed = cpp_comp.compiles('''
  #include <cfloat>
  #if defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD == 0
  #error doubles are computed in double precision
  #endif
  int main() { return 0; }
''', args: x86_args, name: 'doubles computed in extended precision')
if float_store_needed and cpp_comp.has_argument('-ffloat-store')
  add_global_arguments(['-ffloat-store'], language: 'c')
  add_global_arguments(['-ffloat-store'], language: 'cpp')
  pc_cflags += ['-ffloat-store']
endif

flex = find_program('flex', required: true)
bison = find_program('bison', required: true)

subdir('gaol')

subdir('check')
subdir('examples')
if doc
  subdir('doc')
endif

## The pkg-config file (pkg-config --cflags --libs gaol): the flags of interval
## arithmetic, which the code including GAOL's headers has to be compiled with,
## and mathlib. In a meson project: gaol_dep = dependency('gaol')
pkg = import('pkgconfig')
pkg.generate(
  libraries: [gaollib] + mathlib_pc_libs + ['-lm'],
  name: 'gaol',
  description: 'GAOL, Not Just Another Interval Library',
  url: 'https://github.com/Jordan08/GAOL',
  extra_cflags: pc_cflags,
)
