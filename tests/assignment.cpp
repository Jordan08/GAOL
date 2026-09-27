// GAOL 4's unit test check/assignment.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"

class assignment_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests

  void test_assignment() {
      interval x(-5,10);
      interval y;

      y = x;
      TEST_SEQ(x,y);
  }

  void test_arithmetic_assignment() {
      interval x(-5,4);
      interval y(2,3);

      x += y;
      TEST_SEQ(x,interval(-3,7));
      x -= y;
      TEST_SEQ(x,interval(-6,5));
      x *= y;
      TEST_SEQ(x,interval(-18,15));
      x /= y;
      TEST_EQ(x,interval(-9,7.5));
      x = interval(-12,12);
      x /= interval(-6,23);
      TEST_SEQ(x,interval::universe());
      x = interval(-12,12);
      x /= interval::zero();
      TEST_EMPTY(x);
      x = interval::universe();
      x /= interval::zero();
      TEST_EMPTY(x);
      x = interval(-12,12);
      x %= interval(-6,23);
      TEST_SEQ(x,interval::universe());
      x = interval(-12,12);
      x %= interval::zero();
      TEST_SEQ(x,interval::universe());
      x = interval::universe();
      x %= interval::zero();
      TEST_SEQ(x,interval::universe());

  }

  void test_logic_assignment() {
      interval x(-5,4);

      x |= interval(-2,8);
      TEST_SEQ(x,interval(-5,8));
      x |= interval(10,12);
      TEST_SEQ(x,interval(-5,12));
      x |= interval(-13,-8);
      TEST_SEQ(x,interval(-13,12));
      x &= interval(-15,-1);
      TEST_SEQ(x,interval(-13,-1));
      x = interval(-5,6);
      x |= interval::emptyset();
      TEST_SEQ(x,interval(-5,6));
      x &= interval::emptyset();
      TEST_EMPTY(x);
      x = interval(-5,6);
      x &= interval::universe();
      TEST_SEQ(x,interval(-5,6));
      x |= interval::universe();
      TEST_SEQ(x,interval::universe());
  }

  // <-- End of tests
};

GAOL_UNIT_MAIN(assignment_test, "assignment",
               GAOL_UNIT_TEST(test_assignment),
               GAOL_UNIT_TEST(test_arithmetic_assignment),
               GAOL_UNIT_TEST(test_logic_assignment))
