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

  // =, &= and |= with a double: interval(d), empty for an infinite d or a
  // NaN, interval(double) being explicit (GAOL v5)
  void test_double_assignment() {
      interval x(-5,4);

      x = 1234.5;
      TEST_SEQ(x,interval(1234.5,1234.5));
      x = 0;
      TEST_SEQ(x,interval::zero());
      x = GAOL_INFINITY;
      TEST_EMPTY(x);
      x = interval(-5,4);
      x = std::numeric_limits<double>::quiet_NaN();
      TEST_EMPTY(x);

      x = interval(-5,4);
      x |= 7.0;
      TEST_SEQ(x,interval(-5,7));
      x |= -GAOL_INFINITY;
      TEST_SEQ(x,interval(-5,7));
      x &= 2.0;
      TEST_SEQ(x,interval(2,2));
      x &= 3.0;
      TEST_EMPTY(x);
      x |= 3.0;
      TEST_SEQ(x,interval(3,3));
      x &= GAOL_INFINITY;
      TEST_EMPTY(x);
  }

  // =, &= and |= with an integer: interval(n), the tightest interval
  // containing it, the two doubles around 2^53 + 1, which the double 2^53
  // did not contain (GAOL v5, point D.21)
  void test_integer_assignment() {
      const double two53 = std::ldexp(1.0, 53);
      const volatile long long n = 9007199254740993LL; // 2^53 + 1
      interval x(-5,4);

      x = n;
      TEST_TRUE(x.left() == two53 && x.right() == two53 + 2);
      x = 3;
      TEST_SEQ(x,interval(3.0));
      x = 7u;
      TEST_SEQ(x,interval(7.0));
      x = interval(0.0, two53);
      x |= n;
      TEST_TRUE(x.left() == 0 && x.right() == two53 + 2);
      x &= n;
      TEST_TRUE(x.left() == two53 && x.right() == two53 + 2);
      x = interval(0.0, two53);
      x &= n;
      TEST_TRUE(x.left() == two53 && x.right() == two53);
      x &= -1;
      TEST_EMPTY(x);
  }

  // <-- End of tests
};

GAOL_UNIT_MAIN(assignment_test, "assignment",
               GAOL_UNIT_TEST(test_assignment),
               GAOL_UNIT_TEST(test_arithmetic_assignment),
               GAOL_UNIT_TEST(test_logic_assignment),
               GAOL_UNIT_TEST(test_double_assignment),
               GAOL_UNIT_TEST(test_integer_assignment))
