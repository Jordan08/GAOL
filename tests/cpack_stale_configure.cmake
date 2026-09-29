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
# A copy is made of symbolic links to the files and directories of the tree,
# but for CMakeLists.txt, VERSION.txt and configure, which are copies: CMake
# reads them from the copy, and the script never writes through a link, into
# the tree. The version the committed configure was generated for is asked of
# configure itself (configure --version, as a user of the archive sees it), not
# read as CMakeLists.txt reads it, so that the two do not share a mistake.
#
# ctest runs it (tests/CMakeLists.txt), and by hand:
#
#   cmake -DGAOL_SOURCE_DIR=<sources of GAOL> -DGAOL_WORK_DIR=<directory>/cpack_stale_configure \
#         -DGAOL_GENERATOR=<generator> -DGAOL_C_COMPILER=<cc> -DGAOL_CXX_COMPILER=<c++> \
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
# The script removes what it made in this directory, and nothing else
if(NOT GAOL_WORK_DIR MATCHES "/cpack_stale_configure$")
  message(FATAL_ERROR "GAOL_WORK_DIR, ${GAOL_WORK_DIR}, is not a directory of this test (cpack_stale_configure)")
endif()

# The version configure was generated for, and the next one
execute_process(COMMAND sh "${GAOL_SOURCE_DIR}/configure" --version
  OUTPUT_VARIABLE _output RESULT_VARIABLE _status)
if(NOT _status EQUAL 0 OR NOT _output MATCHES "^gaol configure ([0-9]+)\\.([0-9]+)\\.([0-9]+)\n")
  message(FATAL_ERROR "configure --version, which tells the version configure was generated for, gave (status ${_status}):\n${_output}")
endif()
set(_major "${CMAKE_MATCH_1}")
set(_minor "${CMAKE_MATCH_2}")
set(_micro "${CMAKE_MATCH_3}")
math(EXPR _next_micro "${_micro} + 1")
set(_old "${_major}.${_minor}.${_micro}")
set(_new "${_major}.${_minor}.${_next_micro}")

set(_tree "${GAOL_WORK_DIR}/tree")
set(_build "${GAOL_WORK_DIR}/build")

# What a run left is removed with its links one by one, so that nothing is
# ever removed through them
file(GLOB _left "${_tree}/*")
foreach(_path ${_left})
  file(REMOVE "${_path}")
endforeach()
file(REMOVE_RECURSE "${GAOL_WORK_DIR}")

file(MAKE_DIRECTORY "${_tree}")
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

# Configures the copy as it is, in the same directory of build each time (the
# checks of the compiler are cached), with the compiler that built GAOL, and
# checks its warning: <expected> is WARNS, with the versions it names, or SILENT
function(check_configure title version expected)
  file(REMOVE "${_tree}/VERSION.txt")
  file(WRITE "${_tree}/VERSION.txt" "${version}\n")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${_tree}" -B "${_build}" -G "${GAOL_GENERATOR}"
            "-DCMAKE_C_COMPILER=${GAOL_C_COMPILER}" "-DCMAKE_CXX_COMPILER=${GAOL_CXX_COMPILER}"
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

file(GLOB _made "${_tree}/*")
foreach(_path ${_made})
  file(REMOVE "${_path}")
endforeach()
file(REMOVE_RECURSE "${GAOL_WORK_DIR}")
