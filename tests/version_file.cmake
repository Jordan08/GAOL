# ==================================================================
#  The reading of VERSION.txt by the CMake build (GAOL v5)
# ==================================================================
#
# The test version_file (tests/CMakeLists.txt) runs this script:
#
#   cmake -DGAOL_SOURCE_DIR=<sources of GAOL> -DWORK_DIR=<directory> -P tests/version_file.cmake
#
# CMakeLists.txt reads VERSION.txt with gaol_read_version(), which is in
# cmake/gaol_version.cmake, before project(). The function has to ignore a
# UTF-8 byte order mark at the start of the file, which some editors of Windows
# write, and the line ends of Windows (CR LF), and to refuse anything but three
# numbers without leading zeros, telling the bytes it found in hexadecimal. It
# refused a file starting with a byte order mark as VERSION.txt holds "5.0.0",
# the mark, whose three bytes make no visible character, being part of the
# text it quoted. The files are written here, and the versions expected are
# too: they are not taken from the code under test.
#
# The line ends are not compared with the bytes of the files: a platform that
# writes CR LF for LF changes none of what the checks ignore.
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ

cmake_minimum_required(VERSION 3.14)

if(NOT GAOL_SOURCE_DIR OR NOT WORK_DIR)
  message(FATAL_ERROR "usage: cmake -DGAOL_SOURCE_DIR=<sources of GAOL> -DWORK_DIR=<directory> -P version_file.cmake")
endif()
include("${GAOL_SOURCE_DIR}/cmake/gaol_version.cmake")
file(MAKE_DIRECTORY "${WORK_DIR}")

# The three bytes of a UTF-8 byte order mark, EF BB BF, and those of another
# character that is not seen, a zero-width space, E2 80 8B
string(ASCII 239 187 191 bom)
string(ASCII 226 128 139 zwsp)
set(crlf "\r\n")
set(file "${WORK_DIR}/VERSION.txt")

set_property(GLOBAL PROPERTY CHECKS 0)
set_property(GLOBAL PROPERTY FAILURES 0)

# One more check, or one more failure
function(count what)
  get_property(n GLOBAL PROPERTY ${what})
  math(EXPR n "${n} + 1")
  set_property(GLOBAL PROPERTY ${what} ${n})
endfunction()

function(fail message)
  message(SEND_ERROR "${message}")
  count(FAILURES)
endfunction()

# The file holds <content> and has to give the version <expected>
function(check_accepted title content expected)
  count(CHECKS)
  file(WRITE "${file}" "${content}")
  gaol_read_version("${file}" version error)
  if(error)
    fail("${title}: refused, ${error}")
  elseif(NOT version STREQUAL "${expected}")
    fail("${title}: read \"${version}\", expected \"${expected}\"")
  endif()
endfunction()

# The file <file> has to be refused, with a message that gives its bytes:
# <hex>, a regular expression, is what follows "bytes in hexadecimal: "
function(check_refused_file title file hex)
  count(CHECKS)
  gaol_read_version("${file}" version error)
  if(NOT error)
    fail("${title}: accepted as \"${version}\"")
  elseif(version)
    fail("${title}: refused and read \"${version}\"")
  elseif(NOT error MATCHES "^VERSION\\.txt holds .* \\(bytes in hexadecimal: ${hex}\\), where it should hold the version of GAOL")
    fail("${title}: the message does not give the bytes ${hex}: ${error}")
  endif()
endfunction()

# The file holds <content>, and has to be refused as above
function(check_refused title content hex)
  file(WRITE "${file}" "${content}")
  check_refused_file("${title}" "${file}" "${hex}")
endfunction()

# The mark has to be in the files below, where it is written: without it, the
# checks would test nothing
file(WRITE "${file}" "${bom}5.0.0")
file(READ "${file}" head LIMIT 3 HEX)
if(NOT head STREQUAL "efbbbf")
  message(FATAL_ERROR "the byte order mark written is ${head}, not efbbbf")
endif()

check_accepted("a line" "5.0.0\n" "5.0.0")
check_accepted("no line end" "5.0.0" "5.0.0")
check_accepted("a mark, a line" "${bom}5.0.0\n" "5.0.0")
check_accepted("a mark, no line end" "${bom}5.0.0" "5.0.0")
check_accepted("CR LF" "5.0.0${crlf}" "5.0.0")
check_accepted("a mark, CR LF" "${bom}5.0.0${crlf}" "5.0.0")
check_accepted("a mark, empty lines and blanks" "${bom}${crlf}  5.0.0 \t${crlf}${crlf}" "5.0.0")
check_accepted("numbers of several digits" "${bom}12.345.6789${crlf}" "12.345.6789")
check_accepted("zeros" "${bom}0.0.0\n" "0.0.0")

# In the bytes: 35 2e 30 2e 30 is 5.0.0, 78 is x, 20 is a blank, 0a the line
# end (which a platform may write as 0d 0a)
check_refused("a mark alone" "${bom}" "ef bb bf")
check_refused("two marks" "${bom}${bom}5.0.0" "ef bb bf ef bb bf 35 2e 30 2e 30")
check_refused("a mark inside" "5.${bom}0.0" "35 2e ef bb bf 30 2e 30")
check_refused("two numbers" "${bom}5.0" "ef bb bf 35 2e 30")
check_refused("four numbers" "${bom}5.0.0.1" "ef bb bf 35 2e 30 2e 30 2e 31")
check_refused("a leading zero" "${bom}05.0.0" "ef bb bf 30 35 2e 30 2e 30")
check_refused("a letter" "5.0.0x" "35 2e 30 2e 30 78")
check_refused("a zero-width space" "${bom}5.0.0${zwsp}" "ef bb bf 35 2e 30 2e 30 e2 80 8b")
check_refused("a blank inside" "5. 0.0" "35 2e 20 30 2e 30")
check_refused("two lines" "${bom}5.0.0\n6.0.0\n" "ef bb bf 35 2e 30 2e 30 .*36 2e 30 2e 30 .*")
check_refused("an empty file" "" "")
# CMake cannot write a NUL byte: tests/version_file/ holds two files that have
# some. The first is "5.0.0" and a line end in UTF-16, little-endian, with its
# byte order mark, as Windows PowerShell 5 writes for a redirection; the
# message has to be all there, which a NUL byte in the text quoted cut. The
# second is 5.0.0, a NUL byte and an x: a regular expression stops at the NUL
# byte, and took the text for 5.0.0
check_refused_file("UTF-16" "${GAOL_SOURCE_DIR}/tests/version_file/utf16le_bom.txt"
                   "ff fe 35 00 2e 00 30 00 2e 00 30 00 0a 00")
check_refused_file("a NUL byte after the version" "${GAOL_SOURCE_DIR}/tests/version_file/nul_after_version.txt"
                   "35 2e 30 2e 30 00 78")

get_property(checks GLOBAL PROPERTY CHECKS)
get_property(failures GLOBAL PROPERTY FAILURES)
if(failures)
  message(FATAL_ERROR "${failures} of ${checks} checks failed")
endif()
message(STATUS "${checks} checks, 0 failed")
