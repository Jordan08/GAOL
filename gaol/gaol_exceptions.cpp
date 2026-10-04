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
  \file   gaol_exceptions.cpp
  \brief  

  <long description>

  \author Frederic Goualard
  \date   2002-12-02
*/

#include "gaol/gaol_config.h"

#if GAOL_EXCEPTIONS_ENABLED

#include <iostream>

#include "gaol/gaol_exceptions.h"

namespace gaol_core {

  // Class gaol_exception
  gaol_exception::gaol_exception(const char* f, unsigned l)
  {
    which_file_=f;
    which_line_=l;
  }

  gaol_exception::gaol_exception(const char* f, unsigned l, const std::string& e)
  {
    which_file_=f;
    which_line_=l;
    explanation_ = e;
  }

  const char*
  gaol_exception::file() const
  {
    return which_file_;
  }

  unsigned int
  gaol_exception::line() const
  {
    return which_line_;
  }


  std::string
  gaol_exception::explanation() const
  {
    return explanation_;
  }

  /*
    The explanation, or a text of its own where there is none, so that what a
    handler of std::exception prints, or the terminate handler of an exception
    nothing catches, is never empty (GAOL v5). GAOL 4 left the what() of
    std::exception, "std::exception", whatever went wrong.
  */
  const char*
  gaol_exception::what() const noexcept
  {
    return explanation_.empty() ? "gaol_exception" : explanation_.c_str();
  }

  std::ostream& operator<<(std::ostream& out, const gaol_exception &e)
  {
    // what() is the explanation now, written below: not here too (GAOL v5)
    out << e.file() << ", line " << e.line() << ": exception thrown";
    if (e.explanation().length() != 0) { // Some explanation given?
      out << ": " << e.explanation();
    }
    return out;
  }

} // namespace gaol_core

#endif /* GAOL_EXCEPTIONS_ENABLED */
