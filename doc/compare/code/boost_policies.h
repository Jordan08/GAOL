// The intervals of Boost.Interval that the comparison measures, shared by
// bench_boost.cpp, the special cases (cases.py) and enclosure_boost.cpp.
//
// boost::numeric::interval<double> alone, the type with the default policies,
// rounded_math<double> and checking_strict<double>, has the arithmetic, sqrt,
// square, the integer powers and the roots, but no elementary function: its
// rounding policy, save_state<rounded_arith_opp<double> >, has no exp_down()
// nor sin_up(), and exp(x) does not compile. The elementary functions come with
// a policy rounded_transc_*<double>, which calls the functions of the C
// library, std::exp or std::sin, under the rounding direction the bound needs.
// The intervals here are the default ones with that layer added:
// rounded_transc_opp<double> derives from rounded_arith_opp<double>, the
// arithmetic of rounded_math<double>, so that everything else is the code of
// interval<double>, and it is the policy of the example of Boost.Interval that
// takes the elementary functions of the C library (examples/findroot_demo.cpp);
// the checking stays the default one, checking_strict<double>, which throws
// std::runtime_error when an operation creates the empty set. rounded_transc_std,
// which sets the direction downward or upward for each call and leaves it there,
// is the other choice; enclosure_boost.cpp measures both.
//
// Copyright (c) 2026 ENSTA, France
//
// Created 2026-10-06 by Jordan NININ
#ifndef BOOST_POLICIES_H
#define BOOST_POLICIES_H

#include <boost/numeric/interval.hpp>

typedef boost::numeric::interval<
  double, boost::numeric::interval_lib::policies<
            boost::numeric::interval_lib::save_state<boost::numeric::interval_lib::rounded_transc_opp<double> >,
            boost::numeric::interval_lib::checking_strict<double> > >
  boost_interval;

typedef boost::numeric::interval<
  double, boost::numeric::interval_lib::policies<
            boost::numeric::interval_lib::save_state<boost::numeric::interval_lib::rounded_transc_std<double> >,
            boost::numeric::interval_lib::checking_strict<double> > >
  boost_interval_std;

#endif // BOOST_POLICIES_H
