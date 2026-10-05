/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: the unit tests of GAOL 4.
 *
 * The unit tests Frederic Goualard wrote for GAOL 4 with CppUnit (check/,
 * check/tests.h) run with the checks of gaol_tests.h, and no longer need
 * CppUnit: each suite is a class whose tests are functions, as before, and
 * GAOL_UNIT_MAIN() writes the main() that runs them, calling setUp() and
 * tearDown() around each. A macro TEST_... counts a check named after the
 * test it is in, and describes a failure by its line, its expression and
 * the values compared; a failure no longer ends the test, as CPPUNIT_ASSERT
 * did, so that every failure of a run is reported. gaol_tests::summary()
 * gives the exit status, 0 when every check passed.
 *
 * Copyright (c) 2001-2006 Laboratoire d'Informatique de Nantes-Atlantique
 * Copyright (c) 2026 ENSTA, France
 *
 * check/tests.h: Frederic Goualard, 2006-03-16
 * Modified 2026-09-27 by Jordan NININ: CppUnit replaced by gaol_tests.h
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_UNIT_TESTS_H
#define GAOL_UNIT_TESTS_H

#include <cmath>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <utility>

#include "gaol_tests.h"

using namespace gaol;

namespace gaol_unit
{
  // The name of the test running, "suite: test", which names its checks
  inline std::string& current()
  {
    static std::string name;
    return name;
  }

  inline std::string where(int line, const std::string& expression)
  {
    return "line " + std::to_string(line) + ", " + expression;
  }

  template<class T>
  std::string show(const T& x)
  {
    std::ostringstream s;
    s.precision(17);
    s << x;
    return s.str();
  }

  inline std::string show(const gaol::interval& x)
  {
    return gaol_tests::hex(x);
  }

  inline bool check(bool ok, int line, const std::string& expression)
  {
    return gaol_tests::check(current(), ok, [&] { return where(line, expression); });
  }

  template<class A, class B>
  bool check(bool ok, int line, const std::string& expression, const A& a, const B& b)
  {
    return gaol_tests::check(current(), ok, [&] { return where(line, expression) + ": " + show(a) + " and " + show(b); });
  }

  // CPPUNIT_FAIL(message): a check that fails
  inline void fail(const std::string& message, int line)
  {
    gaol_tests::check(current(), false, [&] { return where(line, message); });
  }

  // The random numbers of the tests, from a seed of their own rather than from
  // the process identifier with srand48(getpid()), which Visual C++ does not
  // have: the checks are the same from one run to the next (GAOL v5)
  inline std::mt19937& generator()
  {
    static std::mt19937 g(10);
    return g;
  }

  // A double drawn in [0, 1), as drand48()
  inline double uniform()
  {
    return std::uniform_real_distribution<double>(0.0, 1.0)(generator());
  }

  // A non-negative integer, as rand()
  inline int integer()
  {
    return std::uniform_int_distribution<int>(0, 2147483647)(generator());
  }

  // Runs test of a new fixture, between its setUp() and its tearDown(); an
  // exception is a failure of the test
  template<class Fixture>
  void run(const std::string& suite, const std::string& test, void (Fixture::*method)())
  {
    current() = suite + ": " + test;
    try {
      Fixture fixture;
      fixture.setUp();
      (fixture.*method)();
      fixture.tearDown();
    } catch (const std::exception& e) {
      gaol_tests::check(current(), false, [&] { return std::string("exception: ") + e.what(); });
    } catch (...) {
      gaol_tests::check(current(), false, [] { return std::string("exception"); });
    }
  }
}

