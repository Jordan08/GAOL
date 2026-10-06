# ==================================================================
#  The warning of a configure generated for another version (GAOL v5)
# ==================================================================
#
# CPack puts configure in the archive of the sources as it is committed, and
# configure says the version autoconf read from VERSION.txt when it generated
# it (configure --version): after a change of VERSION.txt without a new
# generation of configure, the archive of the new version holds a configure
# that says the old one. The CPack block of CMakeLists.txt warns of it when
# CMake configures, whether or not the archive is made. This script checks that
# it does, and only then, by configuring GAOL as the project built, from copies
# of the tree that differ by their VERSION.txt and their configure:
#
#   - VERSION.txt holds the version after the one configure was generated for:
#     the warning names both versions;
#   - VERSION.txt holds the version configure was generated for: no warning;
#   - the same configure with the line ends of Windows (a checkout with
#     core.autocrlf): the warning again;
#   - a configure without a line PACKAGE_VERSION=, and a tree without configure
#     (an archive of another kind): no warning, and CMake configures.
#
# The archive takes its name from the version CMake read when it configured:
# the copy configured for the version configure was generated for, VERSION.txt
# then holds the next one, and CPack has to configure the build directory
# again before it makes the archive (cmake/gaol_package_source.cmake), warn,
# and name the archive after the new version, CMake giving no warning of its
# own in the files CPack reads. CPack runs as the target
# package_source runs it, in the build directory, whatever the generator:
# Ninja configured again before package_source, the Makefile generators did
# not.
#
# A copy is made of symbolic links to the files and directories of the tree,
# but for CMakeLists.txt, VERSION.txt and configure, which are copies: CMake
# reads them from the copy, and the script never writes through a link, into
# the tree. The version the committed configure was generated for is asked of
# configure itself (configure --version, as a user of the archive sees it), not
# read as CMakeLists.txt reads it, so that the two do not share a mistake. The
# copy is configured with the generator, its program and the compilers that
# built GAOL, the compilers given to CMake as CC and CXX, the way CMake reads
# them with their arguments (CC="ccache gcc"). Without sh, or where the system
# makes no symbolic link, the script says that it is skipped
# (SKIP_REGULAR_EXPRESSION of tests/CMakeLists.txt) rather than failing.
#
# ctest runs it (tests/CMakeLists.txt), and by hand:
#
#   cmake -DGAOL_SOURCE_DIR=<sources of GAOL> -DGAOL_WORK_DIR=<directory>/cpack_stale_configure \
#         -DGAOL_GENERATOR=<generator> [-DGAOL_MAKE_PROGRAM=<its program>] \
#         "-DGAOL_C_COMPILER=<cc and its arguments>" "-DGAOL_CXX_COMPILER=<c++ and its arguments>" \
#         -P tests/cpack_stale_configure.cmake
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ

cmake_minimum_required(VERSION 3.14)

foreach(_name GAOL_SOURCE_DIR GAOL_WORK_DIR GAOL_GENERATOR GAOL_C_COMPILER GAOL_CXX_COMPILER)
  if("${${_name}}" STREQUAL "")
    message(FATAL_ERROR "${_name} is not given (see the head of this file)")
  endif()
endforeach()
# The script removes the whole of this directory, when it starts, and when it
# passes or is skipped (a failure leaves it to be looked at): it has to be the
# directory of this test
if(NOT GAOL_WORK_DIR MATCHES "/cpack_stale_configure$")
  message(FATAL_ERROR "GAOL_WORK_DIR, ${GAOL_WORK_DIR}, is not a directory of this test (cpack_stale_configure)")
endif()

set(_tree "${GAOL_WORK_DIR}/tree")
set(_build "${GAOL_WORK_DIR}/build")

# Removes the directory, the links of the copy one by one first. file(REMOVE)
# and file(REMOVE_RECURSE) remove a symbolic link, not what it points to,
# which holds also for the links CPack copies into the build directory: nothing
# is ever removed through them
macro(remove_work_dir)
  file(GLOB _links "${_tree}/*")
  foreach(_path ${_links})
    file(REMOVE "${_path}")
  endforeach()
  file(REMOVE_RECURSE "${GAOL_WORK_DIR}")
endmacro()

# Ends the script, which passes, saying why it is skipped: ctest reads the
# message (SKIP_REGULAR_EXPRESSION)
macro(skip why)
  remove_work_dir()
  message(STATUS "cpack_stale_configure: skipped, ${why}")
  return()
endmacro()

