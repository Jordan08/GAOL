# ==================================================================
#  The version of GAOL, read from VERSION.txt (GAOL v5)
# ==================================================================
#
# gaol_read_version(<file> <version> <error>) reads the version of GAOL from
# <file>, VERSION.txt, which configure.ac and meson.build read too (see
# doc/building.md). CMakeLists.txt includes this file before project(), and
# tests/version_file.cmake (the test version_file) calls the function with the
# files it writes.
#
# The file holds three numbers without leading zeros, which the macros
# GAOL_*VERSION write as C integers, and nothing else. A UTF-8 byte order mark
# (EF BB BF) at its start, which some editors of Windows write, the line ends
# of Windows (CR LF), and the blanks and empty lines around the version are
# ignored; a NUL byte, as a file of UTF-16 characters holds, is refused.
# <version> is the version, or empty when the file is refused, and <error> is
# empty, or the message that refuses the file: it gives the first bytes of the
# file in hexadecimal, which show what the quoted text does not (a byte order
# mark or a zero-width space cannot be seen).
#
# Copyright (c) 2026 ENSTA, France
#
# Created 2026-09-29 by Jordan NININ

function(gaol_read_version file version error)
  set(${version} "" PARENT_SCOPE)
  set(${error} "" PARENT_SCOPE)
  file(READ "${file}" _text)
  file(READ "${file}" _head LIMIT 3 HEX)
  if(_head STREQUAL "efbbbf")
    # The bytes of the mark, not characters: file(READ) does not decode
    string(SUBSTRING "${_text}" 3 -1 _text)
  endif()
  # string(STRIP) removes the CR of a line end of Windows as any other blank
  string(STRIP "${_text}" _text)
  # file(READ) keeps a NUL byte in the text, as those of a file of UTF-16
  # characters, which Windows PowerShell 5 writes for a redirection (FF FE 35
  # 00 2E 00...), but a regular expression and a message stop at the first NUL
  # byte: _shown is the text up to it, which is what the message quotes, and a
  # file holding a NUL byte is refused, whatever the text before it
  string(REGEX MATCH "^.*" _shown "${_text}")
  string(LENGTH "${_text}" _length)
  string(LENGTH "${_shown}" _shown_length)
  if(_length EQUAL _shown_length AND _text MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$")
    set(${version} "${_text}" PARENT_SCOPE)
  else()
    file(READ "${file}" _hex LIMIT 32 HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "\\1 " _hex "${_hex}")
    string(STRIP "${_hex}" _hex)
    set(${error} "VERSION.txt holds \"${_shown}\" (bytes in hexadecimal: ${_hex}), where it should hold the version of GAOL, three numbers without leading zeros such as 5.0.0" PARENT_SCOPE)
  endif()
endfunction()