#define TEST_TRUE(a)   gaol_unit::check(bool(a), __LINE__, "TEST_TRUE(" #a ")")
#define TEST_FALSE(a)  gaol_unit::check(!(a), __LINE__, "TEST_FALSE(" #a ")")
#define TEST_EMPTY(a)  do { const auto gaol_a_ = (a); \
    gaol_unit::check(gaol_a_.is_empty(), __LINE__, "TEST_EMPTY(" #a ")", gaol_a_, "the empty set"); } while (0)
// Set equal
#define TEST_SEQ(a,b)  do { const auto gaol_a_ = (a); const auto gaol_b_ = (b); \
    gaol_unit::check(gaol_a_.set_eq(gaol_b_), __LINE__, "TEST_SEQ(" #a ", " #b ")", gaol_a_, gaol_b_); } while (0)
// Possibly equal: not disjoint, possibly_eq() being gone (GAOL v5)
#define TEST_PEQ(a,b)  do { const auto gaol_a_ = (a); const auto gaol_b_ = (b); \
    gaol_unit::check(!gaol_a_.set_disjoint(gaol_b_), __LINE__, "TEST_PEQ(" #a ", " #b ")", gaol_a_, gaol_b_); } while (0)
// At a Hausdorff distance of 1e-8 at most
#define TEST_EQ(a,b)   do { const auto gaol_a_ = (a); const auto gaol_b_ = (b); \
    gaol_unit::check(hausdorff(gaol_a_, gaol_b_) <= 1e-8, __LINE__, "TEST_EQ(" #a ", " #b ")", gaol_a_, gaol_b_); } while (0)
#define TEST_CONT(a,b) do { const auto gaol_a_ = (a); const auto gaol_b_ = (b); \
    gaol_unit::check(gaol_a_.set_contains(gaol_b_), __LINE__, "TEST_CONT(" #a ", " #b ")", gaol_a_, gaol_b_); } while (0)
#define CPPUNIT_ASSERT(a) TEST_TRUE(a)
#define CPPUNIT_FAIL(message) gaol_unit::fail(message, __LINE__)

#ifdef GAOL_FLOAT_INTERVALS
// The intervals of floats, which only a developer of GAOL compiles (see
// gaol/gaol_config.h)
#define TEST_PEQ4(a,b)  TEST_TRUE(interval2f(a).possibly_eq_all(interval2f(b)))
#define TEST_SEQ4(a,b)  TEST_TRUE(interval2f(a).set_eq_all(interval2f(b)))
#define TEST_EMPTY4(a)  TEST_TRUE(interval2f(a).first().is_empty() || interval2f(a).second().is_empty())
#define TEST_CONTAINS4(a,b) TEST_TRUE(interval2f(a).set_contains_all(interval2f(b)))
#define TEST_EQ4(a,b)   TEST_TRUE(hausdorff((a).first(),(b).first())<=1e-5 \
                                  && hausdorff((a).second(),(b).second())<=1e-5)
#endif

// main(), which runs the tests of the suite Fixture, named suite in the
// report, and returns 0 when every check passed
#define GAOL_UNIT_MAIN(Fixture, suite, ...)                                   \
  int main()                                                                  \
  {                                                                           \
    gaol::init();                                                             \
    using gaol_unit_fixture = Fixture;                                        \
    for (const auto& gaol_test_ : {__VA_ARGS__}) {                            \
      gaol_unit::run<Fixture>(suite, gaol_test_.first, gaol_test_.second);    \
    }                                                                         \
    gaol::cleanup();                                                          \
    return gaol_tests::summary();                                             \
  }
#define GAOL_UNIT_TEST(test) std::make_pair(std::string(#test), &gaol_unit_fixture::test)

const interval max_inf(std::numeric_limits<double>::max(),+GAOL_INFINITY);
const interval m_inf_m_max(-GAOL_INFINITY,-std::numeric_limits<double>::max());

const interval m_min_zero(-std::numeric_limits<double>::min(),0);
const interval zero_min(0.0,std::numeric_limits<double>::min());
const interval m_min_min(-std::numeric_limits<double>::min(),std::numeric_limits<double>::min());

#endif /* GAOL_UNIT_TESTS_H */
