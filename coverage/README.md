# Coverage of the tests

*Written by `scripts/coverage_summary.py` from the report of gcovr, which `cmake --build <build> --target coverage` runs (`-DGAOL_COVERAGE=ON`). Last run: 2026-09-20.*

The line-by-line report is [coverage.html](coverage.html), one page holding its own style.

## Conclusion

| | Lines | Covered | Not run | % | |
|---|---:|---:|---:|---:|---|
| **GAOL (`gaol/`)** | 3102 | 2716 | 386 | **87.6** | `██████████████████░░` |

**The tests run 87.6 % of the lines of GAOL, above the 80 % GAOL v5 aims at.**

Branches: 66.5 % of those of GAOL are taken both ways.

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
| `gaol/gaol_expression.cpp` | 596 | 516 | 86.6 | 58.0 % |
| `gaol/gaol_interval.cpp` | 1058 | 961 | 90.8 | 65.9 % |
| `gaol/gaol_interval_sse.cpp` | 557 | 521 | 93.5 | 71.0 % |
| `gaol/gaol_interval.h` | 161 | 159 | 98.8 | 80.6 % |
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
| `gaol/gaol_roundeven.h` | 18 | 18 | 100.0 | 100.0 % |
| `gaol/gaol_u128.h` | 54 | 54 | 100.0 | 100.0 % |

## The lines the tests never run

- `gaol/gaol_profile.cpp`: 57, 60-61, 83, 85-86, 88, 92-95, 98, 100, 109, 111-113, 115, 117-118, 120, 122-124, 126, 128-129, 131, 133, 136, 138
- `gaol/s_nextafter.c`: 38, 46-49, 51, 53-59, 61-62, 64-70, 73-74
- `gaol/sysdeps/gaol_exact_c99.h`: 38, 40, 43, 45-46
- `gaol/gaol_expr_visitor.h`: 83-160
- `gaol/gaol_exceptions.cpp`: 42, 44-46, 56, 58, 62, 64, 69, 71, 74, 76-79, 81
- `gaol/gaol_common.cpp`: 96, 98-99, 101, 103-104, 106, 108-109, 111, 113-114, 131
- `gaol/gaol_parser.cpp`: 68-70
- `gaol/gaol_expression.cpp`: 120, 193, 196, 218, 238, 240, 249, 251, 274, 276-277, 307, 309-310, 357, 359, 383, 396, 398, 440, 442, 528, 530, 551, 553-556, 559, 561-562, 565, 567, 606, 608, 643, 645, 751, 753, 778, 780-785, 788, 790-791, 794, 796, 830, 832, 866, 868, 883, 902, 904, 938, 940, 974, 976, 1010, 1012, 1028, 1047, 1049, 1064, 1083, 1085, 1119, 1121, 1192, 1194, 1226, 1228, 1254, 1256, 1319, 1342
- `gaol/gaol_interval.cpp`: 237, 246, 295, 520-524, 527-531, 560-563, 703, 711, 717, 725, 755, 770, 773, 777-778, 780, 784, 795, 798, 802-803, 805, 809, 819, 822, 918, 1018-1020, 1022, 1034, 1115, 1118, 1175, 1179, 1186, 1215, 1223, 1347-1348, 1403, 1417-1418, 1421-1423, 1470, 1482, 1497, 1593, 1595-1597, 1600-1602, 1604-1606, 1624, 1626-1630, 1640, 1720, 1772, 1794, 1805, 1833, 1906, 2017, 2055, 2058-2059, 2061, 2070, 2099, 2115, 2360, 2363, 2404, 2407, 2417, 2425
- `gaol/gaol_interval_sse.cpp`: 42, 52, 55-56, 58, 63, 65-66, 68, 70-71, 73, 79, 81, 86, 121-124, 556-557, 586, 696, 702, 705, 1018, 1045, 1049, 1070, 1074, 1096, 1100, 1358, 1361-1363
- `gaol/gaol_interval.h`: 535, 537
- `gaol/gaol_expr_eval.h`: 65

