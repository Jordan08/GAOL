# ==================================================================
#  A compile test whose compilation has to fail (GAOL v5)
# ==================================================================
#
# The tests refused_finite_math_only, refused_fast_math and
# nodiscard_discard_cxx<N> (tests/CMakeLists.txt) run this script:
#
#   cmake -DORIGINAL=<source> -DSOURCE=<copy of the source>
#         -DBUILD_DIR=<build directory> -DCONFIG=<configuration>
#         -DTARGET=<target> -P tests/compile_error.cmake
#
# Each builds the object of its target, whose compilation has to fail with a
# message of the compiler, which CTest looks for in the output of the build.
# Should the compilation succeed once (a header that regressed, or the check
# that the test fails without the fix), the object is up to date, and if the
# header is then restored with its former modification time (cp -p, an
# archive), the build compiles nothing and prints no message: the test failed
# until the object was removed. The target therefore compiles SOURCE, a copy
# of its source ORIGINAL in the build tree, copied again and touched here
# before the build, which compiles it again whatever the times of the headers
# and of the source: CMake makes the copy when it configures, and does not
# configure again for a source restored with its former modification time.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

cmake_minimum_required(VERSION 3.14)

configure_file("${ORIGINAL}" "${SOURCE}" COPYONLY)
file(TOUCH "${SOURCE}")
# The output of the build goes to that of the test, where CTest looks for the
# message; the exit status of the build is not looked at, as a build fails for
# many reasons
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --config "${CONFIG}"
                        --target "${TARGET}")
