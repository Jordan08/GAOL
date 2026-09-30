<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-20 by Jordan NININ -->
# Using GAOL

Part of the documentation of [GAOL v5](../README.md#documentation).

GAOL's interval operations are inline: the code that includes GAOL's headers
has to be compiled with the flags of interval arithmetic, not only GAOL
itself. With GCC and Clang, each where the compiler takes it:

- `-frounding-math -fno-fast-math -ffp-contract=off`, so that the compiler
  rounds each operation in the direction in effect, as written, and neither
  evaluates it at compile time in the default rounding nor contracts a
  multiplication and an addition into a fused one;
- `-msse2 -mfpmath=sse` on 32-bit x86, so that doubles are computed in double
  precision rather than on the x87 unit;
- `-msse2 -msse3` on x86 processors with the SSE2 intervals (`GAOL_SIMD`);
- `-ffloat-store` where doubles are still computed on the x87 unit;
- `-mfma` on x86, and `-mfpu=neon-vfpv4 -mfloat-abi=hard` on 32-bit ARM, where
  GAOL is compiled with the fused multiply-add instructions of the processor
  (`GAOL_FMA`, see [Building GAOL](building.md)): the code using GAOL runs only
  on a processor with them, as GAOL, and is compiled for it.

With Visual C++, `/fp:strict`, and `/arch:AVX2` for x64 with `GAOL_FMA`. Each build installs them with GAOL, in
`gaol::gaol` and `gaol.pc` (below). `gaol/gaol_config.h` refuses code compiled
by Visual C++ without `/fp:strict`. GCC and Clang do not tell the code whether
`-frounding-math` and `-ffp-contract=off` were given: there, it only refuses
what contradicts them, `-ffast-math`, `-ffinite-math-only` and doubles computed
on the x87 unit (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)).
`-fno-fast-math` turns the first two off when it comes after them on the
command line, and does nothing when it comes before them, where the
compilation stops.

