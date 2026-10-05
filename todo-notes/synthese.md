# Ce qui reste utile des rapports

Les rapports des agents qui ont travaillé sur les points de `TODO.md`, du 29
septembre au 2 octobre 2026, ont été relus le 4 octobre. Toutes les pull
requests qu'ils décrivent sont fusionnées, et leurs questions ont reçu une
réponse : décisions dans `TODO.md` et dans les commentaires de l'issue #49,
questions reportées dans les issues #64 à #70 et #80. Ce fichier garde ce qui
sert encore. Les rapports eux-mêmes ont été retirés de l'arbre ; ils restent
dans l'historique de git, au commit `16a2f60` :

```sh
git show 16a2f60:todo-notes/01.md
git ls-tree -r --name-only 16a2f60 todo-notes
```

Chaque section donne le rapport d'origine et la pull request qu'il décrit. Les
textes ont été écrits en anglais ou en français, selon le relecteur.

1. **Les textes proposés pour `ChangeLog` et `doc/differences.md`.** La pull
   request de synthèse les reprendra (point Y de `TODO.md`). Ils sont recopiés
   tels que les rapports les proposaient, avec une note quand des changements
   plus récents les ont rendus inexacts. Aucun n'est encore dans `ChangeLog`
   ni dans `doc/differences.md`, que rien n'a touchés depuis le 28 septembre.
2. **Le contenu encore utile** aux points qui restent : mesures, cas de test,
   mises en garde, et ce qu'aucun autre document ne dit (les pull requests #37
   et #39 n'ont pas de description).

## 1. Les textes pour `ChangeLog` et `doc/differences.md`

### `01.md` (#37) — point 1, the pow of the standard written once

ChangeLog:

```text
* gaol/gaol_interval.cpp: the pow of IEEE 1788-2015 (Table 9.1) is written once, pow_standard(); gaol_pow_hybrid() adds the integer power for a degenerate integer exponent and gaol_ieee1788::pow calls it; tests/ieee1788.cpp checks their bounds on 89 boxes.
```

doc/differences.md: none proposed.

In `configure-clean`: **no** (neither file mentions `pow_standard`). The
synthesis should write it together with #63 (exact corners) and #61.

### `05.md` (#39) — point 5, `-ffinite-math-only` refused

ChangeLog:

```text
* gaol/gaol_config.h: -ffinite-math-only is refused, as -ffast-math is (also Clang's -Ofast -frounding-math); tests/CMakeLists.txt: compile tests refused_finite_math_only and refused_fast_math.
```

doc/differences.md:

```text
- **`-ffinite-math-only`** is refused by `gaol/gaol_config.h`, as `-ffast-math` was. The compiler then takes NaN and infinities never to occur, in the inline functions of the headers too: the empty interval, whose bounds are NaN, was no longer told empty (`([1, 2] & [3, 4]).is_empty()` was false with GCC 9.4 and Clang 18). The check reads `__FINITE_MATH_ONLY__`, which also refuses what `__FAST_MATH__` does not show: Clang leaves `__FAST_MATH__` undefined for `-Ofast` or `-ffast-math` followed by `-frounding-math`, and keeps `__FINITE_MATH_ONLY__` at 1. The CMake tests `refused_finite_math_only` and `refused_fast_math` check the refusal. What no macro shows is not refused and is documented in [Compilers and options refused](three-builds.md#compilers-and-options-refused): `-funsafe-math-optimizations` and `-ffast-math -fno-finite-math-only` with GCC, which compiles the probe `1.0 + tiny == 1.0` as `tiny == 0.0`, so that `width()` is below the exact width after the code using GAOL left the rounding direction to nearest, and `-fno-honor-nans` alone with Clang.
```

In `configure-clean`: **no** (neither file mentions `-ffinite-math-only`;
`doc/differences.md` l.886-887 mentions `-fno-fast-math` only as a flag of the
builds). To adapt: TODO G.53 (the `is_empty()` claim holds in optimized code,
with bounds the compiler cannot fold, and differs between GCC 13 and Clang 18),
and the probes are two since #62 (`1.0 + tiny == 1.0` and
`1.0 + (subnormal + 0.0) == 1.0`, `gaol/gaol_config.h` l.219-228, which also
lists `-fno-signed-zeros` now).

### `06.md` (#54) — point 6, `fegetround()` read from MXCSR

ChangeLog:

```text
* gaol/core_math_port.h: on x86-64 the sources of CORE-MATH read the rounding direction from MXCSR, not from the x87 unit; pow missed subnormal results when the two differed (tests/rounding_direction.cpp).
```

doc/differences.md:

```text
- **`pow` with a subnormal result no longer misses the exact value when the x87 unit and the SSE instructions round differently** (x86-64, glibc). CORE-MATH rounds such results itself, in the direction `fegetround()` gives, and glibc reads it from the x87 unit alone, while GAOL sets and checks MXCSR. With the x87 unit to nearest and MXCSR upward, the state `exactinit()` of the predicates of Shewchuk and of Triangle leaves, the upper bound was below the exact value for about half of the arguments (182 of 400 random ones). `gaol/core_math_port.h` gives the CORE-MATH sources a `fegetround()` read from MXCSR, as the `get_rounding_mode()` of `cbrt.c`, `rsqrt.c` and `asinpi.c` does; the sources are unchanged. Not on 32-bit x86, where GAOL sets both units. `tests/rounding_direction.cpp` checks three powers against mpmath in the six states.
```

In `configure-clean`: **no**. Notes for the synthesis:

