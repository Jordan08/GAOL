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
   **Done otherwise.** `interval` and GAOL's functions are in `gaol_core`,
   where argument-dependent lookup finds no `pow`, and `gaol` and
   `gaol_ieee1788` each have their own, plain functions. `pow(x, p)` of
   `gaol_ieee1788` is `pow(x, [p])`, an int exponent included, the integer
   power being `pown(x, n)` as in the standard.

2. **Binary compatibility.** `libgaol.a` no longer has the symbol
   `gaol::pow(interval const&, interval const&)`: an object compiled against
   the former header (Ibex's GAOL backend, for instance) no longer links.
   Fix: keep in `gaol/gaol_interval.cpp` a plain definition of
   `pow(const interval&, const interval&)` that calls `pow_hybrid()`, without
   declaring it in the header. It keeps the former symbol and does not change
   overload resolution.
   **No longer applies.** Every symbol of the library names `gaol_core` now:
   a program is compiled again against the new headers.

3. **An integer exponent beyond the ints gives [-oo, +oo]** (older than
   a7544a1). `gaol_ieee1788::pow([2, 3], [1e10])` is [-oo, +oo], where the
   exponent [1e10, 1e10 + 2] gives [1.79e308, +oo]: a valid enclosure, with
   negative values for a positive base. The pow of the standard goes through
   `pow_hybrid()`, which takes pown for any degenerate integer exponent
   (`gaol/gaol_interval.cpp`, the `interval::universe()` for an integer beyond
   the ints).
   **Done** in `gaol/gaol_ieee1788.h`: CORE-MATH's pow at the bounds of x,
   [DBL_MAX, +oo] here; `tests/other_functions.cpp` checks five such
   exponents.

4. **Logic written twice** (older than a7544a1). `gaol_ieee1788::pow` repeats
   the intersection with [0, +oo] and the case of a zero base that
   `pow_hybrid()` does again. Fix, which settles point 3 as well: export the
   standard half of `pow_hybrid()` (for instance `pow_real()`), have
   `pow_hybrid()` add only the pown case of a degenerate integer exponent, and
   have `gaol_ieee1788::pow` call `pow_real()` directly.
   **Still open.** `gaol_ieee1788::pow` still intersects x with [0, +oo] and
   takes x = {0} apart before `gaol_pow_hybrid()` does it again; point 3 was
   settled apart.

## The names of gaol_ieee1788

5. **The overload set depends on the order of the includes.** A
   using-declaration such as `using ::gaol::sin;` takes the overloads declared
   before it. When `gaol/gaol_expression.h` is included before `gaol/gaol`,
   those of `expression` come in too, and `gaol_ieee1788::sin(0.5)` no longer
   compiles (a double converts to `interval` and to `expression`). Rare, a
   double being passed to a function of the standard, but the header says
   `interval` is the only type of its operations.
   **Done.** `gaol/gaol_ieee1788.h` includes `gaol/gaol_expression.h` before
   its using-declarations, whatever the program includes first.

6. **Clash with `using namespace std;`.** `less(x, y)` is then ambiguous
   between `gaol_ieee1788::less` and the class template `std::less`. To be
   said in `doc/using.md`, which tells that one line is enough.
   **Done.** `doc/using.md` says to write `gaol_ieee1788::less(x, y)` there.

## intervalToExact

7. **Not exception-safe nor thread-safe** (older than a7544a1).
   `intervalToExact()` switches the global `interval::format()` to hexa and
   sets it back on the normal return only: an exception from `s << x` leaves
   every later output in hexadecimal, and another thread printing meanwhile
   prints hexadecimal. Fix: a guard that sets the format back in its
   destructor, or writing the bounds without the global format.
   **Done.** `exact_string(I)` (`gaol/gaol_interval.h`) writes the bounds
   without the global format; `operator<<` in `interval_format::hexa` and
   `intervalToExact()` call it. `tests/ieee1788.cpp` calls `intervalToExact()`
   in one thread while another writes intervals in `interval_format::bounds`:
   11422 of the 12680 intervals it wrote were in hexadecimal before.

## Tests and documentation

8. **The test of the using-directive does not cover point 1.**
   `ieee1788_using_directive()` (`tests/other_functions.cpp`) compiles
   `pow(x, 3)` and `pow(x, 0.5)` but checks neither on a negative base;
   `pow(interval, interval)` is checked on a negative base.
   **Done.** `tests/ieee1788.cpp`, under `using namespace gaol_ieee1788;`
   alone, checks `pow(x, 2)`, `pow(x, 2.0)` and `pow([0], 0)` on
   x = [-4, -1], and the expressions `pow(e1, e2)` and `pown(e, n)`.

9. **The documentation of pow.** The Doxygen block `\brief I^J` of
   `gaol/gaol_interval.h` now documents `pow_hybrid()`, and the template
   `pow` has a plain comment only; `manual/gaol.tex` still describes
   `pow(const interval&, const interval&)` as a plain function.
   **Done** for the comments, which name `gaol_pown()`, `gaol_pow_hybrid()` and
   `gaol_pow_real()` and the `gaol::pow` each one is; `manual/gaol.tex` is the
   manual of GAOL 4, left as it is.

## CORE-MATH

10. **Propose the local fixes upstream.** `3rd/README.md` lists four changes
    of the CORE-MATH sources that are fixes rather than adaptations to GAOL:
    the masks `~0ull` of `sinh.c`, `cosh.c` and `tanh.c`, the signed shifts of
    `cospi.c`, the
    128-bit shift of `asinpi.c`, and the 64-bit `__builtin_expect` of
    `rsqrt.c`, which took subnormals for +0 wherever `long` has 32 bits.

Left: points 4 and 10.
