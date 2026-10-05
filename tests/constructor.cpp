// GAOL 4's unit test check/constructor.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"

#include <cfenv>
#include <climits>
#include <type_traits>

#define TEST_INOUT_EQ(Istr,Ires)  \
  try {                           \
       interval I = textToInterval(Istr); \
       TEST_EQ(I,Ires);           \
  } catch (const input_format_error&) {  \
    CPPUNIT_FAIL(std::string("Wrong format: ")+std::string(Istr)); \
  }

#define TEST_INOUT_SEQ(Istr,Ires) \
  try {                           \
       interval I = textToInterval(Istr); \
       TEST_SEQ(I,Ires);          \
  } catch (const input_format_error&) {  \
    CPPUNIT_FAIL(std::string("Wrong format: ")+std::string(Istr)); \
  }


class constructor_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests
  void test_constructor_numbers() {
      interval a;
      TEST_SEQ(a,interval::universe());
      interval b(-4,6);
      CPPUNIT_ASSERT(b.left()==-4 && b.right() == 6);
      interval c(6.5);
      CPPUNIT_ASSERT(c.left() == c.right() && c.left() == 6.5);
      interval d(b);
      CPPUNIT_ASSERT(d.left()==-4 && d.right() == 6);
  }

  /* Integers (GAOL v5, point D.21): interval(n) is the tightest interval
     containing n, the two doubles around it where n is no double, whatever
     the rounding direction, and interval(a, b) with an integer bound contains
     it. Converted to a double, 2^53 + 1 was [2^53, 2^53] and 2^64 - 1
     [2^64, 2^64], which do not contain them. The integers are opaque, so that
     they are converted at run time, in each rounding direction. */
  static_assert(!std::is_convertible<long long, interval>::value && !std::is_convertible<int, interval>::value,
                "an integer does not convert implicitly to an interval");
  static_assert(std::is_constructible<interval, long long>::value
                && std::is_constructible<interval, long long, double>::value
                && std::is_constructible<interval, double, unsigned long long>::value,
                "interval(n), interval(n, d) and interval(d, n)");

  void test_constructor_integers() {
    const double two53 = std::ldexp(1.0, 53), two63 = std::ldexp(1.0, 63), two64 = std::ldexp(1.0, 64);
    const volatile long long n = 9007199254740993LL, n2 = 9007199254740994LL; // 2^53 + 1, 2^53 + 2
    const volatile long long llmax = LLONG_MAX, llmin = LLONG_MIN;
    const volatile unsigned long long ullmax = ULLONG_MAX, ull63 = 9223372036854775809ULL; // 2^64 - 1, 2^63 + 1
    const int directions[] = { FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
    const int saved = std::fegetround();
    for (int direction : directions) {
      CPPUNIT_ASSERT(std::fesetround(direction) == 0);
      const interval a(n), b(-n), c(n2), d(llmax), e(llmin), f(ullmax), g(ull63);
      const interval h(n, n), i(-n, n), j(1.5, n), k(n, 1e300), l(n, 1), m(0, 0.5);
      std::fesetround(saved);
      TEST_TRUE(a.left() == two53 && a.right() == two53 + 2);
      TEST_TRUE(b.left() == -(two53 + 2) && b.right() == -two53);
      TEST_TRUE(c.left() == two53 + 2 && c.right() == two53 + 2);
      TEST_TRUE(d.left() == two63 - 1024 && d.right() == two63);
      TEST_TRUE(e.left() == -two63 && e.right() == -two63);
      TEST_TRUE(f.left() == two64 - 2048 && f.right() == two64);
      TEST_TRUE(g.left() == two63 && g.right() == two63 + 2048);
      TEST_TRUE(h.left() == two53 && h.right() == two53 + 2);
      TEST_TRUE(i.left() == -(two53 + 2) && i.right() == two53 + 2);
      TEST_TRUE(j.left() == 1.5 && j.right() == two53 + 2);
      TEST_TRUE(k.left() == two53 && k.right() == 1e300);
      TEST_EMPTY(l);
      TEST_TRUE(m.left() == 0 && m.right() == 0.5);
    }
    // Bounds in the wrong order, compared as the numbers they are: empty,
    // though both may lie between the same two doubles, where the double below
    // a and the double above b are in order
    const long long n1 = n - 1, n3 = n + 2; // 2^53, 2^53 + 3
    TEST_EMPTY(interval(n, n1));
    TEST_EMPTY(interval(n3, n));
    TEST_EMPTY(interval(n, two53));
    TEST_EMPTY(interval(two53 + 2, n));
    TEST_EMPTY(interval(ullmax, two64 - 2048));
    TEST_EMPTY(interval(1u, -1));
    TEST_EMPTY(interval(n, std::numeric_limits<double>::quiet_NaN()));
    TEST_TRUE(interval(two53, n).left() == two53 && interval(two53, n).right() == two53 + 2);
    TEST_TRUE(interval(1.0f, n).left() == 1 && interval(1.0f, n).right() == two53 + 2);
    TEST_TRUE(interval(-1LL, ullmax).left() == -1 && interval(-1LL, ullmax).right() == two64);
    TEST_SEQ(interval(-1, 0u), interval(-1.0, 0.0));
    // Every integer type but bool, as the double it is for the small ones
    TEST_SEQ(interval(static_cast<short>(-5)), interval(-5.0));
    TEST_SEQ(interval('A'), interval(65.0));
    TEST_SEQ(interval(static_cast<signed char>(-3)), interval(-3.0));
    TEST_SEQ(interval(5u), interval(5.0));
    TEST_SEQ(interval(5L), interval(5.0));
    TEST_SEQ(interval(static_cast<std::uint64_t>(5)), interval(5.0));
    TEST_SEQ(interval(1, 2), interval(1.0, 2.0));
    TEST_SEQ(interval(-4, 6u), interval(-4.0, 6.0));
  }

  // textToInterval() in place of interval(const char*) of GAOL 4 (GAOL v5)
  void test_constructor_string() {
    interval a = textToInterval("empty"), b = textToInterval("[empty]");
		interval c = textToInterval("5.80258497501207e-14");
		CPPUNIT_ASSERT(!c.is_empty());
    TEST_EMPTY(a);
    TEST_EMPTY(b);
    TEST_INOUT_EQ("[3,4]",interval(3,4));
    TEST_INOUT_EQ("4.5",interval(4.5,4.5));
    TEST_INOUT_EQ("1.0/10",interval(1.0)/interval(10.0));
    TEST_INOUT_EQ("1.0/10.0",textToInterval("[0.1,0.1]","[0.1,0.1]"));
}
  // <-- End of tests
};


GAOL_UNIT_MAIN(constructor_test, "constructor",
               GAOL_UNIT_TEST(test_constructor_string),
               GAOL_UNIT_TEST(test_constructor_numbers),
               GAOL_UNIT_TEST(test_constructor_integers))
