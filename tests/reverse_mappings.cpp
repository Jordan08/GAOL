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

// GAOL 4's unit test check/reverse_mappings.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"


using std::cout;
using std::endl;

class reverse_mappings_test {
	static const unsigned int MAX = 100;
	//static const unsigned int MAX = 100000;
  public:
  void setUp() {
    // The random numbers come from gaol_unit::uniform() and gaol_unit::integer(),
    // with a seed of their own, rather than from drand48() and rand() seeded
    // with the process identifier: the tests are the same at each run (GAOL v5)
  }
  void tearDown() {
  }

   // --> Beginning of tests
   void test_acos_rel() {
	  TEST_EMPTY(acos_rel(interval::emptyset(),interval::emptyset()));
	  TEST_EMPTY(acos_rel(interval(0,2),interval::emptyset()));
	  TEST_EMPTY(acos_rel(interval::universe(),interval::emptyset()));
	  TEST_EMPTY(acos_rel(interval::emptyset(),interval(0,2)));
	  TEST_EMPTY(acos_rel(interval::emptyset(),interval::universe()));

	  interval I, J;
     TEST_SEQ(acos_rel(interval::minus_one_plus_one(),interval(-5,6)),interval(-5,6));
	  TEST_EMPTY(acos_rel(interval(-3,-2),interval::universe()));
	  TEST_EMPTY(acos_rel(interval(2,4),interval::universe()));
	  // 2^52, which gaol/gaol_port.h no longer declares (GAOL v5)
	  const double two_power_52 = 4503599627370496.0;
	  I = interval(previous_float(two_power_52),two_power_52);
	  J = cos(I);
	  TEST_SEQ(acos_rel(J,I),I);
	  // Fails with a cosine compiled with contraction into fused multiply-adds,
	  // as GCC compiles by default on AArch64: cos(I) is then far from its
	  // value (see BRANRED_ARGUMENTS in tests/elementary_values.py)
	  I = interval(previous_float(previous_float(two_power_52)),previous_float(two_power_52));
	  J = cos(I);
	  TEST_SEQ(acos_rel(J,I),I);
	  TEST_HULL(acos_rel(interval(0.25,0.5),interval(1.02,1.32)), 0x1.0c152382d7365p+0, 0x1.51700e0c14b25p+0);
	  I = interval(-1e8,next_float(-1e8));
	  J = cos(I);
	  TEST_SEQ(acos_rel(J,I),I);
	  I = interval(previous_float(1e8),1e8);
	  J = cos(I);
	  TEST_SEQ(acos_rel(J,I),I);
	  I = interval::pi();
	  J = cos(I);
	  TEST_SEQ(acos_rel(J,I),I);
	  I = interval::pi();
	  J = cos(I);
	  TEST_SEQ(acos_rel(J,I),I);
	  for (int i = 10; i <= 10000; i*=10) {
        I = interval(0.5,0.7)+i*interval::pi();
	  	  J = cos(I);
		  TEST_SEQ(acos_rel(J,I),I);
	  }
	  TEST_HULL(acos_rel(interval(0.5,0.75),interval(-6.3,6.3)), -0x1.63de6e1ebf1eep+2, 0x1.63de6e1ebf1eep+2);
	  TEST_HULL(acos_rel(interval(0.5,0.75),interval(-2,6.3)), -0x1.0c152382d7366p+0, 0x1.63de6e1ebf1eep+2);
	  TEST_HULL(acos_rel(interval(0.5,0.75),interval(-6.3,2)), -0x1.63de6e1ebf1eep+2, 0x1.0c152382d7366p+0);
	  TEST_HULL(acos_rel(interval(0.9,0.95),interval(-0.32,0.2)), -0x1.47ae147ae147bp-2, -0x1.452e8fa93e43bp-2);
	  TEST_HULL(acos_rel(interval::universe(),interval(-3,2)), -0x1.8000000000000p+1, 0x1.0000000000000p+1);
	  TEST_HULL(acos_rel(interval::universe(),interval(0.5,0.6)), 0x1.0000000000000p-1, 0x1.3333333333333p-1);
	}

