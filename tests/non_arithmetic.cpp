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

// GAOL 4's unit test check/non_arithmetic.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"

using namespace gaol;

class non_arithmetic_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests
  void test_pow_int() {
    TEST_SEQ(pow(interval::zero(),0),interval::one());
    CPPUNIT_ASSERT(pow(interval::emptyset(),0).is_empty());
    CPPUNIT_ASSERT(pow(interval::emptyset(),2).is_empty());
    CPPUNIT_ASSERT(pow(interval::emptyset(),3).is_empty());
    CPPUNIT_ASSERT(pow(interval::emptyset(),-2).is_empty());
    CPPUNIT_ASSERT(pow(interval::emptyset(),-3).is_empty());
    TEST_SEQ(pow(interval(-3,5),2), interval(0, 25));

   // Negative interval
    TEST_HULL_EXACT_HI(pow(interval(-3,-2),-4), 0x1.948b0fcd6e9e0p-7, 0x1.0000000000000p-4);
    TEST_HULL_EXACT_LO(pow(interval(-3,-2),-3), -0x1.0000000000000p-3, -0x1.2f684bda12f68p-5);
    TEST_HULL_EXACT_HI(pow(interval(-3,-2),-2), 0x1.c71c71c71c71cp-4, 0x1.0000000000000p-2);
    TEST_HULL_EXACT_LO(pow(interval(-3,-2),-1), -0x1.0000000000000p-1, -0x1.5555555555555p-2);
    TEST_SEQ(pow(interval(-3,-2),0),interval::one());
    TEST_SEQ(pow(interval(-3,-2),1), interval(-3, -2));
    TEST_SEQ(pow(interval(-3,-2),2), interval(4, 9));
    TEST_SEQ(pow(interval(-3,-2),3), interval(-27, -8));
    TEST_SEQ(pow(interval(-3,-2),4), interval(16, 81));

     // Positive interval
    TEST_HULL_EXACT_HI(pow(interval(2,3),-4), 0x1.948b0fcd6e9e0p-7, 0x1.0000000000000p-4);
    TEST_HULL_EXACT_HI(pow(interval(2,3),-3), 0x1.2f684bda12f68p-5, 0x1.0000000000000p-3);
    TEST_HULL_EXACT_HI(pow(interval(2,3),-2), 0x1.c71c71c71c71cp-4, 0x1.0000000000000p-2);
    TEST_HULL_EXACT_HI(pow(interval(2,3),-1), 0x1.5555555555555p-2, 0x1.0000000000000p-1);
    TEST_SEQ(pow(interval(2,3),0),interval::one());
    TEST_SEQ(pow(interval(2,3),1), interval(2, 3));
    TEST_SEQ(pow(interval(2,3),2), interval(4, 9));
    TEST_SEQ(pow(interval(2,3),3), interval(8, 27));
    TEST_SEQ(pow(interval(2,3),4), interval(16, 81));

    // Interval straddling 0
    TEST_HULL(pow(interval(-3,2),-4), 0x1.948b0fcd6e9e0p-7, GAOL_INFINITY);
    TEST_SEQ(pow(interval(-3,2),-3),interval::universe());
    TEST_HULL(pow(interval(-3,2),-2), 0x1.c71c71c71c71cp-4, GAOL_INFINITY);
    TEST_SEQ(pow(interval(-3,2),-1),interval::universe());
    TEST_SEQ(pow(interval(-3,2),0),interval::one());
    TEST_SEQ(pow(interval(-3,2),1), interval(-3, 2));
    TEST_SEQ(pow(interval(-3,2),2), interval(0, 9));
    TEST_SEQ(pow(interval(-3,2),3), interval(-27, 8));
    TEST_SEQ(pow(interval(-3,2),4), interval(0, 81));

    TEST_HULL(pow(interval(-2,3),-4), 0x1.948b0fcd6e9e0p-7, GAOL_INFINITY);
    TEST_SEQ(pow(interval(-2,3),-3),interval::universe());
    TEST_HULL(pow(interval(-2,3),-2), 0x1.c71c71c71c71cp-4, GAOL_INFINITY);
    TEST_SEQ(pow(interval(-2,3),-1),interval::universe());
    TEST_SEQ(pow(interval(-2,3),0),interval::one());
    TEST_SEQ(pow(interval(-2,3),1), interval(-2, 3));
    TEST_SEQ(pow(interval(-2,3),2), interval(0, 9));
    TEST_SEQ(pow(interval(-2,3),3), interval(-8, 27));
    TEST_SEQ(pow(interval(-2,3),4), interval(0, 81));

    // 15^17 and 15^18 are no doubles (a literal would be one, rounded to
    // nearest, and a power that is the double nearest would not hold the exact
    // one): the tightest enclosure of the exact power
    TEST_TRUE(gaol_tests::is_tightest_enclosure(pow(interval(-15,-15),17),gaol_tests::power(-15.0,17)));
    TEST_TRUE(gaol_tests::is_tightest_enclosure(pow(interval(15,15),17),gaol_tests::power(15.0,17)));
    TEST_TRUE(gaol_tests::is_tightest_enclosure(pow(interval(-15,-15),18),gaol_tests::power(-15.0,18)));
    TEST_TRUE(gaol_tests::is_tightest_enclosure(pow(interval(15,15),18),gaol_tests::power(15.0,18)));

    TEST_SEQ(pow(m_inf_m_max,3),m_inf_m_max);
    TEST_SEQ(pow(m_inf_m_max,4),max_inf);
    TEST_SEQ(pow(max_inf,3),max_inf);
    TEST_SEQ(pow(max_inf,4),max_inf);
	TEST_SEQ(pow(interval::universe(),0),interval::one());
  }

	void test_pow() {
		TEST_SEQ(pow(interval::universe(),interval::zero()),interval::one());
		TEST_SEQ(pow(interval(3,4),interval(2,3)), interval(9, 64));

	}


  void test_sqr() {
    TEST_EMPTY(sqr(interval::emptyset()));
    TEST_SEQ(sqr(interval::universe()),interval::positive());
    TEST_SEQ(sqr(m_inf_m_max),max_inf);
    TEST_SEQ(sqr(max_inf),max_inf);

    TEST_SEQ(sqr(interval(-3,-2)),interval(4,9));
    TEST_SEQ(sqr(interval(2,3)),interval(4,9));
    TEST_SEQ(sqr(interval(-3,2)),interval(0,9));
    TEST_SEQ(sqr(interval(-2,3)),interval(0,9));
  }

  void test_sqrt() {
    TEST_EMPTY(sqrt(interval::emptyset()));
    TEST_SEQ(sqrt(interval::universe()),interval::positive());

    TEST_EMPTY(sqrt(interval(-3,-2)));
    TEST_HULL(sqrt(interval(2,3)), 0x1.6a09e667f3bccp+0, 0x1.bb67ae8584cabp+0);
    TEST_HULL_EXACT_LO(sqrt(interval(-3,2)), 0x0.0p+0, 0x1.6a09e667f3bcdp+0);
    TEST_HULL_EXACT_LO(sqrt(interval(-2,3)), 0x0.0p+0, 0x1.bb67ae8584cabp+0);

  }

  void test_log() {
    TEST_EMPTY(log(interval(-4.0,0.0)));
    TEST_EMPTY(log(interval::zero()));
    TEST_EMPTY(log(interval::emptyset()));
	interval tmp = log(interval(-3,4));
	// ln 4 = 1.3862943611198906188..., between 0x1.62e42fefa39efp+0 and 0x1.62e42fefa39f0p+0:
	// the bound encloses it, and is within 1e-8 of it
	CPPUNIT_ASSERT(tmp.left()==-GAOL_INFINITY && tmp.right()>=0x1.62e42fefa39f0p+0
		       && (tmp.right()-1.386294361119891)<=1e-8);
	TEST_HULL(log(interval(5,9)), 0x1.9c041f7ed8d33p+0, 0x1.193ea7aad030bp+1);
	TEST_HULL(log(interval(56.1)), 0x1.01bc97d8d719ap+2, 0x1.01bc97d8d719bp+2);
  }

  void test_exp() {
    TEST_EMPTY(exp(interval::emptyset()));
    TEST_SEQ(exp(interval::zero()), interval(1));
    TEST_HULL(exp(interval::one()), 0x1.5bf0a8b145769p+1, 0x1.5bf0a8b14576ap+1);
    TEST_SEQ(exp(interval(740.0)),interval(std::numeric_limits<double>::max(),GAOL_INFINITY));
    TEST_HULL(exp(interval(-800)), 0x0.0p+0, 0x1p-1074);
    TEST_HULL(exp(interval(-10,-5)), 0x1.7cd79b5647c9ap-15, 0x1.b993fe00d5377p-8);
    TEST_HULL(exp(interval(-5,9)), 0x1.b993fe00d5376p-8, 0x1.fa7157c470f82p+12);
    TEST_HULL(exp(interval(9,11)), 0x1.fa7157c470f81p+12, 0x1.d3c4488ee4f80p+15);
    TEST_HULL(exp(interval(-3.5)), 0x1.eec1018e4ff65p-6, 0x1.eec1018e4ff66p-6);
    TEST_HULL(exp(interval(3.5)), 0x1.08ec721396bdbp+5, 0x1.08ec721396bdcp+5);
	CPPUNIT_ASSERT(exp(m_inf_m_max).certainly_positive());
  }

	void test_nth_root() {
		TEST_EMPTY(nth_root(interval::universe(),0));
		TEST_SEQ(nth_root(interval(4,5),1),interval(4,5));
		TEST_HULL_EXACT_LO(nth_root(interval(4,5),2), 0x1.0000000000000p+1, 0x1.1e3779b97f4a8p+1);
		TEST_EMPTY(nth_root(interval::emptyset(),2));
		TEST_EMPTY(nth_root(interval::emptyset(),5));
		TEST_EMPTY(nth_root(interval::emptyset(),4));
		TEST_HULL_EXACT_LO(nth_root(interval(-4,3),4), 0x0.0p+0, 0x1.50ea39fcbf167p+0);
		TEST_HULL(nth_root(interval(-4,3),5), -0x1.51cb453b9536dp+0, 0x1.3ee8390d43956p+0);
		TEST_HULL(nth_root(interval(0.5,2),4), 0x1.ae89f995ad3adp-1, 0x1.306fe0a31b716p+0);
		TEST_HULL(nth_root(interval(0.2,0.5),4), 0x1.56652116c816cp-1, 0x1.ae89f995ad3aep-1);
		TEST_HULL(nth_root(interval(0.5,2),5), 0x1.bdb8cdadbe120p-1, 0x1.2611186bae675p+0);
		TEST_HULL(nth_root(interval(0.2,0.5),5), 0x1.731651f09a68ep-1, 0x1.bdb8cdadbe121p-1);
		TEST_HULL(nth_root(interval(0.1),4), 0x1.1feb33c1c381ep-1, 0x1.1feb33c1c381fp-1);
		TEST_HULL(nth_root(interval(0.1),5), 0x1.430cd74f6d478p-1, 0x1.430cd74f6d479p-1);
		TEST_HULL(nth_root(interval(2.1),4), 0x1.342c5329399b5p+0, 0x1.342c5329399b6p+0);
		TEST_HULL(nth_root(interval(2.1),5), 0x1.28f3495d8362fp+0, 0x1.28f3495d83630p+0);

	}
  // <-- End of tests
};

GAOL_UNIT_MAIN(non_arithmetic_test, "non_arithmetic",
               GAOL_UNIT_TEST(test_pow_int),
               GAOL_UNIT_TEST(test_pow),
               GAOL_UNIT_TEST(test_sqr),
               GAOL_UNIT_TEST(test_sqrt),
               GAOL_UNIT_TEST(test_log),
               GAOL_UNIT_TEST(test_exp),
               GAOL_UNIT_TEST(test_nth_root))
