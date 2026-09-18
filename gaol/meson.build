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

if mathlib == 'apmathlib'
  math_header = custom_target(
    'gen-math-header',
    input: 'gaol_double_op_apmathlib.h',
    output: 'gaol_double_op.h',
    # A copy rather than a hard link, which fails across file systems (a build
    # directory outside a bind mount in a container)
    command: ['cp', '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: 'include/gaol',
  )
elif mathlib == 'crlibm'
  math_header = custom_target(
    'gen-math-header',
    input: 'gaol_double_op_crlibm.h',
    output: 'gaol_double_op.h',
    # A copy rather than a hard link, which fails across file systems (a build
    # directory outside a bind mount in a container)
    command: ['cp', '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: 'include/gaol',
  )
else
  math_header = custom_target(
    'gen-math-header',
    input: 'gaol_double_op_m.h',
    output: 'gaol_double_op.h',
    # A copy rather than a hard link, which fails across file systems (a build
    # directory outside a bind mount in a container)
    command: ['cp', '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: 'include/gaol',
  )
endif

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

gaol_SOURCES = [math_header]
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
  # The hyperbolic functions of CORE-MATH (gaol_core_math.h)
  'core_math_sinh.c',
  'core_math_cosh.c',
  'core_math_tanh.c',
  'core_math_asinh.c',
  'core_math_acosh.c',
  'core_math_atanh.c',
  # CORE-MATH's log, where there is a 128-bit integer type (GAOL_CORE_MATH_LOG)
  'core_math_log.c',
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
  'gaol_expression.h',
  'gaol_limits.h',
  'gaol_expr_eval.h',
  'gaol_expr_visitor.h',
  'gaol_parameters.h',
  'gaol_double_op_apmathlib.h',
  'gaol_double_op_crlibm.h',
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
  'gaol_interval_sse.cpp',
]

incdir = include_directories(['.', '..', './sysdeps'])

gaollib = both_libraries(
  'gaol',
  sources: gaol_SOURCES,
  dependencies: deps,
  install: true,
  extra_files: EXTRA_DIST,
  include_directories: incdir,
)

install_headers(gaol_HEADERS + FLOAT_INTERVALS_HEADER, subdir: 'gaol')
