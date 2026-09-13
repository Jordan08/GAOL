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
 *--------------------------------------------------------------------------
 * CVS: $Id: gaol_exact.h 54 2009-03-01 13:53:01Z goualard $
 * Last modified: 
 * By: 
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_exact.h
  \brief  Functions needed by dtoa.c to report inexact translation from
  ASCII to double by strtod().

  \note This is C code, not C++. Moreover, it is not meant to be included in
  any namespace.

  <long description>

  \author Frederic Goualard
  \date   2002-12-05
*/


#ifndef __gaol_exact_h__
#define __gaol_exact_h__

// To overcome problems with old versions of autoconf
#undef PACKAGE
#include "gaol/gaol_config.h"

#ifdef __cplusplus
extern "C" {
#endif
    
int get_inexact(void);
void clear_inexact(void);

#ifdef __cplusplus
}
#endif
   
    
/* For Visual C++, the <fenv.h> version of get_inexact() and clear_inexact(),
   defined in the lexer that uses them: gaol_exact.c would include their 32-bit
   x86 assembly version, and the CMake build does not compile it for Visual C++.
   As in the fork of GAOL by Fabrice Le Bars. */
#if defined (_MSC_VER) && HAVE_FENV_H
#  include "gaol/sysdeps/gaol_exact_c99.h"
#endif

#endif /* __gaol_exact_h__ */