The link of the program takes one option too, where the compiler accepts it
(GCC 13 and later on x86, and its releases 11.4 and 12.4): `-mno-daz-ftz`,
which keeps out of the link the file that sets the flush-to-zero and
denormals-are-zero modes when a program is linked with `-Ofast` (see
[Flush-to-zero and denormals-are-zero](#flush-to-zero-and-denormals-are-zero)).
`gaol::gaol` and `gaol.pc` carry it too, where the compiler that built GAOL
accepts it: the three builds test the compiler for it with a link.

## From CMake

```cmake
find_package(gaol REQUIRED)
target_link_libraries(my_target PRIVATE gaol::gaol)
```

`gaol::gaol` carries the include directory, the flags above, the link option
above for the code built by the compiler that built GAOL, of its version or a
later one (another compiler, as Clang 18, would stop on it), and, for Visual
C++, `__GAOL_PUBLIC__=`, GAOL being a static library. There is no other library
to link: CORE-MATH is compiled into `libgaol` itself. A library whose headers
include GAOL's, as Codac's, links `gaol::gaol` `PUBLIC`, so that its own users
get the flags, and its CMake package finds GAOL again (`find_dependency(gaol)`).
`tests/find_package` is a project using an installed GAOL this way.

A project can also build GAOL for itself, with the options it wants:

```cmake
include(FetchContent)
FetchContent_Declare(gaol GIT_REPOSITORY https://github.com/Jordan08/GAOL.git GIT_TAG master)
FetchContent_MakeAvailable(gaol)
target_link_libraries(my_target PUBLIC gaol::gaol)
```

GAOL is then a target of the project (`gaol::gaol`), built with it, and
`cmake --install` of the project installs it with it, CMake package and
`gaol.pc` included; nothing is downloaded beyond GAOL's sources. `tests/fetch_content` is a project
building GAOL this way.

GAOL, a static library, is compiled as position-independent code
(`-fPIC`), so that `gaol::gaol` can be linked into a shared library, such as
Python bindings. A project that sets `CMAKE_POSITION_INDEPENDENT_CODE`, `ON` or
`OFF`, before `FetchContent_MakeAvailable(gaol)`, or on the command line, is
followed instead.

## From pkg-config

Each build installs `gaol.pc` in the `pkgconfig` directory of its library
directory (`<prefix>/lib/pkgconfig`, or `lib64` or `lib/<multiarch>` rather
than `lib` on the systems whose libraries go there), except the CMake build
with Visual C++. Its `Cflags` carries the flags above with the include
directory, and `Libs` GAOL itself with the C math library, CORE-MATH being
compiled into `libgaol`, and the link option above where the compiler that
built GAOL accepts it. `gaol.pc` is written for that compiler: a program linked
by one that refuses `-mno-daz-ftz` (Clang 18, GCC before 11.4) stops on it, and
is to be linked without it:

```bash
export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig
c++ -std=c++17 -O2 $(pkg-config --cflags gaol) program.cpp $(pkg-config --libs gaol)
```

In a meson project, `dependency('gaol')`.

## Initialization and cleanup

GAOL initializes itself before `main()`, with every compiler: `gaol::init()`
need not be called. Each file including GAOL's headers holds a static object
whose constructor initializes GAOL the first time, as `<iostream>` initializes
the standard streams: GAOL is initialized before the static objects that such
a file defines after its `#include`.

**Every program calls `gaol::cleanup()` right after its last use of GAOL**, at
the latest at the end of `main()`, whatever the compiler:

```cpp
#include <cstdio>
#include <iostream>
#include <gaol/gaol.h>
using namespace gaol;

int main()
{
  // No gaol::init(): GAOL initialized itself before main()
  interval x(1, 2);
  std::cout << pow(x, 3) + sin(x) << std::endl;

  gaol::cleanup();   // always, right after the last use of GAOL
  std::printf("%g\n", 1.0 / 3.0);   // the program's own doubles, to nearest
  return 0;
}
```

Unless GAOL is built with `GAOL_PRESERVE_ROUNDING`, its initialization leaves
the rounding direction upward for the whole program, not only for GAOL's
operations, and `gaol::cleanup()` sets back the one the program started with
(see [The rounding direction](#the-rounding-direction)). GAOL also calls
`gaol::cleanup()` itself, but only when the program ends, once `main()` has
returned and the static objects are destroyed, or when the library is
unloaded: until then, the code after the last use of GAOL (the rest of
`main()`, the destructors of static objects, the functions registered with
`std::atexit()`) computes its doubles rounded upward. Only the first call sets
the direction back: an operation of GAOL after it sets the direction upward
again. What the initialization allocated is freed by the automatic cleanup
only, after the static objects are destroyed, so that the expressions of
`gaol/gaol_expression.h` still alive when the program calls `gaol::cleanup()`
remain valid.

The direction set back is the one GAOL found when it initialized itself. It is
not the one the program started with when an operation of GAOL came first: in
the initialization of a static object of a file that uses GAOL without
including its headers, through a library of the program for instance, or, with
Clang, of an `inline` variable or a static member of a class template, whose
initialization C++ does not order with the rest of the file.

The constants of GAOL (π, the masks of its SSE2 operations) are initialized
when compiling: a static object of the program may compute intervals, before
the files of GAOL are initialized, which with the static library of the CMake
build come after those of the program, except with MinGW-w64.

## The namespaces

GAOL's type `interval` and its functions are in the namespace `gaol_core`. A
program names them through one of two namespaces, which `gaol/gaol.h` declares
and does not open:

- `gaol`, GAOL under its own names;
- `gaol_ieee1788`, GAOL under the names of IEEE 1788-2015 (below).

```cpp
#include <gaol/gaol.h>
using namespace gaol;

interval x(1, 2);
interval y = pow(x, 3) + sin(x);
```

`gaol/gaol.h` no longer opens `gaol` itself, as GAOL 4 did: a program that
relied on it adds `using namespace gaol;`, or names `gaol::interval`.

The two namespaces hold `interval`, and the functions that have the same name
and the same meaning in both (`sin`, `exp`, `sqrt`, `min`...) are the same
functions, those of `gaol_core`. `pow` and `textToInterval` are the ones whose
meaning differs: each namespace has its own, and none is in `gaol_core`, the
namespace where argument-dependent lookup looks for a call `pow(x, y)` on an
interval, so that each namespace finds its own only. A program opens one of the
two namespaces, not both: with both open, `pow(x, y)` is ambiguous, and so is
`textToInterval(s)`, which reads the names of GAOL in `gaol` and those of the
standard in `gaol_ieee1788`.

The `pow` of `gaol` takes the integer power for an integer exponent, a negative
base included: `pow(x, n)` for an `int` or an `unsigned` n, and `pow(x, p)` and
`pow(x, y)` for a double p or a degenerate interval y that is an integer, one
beyond the ints giving [−∞, +∞]; any other exponent takes the pow of IEEE
1788-2015. `pow(e, n)` and `pow(e1, e2)` build the expressions of
`gaol/gaol_expression.h`, computed with the same powers. In `gaol_core`, these
functions are named apart from `pow`: `gaol_pown()`, `gaol_uipow()`,
`gaol_pow_real()`, `gaol_pow_hybrid()`, `gaol_pown_exp()` and
`gaol_pow_exp()`.

## The names of IEEE 1788-2015

GAOL names its operations its own way: the reverse function `coshRev(c, x)` of
the standard is `acosh_rel(c, x)`, `mulRev(b, c, x)` is `div_rel(c, b, x)`,
with its arguments in another order, and `roundTiesToEven(x)` is
`round_ties_to_even(x)`. The namespace `gaol_ieee1788`, which `gaol/gaol.h`
brings along, gives each operation of the standard that GAOL provides the name
and the argument order the standard gives it. It is not in `gaol`, and it holds
the type `interval` as well, so that one line lets a program use GAOL under the
names of the standard:

```cpp
#include <gaol/gaol.h>
using namespace gaol_ieee1788;

interval x = numsToInterval(1, 2);
interval y = mulRev(numsToInterval(2, 2), x);   // x / 2
interval z = sinPi(x) + rootn(y, 3);
bool b = strictLess(x, entire());
```

The functions of GAOL that already have the name and the meaning the standard
gives them (`sin`, `exp`, `sqrt`, `min`...) are the same functions in
`gaol_ieee1788`. Where the standard and GAOL differ, these names follow the
standard:

- `pow(x, y)` is the pow of Table 9.1, defined for x > 0, and for x = 0 when
  y > 0, where the `pow` of `gaol` takes the integer power for an integer
  exponent, a negative base included: `pow([-4, -1], [2])` is [1, 16] in
  `gaol` and the empty set in `gaol_ieee1788`. `pow(x, p)` with a number p is
  `pow(x, [p])`, `pow(x, 2)` included: the power with an integer exponent is
  `pown(x, n)` in the standard, whose n is an integer rather than an interval.
  An integer exponent beyond the ints gives the pow of CORE-MATH at the bounds
  of x, `pow([2, 3], [1e10])` being [DBL_MAX, +∞], where the `pow` of `gaol`
  gives [−∞, +∞]. `pown(e, n)` and `pow(e1, e2)` build expressions computed
  with this `pown` and this `pow`;
- `inf` and `sup` of the empty set are +∞ and −∞, where GAOL's bounds are NaN;
- `isMember(m, x)` is false for an infinite m;
- `textToInterval` reads the names of the functions of the standard, those of
  Tables 9.1 and 10.5 that GAOL provides (`pown([2,5],5)`, `rootn(x,3)`,
  `sinPi(x)`, `logp1(x)`...), `pow` being the pow of Table 9.1, and returns the
  empty set for a string that is no interval, a name of GAOL alone
  (`nth_root`, `cbrt`, `log1p`...) included. `gaol::textToInterval`, which
  replaces the constructor `interval(const char*)` of GAOL 4, reads the names
  of GAOL and throws: as for `pow`, a program calls the one of the namespace
  it opens.

A name of the program's own that one of the standard shadows, a constant `inf`
for instance, is to be qualified: `gaol_ieee1788::inf(x)`. So is `less(x, y)`
next to `using namespace std;`, where it is ambiguous with the class template
`std::less`. Only bare intervals are provided, GAOL having no decorations;
`gaol/gaol_ieee1788.h` lists the operations of the standard GAOL does not
provide.

## A result thrown away

GAOL's functions do not change the interval they are given: they return
another one. `sqrt(x);` alone leaves x as it was, and so do `x.mid();` and
`x.emptyset();`, a static function that returns the empty set. The functions
whose result is all they do carry an attribute that makes the compiler warn
about such a call (GAOL v5): the functions and operators on intervals, their
predicates, the names of `gaol_ieee1788` and the functions building
expressions.

```cpp
sqrt(x);          // warning: the result is thrown away, x is unchanged
x = sqrt(x);      // what was meant
(void)sqrt(x);    // thrown away on purpose: no warning, but with GCC before 7
```

The attribute is `GAOL_NODISCARD`, of `gaol/gaol_config.h`. It is
`[[nodiscard]]` in C++17 and later. A program is compiled in the standard its
project sets, or else in the one of its compiler, which is C++14 for GCC 6 to
10, Clang 6 to 15 and Visual C++. Before C++17, `GAOL_NODISCARD` is
`[[nodiscard]]` too where the compiler takes it there: GCC 7 and later, and
Visual C++ 2019 16.4 and later, whose warning is C4834. Clang, which warns
about `[[nodiscard]]` before C++17 with `-pedantic`, and GCC before 7 take
`__attribute__((warn_unused_result))`, whose warning is `-Wunused-result` (on
by default); older Visual C++ takes `_Check_return_` of `<sal.h>`, which only
the code analysis reports (`/analyze`, warning C6031), not the compiler. A
cast to void (`(void)sqrt(x);`) silences each of them, but the attribute of
GCC before 7: keep the result in a variable there. A program that does not
want the warnings defines `GAOL_NODISCARD` empty before including GAOL
(`-DGAOL_NODISCARD=`). The compound assignments (`x += y`), which do change x,
do not carry it.

## Errors

GAOL reports an error by throwing an exception, of one of the three classes of
`gaol/gaol_exceptions.h`, which `gaol::` names as it names `interval`:

- `input_format_error`: a string that is no interval, given to
  `gaol::textToInterval()` or read by `operator>>`;
- `invalid_action_error`: a function called with an argument it does not take,
  `nb_fp_numbers()` with a NaN, or `nth_root(8, 1.5)` in a string;
- `unavailable_feature_error`: a feature that is not available; no operation
  of GAOL v5 throws it.

They all derive from `gaol_exception`, which derives from `std::exception`, and
their `what()` is the explanation of the error, so that a handler that knows
nothing of GAOL says what went wrong:

```cpp
try {
  interval x = gaol::textToInterval("[1, 2");
} catch (const std::exception& e) {
  std::cerr << e.what() << '\n';   // Syntax error in interval initialization: [1, 2
}
```

An exception that nothing catches ends the program with the same text (with
libstdc++: `terminate called after throwing an instance of
'gaol_core::input_format_error'`, then `what():  Syntax error in interval
initialization: [1, 2`). GAOL 4 wrote `std::exception` in both places, whatever
the error, `gaol_exception` not overriding `what()`. Where an exception has no
explanation, `what()` is `gaol_exception`, never an empty text. A handler of
`gaol_exception` also has `explanation()`, the same text in a `std::string`,
and `file()` and `line()`, where GAOL threw the exception; `std::cerr << e`
writes the three, as `file.cpp, line 12: exception thrown: <explanation>`.

A build without `GAOL_EXCEPTIONS_ENABLED` (`--disable-exceptions` of
`configure`, `-Denable-exception=false` of meson; see
[Building GAOL](building.md)) prints a message and aborts instead of throwing.
The CMake build always throws.

## The rounding direction

Each operation of GAOL sets the rounding direction upward when it is not, and
leaves it upward, whichever way GAOL is built. The bounds are then right
whatever rounding direction the calling code left. The check is an addition,
1 + 2^-60, above 1 only when rounded upward, rather than a reading of the
rounding direction, which cost far more under Rosetta 2 and with 32-bit Visual
C++ (see [What differs from GAOL](differences.md)); on x86 processors, and on
ARM processors with GCC and Clang, it is 1 + (2^-1060 + 0), which shows the
modes that flush the subnormal numbers to zero as well (below). Code that needs
its own rounding direction after GAOL's operations builds GAOL with
`GAOL_PRESERVE_ROUNDING` (`--enable-preserve-rounding`,
`-Denable-preserve-rounding=true`): each operation then also restores the
rounding direction it found, and the modes flushing the subnormals to zero it
cleared, which makes the arithmetic operations several times slower.

Without it, the rounding direction stays upward from the initialization of
GAOL on, for the whole program, and `gaol::cleanup()`, which every program
calls right after its last use of GAOL (see
[Initialization and cleanup](#initialization-and-cleanup)), sets back the one
that the first `gaol::init()` found when GAOL initialized itself: to nearest,
as a program starts, on x86 for the x87 unit and the SSE instructions each, in
the thread calling it. The rest of the floating-point
environment is left as it is, the exception flags raised in particular.

## Flush-to-zero and denormals-are-zero

Processors have modes that flush the subnormal numbers, those below 2^-1022 in
magnitude, to zero. On x86, the flush-to-zero (FTZ) and denormals-are-zero
(DAZ) modes of the SSE instructions, two bits of the control register MXCSR,
make every subnormal result a zero (FTZ) and read every subnormal operand as a
zero (DAZ), in the comparisons too; on ARM, the mode FZ of the control
register (FPCR, FPSCR) does both. With one of them set, the bounds of every
operation with a subnormal result or operand miss the exact result, whatever
the rounding direction: `interval(1e-300) * interval(1e-20)` is [0, 0], and so
are `sqr(interval(1e-160))` and `interval(3e-308) - interval(2.9e-308)`.

They are set by a program linked with `-Ofast`, `-ffast-math` or
`-funsafe-math-optimizations`: GCC and Clang then link `crtfastmath.o`, on x86
and on ARM Linux, whose constructor sets them for the whole program. GCC links
it last, and it runs after GAOL initialized itself; Clang links it first, and
the initialization of a static GAOL, which follows, clears the modes again,
unless GAOL is built with `GAOL_PRESERVE_ROUNDING`, or is a shared library,
initialized before the program. The `-fno-fast-math` of `gaol.pc` and
`gaol::gaol` does not prevent it: it cancels `-ffast-math` when it comes after
it, but not `-Ofast` (GCC 13 and Clang 18 link `crtfastmath.o` all the same,
and the `#error` of `gaol/gaol_config.h` against `-ffast-math`, which `-Ofast`
raises, is silenced) nor, with GCC 13, `-funsafe-math-optimizations`. Loading a
plug-in or a Python module built with `-Ofast` sets the modes too, when it is
loaded, where its compiler linked `crtfastmath.o` into it: Clang 18 does, GCC
did before version 13.

**Linking a program that uses GAOL with `-Ofast`, `-ffast-math` or
`-funsafe-math-optimizations`, or loading code built with them, makes the
bounds wrong**, and GAOL defends itself in two ways:

- On x86 processors, and on ARM processors with GCC and Clang, each operation
  of GAOL that computes bounds clears the modes when it starts, in the check
  that sets the rounding direction upward: the check is one comparison with a
  subnormal number, which shows the modes as well as the direction (see
  [The rounding direction](#the-rounding-direction)), so that the bounds
  computed after the check are the tightest ones, and the modes stay cleared
  for the rest of the program, as the rounding direction stays upward. The
  arithmetic operations, `+`, `-`, `*`, `/` and `%` with an interval or a
  double, make the check before they compare the bounds (GAOL v5). With
  `GAOL_PRESERVE_ROUNDING`, each operation sets the modes back as it found
  them. This is the only defence against a plug-in, which sets the modes after
  GAOL initialized itself, and against the compilers and processors that have
  no `-mno-daz-ftz`. On an Intel Xeon of the Cascade Lake generation, with
  Clang 18, an addition with a subnormal operand and result took as long as one
  of normal numbers (1.2 ns in a chain of dependent additions), as on the
  Intel i7-1185G7 of the review of 2026-09-27, and the check as long as the one
  of the direction alone, 1 + 2^-60, in front of the addition of two SSE2
  intervals. Through the library, `x * y` took 4.0 ns rather than 3.45 ns, and
  as long with the same check made of a normal number (4.2 ns): the cost of its
  second addition there, not of the subnormal; `x + y`, `x / y`, `sqrt`,
  `exp`, `sin` and the other operations measured stayed within the noise of the
  machine (0.3 ns). Some x86 processors take a microcode assist, of the order
  of a hundred cycles, for an operation with a subnormal operand or result:
  each operation would pay it there. Neither they nor the processors of the
  continuous integration (virtual machines, AMD EPYC or Intel Xeon) were
  measured; `tests/performance.cpp`, which it runs, prints the times there.
- `gaol.pc` and `gaol::gaol` give `-mno-daz-ftz` to the link, where the
  compiler accepts it: GCC 13 and later on x86, and its releases 11.4 and 12.4
  (GCC 9.4, GCC for ARM and Clang 18 refuse it, and the builds test the
  compiler with a link, not with its version). It keeps `crtfastmath.o` out of
  the link, so that a program linked with `-Ofast` does not get the modes at
  all, its own code included.

What remains: the functions that read the bounds of an interval or compare them
without computing, or that do so before the check of their operation (the
relations, the constructor from two bounds, `abs()`, `mid()`, `split()`,
`div_rel()`, and the tests of the domain that `sqrt()`, `log()` and the like
make before their check), read a subnormal bound as a zero for as long as a
mode flushing the operands (DAZ, FZ) is set, that is until an operation that
computes has cleared it: `log(interval(1e-310, 1e-309))` and
`sqrt(interval(-1e-310, 4.0))` are then the empty set. The modes of other
processors, and of ARM with Visual C++, are neither checked nor cleared; GCC
links `crtfastmath.o` for none of the other processors GAOL is tested on.
Link a program that uses GAOL without these options, or with `-mno-daz-ftz`,
and compile the code that needs them apart from it.

## The floating-point exceptions

GAOL computes with the floating-point exceptions masked, as a program starts,
and leaves them so: an operation that raises one only sets its flag. **A
program that enables them (`feenableexcept()` of glibc, `_controlfp_s()` of
Visual C++, `fesetenv()` with an environment that traps) disables them while
GAOL computes**, and enables them again afterwards if it wants them: GAOL's
operations raise some, as listed below, and the processor then stops the
program (with SIGFPE on Linux) instead of letting the operation give its
bounds.

- An infinite bound comes from a division by zero or from an overflow:
  `log([0, 1])`, which is `[-oo, 0]`, raises the divide-by-zero exception, and
  so does `1/[0, 1]`, which is `[1, +oo]`, with the SSE2 intervals;
  `[1e308]*10`, which is `[DBL_MAX, +oo]`, raises the overflow exception.
- Almost every operation raises the inexact exception: most bounds are rounded
  results, and where GAOL checks the rounding direction with an addition,
  1 + 2^-60 or 1 + (2^-1060 + 0), whose result is inexact (see
  [The rounding direction](#the-rounding-direction)), even an operation whose
  bounds are exact raises it. On x86, the addition of the subnormal 2^-1060
  also raises the denormal-operand flag of the SSE instructions, which is none
  of the five flags of IEEE 754 (not in `FE_ALL_EXCEPT`), and its exception
  stops the program where the program enabled it (GAOL v5).
- An empty operand raises no invalid-operation exception, though the bounds
  of the empty interval are NaN (GAOL v5): every operation, relation, function
  and output of `gaol::interval` and of `gaol_ieee1788` takes it without
  raising that exception. `is_empty()` and the relations compare the bounds
  with the quiet comparisons of `<cmath>` (`std::islessequal()`...), each one
  instruction, as `<=` is, with GCC and Clang on x86-64, and the functions
  that build their result from the bounds of their operand tell the empty set
  before they give them to the constructor, which compares its bounds. With
  the invalid-operation exception enabled, `is_empty()` of an empty interval
  killed the program, and so did about 50 operations with an empty operand,
  which compared its NaN bounds: `x & y` for an empty `y`, `sqrt`, `exp`,
  `min`, `max`, `floor`, `set_contains()`, `set_disjoint()`, the output of the
  empty set...; and `interval::emptyset()` itself in a build without
  optimization. A NaN the program gives GAOL still raises it: `interval(NAN)`
  and `interval(NAN, 1)`, which give the empty set, compare it. An empty
  operand may still raise the other exceptions: the sum of two empty
  intervals raises the inexact one where GAOL checks the rounding direction
  with an addition, and `atanh_rel([-1, 2], x)` for an empty `x` raises the
  divide-by-zero one while it computes `atanh([-1, 2])`.
- Nonempty operands raise the invalid-operation exception as well. With the
  SSE2 intervals, multiplying a zero bound by an infinite one does:
  `[0]*[1, +oo]` and `[0, +oo]*[0]` (the FPU intervals give the same product
  without it, and so does a build with `GAOL_PRESERVE_ROUNDING`, whose product
  masks the exceptions, see below). `pow` does in every build, through
  CORE-MATH's `pow`, for an exponent of extreme magnitude (`pow([1, 2],
  [4.9e-324])`, `pow([1, 2], [1e-300])`, an exponent interval with a bound near
  `DBL_MAX` such as `[1e300, 1e308]`); with the SSE2 intervals, an exponent with
  an infinite bound also does, through that product (`pow([1], [1, +oo])`).
  This list is not exhaustive.

An empty operand raising no invalid-operation exception therefore does not
make that exception safe to leave enabled: every exception stays disabled
while GAOL computes.

The flags tell nothing of the results: after an operation of GAOL,
`fetestexcept(FE_INEXACT)` is raised whatever the result, and `FE_OVERFLOW` or
`FE_DIVBYZERO` stands for an infinite bound of the interval, not for an
infinite number the program computed. IEEE 1788-2015 leaves the flags of its
operations unspecified. A program that reads them for its own computations
clears them (`std::feclearexcept(FE_ALL_EXCEPT)`) just before, and reads them
before its next operation of GAOL.

GAOL's initialization, which runs before `main()`, sets the default
environment (`fesetenv(FE_DFL_ENV)`), which masks every exception, unless GAOL
is built with `GAOL_PRESERVE_ROUNDING`: the initialization then leaves the
whole environment as it found it, and an exception a static object enabled
before it stays enabled. A program enables the exceptions in `main()` or later,
whichever the build, not in a static object initialized before GAOL.
`gaol::cleanup()` leaves the exceptions and the flags as they are. With
`GAOL_PRESERVE_ROUNDING` and the SSE2 intervals, `+`, `-`, `*`, `/`, `sqr()`
and `inverse()` also write the SSE control register with every exception
masked, which masks again the ones a program enabled: they stop nothing after
the first of these operations. Disable them all the same, the other builds not
masking them.
