/*-*-C++-*------------------------------------------------------------------
* gaol -- NOT Just Another Interval Library
*--------------------------------------------------------------------------
* This file is part of the gaol distribution. Gaol was primarily
* developed at the Swiss Federal Institute of Technology, Lausanne,
* Switzerland, and is now developed at the Laboratoire d'Informatique de
* Nantes-Atlantique, France.
*
* Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
* Copyright (c) 2002-2006 Laboratoire d'Informatique de
*                         Nantes-Atlantique, France
* Copyright (c) 2026 ENSTA, France
*--------------------------------------------------------------------------
* gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
* COPYING file for information.
*--------------------------------------------------------------------------*/

/*!
\file   gaol_common.h
\brief  Functionalities commonly used

\author Frederic Goualard, Jordan NININ
\date   2001-10-03
*/


#ifndef GAOL_COMMON_H
#define GAOL_COMMON_H

#include <cmath>

// The classes of <stdexcept> that gaol_ERROR (below) takes are declared with
// exceptions or without: its static_assert needs the type in both (GAOL v5)
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include "gaol/gaol_config.h"
#include "gaol/gaol_port.h"

// The text of the exceptions, error_text() below
#if GAOL_EXCEPTIONS_ENABLED
#  include <string>
#endif

// The commands of GAOL_DEBUG_VERBOSE (below) write on std::cout:
// gaol/parser/gaol_expression.h did not compile with GAOL_DEBUGGING, which configure
// --enable-debug defined, without <iostream> (GAOL v5)
#if GAOL_DEBUGGING
#  include <iostream>
#endif

namespace gaol_core {
  
  // defined in gaol_common.cpp; public, as GAOL_DEBUG_VERBOSE reads it in the
  // inline functions of the headers, compiled into the code using GAOL (GAOL
  // v5)
  extern GAOL_PUBLIC int debug_level;
  
  // GAOL_DEBUG before GAOL v5, now the option of CMake that builds GAOL for
  // debugging (GAOL v5)
  #if GAOL_DEBUGGING
  #  define GAOL_DEBUG_VERBOSE(lvl,cmd) \
  do { if (debug_level>=lvl) {cmd;} } while(0)
  #else
  #  define GAOL_DEBUG_VERBOSE(lvl,cmd)
  #endif
  
  /*!
  Test for oddness.
  
  \param x
  \return true if x is odd.
  \see even
  \warning Should be used only with integer-like types
  */
  template<typename T> GAOL_NODISCARD GAOL_INLINE
  bool odd(const T& x)
  {
    return (x&1);
  }
  
  /*!
  Test for evenness.
  
  \param x
  \return true if x is even.
  \see odd
  */
  template<typename T> GAOL_NODISCARD GAOL_INLINE bool
  even(const T& x)
  {
    return (!odd(x));
  }
  
  /*!
  \brief returns true if the double is negative.
  
  This functions returns true also for -0.0. The result is undefined
  if "a" is a NaN.
  */
  GAOL_NODISCARD GAOL_INLINE bool is_signed(double a)
  {
    return gaol_signbit(a);
  }
  
  /*!
  \brief computation of the maximum of two doubles
  
  This version is commutative even when one of the operands is a NaN. It also returns
  +0 when comparing +0 and -0.
  */
  GAOL_NODISCARD extern GAOL_PUBLIC double maximum(double a, double b);
  
  /*!
  \brief computation of the minimum of two doubles
  
  This version is commutative even when there is a NaN. It also returns
  -0 when comparing +0 and -0.
  */
  GAOL_NODISCARD extern GAOL_PUBLIC double minimum(double a, double b);
  
  /*!
  \brief Initialization of the library
  \return true if the library was not already initialized and false otherwise
  \param dbg_lvl current debugging level: a debugging code inside
  a call to GAOL_DEBUG_VERBOSE will be executed only if its level is lower or
  equal to the current debugging level. A debugging level equal to zero
  means that no debugging message will appear.
  */
  extern GAOL_PUBLIC  bool init(int dbg_lvl = 0);
  
