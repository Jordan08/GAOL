# Meson build of GAOL
#
# Created 2025-11-25 by Raphaël Chenouard
# Modified in 2026 by Jordan NININ (GAOL v5)
#
project(
  'GAOL',
  'c',
  'cpp',
  version: '5.0.0',
  # Optimized by default, as configure builds GAOL (meson's own default, debug,
  # does not optimize), with NDEBUG as configure and the Release build of CMake
  # define it: meson setup --buildtype=debug builds it for debugging, as the
  # Debug build of CMake and configure --enable-debug (GAOL_DEBUGGING)
  default_options: ['cpp_std=c++11', 'c_std=c99', 'buildtype=release', 'b_ndebug=if-release'],
)
major_version = 5
minor_version = 0
micro_version = 0
# Where to report bugs, and the page of GAOL v5, which configure --help gives
# too (AC_INIT of configure.ac) (GAOL v5)
gaol_bug_report = 'jordan.ninin@ensta.fr'
gaol_url = 'https://github.com/Jordan08/GAOL'

## Main compiler and linker configs

cpp_comp = meson.get_compiler('cpp')
c_comp = meson.get_compiler('c')

gaol_compiler = cpp_comp.get_id() + ' ' + cpp_comp.version()
# mingw-w64 is no longer refused for its version: GAOL takes no function from
# the math library of the system, and no longer changes the rounding direction
# for its elementary functions (see CMakeLists.txt)

msvc = cpp_comp.get_argument_syntax() == 'msvc'

# Options
simd = get_option('enable-simd')
asm = get_option('enable-asm')
preserve_rounding = get_option('enable-preserve-rounding')
exception = get_option('enable-exception')
# The Debug build, as CMAKE_BUILD_TYPE=Debug and configure --enable-debug:
# GAOL checks its assertions (GAOL_DEBUGGING). The option enable-debug, which
# only defined GAOL_DEBUGGING, is gone, as enable-optimize: the build type
# gives both (GAOL v5)
debug = get_option('buildtype') == 'debug'
# The mathematical library
#
# GAOL bounds every one of its elementary functions with CORE-MATH
# (https://core-math.gitlabpages.inria.fr, MIT licence), whose sources are in
# 3rd/math-core and are compiled into libgaol itself (gaol/meson.build): there
# is no library to find, to build apart or to link along with GAOL.
message('Elementary functions: CORE-MATH, from 3rd/math-core (correctly rounded)')

doc = get_option('with-doc')
# The unit tests of tests/ and the examples of examples/, as WITH_TESTS and
# WITH_EXAMPLES of CMake, --with-tests and --with-examples of configure (GAOL
# v5; with-test before)
build_tests = get_option('with-tests')
build_examples = get_option('with-examples')

# The math library of the system, which Visual C++ does not have apart
m_dep = cpp_comp.find_library('m', required: false)

deps = [m_dep]

os_win = host_machine.system() == 'windows'

message('CPU: ' + host_machine.cpu_family() + ' (' + host_machine.cpu() + ')')
message('OS: ' + host_machine.system())

# The processor and the system: gaol/gaol_config.h names them from the macros
# of the compiler, for the three builds alike, and every processor is accepted
# (meson.build refused every cpu() other than aarch64, x86 and x86_64, i686
# and armv7l included)

# gaol/gaol_configuration.h: the macros GAOL's sources read, the same as with
# CMake and configure (see doc/building.md). The checks of headers, functions,
# types and sizes nothing read are gone (GAOL v5).
conf_data = configuration_data()
conf_data.set('GAOL_MAJOR_VERSION', major_version)
conf_data.set('GAOL_MINOR_VERSION', minor_version)
conf_data.set('GAOL_MICRO_VERSION', micro_version)
conf_data.set_quoted('GAOL_VERSION', meson.project_version())

if exception
  conf_data.set('GAOL_EXCEPTIONS_ENABLED', 1)
endif
if debug
  conf_data.set('GAOL_DEBUGGING', 1)
endif
if preserve_rounding
  conf_data.set('GAOL_PRESERVE_ROUNDING', 1)
endif
# Never with Visual C++, as with CMake
if asm and not msvc
  conf_data.set('GAOL_USING_ASM', 1)
endif
# No option for the intervals of floats, gaol::intervalf and gaol::interval2f,
# unfinished, which only a developer of GAOL compiles (see gaol/gaol_config.h)
if get_option('enable-verbose-mode')
  conf_data.set('GAOL_VERBOSE_MODE', 1)
endif

