// GAOL 4's unit test check/misc.cpp, run with the checks of unit_tests.h
// rather than CppUnit (GAOL v5)
#include "unit_tests.h"

// A namespace detail of the program's own, with using namespace gaol (in
// unit_tests.h): GAOL's helpers were in gaol_core::detail, which the namespace
// gaol took whole, and detail::of_the_program() did not compile, "reference
// to 'detail' is ambiguous" (GAOL v5, before its release). They are in
// gaol_detail, a namespace of its own
namespace detail {
  inline int of_the_program() { return 1; }
}

class misc_test {
public:
  void setUp() {
  }
  void tearDown() {
  }

  // --> Beginning of tests

  void test_namespace_detail_of_the_program() {
    TEST_TRUE(detail::of_the_program() == 1);
  }

  // Tests of logical predicates ===================================================================
  void test_predicates() {
    // is_empty()
    CPPUNIT_ASSERT(interval::emptyset().is_empty());
    CPPUNIT_ASSERT(interval(1.0,-1.0).is_empty());

    // is_symmetric()
    CPPUNIT_ASSERT(!interval::emptyset().is_symmetric());
    CPPUNIT_ASSERT(interval::universe().is_symmetric());
    CPPUNIT_ASSERT(interval(-6.0,6.0).is_symmetric());
    CPPUNIT_ASSERT(!interval(-6.0,-5.0).is_symmetric());
    CPPUNIT_ASSERT(!interval(5.0,6.0).is_symmetric());

  }

  // Tests of constants
  void test_constants() {
    CPPUNIT_ASSERT(interval::emptyset().is_empty());
    TEST_PEQ(interval::universe(),interval(-GAOL_INFINITY,+GAOL_INFINITY));
    TEST_SEQ(interval::zero(),interval(0.0,0.0));
    TEST_SEQ(interval::one(),interval(1.0,1.0));
    TEST_SEQ(interval::positive(),interval(0.0,+GAOL_INFINITY));
    TEST_SEQ(interval::negative(),interval(-GAOL_INFINITY,0.0));
    TEST_SEQ(interval::minus_one_plus_one(),interval(-1.0,1.0));
    TEST_PEQ(interval::pi(),interval(3.14159,3.1416));
    TEST_PEQ(interval::two_pi(),interval(6.2831,6.2832));
    TEST_PEQ(interval::half_pi(),interval(1.5707,1.5708));
    TEST_SEQ(interval::one_plus_infinity(),interval(1.0,+GAOL_INFINITY));
  }


  // <-- End of tests
};

GAOL_UNIT_MAIN(misc_test, "misc",
               GAOL_UNIT_TEST(test_constants),
               GAOL_UNIT_TEST(test_predicates),
               GAOL_UNIT_TEST(test_namespace_detail_of_the_program))
