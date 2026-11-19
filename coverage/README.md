<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-09-20)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
# Coverage of the tests

*Written by `coverage/coverage_summary.py` from the report of gcovr, which `cmake --build <build> --target coverage` runs (`-DGAOL_COVERAGE=ON`). Last run: 2026-11-19.*

The line-by-line report is [coverage.html](coverage.html), one page holding its own style.

## The runs this page merges

A run compiles the files of one processor family and the tests run the code of that family only. The runs below are merged (`gcovr --add-tracefile`), so that a line is covered if one of them runs it, and each file of the intervals is counted in the run that compiles it.

- SSE (x86-64): GCC 15.2 on an Intel Core i7-1185G7, the SSE2 intervals, 75 tests of `dev` at `0173d0a` with the examples, none failed (`intervalf` and `interval2f` are skipped by construction), gcovr 7.2.
- AVX-512 (x86-64): the same machine with `-DGAOL_PRESERVE_ROUNDING=ON`, which takes the AVX-512 path (`GAOL_AVX512_ARITHMETIC`) for `+`, `-`, `*`, `/`, `sqr`, `sqrt` and the integer powers, 75 tests of `dev` at `0173d0a` with the examples, none failed (the same two skipped). The run compiles the SSE2 files too, and all its files are merged: the lines it adds to them are those of the preserved rounding direction.
- FPU: the same machine with `-DGAOL_SIMD=OFF`, 72 tests of `dev` at `0173d0a`, none failed (the same two skipped).
- NEON (64-bit ARM): an Apple M2 with Apple clang 21.0.0 (`llvm-cov gcov`), the NEON intervals, 62 tests of `dev` at `fd1194f` (the tree of `0173d0a`) with the examples, none failed (the same two skipped), gcovr 7.2. Only the three files `gaol/core/gaol_interval_neon*.h` are merged: clang does not count the lines of the other files as GCC does, so their lines would not add up.

Each run is `cmake -S . -B <build> -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON -DWITH_EXAMPLES=ON -DGAOL_COVERAGE=ON` (with `-DGAOL_SIMD=OFF` for the FPU, and `-DGAOL_PRESERVE_ROUNDING=ON` for the AVX-512 path, which a processor with AVX-512 then takes) and `cmake --build <build> --target coverage`, which leaves `<build>/coverage.json`. The merge is `gcovr --root . --merge-mode-functions=merge-use-line-min --filter "$PWD/gaol/" --filter "$PWD/3rd/math-core/" --add-tracefile <run1>.json --add-tracefile <run2>.json ... --html --html-self-contained --html-title "Coverage of GAOL" --output coverage/coverage.html --json merged.json`, and this page is `python3 coverage/coverage_summary.py merged.json coverage/README.md --run ...`.

## Conclusion

| | Lines | Covered | Not run | % | |
|---|---:|---:|---:|---:|---|
| **GAOL (`gaol/`)** | 8077 | 7881 | 196 | **97.6** | `████████████████████` |
| CORE-MATH (`3rd/math-core/`) | 8172 | 6741 | 1431 | 82.5 | `████████████████░░░░` |

**The tests run 97.6 % of the lines of GAOL, above the 80 % GAOL v5 aims at.**

Branches: 70.6 % of those of GAOL are taken both ways.

The lines of CORE-MATH that are never run are its accurate phases, which a handful of arguments in a million reach, and the paths of the architectures the runs are not. They are counted among the lines not run, and are checked otherwise: the accurate path of `sin` is compared with the upstream sources compiled unchanged, and that of `cos` with its roundings computed by mpmath (see `doc/tests.md`); the other accurate phases are reached by the tests of their bounds only.

## The intervals of each processor family

The configuration chooses the files that compute the intervals: SSE2 on x86-64, NEON on 64-bit ARM, the FPU elsewhere and with `-DGAOL_SIMD=OFF`, and the AVX-512 path of x86-64 (with the SSE2 files) where the rounding direction is preserved. A run compiles one family, so the files of the others are in the page only if a run of them was merged.

