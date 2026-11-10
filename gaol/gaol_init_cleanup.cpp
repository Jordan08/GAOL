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
\file   gaol_init_cleanup.cpp
\brief  

\author Frederic Goualard, Jordan NININ
\date   2005-05-10
*/

#include <iostream>
#include "gaol/gaol_common.h"
#include "gaol/gaol_init_cleanup.h"
using std::cerr;
using std::cout;
using std::endl;


namespace gaol_core {
  
  /**
  @class init_cleanup
  
  A static object of that class is created to automate initialization and cleanup of the library.
  */
  class init_cleanup {
    public:
    init_cleanup();
    ~init_cleanup();
  };
  
  init_cleanup::init_cleanup()
  {
    initialization_process();
  }
  
  init_cleanup::~init_cleanup()
  {
    cleanup_process();
  }
  
  
  void gaol_init_lib(void)
  {
    static init_cleanup _ic_object;
  }
  
  // The static object of each translation unit including GAOL's headers
  // (gaol/gaol_common.h)
  gaol_initializer::gaol_initializer()
  {
    gaol_init_lib();
  }
  
  void initialization_process(void)
  {
    #if GAOL_VERBOSE_MODE
    // The standard streams may not be constructed yet: GAOL initializes
    // itself before the static objects of the program
    std::ios_base::Init ios_init;
    #endif
    #if !GAOL_PRESERVE_ROUNDING
    GAOL_IF_VERBOSE(cerr << "Automatic initialization of gaol (rounding set to +oo)... ");	
    #else
    GAOL_IF_VERBOSE(cerr << "Automatic initialization of gaol... ");	
    #endif
    init();
    GAOL_IF_VERBOSE(cerr << "done" << endl);
  }
  
  void cleanup_process(void)
  {
    #if !GAOL_PRESERVE_ROUNDING
    GAOL_IF_VERBOSE(cerr << "Automatic cleanup of gaol (rounding set back)... ");
    #else
    GAOL_IF_VERBOSE(cerr << "Automatic cleanup of gaol... ");
    #endif
    cleanup();
    free_initialization();
    GAOL_IF_VERBOSE(cerr << "done" << endl);
  }
  
} // namespace gaol_core
