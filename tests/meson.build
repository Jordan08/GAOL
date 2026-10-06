# Meson build of GAOL: the unit tests of tests/, as tests/CMakeLists.txt builds
# them (see there)
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-27 by Jordan NININ
#
# Built where with-tests is true; each is a test of the suite unit, which meson
# test (make test) and meson test --setup check (make check) run. They are
# compiled as any code using GAOL, in C++17 for the hexadecimal floating
# literals of the tests (those of gaol_tests.h, which all but u128.cpp include,
# of the *_values.h, and of several of the programs; see CMakeLists.txt), and
# linked with the static libgaol, as CMake links them. Exit status 77 is a test
# skipped (intervalf and interval2f where the intervals of floats are not
# compiled). check/, which built the unit tests of GAOL 4 with CppUnit, is
# gone: they are among these (GAOL v5).
gaol_tests = [
  'arithmetic', 'elementary', 'rounding_direction', 'numbers', 'other_functions',
  'ieee1788', 'reverse', 'u128', 'core_math', 'expressions', 'extended_precision',
  'static_initialization', 'debugging', 'fast_math_link',
  # The unit tests of GAOL 4 (unit_tests.h)
  'arithmetic_operators', 'assignment', 'constants', 'constructor', 'float_functions',
  'generic_functions', 'input_output', 'interval_functions', 'misc', 'non_arithmetic',
  'relations', 'reverse_mappings', 'trigonometric', 'intervalf', 'interval2f',
  # Tests for point P (25a-25g): mulRevToPair, inflate, midrad, bisect, hull/intersect, width_enclosure, literals
  'mulRevToPair', 'inflate', 'midrad', 'bisect', 'hull_intersect', 'width_enclosure', 'literals',
]
# The rounding direction after the automatic cleanup of GAOL, read by a
# function registered in a constructor of priority 101, which ELF runs before
# GAOL initializes itself
if host_machine.system() == 'linux'
  gaol_tests += ['automatic_cleanup']
endif
tests_args = []
if msvc
  # getenv() and sscanf(), which the tests use as the C standard has them
  tests_args += ['-D_CRT_SECURE_NO_WARNINGS']
endif
# Linked with the link option of gaol_dep and gaol.pc (-mno-daz-ftz, where the
# compiler takes it), as CMake links them (GAOL v5)
tests_dep = declare_dependency(
  link_with: gaollib_static,
  include_directories: include_directories('..'),
  compile_args: tests_args,
  link_args: gaol_link_args,
  dependencies: deps,
)
foreach t : gaol_tests
  # fast_math_link: -ffast-math to its link only, its source being compiled as
  # any code using GAOL, and GAOL_TESTS_NO_DAZ_FTZ where its link has
  # -mno-daz-ftz (GAOL v5)
  test_cpp_args = []
  test_link_args = []
  if t == 'fast_math_link'
    if not msvc
      test_link_args += ['-ffast-math']
    endif
    if gaol_link_args.length() > 0
      test_cpp_args += ['-DGAOL_TESTS_NO_DAZ_FTZ=1']
    endif
  endif
  exe = executable('gaol_test_' + t, t + '.cpp',
    dependencies: tests_dep,
    cpp_args: test_cpp_args,
    link_args: test_link_args,
    override_options: ['cpp_std=c++17'])
  test(t, exe, suite: 'unit', timeout: 300)
  check_targets += [exe]
endforeach

# The options gaol/gaol_config.h refuses, which the compile tests refused_* of
# tests/CMakeLists.txt check in the CMake build (GAOL v5): refused_options.sh
# compiles refused_options.cpp with the compiler, the cpp_args and the language
# standard of the build, and the flags gaol.pc gives the code using GAOL. Not
# with Visual C++, which has neither option nor macro, nor clang-cl, which
# ignores the options (gaol_config.h refuses the /fp:fast of both).
if not msvc
  refused_args = get_option('cpp_args')
  if get_option('cpp_std') != 'none'
    refused_args += ['-std=' + get_option('cpp_std')]
  endif
  refused_args += pc_cflags + [
    '-I' + join_paths(meson.current_build_dir(), '..'),
    '-I' + join_paths(meson.current_source_dir(), '..')]
  test('refused_options', find_program('sh'),
    args: [files('refused_options.sh')],
    env: {
      'GAOL_CXX': ' '.join(cpp_comp.cmd_array()),
      'GAOL_CXXFLAGS': ' '.join(refused_args),
    },
    suite: 'unit', timeout: 300)
endif

# The time per operation of GAOL and of the same operation on doubles, which
# the continuous integration prints: not a test
executable('gaol_performance', 'performance.cpp',
  dependencies: tests_dep,
  override_options: ['cpp_std=c++17'])