| Family | File | Lines | Covered | % | Branches taken |
|---|---|---:|---:|---:|---:|
| SSE (x86-64) | `gaol/core/gaol_interval_sse.h` | 101 | 101 | 100.0 | 72.3 % |
| SSE (x86-64) | `gaol/core/gaol_interval_sse_impl.h` | 482 | 458 | 95.0 | 66.0 % |
| SSE (x86-64) | `gaol/core/gaol_interval_sse_kernels.h` | 52 | 52 | 100.0 | 56.2 % |
| | **SSE (x86-64)** | 635 | 611 | **96.2** | 66.3 % |
| AVX-512 (x86-64) | `gaol/core/gaol_interval_avx512_impl.h` | 294 | 274 | 93.2 | 93.0 % |
| | **AVX-512 (x86-64)** | 294 | 274 | **93.2** | 93.0 % |
| NEON (64-bit ARM) | `gaol/core/gaol_interval_neon.h` | 118 | 118 | 100.0 | 100.0 % |
| NEON (64-bit ARM) | `gaol/core/gaol_interval_neon_impl.h` | 222 | 222 | 100.0 | 100.0 % |
| NEON (64-bit ARM) | `gaol/core/gaol_interval_neon_kernels.h` | 125 | 124 | 99.2 | 98.2 % |
| | **NEON (64-bit ARM)** | 465 | 464 | **99.8** | 99.4 % |
| FPU | `gaol/core/gaol_interval_fpu.h` | 77 | 77 | 100.0 | 79.3 % |
| FPU | `gaol/core/gaol_interval_fpu_impl.h` | 244 | 244 | 100.0 | 93.9 % |
| FPU | `gaol/core/gaol_interval_fpu_kernels.h` | 47 | 47 | 100.0 | 71.7 % |
| | **FPU** | 368 | 368 | **100.0** | 87.7 % |

## GAOL, file by file

Sorted by coverage, least covered first.

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `gaol/parser/gaol_expr_eval.h` | 187 | 165 | 88.2 | 56.9 % |
| `gaol/core/gaol_interval_avx512_impl.h` | 294 | 274 | 93.2 | 93.0 % |
| `gaol/parser/gaol_expression.cpp` | 860 | 813 | 94.5 | 39.0 % |
| `gaol/core/gaol_interval_io.cpp` | 386 | 365 | 94.6 | 67.2 % |
| `gaol/gaol_common.cpp` | 79 | 75 | 94.9 | 82.7 % |
| `gaol/core/gaol_interval_sse_impl.h` | 482 | 458 | 95.0 | 66.0 % |
| `gaol/gaol_common.h` | 21 | 20 | 95.2 | 66.7 % |
| `gaol/core/gaol_interval_core.h` | 413 | 400 | 96.9 | 72.2 % |
| `gaol/core/gaol_interval_pow.cpp` | 550 | 533 | 96.9 | 72.0 % |
| `gaol/parser/gaol_eval_stack.h` | 34 | 33 | 97.1 | 66.7 % |
| `gaol/math/gaol_fma.h` | 228 | 224 | 98.2 | 97.0 % |
| `gaol/core/gaol_interval_detail.h` | 58 | 57 | 98.3 | 80.4 % |
| `gaol/core/gaol_interval_trig.cpp` | 380 | 375 | 98.7 | 85.8 % |
| `gaol/core/gaol_interval_core.cpp` | 445 | 440 | 98.9 | 63.4 % |
| `gaol/math/gaol_compoundm1.h` | 808 | 801 | 99.1 | 88.9 % |
| `gaol/core/gaol_interval_neon_kernels.h` | 125 | 124 | 99.2 | 98.2 % |
| `gaol/core/gaol_interval_internal.h` | 243 | 242 | 99.6 | 80.1 % |
| `gaol/core/gaol_interval_relational.cpp` | 744 | 742 | 99.7 | 71.8 % |
| `gaol/core/gaol_interval_exp_log.cpp` | 140 | 140 | 100.0 | 76.6 % |
| `gaol/core/gaol_interval_fpu.h` | 77 | 77 | 100.0 | 79.3 % |
| `gaol/core/gaol_interval_fpu_impl.h` | 244 | 244 | 100.0 | 93.9 % |
| `gaol/core/gaol_interval_fpu_kernels.h` | 47 | 47 | 100.0 | 71.7 % |
| `gaol/core/gaol_interval_hyperbolic.cpp` | 61 | 61 | 100.0 | 78.3 % |
| `gaol/core/gaol_interval_neon.h` | 118 | 118 | 100.0 | 100.0 % |
| `gaol/core/gaol_interval_neon_impl.h` | 222 | 222 | 100.0 | 100.0 % |
| `gaol/core/gaol_interval_sse.h` | 101 | 101 | 100.0 | 72.3 % |
| `gaol/core/gaol_interval_sse_kernels.h` | 52 | 52 | 100.0 | 56.2 % |
| `gaol/gaol_fpu.h` | 94 | 94 | 100.0 | 94.4 % |
| `gaol/gaol_fpu_fenv.h` | 41 | 41 | 100.0 | 100.0 % |
| `gaol/gaol_ieee1788.cpp` | 15 | 15 | 100.0 | 66.7 % |
| `gaol/gaol_ieee1788.h` | 116 | 116 | 100.0 | 61.2 % |
| `gaol/gaol_init_cleanup.cpp` | 19 | 19 | 100.0 | 50.0 % |
| `gaol/gaol_interval.cpp` | 46 | 46 | 100.0 | 58.8 % |
| `gaol/gaol_interval.h` | 5 | 5 | 100.0 | -- |
| `gaol/gaol_port.cpp` | 3 | 3 | 100.0 | -- |
| `gaol/gaol_port.h` | 52 | 52 | 100.0 | 100.0 % |
| `gaol/math/gaol_core_math_port.h` | 6 | 6 | 100.0 | 100.0 % |
| `gaol/math/gaol_double_op.h` | 30 | 30 | 100.0 | -- |
| `gaol/math/gaol_ldexp.h` | 23 | 23 | 100.0 | 100.0 % |
| `gaol/math/gaol_round.h` | 23 | 23 | 100.0 | 100.0 % |
| `gaol/math/gaol_roundeven.h` | 19 | 19 | 100.0 | 100.0 % |
| `gaol/math/gaol_u128.h` | 91 | 91 | 100.0 | 100.0 % |
| `gaol/parser/gaol_expression.h` | 91 | 91 | 100.0 | 61.5 % |
| `gaol/parser/gaol_parser.cpp` | 4 | 4 | 100.0 | 100.0 % |

