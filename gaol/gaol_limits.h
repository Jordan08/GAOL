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
  \file   gaol_limits.h
  \brief  

  <long description>

  \author Frederic Goualard
  \date   2001-10-01
*/


#ifndef __gaol_limits_h__
#define __gaol_limits_h__

#include "gaol/gaol_config.h"

// <limits>, which every compiler of C++11 has: the builds no longer check for
// it (HAVE_LIMITS), and the definitions of std::numeric_limits written for the
// compilers without it are gone (GAOL v5)
#include <limits>

#endif /* __gaol_limits_h__ */
