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
# ignored, the blanks being the six of ASCII that string(STRIP) removes (space,
# tab, line feed, vertical tab, form feed, carriage return), as configure and
# meson do; a NUL byte, as a file of UTF-16 characters holds, is refused.
# <version> is the version, or empty when the file is refused, and <error> is
# empty, or the message that refuses the file: it gives the first bytes of the
# file in hexadecimal, which show what the quoted text does not (a byte order
# mark or a zero-width space cannot be seen), and says to save a file of UTF-16
# characters as UTF-8 or ASCII.
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
  # A file of UTF-16 characters, which Windows PowerShell 5 writes for a
  # redirection (FF FE 35 00 2E 00...), holds NUL bytes, and a damaged file may:
  # it is refused, whatever the text before the first NUL byte. file(READ) does
  # not treat a NUL byte alike in every version of CMake (3.14.7 and 3.16.3
  # cut the text there, 4.4.3 keeps it, and a regular expression stops at it),
  # so the NUL byte is looked for in the reading of the whole file in
  # hexadecimal, which is the same in all: two digits per byte, 00 for NUL
  file(READ "${file}" _all HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "\\1;" _bytes "${_all}")
  list(FIND _bytes "00" _nul)
  # _shown is the text up to the first NUL byte, which the message quotes: a
  # message stops at a NUL byte too. The regular expression is not run on an
  # empty text (a file holding a mark alone, or nothing), where string(REGEX
  # MATCH) of CMake before 4.1 stops with an error
  set(_shown "")
  if(_text MATCHES "^.")
    string(REGEX MATCH "^.*" _shown "${_text}")
  endif()
  if(_nul EQUAL -1 AND _text MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$")
    set(${version} "${_text}" PARENT_SCOPE)
  else()
    file(READ "${file}" _hex LIMIT 32 HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "\\1 " _hex "${_hex}")
    string(STRIP "${_hex}" _hex)
    # A file of UTF-16 characters, which starts with its byte order mark (FF FE,
    # FE FF) or holds NUL bytes, is not read as UTF-16: the message says so, as
    # those of configure and meson do. The NUL byte, which the quoted text does
    # not show, is named
    set(_utf16 "")
    if(_head MATCHES "^(fffe|feff)")
      set(_utf16 "; the file is UTF-16: save it as UTF-8 or ASCII")
    elseif(NOT _nul EQUAL -1)
      set(_utf16 "; the file holds a NUL byte as UTF-16 does: save it as UTF-8 or ASCII")
    endif()
    set(${error} "VERSION.txt holds \"${_shown}\" (bytes in hexadecimal: ${_hex}), where it should hold the version of GAOL, three numbers without leading zeros such as 5.0.0${_utf16}" PARENT_SCOPE)
  endif()
endfunction()
