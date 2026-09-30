/*-*-C++-*------------------------------------------------------------------
 * gaol -- Just Another Interval Library
 *--------------------------------------------------------------------------
 * This file is part of the gaol distribution. Gaol was primarily 
 * developed at the Swiss Federal Institute of Technology, Lausanne, 
 * Switzerland, and is now developed at the Institut de Recherche 
 * en Informatique de Nantes, France.
 *
 * Copyright (c) 2001 Swiss Federal Institute of Technology, Switzerland
 * Copyright (c) 2002 Institut de Recherche en Informatique de Nantes, France
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated 
 * COPYING file for information.
 *--------------------------------------------------------------------------
 * By: Frederic Goualard <Frederic.Goualard@lina.univ-nantes.fr>
 *--------------------------------------------------------------------------*/

#include <cstdlib>
#include <iostream>
#include <string>
#include "gaol/gaol.h"
using namespace gaol;

using std::cout;
using std::endl;

namespace {

  bool all_checks_passed = true;

  // Every claim of the output is checked here: a claim that does not hold is
  // reported, and makes the program fail, which is how ctest runs it
  void check(bool holds, const std::string& claim)
  {
    if (!holds) {
      cout << "FAILED: " << claim << endl;
      all_checks_passed = false;
    }
  }

  /*
    The Goldstein-Price function, a classic test of global optimization:

      f(x, y) = [1 + (x + y + 1)^2 (19 - 14x + 3x^2 - 14y + 6xy + 3y^2)]
                [30 + (2x - 3y)^2 (18 - 32x + 12x^2 + 48y - 36xy + 27y^2)]

    This example and the manual of GAOL 4 wrote (x + y)^2 for (x + y + 1)^2,
    and computed another function, which is 111 at (0, -1), not 3.
  */
  interval goldstein_price(const interval& x, const interval& y)
  {
    return (interval(1.0)
            + pow(x + y + interval(1.0), 2)
              * (interval(19.0) - interval(14.0) * x + interval(3.0) * pow(x, 2)
                 - interval(14.0) * y + interval(6.0) * x * y + interval(3.0) * pow(y, 2)))
         * (interval(30.0)
            + pow(interval(2.0) * x - interval(3.0) * y, 2)
              * (interval(18.0) - interval(32.0) * x + interval(12.0) * pow(x, 2)
                 + interval(48.0) * y - interval(36.0) * x * y + interval(27.0) * pow(y, 2)));
  }

} // namespace

int main(void)
{
  // No gaol::init(): GAOL initialized itself before main()

  interval
    x(-2,2),
    y(-2,2), z(0.1);


  cout.precision(16);

  const interval zero = interval::universe()*interval(0.,0.);
  cout << zero << endl;
  check(zero.set_eq(interval(0.0)), "the whole line times 0 is {0}");
  reset_time();
  for (unsigned int i=0;i<1000000;++i) {
    z=goldstein_price(x, y);
  }
  cout << "z = " << z << endl;
  cout << "Elapsed time: " << elapsed_time() << " ms" << endl;

  // The minimum of f over [-2, 2]^2, f(0, -1) = 3, is where the two factors
  // are 1 and 3: the values on a point are exact, integers
  check(goldstein_price(interval(0.0), interval(-1.0)).set_eq(interval(3.0)), "f(0, -1) = 3");

  // z encloses the range of f over [-2, 2]^2, which is [3, 1015690.27...]: 3
  // at (0, -1), and the maximum on the edge y = 2, at x = -1.73737253775830...
  // (sympy and mpmath; see 03_dependency_problem.cpp), a decimal that a double
  // cannot hold. It does not enclose it tightly: the natural extension is
  // [-87881320, 147125080], computed by exact rational arithmetic in the order
  // of the expression. Every bound met on the way is an integer below 2^53
  // (the largest is 147125080), so that no operation rounds and the enclosure
  // is the same with every compiler and every implementation of GAOL.
  const interval range = textToInterval("[3, 1015690.2717980589082988423]");
  check(z.set_contains(range), "z encloses the range of f");
  check(z.set_eq(interval(-87881320.0, 147125080.0)), "z is the natural extension of f");

  // The program of the overview of the manual, with sqr and integers, prints
  // that enclosure
  const interval w=(1+sqr(x+y+1)*(19-14*x+3*sqr(x)-14*y+6*x*y+3*sqr(y)))*
    (30+sqr(2*x-3*y)*(18-32*x+12*sqr(x)+48*y-36*x*y+27*sqr(y)));
  check(w.set_eq(z), "the program of the manual gives the same enclosure");

  // Each occurrence of x and of y ranges over [-2, 2] on its own: the
  // dependency problem (03_dependency_problem.cpp cuts the domain to reduce
  // it). The width of z is 235006400, that of the range 1015687.27..., 231.4
  // times less
  const double ratio = z.width() / 1015687.2717980589;
  cout << "z is " << static_cast<int>(ratio) << " times wider than the range of f" << endl;
  check(ratio > 231.0 && ratio < 232.0, "z is 231 times wider than the range of f");

  // Always, right after the last use of GAOL: sets back the rounding direction
  gaol::cleanup();
  return all_checks_passed ? 0 : EXIT_FAILURE;
}
