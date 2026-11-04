# Coverage of the tests

*Written by `coverage/coverage_summary.py` from the report of gcovr, which `cmake --build <build> --target coverage` runs (`-DGAOL_COVERAGE=ON`). Last run: 2026-11-04.*

The line-by-line report is [coverage.html](coverage.html), one page holding its own style.

## Conclusion

| | Lines | Covered | Not run | % | |
|---|---:|---:|---:|---:|---|
| **GAOL (`gaol/`)** | 5466 | 5153 | 313 | **94.3** | `███████████████████░` |
| CORE-MATH (`3rd/math-core/`) | 8172 | 6732 | 1440 | 82.4 | `████████████████░░░░` |

**The tests run 94.3 % of the lines of GAOL, above the 80 % GAOL v5 aims at.**

Branches: 66.5 % of those of GAOL are taken both ways.

The lines of CORE-MATH that are never run are its accurate phases, which a handful of arguments in a million reach, and the paths of the architectures this machine is not: they are covered by the comparison with the upstream sources instead (see `doc/tests.md`), not by counting lines.

## GAOL, file by file

Sorted by coverage, least covered first.

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `gaol/parser/gaol_expr_visitor.h` | 99 | 9 | 9.1 | -- |
| `gaol/parser/gaol_expr_eval.h` | 173 | 151 | 87.3 | 40.7 % |
| `gaol/parser/gaol_expression.cpp` | 740 | 663 | 89.6 | 35.8 % |
| `gaol/core/gaol_interval_sse.cpp` | 561 | 527 | 93.9 | 66.3 % |
| `gaol/parser/gaol_eval_stack.h` | 34 | 32 | 94.1 | 63.3 % |
| `gaol/gaol_common.h` | 21 | 20 | 95.2 | 66.7 % |
| `gaol/core/gaol_interval_core.h` | 405 | 392 | 96.8 | 69.9 % |
| `gaol/core/gaol_interval_core.cpp` | 2492 | 2418 | 97.0 | 74.0 % |
| `gaol/core/gaol_interval_sse.h` | 91 | 91 | 100.0 | 80.0 % |
| `gaol/gaol_common.cpp` | 63 | 63 | 100.0 | 85.4 % |
| `gaol/gaol_fpu.h` | 82 | 82 | 100.0 | 94.4 % |
| `gaol/gaol_fpu_fenv.h` | 41 | 41 | 100.0 | 100.0 % |
| `gaol/gaol_ieee1788.cpp` | 15 | 15 | 100.0 | 66.7 % |
| `gaol/gaol_ieee1788.h` | 119 | 119 | 100.0 | 63.6 % |
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
| `gaol/parser/gaol_expression.h` | 232 | 232 | 100.0 | 58.0 % |
| `gaol/parser/gaol_parser.cpp` | 4 | 4 | 100.0 | 100.0 % |

## The lines the tests never run

- `gaol/parser/gaol_expr_visitor.h`: 84-173
- `gaol/parser/gaol_expr_eval.h`: 77, 125, 131, 139, 145, 157, 163, 171, 177, 183, 189, 195, 201, 207, 213, 219, 225, 231, 237, 243, 249, 255
- `gaol/parser/gaol_expression.cpp`: 180, 274, 277, 341, 343, 352, 354, 460, 462, 484, 537, 539, 618, 620, 653, 655, 828, 830, 851, 853-858, 861, 863-864, 867, 869, 901, 903, 935, 937, 969, 971, 1003, 1005, 1037, 1039, 1071, 1073, 1106, 1108, 1140, 1142, 1174, 1176, 1243, 1245, 1277, 1279, 1311, 1313, 1379, 1381, 1514, 1517-1521, 1523-1526, 1528-1530, 1532, 1534-1537, 1539-1540, 1542
- `gaol/core/gaol_interval_sse.cpp`: 52, 66, 78, 80-81, 83, 89, 91, 1152, 1158-1159, 1176, 1191-1192, 1207, 1216-1217, 1232-1233, 1241-1243, 1258-1260, 1267-1269, 1285-1286, 1613, 1616-1618
- `gaol/parser/gaol_eval_stack.h`: 104, 140
- `gaol/gaol_common.h`: 305
- `gaol/core/gaol_interval_core.h`: 369, 2022, 2406, 2408-2413, 2415, 2445-2446, 2448
- `gaol/core/gaol_interval_core.cpp`: 287, 298, 331-333, 335-337, 339, 341-342, 344-345, 347-348, 361, 382, 402-403, 405-406, 726-728, 1027, 1761-1762, 1792, 1801, 1837, 1885-1886, 1888-1890, 1913, 1915, 1924-1932, 2463, 2940-2941, 3402-3404, 3406, 3574, 3613, 3616, 3667, 3677, 3721, 3725, 3732, 3762, 3770, 3959-3960, 4093, 4509, 4513, 4720, 4744, 6720, 6976, 7015, 7018

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
| `3rd/math-core/src/binary64/log1p/log1p.c` | 153 | 136 | 88.9 | 74.1 % |
| `3rd/math-core/src/binary64/pow/qint.h` | 314 | 282 | 89.8 | 70.4 % |
| `3rd/math-core/src/binary64/asinpi/asinpi.c` | 249 | 226 | 90.8 | 68.1 % |
| `3rd/math-core/src/binary64/acosh/acosh.c` | 179 | 163 | 91.1 | 73.9 % |
| `3rd/math-core/src/binary64/erfc/erfc.c` | 302 | 277 | 91.7 | 83.0 % |
| `3rd/math-core/src/binary64/sinpi/sinpi.c` | 177 | 163 | 92.1 | 60.0 % |
| `3rd/math-core/src/binary64/log2/log2.c` | 93 | 86 | 92.5 | 86.4 % |
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

