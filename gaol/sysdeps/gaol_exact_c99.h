/*-*-C++-*------------------------------------------------------------------
 * gaol -- Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. It has been developed at
 * the Institut de Recherche en Informatique de Nantes, France
 * 
 * Copyright (c) 2002 Institut de Recherche en Informatique de Nantes
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated 
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard <Frederic.Goualard@irin.univ-nantes.fr>
 *--------------------------------------------------------------------------*/

/*!
  \file   gaol_exact_c99.h
  \brief  functions get_inexact() and clear_inexact() for c99-compliant 
  compilers

  <long description>

  \author Frederic Goualard
  \date   2002-12-05
*/


#ifndef GAOL_EXACT_C99_H
#define GAOL_EXACT_C99_H

/*
 The test on the existence of fenv.h shall be made before including this
 file.
 */
# include <fenv.h>

inline int get_inexact(void)
{
  return fetestexcept(FE_INEXACT) & FE_INEXACT; // C99 standard function
}

inline void clear_inexact(void)
{
  feclearexcept(FE_INEXACT);
}

#endif /* GAOL_EXACT_C99_H */