## The lines the tests never run

- `gaol/parser/gaol_expr_eval.h`: 95, 143, 149, 157, 163, 175, 181, 189, 195, 201, 207, 213, 219, 225, 231, 237, 243, 249, 255, 261, 267, 273
- `gaol/core/gaol_interval_avx512_impl.h`: 582-583, 598, 609-610, 625, 629-630, 646-647, 652-653, 669-670, 675-677, 694-695, 720
- `gaol/parser/gaol_expression.cpp`: 362, 518, 520, 529, 531, 719, 721, 800, 802, 835, 837, 1010, 1012, 1043, 1045-1046, 1049, 1051, 1083, 1085, 1117, 1119, 1151, 1153, 1185, 1187, 1219, 1221, 1253, 1255, 1288, 1290, 1322, 1324, 1356, 1358, 1425, 1427, 1459, 1461, 1493, 1495, 1561, 1563, 1703, 1724-1725
- `gaol/core/gaol_interval_io.cpp`: 147, 337, 346, 382, 430-431, 433-435, 458, 460, 469-472, 474-479
- `gaol/gaol_common.cpp`: 133, 137, 157, 162
- `gaol/core/gaol_interval_sse_impl.h`: 47, 61, 869, 879-880, 897, 912-913, 928, 937-938, 953-954, 962-964, 979-981, 988-990, 1006-1007
- `gaol/gaol_common.h`: 308
- `gaol/core/gaol_interval_core.h`: 1895, 2235, 2237-2242, 2244, 2297-2298, 2300, 2322
- `gaol/core/gaol_interval_pow.cpp`: 188, 633, 672, 675, 726, 736, 781, 786, 794, 826, 836, 1027-1028, 1163, 1223, 1247, 1347
- `gaol/parser/gaol_eval_stack.h`: 102
- `gaol/math/gaol_fma.h`: 215, 226, 586-587
- `gaol/core/gaol_interval_detail.h`: 321
- `gaol/core/gaol_interval_trig.cpp`: 267, 271, 805, 844, 847
- `gaol/core/gaol_interval_core.cpp`: 319-321, 1492, 1505
- `gaol/math/gaol_compoundm1.h`: 189, 365-366, 1406-1407, 1460-1461
- `gaol/core/gaol_interval_neon_kernels.h`: 413
- `gaol/core/gaol_interval_internal.h`: 479
- `gaol/core/gaol_interval_relational.cpp`: 564-565

