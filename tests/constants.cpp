// GAOL 4's unit test check/constants.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"

class constants_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests

  // Tests of logical predicates ===================================================================
  void test_interval_const() {
    TEST_EQ(interval::two_pi(),interval(6.283185307179586, 6.283185307179587));
    TEST_EQ(interval::pi(),interval(3.141592653589793, 3.141592653589794));
    TEST_EQ(interval::half_pi(),interval(1.570796326794897, 1.570796326794897));
    TEST_SEQ(interval::positive(),interval(0,GAOL_INFINITY));
    TEST_SEQ(interval::negative(),interval(-GAOL_INFINITY,0));
    TEST_SEQ(interval::minus_one_plus_one(),interval(-1.0,1.0));
    TEST_SEQ(interval::one_plus_infinity(),interval(1.0,GAOL_INFINITY));
    TEST_SEQ(interval::universe(),interval(-GAOL_INFINITY,+GAOL_INFINITY));
    TEST_SEQ(interval::zero(),interval(0.0,0.0));
    TEST_SEQ(interval::one(),interval(1.0,1.0));
    TEST_EMPTY(interval::emptyset());
  }

  void test_floating_point_const() {
      // The bounds of the constant intervals, which GAOL 4 declared as doubles
      // too, for the code using it (pi_dn, pi_up, two_pi, ln2_dn...), and
      // GAOL v5 no longer does
      TEST_SEQ(interval::pi(),interval(0x1.921fb54442d18p+1,0x1.921fb54442d19p+1));
      TEST_SEQ(interval::half_pi(),interval(0x1.921fb54442d18p+0,0x1.921fb54442d19p+0));
      TEST_SEQ(interval::two_pi(),interval(0x1.921fb54442d18p+2,0x1.921fb54442d19p+2));
      TEST_EQ(interval(0x1.62e42fefa39efp-1,0x1.62e42fefa39f0p-1),log(interval(2)));
      TEST_SEQ(pow(interval(2),52),interval(0x1p+52));
      TEST_SEQ(pow(interval(2),51),interval(0x1p+51));
      CPPUNIT_ASSERT(std::isnan(GAOL_NAN));
      CPPUNIT_ASSERT(std::isinf(GAOL_INFINITY));
      CPPUNIT_ASSERT(GAOL_INFINITY > 0);
  }
  // <-- End of tests
};


GAOL_UNIT_MAIN(constants_test, "constants",
               GAOL_UNIT_TEST(test_interval_const),
               GAOL_UNIT_TEST(test_floating_point_const))