  /*!
  \brief Cleanup function
  
  To be called right after the last use of GAOL. Unless GAOL is built with
  GAOL_PRESERVE_ROUNDING, it also sets back the rounding direction that the
  first call of init() found (GAOL v5). GAOL calls it itself when the
  program ends or the library is unloaded, if the program did not, and then
  frees what the initialization allocated.
  \return true the first time it is called and false afterwards
  */
  extern GAOL_PUBLIC bool cleanup(void);
  
  /*!
  \brief Sets back the rounding direction that the first call of init()
  found (GAOL v5)
  
  cleanup() does so at its first call only: a program that goes back to
  GAOL after it, whose operations set the direction upward again, calls
  restore_rounding() as many times as it needs. With
  GAOL_PRESERVE_ROUNDING, GAOL leaves the direction of the program alone,
  and restore_rounding() does nothing.
  */
  extern GAOL_PUBLIC void restore_rounding(void);
  
  /*!
  \macro Position of an error/warning
  
  \note To be used with gaol_warning() and gaol_error(). See below.
  */
  #define GAOL_FILE_POS __FILE__,__LINE__
  
  /*!
  \brief Reports a benign error
  \note this function only prints a message to the standard error stream.
  With a file name (the second form), a null pointer there is written
  "(unknown file)", and a null message is an empty text (GAOL v5).
  */
  extern GAOL_PUBLIC void gaol_warning(const char *warn);
  extern GAOL_PUBLIC void gaol_warning(const char *file, int line, const char *warn);
  
  
  /*!
  \brief Reports a fatal error.
  \note this function only prints a message to the standard error stream.
  It is up to the user to decide how and when to stop the program.
  With a file name (the second form), a null pointer there is written
  "(unknown file)", and a null message is an empty text (GAOL v5).
  */
  extern GAOL_PUBLIC void gaol_error(const char *err);
  extern GAOL_PUBLIC void gaol_error(const char *file, int line, const char *err);
  
  /*!
  \brief Reports an error of GAOL (GAOL v5)

  \param excep the exception to throw, a class derived from std::exception,
  which the compiler checks (static_assert, with or without exceptions), in
  practice a class of <stdexcept>: GAOL throws
  std::invalid_argument for a string that is no interval and
  std::domain_error for a function called with an argument it does not take.
  GAOL 4 had classes of its own, in gaol_core (gaol_exception,
  input_format_error, invalid_action_error and unavailable_feature_error),
  which GAOL v5 replaces by those of the standard library.
  \param msg the explanation of the error, a null pointer being no
  explanation

  With exceptions, throws \a excep with the text "file:line: msg", where file
  and line are those of the macro, the name of the file of __FILE__ without
  its directories, and __LINE__: what() of the exception says where it was
  thrown and why. Without exceptions
  (GAOL_EXCEPTIONS_ENABLED is 0), writes the error to the standard error
  stream, with gaol_error(), and calls abort().
  */
  #if GAOL_EXCEPTIONS_ENABLED
  #   define gaol_ERROR(excep,msg) do { \
        static_assert(::gaol_detail::is_error_class<excep>::value, "gaol_ERROR: excep must be derived from std::exception"); \
        throw excep(::gaol_detail::error_text(GAOL_FILE_POS,msg)); } while (0)
  #else
  #   define gaol_ERROR(excep,msg) do { \
        static_assert(::gaol_detail::is_error_class<excep>::value, "gaol_ERROR: excep must be derived from std::exception"); \
        gaol_core::gaol_error(GAOL_FILE_POS,msg); std::abort(); } while (0)
  #endif
  
  #if GAOL_VERBOSE_MODE
  #   define GAOL_IF_VERBOSE(a) do { a; } while (0)
  #else
  #   define GAOL_IF_VERBOSE(a)
  #endif
  
  /*!
  \brief Initializes GAOL before the static objects of the code using it
  
  Each translation unit including GAOL's headers holds a static object of
  this class, defined here, before its own static objects: its constructor
  initializes GAOL the first time only (gaol_init_lib(),
  gaol/gaol_init_cleanup.cpp), as <iostream> initializes the standard
  streams, and GAOL cleans up when the program ends, after the static
  objects constructed since are destroyed (GAOL v5). The first gaol::init()
  thus comes before the operations of GAOL in the static objects that such
  a unit defines after including GAOL, and finds the rounding direction the
  program started with, which gaol::cleanup() sets back. The function of
  GCC and Clang that GAOL declared __attribute__((constructor)) ran after
  the constructors of the program, GAOL being a static library, except with
  MinGW-w64, which runs them from the last linked to the first, and Visual
  C++ had none.
  */
  class GAOL_PUBLIC gaol_initializer {
    public:
    gaol_initializer();
  };
  
  static gaol_initializer _gaol_initializer;
  
} // namespace gaol_core