## CORE-MATH, file by file

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `3rd/math-core/src/binary64/log10p1/dint.h` | 124 | 0 | 0.0 | 0.0 % |
| `3rd/math-core/src/binary64/log2p1/dint_log2p1.h` | 124 | 0 | 0.0 | 0.0 % |
| `3rd/math-core/src/binary64/exp10m1/exp10m1.c` | 238 | 124 | 52.1 | 58.1 % |
| `3rd/math-core/src/binary64/atanh/atanh.c` | 155 | 91 | 58.7 | 46.9 % |
| `3rd/math-core/src/binary64/log10p1/log10p1.c` | 208 | 136 | 65.4 | 69.1 % |
| `3rd/math-core/src/binary64/log2p1/log2p1.c` | 226 | 152 | 67.3 | 73.3 % |
| `3rd/math-core/src/binary64/pow/pow.h` | 203 | 141 | 69.5 | 49.6 % |
| `3rd/math-core/src/binary64/atanpi/atanpi.c` | 148 | 106 | 71.6 | 59.3 % |
| `3rd/math-core/src/binary64/log10/dint.h` | 96 | 69 | 71.9 | 53.7 % |
| `3rd/math-core/src/binary64/exp10/exp10.c` | 132 | 97 | 73.5 | 63.5 % |
| `3rd/math-core/src/binary64/log/dint.h` | 96 | 73 | 76.0 | 58.5 % |
| `3rd/math-core/src/binary64/exp2/exp2.c` | 126 | 97 | 77.0 | 69.2 % |
| `3rd/math-core/src/binary64/exp2m1/exp2m1.c` | 223 | 173 | 77.6 | 74.2 % |
| `3rd/math-core/src/binary64/tanpi/tanpi.c` | 190 | 151 | 79.5 | 67.1 % |
| `3rd/math-core/src/binary64/cosh/cosh.c` | 152 | 123 | 80.9 | 61.9 % |
| `3rd/math-core/src/binary64/cospi/cospi.c` | 148 | 123 | 83.1 | 52.1 % |
| `3rd/math-core/src/binary64/pow/pow.c` | 500 | 417 | 83.4 | 66.7 % |
| `3rd/math-core/src/binary64/sinh/sinh.c` | 165 | 138 | 83.6 | 63.2 % |
| `3rd/math-core/src/binary64/tanh/tanh.c` | 150 | 127 | 84.7 | 61.1 % |
| `3rd/math-core/src/binary64/tan/tan.c` | 471 | 399 | 84.7 | 57.2 % |
| `3rd/math-core/src/binary64/atan2/atan2.c` | 165 | 141 | 85.5 | 60.0 % |
| `3rd/math-core/src/binary64/pow/dint.h` | 152 | 131 | 86.2 | 78.3 % |
| `3rd/math-core/src/binary64/asinh/asinh.c` | 175 | 152 | 86.9 | 68.8 % |
| `3rd/math-core/src/binary64/hypot/hypot.c` | 146 | 127 | 87.0 | 61.5 % |
| `3rd/math-core/src/binary64/expm1/expm1.c` | 155 | 136 | 87.7 | 74.1 % |
| `3rd/math-core/src/binary64/pow/qint.h` | 314 | 282 | 89.8 | 70.4 % |
| `3rd/math-core/src/binary64/asinpi/asinpi.c` | 249 | 226 | 90.8 | 68.1 % |
| `3rd/math-core/src/binary64/acosh/acosh.c` | 179 | 163 | 91.1 | 73.9 % |
| `3rd/math-core/src/binary64/erfc/erfc.c` | 302 | 277 | 91.7 | 86.4 % |
| `3rd/math-core/src/binary64/sinpi/sinpi.c` | 177 | 163 | 92.1 | 60.0 % |
| `3rd/math-core/src/binary64/log2/log2.c` | 93 | 86 | 92.5 | 86.4 % |
| `3rd/math-core/src/binary64/acospi/acospi.c` | 147 | 137 | 93.2 | 80.6 % |
| `3rd/math-core/src/binary64/log1p/log1p.c` | 153 | 145 | 94.8 | 83.3 % |
| `3rd/math-core/src/binary64/atan2pi/tint.h` | 214 | 203 | 94.9 | 69.2 % |
| `3rd/math-core/src/binary64/atan/atan.c` | 98 | 93 | 94.9 | 80.6 % |
| `3rd/math-core/src/binary64/atan2/tint.h` | 221 | 210 | 95.0 | 71.2 % |
| `3rd/math-core/src/binary64/exp/exp.c` | 114 | 110 | 96.5 | 87.5 % |
| `3rd/math-core/src/binary64/atan2pi/atan2pi.c` | 154 | 149 | 96.8 | 86.2 % |
| `3rd/math-core/src/binary64/cos/cos.c` | 136 | 132 | 97.1 | 89.5 % |
| `3rd/math-core/src/binary64/cbrt/cbrt.c` | 75 | 73 | 97.3 | 81.2 % |
| `3rd/math-core/src/binary64/log10/log10.c` | 127 | 124 | 97.6 | 67.9 % |
| `3rd/math-core/src/binary64/sin/sin.c` | 174 | 170 | 97.7 | 91.7 % |
| `3rd/math-core/src/binary64/asin/asin.c` | 107 | 106 | 99.1 | 82.1 % |
| `3rd/math-core/src/binary64/acos/acos.c` | 123 | 122 | 99.2 | 79.4 % |
| `3rd/math-core/src/binary64/erf/erf.c` | 151 | 150 | 99.3 | 97.5 % |
| `3rd/math-core/src/binary64/log/log.c` | 127 | 127 | 100.0 | 85.3 % |
| `3rd/math-core/src/binary64/rsqrt/rsqrt.c` | 69 | 69 | 100.0 | 93.8 % |

