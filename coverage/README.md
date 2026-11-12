<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-09-20)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
# Coverage of the tests

*Written by `coverage/coverage_summary.py` from the report of gcovr, which `cmake --build <build> --target coverage` runs (`-DGAOL_COVERAGE=ON`). Last run: 2026-11-12.*

The line-by-line report is [coverage.html](coverage.html), one page holding its own style.

## Conclusion

| | Lines | Covered | Not run | % | |
|---|---:|---:|---:|---:|---|
| **GAOL (`gaol/`)** | 6423 | 6106 | 317 | **95.1** | `███████████████████░` |
| CORE-MATH (`3rd/math-core/`) | 8172 | 6738 | 1434 | 82.5 | `████████████████░░░░` |

**The tests run 95.1 % of the lines of GAOL, above the 80 % GAOL v5 aims at.**

Branches: 68.4 % of those of GAOL are taken both ways.

The lines of CORE-MATH that are never run are its accurate phases, which a handful of arguments in a million reach, and the paths of the architectures this machine is not. They are counted among the lines not run, and are checked otherwise: the accurate path of `sin` is compared with the upstream sources compiled unchanged, and that of `cos` with its roundings computed by mpmath (see `doc/tests.md`); the other accurate phases are reached by the tests of their bounds only.

## GAOL, file by file

Sorted by coverage, least covered first.

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `gaol/parser/gaol_expr_visitor.h` | 99 | 9 | 9.1 | -- |
| `gaol/parser/gaol_expr_eval.h` | 173 | 151 | 87.3 | 40.5 % |
| `gaol/parser/gaol_expression.cpp` | 740 | 663 | 89.6 | 35.8 % |
| `gaol/core/gaol_interval_sse_impl.h` | 411 | 377 | 91.7 | 64.9 % |
| `gaol/parser/gaol_eval_stack.h` | 34 | 32 | 94.1 | 63.3 % |
| `gaol/core/gaol_interval_io.cpp` | 349 | 330 | 94.6 | 68.8 % |
| `gaol/gaol_common.h` | 21 | 20 | 95.2 | 66.7 % |
| `gaol/core/gaol_interval_core.h` | 385 | 372 | 96.6 | 72.9 % |
| `gaol/core/gaol_interval_pow.cpp` | 515 | 498 | 96.7 | 73.1 % |
| `gaol/core/gaol_interval_relational.cpp` | 700 | 677 | 96.7 | 70.3 % |
| `gaol/core/gaol_interval_detail.h` | 58 | 57 | 98.3 | 80.4 % |
| `gaol/core/gaol_interval_trig.cpp` | 350 | 345 | 98.6 | 87.6 % |
| `gaol/core/gaol_interval_core.cpp` | 374 | 369 | 98.7 | 69.3 % |
| `gaol/math/gaol_compoundm1.h` | 808 | 801 | 99.1 | 88.9 % |
| `gaol/core/gaol_interval_internal.h` | 237 | 236 | 99.6 | 85.1 % |
| `gaol/core/gaol_interval_exp_log.cpp` | 124 | 124 | 100.0 | 78.7 % |
| `gaol/core/gaol_interval_hyperbolic.cpp` | 52 | 52 | 100.0 | 82.7 % |
| `gaol/core/gaol_interval_sse.h` | 101 | 101 | 100.0 | 73.1 % |
| `gaol/core/gaol_interval_sse_kernels.h` | 47 | 47 | 100.0 | 58.3 % |
| `gaol/gaol_common.cpp` | 63 | 63 | 100.0 | 85.4 % |
| `gaol/gaol_fpu.h` | 82 | 82 | 100.0 | 94.4 % |
| `gaol/gaol_fpu_fenv.h` | 41 | 41 | 100.0 | 100.0 % |
| `gaol/gaol_ieee1788.cpp` | 15 | 15 | 100.0 | 66.7 % |
| `gaol/gaol_ieee1788.h` | 114 | 114 | 100.0 | 61.5 % |
| `gaol/gaol_init_cleanup.cpp` | 19 | 19 | 100.0 | 50.0 % |
| `gaol/gaol_interval.cpp` | 46 | 46 | 100.0 | 58.8 % |
| `gaol/gaol_interval.h` | 5 | 5 | 100.0 | -- |
| `gaol/gaol_port.cpp` | 3 | 3 | 100.0 | -- |
| `gaol/gaol_port.h` | 52 | 52 | 100.0 | 100.0 % |
| `gaol/math/gaol_core_math_port.h` | 6 | 6 | 100.0 | 100.0 % |
| `gaol/math/gaol_double_op.h` | 30 | 30 | 100.0 | -- |
| `gaol/math/gaol_ldexp.h` | 23 | 23 | 100.0 | 100.0 % |
| `gaol/math/gaol_roundeven.h` | 19 | 19 | 100.0 | 100.0 % |
| `gaol/math/gaol_u128.h` | 91 | 91 | 100.0 | 100.0 % |
| `gaol/parser/gaol_expression.h` | 232 | 232 | 100.0 | 58.2 % |
| `gaol/parser/gaol_parser.cpp` | 4 | 4 | 100.0 | 100.0 % |

