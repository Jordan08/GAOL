# build_tests = get_option('with-test')
# if build_tests.to_lower() == 'yes'
if build_tests
  cppunit = dependency('cppunit')

  test_arithmetic = executable(
    'arithmetic',
    sources: ['tests.h', 'arithmetic.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of arithmetic', test_arithmetic)

  test_assignment = executable(
    'assignment',
    sources: ['tests.h', 'assignment.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of assignment', test_assignment)

  test_constants = executable(
    'constants',
    sources: ['tests.h', 'constants.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of constants', test_constants)

  test_constructor = executable(
    'constructor',
    sources: ['tests.h', 'constructor.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of constructor', test_constructor)

  test_essai = executable(
    'essai',
    sources: ['tests.h', 'essai.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of essai', test_essai)

  test_float_functions = executable(
    'float_functions',
    sources: ['tests.h', 'float_functions.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of float_functions', test_float_functions)

  test_fpu = executable(
    'fpu',
    sources: ['tests.h', 'fpu.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of fpu', test_fpu)

  test_generic_functions = executable(
    'generic_functions',
    sources: ['tests.h', 'generic_functions.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of generic_functions', test_generic_functions)

  test_input_output = executable(
    'input_output',
    sources: ['tests.h', 'input_output.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of input_output', test_input_output)

  test_interval_functions = executable(
    'interval_functions',
    sources: ['tests.h', 'interval_functions.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of interval_functions', test_interval_functions)

  test_intervalf = executable(
    'intervalf',
    sources: ['tests.h', 'intervalf.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of intervalf', test_intervalf)

  test_misc = executable(
    'misc',
    sources: ['tests.h', 'misc.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of misc', test_misc)

  test_non_arithmetic = executable(
    'non_arithmetic',
    sources: ['tests.h', 'non_arithmetic.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of non_arithmetic', test_non_arithmetic)

  test_relations = executable(
    'relations',
    sources: ['tests.h', 'relations.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of relations', test_relations)

  test_reverse_mappings = executable(
    'reverse_mappings',
    sources: ['tests.h', 'reverse_mappings.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of reverse_mappings', test_reverse_mappings)

  test_trigonometric = executable(
    'trigonometric',
    sources: ['tests.h', 'trigonometric.cpp'],
    link_with: gaollib,
    dependencies: deps + [cppunit],
    include_directories: '../',
  )
  test('Test of trigonometric', test_trigonometric)

  if simd
    test_interval2f = executable(
      'interval2f',
      sources: ['tests.h', 'interval2f.cpp'],
      link_with: gaollib,
      dependencies: deps + [cppunit],
      include_directories: '../',
    )
    test('Test of interval2f', test_interval2f)
  endif

  if get_option('check-perf')
    test_performances = executable(
      'performances',
      sources: ['tests.h', 'performances.cpp', 'performance_test_set.cpp'],
      link_with: gaollib,
      dependencies: deps + [cppunit],
      include_directories: '../',
    )
    PROBABILITIES = ['.05', '.1', '.1', '.75']
    operators = [
      'add',
      'sub',
      'mul',
      'div',
      'inv',
      'pow',
      'exp',
      'log',
      'cos',
      'sin',
      'tan',
      'acos',
      'asin',
      'atan',
      'cosh',
      'sinh',
      'tanh',
      'acosh',
      'asinh',
      'atanh',
    ]
    foreach op : operators
      test(
        'Performances test',
        test_performances,
        args: [op, '10', PROBABILITIES],
      )
    endforeach
    # run_command('date | tee performances_result.txt')
    # run_command('hostname | tee -a performances_result.txt')
    # run_command('echo "Compiler: @CXX@" | tee -a performances_result.txt')
    # run_command('echo "Mathematical library: '+mathlib+'" | tee -a performances_result.txt')
    # run_command('./performances add 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances sub 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances mul 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances div 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances inv 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances pow 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances cos 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances sin 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances tan 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances acos 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances asin 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances atan 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances cosh 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances sinh 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances tanh 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances acosh 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances asinh 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances atanh 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances exp 10 '+PROBABILITIES+' | tee -a performances_result.txt')
    # run_command('./performances log 10 '+PROBABILITIES+' | tee -a performances_result.txt')
  endif
endif