  void test_asin_rel() {
  }

  void test_atan_rel() {
	 TEST_EMPTY(atan_rel(interval::emptyset(),interval::emptyset()));
	 TEST_EMPTY(atan_rel(interval::emptyset(),interval(-3,2)));
	 TEST_EMPTY(atan_rel(interval(-3,2),interval::emptyset()));
	 interval I = interval(4.0,6.0);
	 interval J = tan(I);
	 TEST_SEQ(atan_rel(J,I),I);

	 interval TI[MAX];
	 for (unsigned int i = 0; i < MAX; ++i) {
		double Il = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		double Ir = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		reorder(Il,Ir);
		TI[i] = interval(Il,Ir);
	 }
	 for (unsigned int i = 0; i < MAX; ++i) {
	   J = tan(TI[i]);
		TEST_SEQ(atan_rel(J,TI[i]),TI[i]);
	 }
	 TEST_HULL(atan_rel(interval(-5,5),interval::universe()), -GAOL_INFINITY, GAOL_INFINITY);
	 TEST_HULL(atan_rel(interval::universe(),interval(4,5.5)), 0x1.0000000000000p+2, 0x1.6000000000000p+2);
	 TEST_HULL(atan_rel(interval::universe(),interval(4,6.5)), 0x1.0000000000000p+2, 0x1.a000000000000p+2);
	 TEST_HULL(atan_rel(interval(0.5,0.6),interval(3.7,7)), 0x1.afcc1c49a48cdp+2, 0x1.b4b5f0f034eb5p+2);
	 TEST_HULL(atan_rel(interval(0.5,0.6),interval(3.5,7)), 0x1.cd78834f06482p+1, 0x1.b4b5f0f034eb5p+2);
	 TEST_HULL(atan_rel(interval(0.5,0.6),interval(3.62,7)), 0x1.cf5c28f5c28f6p+1, 0x1.b4b5f0f034eb5p+2);
	 TEST_HULL(atan_rel(interval(0.5,0.6),interval(-7,-3.62)), -0x1.74734e3ee1164p+2, -0x1.6f89799850b7cp+2);
  }

  void test_sqrt_rel() {
  }

  void test_nth_root_rel() {
	TEST_EMPTY(nth_root_rel(interval(-3,-2),2,interval::universe()));
	TEST_HULL(nth_root_rel(interval(-3,2),2,interval::universe()), -0x1.6a09e667f3bcdp+0, 0x1.6a09e667f3bcdp+0);
	TEST_HULL(nth_root_rel(interval(2,3),2,interval::universe()), -0x1.bb67ae8584cabp+0, 0x1.bb67ae8584cabp+0);
	TEST_HULL(nth_root_rel(interval(2,3),2,interval(-2,-1.5)), -0x1.bb67ae8584cabp+0, -0x1.8000000000000p+0);
	TEST_HULL(nth_root_rel(interval(2,3),2,interval(1.5,2)), 0x1.8000000000000p+0, 0x1.bb67ae8584cabp+0);

	TEST_HULL(nth_root_rel(interval(2,3),5,interval::universe()), 0x1.2611186bae674p+0, 0x1.3ee8390d43956p+0);
	TEST_HULL(nth_root_rel(interval(0.5,3),5,interval::universe()), 0x1.bdb8cdadbe120p-1, 0x1.3ee8390d43956p+0);
	TEST_HULL(nth_root_rel(interval(-0.2,3),5,interval::universe()), -0x1.731651f09a68fp-1, 0x1.3ee8390d43956p+0);
	TEST_HULL(nth_root_rel(interval(-3,-2),5,interval::universe()), -0x1.3ee8390d43956p+0, -0x1.2611186bae674p+0);

	TEST_HULL(nth_root_rel(interval(0.2,0.5),5,interval::universe()), 0x1.731651f09a68ep-1, 0x1.bdb8cdadbe121p-1);
	TEST_HULL(nth_root_rel(interval(-3,-0.2),5,interval::universe()), -0x1.3ee8390d43956p+0, -0x1.731651f09a68ep-1);

  }