# gaol/gaol_fpu.h and gaol/gaol_exact.h: <fenv.h>; gaol/gaol_profile.cpp:
# getrusage() where <sys/resource.h> declares it, clock() otherwise
if cpp_comp.has_header('fenv.h')
  conf_data.set('HAVE_FENV_H', 1)
endif
if cpp_comp.has_function('getrusage', prefix: '#include <sys/resource.h>')
  conf_data.set('HAVE_GETRUSAGE', 1)
endif

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
# Visual C++: /fp:strict, as CMakeLists.txt gives it to GAOL and to CORE-MATH,
# without which gaol/gaol_config.h refuses to compile
if msvc
  add_global_arguments(['/fp:strict'], language: ['c', 'cpp'])
  pc_cflags += ['/fp:strict']
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
# The optimizations configure (--enable-optimize) and the Release build of
# CMake add to -O3, in the release build type, where the compiler takes them
if get_option('buildtype') == 'release'
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
# (gaol_interval_sse.cpp), -msse3 is given where the compiler takes it, and on
# 32-bit x86 the doubles are computed with SSE2
# (-mfpmath=sse) rather than on the x87 unit, where CORE-MATH rounds some
# results twice and some bounds are wrong (see CMakeLists.txt). Not the SSE2
# intervals on 32-bit Windows, where a std::vector of them crashes (see
# CMakeLists.txt).
use_sse2 = false
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
  if simd and not msvc and not (target_x86_32 and os_win) and cpp_comp.has_argument('-msse2')
    use_sse2 = true
    conf_data.set('USING_SSE2_INSTRUCTIONS', 1)
    add_global_arguments(['-msse2'], language: 'cpp')
    pc_cflags += ['-msse2']
    if cpp_comp.has_argument('-msse3')
      conf_data.set('USING_SSE3_INSTRUCTIONS', 1)
      add_global_arguments(['-msse3'], language: 'cpp')
      pc_cflags += ['-msse3']
    endif
  endif
endif
# enable-fma: the fused multiply-add instructions of the processor, as
# CMakeLists.txt and configure take them (see CMakeLists.txt): /arch:AVX2 with
# Visual C++ but for 32-bit x86, where it breaks the rounding direction; -mfma
# on x86, where it defines __FMA__, but with GCC for Windows (MinGW), which
# does not align the stack for the AVX that -mfma turns on; -mfpu=neon-vfpv4
# -mfloat-abi=hard on 32-bit ARM, where FMA is not on already. Given to the C
# and C++ sources (GAOL, CORE-MATH), and to the code using GAOL
# (gaol.pc), compiled for the same processor. -ffp-contract=off stays. Left out
# where the machine building does not run a program compiled with them, but
# when cross-compiling (see CMakeLists.txt: macOS 14 under Rosetta).
fma_args = []
if get_option('enable-fma')
  if msvc
    msvc_x86_32 = cpp_comp.compiles('''
    #if !defined(_M_IX86)
    #error not 32-bit x86
    #endif
    int main() { return 0; }
    ''', name: 'the target is 32-bit x86 (Visual C++)')
    if not msvc_x86_32 and cpp_comp.has_argument('/arch:AVX2') and c_comp.has_argument('/arch:AVX2')
      fma_args = ['/arch:AVX2']
    endif
  else
    mingw_gcc = cpp_comp.compiles('''
    #if !(defined(_WIN32) && defined(__GNUC__) && !defined(__clang__))
    #error not GCC for Windows
    #endif
    int main() { return 0; }
    ''', name: 'GCC for Windows')
    fma_check = '''
    #if !defined(@0@)
    #error no fused multiply-add instructions
    #endif
    int main() { return 0; }
    '''
    if mingw_gcc
      message('Fused multiply-add instructions left out: GCC for Windows does not align the stack for AVX')
    elif cpp_comp.links(fma_check.format('__FMA__'), args: ['-mfma'], name: '-mfma gives the fused multiply-add instructions (C++)') and c_comp.links(fma_check.format('__FMA__'), args: ['-mfma'], name: '-mfma gives the fused multiply-add instructions (C)')
      fma_args = ['-mfma']
    else
      arm32_without_fma = cpp_comp.compiles('''
      #if !defined(__arm__) || defined(__ARM_FEATURE_FMA)
      #error not a 32-bit ARM target without FMA
      #endif
      int main() { return 0; }
      ''', name: 'a 32-bit ARM target without FMA')
      arm_args = ['-mfpu=neon-vfpv4', '-mfloat-abi=hard']
      if arm32_without_fma and cpp_comp.links(fma_check.format('__ARM_FEATURE_FMA'), args: arm_args, name: '-mfpu=neon-vfpv4 gives the fused multiply-add instructions (C++)') and c_comp.links(fma_check.format('__ARM_FEATURE_FMA'), args: arm_args, name: '-mfpu=neon-vfpv4 gives the fused multiply-add instructions (C)')
        fma_args = arm_args
      endif
    endif
  endif
  if fma_args.length() > 0 and not meson.is_cross_build()
    # fma(a, b, -1) is -2^-60 exactly, which a product rounded first gives as 0
    fma_run = cpp_comp.run('''
    #include <cmath>
    int main() {
      volatile double a = 1.0 + 1.0 / 1073741824.0, b = 1.0 - 1.0 / 1073741824.0, c = -1.0;
      return std::fma(a, b, c) == -1.0 / 1152921504606846976.0 ? 0 : 1;
    }
    ''', args: fma_args, name: 'this machine runs a program compiled with ' + ' '.join(fma_args))
    if not (fma_run.compiled() and fma_run.returncode() == 0)
      message('Fused multiply-add instructions left out: this machine does not run a program compiled with ' + ' '.join(fma_args))
      fma_args = []
    endif
  endif
  if fma_args.length() > 0
    add_global_arguments(fma_args, language: 'c')
    add_global_arguments(fma_args, language: 'cpp')
    pc_cflags += fma_args
  endif
