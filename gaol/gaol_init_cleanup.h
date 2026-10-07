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
\file   gaol_init_cleanup.h
\brief  Initializes and cleans automatically the gaol environment.

Used to avoid havoc if the user forgets to call explicitly gaol::init() 
and gaol::cleanup.

\author Frederic Goualard, Joran NININ
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
