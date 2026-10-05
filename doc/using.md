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

With Visual C++ and clang-cl, `/fp:strict`, `/arch:AVX2` for x64 with
`GAOL_FMA`, and `-msse2` with clang-cl for 32-bit x86, which computes doubles
on the x87 unit with `/arch:SSE2`. Each build installs them with GAOL, in
`gaol::gaol` and `gaol.pc` (below). `gaol/gaol_config.h` refuses code compiled
by Visual C++, or by clang-cl from Clang 16, without `/fp:strict`. GCC and Clang do not tell the code whether
`-frounding-math` and `-ffp-contract=off` were given: there, it only refuses
what contradicts them, `-ffast-math`, `-ffinite-math-only` and doubles computed
on the x87 unit (see
[Compilers and options refused](three-builds.md#compilers-and-options-refused)).
`-fno-fast-math` turns the first two off when it comes after them on the
command line, and does nothing when it comes before them, where the
compilation stops.

With `-frounding-math` on x86, GCC 12.1 to 12.3 and 13.1 to 13.2 (Debian 12
has GCC 12.2) initialize wrongly a double member of a static array of
structures given `std::numeric_limits<double>::denorm_min()`: they store the
long double 2^-1074 there, whose first eight bytes read -0 and whose last two
overwrite the next member (`{ denorm_min(), DBL_MAX }` read `[-0, NaN]`, the
empty set, on 32-bit x86). A double or an array of doubles initialized so is
right, and so are GCC 12.4, 13.3 and later. A program built with these
releases computes such a value at run time, as `tests/rounding_direction.cpp`
does (GAOL v5).

The link of the program takes one option too, where the compiler accepts it
(GCC 13 and later on x86, and from 11.4 and 12.4 in the series 11 and 12):
`-mno-daz-ftz`,
which keeps out of the link the file that sets the flush-to-zero and
denormals-are-zero modes when a program is linked with `-Ofast` (see
[Flush-to-zero and denormals-are-zero](#flush-to-zero-and-denormals-are-zero)).
`gaol::gaol` and `gaol.pc` carry it too, where the compiler that built GAOL
accepts it: the three builds test the compiler for it with a link.

## The headers

A program includes `gaol/gaol` (or `gaol/gaol.h`), and `gaol/gaol_expression.h`
for the expressions, with `gaol/gaol_expr_eval.h` to evaluate them, and
`gaol/gaol_assert.h` for `GAOL_ASSERT`. The three builds install these and the
headers they include, each of which compiles alone but the three that other
headers include in the middle of their code (`gaol_interval_fpu.h`,
`gaol_interval_sse.h`) or for Visual C++ only (`gaol_fpu_msvc.h`), and not the
headers of GAOL's sources (GAOL v5): `gaol_interval_parser.h`, which did not
compile when included, `gaol_init_cleanup.h`, `gaol_exact.h` and `sysdeps/`,
`gaol_core_math.h` and `gaol_u128.h`; `gaol_parameters.h`, which declared
nothing, is gone. GAOL's headers compile without a warning under
`-Wall -Wextra`, and put nothing into the global namespace:
`gaol/gaol_exceptions.h` declared `string` and `exception` there, which a
program now names `std::string` and `std::exception` (GAOL v5). The macros they
define start with `GAOL_` (or `gaol_`), so as not to meet those of the program
or of another library (GAOL v5): `USING_SSE2_INSTRUCTIONS`, `HAVE_FENV_H`,
`WORDS_BIGENDIAN`, `INLINE`, `MEMALIGN`, `__HI`... are now
`GAOL_USING_SSE2_INSTRUCTIONS`, `GAOL_HAVE_FENV_H`, `GAOL_WORDS_BIGENDIAN`,
`GAOL_INLINE`, `GAOL_MEMALIGN`, `GAOL_HI`..., `<stdlib.h>` is no longer included
with `_XOPEN_SOURCE` defined again, and `nb_fp_numbers()` returns an
`unsigned long long`, which GAOL 4 named with the macro `ULONGLONGINT`. The
doubles `pi`, `half_pi`, `two_pi`, `pi_dn`, `pi_up`, `half_pi_dn`, `half_pi_up`,
`ln2_dn`, `ln2_up`, `two_power_51` and `two_power_52` of GAOL 4 are no longer
declared for the program, which found them with `using namespace gaol`, its own
`pi` being ambiguous (GAOL v5): the bounds of π are those of `interval::pi()`.

## From CMake

```cmake
find_package(gaol REQUIRED)
target_link_libraries(my_target PRIVATE gaol::gaol)
```

`gaol::gaol` carries the include directory, the flags above, the link option
above for the code built by GCC 13 or a later one, or by the compiler that
built GAOL, of its major version and not older (another compiler, as Clang 18,
would stop on it, and so would GCC 12.0 to 12.3 with a GAOL built by GCC 11.4:
GCC has the option from 11.4 and 12.4 in the series 11 and 12 only), and, for
Visual C++, `__GAOL_PUBLIC__=`, GAOL being a static library. There is no other library
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
directory (`<prefix>/lib/pkgconfig`, or `lib64` or `lib/<multiarch>` rather than
`lib` on the systems whose libraries go there), except the CMake build with
Visual C++. Its `Cflags` carries the flags above with the include directory, and
`Libs` GAOL itself, CORE-MATH being compiled into `libgaol`, and the link option
above where the compiler that built GAOL accepts it. It names no math library
(GAOL v5): GAOL still calls functions of the math library of the system (those
of `<fenv.h>`, `sqrt()`, `floor()`, `fma()` where the processor has no such
instruction...), which the C++ compiler links itself. `gaol.pc` is written for
that compiler: a program linked by one that refuses `-mno-daz-ftz` (Clang 18, a
GCC before 11.4, GCC 12.0 to 12.3) stops on it, and is to be linked without it:

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
again, and **`gaol::restore_rounding()` sets it back as many times as the
program needs** (GAOL v5). What the initialization allocated is freed by the
automatic cleanup only, after the static objects are destroyed, so that the
expressions of `gaol/gaol_expression.h` still alive when the program calls
`gaol::cleanup()` remain valid.

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
base included: `pow(x, n)` for an n of any integer type (see
[Integers](#integers)), and `pow(x, p)` and `pow(x, y)` for a double p or a
degenerate interval y that is an integer, one beyond the ints giving
[−∞, +∞]; any other exponent takes the pow of IEEE 1788-2015. `pow(e, n)` and `pow(e1, e2)` build the expressions of
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
  with this `pown` and this `pow`, declared with the other expressions by
  `gaol/gaol_expression.h`, which `gaol/gaol` does not include, as with GAOL
  4. A double converts neither to an interval nor to an expression, their
  constructors from a double (and that of an expression from an interval)
  being explicit: `gaol::sin(0.5)` does not compile, with or without
  `gaol/gaol_expression.h`, and is written `gaol::sin(interval(0.5))`;
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

## Integers

An integer given to GAOL is taken as the integer it is (GAOL v5):
`interval(n)` is the tightest interval containing n, [n, n] where n is a
double, as every `int` is, and the two doubles around it otherwise.
`interval(9007199254740993LL)`, 2^53 + 1, is [2^53, 2^53 + 2], where its
conversion to a double gave [2^53, 2^53] or [2^53 + 2, 2^53 + 2], according
to the rounding direction, neither of which contains it, and
`interval(ULLONG_MAX)` contains 2^64 − 1. So do `interval(a, b)` with an
integer bound, which is empty for a > b compared as numbers
(`interval(9007199254740993LL, 9007199254740992LL)` is empty, though both lie
between the same two doubles), the assignment `x = n` and `x |= n`, the
arithmetic operators (`x + n`, `n * x`, `x += n`...), `set_contains(n)`,
`expression(n)`, and `numsToInterval()` and `isMember()` in `gaol_ieee1788`;
`x &= n` is `interval(n)` where x contains n, and the empty set otherwise. The
relations and `set_strictly_contains(n)` compare n as the integer it is:
`interval(9007199254740992.0) < 9007199254740993LL`, [2^53] < 2^53 + 1, is
true, though no double lies between them. Every integer type is taken so,
`bool` aside, whatever the rounding direction (an unscoped enumeration, and
`__int128` outside the GNU modes of GCC and Clang, still convert to a
double, and a `long double` bound next to an integer takes
`interval(double, double)`). An integer that is a double gives the result the
double gives, and for the types of 53 bits or less (`int`, `short`, `char`...)
the same code. `interval(n)` and `expression(n)` are explicit, as their
constructors from a double; `expression(n)` is `expression(double)` where n is
a double.

`pow(x, n)` of `gaol` and `pown(x, n)` of `gaol_ieee1788` take an exponent of
any integer type: `pow(x, 5L)` did not compile, a `long` being as far from an
`int` as from an `unsigned` and a `double`, and `pown(x, 3000000000u)`
converted the exponent to a negative `int`. An exponent within the ints is
taken as an `int`, one within the unsigned ints as an `unsigned`, and one
beyond them gives [−∞, +∞] for a nonempty x, as `gaol::pow(x, [n])` does.
So do `nth_root(x, q)` and `rootn(x, q)`, whose order beyond the ints and the
unsigned ints gives an enclosure of the roots, computed by `pow` with the
exponent 1/q (x^(1/q) is within 2^−21 of 1 for every double x > 0 there),
`pownRev(c, x, p)`, which gives the hull of the parts of x in these roots and,
for an even p, in their opposites, and `pow(e, n)`, `pown(e, n)` and
`nth_root(e, n)` of the expressions. `nth_root(x, 3L)` was ambiguous;
`rootn(x, 3000000000u)` and `pownRev(c, 3000000000u)` converted the order to a
negative `int`, the first giving a wrong interval ([0.99999999839,
0.99999999946] for [2, 8], rather than [1.00000000023, 1.00000000069]) and the
second throwing; `rootn(x, 5000000001LL)` and the powers of expressions cut a
`long` to an `int`; and `nth_root(e, -2)` took the order 4294967294. `pow(e, n)`
and `pown(e, n)` of the expressions give [−∞, +∞] for an n beyond the ints,
where the powers of intervals take the unsigned power up to the unsigned ints,
which no node of an expression computes; beyond the unsigned ints,
`nth_root(e, n)` is the root of the interval but where e contains 0, where it
is wider.

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
leaves it upward, whichever way GAOL is built: the operations of the AVX-512
path of `GAOL_PREFER_AVX512` below are the exception, which set none. The
bounds are then right whatever rounding direction the calling code left. The check is an addition,
1 + 2^-60, above 1 only when rounded upward, rather than a reading of the
rounding direction, which cost far more under Rosetta 2 and with 32-bit Visual
C++ (see [What differs from GAOL](differences.md)); on x86 processors, and on
ARM processors with GCC and Clang, it is 1 + (2^-1060 + 0), which shows the
modes that flush the subnormal numbers to zero as well (below). Code that needs
its own rounding direction after GAOL's operations builds GAOL with
`GAOL_PRESERVE_ROUNDING` (`--enable-preserve-rounding`,
`-Denable-preserve-rounding=true`): each operation then also restores the
rounding direction it found, and the modes flushing the subnormals to zero it
cleared, which makes the arithmetic operations several times slower (see
[What the upward rounding does to the program](#what-the-upward-rounding-does-to-the-program)).

Without it, the rounding direction stays upward from the initialization of
GAOL on, for the whole program, and `gaol::cleanup()`, which every program
calls right after its last use of GAOL (see
[Initialization and cleanup](#initialization-and-cleanup)), sets back the one
that the first `gaol::init()` found when GAOL initialized itself: to nearest,
as a program starts, on x86 for the x87 unit and the SSE instructions each, in
the thread calling it. The rest of the floating-point
environment is left as it is, the exception flags raised in particular.

### What the upward rounding does to the program

The rounding direction is the processor's, and the whole program shares it.
GAOL's results do not depend on it, each operation setting it upward when it is
not, in every thread, but the doubles the program computes do. Unless GAOL is
built with `GAOL_PRESERVE_ROUNDING` — or with `GAOL_PREFER_AVX512` on a
processor that has the AVX-512 instructions, whose arithmetic sets no
direction (below) — they are rounded upward from GAOL's initialization,
before `main()`, until `gaol::cleanup()`, and again after each operation of
GAOL that follows it. The compiler does not undo it: with the
flags of `gaol::gaol`, `-frounding-math` in particular, an inexact operation on
constants written in a function, `1.0 / 3.0`, is not evaluated when compiling,
to nearest, but when the program runs, in the direction in effect. Only what
the compiler must evaluate when compiling, a `constexpr` or the constant
initializer of a static variable, is still rounded to nearest.

What the program computes, with GCC 9.4 or Clang 18 and the GNU C library 2.31
on x86-64
([examples/13_rounding_environment.cpp](../examples/13_rounding_environment.cpp)
computes each row and checks it):

| The program computes | While GAOL is initialized | After `gaol::cleanup()` |
|---|---|---|
| `1.0 / 3.0` | 0.33333333333333337 | 0.33333333333333331 |
| `std::stod("0.3") == 0.3` | false | true |
| `std::lrint(2.3)` | 3 | 2 |
| `std::printf("%.2f", 2.675)` | 2.68 | 2.67 |
| `std::cout << std::setprecision(3) << 3.14159265358979` | 3.15 | 3.14 |
| 0.1 added 10⁷ times | 1000000.0005483569 | 999999.99983897537 |
| 20 000 doubles written with `%.17g`, read back by `std::strtod` | 20 000 come back different | none |
| `TwoSum(1e20, 1e-20)`, the error of a sum | s + e ≠ a + b | s + e = a + b |
| `TwoSum(a, b)`, 50 000 pairs of doubles of exponents from −60 to 60 | s + e ≠ a + b for 10 717 pairs | none |
| `TwoProd(a, b)`, Dekker's splitting, 50 000 pairs of doubles of [1, 2) | p + e ≠ a·b for 1 888 pairs | none |
| `TwoProd(a, b)`, `e = std::fma(a, b, -a*b)`, the same pairs | none | none |

The quotient is the double above 1/3, where the program expects the nearest.
The C library rounds in the direction in effect too, as the C standard
recommends: with the GNU C library, `std::rint(-2.7)` is −2 where it is −3 to
nearest, and `strtod()`, `stod()`, `printf()` and the streams round the decimal
digits they read and write up. A double written with 17 digits, which reads
back to itself to nearest, then comes back one double above, and one more at
each round trip. Another C library may convert to nearest whatever the
direction: the example reports what the one it runs with does. Sums are
biased: each addition rounds up, so 0.1 added ten million times ends 5.5·10⁻⁴
above the exact 10⁶ × 0.1, where to nearest it ends 1.6·10⁻⁴ below. And
TwoSum, the algorithm that recovers the error of a sum,

```cpp
s = a + b;  bb = s - a;  e = (a - (s - bb)) + (b - bb);
```

gives s + e = a + b only to nearest. Rounded upward, the exact error of
10²⁰ + 10⁻²⁰, 10⁻²⁰ − 16384, is no double: s is 10²⁰ + 16384 and e is
−16383.999999999998, whose sum is not 10²⁰ + 10⁻²⁰. TwoProd, which recovers
the error of a product from Dekker's splitting of the operands, breaks the
same way (1 888 of the 50 000 pairs of the table, drawn in [1, 2), lose their
exactness upward; pairs of very different magnitudes keep it, the error of
the product of the split parts they drop staying below the error of the
rounded product), and is exact in every direction when it is computed with
`std::fma` instead: the error of a rounded product is a double, which
`std::fma` computes however it rounds. Compensated sums, and the
double-double arithmetic built on such algorithms, lose their accuracy the
same way. Built with `GAOL_PRESERVE_ROUNDING`, the program computes the right-hand
column all the time.

Other consequences:

- a thread created after GAOL's initialization starts upward too (`std::thread`
  gives a new thread the floating-point environment of the thread constructing
  it), and `gaol::cleanup()` sets back the direction of the calling thread only;
- the libraries the program calls run upward as well: the `exp`, `log` and
  `pow` of the GNU C library 2.31 go from 0.51 ulp to 1.01 ulp from the exact
  value, its `cbrt` from 2.8 to 3.8 ulp (20 000 random arguments each, against
  mpmath), and its `sin`, `cos`, `tan` and `atan` give the same results;
- GCC does not model the rounding direction, even with `-frounding-math`
  ([GCC bug 34678](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=34678)), and
  reuses after `gaol::cleanup()` a double computed before it, still rounded
  upward, where the program computes it again: from `-O1`, GCC 9.4 does it for
  `a / b` written on both sides of the call, where Clang 18 computes the
  quotient again. A program cannot rely on either.

What to do:

- **Call `gaol::cleanup()` right after the last use of GAOL**, before the
  program prints, converts or sums what it computed: its doubles are then
  computed to nearest. A program that goes back to GAOL afterwards calls
  `gaol::restore_rounding()` (or `std::fesetround(FE_TONEAREST)`) after each
  use, since an operation of GAOL sets the direction upward again and
  `gaol::cleanup()` sets it back once only.
- **Read and write numbers through GAOL**, not through the C library:
  `gaol::textToInterval("0.3")` encloses 3/10 in every direction, where
  `std::stod("0.3")` does not, and `exact_string()` and `%a` write a number that
  reads back to the same bits in every direction. GAOL writes its intervals
  rounded outward whatever the direction.
- **Compute in a block the doubles that need rounding to nearest** (an
  error-free transformation, `std::lrint()`, a decimal conversion, a call of a
  library written for that): save the direction with `std::fegetround()`, set
  `FE_TONEAREST`, and set the saved one back when the block ends, with no
  operation of GAOL inside, which would set the direction upward again. The
  example writes such a guard, `nearest_scope`.
- **Do not count on the compiler** to compute a double where the source code
  does. One that has to be computed on this side of a change of direction is
  computed from operands read again through `gaol::rnd_keep()`, which writes its
  argument to volatile memory and returns it: `gaol::rnd_keep(a) /
  gaol::rnd_keep(b)` after `gaol::cleanup()` is the quotient to nearest with
  GCC too, where `gaol::rnd_keep(a / b)` may be the one computed before.
- **Or build GAOL with `GAOL_PRESERVE_ROUNDING`**, and the program keeps its
  own direction: each operation reads it, sets it upward, and sets it back when
  it is done. Nothing else changes: the bounds are the same, and the program
  still calls `gaol::cleanup()`. It costs time: on an Intel i7-1185G7 with
  GCC 9.4, `x + y`, `x - y`, `x * y` and `x / y` take 2.2 to 3.9 times
  longer, `sqr(x)` 6.0 times, `sqrt(x)` 3.5 times, `exp(x)` 1.7 times,
  `log(x)` 1.4 times, and `sin` and `cos` about 1.3 times
  (`tests/performance.cpp`).

### The AVX-512 path

Built with `GAOL_PREFER_AVX512` (`--enable-prefer-avx512`,
`-Denable-prefer-avx512=true`), on by default where the rounding direction
is preserved, whose every operation of `+`, `-`, `*`, `/` and `sqrt` the
path relieves of the store of that direction — `--disable-prefer-avx512`
and `-DGAOL_PREFER_AVX512=OFF` refuse it there —, on a processor that has
the AVX-512
instructions, GAOL has `x + y`, `x - y`, `x * y`, `x / y` and `sqrt(x)`, and
the operations of `+`, `-`, `*` and `/` with a double, computed by the rounding
direction the instructions carry in themselves (GAOL v5). These operations
set no rounding direction: the program keeps its own, in the default build
as with `GAOL_PRESERVE_ROUNDING`, and the other operations — `%`, the
integer powers, the elementary functions, the reading of a number — leave it
upward, as they always do. These operations raise no floating-point
exception flag of their own, the architecture requiring the exception
suppression with the embedded rounding. The modes that flush the subnormal numbers to zero,
which the embedded rounding does not ignore on the operands (measured on an
Intel i7-1185G7), are cleared before the operation reads a bound when a
program or a plug-in has set them, as the other operations clear them; the
subnormal results, which the embedded rounding computes exactly where
flush-to-zero alone would flush them, are the tighter for it.

The bounds are those of the other path, bit for bit: the rounding rule is
the same, and the tests of `tests/`, which a job of the continuous
integration runs with the option on, check it. The library built with the
option runs on any processor of the architecture: `gaol::init()` takes the
path only when the processor has the instructions, and the operations are
those of the default build otherwise. The operations of the path cost more
time on a processor that halves its AVX-512 instructions, where they cost
less on one that does not: on an Intel i7-1185G7 (halved), `x + y` takes
4.6 ns rather than 3.4 in the default build and `sqrt(x)` 16.6 rather than
8.1, where with `GAOL_PRESERVE_ROUNDING` the same `x + y` takes 5.6 ns
rather than 12.8; the processors that do not halve them, as the Xeon of
the continuous integration, are expected to take less. Which of the paths a
GAOL took, the example 13 tells.

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
  [The rounding direction](#the-rounding-direction)), and the modes stay
  cleared for the rest of the program, as the rounding direction stays upward.
  The check comes before the operation reads, compares or copies a bound, the
  test of the empty set aside, and the compiler is kept from making a
  comparison before it, which it would do, not knowing that the check changes
  how a subnormal compares: with GCC 13 at `-O3`, `asinpi()` compared the
  bounds it had read with 0 before its check, and with the modes set
  `asinpi(interval(100*2^-1074))` was [32*2^-1074], above the exact value. With
  `GAOL_PRESERVE_ROUNDING`, each operation sets the modes back as it found
  them, once it has made its result, its maxima and minima of bounds included.
  The bounds of every operation that makes the check are then the ones it gives
  with the modes cleared, the tightest ones, and the modes are cleared, or set
  back, after it (GAOL v5, checked for each of them by
  `tests/rounding_direction.cpp`). This is the only defence against a plug-in,
  which sets the modes after GAOL initialized itself, and against the compilers
  and processors that have no `-mno-daz-ftz`. On an Intel Xeon of the Cascade
  Lake generation, with Clang 18, an addition with a subnormal operand and
  result took as long as one of normal numbers (1.2 ns in a chain of dependent
  additions), as on the Intel i7-1185G7 of the review of 2026-09-27, and the
  check as long as the one of the direction alone, 1 + 2^-60, in front of the
  addition of two SSE2 intervals. Through the library (`gaol_performance`,
  medians of 9 interleaved runs), `x * y` took 4.0 ns rather than 3.4 ns,
  `sqrt` 11.0 ns rather than 10.3 ns and `pow(x, 3)` 13.7 ns rather than
  13.0 ns, and as long with the same check made of a normal number: the cost
  of its second addition, not of the subnormal; `x + y`, `x - y`, `x / y`,
  `sqr`, `exp`, `log`, `sin` and `cos` stayed within the noise of the
  machine. Some x86 processors take a microcode assist, of the order of a
  hundred cycles, for an operation with a subnormal operand or result: each
  operation would pay it there. Neither they nor the processors of the
  continuous integration (virtual machines, AMD EPYC or Intel Xeon) were
  measured; `tests/performance.cpp`, which it runs, prints the times there.
- `gaol.pc` and `gaol::gaol` give `-mno-daz-ftz` to the link, where the
  compiler accepts it: GCC 13 and later on x86, and from 11.4 and 12.4 in the
  series 11 and 12 (GCC 9.4, 12.3, GCC for ARM and Clang 18 refuse it, and the
  builds test the compiler with a link, not with its version). It keeps
  `crtfastmath.o` out of the link, so that a program linked with `-Ofast` does
  not get the modes at all, its own code included.

What remains: the functions that read or compare the bounds of an interval
without computing make no check, and read a subnormal bound as a zero for as
long as a mode flushing the operands (DAZ, FZ) is set, that is until an
operation that checks has cleared it, or, with `GAOL_PRESERVE_ROUNDING`, which
sets the modes back after each operation, for as long as the program keeps
them. They are the constructor from two bounds, the reading and the writing of
text, the relations (`set_contains()`, `certainly_le()`, `==`...), the
intersection `&` and the hull `|`, `max()`, `min()`, `abs()`, `sign()`,
`floor()`, `ceil()`, `integer()` and the other roundings to an integer,
`invabs_rel()`, `mig()`, `mag()`, `midpoint()` and `split()`. Under
denormals-are-zero, `max()` of `interval(3*2^-1074, 100*2^-1074)` and
`interval(200*2^-1074)` is the first one, `abs(interval(-1e-309, -1e-310))`
stays negative, and the hull `|` of the same two intervals, inline in the
headers of GAOL, depends on how the compiler of the program arranges its
comparisons: [0, 0] with GCC 13 at `-O2`, [3*2^-1074, 100*2^-1074] at
`-O0`, the right hull with Clang 18 at `-O2`. The unary minus makes no check
either, but it only exchanges the stored bounds, which the modes do not
change, as they do not change the test of the empty set, made before the check
of every operation (an operation with an empty operand may return before its
check, the modes left as it found them). The modes of other processors, and
of ARM with Visual C++, are neither checked nor cleared; GCC links
`crtfastmath.o` for none of the other processors GAOL is tested on. Link a
program that uses GAOL without these options, or with `-mno-daz-ftz`, and
compile the code that needs them apart from it.

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
  stops the program where the program enabled it; and with flush-to-zero set,
  the sum 2^-1060 + 0, flushed to zero, raises the underflow exception
  (`FE_UNDERFLOW`) too, before the check clears the mode (GAOL v5).
- An empty operand raises no invalid-operation exception, though the bounds
  of the empty interval are NaN (GAOL v5): every operation, relation, function
  and output of `gaol::interval` and of `gaol_ieee1788` takes it without
  raising that exception. `is_empty()` and the relations compare the bounds
  with the quiet comparisons of `<cmath>` (`std::islessequal()`...), each one
  instruction, as `<=` is, with GCC and Clang on x86-64, and so does the first
  comparison of the constructor `interval(l, r)`, which gives the empty set
  for NaN bounds. With Visual C++, where these are calls to the C library,
  which made `x * y` and `sqrt()` 34 to 61% slower on x64, GAOL compiles them
  itself into the same instruction, `ucomisd`, on x64 and on x86 with SSE2;
  on arm64 they stay calls. With the invalid-operation exception enabled, `is_empty()`
  of an empty interval killed the program, and so did about 50 operations
  with an empty operand, which compared its NaN bounds: `x & y` for an empty
  `y`, `sqrt`, `exp`, `min`, `max`, `floor`, `set_contains()`,
  `set_disjoint()`, the output of the empty set...; and `interval::emptyset()`
  itself in a build without optimization. A NaN the program gives GAOL, the
  empty set for `interval(d)`, raises it no longer either: `interval(NAN)`,
  `interval(NAN, 1)`, `x = NAN`, `x += NAN` (and `-=`, `*=`, `/=`, `%=`),
  `x &= NAN`, `x <= NAN` and `pow(x, NAN)` compared it with `<` or `<=`. An
  empty operand may still raise the other exceptions: the sum of two empty
  intervals raises the inexact one where GAOL checks the rounding direction
  with an addition, and `atanh_rel([-1, 2], x)` for an empty `x` raises the
  divide-by-zero one while it computes `atanh([-1, 2])`.
- A compiler may compile a quiet comparison into a signaling one, or compute
  a comparison that follows a test of the empty set before it, and GAOL is
  written against the cases seen. GCC turns a choice made on a quiet
  comparison into a conditional move with a signaling comparison on 32-bit ARM
  (`vcmpe`, GCC 12 to 14;
  [GCC bug 52258](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52258) is of
  this kind) and on POWER9 (`xscmpgedp`, GCC 13 with `-mcpu=power9`): there
  `is_empty()`, the intersection and the constructors of the FPU intervals
  tell NaN bounds with `std::isunordered()` first, which stays quiet, so that
  the choice a program makes on `is_empty()` (`x.is_empty() ? a : b` for an
  empty `x`), `x &= y` for an empty `y` and `floor()`, `max()`... of an empty
  interval raise nothing (the armhf jobs of the continuous integration and a
  POWER9 job under qemu check them, built for size too, where GCC calls the
  intersection rather than inlining it). GCC 9.4 at `-O3` computed the comparisons of the
  bounds of `x &= y` before its test of the empty set, in a loop of
  intersections: they are quiet ones. And GCC vectorizes loops of comparisons
  of bounds into vector comparisons that raise the exception for an empty
  element: GCC 9.4 and 13 where AVX is on (`-mavx`, or `-mfma`, which the
  three builds give where the processor has FMA) and GCC 13 to 15 for 32-bit
  x86 with SSE2 alone (MinGW-w64) make the quiet comparisons signaling
  predicates (`vcmpnlepd`, `cmpnlepd`...), GCC 14 for POWER8 uses `xvcmpgedp`,
  and GCC 11 to 15 for 64-bit ARM compute the second and third comparisons of
  the constructor for an empty element too. With GCC and the FPU intervals, on
  every processor, the bounds `is_empty()` compares, those of the relations
  that do not call it (`straddles_zero()`, `set_contains(d)`...) and the
  doubles given to the constructors go through an empty asm statement, which
  keeps GCC from vectorizing such loops: loops of `x.set_contains(d)` and of
  `x.straddles_zero()` took 0.8 and 1.3 ns per element rather than 0.5 ns with
  GCC 13 on x86-64, about as long as with GCC 9.4, which did not vectorize
  them and took as long as before. A choice the program makes on a relation,
  `x.set_contains(y) ? a : b`, and a loop of relations other than those of
  `tests/rounding_direction.cpp` are not known to raise the exception, but
  nothing keeps a compiler from compiling them so.
- Nonempty operands raised the invalid-operation exception as well, and the
  cases found no longer do (GAOL v5): with the SSE2 intervals, multiplying a
  zero bound by an infinite one did, `[0]*[1, +oo]` and `[0, +oo]*[0]` (the
  FPU intervals gave the same product without it), and through that product
  `pow([1], [1, +oo])`; `pow` did in every build, through CORE-MATH's `pow`,
  for an exponent of extreme magnitude (`pow([1, 2], [4.9e-324])`,
  `pow([1, 2], [1e-300])`, an exponent interval with a bound near `DBL_MAX`
  such as `[1e300, 1e308]`), whose power GAOL now computes itself; and with
  the SSE2 intervals, `div_rel(K, J, I)` did where it divided an infinite
  bound of `K` by an infinite bound of `J` in a half of the register it did
  not keep (`div_rel([2, +oo], [-oo, 2], I)`), and raised the
  division-by-zero exception where it divided a bound by a zero one there
  (`div_rel([-2], [-2, 0], I)`). This list is not exhaustive.

An empty operand, and the nonempty operands tested, raising no
invalid-operation exception therefore does not make that exception safe to
leave enabled: every exception stays disabled while GAOL computes.

The flags tell nothing of the results: after most operations of GAOL,
`fetestexcept(FE_INEXACT)` is raised whatever the result (not after `-x`,
`abs()`, `&`, `|`, `floor()`, `max()` or `min()`, which round nothing and
check no rounding direction), and `FE_OVERFLOW` or `FE_DIVBYZERO` stands for
an infinite bound of the interval, or an infinite result of `width()` or
`rad()`, not for an infinite number the program computed: `midpoint()` and
`mid()` of `[DBL_MAX]`, which overflowed the sum of the bounds, raise none
(GAOL v5). IEEE 1788-2015 leaves the flags of its operations unspecified. A
program that reads them for its own computations clears them
(`std::feclearexcept(FE_ALL_EXCEPT)`) just before, and reads them before its
next operation of GAOL.

GAOL's initialization, which runs before `main()`, sets the default
environment (`fesetenv(FE_DFL_ENV)`), which masks every exception, unless GAOL
is built with `GAOL_PRESERVE_ROUNDING`: the initialization then leaves the
whole environment as it found it, and an exception a static object enabled
before it stays enabled. A program enables the exceptions in `main()` or later,
whichever the build, not in a static object initialized before GAOL.
`gaol::cleanup()` leaves the exceptions and the flags as they are, and so does
every operation, whichever way GAOL is built: it leaves the exceptions the
program enabled enabled, and the flags it raised raised, and adds the flags it
raises itself. With `GAOL_PRESERVE_ROUNDING` and the SSE2 intervals, `+`, `-`,
`*`, `/`, `%`, `div_rel()`, `sqr()`, `inverse()`, the integer powers and
`+= d`, `-= d`, `*= d`, `/= d`, `%= d` wrote the SSE control register whole,
every exception masked and every flag cleared: an exception the program had
enabled was masked again by the first of these operations, and the flags it
had raised were lost (GAOL v5, checked by `tests/rounding_direction.cpp`).
