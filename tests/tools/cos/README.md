<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-27 by Jordan NININ -->
# Verification tools of the accurate path of cos

Part of the [tests](../../../doc/tests.md) of GAOL v5.

`cos_accurate_path()` of [tests/core_math.cpp](../../core_math.cpp) checks the
bounds of `cos` at 45 arguments that take the accurate path of CORE-MATH's
`cos.c` (`cos_large_accurate()`) in the upward rounding GAOL computes in. That
path computes with the 128-bit integer throughout and is ported to `gaol_u128`
([3rd/README.md](../../../3rd/README.md)): the rest of the test compares
CORE-MATH with itself, and cannot see a fault of that port.

These tools are not part of the build nor of `make test`: they are read and run
by whoever changes `cos.c`, `gaol/gaol_u128.h` or the table.

| File | What it does |
|---|---|
| `accurate_values.py` | Computes the correct roundings of cos(x), nearest, upward, downward and toward zero, with mpmath at 4,000 bits and exact rationals, apart from CORE-MATH and from GAOL. Without argument it reads the 45 rows of the table of `cos_accurate_path()`, recomputes each from its x and compares (exit status 1 on a difference). With `--rows X...` it prints the rows of the arguments given. |
| `mutate.py` | Mutation testing of the port: puts one deliberate error at a time in a line of `cos.c` that carries `/* GAOL */`, rebuilds the test and runs it. 17 errors, and a swap of the halves of the macro `U128` for the build with `-DGAOL_U128_EMULATION=ON`. |

## How the arguments were found

A copy of `cos.c` with counters in `cos_large_accurate()` (no other change,
compiled with `-O2 -frounding-math -ffp-contract=off`) was called in the upward
rounding on random arguments of every magnitude, from 2<sup>-16</sup> to the
largest double, and near the multiples of π/2. All the arguments beyond
2<sup>31</sup> take the accurate path, and about 0.6 % of those from
2<sup>-13</sup> to 2<sup>31</sup> do, in each of the four roundings alike (11,377
of 2 million in the upward one). The arguments kept cover the cases of the
path:

- the reduction of the argument rounding `k` up or not (the sign `neg` of the
  remainder `r`);
- the index `i1` of `t1 = i1 π/64` below, equal to and above 32;
- the three ways of combining `cos(z) cos(r)` and `sin(z) sin(r)`: their sum,
  their difference, and their difference with a change of sign, which happens
  only for `i1` = 32 and `neg` = 1, next to an odd multiple of π/2;
- every range of exponents of the table of 1/(2π) beyond 2<sup>31</sup> (17
  of them, `i` from 0 to 16 in `reduce_large_acc()`), `i` = -1 below
  2<sup>-12</sup> and `i` = 0 between;
- the four arguments of `exceptions()`, with both signs, which are not
  2<sup>-16</sup> or more but 2<sup>-23</sup> to 2<sup>-20</sup>: they take the
  accurate path in the three directed roundings only.

Two arguments of the group of 2<sup>29</sup> to 2<sup>31</sup>, near an odd
multiple of π/2, were added after a mutation (the V2 word of
`reduce_large_acc()` taking its low bits from the wrong word of the table) that
no other argument showed.

## What the mutation testing shows, and what it cannot

The 17 errors are, in order: the factor 8 of `r`; the `+ 5` of the exceptions;
the sum of the combination made a difference and the comparison of `c1u` and
`s1u` inverted; the count of zeros of `u128_tod()` on the low half; `shr(v, 64)`
of `reduce_large_acc()` made 63; the `+ 1` of `mask + 1` dropped; the sum and the
difference swapped in `s1u` and in `c1u`; the mask of `k + 1` dropped; the halves of
`gaol_u128_make(V0, V1)` swapped; the round bit and the shift of the 53 bits of
`u128_tod()` moved by one; the 13 leading bits not cleared; the V2 word taken
from `_T[i+2]`; the shift by `f` always made; the sign of the result not
changed for `i1 >= 32`. Run on the 45 arguments, with the native type and with
the halves forced, the test fails for 15 of the 17; the halves of `U128`
swapped is found with the halves.

The two it does not find do not change a result the test could show. `k` is
read through its bits 0 to 12 only, so its mask changes nothing; and a `+ 1`
dropped from `mask + 1` moves the remainder by 2<sup>-128</sup>, which
no argument of the doubles shows (an independent review compared 37 errors of
this kind, 29 of which the test finds: of the 8 that pass, one changes no
result and the others are errors of a high-degree term of the polynomials, of
the bit `neg` and of `mask + k`, which change `cos` by less than about
2<sup>-70</sup> relatively, and over 10<sup>8</sup> random arguments in the
four roundings it found no difference for 5 of them). The test finds a fault that moves a result by more than that;
the only hard-to-round arguments of the table are those of `exceptions()`,
whose `k` and `neg` are 0.