  void test_invabs_rel() {
	TEST_EMPTY(invabs_rel(interval::emptyset(),interval::emptyset()));
	TEST_EMPTY(invabs_rel(interval::emptyset(),interval::universe()));
	TEST_EMPTY(invabs_rel(interval::universe(),interval::emptyset()));
	TEST_SEQ(invabs_rel(interval::universe(),interval::universe()),interval::universe());
	TEST_SEQ(invabs_rel(interval(2,3),interval(1,2.5)),interval(2,2.5));
	TEST_SEQ(invabs_rel(interval(2,3),interval(-2.5,-1)),interval(-2.5,-2));
	TEST_SEQ(invabs_rel(interval(2,3),interval(-2.5,2.75)),interval(-2.5,2.75));
	TEST_SEQ(invabs_rel(interval(2,3),interval(-10,1)),interval(-3,-2));
	TEST_SEQ(invabs_rel(interval(-3,4),interval(2,3)),interval(2,3));
	TEST_SEQ(invabs_rel(interval(-3,4),interval(-3,-2)),interval(-3,-2));
  }

  void test_div_rel() {
	 interval TI[MAX], TJ[MAX];
	 for (unsigned int i = 0; i < MAX; ++i) {
		double Il = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		double Ir = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		reorder(Il,Ir);
		TI[i] = interval(Il,Ir);
	 }
	 for (unsigned int i = 0; i < MAX; ++i) {
		double Il = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		double Ir = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		reorder(Il,Ir);
		TJ[i] = interval(Il,Ir);
	 }
	 for (unsigned int i = 0; i < MAX; ++i) {
	   interval K = TI[i]*TJ[i];
		TEST_SEQ(div_rel(K,TJ[i],TI[i]),TI[i]);
	 }
	
	TEST_HULL(div_rel(interval(-6,0),interval(3),interval(-2,0)), -0x1.0000000000000p+1, 0x0.0p+0);
	TEST_HULL(div_rel(interval(-4.7,-1.5),interval(3),interval(-2,0)), -0x1.9111111111112p+0, -0x1.0000000000000p-1);
	TEST_HULL(div_rel(interval(-1.5,0),interval(3),interval(-2,0)), -0x1.0000000000000p-1, 0x0.0p+0);
	TEST_HULL(div_rel(interval(-3,0),interval(3),interval(-2,0)), -0x1.0000000000000p+0, 0x0.0p+0);

  }

  void test_acosh_rel() {
	 TEST_EMPTY(acosh_rel(interval::emptyset(),interval::emptyset()));
	 TEST_EMPTY(acosh_rel(interval(0,2),interval::emptyset()));
	 TEST_EMPTY(acosh_rel(interval::universe(),interval::emptyset()));
	 TEST_EMPTY(acosh_rel(interval::emptyset(),interval(0,2)));
	 TEST_EMPTY(acosh_rel(interval::emptyset(),interval::universe()));

	 interval TI[MAX];
	 for (unsigned int i = 0; i < MAX; ++i) {
		double Il = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		double Ir = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		reorder(Il,Ir);
		TI[i] = interval(Il,Ir);
	 }
	 for (unsigned int i = 0; i < MAX; ++i) {
	   interval J = cosh(TI[i]);
		TEST_SEQ(acosh_rel(J,TI[i]),TI[i]);
	 }
  }

  void test_asinh_rel() {
	 TEST_EMPTY(asinh_rel(interval::emptyset(),interval::emptyset()));
	 TEST_EMPTY(asinh_rel(interval(0,2),interval::emptyset()));
	 TEST_EMPTY(asinh_rel(interval::universe(),interval::emptyset()));
	 TEST_EMPTY(asinh_rel(interval::emptyset(),interval(0,2)));
	 TEST_EMPTY(asinh_rel(interval::emptyset(),interval::universe()));

	 interval TI[MAX];
	 for (unsigned int i = 0; i < MAX; ++i) {
		double Il = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		double Ir = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		reorder(Il,Ir);
		TI[i] = interval(Il,Ir);
	 }
	 for (unsigned int i = 0; i < MAX; ++i) {
	   interval J = sinh(TI[i]);
		TEST_SEQ(asinh_rel(J,TI[i]),TI[i]);
	 }

  }

