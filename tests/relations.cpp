// GAOL 4's unit test check/relations.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"

#include <algorithm>
#include <climits>
#include <set>
#include <vector>

class relations_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests

  // Tests of logical predicates ===================================================================
  void test_set() {
    TEST_TRUE(interval(4,8).set_contains(interval(4,4)));
    TEST_TRUE(interval(-6,GAOL_INFINITY).set_contains(4.5));
    TEST_TRUE(interval(-6,GAOL_INFINITY).set_contains(GAOL_INFINITY));
    TEST_TRUE(interval::universe().set_contains(interval::emptyset()));
    TEST_TRUE(interval::emptyset().set_contains(interval::emptyset()));
    TEST_TRUE(!interval(4,8).set_strictly_contains(interval(4,4)));
    TEST_TRUE(interval::emptyset().set_strictly_contains(interval::emptyset()));
    TEST_TRUE(interval(3,4).set_disjoint(interval(5,8)));
    TEST_TRUE(interval(-4,-3).set_disjoint(interval(5,8)));
    TEST_TRUE(interval::emptyset().set_disjoint(interval::emptyset()));
    TEST_TRUE(interval::emptyset().set_disjoint(interval(3,4)));
    TEST_TRUE(interval(-3,5).set_disjoint(interval::emptyset()));
    TEST_TRUE(interval::universe().set_eq(interval::universe()));
    TEST_TRUE(interval::emptyset().set_eq(interval::emptyset()));
    TEST_TRUE(interval::emptyset().set_neq(interval::universe()));
    TEST_TRUE(interval::universe().set_neq(interval::emptyset()));
    TEST_TRUE(interval(4,5).set_le(interval(-3,12)));
    TEST_TRUE(interval::emptyset().set_le(interval(3.0)));
    // interior(Empty, Empty) is true in IEEE 1788-2015 (Table 10.4)
    TEST_TRUE(interval::emptyset().set_le(interval::emptyset()));
    TEST_FALSE(interval(-6,4).set_le(interval(3,7)));
    TEST_TRUE(interval(4.5,6).set_leq(interval(4.5,6)));
    TEST_FALSE(interval(3.5,9).set_leq(interval(2,6)));
    TEST_TRUE(interval::emptyset().set_leq(interval::emptyset()));
}
    void test_certainly() {
      TEST_TRUE(interval(4,5).certainly_le(interval(6,9)));
      TEST_FALSE(interval(4,5).certainly_le(interval(5,9)));
      TEST_TRUE(interval::emptyset().certainly_le(interval(4,6)));
      TEST_TRUE(interval::emptyset().certainly_le(interval::emptyset()));

      TEST_TRUE(interval(4,5).certainly_leq(interval(6,9)));
      TEST_TRUE(interval(4,5).certainly_leq(interval(5,9)));
      TEST_FALSE(interval(5,9).certainly_leq(interval(4,5)));
      TEST_FALSE(interval(4,8).certainly_leq(interval(5,9)));
      TEST_TRUE(interval::emptyset().certainly_leq(interval(4,6)));
      // precedes(a, Empty) is true in IEEE 1788-2015 (Table 10.4)
      TEST_TRUE(interval(4,6).certainly_leq(interval::emptyset()));
      TEST_TRUE(interval(4,6).certainly_le(interval::emptyset()));

      TEST_TRUE(interval(8,10).certainly_geq(interval(4,8)));
      TEST_TRUE(interval(9,10).certainly_geq(interval(4,8)));
      TEST_TRUE(interval::emptyset().certainly_geq(interval::emptyset()));
      TEST_TRUE(interval::emptyset().certainly_geq(interval(3,5)));
      TEST_TRUE(interval(4,8).certainly_geq(interval::emptyset()));

      TEST_FALSE(interval(8,10).certainly_ge(interval(4,8)));
      TEST_TRUE(interval(9,10).certainly_ge(interval(4,8)));
      TEST_TRUE(interval::emptyset().certainly_ge(interval::emptyset()));
      TEST_TRUE(interval::emptyset().certainly_ge(interval(3,5)));
      TEST_TRUE(interval(4,8).certainly_ge(interval::emptyset()));

      TEST_TRUE(interval::emptyset().certainly_positive());
      TEST_FALSE(interval::universe().certainly_positive());
      TEST_TRUE(interval(0,5).certainly_positive());
      TEST_FALSE(interval(-5,0).certainly_positive());
      TEST_TRUE(interval::zero().certainly_positive());
      TEST_TRUE(interval(-0.0,0.0).certainly_positive());

      TEST_TRUE(interval::emptyset().certainly_strictly_positive());
      TEST_FALSE(interval::universe().certainly_strictly_positive());
      TEST_FALSE(interval(0,5).certainly_strictly_positive());
      TEST_FALSE(interval(-5,0).certainly_strictly_positive());
      TEST_FALSE(interval::zero().certainly_strictly_positive());
      TEST_FALSE(interval(-0.0,0.0).certainly_strictly_positive());
      TEST_TRUE(interval(4,5).certainly_strictly_positive());

      TEST_TRUE(interval::emptyset().certainly_negative());
      TEST_FALSE(interval::universe().certainly_negative());
      TEST_TRUE(interval(-5,-3).certainly_negative());
      TEST_TRUE(interval(-3,+0.0).certainly_negative());
      TEST_FALSE(interval(-4,5).certainly_negative());

      TEST_TRUE(interval::emptyset().certainly_strictly_negative());
      TEST_FALSE(interval::universe().certainly_strictly_negative());
      TEST_TRUE(interval(-5,-3).certainly_strictly_negative());
      TEST_FALSE(interval(-3,+0.0).certainly_strictly_negative());
      TEST_FALSE(interval(-4,5).certainly_strictly_negative());
 }

  // <, <=, > and >= between an interval and a double: the relations with
  // interval(d), interval(double) being explicit (GAOL v5)
  void test_symbols_with_double() {
    TEST_TRUE(interval(1,2) < 3.0);
    TEST_FALSE(interval(1,3) < 3.0);
    TEST_TRUE(interval(1,3) <= 3.0);
    TEST_TRUE(0.0 < interval(1,2));
    TEST_TRUE(2.0 <= interval(2,3));
    TEST_FALSE(2.5 <= interval(2,3));
    TEST_TRUE(interval(4,5) > 3);
    TEST_TRUE(3 > interval(1,2));
    TEST_TRUE(interval(2,3) >= 2.0);
    TEST_FALSE(interval(1,3) >= 2.0);
    TEST_TRUE(3.0 >= interval(1,3));
    // No point of the empty set contradicts them
    TEST_TRUE(interval::emptyset() < 0.0 && interval::emptyset() > 0.0);

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const interval xs[] = { interval(1,2), interval(-1,1), interval(0,GAOL_INFINITY),
                            interval::universe(), interval::emptyset() };
    const double ds[] = { -1.0, 0.0, 1.0, 2.0, GAOL_INFINITY, -GAOL_INFINITY, nan };
    for (const interval& x : xs) {
      for (double d : ds) {
        const interval i(d);
        TEST_TRUE((x < d) == (x < i) && (d < x) == (i < x));
        TEST_TRUE((x <= d) == (x <= i) && (d <= x) == (i <= x));
        TEST_TRUE((x > d) == (x > i) && (d > x) == (i > x));
        TEST_TRUE((x >= d) == (x >= i) && (d >= x) == (i >= x));
      }
    }
  }

  void test_misc() {
    // straddles_zero()
    TEST_TRUE(interval(-4,5).straddles_zero());
    TEST_TRUE(!interval(-4,-3).straddles_zero());
    TEST_TRUE(!interval(3,5).straddles_zero());
    TEST_TRUE(interval::universe().straddles_zero());
    TEST_TRUE(!interval::emptyset().straddles_zero());
    TEST_TRUE(interval(-3,0).straddles_zero());
    TEST_TRUE(interval(0,3).straddles_zero());


    // strictly_straddles_zero()
    TEST_TRUE(interval(-4,5).strictly_straddles_zero());
    TEST_TRUE(!interval(-4,-3).strictly_straddles_zero());
    TEST_TRUE(!interval(3,5).strictly_straddles_zero());
    TEST_TRUE(interval::universe().strictly_straddles_zero());
    TEST_TRUE(!interval::emptyset().strictly_straddles_zero());
    TEST_TRUE(!interval(-3,0).strictly_straddles_zero());
    TEST_TRUE(!interval(0,3).strictly_straddles_zero());

    // is_a_double()
    TEST_TRUE(interval(3.0,3.0).is_a_double());
    TEST_TRUE(!interval(3.0,4.0).is_a_double());
    TEST_TRUE(!interval::emptyset().is_a_double());

    // is_an_int()
    TEST_TRUE(interval(3.0,3.0).is_an_int());
    TEST_TRUE(!interval(3.5,3.5).is_an_int());
    TEST_TRUE(!interval::emptyset().is_an_int());
    TEST_TRUE(!interval(1.0e100,1.0e100).is_an_int());

    // is_canonical()
    TEST_TRUE(!interval::emptyset().is_canonical());
    TEST_TRUE(textToInterval("1/10.0").is_canonical());
	TEST_TRUE(interval(3,next_float(3)).is_canonical());
	TEST_TRUE(interval(previous_float(-3),-3).is_canonical());
    // is_empty()
    TEST_TRUE(interval::emptyset().is_empty());
    TEST_FALSE(interval::universe().is_empty());
    TEST_FALSE(textToInterval("1.0/10").is_empty());
    TEST_FALSE(interval(4,5).is_empty());
    TEST_TRUE(interval(5,4).is_empty());
	TEST_FALSE(interval(-4,5).is_empty());
	TEST_TRUE(interval(std::numeric_limits<double>::quiet_NaN(),4).is_empty());
	TEST_TRUE(interval(-6,std::numeric_limits<double>::quiet_NaN()).is_empty());
	TEST_TRUE(interval(-std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::quiet_NaN()).is_empty());


    // is_zero()
    TEST_TRUE(interval::zero().is_zero());
    TEST_TRUE(interval(0.0,0.0).is_zero());
    TEST_TRUE(interval(-0.0,+0.0).is_zero());
    TEST_FALSE(interval(0,5).is_zero());
    TEST_FALSE(interval::emptyset().is_zero());

    // is_symmetric()
    TEST_FALSE(interval::emptyset().is_symmetric());
    TEST_TRUE(interval::universe().is_symmetric());
    TEST_TRUE(interval::zero().is_symmetric());
    TEST_TRUE(interval(-5,5).is_symmetric());
    TEST_FALSE(interval(-5,4).is_symmetric());

    // is_finite()
    TEST_TRUE(interval(4,5).is_finite());
    TEST_FALSE(interval(3,GAOL_INFINITY).is_finite());
    TEST_FALSE(interval::universe().is_finite());
    TEST_TRUE(interval::emptyset().is_finite());
  }

  /* <, <=, > and >= between an interval and an integer, set_contains() and
     set_strictly_contains() of an integer (GAOL v5, point D.21): n compared
     as the integer it is. 2^53 + 1, converted to the double 2^53, was not
     above [2^53], and [2^53] contained it. */
  void test_symbols_with_integers() {
    const double two53 = std::ldexp(1.0, 53), two64 = std::ldexp(1.0, 64);
    const volatile long long n = 9007199254740993LL; // 2^53 + 1
    const interval below(two53), above(two53 + 2), around(two53, two53 + 2);
    TEST_TRUE(below < n && below <= n && n > below && n >= below);
    TEST_FALSE(below > n || below >= n || n < below || n <= below);
    TEST_TRUE(above > n && above >= n && n < above && n <= above);
    TEST_FALSE(above < n || above <= n || n > above || n >= above);
    TEST_FALSE(around < n || around <= n || around > n || around >= n);
    TEST_FALSE(n < around || n <= around || n > around || n >= around);
    TEST_TRUE(around.set_contains(n) && around.set_strictly_contains(n));
    TEST_FALSE(below.set_contains(n) || above.set_contains(n) || below.set_strictly_contains(n));
    TEST_FALSE(around.set_strictly_contains(9007199254740994LL));
    TEST_TRUE(around.set_contains(9007199254740994LL));
    const volatile unsigned long long m = ULLONG_MAX; // 2^64 - 1
    TEST_TRUE(interval(two64) > m && m < interval(two64) && interval(two64 - 2048) < m);
    TEST_FALSE(interval(two64).set_contains(m) || interval(two64).set_strictly_contains(m));
    TEST_TRUE(interval(two64 - 2048, two64).set_strictly_contains(m));
    TEST_FALSE(interval(two64 - 2048, two64 - 2048).set_strictly_contains(m));
    // No point of the empty set contradicts them
    TEST_TRUE(interval::emptyset() < n && interval::emptyset() > n && n <= interval::emptyset());
    TEST_FALSE(interval::emptyset().set_contains(n));

    // An integer that is a double: the relations with that double
    const interval xs[] = { interval(1,2), interval(-1,1), interval(0,GAOL_INFINITY),
                            interval::universe(), interval::emptyset() };
    for (const interval& x : xs) {
      for (int k = -2; k <= 3; ++k) {
        const double d = k;
        const long long ll = k;
        TEST_TRUE((x < k) == (x < d) && (k < x) == (d < x) && (x < ll) == (x < d) && (ll < x) == (d < x));
        TEST_TRUE((x <= k) == (x <= d) && (k <= x) == (d <= x) && (x <= ll) == (x <= d) && (ll <= x) == (d <= x));
        TEST_TRUE((x > k) == (x > d) && (k > x) == (d > x) && (x > ll) == (x > d) && (ll > x) == (d > x));
        TEST_TRUE((x >= k) == (x >= d) && (k >= x) == (d >= x) && (x >= ll) == (x >= d) && (ll >= x) == (d >= x));
        TEST_TRUE(x.set_contains(k) == x.set_contains(d) && x.set_strictly_contains(ll) == x.set_strictly_contains(d));
      }
    }
  }

  /* lexicographic_less (GAOL v5, point V): a total order for the containers
     of the standard library. < is strictPrecedes of IEEE 1788-2015, true as
     soon as either interval is empty: a std::set<interval> built on < loses
     the intervals that overlap, and std::sort reads past the end of a vector
     that holds an empty interval, which the sort with lexicographic_less()
     checks here, an empty interval sorted to its place, at the front of
     the vector. */
  void test_lexicographic_less() {
    // Strict: no interval is less than itself
    const interval xs[] = { interval::emptyset(), interval(-GAOL_INFINITY, 1),
                            interval(-GAOL_INFINITY, 2), interval::universe(),
                            interval(-1, 1), interval(0, 1), interval(0, 2),
                            interval(1, 1), interval(1, 2),
                            interval(1, GAOL_INFINITY) };
    gaol::lexicographic_less const less;
    gaol_core::lexicographic_less const less_core;
    for (const interval& x : xs) {
      TEST_FALSE(less(x, x));
      TEST_FALSE(less_core(x, x));
    }
    for (size_t i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
      for (size_t j = 0; j < sizeof(xs)/sizeof(xs[0]); ++j) {
        // The expected order of the list above, and a strict order: for two
        // different intervals exactly one of less(a, b) and less(b, a),
        // for two equal intervals neither
        TEST_TRUE(less(xs[i], xs[j]) == (i < j));
        TEST_TRUE(less(xs[i], xs[j]) != less(xs[j], xs[i]) || i == j);
        TEST_TRUE(!(less(xs[i], xs[j]) && less(xs[j], xs[i])));
      }
    }
    // The empty set comes first
    TEST_TRUE(less(interval::emptyset(), interval::universe()));
    TEST_FALSE(less(interval::emptyset(), interval::emptyset()));
    TEST_FALSE(less(interval::universe(), interval::emptyset()));
    // -0.0 and +0.0 are equal as bounds
    TEST_FALSE(less(interval(-0.0, 0.0), interval(0.0, 0.0)));
    TEST_FALSE(less(interval(0.0, 0.0), interval(-0.0, 0.0)));

    // A std::set keeps the intervals that overlap and the empty set
    std::set<interval, gaol::lexicographic_less> s;
    s.insert(interval(0, 1));
    s.insert(interval(0.5, 3));   // overlaps the first: < loses it
    s.insert(interval(1, 2));    // overlaps both
    s.insert(interval::emptyset());
    TEST_TRUE(s.size() == 4);
    TEST_TRUE(s.count(interval(0.5, 3)) == 1);
    TEST_TRUE(s.begin()->is_empty());
    // std::sort of a vector holding an empty interval, which reads past the
    // end of the vector with < (certainly_le)
    std::vector<interval> v;
    v.push_back(interval(1, 2));
    v.push_back(interval::emptyset());
    v.push_back(interval(-3, 0));
    std::sort(v.begin(), v.end(), less);
    TEST_TRUE(v.size() == 3);
    TEST_TRUE(v[0].is_empty());
    TEST_TRUE(less(v[1], v[2]));
    TEST_TRUE(v[1].set_eq(interval(-3, 0)) && v[2].set_eq(interval(1, 2)));
  }

  // <-- End of tests
};


GAOL_UNIT_MAIN(relations_test, "relations",
               GAOL_UNIT_TEST(test_set),
               GAOL_UNIT_TEST(test_certainly),
               GAOL_UNIT_TEST(test_symbols_with_double),
               GAOL_UNIT_TEST(test_symbols_with_integers),
               GAOL_UNIT_TEST(test_lexicographic_less),
               GAOL_UNIT_TEST(test_misc))
