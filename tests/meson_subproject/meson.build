# ==================================================================
#  The tests of GAOL built with a GAOL brought in as a meson subproject
# ==================================================================
#
# Builds six of the tests of the parent directory, those of tests/fetch_content,
# as a meson project bringing GAOL in as a subproject would (GAOL v5): GAOL is
# then targets of this project, built with it, and gaol_dep gives the code of
# this project the flags of interval arithmetic, which the test flags checks.
# In the default build type of meson, debug, which this project is to be
# configured in, GAOL is built in release all the same, which the test release
# checks. GAOL is to be in subprojects/gaol, a link to its sources, as the
# continuous integration makes it (.github/workflows/build-systems.yml):
#
#   mkdir tests/meson_subproject/subprojects
#   ln -s <sources of GAOL> tests/meson_subproject/subprojects/gaol
#   meson setup build-meson-subproject tests/meson_subproject
#   meson test -C build-meson-subproject
#   rm tests/meson_subproject/subprojects/gaol
#
# The link, which git ignores, is to be removed after: it leads back to the
# root of the sources, and a copy of them that follows links, as tar -h does,
# would never end.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

project('gaol_tests_meson_subproject', 'cpp')

# A default test setup of this project's own, as a project may have: GAOL
# declared one too as a subproject, and meson stopped on it ("is_default can be
# set to true only once")
add_test_setup('meson_subproject', is_default: true)

gaol_dep = subproject('gaol').get_variable('gaol_dep')

foreach t : ['arithmetic', 'elementary', 'rounding_direction', 'numbers',
             'other_functions', 'reverse']
  exe = executable('gaol_test_' + t, '../' + t + '.cpp',
    dependencies: gaol_dep,
    override_options: ['cpp_std=c++17'])
  test(t, exe, timeout: 300)
endforeach

# The arguments GAOL gives its own targets do not reach those of this project:
# the commands that compile the tests above have to hold the flags of the
# gaol.pc the subproject wrote, and those that link them its link option
# (-mno-daz-ftz, where the compiler takes it), which only gaol_dep gives them.
# Without them, the tests pass all the same (GCC 9.4, x86_64)
test('flags', find_program('python3'),
  args: [files('check_flags.py'), meson.current_build_dir(),
         meson.current_source_dir()])

# The libraries of GAOL and CORE-MATH compiled as in release, -O3 and NDEBUG,
# and GAOL_DEBUGGING undefined: before 1.8.4, meson gives a subproject the
# build type of the project (see meson.build)
test('release', find_program('python3'),
  args: [files('../release_flags.py'), meson.current_build_dir(),
         join_paths(meson.current_source_dir(), 'subprojects', 'gaol')])
