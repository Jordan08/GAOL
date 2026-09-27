/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Tests of GAOL v5: GAOL's headers with GAOL_DEBUGGING.
 *
 * The Debug builds define GAOL_DEBUGGING in gaol/gaol_configuration.h
 * (CMAKE_BUILD_TYPE=Debug, configure --enable-debug, meson
 * --buildtype=debug): GAOL then checks its assertions (GAOL_ASSERT), and
 * GAOL_DEBUG(level, command) runs command where the level given to
 * gaol::init() is level or more. gaol/gaol_expression.h did not compile with
 * it: its GAOL_DEBUG write on std::cout, which no header of GAOL included
 * (configure --enable-debug, the only build that defined it, failed). This
 * test includes GAOL's headers with GAOL_DEBUGGING whatever the build, builds
 * and evaluates an expression, whose nodes call GAOL_DEBUG, and checks that
 * GAOL_DEBUG runs its command at the level given to gaol::init().
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

// GAOL_DEBUGGING for GAOL's headers, whatever the build wrote
#include "gaol/gaol_configuration.h"
#undef GAOL_DEBUGGING
#define GAOL_DEBUGGING 1

#include "gaol_tests.h"

#include <iostream>
#include <sstream>
#include <string>

#include "gaol/gaol_expr_eval.h"

using namespace gaol;
using namespace gaol_tests;

int main()
{
  // Level 2: the commands of GAOL_DEBUG(1, ...) and GAOL_DEBUG(2, ...) run
  gaol::init(2);

  std::ostringstream out;
  std::streambuf* const saved = std::cout.rdbuf(out.rdbuf());
  const expression x = expression(interval(1.0, 2.0));
  const expression e = x + x * expression(3.0);
  expr_eval ev;
  e.get_root()->accept(ev);
  GAOL_DEBUG(2, std::cout << "level 2");
  GAOL_DEBUG(3, std::cout << "level 3");
  std::cout.rdbuf(saved);

  check("GAOL_DEBUGGING: an expression is evaluated", ev.result().set_eq(interval(4.0, 8.0)),
        [&] { return hex(ev.result()); });
  check("GAOL_DEBUGGING: GAOL_DEBUG runs its command at the level given to init()",
        out.str().find("level 2") != std::string::npos, [&] { return out.str(); });
  check("GAOL_DEBUGGING: GAOL_DEBUG does not run it above that level",
        out.str().find("level 3") == std::string::npos, [&] { return out.str(); });

  gaol::cleanup();
  return summary();
}