## The lines the tests never run

- `gaol/parser/gaol_expr_visitor.h`: 81-170
- `gaol/parser/gaol_expr_eval.h`: 74, 122, 128, 136, 142, 154, 160, 168, 174, 180, 186, 192, 198, 204, 210, 216, 222, 228, 234, 240, 246, 252
- `gaol/parser/gaol_expression.cpp`: 176, 270, 273, 337, 339, 348, 350, 456, 458, 480, 533, 535, 614, 616, 649, 651, 824, 826, 847, 849-854, 857, 859-860, 863, 865, 897, 899, 931, 933, 965, 967, 999, 1001, 1033, 1035, 1067, 1069, 1102, 1104, 1136, 1138, 1170, 1172, 1239, 1241, 1273, 1275, 1307, 1309, 1375, 1377, 1510, 1513-1517, 1519-1522, 1524-1526, 1528, 1530-1533, 1535-1536, 1538
- `gaol/core/gaol_interval_sse_impl.h`: 47, 61, 73, 75-76, 78, 84, 86, 876, 882-883, 900, 915-916, 931, 940-941, 956-957, 965-967, 982-984, 991-993, 1009-1010, 1302, 1305-1307
- `gaol/parser/gaol_eval_stack.h`: 101, 137
- `gaol/core/gaol_interval_io.cpp`: 260, 269, 305, 353-354, 356-358, 381, 383, 392-400
- `gaol/gaol_common.h`: 302
- `gaol/core/gaol_interval_core.h`: 1871, 2198, 2200-2205, 2207, 2260-2261, 2263, 2285
- `gaol/core/gaol_interval_pow.cpp`: 188, 633, 672, 675, 726, 736, 780, 784, 791, 821, 829, 1019-1020, 1155, 1215, 1239, 1339
- `gaol/core/gaol_interval_relational.cpp`: 58, 69, 102-104, 106-108, 110, 112-113, 115-116, 118-119, 132, 153, 173-174, 176-177, 565-566
- `gaol/core/gaol_interval_detail.h`: 321
- `gaol/core/gaol_interval_trig.cpp`: 267, 271, 805, 844, 847
- `gaol/core/gaol_interval_core.cpp`: 312-314, 1449, 1462
- `gaol/math/gaol_compoundm1.h`: 189, 365-366, 1406-1407, 1460-1461
- `gaol/core/gaol_interval_internal.h`: 476

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
| `3rd/math-core/src/binary64/erfc/erfc.c` | 302 | 277 | 91.7 | 83.0 % |
| `3rd/math-core/src/binary64/sinpi/sinpi.c` | 177 | 163 | 92.1 | 60.0 % |
| `3rd/math-core/src/binary64/log2/log2.c` | 93 | 86 | 92.5 | 86.4 % |
| `3rd/math-core/src/binary64/log1p/log1p.c` | 153 | 142 | 92.8 | 79.6 % |
| `3rd/math-core/src/binary64/acospi/acospi.c` | 147 | 137 | 93.2 | 80.6 % |
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

