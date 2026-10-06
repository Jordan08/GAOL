# ==================================================================
#  The archive of the sources after a change of VERSION.txt (GAOL v5)
# ==================================================================
#
# CPack includes this file when it makes a package, once per generator
# (CPACK_PROJECT_CONFIG_FILE, set by CMakeLists.txt). The archive of the
# sources (package_source) takes its name and its version from
# CPackSourceConfig.cmake, which CMake writes in the build directory when it
# configures. Before package_source, Ninja configures again after a change of
# VERSION.txt (CMAKE_CONFIGURE_DEPENDS), but the Makefile generators run CPack
# at once: the archive was named after the old version, held the new
# VERSION.txt, and CMake did not say whether configure had been generated for
# it. So when VERSION.txt holds another version than the build directory was
# configured for, CMake configures it again here, which warns of a configure
# generated for another version, and the configuration it writes again is read
# in place of the old one. Only for the archive of the sources: the target
# package builds first, which configures again with every generator.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-10-06 by Jordan NININ

# CPack reads this file without the policies of CMakeLists.txt: CMake 3 would
# warn of CMP0007 at the list() of gaol_read_version() otherwise
cmake_policy(PUSH)
cmake_policy(VERSION 3.14...3.25)

# CPack sets CPACK_INSTALLED_DIRECTORIES for the archive of the sources only:
# the packages of the library install the build instead. The version compared
# is GAOL's own, CPACK_PACKAGE_VERSION being that of cpack -R when it is given
if(CPACK_INSTALLED_DIRECTORIES)
  include("${CPACK_GAOL_SOURCE_DIR}/cmake/gaol_version.cmake")
  gaol_read_version("${CPACK_GAOL_SOURCE_DIR}/VERSION.txt" _gaol_version _gaol_version_error)
  # A VERSION.txt refused configures again too, which fails with its message
  if(NOT _gaol_version STREQUAL CPACK_GAOL_VERSION)
    message(STATUS "VERSION.txt no longer holds ${CPACK_GAOL_VERSION}, the version "
      "${CPACK_GAOL_BINARY_DIR} was configured for: CMake configures it again")
    execute_process(
      COMMAND "${CPACK_GAOL_CMAKE_COMMAND}" -S "${CPACK_GAOL_SOURCE_DIR}" -B "${CPACK_GAOL_BINARY_DIR}"
      RESULT_VARIABLE _gaol_status)
    if(NOT _gaol_status EQUAL 0)
      message(FATAL_ERROR "CMake did not configure ${CPACK_GAOL_BINARY_DIR} again (status ${_gaol_status})")
    endif()
    include("${CPACK_SOURCE_OUTPUT_CONFIG_FILE}")
  endif()
endif()

cmake_policy(POP)