# The version configure was generated for, and the next one. Without sh, the
# status is the message of a command that could not start
execute_process(COMMAND sh "${GAOL_SOURCE_DIR}/configure" --version
  OUTPUT_VARIABLE _output RESULT_VARIABLE _status)
string(TOLOWER "${_status}" _lower)
if(_lower MATCHES "no such file")
  skip("sh did not run configure --version (${_status})")
endif()
if(NOT _status EQUAL 0 OR NOT _output MATCHES "^gaol configure ([0-9]+)\\.([0-9]+)\\.([0-9]+)\n")
  message(FATAL_ERROR "configure --version, which tells the version configure was generated for, gave (status ${_status}):\n${_output}")
endif()
set(_major "${CMAKE_MATCH_1}")
set(_minor "${CMAKE_MATCH_2}")
set(_micro "${CMAKE_MATCH_3}")
math(EXPR _next_micro "${_micro} + 1")
set(_old "${_major}.${_minor}.${_micro}")
set(_new "${_major}.${_minor}.${_next_micro}")

# What a run left
remove_work_dir()

file(MAKE_DIRECTORY "${_tree}")
# Whether the system makes symbolic links here, which a file system may not
file(CREATE_LINK "${GAOL_SOURCE_DIR}/VERSION.txt" "${GAOL_WORK_DIR}/probe" RESULT _result SYMBOLIC)
if(NOT _result STREQUAL "0")
  skip("the system made no symbolic link in ${GAOL_WORK_DIR}: ${_result}")
endif()
file(REMOVE "${GAOL_WORK_DIR}/probe")
file(GLOB _entries RELATIVE "${GAOL_SOURCE_DIR}" "${GAOL_SOURCE_DIR}/*")
foreach(_entry ${_entries})
  # The three files to copy, and the hidden ones (.git...), which no
  # configuration reads
  if(_entry MATCHES "^(CMakeLists\\.txt|VERSION\\.txt|configure|\\..*)$")
    continue()
  endif()
  # Not the directory of the build, which is within the sources with cmake -S .
  # -B build: the copy would hold the directory it is in
  string(FIND "${GAOL_WORK_DIR}/" "${GAOL_SOURCE_DIR}/${_entry}/" _within)
  if(_within EQUAL 0)
    continue()
  endif()
  file(CREATE_LINK "${GAOL_SOURCE_DIR}/${_entry}" "${_tree}/${_entry}" RESULT _result SYMBOLIC)
  if(NOT _result STREQUAL "0")
    message(FATAL_ERROR "Cannot link ${_entry} into ${_tree}: ${_result}")
  endif()
endforeach()
file(COPY "${GAOL_SOURCE_DIR}/CMakeLists.txt" DESTINATION "${_tree}")
# A file the regular expressions of CPACK_SOURCE_IGNORE_FILES do not match,
# which the archive has to hold: \.lo$ read without its backslash, as CMake 4
# read it without CPACK_VERBATIM_VARIABLES, matches it
file(WRITE "${_tree}/keep.halo" "Not ignored by CPack\n")

# The program of the generator, which CMake looks for in PATH otherwise: Ninja
# can be elsewhere
set(_make_program)
if(NOT "${GAOL_MAKE_PROGRAM}" STREQUAL "")
  set(_make_program "-DCMAKE_MAKE_PROGRAM=${GAOL_MAKE_PROGRAM}")
endif()

# Writes <version> in the VERSION.txt of the copy
function(write_version version)
  file(REMOVE "${_tree}/VERSION.txt")
  file(WRITE "${_tree}/VERSION.txt" "${version}\n")
endfunction()

# Configures the copy as it is, in the same directory of build each time (the
# checks of the compiler are cached), with the compilers that built GAOL, and
# checks its warning: <expected> is WARNS, with the versions it names, or SILENT
function(check_configure title version expected)
  write_version("${version}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "CC=${GAOL_C_COMPILER}" "CXX=${GAOL_CXX_COMPILER}"
            "${CMAKE_COMMAND}" -S "${_tree}" -B "${_build}" -G "${GAOL_GENERATOR}" ${_make_program}
    RESULT_VARIABLE _status OUTPUT_VARIABLE _output ERROR_VARIABLE _error)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR "${title}: CMake did not configure the copy (status ${_status}):\n${_output}\n${_error}")
  endif()
  # CMake wraps the text of a warning at its blanks, and writes two after a
  # full stop
  string(REGEX REPLACE "[ \t\r\n]+" " " _text "${_output} ${_error}")
  if(expected STREQUAL "WARNS")
    set(_warning "configure was generated for GAOL ${_old}, and VERSION.txt holds ${version}:")
    string(FIND "${_text}" "${_warning}" _found)
    if(_found EQUAL -1)
      message(FATAL_ERROR "${title}: CMake did not warn \"${_warning}\" (does configure still hold the line PACKAGE_VERSION='${_old}'?):\n${_error}")
    endif()
  else()
    string(FIND "${_text}" "configure was generated for" _found)
    if(NOT _found EQUAL -1)
      message(FATAL_ERROR "${title}: CMake warned, and should not have:\n${_error}")
    endif()
  endif()
  message(STATUS "${title}: ${expected}, as expected")
