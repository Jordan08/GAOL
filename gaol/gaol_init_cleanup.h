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
\file   gaol_init_cleanup.h
\brief  Initializes and cleans automatically the gaol environment.

Used to avoid havoc if the user forgets to call explicitly gaol::init() 
and gaol::cleanup.

\author Frederic Goualard, Jordan NININ
\date   2005-05-10
*/


#ifndef GAOL_INIT_CLEANUP_H
#define GAOL_INIT_CLEANUP_H

namespace gaol_core {
  
  void gaol_init_lib(void);
  
  void initialization_process(void);
  void cleanup_process(void);
  void free_initialization(void);
  
} // namespace gaol_core

#endif /* GAOL_INIT_CLEANUP_H */