endif
message('Fused multiply-add flags: ' + ' '.join(fma_args))

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

subdir('gaol')

# make test and make check, as with CMake and configure (GAOL v5): the unit
# tests are in the suite unit, the examples in the suite examples.
#   meson test -C <build>            the unit tests (make test; ninja test)
#   meson test -C <build> --setup check, or ninja -C <build> check
#                                    the tests and the examples (make check)
check_targets = []
if build_tests
  subdir('tests')
endif
if build_examples
  subdir('examples')
endif
if meson.version().version_compare('>=0.57')
  add_test_setup('unit', exclude_suites: ['examples'], is_default: true)
  add_test_setup('check')
else
  message('meson ' + meson.version() + ': meson test and ninja test run the examples too (meson test --suite unit runs the unit tests only)')
endif
if build_tests or build_examples
  run_target('check',
    command: [find_program('meson'), 'test', '-C', meson.current_build_dir(), '--no-rebuild', '--print-errorlogs']
             + (meson.version().version_compare('>=0.57') ? ['--setup', 'check'] : []),
    depends: check_targets)
endif

# make perf, as with CMake and configure: GAOL v5 measured on the benchmark of
# doc/compare/performance.md, whose tables get its new times (see
# doc/compare/code/run_perf.sh). Linux only, as the benchmark (GAOL v5).
bash = find_program('bash', required: false)
if host_machine.system() == 'linux' and bash.found()
  bench_dir = meson.current_source_dir() + '/doc/compare/code'
  gaol_bench = executable('gaol_bench', bench_dir + '/bench_gaol.cpp',
    include_directories: include_directories('.', 'doc/compare/code'),
    cpp_args: ['-DGAOL_BENCH_NAME="gaol5"'],
    link_with: gaollib_static,
    dependencies: deps,
    build_by_default: false)
  run_target('perf',
    command: [bash, bench_dir + '/run_perf.sh', gaol_bench.full_path(),
              'meson ' + get_option('buildtype'), ' '.join(cpp_comp.cmd_array())],
    depends: gaol_bench)
endif

# make parser, as with CMake and configure: the lexer and the parser
# regenerated in the sources with the flex and the bison of the system (see
# gaol/regenerate_parser.sh) (GAOL v5)
run_target('parser', command: [find_program('sh'), files('gaol/regenerate_parser.sh')])

if doc
  subdir('manual')
endif

## The pkg-config file (pkg-config --cflags --libs gaol): the flags of interval
## arithmetic, which the code including GAOL's headers has to be compiled with.
## In a meson project: gaol_dep = dependency('gaol')
pkg = import('pkgconfig')
pkg.generate(
  libraries: [gaollib] + (m_dep.found() ? ['-lm'] : []),
  name: 'gaol',
  description: 'GAOL, Not Just Another Interval Library',
  url: gaol_url,
  extra_cflags: pc_cflags,
)

summary({
  'Version': meson.project_version(),
  'Bug reports': gaol_bug_report,
  'Home page': gaol_url,
  'Debug (GAOL_DEBUGGING)': debug,
  'Preserve rounding': preserve_rounding,
  'SSE2 intervals': use_sse2,
  'Fused multiply-add': ' '.join(fma_args),
  'Unit tests': build_tests,
  'Examples': build_examples,
})