endfunction()

# The configure of the tree
file(COPY "${GAOL_SOURCE_DIR}/configure" DESTINATION "${_tree}")
check_configure("VERSION.txt ${_new}, configure generated for ${_old}" "${_new}" WARNS)
check_configure("VERSION.txt ${_old}, configure generated for ${_old}" "${_old}" SILENT)

# The archive of the sources after a change of VERSION.txt, the build directory
# being configured for the version configure was generated for
write_version("${_new}")
execute_process(COMMAND "${CMAKE_CPACK_COMMAND}" --config CPackSourceConfig.cmake
  WORKING_DIRECTORY "${_build}" RESULT_VARIABLE _status OUTPUT_VARIABLE _output ERROR_VARIABLE _error)
set(_title "VERSION.txt ${_new} after a configuration for ${_old}, package_source")
if(NOT _status EQUAL 0)
  message(FATAL_ERROR "${_title}: CPack did not make the archive (status ${_status}):\n${_output}\n${_error}")
endif()
# Without a warning of CMake for the files CPack reads: those of GAOL, which it
# reads without the policies of CMakeLists.txt, and the configuration CMake
# writes for it (an invalid escape sequence with CMake 3, CMP0010)
if("${_output}\n${_error}" MATCHES "CMake Warning \\(dev\\)")
  message(FATAL_ERROR "${_title}: CMake warned in a file CPack reads:\n${_output}\n${_error}")
endif()
string(REGEX REPLACE "[ \t\r\n]+" " " _text "${_output} ${_error}")
string(FIND "${_text}" "configure was generated for GAOL ${_old}, and VERSION.txt holds ${_new}:" _found)
if(_found EQUAL -1 OR NOT EXISTS "${_build}/gaol-${_new}.tar.gz" OR EXISTS "${_build}/gaol-${_old}.tar.gz")
  file(GLOB _archives RELATIVE "${_build}" "${_build}/*.tar.gz")
  message(FATAL_ERROR "${_title}: CPack did not configure again, warn and make gaol-${_new}.tar.gz "
    "(archives made: ${_archives}):\n${_output}\n${_error}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tzf "gaol-${_new}.tar.gz"
  WORKING_DIRECTORY "${_build}" OUTPUT_VARIABLE _list)
if(NOT _list MATCHES "(^|\n)gaol-${_new}/VERSION\\.txt\n")
  message(FATAL_ERROR "${_title}: gaol-${_new}.tar.gz holds no gaol-${_new}/VERSION.txt")
endif()
if(NOT _list MATCHES "(^|\n)gaol-${_new}/keep\\.halo\n")
  message(FATAL_ERROR "${_title}: gaol-${_new}.tar.gz holds no gaol-${_new}/keep.halo, which "
    "CPACK_SOURCE_IGNORE_FILES does not match (were its backslashes lost?)")
endif()
message(STATUS "${_title}: WARNS and gaol-${_new}.tar.gz, as expected")

# The same, with the line ends of Windows
file(READ "${GAOL_SOURCE_DIR}/configure" _script)
string(REPLACE "\n" "\r\n" _script "${_script}")
file(REMOVE "${_tree}/configure")
file(WRITE "${_tree}/configure" "${_script}")
check_configure("VERSION.txt ${_new}, configure with the line ends of Windows" "${_new}" WARNS)

# No line to read, and no configure
file(REMOVE "${_tree}/configure")
file(WRITE "${_tree}/configure" "#! /bin/sh\nexit 0\n")
check_configure("VERSION.txt ${_new}, configure without PACKAGE_VERSION=" "${_new}" SILENT)
file(REMOVE "${_tree}/configure")
check_configure("VERSION.txt ${_new}, no configure" "${_new}" SILENT)

remove_work_dir()
