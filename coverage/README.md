# Coverage of the tests

*Written by `scripts/coverage_summary.py` from the report of gcovr, which `cmake --build <build> --target coverage` runs (`-DGAOL_COVERAGE=ON`). Last run: 2026-09-20.*

The line-by-line report is [index.html](index.html).

## Conclusion

| | Lines | Covered | Not run | % | |
|---|---:|---:|---:|---:|---|
| **GAOL (`gaol/`)** | 3005 | 2619 | 386 | **87.2** | `█████████████████░░░` |
| CORE-MATH (`3rd/math-core/`) | 4543 | 3321 | 1222 | 73.1 | `███████████████░░░░░` |

**The tests run 87.2 % of the lines of GAOL, above the 80 % this fork aims at.**

Branches: 65.8 % of those of GAOL are taken both ways.

The lines of CORE-MATH that are never run are its accurate phases, which a handful of arguments in a million reach, and the paths of the architectures this machine is not: they are covered by the comparison with the upstream sources instead (see `doc/tests.md`), not by counting lines.

## GAOL, file by file

Sorted by coverage, least covered first.

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `gaol/gaol_profile.cpp` | 31 | 0 | 0.0 | -- |
| `gaol/s_nextafter.c` | 24 | 0 | 0.0 | 0.0 % |
| `gaol/sysdeps/gaol_exact_c99.h` | 5 | 0 | 0.0 | -- |
| `gaol/gaol_expr_visitor.h` | 84 | 6 | 7.1 | -- |
| `gaol/gaol_exceptions.cpp` | 21 | 5 | 23.8 | 16.7 % |
| `gaol/gaol_common.cpp` | 47 | 34 | 72.3 | 81.2 % |
| `gaol/gaol_parser.cpp` | 16 | 13 | 81.2 | 64.3 % |
| `gaol/gaol_expression.cpp` | 593 | 513 | 86.5 | 58.0 % |
| `gaol/gaol_interval.cpp` | 984 | 890 | 90.4 | 65.2 % |
| `gaol/gaol_interval_sse.cpp` | 557 | 520 | 93.4 | 70.7 % |
| `gaol/gaol_u128.h` | 70 | 68 | 97.1 | 100.0 % |
| `gaol/gaol_interval.h` | 143 | 141 | 98.6 | 79.2 % |
| `gaol/gaol_expr_eval.h` | 124 | 123 | 99.2 | 46.1 % |
| `gaol/gaol_common.h` | 4 | 4 | 100.0 | -- |
| `gaol/gaol_double_op.h` | 30 | 30 | 100.0 | -- |
| `gaol/gaol_eval_stack.h` | 11 | 11 | 100.0 | 100.0 % |
| `gaol/gaol_exceptions.h` | 5 | 5 | 100.0 | 50.0 % |
| `gaol/gaol_expression.h` | 144 | 144 | 100.0 | -- |
| `gaol/gaol_fpu.h` | 7 | 7 | 100.0 | 100.0 % |
| `gaol/gaol_fpu_fenv.h` | 21 | 21 | 100.0 | 50.0 % |
| `gaol/gaol_init_cleanup.cpp` | 15 | 15 | 100.0 | 37.5 % |
| `gaol/gaol_interval_sse.h` | 64 | 64 | 100.0 | 90.6 % |
| `gaol/gaol_port.cpp` | 3 | 3 | 100.0 | -- |
| `gaol/gaol_port.h` | 2 | 2 | 100.0 | -- |

## The lines the tests never run