- This ledger covers only the first pass of #54. The full one, with the
  `cbrt.c` fix for mingw-w64 x64, is in `todo-notes/2026-09-30/06.md` ("For
  ChangeLog" l.233 and l.475, "For doc/differences.md" l.239 and l.482): use
  that one.
- "as the `get_rounding_mode()` of `cbrt.c`, `rsqrt.c` and `asinpi.c` does"
  holds for GCC and Clang only, not Visual C++ x64 (TODO A.56).
- `doc/differences.md` l.37-44 still gives the old, wrong reason for refusing
  some mingw-w64 ("whose `<fenv.h>` answers `fegetround()` from a state of its
  own"); the real reason is `fma()` and `round()` (#51). This ledger does not
  correct it; the one of point 6b (`todo-notes/2026-09-30/06b.md`, not in my
  set) should.

### `07.md` (#31) — point 7, `atanh([1, x])` empty

ChangeLog:

```text
* gaol/gaol_interval.cpp (atanh): the empty set for an interval with no point of (-1, 1), atanh([1]) having been [MAX, +oo];
	atanh_rel and tanhRev follow; tests in tests/elementary.cpp and tests/reverse_mappings.cpp.
```

doc/differences.md:

```text
- **`atanh()`** gives the empty set for an interval holding no point of `(-1, 1)`, where atanh is defined (IEEE 1788-2015, Table 9.1): `atanh([1])`, `atanh([1, 5])` and `atanh([1, +oo])` are empty, and so are `atanh_rel([1], x)` and `tanhRev([1])`. GAOL v5 with CORE-MATH gave `[MAX, +oo]` for them, the value of CORE-MATH at 1 being `+oo` and the lower bound the double below it; `atanh([-1])` and `atanh([-5, -1])` were empty already, because `interval(-oo, -oo)` is. (GAOL 4's own result was not measured: adapt or drop the sentence about the former values.)
```

In `configure-clean`: **no** (`doc/differences.md` mentions `atanh` only in
the list of CORE-MATH functions, l.416 and l.613). Per the decision, the final
parenthesis becomes a statement that GAOL 4 was not measured.

### `09.md` (#36) — point 9, `tan([-M_PI_2, M_PI_2])`

ChangeLog:

```text
* gaol/gaol_interval.cpp (tan): an interval whose width rounded upward is at most the double below pi is narrower than pi, so tan([-M_PI_2, M_PI_2]) is [-1.63e16, 1.63e16], no longer [-oo, +oo].
	* tests/elementary.cpp, tests/elementary_values.py: tan next to two consecutive poles, widths about the double below pi.
```

doc/differences.md:

```text
- **`tan()` is finite over the intervals of width π̲, the double below π, that hold no pole.** The width of the interval, rounded upward, was compared with π̲ with `<`, so that the width π̲ itself was taken for possibly holding two poles: `tan([-M_PI_2, M_PI_2])`, whose bounds are 6.1e-17 inside the poles ∓π/2, was `[-oo, +oo]`, and so were four other intervals whose exact width lies between the double below π̲ and π̲ and rounds up to it (review #8 of examples/examples.md). The width rounded upward being at least the exact width, `w <= π̲` proves the exact width below π, at most one pole is within the interval, and the signs of the cosine at the bounds tell whether it is: `tan([-M_PI_2, M_PI_2])` is `[-0x1.d02967c31cdb5p+53, 0x1.d02967c31cdb5p+53]`, about ±1.63e16. `tests/elementary.cpp` checks it, and 158 intervals next to two consecutive poles against mpmath (`tan_pole_intervals` of `elementary_values.h`); the intervals wider than π̲ still give `[-oo, +oo]`.
```

In `configure-clean`: **no** (the `tan()` bullets of `doc/differences.md`,
l.180-201 and l.534-548, are about the rounding checks and the tightness near
poles, not the width π̲). When M.9 is done, the synthesis may add that the
widths between π̲ and π are a theoretical case, as decided.

### `10.md` (#41) — point 10, `hausdorff()` on infinite bounds, `nb_fp_numbers()` on -0

ChangeLog:

```text
hausdorff(): equal bounds, infinite ones included, are at distance 0 (hausdorff([1,+oo],[2,+oo]) is 1, not +oo), in the SSE and FPU builds; nb_fp_numbers(): the doubles are numbered by the bits of |a| and |b|, so -0 is +0 (nb_fp_numbers(-0.0, 1.0) was 13830554455654793217).
```

doc/differences.md:

```text
- **`hausdorff()`** on infinite bounds and **`nb_fp_numbers()`** on -0. GAOL 4 returned +oo as soon as a bound was infinite (but for two entire intervals): hausdorff([1,+oo],[1,+oo]) and hausdorff([1,+oo],[2,+oo]), whose distance is 1, were +oo, so a fixed-point loop on a box with an unbounded side never stopped. Equal bounds, infinite ones included, are now at distance 0. GAOL 4 numbered the doubles by the bits of a and b, sign bit included: nb_fp_numbers(-0.0, 1.0) and nb_fp_numbers(-1.0, 0.0) wrapped around to 13830554455654793217 (the lower bound of [1, 2] - 1 is -0); they are now 4607182418800017409, -0 and +0 being one number.
```

In `configure-clean`: **no**; `doc/differences.md` l.288-290 has the older
`hausdorff()` bullet (tightest upper bound) to merge with it, as decided.
`nb_fp_numbers()` returns `unsigned long long` since #76: the text stays
right. When R.50 is done, the early exit can be added to the same bullet.

### `11.md` (#40) — point 11, the reader under flush-to-zero and denormals-are-zero

ChangeLog:

```text
* gaol/gaol_interval_lexer.lpp: numbers are read right whatever the flush-to-zero and denormals-are-zero modes (0 is 0, the subnormals are enclosed tightly): the reader takes the doubles apart by their bits.
```

doc/differences.md:

```text
- **Numbers are read whatever the flush-to-zero and denormals-are-zero modes** of the processor (`gaol/gaol_interval_lexer.lpp`). With denormals-are-zero, which a program linked with `-Ofast` has (`crtfastmath.o`) and the code it loads can set, a subnormal is 0 in a comparison and for `std::frexp()`, and the reader, which compared each number with the doubles around it as doubles, took every subnormal for 0: `textToInterval("1e-310")` was `[0x0.fffffffffffffp-1022, 0x1p-1022]`, which does not contain 1e-310, and `textToInterval("0")` was the greatest subnormal. The doubles are now taken apart by their bits, so that the bounds are the tightest whatever the modes; `tests/numbers.cpp` reads subnormals with flush-to-zero and denormals-are-zero set (x86).
```

In `configure-clean`: **no** (`doc/differences.md` has nothing on
denormals-are-zero or flush-to-zero). The synthesis should write it with the
ledger of point 4 (#62), which clears the modes in the operations.

### `13.md` (#43) — point 13, a point interval written in a form the reader refuses

ChangeLog:

```text
* gaol/gaol_interval.cpp (display_bounds): a point interval is written
	<a, a> only where its digits are exact, [l, r] otherwise, so that the
	reader takes the text back: interval(0.1) is [0.1, 0.1000000000000001].
```

doc/differences.md:

```text
- **A point interval is written in a form the reader takes.** In the format `bounds` (and in `agreeing` where it writes the bounds), `operator<<` writes `<a, a>` only for a point interval whose double the digits write exactly (`<4, 4>`, `<0.5, 0.5>`, `<-0, 0>`), and `[a, b]`, the two bounds rounded outward, otherwise.
  - **Before.** Every point interval was written `<a, b>`, the text of its double rounded downward and upward: `interval(0.1)` was `<0.1, 0.1000000000000001>`. The reader takes `<a, b>` for two numbers that are the same double, exactly, so it refused the text (`input_format_error`, and the empty set with `gaol_ieee1788::textToInterval`): `textToInterval(intervalToText(interval(0.1)))` was the empty set, against the manual, and so was every point whose double has more digits than `interval::precision()` (GAOL 4 wrote the same).
  - **Now.** `interval(0.1)` is `[0.1, 0.1000000000000001]`, which is read back as an interval containing it; a zero of either sign keeps its angles, `<-0, 0>` being read as `[-0, 0]`. `tests/numbers.cpp` reads back the text of point intervals written with 1 to 25 digits and six sets of flags, and `tests/ieee1788.cpp` checks `textToInterval(intervalToText(interval(0.1)))`.
```

In `configure-clean`: **no**. **Outdated**: these texts describe `<a, a>` for
exact points and `<-0, 0>` for zeros, while since #57 an exact point is `[a]`
(`[4]`, `[0.5]`) and since #57/#71 every zero point `[0]` (`[0x0p+0]` in
hexadecimal). The "Before" part stays true; the "Now" part must be rewritten
from #57 and the "Pour `doc/differences.md`" section of #71.

### `14.md` (#32) — point 14, `what()` of GAOL's exceptions

ChangeLog:

```text
* gaol/gaol_exceptions.h, gaol/gaol_exceptions.cpp: gaol_exception::what() returns the explanation ("gaol_exception" if none), where GAOL 4 left "std::exception";
operator<< of the exceptions writes the explanation once.
```

doc/differences.md:

```text
- **`what()` of GAOL's exceptions is their explanation.** `gaol_exception`
  overrides `what()`: it returns the explanation, or `gaol_exception` where
  there is none, so that `catch (const std::exception& e)` and the message of
  an exception that nothing catches say what went wrong. GAOL left the `what()`
  of `std::exception`, which is `std::exception` with libstdc++ and libc++,
  whatever the error. `operator<<` of an exception writes the explanation once,
  `file, line n: exception thrown: explanation`; it wrote `what()` next to it,
  `file, line n: exception std::exception thrown: explanation`. (Suggested
  place: after the bullet on `operator>>`.)
```

In `configure-clean`: **no**. To adapt once S.14 is done: without explanation
(or with an empty one) `what()` will return the name of the derived class
(`input_format_error`...), not `gaol_exception`.

### `15.md` (#46) — point 15, `operator>>` on an empty line

ChangeLog:

```text
* gaol/gaol_interval.cpp (operator>>): skips the blanks, line ends included, before the line it reads: a file ending with an empty line, a blank line between two intervals and `in >> d >> x` over two lines no longer throw input_format_error.
```

doc/differences.md:

```text
Replace the bullet "`operator>>` ends with the input" of doc/differences.md by: "- **`operator>>` ends with the input, and skips blank lines.** The blanks before the line, line ends included, are skipped as before a number (`std::ws`, whatever `std::noskipws` says): an empty line, or one of blanks, is not a line to read. GAOL read the rest of the line where the previous value stopped, so that `in >> d >> x` over the lines `1.5` and `[1, 2]` read the empty rest of the first one, threw `input_format_error`, and a file ending with an empty line ended `while (in >> x)` with the same exception. Where no line is left, it sets the `failbit` of the stream, leaves the interval as it was and throws nothing, as the reading of a double does, and `while (in >> x)` ends there. GAOL read the empty text then, threw `input_format_error` and emptied the interval: such a loop always ended with an exception. A line that is no interval sets the `failbit` too, and empties the interval; then `input_format_error` is thrown as before, a stream whose `exceptions()` include `failbit` included (`invalid_action_error` for a function called with an argument it does not take); an interval written on several lines is still no interval. A program that reads on after it calls `in.clear()` first: GAOL read the next line." (The old bullet says "A line that is no interval, a blank one included", which is no longer true.)
```

In `configure-clean`: **no**; `doc/differences.md` l.398-407 still has the
old bullet ("A line that is no interval, a blank one included, ..."), which
this text replaces (also listed in F.59).

### `16.md` (#57) — point 16, the `width` and `center` formats, `[empty]`, `intervalToText()`

ChangeLog (`ledger_changelog`):

```text
<ledger_changelog>The formats width and center write the midpoint() and rad() of IEEE 1788-2015 (radius rounded upward, a point interval without radius) and the empty set as [empty]; intervalToText writes [l, r] or [empty] whatever the output format, precision and locale.</ledger_changelog>
```

doc/differences.md (`ledger_differences`):

```text
<ledger_differences>- **The formats `width` and `center` write `midpoint()` and `rad()`, and `[empty]`.** `c (+/- w)` is the midpoint, rounded to nearest, and the radius of IEEE 1788-2015 (12.12.8), rounded upward: the interval is within c plus or minus w for the doubles, and a radius that is not zero is never written 0; a point interval is written as its center, and an unbounded one has an infinite radius. They are displays, not enclosures: the digits of c are those of the precision, and the formats `bounds`, `agreeing` and `hexa` are the ones that enclose. The empty set is `[empty]` in every format, as the manual said.
  - **Before.** GAOL wrote `(l+r)/2` and `(r-l)/2` rounded to nearest: `[1, 1+2^-52]` was `1 (+/- 1.11e-16)`, which stops short of the upper bound, `[0, 5e-324]` had a radius 0, `[1e308, 1.7e308]` a midpoint `inf`, the point interval `0.1` was written `0.1000000000000001`, and the empty set `empty`. The manual and the header called the radius the width.
- **`gaol_ieee1788::intervalToText()` writes an interval literal.** It writes `[l, r]` or `[empty]` with 16 digits and a decimal point, whatever the global output format, the precision of the intervals and the locale, and `[4, 4]` for a point interval, `<4, 4>` being no literal of the standard (13.3, 12.11). GAOL wrote what `operator<<` writes: the width format, the agreeing digits, a decimal comma under a comma locale.</ledger_differences>
```

In `configure-clean`: **no** (`doc/differences.md` has no bullet on the width
or center formats, `[empty]` or `intervalToText`). **Outdated in part**: a
point interval is `[a]` (`[4]`), not `[4, 4]`; `intervalToText` follows
`interval::precision()` (#71), not 16 digits; the radius is grouped and
unsigned under `showpos`, and a zero centre is `0` (#71). Merge with the
"Pour `doc/differences.md`" section of the description of #71. Note too that
`doc/differences.md` l.391-397 ("The interval is written in a stream of its
own") no longer matches #71, which builds the text in a string.

### `18.md` (#42) — lecture d'un long nombre sous une locale à virgule

ChangeLog (l. 60) :

~~~~text
gaol_interval_lexer.lpp: a number is taken apart once for the comparisons of gaol_enclose_number(), so that a long number reads in the same time under a comma locale as under the C locale (20000 characters: 2.5 s down to 0.03 s).
~~~~

doc/differences.md (l. 64) :

~~~~text
In the bullet "Numbers are read, and exact texts written, whatever the locale", after "and at most about 125 whatever it gives": "The text of the number is taken apart once, not by each comparison: a number of 20000 characters reads in about 0.03 s whatever the locale (2.5 s under a comma locale before, each comparison reading the text again in a time quadratic in its length)."
~~~~

Présence : **absents**. Le point d'insertion existe toujours tel quel
(`doc/differences.md`, puce « Numbers are read, and exact texts written,
whatever the locale », l. 360-373, « and at most about 125 whatever it gives »).

### `24.md` (#47) — exceptions et indicateurs flottants

ChangeLog (l. 56) :

~~~~text
gaol/: is_empty() and interval::emptyset() no longer raise the invalid-operation exception (std::islessequal, no comparison of NaN): a program that enabled it (feenableexcept) died on the first empty interval; doc/using.md and the manual say the exceptions must be masked while GAOL computes and what the flags mean.
~~~~

doc/differences.md (l. 60) :

~~~~text
- **`is_empty()` and `interval::emptyset()` raise no floating-point exception.** The empty set has NaN bounds, and `is_empty()` compared them with `<=`, a signaling comparison: with the invalid-operation exception enabled (`feenableexcept(FE_INVALID)`) every emptiness test of an empty interval, and every operation that starts with it, died on SIGFPE, and without it the test raised `FE_INVALID`. `interval::emptyset()` did the same in a build without optimization, through the comparisons of `interval(double)`. `is_empty()` now uses `std::islessequal` (`ucomisd` for `comisd`, same cost) and `emptyset()` sets its bounds without comparing. The other operations still raise the exceptions they raise (divide-by-zero for `log([0, 1])`, overflow for `[1e308]*10`, inexact for almost every operation): the exceptions have to be masked while GAOL computes (`doc/using.md`, the manual); `tests/rounding_direction.cpp` checks the empty sets with FE_INVALID enabled, in a child process (glibc).
~~~~

Présence : **absents** (ni `FE_INVALID`, ni `islessequal` dans
`doc/differences.md` ; la puce de l'ensemble vide, l. 640-642, ne parle pas
des exceptions). Le texte précède #50 et les décisions du 3 octobre : « The
other operations still raise the exceptions they raise » est dépassé pour les
opérandes vides (#50, texte dans `pr-50.md`) et le sera pour `0 × oo` (D.24) ;
à fondre dans la pull request de synthèse.

### `29.md` (#35) — le test sous une locale à virgule dans la CI

ChangeLog (l. 26) :

~~~~text
* .github: the Linux jobs generate fr_FR.UTF-8 (scripts/comma-locale.sh) and fail if tests/numbers.cpp did not check under it.
~~~~

doc/differences.md (l. 30, facultatif) :

~~~~text
Optional, appended to the bullet 'Numbers are read, and exact texts written, whatever the locale', after 'tests/numbers.cpp reads and writes under such a locale where one is installed.': ' The Linux jobs of the continuous integration generate fr_FR.UTF-8 for it and fail if it did not check under it.' Otherwise none.
~~~~

Présence : **absents**. Le texte dit « the Linux jobs » : depuis #74, chaque
job (Linux, macOS, Windows, conteneurs, `build-systems.yml`) génère ou vérifie
la locale ; à reformuler.

### `39.md` (#53) — la fonction de Goldstein-Price du manuel

doc/differences.md (l. 52 ; pas de texte pour ChangeLog) :

~~~~text
- **The Goldstein-Price function of the manual and of `examples/16_Goldstein_Price.cpp`** had the factor (x + y)^2 where the function has (x + y + 1)^2, as in GAOL 4; the manual said its output [-56254330, 94177270] was the range of f, which is [3, 1015690.27...] (148 times narrower). Both now compute the true function, whose natural extension over [-2, 2]^2 is [-87881320, 147125080] (231 times the range); the manual says that z encloses the range, and why it is wider. The example checks its output (exact integer bounds computed apart) and fails if a check fails.
~~~~

Présence : **absent** (« Goldstein » n'apparaît pas dans
`doc/differences.md`).

### `42.md` (#44) — meson 0.53 et le faux Python du Microsoft Store

ChangeLog (l. 51 ; pas de texte pour doc/differences.md) :

~~~~text
Document the Python that meson setup runs, and the Microsoft Store aliases (WindowsApps) that meson 0.53.0 and earlier take for it (doc/building.md, manual).
~~~~

Présence : **absent**.

### `43.md` (#45) — un `VERSION.txt` qui commence par une marque d'ordre des octets

ChangeLog (l. 56) :

~~~~text
* CMakeLists.txt, cmake/gaol_version.cmake, meson.build, configure.ac: VERSION.txt may start with a UTF-8 byte order mark (as some editors of Windows write it); the error for a refused VERSION.txt gives its first bytes in hexadecimal; test version_file and .github/scripts/version-file.sh.
~~~~

doc/differences.md (l. 60) :

~~~~text
- **`VERSION.txt` is read alike by the three builds**: a UTF-8 byte order mark at the start of the file (which some editors of Windows write), the line ends of Windows and the blanks and empty lines around the version are ignored, where CMake, meson and configure refused a file with a mark as `VERSION.txt holds "5.0.0"`, the mark not being seen. A file that is refused (a second mark, UTF-16 characters, a letter) gets a message that gives its first bytes in hexadecimal. The test `version_file` (CMake) and `.github/scripts/version-file.sh` (configure, meson) check it.
~~~~

Présence : **absents** (ni « byte order mark » ni « BOM »). À compléter avec
b505f0b : mêmes blancs dans les trois builds et autoconf, octet NUL refusé
partout, UTF-16 nommé dans le message.

### `44.md` (#33) — l'archive CPack et un `configure` en retard

ChangeLog (l. 26) :

~~~~text
* CMakeLists.txt, tests/cpack_stale_configure.cmake: CMake warns, when it configures, that configure was generated for another version than VERSION.txt holds, which the archive of the sources (package_source) would carry; a test configures copies of the tree to check it.
~~~~

doc/differences.md (l. 30) :

~~~~text
- **CMake warns of a `configure` generated for another version**: the archive of the sources holds `configure` as it is committed, and `configure --version` gives the version autoconf read from `VERSION.txt` when it generated it (the line `PACKAGE_VERSION=`). After a change of `VERSION.txt` without a new generation of `configure`, the archive of the new version held a `configure` that said the old one, which only the autotools job of the continuous integration reported. CMake now warns while it configures GAOL as the project built, naming both versions, and says nothing without `configure` or without that line; `tests/cpack_stale_configure.cmake` checks it (see [Building GAOL](building.md#the-version-of-gaol)).
~~~~

Présence : **absents**. Une fois N.44 fait, ajouter que `package_source`
reconfigure d'abord.

### `2026-09-30/06.md` (#54) — `fegetround()` lu dans MXCSR, puis `cbrt` avec mingw-w64

ChangeLog, première passe (l. 235-237) :

~~~~text
* gaol/core_math_port.h: on x86-64 the sources of CORE-MATH read the rounding
  direction from MXCSR, not from the x87 unit; pow missed subnormal results
  when the two differed (tests/rounding_direction.cpp).
~~~~

doc/differences.md, première passe (l. 241-251) :

~~~~text
- **`pow` with a subnormal result no longer misses the exact value when the
  x87 unit and the SSE instructions round differently** (x86-64, glibc).
  CORE-MATH rounds such results itself, in the direction `fegetround()` gives,
  and glibc reads it from the x87 unit alone, while GAOL sets and checks
  MXCSR. With the x87 unit to nearest and MXCSR upward, the state
  `exactinit()` of the predicates of Shewchuk and of Triangle leaves, the
  upper bound was below the exact value for about half of the arguments.
  `gaol/core_math_port.h` gives the CORE-MATH sources a `fegetround()` read
  from MXCSR on x86-64; the sources are unchanged. Not on 32-bit x86, where
  GAOL sets both units. `tests/rounding_direction.cpp` checks three powers
  against mpmath in the six states.
~~~~

ChangeLog, troisième passe (l. 477-480) :

~~~~text
* 3rd/math-core/src/binary64/cbrt/cbrt.c: with mingw-w64 on x86-64,
  get_rounding_mode() returned the FE_* value of fegetround() where 0 to 3
  are expected; nth_root(x, 3) missed the cube root at the hardest arguments
  (tests/core_math.cpp).
~~~~

doc/differences.md, troisième passe (l. 484-493) :

~~~~text
- **`nth_root(x, 3)` encloses the cube root with MinGW-w64 and MSYS2 on
  x64.** CORE-MATH's `cbrt` rounds seven hard arguments (and their products
  by powers of 8) apart, from a direction coded 0 to 3; with mingw-w64 on
  x86-64 its `get_rounding_mode()` returned the value of `fegetround()`
  instead (0x800 for upward), so that these were rounded toward zero and the
  upper bound was one double below the cube root, e.g.
  `nth_root([0x1.3a9ccd7f022dbp+0], 3) = [0x1.1236160ba9b92p+0,
  0x1.1236160ba9b93p+0]`. The value now goes through the switch of the other
  branches (change 6 of `3rd/README.md`); `tests/core_math.cpp` checks the 70
  arguments against mpmath.
~~~~

Présence : **absents**. Dans `doc/differences.md`, MXCSR n'apparaît que dans
les puces sur le sens d'arrondi de GAOL lui-même (l. 152-167, 274-286), et la
puce `cbrt` (l. 102) dit seulement que `nth_root(x, 3)` est le `cbrt` de
CORE-MATH. Avec #59, la puce `cbrt` gagnerait une phrase sur clang-cl
(`_WIN32`).

### `2026-09-30/06b.md` (aucune (enquête)) — enquête sur le refus de mingw-w64 avant 12

ChangeLog (l. 257) :

~~~~text
None (no change committed).
~~~~

doc/differences.md (l. 261-264) :

~~~~text
None (no change committed). If option 1(a) is taken, the item of
`doc/differences.md` line 37 ("only those whose `<fenv.h>` answers
`fegetround()` from a state of its own are still refused") should name
`fma()` and `round()` instead.
~~~~

Présence : l'option 1(a) a été prise (#51), mais **l'item de
`doc/differences.md` l. 37-44 dit toujours** « only those whose `<fenv.h>`
answers `fegetround()` from a state of its own are still refused ». Le texte
de remplacement est dans `06b-msvcrt.md`.

### `2026-09-30/06b-msvcrt.md` (#51) — mingw-w64 lié à msvcrt.dll refusé, et la vraie raison

Premier tour, ChangeLog (l. 397-410) :

~~~~text
- gaol/gaol_config.h: refuse on x86-64 every mingw-w64 linked with msvcrt.dll
  (`_UCRT` undefined), besides those before version 12. The reason given is
  now the true one: the fma() of mingw-w64's own math library is not
  correctly rounded and CORE-MATH computes with it, and its round() depends on
  the rounding direction. It is not fegetround(), which reads the x87 control
  word. The 32-bit x86 refusal before 11 names the fma() of that mingw-w64.
- tests/core_math.cpp: check first the fma() and round() of the C library in
  the four rounding directions, against values computed with exact rational
  arithmetic.
- .github/workflows/windows.yml: the refused MinGW-w64 jobs look for the
  reason in the message; MSYS2 MINGW64 (GCC and Clang) is checked refused.
- CMakeLists.txt, configure.ac, configure, meson.build, doc/three-builds.md,
  doc/continuous-integration.md, doc/tests.md, manual/v5/gaol.tex,
  gaol/gaol_fpu_fenv.h: the same reason in the comments and the docs.
~~~~

Premier tour, doc/differences.md (l. 414-433) :

~~~~text
Replace the item at line 37 ("mingw-w64 is no longer refused for its
version...") with:

```
  - mingw-w64 is no longer refused for its version alone. Both former reasons
    are gone: the hyperbolic functions of its math library, which GAOL no
    longer uses, and the cost of its `fesetround()`, which GAOL no longer calls
    for its elementary functions. MinGW-w64 GCC 12 to 15 are built and tested
    again on 32-bit x86, and GCC 14 and 15, MSYS2 UCRT64 and CLANG64 on x86-64.
    Still refused are the mingw-w64 whose `fma()` and `round()` are those of
    mingw-w64's own math library: on x86-64, before version 12 and those linked
    with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers
    of Debian and Ubuntu); on 32-bit x86, before 11. That `fma()` is not
    correctly rounded, and CORE-MATH computes with it; that `round()` depends
    on the rounding direction (`gaol/gaol_config.h`).
```

and, in the next item, "`tests/core_math.cpp` checks the bounds against
CORE-MATH..." becomes "`tests/core_math.cpp` checks the `fma()` and `round()` of
the C library, then the bounds against CORE-MATH...".
~~~~

Tour de relecture, ChangeLog, qui remplace le premier (l. 687-704) :

~~~~text
- gaol/gaol_config.h: refuse on x86-64 every mingw-w64 linked with msvcrt.dll
  (`_UCRT` undefined), besides those before version 12. The reason given is
  now the true one: the fma() and round() of mingw-w64's own math library,
  computed in doubles. That fma() is not correctly rounded, and CORE-MATH
  computes with it; that round() depends on the rounding direction. It is not
  fegetround(), which reads the x87 control word. On 32-bit x86, the refusal
  before 11 names the fma() of the mingw-w64 9 of WinLibs, which rounds each
  of its partial sums to a double. On ARM, the refusal before 11 is kept, as
  not tested. Cygwin (no `__MINGW32__`) is no longer caught by these checks.
- tests/core_math.cpp: check first the fma() and round() of the C library in
  the four rounding directions, against values computed with exact rational
  arithmetic.
- .github/workflows/windows.yml: the refused MinGW-w64 jobs look for the
  reason in the message; MSYS2 MINGW64 (GCC and Clang) is checked refused.
- CMakeLists.txt, configure.ac, configure, meson.build, doc/three-builds.md,
  doc/continuous-integration.md, doc/tests.md, manual/v5/gaol.tex,
  gaol/gaol_fpu_fenv.h: the same reasons, per architecture, in the comments
  and the docs.
~~~~

Tour de relecture, doc/differences.md, qui remplace le premier (l. 706-729) :

~~~~text
**For doc/differences.md.** Replace the item at line 37 ("mingw-w64 is no
longer refused for its version...") with:

```
  - mingw-w64 is no longer refused for its version alone. Both former reasons
    are gone: the hyperbolic functions of its math library, which GAOL no
    longer uses, and the cost of its `fesetround()`, which GAOL no longer calls
    for its elementary functions. MinGW-w64 GCC 12 to 15 are built and tested
    again on 32-bit x86, and GCC 14 and 15, MSYS2 UCRT64 and CLANG64 on x86-64.
    Still refused are the mingw-w64 whose `fma()` or `round()` is wrong
    (`gaol/gaol_config.h`). On x86-64, those before version 12 and those linked
    with `msvcrt.dll` rather than the UCRT (MSYS2 MINGW64, the cross compilers
    of Debian and Ubuntu) give the programs the `fma()` and `round()` of
    mingw-w64's own math library, computed in doubles: that `fma()` is not
    correctly rounded, and CORE-MATH computes with it; that `round()` depends
    on the rounding direction. On 32-bit x86, mingw-w64 before 11 is refused,
    as the mingw-w64 9 of WinLibs (GCC 11), whose `fma()` rounds each of its
    partial sums to a double. On ARM, mingw-w64 before 11 stays refused, not
    tested.
```

In the next item, "`tests/core_math.cpp` checks the bounds against
CORE-MATH..." becomes "`tests/core_math.cpp` checks the `fma()` and `round()` of
the C library, then the bounds against CORE-MATH...".
~~~~

Présence : **absents** (ni `msvcrt` ni `_UCRT` dans `ChangeLog` ni dans
`doc/differences.md`) ; l'item l. 37-44 de `doc/differences.md` est toujours
l'ancien, à la raison fausse. Le texte final dit « On ARM, mingw-w64 before 11
stays refused, not tested » : une fois A.6 fait (décision du 3 octobre), ce
sera « avant 12 ou sans `_UCRT` », comme sur x86-64, et
`GAOL_RND_MINGW_FENV_ONLY` aura disparu.

### `2026-09-30/16.md` (#57) — points 16 et 13, puis tour de relecture

Premier rapport, suite (le texte cité, « the first report », est dans
`todo-notes/16.md`, l. 62-65) :

````text
**For ChangeLog**

The formats width and center write the midpoint() and rad() of IEEE 1788-2015
(radius rounded upward, a point interval without radius) and the empty set as
[empty]; intervalToText writes [l, r] or [empty] whatever the output format,
precision and locale. (Unchanged from the first report; the follow-up commits
change no behaviour.)
````
_(`16.md`, l. 247-253)_

````text
**For doc/differences.md**

Unchanged from the first report of point 16 (the two entries on the width and
````
_(`16.md`, l. 255-257)_

Décision du 13 :

````text
**For ChangeLog**

* gaol/gaol_interval.cpp (display_bounds): a point interval whose digits
	are exact is written [a], the literal of IEEE 1788-2015 for a point,
	and a zero of either sign [0], by operator<< and intervalToText, instead
	of <a, a>; the reader still reads <a, b>.
````
_(`16.md`, l. 567-572)_

````text
**For doc/differences.md**

Replace, in the entry "A point interval is written in a form the reader
takes" (point 13) and in the intervalToText entry of point 16:

- **A point interval is written `[a]`.** In the format `bounds` (and in
  `agreeing` where it writes the bounds), `operator<<` writes a point interval
  whose double the digits write exactly as `[a]`, the literal of IEEE
  1788-2015 for a point (`[4]`, `[0.5]`), read back as that point; a zero of
  either sign is `[0]`, whatever the build (`interval::zero()` is `[-0, +0]`
  with SSE2 and `[+0, +0]` with the FPU intervals); any other point is written
  `[l, r]`, its two bounds rounded outward (`[0.1, 0.1000000000000001]`), read
  back as an interval containing it. `gaol_ieee1788::intervalToText` writes
  the same (`[4]`, `[0]`).
  - **Before.** GAOL wrote every point interval `<a, b>`, the text of its
    double rounded downward and upward: `<4, 4>`, a form of GAOL that is no
    literal of the standard, and `<0.1, 0.1000000000000001>`, which the reader
    refused (`input_format_error`, the empty set with
    `gaol_ieee1788::textToInterval`). The reader still reads `<a, b>`, so the
    texts GAOL 4 wrote for exact points are read back.
````
_(`16.md`, l. 574-593)_

Tour de relecture :

````text
**For ChangeLog**

Add to the entry of the previous section:
	operator<< writes a point interval whose text has a decimal comma
	(under a locale writing one) with its two bounds, [-2,5, -2,5], which
	the reader refuses, rather than [a], which it read as two numbers.
````
_(`16.md`, l. 807-812)_

````text
**For doc/differences.md**

In the entry "A point interval is written `[a]`", after "read back as that
point", add: "Under a locale writing a decimal comma, a point whose text has
a comma is written with its two bounds (`[-2,5, -2,5]`), which the reader
refuses, as it refuses every bound written with a decimal comma: `[-2,5]`
would be read as [-2, 5]."
````
_(`16.md`, l. 814-820)_

**Dans `configure-clean` :** rien de tout cela n'est dans `ChangeLog` ni dans
`doc/differences.md`. **Attention, une partie de ces textes est fausse depuis
#71** :

- l'ajout du tour de relecture (« Under a locale writing a decimal comma, a
  point whose text has a comma is written with its two bounds (`[-2,5,
  -2,5]`), which the reader refuses ») : depuis #71, le format `bounds` écrit
  dans la locale C, `[-2.5]`, que le lecteur relit ;
- « intervalToText writes [l, r] or [empty] whatever the output format,
  precision and locale » : il suit maintenant `interval::precision()` ;
- « hexa still writes `[a, a]` exactly » (comportement, dans le rapport) : le
  format `hexa` écrit `[a]`, et `[0x0p+0]` pour un zéro ;
- le format `agreeing` n'écrit plus ses chiffres communs que pour deux bornes
  finies, non nulles, de même signe et alignées.

La pull request de synthèse doit donc partir de l'entrée du point 13
(`todo-notes/13.md`, sections « For ChangeLog » et « For doc/differences.md », l. 54-64), des entrées `width`/`center` et
`intervalToText` du premier rapport (`todo-notes/16.md`, l. 62-65), du
remplacement ci-dessus pour `[a]`, et de la section « Pour
`doc/differences.md` » de la description de #71, qui remplace les parties
« virgule », « hexa » et « précision ».

### `2026-09-30/24b.md` (#50) — opérandes vides sans FE_INVALID

````text
**For ChangeLog**

gaol/: every operation, relation, function and output with an empty operand
raises no invalid-operation exception: quiet comparisons (`std::islessequal()`...)
in `&=`, `set_contains()`, `set_disjoint()`, `set_leq()`, `straddles_zero()`,
`is_canonical()`, `chi()` and the output, and the empty set told before the
constructor in `sqrt`, `exp`, `min`, `max`, `floor`, `ceil`, `integer`,
`split`, `modulo_k_pi` and `textToInterval(sl, sr)`; the FPU unary minus
exchanges the bounds. About 50 operations killed a program that enabled the
exception (`feenableexcept(FE_INVALID)`); tests/rounding_direction.cpp checks
them all.
````
_(`24b.md`, l. 317-327)_

````text
**For doc/differences.md**

- **An empty operand raises no invalid-operation exception anywhere.** Point 24
  made `is_empty()` and `interval::emptyset()` quiet; about 50 other
  operations still compared the NaN bounds of an empty operand with `<`, `<=`,
  `>=` or `>`, or gave them to the constructor, which compares them:
  `x & y` with an empty `y` (so `x &= f(x)` in a contractor), `sqrt`, `exp`,
  `min`, `max`, `floor`, `ceil`, `set_contains()`, `set_disjoint()`,
  `set_leq()`, `split()`, the output of the empty set, the reverse functions
  intersecting with an empty `x`, and with the FPU intervals the unary minus.
  With the invalid-operation exception enabled they died on SIGFPE, and
  otherwise raised `FE_INVALID`. They now use the quiet comparisons of
  `<cmath>` (one instruction, as `<=`, with GCC and Clang on x86-64) or tell
  the empty set before the constructor;
  the values are unchanged. A NaN the program gives (`interval(NAN)`) still
  raises it, and so do some nonempty operands (see `doc/using.md`); the other
  exceptions are not concerned (an empty operand may still raise the inexact
  one, through the check of the rounding direction of the addition).
````
_(`24b.md`, l. 329-346)_

**Dans `configure-clean` :** absent des deux fichiers (aucune mention de
`FE_INVALID` ni de « invalid-operation » dans `doc/differences.md`). L'entrée
du point 24 (#47), à fusionner avec celle-ci, n'y est pas non plus
(`todo-notes/24.md`).

### `2026-09-30/31.md` (#56) — correctifs à proposer en amont, trois passes

Première passe :

````text
**For ChangeLog**

	* 3rd/README.md: the fixes of the vendored CORE-MATH sources written as
	patches against upstream master b1a4badf, to propose to CORE-MATH
	(the masks of a 64-bit long, the shift of asinpi_acc, the
	__builtin_expect of rsqrt, the __builtin_roundeven of sin, the
	rounding direction of pow's subnormal results); cospi is fixed
	upstream, and glibc has none of these defects.

**For doc/differences.md**

Nothing: no behaviour of GAOL changes. At most, in the part on CORE-MATH, one
sentence: "The fixes GAOL makes to CORE-MATH are written as patches to propose
upstream in `3rd/README.md`."
````
_(`31.md`, l. 241-254)_

Deuxième passe :

````text
**For ChangeLog**

	* 3rd/README.md: the fixes of the vendored CORE-MATH sources written as
	patches against upstream master b1a4badf, to propose to CORE-MATH
	(the masks of a 64-bit long, the shift of asinpi_acc, the
	__builtin_expect of rsqrt, the __builtin_roundeven of sin, the
	rounding direction of pow's subnormal results, the rounding field of
	MXCSR in cbrt, rsqrt and asinpi with mingw-w64 and clang-cl); cospi is
	fixed upstream, and glibc has none of these defects.

**For doc/differences.md**

Nothing (no behaviour of GAOL changes).
````
_(`31.md`, l. 487-499)_

Tour de relecture (le dernier en date) :

````text
**For ChangeLog**

	* 3rd/README.md: the fixes of the vendored CORE-MATH sources written as
	a series of patches against upstream master b1a4badf, to propose to
	CORE-MATH (the masks of a 64-bit long, the shift of asinpi_acc, the
	__builtin_expect of rsqrt, the __builtin_roundeven of sin, the
	rounding direction of pow's subnormal results, the rounding field of
	MXCSR in cbrt, rsqrt and asinpi with mingw-w64, clang-cl and Cygwin);
	cospi is fixed upstream, and glibc has none of these defects.
	* manual/v5/gaol.tex: 3rd/README.md also holds the fixes to propose
	upstream.

**For doc/differences.md**

Nothing: no behaviour of GAOL changes.
````
_(`31.md`, l. 769-783)_

**Dans `configure-clean` :** absent de `ChangeLog` ; la phrase facultative pour
`doc/differences.md` (« The fixes GAOL makes to CORE-MATH are written as
patches to propose upstream in `3rd/README.md` ») n'y est pas (l'entrée
« CORE-MATH is in the sources », l. 1026-1029, n'en parle pas). Depuis, le
README a un septième correctif (`exact_pow()`, #63), et les changements
`/* GAOL */` de `sinpi.c` et `log1p.c` décidés le 3 octobre s'ajouteront : le
texte de `ChangeLog` (six correctifs) est à compléter.

### `2026-09-30/39.md` (#53) — Goldstein-Price, suite

````text
**For ChangeLog**

- examples/16_Goldstein_Price.cpp: goldstein_price() written with interval(double) constants; the time is printed in ms.
- examples/03_dependency_problem.cpp, examples/16_Goldstein_Price.cpp: the maximum of Goldstein-Price on [-2, 2]^2 is credited to sympy and mpmath in both.

**For doc/differences.md**

Nothing beyond the entry of point 39 already written. You may add "`16_Goldstein_Price` prints its time in milliseconds (`Elapsed time: N ms`)".
````
_(`39.md`, l. 91-98)_

**Dans `configure-clean` :** absent (ni « Goldstein » ni « Elapsed » dans les
deux fichiers). « The entry of point 39 already written » est dans
`todo-notes/39.md`.

### `2026-09-30/40.md` (#55) — petites erreurs

Premier tour :

````text
**For ChangeLog**

	* gaol/gaol_config.h (GAOL_NODISCARD): before C++17,
	__attribute__((warn_unused_result)) with GCC and Clang and
	_Check_return_ of <sal.h> with Visual C++, where it was empty.
	* gaol/gaol_interval.cpp (chi): GAOL_NAN for the empty set, as
	width() and mig() return; the quotient of the NaN bounds was -nan
	with GCC and raised FE_INVALID.
	* gaol/gaol_interval.h (chi): chi([0,0]) is -1 in the comment, with
	the cases of the infinite bounds and of the empty set.
	(textToInterval): names the section of the manual on the syntax.
	* tests/nodiscard.cpp, tests/CMakeLists.txt: the compile tests
	nodiscard_discard_cxx11/14/17 and nodiscard_used_cxx11/14/17, with
	-Werror=unused-result and -Werror=attributes; every compile test
	takes the lock gaol_compile_tests.
	* tests/gaol_tests.h: the references are computed with 2000 or 5000
	bits, not 400.
	* tests/interval_functions.cpp: chi of the empty set is a positive
	NaN; chi([-oo, 5]) and chi([-4, 4]).
	* manual/v5/gaol.tex: boolalpha in the 29 examples that print a bool;
	chi of the empty set and of the intervals with an infinite bound.

**For doc/differences.md**

- **The warning for a result thrown away before C++17.** `GAOL_NODISCARD` was empty before C++17, so a CMake project with GCC 9 (C++14 by default) got no warning for `sqrt(x);`. It is now `__attribute__((warn_unused_result))` with GCC and Clang, and `_Check_return_` with Visual C++ (reported by `/analyze`, warning C6031). GCC then also warns about a cast to void.
- **`chi()` of the empty set.** It is the positive NaN that `width()` returns, printed `nan` with every compiler, and it raises no FE_INVALID. It was the quotient of the NaN bounds, `-nan` with GCC.
````
_(`40.md`, l. 205-230)_

Tour de relecture (remplace les entrées correspondantes du premier tour) :

````text

**For TODO.md**, point 40 (d): `GAOL_NODISCARD` is `[[nodiscard]]` before C++17 wherever the compiler takes it there (GCC 7+, Visual C++ 2019 16.4+, warning C4834). Otherwise it is the attribute of Clang (and of GCC before 7), or `_Check_return_` with older Visual C++. It is checked by six CMake compile tests with GCC, Clang and Visual C++. The rest of point 40 is as in the first round.

**For ChangeLog**, replacing the first-round entries of `gaol/gaol_config.h`, `tests/nodiscard.cpp` and `tests/CMakeLists.txt`:

	* gaol/gaol_config.h (GAOL_NODISCARD): before C++17, [[nodiscard]]
	where __has_cpp_attribute(nodiscard) says the compiler takes it (GCC
	7 and later, Visual C++ 2019 16.4 and later),
	__attribute__((warn_unused_result)) with Clang and older GCC, and
	_Check_return_ of <sal.h> with older Visual C++, where it was empty.
	* tests/nodiscard.cpp, tests/CMakeLists.txt: the compile tests
	nodiscard_discard_cxx11/14/17 and nodiscard_used_cxx11/14/17, with
	GCC, Clang and Visual C++ 2019 16.4 and later, the warning for a
	result thrown away and those for an ignored attribute made errors;
	every compile test takes the lock gaol_compile_tests.
	* gaol/gaol_interval.cpp (chi): GAOL_NAN for the empty set, as
	width() and mig() return; the quotient of the NaN bounds was -nan
	with GCC and raised FE_INVALID.

**For doc/differences.md**, replacing the first-round items:

- **The warning for a result thrown away before C++17.** `GAOL_NODISCARD` was empty before C++17. So a CMake project got no warning for `sqrt(x);` with GCC 9 or Visual C++, both C++14 by default. It is now `[[nodiscard]]` there with GCC 7 and later and with Visual C++ 2019 16.4 and later (warning C4834), and `__attribute__((warn_unused_result))` with Clang. A cast to void silences it, except with GCC before 7.
- **`chi()` of the empty set.** It is the positive NaN that `width()` returns, printed `nan` with every compiler, where it was the quotient of the NaN bounds, `-nan` with GCC. It no longer raises FE_INVALID either: checked by hand here, and by the test of point 24b.
````
_(`40.md`, l. 378-401)_

**Dans `configure-clean` :** absent (ni `nodiscard` ni `chi` dans
`doc/differences.md` ; les entrées `chi()` de `ChangeLog`, l. 426-432, sont
celles de GAOL 4). `doc/differences.md` l. 874 dit déjà que `width()` du vide
est NaN, ce sur quoi s'appuie l'entrée de `chi()`.

### `2026-09-30/cc.md` (commits directs sur configure-clean) — suites des points 39, 42 et 43, commits directs

````text
**For ChangeLog**

	* examples/examples.md: the manual's examples print what it shows, but
	its overview program evaluated Goldstein-Price without its +1.
	* .github/workflows/build-systems.yml: meson setup with no Python on
	PATH, which guards find_program('python3', 'python') and meson's
	fallback on its own Python.
	* configure.ac, meson.build, cmake/gaol_version.cmake: VERSION.txt is
	read by the same rules everywhere (the six blanks of ASCII around the
	version, a NUL byte refused, UTF-16 named in the message); autoconf no
	longer removes the blanks inside the version.

**For doc/differences.md**

- **`VERSION.txt` is read by the same rules by CMake, meson, configure and autoconf.** They ignore the six
  ASCII blanks around the version (meson stripped Unicode spaces too, configure dropped a CR anywhere, and
  autoconf dropped blanks inside the version). They refuse a NUL byte, which configure and autoconf used to
  let through. A UTF-16 file is refused with a message that says to save it as UTF-8 or ASCII.
````
_(`cc.md`, l. 251-268)_

Après la relecture :

````text

### `2026-10-01/03.md` (#63) — point 3, pow(x, y) exact at the corners

Copied from `todo-notes/2026-10-01/03.md`, lines 277 to 289:

````text
**For ChangeLog**

pow(x, y): at the corner of the box that gives the lower bound, where x^y is
a double, the lower bound is x^y itself instead of the double below
CORE-MATH's value. `pow_is_double()` proves it exactly from the bits of x and
y. `pow([4], 0.5)` is [2] (it was [2 − 2^-52, 2]), and special cases 149 to
152 and 163 now give IEEE 1788's result (TODO point 3).

**For doc/differences.md**

`pow(x, y)` (and `gaol_ieee1788::pow`) is the tightest at the corners of a box
with finite bounds. The lower bound is the power itself where it is a double:
`pow([4], 0.5)` is `[2]`, where GAOL v5 gave `[2 − 2^-52, 2]` before.
````

State in `configure-clean`:

- **ChangeLog**: absent.
- **`doc/differences.md`**: absent, and now **wrong**. Its item at l. 450–452
  still says that `pow(I, J)` takes the tightest bounds at the corners "but for
  a lower bound one double below where the power there is a double (issue #8)".
  Point 3 removed that exception, so the synthesis must rewrite this clause and
  not only add the text above.
- **Missing from the proposed texts**: what #63 added after the report.
  - Visual C++: CORE-MATH's pow rounded downward was −0 between 2^-1075 and
    about 2^-947, because the UCRT's `NAN` and `INFINITY` are computed at run
    time. They are now read from their bits.
  - 32-bit Windows, every compiler: CORE-MATH takes the square root of SSE2
    rather than the C library's, which rounds to nearest. The upper bound of
    `pow([0x0.0000100020002p-1022], [0.5])` did not enclose the root. A.3
    extends this to every 32-bit x86.
  - armhf: `exact_pow()` no longer converts a 54-bit integer with
    `__aeabi_l2d`, which rounds to nearest. The upper bound fell below x^y, for
    example in `pow([3·2^-30], [34, 35])`. This is patch 7.
  - Special cases: 275 of 286 now give IEEE 1788's result.

  The texts for these are in the description of #63 only.

### `2026-10-01/04.md` (#62) — point 4, flush-to-zero and denormals-are-zero

Copied from `todo-notes/2026-10-01/04.md`, lines 192 to 204:

````text
**For ChangeLog**

GAOL v5: the modes flushing subnormals to zero (FTZ/DAZ on x86, FZ on ARM) are detected by the check of each operation
(1 + (2^-1060 + 0)) and cleared; the FPU intervals and the operations with a double check before comparing bounds;
`-mno-daz-ftz` is given to the link of code using GAOL where the compiler accepts it; new test `fast_math_link`.

**For doc/differences.md**

- **Flush-to-zero / denormals-are-zero** (GAOL v5): GAOL 4's check `1 + tiny == 1` saw only the rounding direction; a program
  linked with `-Ofast` (crtfastmath.o) or loading code built so had `[1e-300]*[1e-20]` = [0, 0], and with the FPU intervals
  `[1e-300]/[100·2^-1074]` empty. Each operation now probes with a subnormal, 1 + (2^-1060 + 0), and clears FTZ/DAZ (x86) or FZ
  (ARM, GCC/Clang); the arithmetic operations do it before comparing bounds; `GAOL_PRESERVE_ROUNDING` restores the modes;
  `gaol::gaol`/`gaol.pc` pass `-mno-daz-ftz` where the compiler accepts it (GCC 11.4, 12.4, 13+ on x86).
````

State in `configure-clean`: absent from ChangeLog and from
`doc/differences.md`, which says nothing of flush-to-zero.

**The texts are incomplete against the merged #62.** They should add:

- the check made before any bound is read or compared, and the barrier;
- the result made before the modes are restored, maxima and minima included;
- the cases this fixed (`asinpi`, `sqrt`, `atan2`, `sinpi`, `acos_rel`,
  `asin_rel`);
- +oo read after the check in `expm1`, `exp2m1` and `exp10m1`. This also fixed
  a bug of clang-cl;
- the exact rule of `gaol::gaol` for `-mno-daz-ftz` (`adadcc6`);
- the differential test.

### `2026-10-01/08.md` (#61) — point 8, pow(x, n) for large n

Copied from `todo-notes/2026-10-01/08.md`, lines 240 to 266:

````text
**For ChangeLog**

pow(x, n): the lower bound keeps the square of the rest it carries, and is the
tightest or one double beyond for every n (it was 1962 doubles below the
tightest for pow([1.0000001], 2^32 - 1)); the SSE2 intervals multiply the
rounded powers from the lowest bit of n, as the FPU intervals, and both give
the same bounds; the accuracy stated is within 5 n log2(n) 2^-104 of a double.

**For doc/differences.md**

In the item **Integer powers are computed from exact products** (issue #7):

- *How.*: replace "the lower bound keeps −l rounded upward and leaves out the
  square of l, which would raise it: the bounds are proved, and the tightest
  unless the power is within n·2^-104 of a double" by "the lower bound keeps
  −l rounded upward and the square of l, which raises it (left out before the
  review of 2026-09-27, n° 7: the lower bound of `pow([1.0000001], n)` was 8
  doubles below the tightest for n = 2^28 − 1, 1962 for 2^32 − 1): the bounds
  are proved, and the tightest unless the power is within 5n·log2(n)·2^-104 of
  a double".
- *Where not.*: add "for both bounds where one is 0 or infinite or its power
  out of range; the SSE2 intervals multiply these products from the lowest bit
  of n, as the FPU intervals (they multiplied from the highest bit, and the
  two builds gave different bounds for about half of these intervals)".
- *Tests.*: add "and the tightest bounds for 13 powers with n from 2^24 + 1 to
  2^32 − 1, against mpmath and exact integer arithmetic; the rounded products
  equal to the right-to-left binary exponentiation in both builds".
````

State in `configure-clean`:

- **ChangeLog**: absent.
- **`doc/differences.md`**: the item "Integer powers are computed from exact
  products" (l. 466–486) still has the old text, so the replacements proposed
  above apply as written: "leaves out the square of l, which would raise it…
  within n·2^-104 of a double" at l. 474–477, and *Where not* and *Tests*
  without the new parts.
- **Points to watch when they are used**:
  1. If a measured factor is quoted, it is "about 1.8", not 1.7.
  2. The *Where not* addition repeats the approximate wording that #61's
     reviewer pointed out (see below).
  3. Once B.8 (`ipow_exact_dn(0)` exact) is done, the "a bound 0" part of
     *Where not* will change.
  4. ChangeLog could also say that `gaol_core::MSB_position()` is no longer
     compiled. It was exported where visibility is not hidden, so the symbol
     leaves the library. Only 08.md and #61 say so.

### `2026-10-01/47.md` (#59) — point 47, clang-cl x64 and the rounding direction of cbrt, rsqrt and asinpi

Copied from `todo-notes/2026-10-01/47.md`, lines 420 to 445:

````text
**For ChangeLog**

* 3rd/math-core/src/binary64/cbrt/cbrt.c, rsqrt/rsqrt.c, asinpi/asinpi.c:
  get_rounding_mode() takes the branch of Windows under _WIN32 too: clang-cl
  on x86-64, which does not define __WIN32__, took the directed roundings for
  toward zero, and the bound of nth_root(x, 3) farther from zero and the upper
  bounds of rsqrt and asinpi did not enclose (tests/core_math.cpp).
* .github/workflows/windows.yml: GAOL built and tested with clang-cl on x64.
* tests/fp_strict/CMakeLists.txt: Visual C++ only, not clang-cl.

**For doc/differences.md**

- **`nth_root(x, 3)`, `rsqrt` and `asinpi` enclose the exact values with
  clang-cl on x64.** CORE-MATH's `cbrt`, `rsqrt` and `asinpi` read the
  rounding direction from MXCSR and turn it into the `FE_*` value of the C
  library by a shift chosen from `__WIN32__`, which clang-cl does not define:
  they compared glibc's layout with the UCRT's values and took the downward
  and upward roundings for toward zero, so that the bound of `nth_root(x, 3)`
  farther from zero and the upper bounds of `rsqrt` and `asinpi` were one
  double on the wrong side of the exact value at hard arguments (and at about
  half of all arguments of `asinpi`), e.g. `rsqrt([0x1.a6a9cc15abccep+0])` had
  `0x1.8e77a118a3095p-1` as upper bound, and `nth_root([-0x1.3a9ccd7f022dbp+0],
  3)` `-0x1.1236160ba9b93p+0` as lower bound (clang-cl 18, run under wine).
  `_WIN32` is now tested too (change 7 of `3rd/README.md`); `tests/core_math.cpp` checks `rsqrt` at the successors
  of the powers of 4, and the continuous integration builds GAOL with
  clang-cl.
````

State in `configure-clean`: absent from ChangeLog and from
`doc/differences.md`, which has no entry for clang-cl.

The texts need two updates:

- **The third ChangeLog bullet is obsolete.** Since #73, `tests/fp_strict`
  checks clang-cl from Clang 16 too ("Refusal of Visual C++ and clang-cl
  without /fp:strict").
- **The `windows.yml` bullet**: there are now five clang-cl jobs, with VS 2026
  x86, x64 and arm64.

Point K (#72, #73) is still to be written "d'après #72 et #73" (TODO, point Y).

### `2026-10-01/armhf.md` (#60) — the 24b fix on armhf, operator&= raised FE_INVALID

Copied from `todo-notes/2026-10-01/armhf.md`, lines 271 to 283:

````text
**For ChangeLog**

gaol/gaol_interval_fpu.h: on 32-bit ARM, `operator&=` tells an empty operand
with `std::isunordered()` before comparing the bounds. GCC 12 to 14 compiled
its quiet comparisons into the signaling `vcmpe` where it turned the choice of
a bound into a conditional move, so that `x & y`, `intersection()` and the
reverse functions raised FE_INVALID for an empty `y` on armhf.

**For doc/differences.md**

Nothing new: the entry of point 24b ("An empty operand raises no
invalid-operation exception anywhere") becomes true on 32-bit ARM for the
intersection; if the maintainer documents open question 2, a sentence there.
````

State in `configure-clean`:

- **ChangeLog**: absent.
- **`doc/differences.md`**: it has **no 24b entry at all**. The entry "An empty
  operand raises no invalid-operation exception anywhere", to which the text
  above refers, is a proposed text in `todo-notes/2026-09-30/24b.md`.
- **The ChangeLog text describes the first version**: "tells an empty operand
  with `std::isunordered()`". The final code tells `this` with `is_empty()`
  and `I` with `std::isunordered()` (`gaol_interval_fpu.h` l. 126–157).
- **D.24 will change it again**: it decides one `std::isunordered()` form of
  `&=` on every processor. So write this entry after D, or with it.

## 2. Le contenu encore utile

### `01.md` (#37) — point 1, the pow of the standard written once

- **Timings** (GCC 9.4, i7-1185G7, best of 7) beyond the "+2 to 3 %" of B.1:
  `gaol_ieee1788::pow` with a non-integer exponent -3 %, with `[3]` -6 to
  -11 %; `gaol::pow(x, [3])` -6 %; `gaol::pow(x, 3)` unchanged. A baseline
  for B.1 when it removes the two checks.
- **Differential proof**, recorded nowhere else (PR #37 has no description):
  10 500 grid boxes under the four rounding directions plus 60 000 random
  boxes, against a2ca992: FPU build identical; SSE2 identical but 5 126 zero
  lower bounds of `gaol::pow` (-0 to +0); `GAOL_PRESERVE_ROUNDING` as SSE2; no
  result depends on the rounding direction in effect at the call.
- **Mutation testing**: 24 of 29 mutants killed. The 5 equivalent mutants are
  the checks that can go without changing any bound, useful for B.1: `y`
  empty unchecked in `pow_standard()` (NaN bounds still give the empty set),
  `exp(y*log(x))` for `exp(y*log(base))` (log cuts to its domain), the
  `[±oo]` guard, `at_upper == 1.0` (CORE-MATH's pow(1, n) is exactly 1), and
  a redundant `is_empty()` in `gaol_ieee1788::pow` (gone since: it calls
  `pow_standard()` alone).
- **Boxes of `pow_on_boxes()` one double from the tightest bound that are not
  on the corners branch**: row 79 `{I(0,2), I(-1.5,-0.5)}` and the
  exp(y log x) rows `{I(1,3), I(-oo,2)}`, `{I(0.25,0.5), I(-oo,-1)}`,
  `{I(2,4), I(1,oo)}`, `{I(4,oo), P(-0.5)}`. These are the rows B.2 (corners
  extended to infinite bounds and to a base from 0) will change; their
  expected values were checked with `checkrows.py` (mpmath, 500 bits), which
  is now in `tests/tools/pow/` and rechecked them on 4 October: in the current
  table, boxes 82, 86, 87, 88 and 91 are still one double from the tightest
  bound.
- **Test cases for B.2 and B.8** from the correctness review:
  `gaol_ieee1788::pow([-oo, 2^-1000], [-oo, -1])` = `[0x1.ffffffffffd97p+999,
  +oo]`, 617 doubles below the tightest bound 2^1000 (B.2 cites only
  `pow([4, +oo], 0.5)`); before #61,
  `[0x1.fffffffffffd8p-1, 1.8e300]^(-2^31)` had an upper bound about 1e-6
  relative above the exact one (worth rechecking once in B.8).

---

### `05.md` (#39) — point 5, `-ffinite-math-only` refused

- With `-funsafe-math-optimizations`, the tests themselves fail:
  `rounding_direction` 2 of 16 772 checks (on `width()`), `arithmetic` 93 773
  of 1 240 155 (their reference computations compiled with the flag too, so
  what breaks in GAOL's inline code beyond `width()` is not established).
  Recorded only in `TODO-todo-status.md`; useful for the run-time check
  prototype of G.5.
- The two compile tests match the message of the header with
  `PASS_REGULAR_EXPRESSION` ("GAOL cannot be compiled with -ffinite-math-only"
  and "... -ffast-math", `tests/CMakeLists.txt` l.149-152): adding a remedy to
  the `__FAST_MATH__` `#error` (G.5) must keep that phrase. A compiler that
  wraps its diagnostics (`-fmessage-length`) could split it; with
  `make -jN test` the nested make may print a jobserver warning; each test
  costs about 1 s.
- Clang 18 `-fno-honor-nans` alone breaks the empty set like
  `-ffinite-math-only`, and per-function overrides (`#pragma GCC optimize`,
  `__attribute__((optimize))`, `#pragma clang fp`) were not tested: inputs for
  the G.5 prototype (the pragmas are named in G.5).

---

### `06.md` (#54) — point 6, `fegetround()` read from MXCSR

Nothing beyond the notes above (the details of the fix are in
`gaol/core_math_port.h`, `3rd/README.md` and PR #54).

---

### `07.md` (#31) — point 7, `atanh([1, x])` empty

- From the review of #31 (in `TODO-todo-status.md`, not in this file, and
  dropped from the current R.7): removing the clause `J.right() == -1.0` of
  `atanh()` (`gaol/gaol_interval.cpp` l.3299) fails no test, `atanh([-1])`,
  `atanh([-5, -1])` and `atanh([-oo, -1])` being empty through
  `interval(-oo, -oo)` anyway. A test would protect it only if that rule of
  the constructor changed. Could join R.7.

---

### `09.md` (#36) — point 9, `tan([-M_PI_2, M_PI_2])`

Nothing (the measurements are in TODO M.9 and PR #36).

---

### `10.md` (#41) — point 10, `hausdorff()` on infinite bounds, `nb_fp_numbers()` on -0

- The line numbers of TODO U.64 have moved: the lines of `doc/tests.md` over
  100 columns are now l.43, 72, 293, 323, 325 and 333 (U.64 says 43, 72, 298,
  300, 308; l.293 is new, from dc036c6), the orphan "With flush-to-zero,
  denormals-are-zero or both set in MXCSR" is l.306 (U.64: 281) and "The
  reading of" alone l.505 (U.64: 476).

---

### `11.md` (#40) — point 11, the reader under flush-to-zero and denormals-are-zero

- A reproducer for issue #68 from the correctness review: under DAZ, 1 645 of
  30 000 random texts `[a, b]` with a > b, both subnormal, gave an interval
  with reversed bounds; for instance `[0x2.19e9cb0p-1041, 0x3.f1p-1073]` gives
  the bits `0000000433d39600 0000000000000008` (the base commit did so in more
  cases). Issue #68 cites only `interval(0x1p-1073, 0x1p-1074)`.
- The bit-based reader still makes two comparisons per number (glibc's
  `strtod` gives the same first guess in every mode); `gaol_compare_number()`
  with a NaN candidate and v = 0 now returns 1 instead of 0 (no caller passes
  a NaN).

---

### `13.md` (#43) — point 13, a point interval written in a form the reader refuses

- A stale comment, tracked nowhere: above `display_bounds()`,
  `gaol/gaol_interval.cpp` l.1046 says "the hexadecimal format writes the
  signs of the bounds", while since #71 a zero point is written `[0x0p+0]`
  whatever the signs (`gaol/gaol_interval.h` l.92-93, manual l.3718,
  `doc/accuracy.md` l.201).

---

### `14.md` (#32) — point 14, `what()` of GAOL's exceptions

- `operator<<` of an exception still calls `explanation()` twice, copying the
  string twice (`gaol/gaol_exceptions.cpp` l.86-87). Harmless; could be
  tidied while S.14 edits that file.

---

### `15.md` (#46) — point 15, `operator>>` on an empty line

- Not in the ledger, possibly worth a clause in the synthesis: interactive
  input (`std::cin`) now waits for a non-blank line instead of failing on a
  bare Enter, as for numbers; the message of a refused line no longer shows
  the blanks before it; the stream tied to the input is flushed before the
  first read, as for a number.

---

### `16.md` (#57) — point 16, the `width` and `center` formats, `[empty]`, `intervalToText()`

Nothing beyond the notes above.

### `18.md` (#42) — lecture d'un long nombre sous une locale à virgule

- Chiffres à harmoniser dans la pull request de synthèse : le ledger dit
  « 2.5 s down to 0.03 s », F.18 dit 0,02 s, la relecture a mesuré 2,1 s puis
  0,021 s ; forme incertaine à 100 000 chiffres : 1,3 s contre 0,5 s
  (relecture) et 1,1 s contre 0,46 s (F.18).
- Si l'on revenait sur la décision : 9 chiffres décimaux (7 hexadécimaux) par
  passe rendraient l'analyse environ 9 fois plus rapide, une conversion par
  dichotomie la rendrait sous-quadratique (pas dans `TODO.md`).
- Sous QEMU, si `numbers` approche son délai, on peut baisser les 5000 et
  20000 chiffres du test (à 5000, l'ancien code échoue encore d'un facteur 40
  environ, avec une marge plus étroite à cause des 50 ms constants). Le délai
  est aujourd'hui `GAOL_NUMBERS_TIMEOUT` (300 s ; 600 s pour le job macOS GCC
  avec sanitizers, dc036c6).
- Les nombres courts sont plus rapides (0.1 : 550 ns contre 945 ns).
- `gaol_take_apart()` précède `round_nearest()`, mais `gaol_compare_number()`
  alloue encore (`gaol_natural_mul_u64()`, l. 351) après `round_nearest()`
  (l. 378) : ce que dit **H.23** reste vrai.

---

### `24.md` (#47) — exceptions et indicateurs flottants

- Ajouter `#include <cmath>` à `gaol/gaol_intervalf.h` (l'en-tête ne compile
  pas seul avec `-DGAOL_FLOAT_INTERVALS=1`) : à mettre dans D.24, qui traite
  déjà de ces intervalles.
- Coût mesuré de `std::islessequal` contre `<=` : nul (0,634 ns avec GCC 9.4,
  0,511 ns avec Clang 18, chaîne dépendante 4,63-4,69 ns) ; utile si l'on
  mesure le coût sous Visual C++ demandé en D.24.

---

### `29.md` (#35) — le test sous une locale à virgule dans la CI

Rien.

---

### `39.md` (#53) — la fonction de Goldstein-Price du manuel

Rien (l'enveloppe exacte, le rapport 231 et le maximum sont dans les
commentaires de l'exemple 16).

---

### `42.md` (#44) — meson 0.53 et le faux Python du Microsoft Store

Rien (le mécanisme de meson est dans le commentaire de `meson.build` et dans
`doc/building.md`).

---

### `43.md` (#45) — un `VERSION.txt` qui commence par une marque d'ordre des octets

Rien.

---

### `44.md` (#33) — l'archive CPack et un `configure` en retard

Rien (les durées sont dans N.44).

---

### `2026-09-30/06.md` (#54) — `fegetround()` lu dans MXCSR, puis `cbrt` avec mingw-w64

- Avertissement d'alors pour wine : un préfixe 64 bits seul ne charge plus
  après l'installation de `wine32:i386` ; utiliser un préfixe créé après
  (voir aussi `06b-msvcrt.md`).
- Le tableau des lecteurs du sens d'arrondi des 36 sources de CORE-MATH est
  repris dans `3rd/README.md` (correctifs 6 et 7) : rien de neuf.

---

### `2026-09-30/06b.md` (aucune (enquête)) — enquête sur le refus de mingw-w64 avant 12

Les taux d'erreur du `fma()` de mingw-w64 et le tableau des paquets
Chocolatey sont repris dans `gaol/gaol_config.h`, `doc/continuous-integration.md`
(l. 118) et les commentaires de `windows.yml` : rien de neuf.

---

### `2026-09-30/06b-msvcrt.md` (#51) — mingw-w64 lié à msvcrt.dll refusé, et la vraie raison

- Deux triplets où le `fma()` x87 des MinGW-w64 x86 acceptés (mingw-w64 11) se
  trompe, nulle part dans le dépôt : a = −0x1.53d31ea5098bcp-4,
  b = −0x1.8366682864b75p+10, c = −0x1.9535d79821296p-3 (vers le haut, le
  résultat juste est 0x1.00babe64a5209p+7, mingw-w64 11 x86 donne le double
  au-dessus) ; a = −0x1.42d4192294c63p+11, b = −0x1.2c80de6befe77p+11,
  c = 0x1.98cee0aa846fdp-38 (faux vers le bas et vers zéro). Les 91 triplets
  (`tmp/06b/guard/rows.pkl`) étaient dans un scratchpad, perdu. Utiles si la
  décision sur le x86 32 bits est rouverte, ou pour #69.
- État de la machine laissé par l'agent : architecture i386 ajoutée, environ
  80 bibliothèques i386 installées automatiquement restées (pas
  d'`apt-get autoremove`), environ 20 paquets amd64 mis à jour depuis
  noble-updates ; `wine32:i386`, `libwine:i386` et un paquet factice
  `libgphoto2-6t64:i386` (bloqué par le `libgd3` du PPA ondrej/php) retirés
  ensuite. Pour refaire les tests 32 bits : installer le paquet factice, puis
  `wine32:i386`, renommer `/usr/lib/wine/wine` en `wine.i386` et prendre un
  préfixe neuf (le paquet factice et les fichiers de chaîne étaient dans le
  scratchpad).
- L'issue #69 renvoie à `tests/rounding_direction.cpp` pour les cas
  reproductibles : c'est `tests/core_math.cpp` (`c_library_fma_and_round()`,
  l. 196).

---

### `2026-09-30/16.md` (#57) — points 16 et 13, puis tour de relecture

- `doc/differences.md` contredit déjà #71 et sera à corriger avec ces
  entrées : l. 370-371 (« The decimal formats of `operator<<` keep the
  decimal point of the locale of the stream »), l. 386-387 (« The flags, the
  precision and the decimal point of the stream are kept ») et l. 391-393
  (« written in a stream of its own, with the flags and the locale of the
  stream ») : faux pour le format `bounds` depuis #71 (locale C, texte formé
  dans une `std::string`).
- Rien d'autre qui ne soit déjà dans le code (commentaires de
  `display_bounds()`, de `Random` dans `tests/gaol_tests.h` l. 777), dans
  `TODO.md` (Q 45) ou dans les descriptions de #57 et #71.

---

### `2026-09-30/review-16.md` (#57) — relecture de `todo-16-display-formats` à `24b048e`

Rien : les sondes (`comma.cpp`, `macsim.cpp`, `dazdtoa.c`…) étaient dans le
bloc-notes de l'agent ; le test zéro de gdtoa (`if (!dval(&d)) return "0"`)
est cité par `tests/numbers.cpp` (l. 316) et par #68.

---

### `2026-09-30/24b.md` (#50) — opérandes vides sans FE_INVALID

- **Après la décision (a)**, les tests que 24b a ajoutés deviennent inutiles,
  et pas seulement ceux de `floor`, `ceil` et `integer` que nomme D (24) :
  l'auteur écrit que « `floor`, `ceil`, `integer`, `min`, `max` et `split`
  n'auraient alors plus besoin d'aucun test ». Ce sont les
  `std::isunordered(l, r)` de `gaol/gaol_interval.h` l. 1081-1093 et de
  `gaol/gaol_interval.cpp` l. 3627 et 3633, et les `std::isnan(m)` de
  `split()`, `split_left()`, `split_right()` (`gaol_interval.h` l. 756-775).
  Les mesures de 24b servent de référence : `is_empty()` d'abord coûtait 5 à
  10 % sur `max` et +0,9 ns sur `sqrt` ; la table « Performance » donne les
  temps de 16 opérations avant et après (clang-18).
- **SSE2 n'a pas de `<=` ordonné silencieux** (`_CMP_LE_OQ` n'existe qu'en
  AVX) et `minpd`/`maxpd` lèvent « invalid » sur un NaN silencieux : aucune
  forme sans branchement de `&=` silencieux n'a été trouvée avec SSE2 seul ;
  GCC 13 passe de `vcmplesd`/`vblendvpd` à `ucomisd` + `cmov` + un
  branchement. Utile pour D (24) ; noté nulle part dans `gaol/` ni `doc/`
  (`grep _CMP_LE_OQ` : rien).
- **Visual C++** : les `std::islessequal()` et sœurs de l'UCRT seraient des
  gabarits sur `_fpcomp()`/`_dpcomp()`, donc un appel (jamais vérifié) ; l'`&=`
  FPU, que Visual C++ compile, en fait quatre. `gaol_performance`
  (`tests/performance.cpp`) ne mesure toujours ni `&=` ni aucune relation : la
  mesure demandée par D (24) suppose d'abord d'y ajouter ces opérations. Repli
  envisagé : un assistant GAOL sur `_mm_ucomile_sd` et ses sœurs (pour
  `is_empty()` aussi). Compiler Explorer (voir `40.md`) montre le code de
  Visual C++ sans machine Windows.
- Un intervalle aux bornes inversées non NaN (que seul `interval(__m128d)`
  construit) va au constructeur comme avant, et `sqrt` d'un tel intervalle
  lève le drapeau. Sans conséquence pour GAOL lui-même.

---

### `2026-09-30/review-24b.md` (#50) — relecture de `todo-24b-quiet-empty-operands` à `bdec7d9`

Rien de plus que `24b.md` (le moins unaire FPU passe de 22 à 10 instructions
avec GCC et de 18 à 5 avec clang ; c'est dans le commentaire de l'opérateur).

---

### `2026-09-30/31.md` (#56) — correctifs à proposer en amont, trois passes

- **L'amont a bougé depuis `b1a4badf`** : `3rd/README.md` (l. 1190) cite déjà
  `284b3b0` du 1er octobre 2026. Les six blocs n'ont été vérifiés
  (`git apply --check`, `git am`) que sur `b1a4badf`. Avant l'envoi (#65),
  les rejouer sur le master du moment. Les fichiers `format-patch` et le clone
  `gaol-proposals-v3` étaient dans le bloc-notes de l'agent (`tmp/31/`) : les
  blocs du README sont la seule copie, et le bloc 6 suppose les correctifs 2 et
  3 appliqués (lignes `index`, dit dans le README).
- Vérifications de CORE-MATH non faites : `./check.sh --special` de sincos
  seulement en `--rndn` (26 minutes), lgamma pas du tout ; aucun essai en
  ILP32 (i686), faute de multilib et de wine32. Le README dit que le correctif
  ne change pas leur code objet sur x86-64 Linux.
- L'environnement MSYS de MSYS2 (runtime de Cygwin) a sans doute le défaut du
  correctif 6 comme Cygwin ; non vérifié, le README ne nomme que Cygwin.

---

### `2026-09-30/review-31.md` (#56) — relecture de `todo-31-upstream-patches` à `5bf9a23`

Rien de plus que `31.md`.

---

### `2026-09-30/39.md` (#53) — Goldstein-Price, suite

Rien : les chiffres du maximum et leur provenance (sympy, mpmath, grille numpy,
branch and bound) sont dans les commentaires de 03 et 16.

---

### `2026-09-30/review-39.md` (#53) — relecture de `todo-39-goldstein-price` à `969595e`

Rien : rapport de largeurs 231,3767 (l'ancienne enveloppe 148,1), déjà dans
l'exemple et dans `examples.md`.

---

### `2026-09-30/40.md` (#55) — petites erreurs

- **`run_examples.py` est perdu** : il était dans le bloc-notes des agents
  (`$SCR/tmp/40/run_examples.py`, encore utilisé le 1er octobre par le
  point 03), n'est pas dans le dépôt (`manual/` n'a que `build-pdf.sh` et les
  fichiers de build) et `find / -name run_examples.py` ne le trouve pas sur
  cette machine. La décision U (40) (« va dans `manual/` ») demande de le
  réécrire. Spécification d'après `40.md` (e) : extraire les 88 blocs
  d'exemple de `manual/v5/gaol.tex` qui montrent une sortie, compiler chacun
  contre un build avec `using namespace gaol` (`gaol_ieee1788` pour les deux
  blocs de ce chapitre), l'exécuter et comparer à la sortie montrée ; une
  exception connue, `@version` (la macro du manuel pour 5.0.0). Le
  30 septembre : 58 sur 88 avant #55, 88 sur 88 après (sse, fpu, clang, C++11
  et C++17).
- **Compiler Explorer** (`https://godbolt.org/api/compiler/<id>/compile`)
  donne les vrais Visual C++ 2015 à 2026 (x64, x86, arm64) et GCC 4.8 à 16 :
  c'est ainsi que les questions de Visual C++ ont été tranchées sans Windows
  (les outils `ce.py`… sont perdus avec le bloc-notes). Utile pour D (24)
  (Visual C++ compile-t-il `std::islessequal()` en appel ?) ; Visual C++
  2017 15.8 et 15.9 n'y sont pas (U, 40).
- Le build meson donnait 6 avertissements dans `tests/rounding_direction.cpp`
  (`-Wconversion`, `-Wunused-function`), antérieurs à la branche
  (`review-40.md`) : meson ajoute `-Wconversion` à tout (`meson.build`
  l. 235-239), alors que les jobs `-Werror` de #76 passent par CMake avec
  `-Wall -Wextra`. Non revérifié depuis.
- Pour G (40), remarque de cette revue : seuls les répertoires d'inclusion des
  tests `refused_*` sont faux en sous-projet (`tests/CMakeLists.txt` l. 139) ;
  leur `--build "${CMAKE_BINARY_DIR}"` (l. 142) est juste, l'arbre de build
  étant celui du projet de tête, et les tests nodiscard le gardent aussi
  (l. 220-224).
- Pour le vide, seul `left()` s'écrit `-nan` ; `chi`, `width`, `mig`, `smig`,
  `mag`, `midpoint` et `rad` s'écrivent `nan` (`review-40.md`).

---

### `2026-09-30/review-40.md` (#55) — relecture de `todo-40-small-errors` à `8007529`

Rien de plus que `40.md`.

---

### `2026-09-30/cc.md` (commits directs sur configure-clean) — suites des points 39, 42 et 43, commits directs

- La question du 3 octobre sur le `.strip()` de meson (#45, 19:11, « Non,
  laisser ») portait sur un état déjà changé : `b505f0b` (ce lot) fait déjà
  retirer à meson les seuls six blancs ASCII (`strip(string.whitespace)`,
  `meson.build` l. 24-32), comme CMake et configure. « Laisser » garde donc
  cette forme stricte ; le « `.strip()` de meson restent » du commentaire de
  l'issue #49 désigne ce code-là, pas l'ancien `.strip()`.
- Le mode texte de Python change un CR seul en LF : la citation de meson
  montre `\n` là où le fichier a `0d` (les octets en hexadécimal sont justes).
  Non noté dans `meson.build`.
- Risque « `od` de BSD » de `cc.md` : **MOOT**, les jobs autotools macOS
  lancent `version-file.sh configure` (`build-systems.yml` l. 51-52 et 76) et
  passent.

---

### `2026-09-30/review-cc.md` (commits directs sur configure-clean) — relecture de `configure-clean` à `b4fcb21`

Rien : le fuzz différentiel des quatre lecteurs (4000 fichiers, résultats
identiques) était dans le bloc-notes de l'agent.

### `2026-10-01/03.md` (#63) — point 3, pow(x, y) exact at the corners

- **Notes for B.2, way (b): not in TODO B.2**, which only says "avec des tests
  aux limites".
  - `tests/ieee1788.cpp` l. 265–266 expect `0x1.fffffffffffffp+0` for the boxes
    `I(0.25, 0.5) × I(-oo, -1)` and `I(2, 4) × I(1, oo)`. They must become the
    exact power once the corners take infinite bounds.
  - `pow_is_double()` must stay before the CORE-MATH call in `pow_lo()`.
  - `pow_lo()`, `pow_hi()` and `pow_is_double()` never check the rounding
    direction again, so B.2 (a) can move the check freely.
  - `pow_exact_at_corners()` (core_math) and `pow_on_boxes()` (ieee1788, bit
    for bit) are the regression net.
- **Mutants that the test must catch, if `pow_is_double()` is touched in B.2**.
  The table A to F is in 03.md only; #63 just says "11 mutants detected".
  - The product `e*y` rounded, for powers of two.
  - No check of `c^a·2^(f·a) < 2^1024` (e.g. (9·2^680)^1.5).
  - No check of f·a ≥ −1074 (e.g. (3·2^-1074)^2).
  - No check that 2^k divides e (e.g. 18^0.5).
  - No check of `r*r == m`.
  - `odd_significand()` off by one.
- **Cost of `pow_is_double()`** (clang-18, Xeon), against about 30 ns for
  CORE-MATH's pow. Not in the code comment, which says only "leaves at the first
  tests for nearly every y"; useful for Y.33.
  - About 2 ns for a non-integer y.
  - 4 to 8 ns for an integer y. A cheap rejection helps there: m ≥ 2^27 with
    y ≥ 2 gives c^a ≥ 2^54.
- **Stress run**: 62 million pairs near exact powers, two seeds, 0 mismatches,
  253 310 exact. The tool, `$SCR/tmp/03/stress/stress.cpp`, is not preserved:
  it is not in any surviving scratchpad.

---

### `2026-10-01/04.md` (#62) — point 4, flush-to-zero and denormals-are-zero

- **The full cost table**: 11 operations; base, a 2^-60 probe of the same shape,
  and new; Xeon Cascade Lake under KVM, clang-18, medians of 7 runs.
  - `doc/using.md` l. 431–440 and `gaol_fpu.h` l. 359–366 keep only a summary.
  - The table is the baseline for the CI measurement that Q.4 asks for.
  - The micro-benchmark also showed no subnormal penalty on that processor, and
    the 2^-60 variant cost the same.
- **FIZ has never been honoured anywhere it was tested**: qemu 8.2 has no
  FEAT_AFP. Only a real Armv8.7 machine, perhaps a recent macOS arm64 runner,
  would test it.
- **Two facts already recorded elsewhere**:
  - the GCC tags with `-mno-daz-ftz` are 11.4, 11.5, 12.4 and 13.1 and later,
    not 11.3, 12.2 or 12.3. `doc/using.md` l. 38, 83–84 and 123 cover this;
  - loading a Clang 18 `-Ofast` `.so` set MXCSR from 0x1f80 to 0x9fc0. This is
    covered qualitatively in `doc/using.md` l. 404–405.

  Nothing to add.

---

### `2026-10-01/08.md` (#61) — point 8, pow(x, n) for large n

- **A remark of #61's review that was not taken, and is recorded only in the
  description of #61** (not in TODO).
  - For an even power of an interval containing 0, the power of the bound of
    smaller magnitude is never computed: `pow([-2^-400, 2], 4)` takes the
    exact products.
  - So "for both bounds where one is 0 or … below 2^-968" is approximate in
    three places: `doc/accuracy.md` l. 94, the comment at
    `gaol_interval_sse.cpp` l. 299–301, and `gaol_interval.cpp` l. 193–197.
  - Its natural home is B.51 (the texts of pow), or B.8 together with
    `ipow_exact_dn(0)`.
- **Recorded only in 08.md and #61**: the alternative order, the FPU intervals
  in the SSE2 order, was implemented, measured and dropped. It made the FPU
  fallback 0.6 to 3.9 ns slower. Useful if someone wonders why the FPU order
  was chosen; the code comment says only "faster".
- **Nothing else**: the review numbers (8, 557, 1962 doubles), the error bound
  and the speed are in the code comments (`gaol_interval.cpp` l. 100–129,
  `gaol_interval_sse.cpp` l. 298–308).

---

### `2026-10-01/47.md` (#59) — point 47, clang-cl x64 and the rounding direction of cbrt, rsqrt and asinpi

- **The square root of the UCRT on x64.** With `/fp:strict`, clang-cl calls the
  UCRT's `sqrt()` from CORE-MATH (`rsqrt`, `asinpi`...) instead of inlining
  `sqrtsd`. Visual C++ x64 does the same: `core_math_port.h` l. 323–326 maps
  `__builtin_sqrt` to `sqrt()` off 32-bit x86.
  - So on x64 Windows, CORE-MATH relies on the UCRT's `sqrt` rounding in the
    direction of MXCSR, unlike 32-bit Windows, where #63 found it rounds to
    nearest.
  - The CI's `core_math` passes on both compilers (`rsqrt_hard_cases`, asinpi
    next to ±1).
  - But the comment of `core_math_port.h` (l. 180–200) discusses 32-bit
    Windows only. One sentence there would record the assumption. It is low
    priority, and fits A.56 (the comments of the rounding direction).
- **Any clang for x86_64 with `_WIN32` but no `__WIN32__` takes the new branch
  too**, rightly, since it uses the UCRT (e.g. the clang-based Intel icx for
  Windows, windows-itanium). This is not in `3rd/README.md`. It becomes moot
  once patch 6 in its table form (A.31) replaces the one-word change.
- **Recipe to build and run clang-cl without Windows**: clang-cl 18 with the
  MSVC 14.44 STL/CRT and UCRT 10.0.26100 taken from the VS manifest (xwin),
  `lld-link`, `/MT`, run under wine.
  - Clang 18 needed `_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH` with STL 14.44
    (VS 17.14 bundles Clang 19).
  - It is in review-47.md only. The files were deleted (#72: "l'environnement
    des sessions précédentes a été effacé").
  - It is useful if a clang-cl failure must be reproduced locally.

---

### `2026-10-01/review-47.md` (#59) — review of point 47 (head reviewed `118c852`)

- **The legacy UCRT 10.0.10240** (VS component "Windows Universal CRT SDK") has
  `FE_UPWARD` 0x100, and its `fesetround(FE_UPWARD)` really sets downward
  (MXCSR RC 0x2000, under wine). `3rd/README.md` l. 201 and 904 already
  mention the pre-14393 values; the "really sets downward" observation is
  extra. CMake and MSBuild take the latest SDK.
- **The clang-cl recipe**: see 47.md above.
- **Nothing else** outside what 47.md and #59 kept.

---

### `2026-10-01/armhf.md` (#60) — the 24b fix on armhf, operator&= raised FE_INVALID

- **A warning for D.24, not in TODO.** armhf.md's open question 2 says that
  `is_empty()` written as `std::isunordered(lb_, rb_) || left() > right()` on
  ARM32 must be checked: GCC must not fold it back into a single reversible
  UNGT, which would become `vcmpe` again. The test is then the program's own
  `x.is_empty() ? p : q` for an empty x, which compiled to `vcmpe; it ls;
  vmovls` with GCC 13.3. #60's three choice checks cover part of it, not under
  `-Os`.
- **Measurements of the single-`isunordered` form**, for D.24, which asks "à
  vérifier avec Clang 18":
  - GCC 13 x86-64: throughput loop 1.95 → 1.21 ns;
  - clang-18: +9 % in the random throughput loop (1.73–1.80 → 1.89–1.97 ns),
    −8 % in the random in-place loop, within 2–3 % in the others;
  - the two-guard form: +20 to 25 % in two clang loops.

  These numbers are in armhf.md only (`tmp/armhf/cg/bench_variants.cpp`, lost).
- **The survey of 24b** (`tmp/review-24b/survey.cpp`: 287 operations, 4845
  combinations with an empty operand, plus a mode that dumps the bits of every
  result) is not in the repository. It is not in any surviving scratchpad
  either: only the older survey of point 24 remains, in
  `/tmp/claude-1001/-home-jninin-Documents-WORK-DEV-GAOL-GAOL-V8/334ea319-d088-4702-9b10-e0628647c222/scratchpad/tmp/24-fp-exceptions-rev/survey.cpp`.
  - D.24 (`is_empty()` on ARM32 and POWER9, relations without FE_INVALID even
    vectorized, the single-`isunordered` `&=`) would profit from rebuilding it.
  - `tests/rounding_direction.cpp` has only its 61 entries.
- **The GCC mechanism in full**:
  - `REVERSE_CONDITION` gives `reverse_condition_maybe_unordered`;
  - `movdfcc` calls `arm_gen_compare_reg`;
  - CCFPE goes to LT, LE, GT and GE, CCFP to EQ, NE, UNORDERED, ORDERED and the
    UN* codes;
  - AArch64 uses `fcmp` and `fcsel`, which are quiet.

  It is in armhf.md only; the comment of `gaol_interval_fpu.h` l. 130–145
  gives the short form. It is what a GCC report (2b) would need.
- **`#if defined(__arm__) && !defined(__aarch64__)` does not cover Visual C++
  for 32-bit ARM** (`_M_ARM`). That compiler is not in the CI, and nothing
  records this outside armhf.md. It is harmless as long as Visual C++ ARM32 is
  not a target.
