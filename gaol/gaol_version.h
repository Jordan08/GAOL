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
 * Modified by Joran NININ (GAOL v5)
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated 
 * COPYING file for information.
 *--------------------------------------------------------------------------*/
/*!
  \file   gaol_version.h
  \brief  

  <long description>

  \author Goualard Frederic
  \date   2001-09-28
*/


#ifndef GAOL_VERSION_H
#define GAOL_VERSION_H

// The version the build wrote into gaol/gaol_configuration.h, with every
// compiler: MinGW included nothing, and GAOL_MAJOR_VERSION was undefined there
// unless another header of GAOL had been included first (GAOL v5)
#include "gaol/gaol_config.h"

namespace gaol_core {
  const unsigned int version_major = GAOL_MAJOR_VERSION;
  const unsigned int version_minor = GAOL_MINOR_VERSION;
  const unsigned int version_micro = GAOL_MICRO_VERSION;
  const char *const version = GAOL_VERSION;
}

// In the namespace gaol too, as in GAOL 4 (see gaol/gaol_interval.h)
namespace gaol {
  using gaol_core::version_major;
  using gaol_core::version_minor;
  using gaol_core::version_micro;
  using gaol_core::version;
}
#endif /* GAOL_VERSION_H */
