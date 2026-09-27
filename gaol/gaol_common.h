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
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_common.h
  \brief  Functionalities commonly used

  \author Frederic Goualard
  \date   2001-10-03
*/


#ifndef __gaol_common_h__
#define __gaol_common_h__

#include <cmath>

#include <cstdlib>
#include "gaol/gaol_config.h"
#include "gaol/gaol_port.h"

// The commands of GAOL_DEBUG (below) write on std::cout: gaol/gaol_expression.h
// did not compile with GAOL_DEBUGGING, which configure --enable-debug defined,
// without <iostream> (GAOL v5)
#if GAOL_DEBUGGING
#  include <iostream>
#endif

namespace gaol_core {

  // defined in gaol_common.cpp; public, as GAOL_DEBUG reads it in the inline
  // functions of the headers, compiled into the code using GAOL (GAOL v5)
  extern __GAOL_PUBLIC__ int debug_level;

#if GAOL_DEBUGGING
#  define GAOL_DEBUG(lvl,cmd) do { if (debug_level>=lvl) {cmd;} } while(0)
#else
#  define GAOL_DEBUG(lvl,cmd)
#endif

/*!
  Test for oddness.

  \param x
  \return true if x is odd.
  \see even
  \warning Should be used only with integer-like types
*/
template<typename T> GAOL_NODISCARD INLINE
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
template<typename T> GAOL_NODISCARD INLINE bool
even(const T& x)
{
  return (!odd(x));
}

/*!
  \brief returns true if the double is negative.

  This functions returns true also for -0.0. The result is undefined
  if "a" is a NaN.
*/
GAOL_NODISCARD INLINE bool is_signed(double a)
{
  return gaol_signbit(a);
}

/*!
  \brief computation of the maximum of two doubles

  This version is commutative even when one of the operands is a NaN. It also returns
  +0 when comparing +0 and -0.
*/
GAOL_NODISCARD extern __GAOL_PUBLIC__ double maximum(double a, double b);

/*!
  \brief computation of the minimum of two doubles

  This version is commutative even when there is a NaN. It also returns
  -0 when comparing +0 and -0.
*/
GAOL_NODISCARD extern __GAOL_PUBLIC__ double minimum(double a, double b);

  /*!
    \brief Initialization of the library
    \return true if the library was not already initialized and false otherwise
    \param dbg_lvl current debugging level: a debugging code inside
    a call to GAOL_DEBUG will be executed only if its level is lower or
    equal to the current debugging level. A debugging level equal to zero
    means that no debugging message will appear.
  */
extern __GAOL_PUBLIC__  bool init(int dbg_lvl = 0);

  /*!
    \brief Cleanup function

    To be called right after the last use of GAOL. Unless GAOL is built with
    GAOL_PRESERVE_ROUNDING, it also sets back the rounding direction that the
    first call of init() found (GAOL v5). GAOL calls it itself when the
    program ends or the library is unloaded, if the program did not, and then
    frees what the initialization allocated.
    \return true the first time it is called and false afterwards
  */
extern __GAOL_PUBLIC__ bool cleanup(void);

/*!
  \macro Position of an error/warning

  \note To be used with gaol_warning() and gaol_error(). See below.
 */
#define GAOL_FILE_POS __FILE__,__LINE__

/*!
  \brief Reports a benign error
  \note this function only prints a message to the standard error stream.
 */
extern __GAOL_PUBLIC__ void gaol_warning(const char *warn);
extern __GAOL_PUBLIC__ void gaol_warning(const char *file, int line, const char *warn);


/*!
  \brief Reports a fatal error.
  \note this function only prints a message to the standard error stream.
  It is up to the user to decide how and when to stop the program.
 */
extern __GAOL_PUBLIC__ void gaol_error(const char *err);
extern __GAOL_PUBLIC__ void gaol_error(const char *file, int line, const char *err);

#if GAOL_EXCEPTIONS_ENABLED
#   define gaol_ERROR(excep,msg) do { throw gaol_core::excep(GAOL_FILE_POS,msg); } while (0)
#else
#   define gaol_ERROR(excep,msg) do { gaol_core::gaol_error(GAOL_FILE_POS,msg); std::abort(); } while (0)
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
  class __GAOL_PUBLIC__ gaol_initializer {
  public:
    gaol_initializer();
  };

  static gaol_initializer _gaol_initializer;

} // namespace gaol_core

#endif /* __gaol_common_h__ */