  void test_atanh_rel() {
	 TEST_EMPTY(atanh_rel(interval::emptyset(),interval::emptyset()));
	 TEST_EMPTY(atanh_rel(interval(0,2),interval::emptyset()));
	 TEST_EMPTY(atanh_rel(interval::universe(),interval::emptyset()));
	 TEST_EMPTY(atanh_rel(interval::emptyset(),interval(0,2)));
	 TEST_EMPTY(atanh_rel(interval::emptyset(),interval::universe()));

	 // tanh takes its values in (-1, 1), where atanh is defined (IEEE
	 // 1788-2015, Table 9.1): no x has its tanh in [1], [1,5], [1,+oo], [-1]
	 // or [-5,-1]. GAOL gave [MAX,+oo] for the first three
	 TEST_EMPTY(atanh_rel(interval(1.0),interval::universe()));
	 TEST_EMPTY(atanh_rel(interval(1.0,5.0),interval::universe()));
	 TEST_EMPTY(atanh_rel(interval(1.0,GAOL_INFINITY),interval(0.0,GAOL_INFINITY)));
	 TEST_EMPTY(atanh_rel(interval(-1.0),interval::universe()));
	 TEST_EMPTY(atanh_rel(interval(-5.0,-1.0),interval::universe()));
	 // ... while a J that holds points of (-1, 1) keeps its preimage, whose
	 // end is +oo where J ends at 1: tanh(x) is in [0.5,1] from atanh(0.5)
	 // on, and every x has its tanh in [-1,1]
	 TEST_HULL(atanh_rel(interval(0.5,1),interval(0,100)), 0x1.193ea7aad030ap-1, 0x1.9000000000000p+6);
	 TEST_HULL(atanh_rel(interval(-1,1),interval(1,2)), 0x1.0000000000000p+0, 0x1.0000000000000p+1);
	 // The infinite ends themselves, which a bounded I hides: TEST_HULL has an
	 // infinite bound be the one asked for (GAOL v5)
	 TEST_HULL(atanh_rel(interval(0.5,1),interval(0,GAOL_INFINITY)), 0x1.193ea7aad030ap-1, GAOL_INFINITY);
	 TEST_HULL(atanh_rel(interval(-1,-0.5),interval(-GAOL_INFINITY,0)), -GAOL_INFINITY, -0x1.193ea7aad030ap-1);

	 interval TI[MAX];
	 for (unsigned int i = 0; i < MAX; ++i) {
		double Il = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		double Ir = gaol_unit::uniform()*(gaol_unit::integer() % 1000000);
		reorder(Il,Ir);
		TI[i] = interval(Il,Ir);
	 }
	 for (unsigned int i = 0; i < MAX; ++i) {
	   interval J = tanh(TI[i]);
		TEST_SEQ(atanh_rel(J,TI[i]),TI[i]);
	 }

  }
  // <-- End of tests
  protected:
  void reorder(double & l, double& r)
  {
	 if (l > r) {
		double tmp = l;
		l = r;
		r = tmp;
	 }
  }
};

GAOL_UNIT_MAIN(reverse_mappings_test, "reverse_mappings",
               GAOL_UNIT_TEST(test_acos_rel),
               GAOL_UNIT_TEST(test_asin_rel),
               GAOL_UNIT_TEST(test_atan_rel),
               GAOL_UNIT_TEST(test_sqrt_rel),
               GAOL_UNIT_TEST(test_nth_root_rel),
               GAOL_UNIT_TEST(test_invabs_rel),
               GAOL_UNIT_TEST(test_div_rel),
               GAOL_UNIT_TEST(test_acosh_rel),
               GAOL_UNIT_TEST(test_asinh_rel),
               GAOL_UNIT_TEST(test_atanh_rel))
