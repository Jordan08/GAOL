# TODO

What is left to do on GAOL v5. What is done is in
[What differs from GAOL](doc/differences.md).

## Code

1. **The pow of the standard is written twice.** `gaol_ieee1788::pow`
   (`gaol/gaol_ieee1788.h`) intersects x with [0, +oo] and takes x = {0}
   apart, and `gaol_pow_hybrid()` does it again. Fix: export the standard half
   of `gaol_pow_hybrid()`, the pow of Table 9.1 for an interval exponent, have
   `gaol_pow_hybrid()` add only the pown case of a degenerate integer
   exponent, and have `gaol_ieee1788::pow` call that half directly, apart
   from its integer exponents beyond the ints.

2. **The rounding direction is still checked more than once** by `tan()`, the
   relational functions of the trigonometric functions (`acos_rel()`,
   `asin_rel()`, `atan_rel()`) and the negative integer powers `pow(x, -n)`,
   which call operations of intervals (`+`, `/`, `inverse()`, `atan()`...)
   that check it again: six times per call of `tan()` and of the relational
   functions. Fix: cores of these operations without the check, in
   `gaol/gaol_interval_sse.cpp` and `gaol/gaol_interval_fpu.cpp`, called
   after the check of the function calling them, as the functions of
   namespace `upward` are for the bounds at doubles.

3. **`pow(x, y)` is one double wide where the power at a corner is a double.**
   The lower bound is the double below CORE-MATH's value rounded upward, even
   where that value is exact: `pow([4], 0.5)` is [2 − 2^-52, 2], and cases
   149 to 152 and 163 of [doc/compare/special_cases.md](doc/compare/special_cases.md)
   are wider than IEEE 1788's result for this reason alone. Fix: keep the
   value as the lower bound where the power is exact, as `exp2`, `log2`,
   `nth_root(x, 3)` and the functions of Table 10.5 do.

## CORE-MATH

4. **Propose the fixes of the vendored sources upstream.**
   [3rd/README.md](3rd/README.md) lists five changes of the CORE-MATH sources;
   four of them are fixes rather than adaptations to GAOL: the masks `~0ull`
   of `sinh.c`, `cosh.c` and `tanh.c`, the signed shifts of `cospi.c`, the
   128-bit shift of `asinpi.c`, and the 64-bit `__builtin_expect` of
   `rsqrt.c`, which took subnormals for +0 wherever `long` has 32 bits.
   This is also how the work of GAOL v5 reaches the glibc, which imports
   CORE-MATH's functions (glibc 2.41 to 2.44; `cosh`, `sinh` and `tanh` in
   2.44) and runs on 32-bit Linux targets, where `long` has 32 bits too.
   To check: whether the glibc's copies still carry the masks `~0ul` and the
   64-bit `__builtin_expect`; if so, the same fixes are sent to the glibc
   (libc-alpha, with a `Signed-off-by` line: no copyright assignment to the
   FSF since August 2021).

## Documentation

5. **The coverage report** ([coverage/README.md](coverage/README.md)) was
   written on 2026-09-20: its lines not run no longer match the sources. To
   write again with `-DGAOL_COVERAGE=ON` and the target `coverage`, which
   need gcovr.

6. **The timings of [doc/compare/performance.md](doc/compare/performance.md)**
   were measured at `bb6f7e4` with files outside the sources changed
   (`bb6f7e4-dirty`), before the rounding direction was checked once per
   function of intervals (`exp()` 5.7 % faster since, `sin()` 3.6 %,
   `pow(x, y)` 3.4 %). To measure again on a clean commit, the machine doing
   nothing else (`doc/compare/code/run_bench.sh`).

## Licence

