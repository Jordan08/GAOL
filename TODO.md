# TODO

What is left to do on the branch MATH-CORE, found by the review of commit
a7544a1 (`gaol_ieee1788`, out of `gaol`). Each point was reproduced unless it
says otherwise. The continuous integration is green again since 1436918.

## pow in gaol_ieee1788

1. **`pow` with a number as exponent is GAOL's pow under the using-directive.**
   With `using namespace gaol_ieee1788;` and x = [-4, -1], `pow(x, 2)` and
   `pow(x, 2.0)` are [1, 16] (GAOL's pown, found by argument-dependent lookup),
   where `pow(x, interval(2))` is the empty set (the standard); `pow([0], 0)`
   is [1], where the standard has no value. The same spelling changes its
   meaning with the type of the exponent, silently.
   Fix: the trick used for `pow(interval, interval)`. `gaol::pow(interval, int)`
   and `gaol::pow(interval, double)` become function templates, and
   `gaol_ieee1788` gets its own `pow(interval, int)` and `pow(interval, double)`
   that follow the standard.

2. **Binary compatibility.** `libgaol.a` no longer has the symbol
   `gaol::pow(interval const&, interval const&)`: an object compiled against
   the former header (Ibex's GAOL backend, for instance) no longer links.
   Fix: keep in `gaol/gaol_interval.cpp` a plain definition of
   `pow(const interval&, const interval&)` that calls `pow_hybrid()`, without
   declaring it in the header. It keeps the former symbol and does not change
   overload resolution.

3. **An integer exponent beyond the ints gives [-oo, +oo]** (older than
   a7544a1). `gaol_ieee1788::pow([2, 3], [1e10])` is [-oo, +oo], where the
   exponent [1e10, 1e10 + 2] gives [1.79e308, +oo]: a valid enclosure, with
   negative values for a positive base. The pow of the standard goes through
   `pow_hybrid()`, which takes pown for any degenerate integer exponent
   (`gaol/gaol_interval.cpp`, the `interval::universe()` for an integer beyond
   the ints).

4. **Logic written twice** (older than a7544a1). `gaol_ieee1788::pow` repeats
   the intersection with [0, +oo] and the case of a zero base that
   `pow_hybrid()` does again. Fix, which settles point 3 as well: export the
   standard half of `pow_hybrid()` (for instance `pow_real()`), have
   `pow_hybrid()` add only the pown case of a degenerate integer exponent, and
   have `gaol_ieee1788::pow` call `pow_real()` directly.

## The names of gaol_ieee1788

5. **The overload set depends on the order of the includes.** A
   using-declaration such as `using ::gaol::sin;` takes the overloads declared
   before it. When `gaol/gaol_expression.h` is included before `gaol/gaol`,
   those of `expression` come in too, and `gaol_ieee1788::sin(0.5)` no longer
   compiles (a double converts to `interval` and to `expression`). Rare, a
   double being passed to a function of the standard, but the header says
   `interval` is the only type of its operations.

6. **Clash with `using namespace std;`.** `less(x, y)` is then ambiguous
   between `gaol_ieee1788::less` and the class template `std::less`. To be
   said in `doc/using.md`, which tells that one line is enough.

## intervalToExact

7. **Not exception-safe nor thread-safe** (older than a7544a1).
   `intervalToExact()` switches the global `interval::format()` to hexa and
   sets it back on the normal return only: an exception from `s << x` leaves
   every later output in hexadecimal, and another thread printing meanwhile
   prints hexadecimal. Fix: a guard that sets the format back in its
   destructor, or writing the bounds without the global format.

## Tests and documentation

8. **The test of the using-directive does not cover point 1.**
   `ieee1788_using_directive()` (`tests/other_functions.cpp`) compiles
   `pow(x, 3)` and `pow(x, 0.5)` but checks neither on a negative base;
   `pow(interval, interval)` is checked on a negative base.

9. **The documentation of pow.** The Doxygen block `\brief I^J` of
   `gaol/gaol_interval.h` now documents `pow_hybrid()`, and the template
   `pow` has a plain comment only; `manual/gaol.tex` still describes
   `pow(const interval&, const interval&)` as a plain function.

## CORE-MATH

10. **Propose the local fixes upstream.** `3rd/README.md` lists four changes
    of the CORE-MATH sources that are fixes rather than adaptations to GAOL:
    the masks `~0ull` of `sinh.c`, `cosh.c` and `tanh.c`, the signed shifts of
    `cospi.c`, the
    128-bit shift of `asinpi.c`, and the 64-bit `__builtin_expect` of
    `rsqrt.c`, which took subnormals for +0 wherever `long` has 32 bits.

Suggested order: points 1 to 4 together, all being about pow, then 8, then 6
and 9.
