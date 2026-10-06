# ==================================================================
#  The tests of GAOL built with a GAOL brought in as a meson subproject
# ==================================================================
#
# Builds the tests of the parent directory as a meson project bringing GAOL in
# as a subproject would (GAOL v5): GAOL is then targets of this project, built
# with it, and gaol_dep gives the code of this project the flags of interval
# arithmetic, which the test flags checks. GAOL is to be in subprojects/gaol, a
# link to its sources, as the continuous integration makes it
# (.github/workflows/build-systems.yml):
#
#   mkdir tests/meson_subproject/subprojects
#   ln -s <sources of GAOL> tests/meson_subproject/subprojects/gaol
#   meson setup build-meson-subproject tests/meson_subproject --buildtype=release
#   meson test -C build-meson-subproject
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

project('gaol_tests_meson_subproject', 'cpp')

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
# gaol.pc the subproject wrote, which only gaol_dep gives them. Without them,
# the tests pass all the same (GCC 9.4, x86_64)
test('flags', find_program('python3'),
  args: [files('check_flags.py'), meson.build_root(),
         meson.current_source_dir()])
