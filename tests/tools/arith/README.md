<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-19 by Jordan NININ -->
# Verification tool of the arithmetic

Part of the [tests](../../../doc/tests.md) of GAOL v5.

`diffarith` compares two builds of GAOL on the operations of the interval
arithmetic: the FPU, the SSE2 and the NEON intervals (`GAOL_SIMD`,
`GAOL_ARM64`), a version before a change and the one after, or two compilers.
It was written for the NEON intervals of 64-bit ARM processors, whose kernels
(`gaol/gaol_interval_neon.cpp`) replace the code of the FPU intervals
(`gaol/gaol_interval_fpu.cpp`), and which have to give the same bounds and the
same exception flags, but for the sign of a zero.

It is not part of the build nor of `make test`: it compares builds, and its
result is read by whoever changes the arithmetic of the intervals.

| File | What it does |
|---|---|
| `diffarith.cpp` | Prints, for each operation on a grid of boxes, the bounds of the result in hexadecimal (the signs of the zeros included) and the flags FE_INVALID and FE_DIVBYZERO it raised, with the rounding direction of the caller set to nearest, upward, downward and toward zero. The operations are `+ - * / %` and `&= \|=` of every pair of boxes, `hausdorff` and `div_rel` of one pair in 11, `sqr`, `sqrt`, `inverse`, the negation, `abs`, `mid`, `midpoint` and `exp` of each box, and `+= -= *= /= %=` with a double (a bound of the grid, or a NaN). The boxes are the intervals between the bounds of a grid of 23 numbers (the infinities, ±`DBL_MAX`, ±1e300, −2.5, ±1, ±`DBL_MIN`, ±the smallest subnormal, ±0, 0.1, 3 and six random numbers), the empty set, [−∞, +∞] and `interval::zero()` |
| `diffarith.py` | Builds `diffarith.cpp` against two builds of GAOL or more, runs it and compares the outputs, line by line: identical lines, lines that differ only in the sign of zero bounds, and lines that differ otherwise (a bound, an empty set, an exception flag), by operation |

The builds are CMake builds with the tests and the compile commands, as for
the [tools of pow](../pow/README.md) (`build.py` of that directory compiles the
program with the command and the library of each build):

```bash
cmake -S . -B build-neon -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake -S . -B build-fpu  -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGAOL_ARM64=OFF
cmake --build build-neon -j 4
cmake --build build-fpu -j 4
python3 tests/tools/arith/diffarith.py run --out /tmp/arith neon=build-neon fpu=build-fpu
```

`run` writes `/tmp/arith/LABEL/diffarith.txt` for each build and compares each
with the first. `compare A.txt B.txt` compares two outputs, however they were
made. It exits with 1 when a line differs otherwise than by the sign of a zero.

On a 64-bit ARM processor (an Apple M2, AppleClang 21), the NEON build and the
FPU one gave the same bounds and the same flags on the 2.39 million lines of
the grid (four rounding directions of the caller), but for 45 336 lines (2%)
that differ in the sign of a zero bound only: the maximum of 64-bit ARM ranks +0 above −0 where the code of the
FPU intervals compares the bounds, and `interval::zero()` is stored as with
the SSE2 intervals (see [Accuracy](../../../doc/accuracy.md)).
