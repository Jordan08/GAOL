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

// GAOL 4's unit test check/trigonometric.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include <iostream>
#include "unit_tests.h"

using namespace gaol;

class trigonometric_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests
  void test_cos() {
    TEST_EMPTY(cos(interval::emptyset()));

	TEST_HULL(cos(interval(-1,-0.5)), 0x1.14a280fb5068bp-1, 0x1.c1528065b7d50p-1);
    TEST_SEQ(cos(interval(-10,10)),interval(-1,1));
    TEST_SEQ(cos(interval::universe()),interval(-1,1));
    TEST_HULL_EXACT_LO(cos(interval::pi()), -0x1.0000000000000p+0, -0x1.fffffffffffffp-1);

	TEST_HULL_EXACT_LO(cos(interval(3,3.5)), -0x1.0000000000000p+0, -0x1.df77403c11a5ep-1);
	TEST_HULL_EXACT_LO(cos(interval(-3.5,-3)), -0x1.0000000000000p+0, -0x1.df77403c11a5ep-1);
	TEST_SEQ(cos(interval(-3.5,3)), interval(-1, 1));
	TEST_HULL(cos(interval(10,12)), -0x1.ad9ac890c6b20p-1, 0x1.b00da046b65e3p-1);
	TEST_HULL(cos(interval(13,14)), 0x1.1809aec2ca0ecp-3, 0x1.d09cdd5260cb8p-1);
	TEST_HULL_EXACT_HI(cos(interval(10,14)), -0x1.ad9ac890c6b20p-1, 0x1.0000000000000p+0);
	TEST_HULL_EXACT_LO(cos(interval(14,16)), -0x1.0000000000000p+0, 0x1.1809aec2ca0edp-3);
	TEST_HULL(cos(interval(-11,-10)), -0x1.ad9ac890c6b20p-1, 0x1.220ae41ee2fe0p-8);
	TEST_HULL(cos(interval(-14,-13)), 0x1.1809aec2ca0ecp-3, 0x1.d09cdd5260cb8p-1);
	TEST_HULL_EXACT_LO(cos(interval(-16,-14)), -0x1.0000000000000p+0, 0x1.1809aec2ca0edp-3);
	TEST_HULL_EXACT_HI(cos(interval(-102,-100)), 0x1.a01854b1e3e17p-4, 0x1.0000000000000p+0);
	TEST_SEQ(cos(interval(4600000000000000.0,4700000000000000.0)), interval(-1, 1));
	TEST_HULL(cos(interval(4503599627370495.0,4503599627370496.0)), -0x1.f1300d6815040p-2, 0x1.e4a6e3ea69b14p-2);
  }

  // sin() has no test here: tests/elementary.cpp and tests/core_math.cpp test
  // it, at doubles and over intervals

  void test_tan() {
	TEST_EMPTY(tan(interval::emptyset()));
 	TEST_SEQ(tan(interval::universe()),interval::universe());
	TEST_HULL_EXACT_LO(tan(interval(0,1)), 0x0.0p+0, 0x1.8eb245cbee3a6p+0);
	TEST_HULL_EXACT_HI(tan(interval(-1,0)), -0x1.8eb245cbee3a6p+0, 0x0.0p+0);
	TEST_HULL(tan(interval(-2,-1)), -GAOL_INFINITY, GAOL_INFINITY);
	TEST_HULL(tan(interval(202,203)), -GAOL_INFINITY, GAOL_INFINITY);
  }

  void test_acos() {
	TEST_EMPTY(acos(interval::emptyset()));
	TEST_EMPTY(acos(interval(-6,-3)));
	TEST_EMPTY(acos(interval(3,5)));
	TEST_HULL(acos(interval(-1.5,-0.5)), 0x1.0c152382d7365p+1, 0x1.921fb54442d19p+1);
	TEST_HULL(acos(interval(-0.5,0.5)), 0x1.0c152382d7365p+0, 0x1.0c152382d7366p+1);
	TEST_HULL(acos(interval(0.1)), 0x1.787b22ce3f590p+0, 0x1.787b22ce3f591p+0);
  }
  void test_asin() {
	TEST_EMPTY(asin(interval::emptyset()));
	TEST_EMPTY(asin(interval(-6,-3)));
	TEST_EMPTY(asin(interval(3,5)));
	TEST_HULL(asin(interval(-1.5,-0.5)), -0x1.921fb54442d19p+0, -0x1.0c152382d7365p-1);
	TEST_HULL(asin(interval(-0.5,0.5)), -0x1.0c152382d7366p-1, 0x1.0c152382d7366p-1);
	TEST_HULL(asin(interval(0.1)), 0x1.9a49276037884p-4, 0x1.9a49276037885p-4);
  }
  void test_atan() {
	TEST_EMPTY(atan(interval::emptyset()));
	TEST_HULL(atan(max_inf), 0x1.921fb54442d18p+0, 0x1.921fb54442d19p+0);
	TEST_HULL(atan(m_inf_m_max), -0x1.921fb54442d19p+0, -0x1.921fb54442d18p+0);
	TEST_HULL(atan(interval(-10,3)), -0x1.789bd2c160054p+0, 0x1.3fc176b7a8560p+0);
  }

  void test_cosh() {
	TEST_EMPTY(cosh(interval::emptyset()));
	TEST_SEQ(cosh(max_inf),max_inf);
	TEST_SEQ(cosh(m_inf_m_max),max_inf);
	TEST_HULL(cosh(interval(-5,-3)), 0x1.422a497d6185ep+3, 0x1.28d6fcbeff3aap+6);
	TEST_HULL_EXACT_LO(cosh(interval(-3,5)), 0x1.0000000000000p+0, 0x1.28d6fcbeff3aap+6);
	TEST_HULL_EXACT_LO(cosh(interval(-5,3)), 0x1.0000000000000p+0, 0x1.28d6fcbeff3aap+6);
	TEST_HULL(cosh(interval(3,5)), 0x1.422a497d6185ep+3, 0x1.28d6fcbeff3aap+6);
  }
  void test_sinh() {
	TEST_EMPTY(sinh(interval::emptyset()));
	TEST_SEQ(sinh(m_inf_m_max),m_inf_m_max);
	TEST_SEQ(sinh(max_inf),max_inf);
	TEST_HULL(sinh(interval(-3,5)), -0x1.40926e70949aep+3, 0x1.28d0166f07375p+6);
	TEST_HULL(sinh(interval::universe()), -GAOL_INFINITY, GAOL_INFINITY);
  }

  void test_tanh() {
	TEST_EMPTY(tanh(interval::emptyset()));
	TEST_HULL_EXACT_LO(tanh(m_inf_m_max), -0x1.0000000000000p+0, -0x1.fffffffffffffp-1);
	TEST_SEQ(tanh(max_inf),interval(previous_float(1),1));
	TEST_SEQ(tanh(interval::universe()), interval(-1, 1));
	TEST_HULL(tanh(interval(-3,5)), -0x1.fd77d111a0b00p-1, 0x1.fff419668df12p-1);
	TEST_HULL(tanh(interval(-1,0.5)), -0x1.85efab514f395p-1, 0x1.d9353d7568af4p-2);
  }

  void test_acosh() {
	TEST_EMPTY(acosh(interval::emptyset()));
	TEST_EMPTY(acosh(m_inf_m_max));
	TEST_CONT(interval(700,GAOL_INFINITY),acosh(max_inf));
	TEST_SEQ(acosh(interval(0,1)), interval(0));
	TEST_HULL(acosh(interval(2,4)), 0x1.5124271980434p+0, 0x1.081eb4b421592p+1);
	TEST_HULL(acosh(interval(100.1)), 0x1.53279947a8447p+2, 0x1.53279947a8448p+2);

  }
  void test_asinh() {
	TEST_EMPTY(asinh(interval::emptyset()));
	TEST_SEQ(asinh(interval::universe()),interval::universe());
	TEST_CONT(interval(-GAOL_INFINITY,-700),asinh(m_inf_m_max));
	TEST_CONT(interval(700,GAOL_INFINITY),asinh(max_inf));
	TEST_HULL(asinh(interval(-3,2)), -0x1.d185b507edc0ep+0, 0x1.719218313d088p+0);
  }

  void test_atanh() {
	TEST_EMPTY(atanh(interval::emptyset()));
	TEST_EMPTY(atanh(m_inf_m_max));
	TEST_EMPTY(atanh(max_inf));
	TEST_SEQ(atanh(interval::universe()),interval::universe());
	TEST_HULL(atanh(interval(-2,-0.5)), -GAOL_INFINITY, -0x1.193ea7aad030ap-1);
	TEST_HULL_EXACT_LO(atanh(interval(0,3)), 0x0.0p+0, GAOL_INFINITY);
	TEST_HULL(atanh(interval(-0.5,0.25)), -0x1.193ea7aad030bp-1, 0x1.058aefa811452p-2);
  }

  // <-- End of tests
};

GAOL_UNIT_MAIN(trigonometric_test, "trigonometric",
               GAOL_UNIT_TEST(test_cos),
               GAOL_UNIT_TEST(test_tan),
               GAOL_UNIT_TEST(test_acos),
               GAOL_UNIT_TEST(test_asin),
               GAOL_UNIT_TEST(test_atan),
               GAOL_UNIT_TEST(test_cosh),
               GAOL_UNIT_TEST(test_sinh),
               GAOL_UNIT_TEST(test_tanh),
               GAOL_UNIT_TEST(test_acosh),
               GAOL_UNIT_TEST(test_asinh),
               GAOL_UNIT_TEST(test_atanh))
