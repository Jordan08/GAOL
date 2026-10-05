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
  \file   gaol_flags.h
  \brief  

  <long description>

  \author Frederic Goualard
  \date   2001-10-30
*/


#ifndef GAOL_FLAGS_H
#define GAOL_FLAGS_H

namespace gaol_core {


  /*!
    \brief Type for priority of operators in expressions
   */
  class prec_t {
  public:
    enum { 
      null_prec   =  0, // null node
      plus_prec   =  2, // +, -, 
      mult_prec   =  5, // *, /
      uminus_prec =  6, // unary minus, pow, 
      cst_prec    = 10  // constant, variable, sine, cosine, ...
    };
  };

} // namespace gaol_core

#endif /* GAOL_FLAGS_H */
