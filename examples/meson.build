# Meson build of GAOL: the examples of examples/ (see examples.md)
#
# Created 2025-11-25 by Raphaël Chenouard
# Modified in 2026 by Jordan NININ (GAOL v5)
#
# Built where with-examples is true, as examples/CMakeLists.txt builds them
# with WITH_EXAMPLES; each checks what it prints, and is a test of the suite
# examples, which meson test --setup check (make check) runs and meson test
# (make test) leaves out. In C++17, with the threads of 13_rounding_environment.
threads_dep = dependency('threads')
gaol_examples = [
  '01_first_steps',
  '02_decimals_and_rump',
  '03_dependency_problem',
  '04_centered_form',
  '05_interval_newton',
  '06_global_optimization',
  '07_contractors',
  '08_sivia',
  '09_parameter_estimation',
  '10_krawczyk',
  '11_ode_enclosure',
  '12_affine_arithmetic',
  '13_rounding_environment',
  '14_generic_programming',
  '15_text_and_ieee1788',
  '16_Goldstein_Price',
]
foreach ex : gaol_examples
  exe = executable(ex, ex + '.cpp',
    dependencies: [gaol_dep, threads_dep],
    override_options: ['cpp_std=c++17'])
  test(ex, exe, suite: 'examples')
  check_targets += [exe]
endforeach