7. **Move GAOL v5 to the MIT licence, as CORE-MATH.** GAOL v5 is under the
   GNU LGPL v2 of GAOL (`COPYING.LIB`), and only the holders of the rights can
   change that.
   - **Whose code it is** (`git blame` on 2026-09-22, outside `3rd/` and the
     parser written by Bison): in `gaol/`, 13 592 lines of Frédéric Goualard,
     5 650 of Jordan Ninin and 2 of Raphaël Chenouard; in `check/`, 3 148 and
     41; `tests/`, 11 242 lines, all of Jordan Ninin; the meson files, about
     740 lines of Raphaël Chenouard and 360 of Jordan Ninin.
   - **Who has to agree.** Frédéric Goualard, and the establishments the
     headers name: the EPFL (2001), the IRIN and the LINA (2002-2011, now the
     LS2N of Nantes Université); in France, software written by an agent in
     the course of their duties belongs to their employer (article L113-9 of
     the Code de la propriété intellectuelle), so the agreement goes through
     the technology transfer offices of these establishments. ENSTA, which
     holds the copyright of the files GAOL v5 adds. Raphaël Chenouard for the
     meson files, or they are written again. Nothing to ask for CORE-MATH
     (already MIT),
     `gaol/s_nextafter.c` (Sun's licence, permissive: its notice stays) and
     the parser written by Bison (its exception leaves the licence free).
     Rewriting Goualard's code instead is no way round: code rewritten from
     it remains a derived work.
   - **If the agreement is refused:** the files of ENSTA alone (`tests/` and
     the files GAOL v5 adds) under MIT, with ENSTA's agreement, reusable
     anywhere; the library as a whole stays under the LGPL.
   - **Once agreed:** `COPYING.LIB` replaced by a `LICENSE`, a line
     `SPDX-License-Identifier: MIT` in the headers, the section Licences of
     `README.md`, the `License:` of `gaol.spec.in`. The releases already
     published (GAOL 4.2.2) stay under the LGPL.
   - **What MIT brings.** One licence for GAOL v5 and CORE-MATH. No more
     doubt for software that is not free: the LGPL v2 (section 5) leaves a
     program compiled with the library free only if it takes from it "small
     inline functions (ten lines or less in length)", and
     `gaol_interval.h`, `gaol_double_op.h`, `gaol_interval_sse.h` and
     `gaol_interval_fpu.h` define about 225 `INLINE` functions. Code
     reusable by any project, which is how CORE-MATH entered the glibc. And
     MIT is in the list of licences the French administrations may use
     (article D323-2-1 of the Code des relations entre le public et
     l'administration), where the LGPL v2 is not (the LGPL-3.0-or-later is).
   - **What MIT loses.** The reciprocity: whoever improves GAOL v5 may
     distribute the improvements without their sources. No clause on patents
     (Apache 2.0 has one); a small risk here.
   - **GCC.** Its runtime libraries (libstdc++, libgcc) are under the GPLv3
     with the GCC Runtime Library Exception. Code under MIT can go in (libffi
     is); code under the LGPL v2 can be made GPL (section 3 of `COPYING.LIB`)
     but cannot receive the exception without its holders. A `Signed-off-by`
     line has been enough since June 2021. But libstdc++ holds only what the
     C++ standard defines, and interval arithmetic is not in it: N2137
     (Brönnimann, Melquiond, Pion, 2006) was not adopted. A new proposal to
     WG21, which could build on IEEE 1788-2015, would have to come first.
   - **glibc.** It is not part of GCC: it is the C library of Linux systems,
     whatever the compiler, and its libm computes the `exp` that `std::exp`
     of libstdc++ calls. It is under the LGPL-2.1-or-later and holds only C
     code for what ISO C, POSIX or GNU define, so the C++ classes of GAOL v5
     cannot go in, whatever their licence. The work of GAOL v5 reaches it
     through CORE-MATH (point 4), or through C patches for standard
     functions, sent to libc-alpha with ENSTA's agreement.
   - **References:**
     [Contributing to GCC](https://gcc.gnu.org/contribute.html),
     [glibc copyright assignment policy](https://sourceware.org/pipermail/libc-alpha/2021-July/129577.html),
     [licences of the French administrations](https://www.data.gouv.fr/pages/legal/licences),
     [NEWS of the glibc](https://sourceware.org/git/?p=glibc.git;a=blob;f=NEWS).
