<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-18 by Jordan NININ -->
# A short tutorial: interval algorithms with GAOL v5

Part of the documentation of [GAOL v5](../README.md#documentation).

GAOL computes with intervals: an operation on intervals gives an interval that
contains every result of the operation on the points of its operands, and the
algorithms of interval computing build on that. This tutorial writes four of
them with GAOL, each as a complete program of a few dozen lines: the range of a
function by subdivision, the interval Newton method, a contractor, and a branch
and bound. They are the condensed forms of the examples 03, 05, 07 and 06 of
this directory (see [the examples](examples.md#12-the-programs)), which check what they
print against values computed apart and are the next thing to read; the first
program of the [README](../README.md#a-first-program) is the shortest one.

To try a program, save it as `prog.cpp`, and compile it with the flags GAOL
gives (see [Using GAOL](../doc/using.md#from-pkg-config)):

```bash
c++ -std=c++11 -O2 $(pkg-config --cflags gaol) prog.cpp -o prog $(pkg-config --libs gaol)
```

The outputs shown are those of the programs, the same with the SSE2 and the FPU
intervals. GAOL writes the bounds of an interval rounded outward, so that the
text always encloses the interval: `[0.24, 0.25]` is printed
`[0.2399999999999999, 0.25]`, the double below 0.24 and the double 0.25.

## 1. The range of a function, and subdivision

Computing a formula with intervals encloses the range of the function it
writes (the fundamental theorem of interval arithmetic), but usually
overestimates it: each occurrence of a variable ranges over its interval on its
own, as if the others did not exist. This is the dependency problem. On
x (1 − x) over [0.4, 0.6], whose range is [0.24, 0.25], the formula written as
it is gives an enclosure 20 times too wide, the same function written
with x once, 1/4 − (x − 1/2)², gives the range itself, and cutting x into
pieces, whose images are joined by the hull `|`, narrows the enclosure
linearly: four times as many pieces, four times less excess.

```cpp
#include <iostream>
#include <utility>
#include <gaol/gaol.h>
using namespace gaol;

// f(x) = x (1 - x), written as it is: x occurs twice
interval f(const interval& x)
{
  return x * (1.0 - x);
}

// The same function where x occurs once: 1/4 - (x - 1/2)^2
interval g(const interval& x)
{
  return 0.25 - sqr(x - 0.5);
}

// The hull of f over the 2^depth pieces of x that bisect() cuts: the pieces
// cover x, so the hull encloses the range of f over x
interval image(const interval& x, int depth)
{
  if (depth == 0) {
    return f(x);
  }
  const std::pair<interval, interval> halves = x.bisect(0.5);
  return image(halves.first, depth - 1) | image(halves.second, depth - 1);
}

int main()
{
  const interval x = textToInterval("[0.4, 0.6]");   // the range of f is [0.24, 0.25]
  std::cout << "f(x)         " << f(x) << '\n';
  std::cout << "g(x)         " << g(x) << '\n';
  for (int depth = 2; depth <= 8; depth += 2) {
    std::cout << "2^" << depth << " pieces   " << image(x, depth) << '\n';
  }
  gaol::cleanup();
  return 0;
}
```

```text
f(x)         [0.1599999999999999, 0.3600000000000002]
g(x)         [0.2399999999999999, 0.25]
2^2 pieces   [0.2199999999999999, 0.2750000000000001]
2^4 pieces   [0.2349999999999999, 0.2562500000000001]
2^6 pieces   [0.2387499999999999, 0.2515625000000001]
2^8 pieces   [0.2396874999999999, 0.2503906250000001]
```

- `textToInterval("[0.4, 0.6]")` encloses the decimals 0.4 and 0.6, which the
  doubles 0.4 and 0.6 are not (`interval(0.4, 0.6)` would be another box).
- `sqr(x)` is tighter than `x * x`, and `0.25`, `0.5` and `1.0` are exact
  doubles. A constant that is no double, such as 0.1, is read from its text.
- `x.bisect(0.5)` cuts x in two intervals that share the cut point, so that the
  pieces cover x and their hull encloses the range over x. A ratio other than 0.5
  keeps a solution of a symmetric problem off the cut: IBEX's bisectors cut at 45 %
  (example 05 shows why), and the `bisect()` of Codac at 0.49 by default.

Example 03 shows the dependency problem on more functions (the polynomial
(x − 1)⁵ in Horner's form and in its power form, the Goldstein-Price function)
and the wrapping effect of the rotations of a box.

## 2. The interval Newton method

The Newton step of the interval Newton method encloses the roots of f in an
interval X: a root x satisfies f(x) = f(m) + f'(ξ)(x − m) = 0 for some ξ in X, so
x − m is a solution of the equation (x − m) d = −f(m) for some d in the
enclosure f'(X) of the derivative. Where f'(X) holds no 0, that is a division,
m − f(m) / f'(X), and its result strictly inside X proves that X holds a root. Where
f'(X) holds 0, the division `/` is of no use: it is the set of the quotients
a / b with b other than 0, which is [0] for 0 / f'(X) where f(m) = 0, and
"proves" one root where there are several (the manual's "Common errors" show
it). The relational division `div_rel`, which solves the equation, is the one
to use, and dividing by the negative and by the positive part of f'(X) apart
keeps the gap around m, so that one step can cut X into two intervals.

```cpp
#include <iostream>
#include <vector>
#include <gaol/gaol.h>
using namespace gaol;

// f(x) = x^2 - 2 and its derivative, written once for intervals
interval f(const interval& x)  { return sqr(x) - 2.0; }
interval df(const interval& x) { return 2.0 * x; }

// The part of X where a root of f may be, after one step of the interval
// Newton method: a root x of f in X satisfies (x - m) d = -f(m) for some d in
// f'(X), where m is a point of X. div_rel() solves that relation for x - m
// (the division / does not: see "Newton's method" in the manual), and cutting
// f'(X) into its negative and positive parts keeps the gap around m that the
// division by an interval holding 0 would fill.
std::vector<interval> newton_step(const interval& X)
{
  const interval m = X.mid();                    // an interval enclosing the middle of X
  const interval fm = f(m);                      // f(m) computed with intervals, rounded outward
  const interval D = df(X);
  const interval neg = (m + div_rel(-fm, D & interval::negative(), X - m)) & X;
  const interval pos = (m + div_rel(-fm, D & interval::positive(), X - m)) & X;
  std::vector<interval> parts;
  if (!neg.is_empty()) {
    parts.push_back(neg);
  }
  if (!pos.is_empty()) {
    parts.push_back(pos);
  }
  return parts;
}

int main()
{
  std::vector<interval> work(1, interval(-3.0, 3.0)), roots;
  while (!work.empty()) {
    interval X = work.back();
    work.pop_back();
    if (!f(X).set_contains(0.0)) {
      continue;                                  // f(X) encloses the range of f: no root in X
    }
    if (X.width() < 1e-12) {
      roots.push_back(X);
      continue;
    }
    for (const interval& part : newton_step(X)) {
      if (part.set_eq(X)) {                      // no progress: cut X in two
        const std::pair<interval, interval> halves = X.bisect(0.5);
        work.push_back(halves.first);
        work.push_back(halves.second);
      } else {
        work.push_back(part);
      }
    }
  }
  for (const interval& r : roots) {
    std::cout << r << '\n';
  }
  gaol::cleanup();
  return 0;
}
```

```text
[1.414213562373094, 1.414213562373096]
[-1.414213562373096, -1.414213562373094]
```

- `X.mid()` is an interval enclosing the middle of X, so that `f(m)` is
  computed with intervals and rounded outward; with doubles it would not
  enclose f at the middle.
- `f(X)` encloses the range of f over X: where it holds no 0, X holds no root.
  `set_contains()` is the test of membership, and there is no `==` on
  intervals (`set_eq()` is the equality of two sets).
- The two intervals printed are the ones no test discarded, each narrower than
  1e-12 and holding one of ±√2. To prove that an interval holds exactly one
  root, the algorithm asks that the step N(X) lies strictly inside X
  (`X.set_strictly_contains(N)`) and that f'(X) holds no 0, as example 05 does.
  It also takes the enclosure of f' from the automatic differentiation of
  `examples/dual.h`, instead of writing it by hand.

## 3. A contractor

A contractor shrinks a box without losing any solution of a constraint. The
forward-backward contractor of x² + y² = 1 evaluates the expression tree
forward, from x and y to the sum, and intersects the root with 1; it then goes
backward, from the root down, and each node is intersected with what the
others allow. GAOL's relational functions, the `*_rel` functions, are the
backward operations: `sqrt_rel(a, x)` is the hull of the x of X with x² in a,
both signs of the roots included, where `sqrt(a)` would keep the positive root
alone and lose the solutions with x < 0.

```cpp
#include <iostream>
#include <gaol/gaol.h>
using namespace gaol;

// The contractor of x^2 + y^2 = 1: forward-backward evaluation of the tree
// s = a + c, a = sqr(x), c = sqr(y), with s = 1. Returns false when the box
// holds no solution.
bool contract(interval& x, interval& y)
{
  // forward: an enclosure of each node over the box
  interval a = sqr(x);
  interval c = sqr(y);
  interval s = (a + c) & interval(1.0);     // the root must be 1
  if (s.is_empty()) {
    return false;
  }
  // backward: s = a + c gives a = s - c and c = s - a ...
  a &= s - c;
  c &= s - a;
  // ... and a = x^2 gives x = +-sqrt(a): sqrt_rel() keeps both signs, and
  // intersects with x itself, where sqrt(a) would keep the positive one only
  x = sqrt_rel(a, x);
  y = sqrt_rel(c, y);
  return !x.is_empty() && !y.is_empty();
}

int main()
{
  interval x(0.8, 3.0), y(-3.0, 3.0);
  std::cout << "box           " << x << ' ' << y << '\n';
  contract(x, y);
  std::cout << "contracted    " << x << ' ' << y << '\n';
  interval u(2.0, 3.0), v(2.0, 3.0);
  std::cout << "box           " << u << ' ' << v << '\n'
            << (contract(u, v) ? "may hold a solution\n" : "proved: no solution\n");
  gaol::cleanup();
  return 0;
}
```

```text
box           [0.8, 3] [-3, 3]
contracted    [0.8, 1] [-0.6000000000000001, 0.6000000000000001]
box           [2, 3] [2, 3]
proved: no solution
```

- The first box is cut to x in [0.8, 1], y in [−0.6, 0.6] (the double 0.8 and
  the bounds are enclosures): the points of the box off this part are not on
  the circle. The second box is proved empty of solutions: its `s` is empty.
- The names are those of GAOL, and of IEEE 1788-2015 in `gaol_ieee1788`:
  `sqrt_rel(c, x)` is `sqrRev(c, x)`, `div_rel(c, b, x)` is `mulRev(b, c, x)`
  with its arguments in another order, `nth_root_rel(c, n, x)` is
  `pownRev(c, x, n)`.
- Contractors in a loop until no side shrinks by 10 % any more, cutting the
  widest side, are a branch and prune and a paving: examples 07, 08 and 09, with
  the box type of `examples/box.h`, and the contractor on the constraint
  y = eˣ − 1.5 as well.

## 4. Branch and bound

A branch and bound finds the global minimum of f over an interval by cutting
it into pieces. The enclosure of f over a piece gives a lower bound of f there;
f at the middle of a piece, which is a value of f, gives an upper bound of the
minimum. A piece whose lower bound exceeds the best upper bound holds no
minimizer and is dropped; the others are cut again, the one with the least
lower bound first, until the least lower bound and the best upper bound are
within a tolerance.

```cpp
#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
#include <gaol/gaol.h>
using namespace gaol;

// f(x) = x^4 - 4 x^2 + x, written once for intervals
interval f(const interval& x)
{
  return sqr(sqr(x)) - 4.0 * sqr(x) + x;
}

// A piece of the search: an interval of x, and an enclosure of f over it
struct Piece {
  interval x;
  interval range;
};

// The order of the queue: the piece whose enclosure has the least lower bound
// first. It compares doubles. The operators < of intervals are no ordering,
// and std::priority_queue would misbehave with them.
struct LargerLowerBound {
  bool operator()(const Piece& a, const Piece& b) const
  {
    return a.range.left() > b.range.left();
  }
};

int main()
{
  const double tolerance = 1e-6;
  std::priority_queue<Piece, std::vector<Piece>, LargerLowerBound> queue;
  const interval domain(-3.0, 3.0);
  queue.push(Piece{ domain, f(domain) });
  double upper = f(domain.mid()).right();        // f(m) is above the minimum: an upper bound of it
  int cut = 0;
  while (!queue.empty()) {
    const Piece p = queue.top();
    queue.pop();
    if (p.range.left() > upper - tolerance) {    // no piece left can lower the minimum by more than the tolerance
      std::cout << "the minimum of f is in " << interval(p.range.left(), upper) << '\n';
      break;
    }
    ++cut;
    const std::pair<interval, interval> halves = p.x.bisect(0.5);
    for (const interval& half : { halves.first, halves.second }) {
      const Piece q = { half, f(half) };
      upper = std::min(upper, f(half.mid()).right());
      if (q.range.left() <= upper) {             // else no point of this piece goes below a value already seen
        queue.push(q);
      }
    }
  }
  std::cout << cut << " cuts\n";
  gaol::cleanup();
  return 0;
}
```

```text
the minimum of f is in [-5.44419306660636, -5.444192066610894]
19933 cuts
```

The minimum of x⁴ − 4x² + x is −5.4441920666108974... (mpmath), which the
interval printed contains.

- A priority queue needs an ordering. `<` on intervals is not one (it is the
  certainly relation of IEEE 1788-2015), so the comparator takes the lower bound
  of the enclosure, a double (see
  [Relations, sorting and containers](../doc/using.md#relations-sorting-and-containers)).
- The natural form of f converges slowly, the dependency problem returning.
  Example 06 compares, on the six-hump camel function, the natural form, which
  needs 25 057 bisections to bring the enclosure of the minimum within 1e-3 and does not
  reach 1e-6 within 120 000, and the mean-value form, with the derivative from
  `examples/dual.h`, which reaches 1e-9 in 1 127, and 261 with a monotonicity
  test.

## Where next

- [Examples](examples.md): the sixteen programs of this directory, which solve
  systems (Krawczyk), estimate parameters, enclose differential equations and
  use affine arithmetic, and a review of how intervals are used with GAOL v5.
- [Using GAOL](../doc/using.md): the tools of interval algorithms (`inflate`,
  `bisect`, `midrad`, `mulRevToPair`), the namespaces, the names of IBEX, Codac,
  C-XSC and Boost.Interval next to GAOL's, and the pitfalls.
- The manual of GAOL v5, `manual/v5/gaol.pdf`: each operation, with examples.
