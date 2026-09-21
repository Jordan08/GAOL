<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->

# GAOL
<em>Not Just Another Interval Library</em>

GAOL is a C++ [Interval Arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic) library that strives to offer fast and reliable operators for constraint solvers. 

## Authors

GAOL v5 is written by Jordan Ninin (ENSTA). It continues GAOL as
[Frédéric Goualard](https://frederic.goualard.net/) developed it, from its
version 4.2.2 (see [What differs from GAOL](doc/differences.md)).

GAOL was written by Frédéric Goualard, Associate Professor in Computing
Science at Nantes Université (LS2N, UMR CNRS 6004), who has been its main
developer since 2001. Its page is the
[GAOL section](https://frederic.goualard.net/#research-software-gaol) of
Frédéric Goualard's site, and its original repository is
[goualard-f/GAOL](https://github.com/goualard-f/GAOL).

Publications of Frédéric Goualard related to GAOL (see the
[full list](https://frederic.goualard.net/#Publications)):

- [Fast and Correct SIMD Algorithms for Interval Arithmetic](https://frederic.goualard.net/publications/interval-sse2_goualard_para08.pdf).
  PARA '08, Lecture Notes in Computer Science 6126–6127, Springer, 2012.
- [Interval Extensions of Multivalued Inverse Functions](https://hal.archives-ouvertes.fr/hal-00288457v1).
  Research report hal-00288457, 2008.
- [How do you compute the midpoint of an interval?](https://hal.archives-ouvertes.fr/hal-00576641v2).
  ACM Transactions on Mathematical Software 40(2), 2014.

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
[Accuracy of the operations](doc/accuracy.md) documents. The few cases where
it differs, mostly the `pow(x, y)` of the namespace `gaol`, which takes the
integer power `pown` for an integer exponent, are listed in
[the special cases](doc/compare/special_cases.md#what-the-cases-show); the
namespace `gaol_ieee1788` gives the operations under the names of the standard,
its `pow` being the standard's (see [Using GAOL](doc/using.md#the-namespaces)).

## Quick start

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=<prefix> -DGAOL_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release
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
[Using GAOL](doc/using.md#from-cmake)).

## Documentation

- [Building GAOL](doc/building.md): the CMake, autotools and meson builds, and
  their options.
- [Using GAOL](doc/using.md): the flags of interval arithmetic, GAOL from CMake
  and from pkg-config, and the rounding direction.
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
- [Comparison with libieeep1788, filib++, PROFIL/BIAS and Solaris Studio](doc/compare/README.md):
  the [special cases](doc/compare/special_cases.md) of the five libraries
  against IEEE 1788-2015, and their [performance](doc/compare/performance.md),
  with the scripts to run the comparison again.
- The manual of GAOL, by Frédéric Goualard: `manual/gaol.pdf`, with its LaTeX
  sources in `manual/`, which follow the changes of GAOL v5. `make -C manual
  pdf` after `./configure`, or `meson compile -C <build> pdf` after `meson setup
  <build> -Dwith-doc=true`, builds it again (`manual/build-pdf.sh`).

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