- `gaol/gaol_profile.cpp`: 57, 60-61, 83, 85-86, 88, 92-95, 98, 100, 109, 111-113, 115, 117-118, 120, 122-124, 126, 128-129, 131, 133, 136, 138
- `gaol/s_nextafter.c`: 38, 46-49, 51, 53-59, 61-62, 64-70, 73-74
- `gaol/sysdeps/gaol_exact_c99.h`: 38, 40, 43, 45-46
- `gaol/gaol_expr_visitor.h`: 83-160
- `gaol/gaol_exceptions.cpp`: 42, 44-46, 56, 58, 62, 64, 69, 71, 74, 76-79, 81
- `gaol/gaol_common.cpp`: 96, 98-99, 101, 103-104, 106, 108-109, 111, 113-114, 131
- `gaol/gaol_parser.cpp`: 68-70
- `gaol/gaol_expression.cpp`: 120, 172, 175, 197, 217, 219, 228, 230, 253, 255-256, 286, 288-289, 336, 338, 362, 375, 377, 419, 421, 507, 509, 530, 532-535, 538, 540-541, 544, 546, 585, 587, 622, 624, 730, 732, 757, 759-764, 767, 769-770, 773, 775, 809, 811, 845, 847, 862, 881, 883, 917, 919, 953, 955, 989, 991, 1007, 1026, 1028, 1043, 1062, 1064, 1098, 1100, 1171, 1173, 1205, 1207, 1233, 1235, 1298, 1321
- `gaol/gaol_interval.cpp`: 237, 246, 295, 520-524, 527-531, 560-563, 703, 711, 717, 725, 755, 770, 773, 777-778, 780, 784, 795, 798, 802-803, 805, 809, 819, 822, 918, 1018-1020, 1022, 1034, 1115, 1118, 1175, 1179, 1186, 1215, 1223, 1295-1296, 1324, 1338-1339, 1342-1344, 1402, 1404-1406, 1409-1411, 1413-1415, 1433, 1435-1439, 1449, 1529, 1581, 1603, 1614, 1642, 1715, 1826, 1864, 1867-1868, 1870, 1879, 1908, 1924, 2169, 2172, 2213, 2216, 2226, 2234
- `gaol/gaol_interval_sse.cpp`: 42, 52, 55-56, 58, 63, 65-66, 68, 70-71, 73, 79, 81, 86, 121-124, 556-557, 586, 696, 702, 705, 1018, 1045, 1049, 1070, 1074, 1096, 1100, 1155, 1358, 1361-1363
- `gaol/gaol_u128.h`: 86, 90
- `gaol/gaol_interval.h`: 534, 536
- `gaol/gaol_expr_eval.h`: 65

## CORE-MATH, file by file

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `3rd/math-core/src/binary64/pow/qint.h` | 314 | 0 | 0.0 | 0.0 % |
| `3rd/math-core/src/binary64/pow/pow.h` | 204 | 69 | 33.8 | 13.2 % |
| `3rd/math-core/src/binary64/atanh/atanh.c` | 155 | 91 | 58.7 | 46.9 % |
| `3rd/math-core/src/binary64/pow/pow.c` | 494 | 295 | 59.7 | 41.2 % |
| `3rd/math-core/src/binary64/tanh/tanh.c` | 150 | 103 | 68.7 | 55.6 % |
| `3rd/math-core/src/binary64/log/dint.h` | 96 | 73 | 76.0 | 56.1 % |
| `3rd/math-core/src/binary64/atan2/atan2.c` | 165 | 131 | 79.4 | 49.1 % |
| `3rd/math-core/src/binary64/cosh/cosh.c` | 152 | 123 | 80.9 | 60.9 % |
| `3rd/math-core/src/binary64/cos/cos.c` | 429 | 349 | 81.4 | 52.3 % |
| `3rd/math-core/src/binary64/acosh/acosh.c` | 179 | 148 | 82.7 | 71.7 % |
| `3rd/math-core/src/binary64/sin/sin.c` | 431 | 359 | 83.3 | 58.0 % |
| `3rd/math-core/src/binary64/sinh/sinh.c` | 165 | 138 | 83.6 | 61.9 % |
| `3rd/math-core/src/binary64/tan/tan.c` | 471 | 399 | 84.7 | 57.2 % |
| `3rd/math-core/src/binary64/pow/dint.h` | 152 | 131 | 86.2 | 73.3 % |
| `3rd/math-core/src/binary64/asinh/asinh.c` | 175 | 152 | 86.9 | 66.7 % |
| `3rd/math-core/src/binary64/atan2/tint.h` | 222 | 194 | 87.4 | 56.8 % |
| `3rd/math-core/src/binary64/exp/exp.c` | 136 | 122 | 89.7 | 76.2 % |
| `3rd/math-core/src/binary64/atan/atan.c` | 98 | 93 | 94.9 | 80.6 % |
| `3rd/math-core/src/binary64/log/log.c` | 120 | 118 | 98.3 | 76.3 % |
| `3rd/math-core/src/binary64/asin/asin.c` | 109 | 108 | 99.1 | 83.3 % |
| `3rd/math-core/src/binary64/acos/acos.c` | 126 | 125 | 99.2 | 79.4 % |

