/*-*-C++-*------------------------------------------------------------------
 * GAOL -- not Just Another Interval Library
 *--------------------------------------------------------------------------
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002-2006 Laboratoire d'Informatique de
 *                         Nantes-Atlantique, France
 * Copyright (c) 2026 ENSTA, France
 *
 * By: Frederic Goualard, Jordan NININ
 *--------------------------------------------------------------------------
 * GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING.LIB file for information.
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
#endif /* GAOL_VERSION_H */
