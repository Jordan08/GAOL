/*-*-C++-*----------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *------------------------------------------------------------------------------
 * Tools of GAOL v5: the bounds of pow on the boxes of a rows file
 * (tests/tools/pow).
 *
 * A rows file has one box per line, "X | Y | S | H", where X is the base, Y
 * the exponent, S gaol_ieee1788::pow(X, Y) and H gaol::pow(X, Y); each is
 * "empty" or two numbers "l u" (decimal or hexadecimal, inf and -inf), and S
 * and H may be left out. Lines starting with '#' are comments, kept as they
 * are; gen_table.py extract writes such a file from the table of
 * pow_on_boxes() in tests/ieee1788.cpp.
 *
 * Writes the same file to the standard output, X and Y as they were written,
 * S and H computed by the GAOL the program is linked with, in hexadecimal and
 * without the sign of a zero, which the test does not check. For a degenerate
 * exponent Y = [p], gaol_ieee1788::pow(X, p) and gaol::pow(X, p) have to be S
 * and H, which tests/ieee1788.cpp checks too: the boxes where they are not
 * are written to the standard error, and the exit status is then 1.
 *
 *   evalrows rows.txt > results.txt
 *
 * The decimal numbers are read to nearest, as a compiler reads them.
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-29 by Jordan NININ
 *------------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *----------------------------------------------------------------------------*/

#include "gaol/gaol.h"
#include "gaol/gaol_ieee1788.h"

#include <cfenv>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using gaol::interval;

namespace
{
  std::string fmt(const interval& x)
  {
    if (x.is_empty()) {
      return "empty";
    }
    double l = x.left(), r = x.right();
    if (l == 0.0) {
      l = 0.0; // no sign of a zero
    }
    if (r == 0.0) {
      r = 0.0;
    }
    char b[80];
    std::snprintf(b, sizeof b, "%a %a", l, r);
    return b;
  }

  std::string trim(const std::string& s)
  {
    const std::size_t a = s.find_first_not_of(" \t\r"), b = s.find_last_not_of(" \t\r");
    return (a == std::string::npos) ? std::string() : s.substr(a, b - a + 1);
  }

  bool same(const interval& a, const interval& b)
  {
    return a.is_empty() ? b.is_empty() : (!b.is_empty() && a.left() == b.left() && a.right() == b.right());
  }

  // "empty" or "l u", read to nearest
  bool read_interval(const std::string& field, interval& x)
  {
    std::istringstream in(field);
    std::string a, b, rest;
    in >> a;
    if (a == "empty") {
      x = interval::emptyset();
      return !(in >> rest);
    }
    if (!(in >> b) || (in >> rest)) {
      return false;
    }
    char *ea, *eb;
    const int saved = std::fegetround();
    std::fesetround(FE_TONEAREST);
    const double l = std::strtod(a.c_str(), &ea), u = std::strtod(b.c_str(), &eb);
    std::fesetround(saved);
    if (*ea != '\0' || *eb != '\0') {
      return false;
    }
    x = interval(l, u);
    return true;
  }
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    std::fprintf(stderr, "usage: evalrows rows.txt > results.txt\n");
    return 2;
  }
  std::ifstream f(argv[1]);
  if (!f) {
    std::fprintf(stderr, "evalrows: cannot read %s\n", argv[1]);
    return 2;
  }
  gaol::init();
  int status = 0;
  std::string line;
  for (long n = 1; std::getline(f, line); ++n) {
    if (trim(line).empty() || trim(line)[0] == '#') {
      std::printf("%s\n", trim(line).c_str());
      continue;
    }
    std::string fields[4];
    std::istringstream in(line);
    int k = 0;
    while (k < 4 && std::getline(in, fields[k], '|')) {
      fields[k] = trim(fields[k]);
      ++k;
    }
    interval x, y;
    if (k < 2 || !read_interval(fields[0], x) || !read_interval(fields[1], y)) {
      std::fprintf(stderr, "evalrows: %s:%ld: not a box: %s\n", argv[1], n, line.c_str());
      gaol::cleanup();
      return 2;
    }
    const interval s = gaol_ieee1788::pow(x, y), h = gaol::pow(x, y);
    std::printf("%s | %s | %s | %s\n", fields[0].c_str(), fields[1].c_str(), fmt(s).c_str(), fmt(h).c_str());
    if (!y.is_empty() && y.left() == y.right()) {
      const double p = y.left();
      const interval sp = gaol_ieee1788::pow(x, p), hp = gaol::pow(x, p);
      if (!same(sp, s) || !same(hp, h)) {
        std::fprintf(stderr, "evalrows: %s:%ld: with the exponent as a double: %s | %s\n", argv[1], n,
                     fmt(sp).c_str(), fmt(hp).c_str());
        status = 1;
      }
    }
  }
  gaol::cleanup();
  return status;
}
