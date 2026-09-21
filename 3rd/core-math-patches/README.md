<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-09-21 by Jordan NININ -->
# Fixes to propose to CORE-MATH

Four of the changes GAOL makes to the sources of CORE-MATH in `../math-core`
are fixes of CORE-MATH itself rather than adaptations to GAOL (see
[3rd/README.md](../README.md)). They are written here as patches against the
upstream sources, in upstream's own terms (its native 128-bit integer, no
`/* GAOL */` marker), ready to be proposed on
<https://gitlab.inria.fr/core-math/core-math> or to core-math@inria.fr.

| | |
| --- | --- |
| Against | upstream `master`, commit `03720fea065088b23dd7dd60546ea615a14c9e80` (21 September 2026), which still has the four defects; each patch applies there with `git apply` |
| GAOL's copy | commit `671f2c7355d76c670f59d714d41a99eb1cf620b6` (19 September 2026), where the six files are the same |

| Patch | Defect | Where it shows |
| --- | --- | --- |
| `0001-sinh-cosh-tanh-mask-0ull.patch` | `~0ul >> 12`, meant as the mask of the 52 low bits of a double, in `sinh.c`, `cosh.c` and `tanh.c` (ten times, on nine lines): `unsigned long` has 32 bits on Windows and on the 32-bit targets, where the mask has 20 bits. Upstream writes `~0ull` in its other sources | the tests of the hard cases of these three functions, on those targets |
| `0002-cospi-unsigned-shifts.patch` | `m`, a signed 64-bit integer holding the significand with its hidden bit, shifted left by 11 to 13 places in the test of an integer or half-integer argument of `cospi.c`: an overflow, undefined behaviour, which UBSan reports. `sinpi.c` and `tanpi.c` convert `m` to `uint64_t` first; this does too | UBSan, on every target |
| `0003-asinpi-128-bit-shift.patch` | `D`, the 64-bit integer `dc` times 2<sup>ss</sup>, made of its halves `dc << ss` and `dc >> (64 - ss)` in `asinpi_acc`, which is right for 0 < ss < 64 only: next to ±1, ss reaches 65 to 69, and both shifts are undefined behaviour, which UBSan reports. It is now `(u128)(i128)dc << ss`, the same value for 0 < ss < 64 | UBSan, on the arguments of the accurate phase next to ±1 |
| `0004-rsqrt-expect-64-bit.patch` | `__builtin_expect(ix.u, 1)` in `cr_rsqrt`, the 64 bits of x passed where `__builtin_expect` takes a `long`: on the targets where `long` has 32 bits, the high half is dropped, and a subnormal whose low 32 bits are 0 is taken for +0, its rsqrt being +∞. It is now `ix.u != 0` | `rsqrt(2^-1040)` to `rsqrt(2^-1024)` among the powers of 4 on i386, armhf and Windows with GCC or Clang |

## How they were checked

Each function was compiled from the upstream sources as they are and as
patched, and the two were compared over 1.6 million arguments in the four
rounding directions (any double, doubles of moderate magnitude, arguments next
to ±1, and the powers of 4 with their neighbours, subnormal ones included):

- on x86-64 with GCC 13, where `long` has 64 bits and the undefined shifts
  happen to give the intended values: the same bits everywhere, for the six
  functions;
- on i386 with Clang 18, which has a 128-bit integer there: the same bits for
  `sinh`, `cosh`, `tanh`, `cospi` and `asinpi`, the masks of the first three
  only being used on rare hard cases; for `rsqrt`, 6 029 differences, where the
  upstream function returns +∞ for a subnormal power of 4 and the patched one
  the right power of two, 2<sup>520</sup> for 2<sup>−1040</sup>.

In GAOL, the same changes are in `../math-core` since commits 9ed2aa7
(`cospi`), 7eecc73 (`asinpi`), 2c267a0 (`rsqrt`) and ecd72b8 (`sinh`,
`cosh`, `tanh`), and the continuous integration runs them with UBSan and on the
32-bit targets.
