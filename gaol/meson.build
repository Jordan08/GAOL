# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-20 by Jordan NININ
#
configure_file(
  input: 'gaol_configuration_meson.h.in',
  output: 'gaol_configuration.h',
  configuration: conf_data,
  install: true,
  install_dir: 'include/gaol',
)

if flex.found()
  lgen = generator(flex, output: '@BASENAME@.cpp', arguments: ['-o', '@OUTPUT@', '@INPUT@'])
  lfiles = lgen.process('gaol_interval_lexer.lpp')
endif
if bison.found()
  pgen = generator(
    bison,
    output: ['@BASENAME@.cpp', '@BASENAME@.h'],
    arguments: ['@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'],
  )
  pfiles = pgen.process('gaol_interval_parser.ypp')
endif
lexer_files = ['gaol_interval_lexer.cpp']
parser_files = ['gaol_interval_parser.cpp', 'gaol_interval_parser.h']

FLOAT_INTERVALS_SOURCE = []
FLOAT_INTERVALS_HEADER = []
if float_intervals
  FLOAT_INTERVALS_SOURCE += ['gaol_intervalf.cpp']
  FLOAT_INTERVALS_HEADER += ['gaol_intervalf.h']
  if use_sse3
    FLOAT_INTERVALS_SOURCE += ['gaol_interval2f.cpp']
    FLOAT_INTERVALS_HEADER += ['gaol_interval2f.h']
  endif
endif

# The elementary functions of CORE-MATH (3rd/math-core), compiled into libgaol
# itself: nothing is linked along with GAOL. Each source is compiled with
# gaol/core_math_port.h included first, by the compiler rather than by the
# source, so that importing a newer CORE-MATH stays a plain copy.
core_math_functions = [
  'exp', 'log', 'pow', 'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'atan2',
  'sinh', 'cosh', 'tanh', 'asinh', 'acosh', 'atanh',
  # cbrt, which nth_root(x, 3) takes, and the exponentials and logarithms in
  # base 2 and 10, which IEEE 1788-2015 requires (Table 9.1) (GAOL v5)
  'cbrt', 'exp2', 'exp10', 'log2', 'log10',

  # The forward functions IEEE 1788-2015 recommends (Table 10.5) (GAOL v5)
  'expm1', 'exp2m1', 'exp10m1', 'sinpi', 'cospi', 'tanpi', 'acospi', 'atanpi',
]
incdir = include_directories(['.', '..', './sysdeps'])

core_math_root = meson.source_root() + '/3rd/math-core/src/binary64/'
core_math_sources = []
foreach f : core_math_functions
  core_math_sources += [core_math_root + f + '/' + f + '.c']
endforeach
core_math_port = meson.source_root() + '/gaol/core_math_port.h'
if cpp_comp.get_argument_syntax() == 'msvc'
  core_math_args = ['/FI' + core_math_port]
else
  core_math_args = ['-include', core_math_port]
  # CORE-MATH asks for its computations to be rounded to double precision
  # (FLT_EVAL_METHOD = 0, see 3rd/math-core/README.md)
  if c_comp.has_argument('-fexcess-precision=standard')
    core_math_args += ['-fexcess-precision=standard']
  endif
endif

core_math_lib = static_library(
  'gaol_core_math',
  core_math_sources,
  c_args: core_math_args,
  include_directories: incdir,
  # The sources use the GNU extensions of C (the inline asm of atan2's tint.h),
  # which the c99 of the project refuses. And b_lto=false keeps the
  # interprocedural optimization of -Db_lto=true away from them: reached by it,
  # sin and cos took half as long again (see doc/three-builds.md)
  override_options: ['c_std=gnu11', 'b_lto=false'],
  pic: true,
  install: false,
)

fpu_source = 'gaol_interval_fpu.cpp'
if use_sse2
  fpu_source = 'gaol_interval_sse.cpp'
endif
fpu_target = custom_target(
  'gaol_fpu',
  output: 'gaol_interval_fpu.cpp',
  input: fpu_source,
  command: ['cp', '@INPUT@', '@OUTPUT@'],
  # install: true,
  # install_dir: 'include/gaol',
)

gaol_SOURCES = []
gaol_SOURCES += [
  'gaol_interval.cpp',
  'gaol_profile.cpp',
  'gaol_common.cpp',
  'gaol_parser.cpp',
  'gaol_expression.cpp',
  'gaol_port.cpp',
  'gaol_exceptions.cpp',
  'gaol_init_cleanup.cpp',
  's_nextafter.c',
  'gaol_exact.c',
  # CORE-MATH's log, where there is a 128-bit integer type (GAOL_CORE_MATH_LOG)
]
gaol_SOURCES += FLOAT_INTERVALS_SOURCE
foreach f : lexer_files
  if f.endswith('.cpp')
    gaol_SOURCES += [f]
  endif
endforeach
foreach f : parser_files
  if f.endswith('.cpp')
    gaol_SOURCES += [f]
  endif
endforeach

gaol_HEADERS = []
gaol_HEADERS += [
  'gaol',
  'gaol_assert.h',
  'gaol_exact.h',
  'gaol_core_math.h',
  'gaol_interval.h',
  'gaol_common.h',
  'gaol_exceptions.h',
  'gaol_config.h',
  'gaol_config_mingw.h',
  'gaol_config_msvc.h',
  'gaol_version_msvc.h',
  'gaol_version_mingw.h',
  'gaol_expression.h',
  'gaol_limits.h',
  'gaol_expr_eval.h',
  'gaol_expr_visitor.h',
  'gaol_parameters.h',
  'gaol_double_op.h',
  'gaol_u128.h',
  'gaol_roundeven.h',
  'gaol_flags.h',
  'gaol_parser.h',
  'gaol_port.h',
  'gaol_fpu.h',
  'gaol_fpu_fenv.h',
  'gaol_profile.h',
  'gaol.h',
  'gaol_eval_stack.h',
  'gaol_init_cleanup.h',
  'gaol_version.h',
  'gaol_allocator.h',
  'gaol_interval_fpu.h',
  'gaol_interval_sse.h',
]
foreach f : lexer_files
  if f.endswith('.h')
    gaol_HEADERS += [f]
  endif
endforeach
foreach f : parser_files
  if f.endswith('.h')
    gaol_HEADERS += [f]
  endif
endforeach

EXTRA_DIST = [
  'sysdeps/gaol_exact_c99.h',
  'sysdeps/gaol_exact_msvc.h',
  's_nextafter.c',
  'gaol_interval_lexer.lpp',
  'gaol_interval_parser.ypp',
  'dllmain.cpp',
  'gaol_config_mingw.h',
  'gaol_config_msvc.h',
  'gaol_version_msvc.h',
  'gaol_version_mingw.h',
  'gaol_interval_sse.cpp',
]


gaollib = both_libraries(
  'gaol',
  sources: gaol_SOURCES,
  dependencies: deps,
  link_whole: core_math_lib,
  install: true,
  extra_files: EXTRA_DIST,
  include_directories: incdir,
)

install_headers(gaol_HEADERS + FLOAT_INTERVALS_HEADER, subdir: 'gaol')
