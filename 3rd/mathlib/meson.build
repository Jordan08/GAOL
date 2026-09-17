# ==================================================================
#  meson build of mathlib, for the meson build of GAOL
# ==================================================================
#
# Builds libultim from the sources of this directory and installs it with its
# headers, as CMakeLists.txt does for the CMake build of GAOL. The meson build
# of GAOL reaches this file with subdir() when it uses the mathlib of
# 3rd/mathlib (see meson.build at the root of GAOL), once it has given its
# global arguments, which carry the flags of interval arithmetic. Defines
# ultim_dep.

# mathlib_configuration.h, in the build directory of this file, which meson
# puts on the include path of the library
configure_file(
  input: 'mathlib_configuration.h.in',
  output: 'mathlib_configuration.h',
  copy: true,
  install: true,
  install_dir: get_option('includedir'),
)

# The sources of libultim, as listed in src/Makefile.am
ultim_sources = files(
  'src/atnat2.c', 'src/halfulp.c', 'src/mplog.c', 'src/slowexp2.c', 'src/ucot.c', 'src/upow.c',
  'src/atnat.c', 'src/mpa.c', 'src/mpsqrt.c', 'src/slowexp.c', 'src/uexp2.c', 'src/urem.c',
  'src/branred.c', 'src/mpatan2.c', 'src/mptan.c', 'src/slowlog2.c', 'src/uexp.c', 'src/uroot.c',
  'src/doasin.c', 'src/mpatan.c', 'src/sincos32.c', 'src/slowpow.c', 'src/ulog2.c', 'src/usncs.c',
  'src/dosincos.c', 'src/mpexp.c', 'src/slowcot.c', 'src/uasncs.c', 'src/ulog.c', 'src/utan.c',
  'src/DPChange.c',
)

ultim_c_args = []
if c_comp.get_argument_syntax() == 'msvc'
  # As in CMakeLists.txt: not __declspec(dllexport), for a static library
  ultim_c_args += ['-D_MATHLIB_DLL_=extern']
endif
# mathlib's warnings turned off, as in CMakeLists.txt
if c_comp.has_argument('-w')
  ultim_c_args += ['-w']
endif

ultim_lib = static_library(
  'ultim',
  ultim_sources,
  c_args: ultim_c_args,
  install: true,
)
ultim_dep = declare_dependency(link_with: ultim_lib)

install_headers(
  'src/MathLib.h',
  'src/mathlib_config.h',
  'src/mathlib_config_msvc.h',
  'src/mathlib_config_mingw.h',
)
