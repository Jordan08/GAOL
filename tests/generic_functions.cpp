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

// GAOL 4's unit test check/generic_functions.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)

#include "unit_tests.h"

class generic_function_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests
  void test_generic_function() {
      CPPUNIT_ASSERT(!odd(4));
      CPPUNIT_ASSERT(odd(5));
      CPPUNIT_ASSERT(even(4));
      CPPUNIT_ASSERT(!even(5));
      CPPUNIT_ASSERT(!odd(-4));
      CPPUNIT_ASSERT(odd(-5));
      CPPUNIT_ASSERT(even(-4));
      CPPUNIT_ASSERT(!even(-5));
  }
  
  
  // <-- End of tests
};


GAOL_UNIT_MAIN(generic_function_test, "generic_functions",
               GAOL_UNIT_TEST(test_generic_function))