// What GAOL's headers define for their own use (see gaol/gaol_port.h)
namespace gaol_detail {
  /*
    A null pointer is no file name (GAOL v5, decided on 2026-10-24): written
    to a stream, which the standard leaves undefined and libstdc++ puts in the
    state bad, the diagnostics that followed were lost. The name of the file of
    the exceptions of GAOL (error_text() below), gaol_warning() and
    gaol_error() is this one: "(unknown file)" for a null pointer. The macros
    of GAOL always give __FILE__; a direct call can give a null pointer.
  */
  inline const char* known_file(const char* file)
  {
    return (file != nullptr) ? file : "(unknown file)";
  }

  /*
    A null pointer is no message either (GAOL v5, decided on 2026-10-24): an
    empty text, as in the text of the exceptions of GAOL (error_text() below)
  */
  inline const char* known_text(const char* text)
  {
    return (text != nullptr) ? text : "";
  }

  /*
    What gaol_ERROR takes for its first argument (GAOL v5): a class that
    std::exception is a public and unambiguous base of, so that a handler of
    std::exception catches it. The macro checks it with a static_assert in the
    two builds, with exceptions or without, where a type that is no exception
    (an int, a std::string) would otherwise compile in the build that aborts.
  */
  template <class E>
  struct is_error_class : std::integral_constant<bool, std::is_convertible<E*, std::exception*>::value> {};

#if GAOL_EXCEPTIONS_ENABLED
  /*
    The text of the exceptions of GAOL, "file:line: message": the standard
    exceptions that gaol_ERROR throws hold a text only, where those of GAOL 4
    had the file, the line and the explanation apart (GAOL v5). The file is
    the name of the source without its directories, after the last / or \ of
    __FILE__, which is the path the build gave the compiler (an absolute path
    with CMake, a relative one with meson): the path of the machine that built
    GAOL is not in the messages, and the text is the same with the three
    builds. It is "(unknown file)" for a null pointer, and the message is
    empty for one, as for gaol_warning() and gaol_error().
  */
  inline std::string error_text(const char* file, int line, const char* msg)
  {
    std::string text(known_file(file));
    const std::string::size_type slash = text.find_last_of("/\\");
    if (slash != std::string::npos) {
      text.erase(0, slash + 1);
    }
    text += ':';
    text += std::to_string(line);
    text += ": ";
    text += known_text(msg);
    return text;
  }
#endif
} // namespace gaol_detail

// In the namespace gaol too, as in GAOL 4, and restore_rounding() of GAOL v5;
// not gaol_initializer (see gaol/core/gaol_interval_core.h)
namespace gaol {
  using gaol_core::debug_level;
  using gaol_core::odd;
  using gaol_core::even;
  using gaol_core::is_signed;
  using gaol_core::maximum;
  using gaol_core::minimum;
  using gaol_core::init;
  using gaol_core::cleanup;
  using gaol_core::restore_rounding;
  using gaol_core::gaol_warning;
  using gaol_core::gaol_error;
} // namespace gaol

#endif /* GAOL_COMMON_H */
