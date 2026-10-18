<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->

# GAOL
<em>Not Just Another Interval Library</em>

[![Linux](https://github.com/Jordan08/GAOL/actions/workflows/linux.yml/badge.svg?branch=configure-clean)](https://github.com/Jordan08/GAOL/actions/workflows/linux.yml?query=branch%3Aconfigure-clean)
[![macOS](https://github.com/Jordan08/GAOL/actions/workflows/macos.yml/badge.svg?branch=configure-clean)](https://github.com/Jordan08/GAOL/actions/workflows/macos.yml?query=branch%3Aconfigure-clean)
[![Windows](https://github.com/Jordan08/GAOL/actions/workflows/windows.yml/badge.svg?branch=configure-clean)](https://github.com/Jordan08/GAOL/actions/workflows/windows.yml?query=branch%3Aconfigure-clean)
[![Linux containers](https://github.com/Jordan08/GAOL/actions/workflows/containers.yml/badge.svg?branch=configure-clean)](https://github.com/Jordan08/GAOL/actions/workflows/containers.yml?query=branch%3Aconfigure-clean)
[![Autotools and meson](https://github.com/Jordan08/GAOL/actions/workflows/build-systems.yml/badge.svg?branch=configure-clean)](https://github.com/Jordan08/GAOL/actions/workflows/build-systems.yml?query=branch%3Aconfigure-clean)
[![Manual](https://github.com/Jordan08/GAOL/actions/workflows/manual.yml/badge.svg?branch=configure-clean)](https://github.com/Jordan08/GAOL/actions/workflows/manual.yml?query=branch%3Aconfigure-clean)

GAOL is a C++ [Interval Arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic) library that strives to offer fast and reliable operators for constraint solvers. 

## Authors

GAOL v5 was written by Jordan Ninin, Associate Professor in Computer Science at
ENSTA (Lab-STICC, UMR CNRS 6285), who has been its main developer since 2026.
Its page is the repository [Jordan08/GAOL](https://github.com/Jordan08/GAOL),
and Jordan Ninin's page, [webperso.ensta.fr/ninin](https://webperso.ensta.fr/ninin/),
lists his publications.

GAOL v5 continues GAOL as
[Frédéric Goualard](https://frederic.goualard.net/) developed it, from its
version 4.2.2 (see [What differs from GAOL](doc/differences.md)).

GAOL was written by Frédéric Goualard, Associate Professor in Computing
Science at Nantes Université (LS2N, UMR CNRS 6004), who has been its main
developer since 2001. Its page is the
[GAOL section](https://frederic.goualard.net/#research-software-gaol) of
Frédéric Goualard's site, and its original repository is
[goualard-f/GAOL](https://github.com/goualard-f/GAOL). His publications,
those related to GAOL among them, are in the
[full list](https://frederic.goualard.net/#Publications) of his site.

## GAOL v5

GAOL v5, this version of [GAOL](https://github.com/goualard-f/GAOL), adds a CMake build,
tests of the bounds GAOL computes, and the changes GAOL needs to compile and
compute right on every system it can: Linux, macOS and Windows, on x86, x86_64,
ARM, arm64 and the other processors of Debian. It was written for
[Codac](https://github.com/codac-team/codac), whose intervals are built upon
GAOL. The autotools and meson builds of GAOL are kept, and the three builds
configure GAOL the same way (see [The three builds](doc/three-builds.md)).

GAOL v5 follows [IEEE 1788-2015](https://doi.org/10.1109/IEEESTD.2015.7140721),
the standard for interval arithmetic, in the operations GAOL provides: the
empty set, infinite bounds, the domains of the functions, the comparisons,
the interval literals, and the accuracy of each operation, which
[Accuracy of the operations](doc/accuracy.md) documents.

### Two namespaces: `gaol` and `gaol_ieee1788`

A program uses GAOL v5 through one of two namespaces, which hold the same
`interval` and compute with the same functions:

- `gaol`, GAOL under its own names, to stay compatible with GAOL 4: a program
  written for GAOL 4, or for IBEX and Codac, keeps its names and, but for the
  [results that change](doc/differences.md), its results;
- `gaol_ieee1788`, GAOL under the names of IEEE 1788-2015 (`numsToInterval`,
  `mulRev`, `sinPi`, `pown`...), to follow the standard.

The main difference between the two is the power. The `pow` of
`gaol_ieee1788` is the real power of the standard, e^(y log x), defined for
x > 0 only (and x = 0 when y > 0), and its `pown(x, n)` the integer power;
the `pow` of `gaol` is GAOL's, which takes the integer power for an integer
exponent, a negative base included, and the real power otherwise:
`pow([-4, -1], 2)` is [1, 16] in `gaol` and the empty set in
`gaol_ieee1788`, where `pown([-4, -1], 2)` is [1, 16]. The other few cases
where `gaol` differs from the standard are listed in
[the special cases](doc/compare/special_cases.md#what-the-cases-show), and
[Using GAOL](doc/using.md#the-powers-in-the-two-namespaces) gives the powers of
both namespaces side by side.

```cpp
#include <gaol/gaol.h>

gaol::interval x(-4, -1);
gaol::interval a = gaol::pow(x, 2);           // [1, 16]
gaol::interval b = gaol_ieee1788::pow(x, 2);  // the empty set
gaol::interval c = gaol_ieee1788::pown(x, 2); // [1, 16]
```

## Quick start

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix> -DWITH_TESTS=ON
cmake --build build --config Release
cmake --build build --target test
cmake --install build --config Release
```

Each build compiles the sources of
[CORE-MATH](https://core-math.gitlabpages.inria.fr/), the mathematical library
GAOL computes its elementary functions with (`3rd/math-core`, see
[3rd/README.md](3rd/README.md)), into `libgaol` itself: there is no
mathematical library to install or to link along with GAOL. A CMake project
then uses the installed GAOL with:

```cmake
find_package(gaol REQUIRED)
target_link_libraries(my_target PRIVATE gaol::gaol)
```

`gaol::gaol`, and `gaol.pc` for pkg-config, carry the flags of interval
arithmetic that the code including GAOL's headers has to be compiled with. A
project can also build GAOL for itself, with FetchContent (see
[Using GAOL](doc/using.md#from-cmake)), or as a meson subproject (see
[Using GAOL](doc/using.md#from-pkg-config)).

A program calls `gaol::cleanup()` right after its last use of GAOL:
GAOL's initialization, which runs by itself before `main()`, sets the
rounding direction upward for the whole program (unless GAOL is built with
`GAOL_PRESERVE_ROUNDING`), and `gaol::cleanup()` sets back the one the
program started with (see
[Using GAOL](doc/using.md#initialization-and-cleanup)).

## A first program

Once GAOL is installed, this program encloses the values of x (1 − x) for x in
[0.4, 0.6] (`examples/tutorial.md` goes on from it):

```cpp
#include <iostream>
#include <gaol/gaol.h>
using namespace gaol;

int main()
{
  const interval x = textToInterval("[0.4, 0.6]");   // the decimals 0.4 and 0.6, enclosed
  std::cout << x * (1.0 - x) << '\n';       // [0.1599999999999999, 0.3600000000000002]
  std::cout << 0.25 - sqr(x - 0.5) << '\n';  // [0.2399999999999999, 0.25]
  gaol::cleanup();                            // right after the last use of GAOL
}
```

```bash
c++ -std=c++11 -O2 $(pkg-config --cflags gaol) first.cpp -o first $(pkg-config --libs gaol)
```

Each value printed is an interval that contains the exact one, whatever the
rounding direction the program is in. The first, x written twice, is 20 times wider than the range of the
function, [0.24, 0.25], because each occurrence of x ranges over [0.4, 0.6] on its
own (the dependency problem); the second, the same function with x written
once, encloses the range itself. The bounds are written rounded outward, so
that the text encloses the interval: `[0.2399999999999999, 0.25]` encloses
[0.24, 0.25]. The
[tutorial](examples/tutorial.md) writes four algorithms of interval computing
with GAOL: the range of a function by subdivision, the interval Newton method,
a contractor and a branch and bound; the sixteen programs of
[`examples/`](examples/examples.md#12-the-programs), which check what they
print, go further, and [Using GAOL](doc/using.md) gives the pitfalls and the
names of IBEX, Codac, C-XSC and Boost.Interval next to GAOL's.

## Documentation

- [A short tutorial](examples/tutorial.md): four algorithms of interval
  computing written with GAOL, and [the examples](examples/examples.md) they
  come from: sixteen programs that use intervals as IBEX, Codac, C-XSC and the
  textbooks do, a review of how intervals are used with GAOL v5, and a comparison
  with other libraries.
- [Building GAOL](doc/building.md): the CMake, autotools and meson builds, and
  their options.
- [Using GAOL](doc/using.md): the flags of interval arithmetic, GAOL from CMake
  and from pkg-config, the call of `gaol::cleanup()` at the end of a program,
  the rounding direction, the names of other libraries next to GAOL's, and the
  common errors.
- [The three builds](doc/three-builds.md): what CMake, configure and meson
  agree on, and the compilers and options they refuse.
- [Tests](doc/tests.md): what the programs of `tests/` check, and what they
  show of GAOL.
- [Accuracy of the operations](doc/accuracy.md): the tightness of each
  operation, from its algorithm, as IEEE 1788-2015 requires it to be
  documented (12.10.3).
- [What differs from GAOL](doc/differences.md): each change GAOL v5 brings, and
  where it comes from.
- [Continuous integration](doc/continuous-integration.md): the systems,
  processors and compilers GAOL is built and tested on.
- [Comparison with libieeep1788, filib++, PROFIL/BIAS, Solaris Studio and Boost.Interval](doc/compare/README.md):
  the [special cases](doc/compare/special_cases.md) of the six libraries
  against IEEE 1788-2015, the [enclosure](doc/compare/enclosure.md) of the
  elementary functions of GAOL and Boost.Interval, and their
  [performance](doc/compare/performance.md), with the scripts to run the
  comparison again.
- The manual of GAOL v5: `manual/v5/gaol.pdf`, with its LaTeX sources in
  `manual/v5/`. It follows the manual of GAOL 4 by Frédéric Goualard, and
  describes GAOL v5: its installation, each operation with examples, the names
  of IEEE 1788-2015, the accuracy of the operations, and what a program written
  for GAOL 4 has to change.
- The manual of GAOL 4, by Frédéric Goualard: `manual/v4/gaol.pdf`, with its
  LaTeX sources in `manual/v4/`, kept as it was. It still describes the
  mathematical libraries and the options of GAOL 4.
- `make -C manual pdf` after `./configure`, or `meson compile -C <build> pdf`
  after `meson setup <build> -Dwith-doc=true`, builds both manuals again
  (`manual/build-pdf.sh`).

## Licences

GAOL, by [Frédéric Goualard](https://frederic.goualard.net/), is distributed
under the GNU LGPL v2 (`COPYING.LIB`).
[CORE-MATH](https://core-math.gitlabpages.inria.fr/), whose sources are in
`3rd/math-core` and provide every elementary function of GAOL, is distributed
under the MIT licence (`3rd/math-core/LICENSE`).

The files GAOL v5 adds, which are neither GAOL's nor CORE-MATH's, carry the
copyright of ENSTA and are distributed under the same GNU LGPL v2 as GAOL:

    Copyright (c) 2026 ENSTA, France
    Created 2026-09-20 by Jordan NININ

Each of them names it in its own header. The files of GAOL and of CORE-MATH
that GAOL v5 changes keep the copyright of their authors.
