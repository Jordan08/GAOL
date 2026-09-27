# Coverage of the tests

*Written by `coverage/coverage_summary.py` from the report of gcovr, which `cmake --build <build> --target coverage` runs (`-DGAOL_COVERAGE=ON`). Last run: 2026-09-27.*

The line-by-line report is [coverage.html](coverage.html), one page holding its own style.

## Conclusion

| | Lines | Covered | Not run | % | |
|---|---:|---:|---:|---:|---|
| **GAOL (`gaol/`)** | 3684 | 3214 | 470 | **87.2** | `█████████████████░░░` |

**The tests run 87.2 % of the lines of GAOL, above the 80 % GAOL v5 aims at.**

Branches: 67.1 % of those of GAOL are taken both ways.

The lines of CORE-MATH that are never run are its accurate phases, which a handful of arguments in a million reach, and the paths of the architectures this machine is not: they are covered by the comparison with the upstream sources instead (see `doc/tests.md`), not by counting lines.

## GAOL, file by file

Sorted by coverage, least covered first.

| File | Lines | Covered | % | Branches taken |
|---|---:|---:|---:|---:|
| `gaol/gaol_profile.cpp` | 31 | 0 | 0.0 | -- |
| `gaol/s_nextafter.c` | 24 | 0 | 0.0 | 0.0 % |
| `gaol/sysdeps/gaol_exact_c99.h` | 5 | 0 | 0.0 | -- |
| `gaol/gaol_expr_visitor.h` | 96 | 4 | 4.2 | -- |
| `gaol/gaol_exceptions.cpp` | 21 | 5 | 23.8 | 16.7 % |
| `gaol/gaol_common.cpp` | 50 | 37 | 74.0 | 82.4 % |
| `gaol/gaol_expression.cpp` | 673 | 546 | 81.1 | 38.8 % |
| `gaol/gaol_eval_stack.h` | 11 | 10 | 90.9 | 75.0 % |
| `gaol/gaol_interval.cpp` | 1381 | 1277 | 92.5 | 69.9 % |
| `gaol/gaol_expr_eval.h` | 140 | 130 | 92.9 | 32.4 % |
| `gaol/gaol_interval_sse.cpp` | 563 | 527 | 93.6 | 69.2 % |
| `gaol/gaol_expression.h` | 168 | 158 | 94.0 | -- |
| `gaol/gaol_interval.h` | 179 | 178 | 99.4 | 84.6 % |
| `gaol/gaol_common.h` | 4 | 4 | 100.0 | -- |
| `gaol/gaol_double_op.h` | 30 | 30 | 100.0 | -- |
| `gaol/gaol_exceptions.h` | 5 | 5 | 100.0 | 50.0 % |
| `gaol/gaol_fpu.h` | 15 | 15 | 100.0 | 75.0 % |
| `gaol/gaol_fpu_fenv.h` | 21 | 21 | 100.0 | 50.0 % |
| `gaol/gaol_ieee1788.h` | 89 | 89 | 100.0 | 66.7 % |
| `gaol/gaol_init_cleanup.cpp` | 19 | 19 | 100.0 | 50.0 % |
| `gaol/gaol_interval_sse.h` | 66 | 66 | 100.0 | 90.6 % |
| `gaol/gaol_parser.cpp` | 2 | 2 | 100.0 | -- |
| `gaol/gaol_port.cpp` | 3 | 3 | 100.0 | -- |
| `gaol/gaol_port.h` | 2 | 2 | 100.0 | -- |
| `gaol/gaol_roundeven.h` | 18 | 18 | 100.0 | 100.0 % |
| `gaol/gaol_u128.h` | 68 | 68 | 100.0 | 100.0 % |

## The lines the tests never run

- `gaol/gaol_profile.cpp`: 57, 60-61, 83, 85-86, 88, 92-95, 98, 100, 109, 111-113, 115, 117-118, 120, 122-124, 126, 128-129, 131, 133, 136, 138
- `gaol/s_nextafter.c`: 38, 46-49, 51, 53-59, 61-62, 64-70, 73-74
- `gaol/sysdeps/gaol_exact_c99.h`: 38, 40, 43, 45-46
- `gaol/gaol_expr_visitor.h`: 81-82, 87-176
- `gaol/gaol_exceptions.cpp`: 42, 44-46, 56, 58, 62, 64, 69, 71, 74, 76-79, 81
- `gaol/gaol_common.cpp`: 125, 127-128, 130, 132-133, 135, 137-138, 140, 142-143, 160
- `gaol/gaol_expression.cpp`: 124, 197, 200, 242, 244, 253, 255, 278, 280-281, 315, 317-318, 365, 367, 391, 404, 406, 448, 450, 537, 539, 555, 574, 576, 594, 597, 616, 618, 635, 653, 655, 670, 706, 761, 763, 769-770, 772-773, 775, 777, 779-780, 782-783, 786, 788, 790-795, 798, 800-801, 804, 806, 840, 842, 876, 878, 893, 912, 914, 929, 948, 950, 965, 984, 986, 1001, 1020, 1022, 1038, 1057, 1059, 1074, 1093, 1095, 1129, 1131, 1147, 1183, 1202, 1204, 1219, 1238, 1240, 1255, 1274, 1276, 1291, 1310, 1312, 1327, 1346, 1348, 1412, 1414, 1481, 1484-1488, 1490-1493, 1495-1496, 1499, 1501-1504, 1506-1507, 1511, 1513-1516, 1518-1519
- `gaol/gaol_eval_stack.h`: 85
- `gaol/gaol_interval.cpp`: 276, 327, 523-527, 530-534, 563-566, 702, 710, 716, 724, 791, 794, 798-799, 801, 805, 816, 819, 823-824, 826, 830, 840, 843, 955, 1058-1060, 1062, 1074, 1155, 1158, 1215, 1219, 1226, 1255, 1263, 1384-1385, 1440, 1454-1455, 1458-1460, 1507, 1519, 1534, 1773, 1777, 1781, 1974, 1998, 2017, 2074, 2182-2183, 2185, 2276, 2278-2280, 2283-2285, 2287-2289, 2307, 2309-2313, 2347, 2435, 2487, 2509, 2520, 2548, 2652, 2771, 2809, 2812-2813, 2815, 2824, 2853, 3107, 3110, 3150, 3153, 3163, 3171
- `gaol/gaol_expr_eval.h`: 54-56, 65, 131-136
- `gaol/gaol_interval_sse.cpp`: 54, 64, 67-68, 70, 75, 77-78, 80, 82-83, 85, 91, 93, 98, 133-136, 570-571, 610, 720, 726, 729, 1042, 1069, 1073, 1094, 1098, 1120, 1124, 1392, 1395-1397
- `gaol/gaol_expression.h`: 831, 834-835, 1068, 1071-1072, 1074, 1076, 1079, 1081
- `gaol/gaol_interval.h`: 971

