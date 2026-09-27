/*-*-C++-*------------------------------------------------------------------
 * gaol -- NOT Just Another Interval Library
 *--------------------------------------------------------------------------
 * Examples of GAOL v5: a box, the vector of intervals that interval
 * algorithms work on.
 *
 * GAOL is a library of scalar intervals. Solvers, pavers and optimizers
 * handle boxes, one interval per variable, which IBEX calls IntervalVector
 * and Codac IntervalVector. The examples share this small class rather than
 * each writing its own: it holds only what they need, with the names IBEX
 * and Codac give these operations (max_width is their max_diam).
 *
 * Copyright (c) 2026 ENSTA, France
 *
 * Created 2026-09-27 by Jordan NININ
 *--------------------------------------------------------------------------
 * gaol is a software distributed WITHOUT ANY WARRANTY. Read the associated
 * COPYING file for information.
 *--------------------------------------------------------------------------*/

#ifndef GAOL_EXAMPLES_BOX_H
#define GAOL_EXAMPLES_BOX_H

#include <cstddef>
#include <initializer_list>
#include <limits>
#include <ostream>
#include <utility>
#include <vector>

#include <gaol/gaol.h>

namespace examples {

using gaol::interval;

class Box {
public:
  Box() = default;
  // n components equal to x; interval() is the whole line [-oo, +oo]
  explicit Box(std::size_t n, const interval& x = interval()) : x_(n, x) {}
  Box(std::initializer_list<interval> l) : x_(l) {}

  std::size_t size() const { return x_.size(); }
  interval& operator[](std::size_t i) { return x_[i]; }
  const interval& operator[](std::size_t i) const { return x_[i]; }

  // A box is empty as soon as one of its components is
  bool is_empty() const {
    for (const interval& x : x_) {
      if (x.is_empty()) {
        return true;
      }
    }
    return false;
  }

  // Index of the widest component, the one bisected first
  std::size_t widest() const {
    std::size_t k = 0;
    for (std::size_t i = 1; i < x_.size(); ++i) {
      if (x_[i].width() > x_[k].width()) {
        k = i;
      }
    }
    return k;
  }

  // Width of the widest component (IBEX's and Codac's max_diam), rounded up
  double max_width() const { return x_.empty() ? 0.0 : x_[widest()].width(); }

  // The midpoints, as the boxes that interval evaluation needs: mid() is the
  // tightest interval around the real midpoint of each component, one double
  // when that midpoint is a double, two consecutive doubles otherwise
  // (interval(0.1, 0.3).mid() is [0.1999999999999999, 0.2000000000000001])
  Box mid() const {
    Box m(x_.size());
    for (std::size_t i = 0; i < x_.size(); ++i) {
      m[i] = x_[i].mid();
    }
    return m;
  }

  // An enclosure of the volume, computed with intervals rather than with the
  // widths of doubles, so that its lower bound is a lower bound too. An empty
  // box has the volume 0. An unbounded side counts for [DBL_MAX, +oo], which
  // makes the volume unbounded, or 0 if another side is a point:
  // interval(+oo) being the empty set, its width interval(x.right()) -
  // interval(x.left()) would be empty, and would empty a sum of volumes
  interval volume() const {
    if (is_empty()) {
      return interval(0.0);
    }
    interval v(1.0);
    for (const interval& x : x_) {
      v *= x.is_finite() ? interval(x.right()) - interval(x.left())
                         : interval(std::numeric_limits<double>::max(), GAOL_INFINITY);
    }
    return v;
  }

  // True when every component of b is a subset of the same component of *this
  bool contains(const Box& b) const {
    for (std::size_t i = 0; i < x_.size(); ++i) {
      if (!x_[i].set_contains(b[i])) {
        return false;
      }
    }
    return true;
  }

  // True when some component of the box can no longer be split: its two
  // bounds are the same double or two consecutive doubles
  bool is_canonical() const {
    for (const interval& x : x_) {
      if (x.is_canonical()) {
        return true;
      }
    }
    return false;
  }

  // Cuts component i at the fraction ratio of its width. IBEX cuts at 0.45 and
  // Codac at 0.49 by default: cutting exactly in the middle often falls on a
  // solution of a symmetric problem, which then lies in two boxes. Unbounded
  // components are cut by split(), at their IEEE 1788 midpoint (0 or
  // +/-DBL_MAX).
  // The cut is computed with intervals, whose bounds do not depend on the
  // rounding direction the program is in: computed with doubles, it would be
  // rounded upward in the default build and to nearest with
  // GAOL_PRESERVE_ROUNDING, and the pavings would differ between the two.
  std::pair<Box, Box> bisect(std::size_t i, double ratio = 0.45) const {
    Box l(*this), r(*this);
    const interval& x = x_[i];
    if (!x.is_finite()) {
      x.split(l[i], r[i]);
      return {l, r};
    }
    double m = (interval(x.left()) + ratio * (interval(x.right()) - interval(x.left()))).right();
    if (!(x.left() < m && m < x.right())) {
      m = x.midpoint();
    }
    l[i] = interval(x.left(), m);
    r[i] = interval(m, x.right());
    return {l, r};
  }

  std::pair<Box, Box> bisect(double ratio = 0.45) const {
    return bisect(widest(), ratio);
  }

  // Intersection and hull, component by component
  Box& operator&=(const Box& b) {
    for (std::size_t i = 0; i < x_.size(); ++i) {
      x_[i] &= b[i];
    }
    return *this;
  }
  Box& operator|=(const Box& b) {
    for (std::size_t i = 0; i < x_.size(); ++i) {
      x_[i] |= b[i];
    }
    return *this;
  }
  friend Box operator&(Box a, const Box& b) { return a &= b; }
  friend Box operator|(Box a, const Box& b) { return a |= b; }

  // Written as IBEX and Codac write boxes: ([a, b] ; [c, d]). The precision
  // of the stream is saved and restored around the intervals.
  friend std::ostream& operator<<(std::ostream& os, const Box& b) {
    const std::streamsize p = os.precision();
    os << '(';
    for (std::size_t i = 0; i < b.size(); ++i) {
      os << (i == 0 ? "" : " ; ") << b[i];
    }
    os << ')';
    os.precision(p);
    return os;
  }

private:
  std::vector<interval> x_;
};

} // namespace examples

#endif /* GAOL_EXAMPLES_BOX_H */
