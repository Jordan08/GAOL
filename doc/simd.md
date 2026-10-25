<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-20 by Jordan NININ -->
# SIMD, FMA and the rounding direction

Part of the documentation of [GAOL v5](../README.md#documentation).

This page evaluates the tracks listed by [issue #101](https://github.com/Jordan08/GAOL/issues/101):
what GAOL computes with vector instructions and with the fused multiply-add
today, where the time of a program goes, what the processors of other families
offer to get a rounding direction without writing a control register, and which
of these tracks is worth a path of its own (GAOL v5). A number comes with its
source or its machine; what no machine of ours could run is said in
[What was not measured](#what-was-not-measured).

## In short

- The two bounds of an interval are one vector of two doubles, and
  `+ - * / sqr` (and `sqrt` where it has a kernel) are one or a few vector
  instructions: **a wider register does not help one interval**. Only a batch
  interface (boxes, arrays of intervals), which GAOL has not, could use 256 or
  512 bits.
- What costs is **the rounding direction**. By default an operation writes the
  control register (MXCSR, FPCR, FPSCR...) only when the direction of the
  program is not upward already; with `GAOL_PRESERVE_ROUNDING` and a program
  that computes to nearest, it writes it twice, at about 3 ns a write on x86
  and 4 to 11 ns on an Apple M2, against one or two ns for the operation. The
  paths that avoid it carry the direction in the instruction: the AVX-512 path
  of x86 (scalar and 512-bit forms) does, and, of the processors read here, the
  RISC-V scalar instructions are the only other ones that could.
- **No other instruction set read here offers it for a vector of two doubles.**
  AVX-512VL and AVX10.2 have no embedded rounding at 128 or 256 bits (AVX10.2
  had it for 256 bits and withdrew it in May 2025). AArch64 has none, with NEON
  or with SVE. POWER, s390x, LoongArch, MIPS and WebAssembly have none either.
- **No compiler gives a rounding direction by instruction** from C or C++:
  `#pragma STDC FENV_ROUND` is ignored by GCC and by Visual C++ and not
  supported by Clang, and the C++ proposals are not adopted. The paths are
  written with intrinsics or inline assembly.
- What is left to try, by order of interest: the RISC-V scalar path with a
  static rounding mode (needs a real board), the scalar embedded rounding for
  the division on the Intel processors that halve the 512-bit registers (the
  square root takes it already), and a few kernels that are scalar today (see
  [Cheap gains](#cheap-gains-that-need-no-new-instruction-set)).

## What GAOL computes with vectors today

An interval keeps its bounds as `<-left, right>` in one vector of two doubles
(the FPU intervals keep the same two numbers in two doubles), and computes in
the rounding direction upward: the lower bound is minus the upper bound of the
opposite, so that one addition rounded upward gives both bounds
([accuracy](accuracy.md)).

| Build | Vector kernels | Scalar |
|---|---|---|
| SSE2 (x86, `GAOL_SIMD`) | `+ - * / sqr`, the operations with a double, `inverse`, `%=`, `-x`, the integer powers with `__FMA__`, the leaves of `div_rel` | `sqrt`, `&=`, `\|=`, `mid`, `midpoint`, the relations, the elementary functions, the reverse functions |
| AVX-512 (`GAOL_PREFER_AVX512`) | `+ - * / sqr`, the operations with a double and the integer powers (with `__FMA__`), with the rounding direction in the instruction; `sqrt` as two scalar square roots with embedded rounding | as SSE2, and `inverse`, `%=` and the leaves of `div_rel`, which stay SSE2 kernels that set the direction |
| NEON (arm64, `GAOL_ARM64`; Linux, macOS and Visual C++) | `+ - * / sqr sqrt`, the operations with a double, `%=`, `inverse`, `&=`, `\|=`, `-x`, the integer powers (two lanes with `vfmaq_f64`) | `mid`, `midpoint`, `div_rel`, the elementary functions |
| FPU (the rest) | none | everything |

The AVX-512 path is off by default: `GAOL_PREFER_AVX512` is on by default only
with `GAOL_PRESERVE_ROUNDING`, and the library then needs a processor with the
instructions ([three builds](three-builds.md)). The timings of the AVX-512
path below are those of that configuration. For `+ - * /` and `sqr` it puts the
two bounds in two of the eight lanes of a 512-bit register, for the embedded
rounding only.

CORE-MATH (`3rd/math-core`; GAOL calls 38 of its binary64 functions) is scalar,
with no vector variant; its functions are called once per bound. `GAOL_FMA` pays
in them and in the integer powers of GAOL
([three builds](three-builds.md#what-the-options-are-worth): `cos` 2.5 times
faster, `pow(x, 3)` 3.2 times), and does nothing for the arithmetic.

## Where the time of a program goes

The call counts below are exact, from a NEON build with
`-finstrument-functions`. The shares of the time are indicative to about 3
points: one Apple M2 (NEON intervals), the profiler `sample` of macOS at 1 ms
during 3 s per program, each sample put to the nearest GAOL function above it,
other work running on the machine; the two passes of the profile differ by up
to 3 points, and the table gives the second.

| Program | Calls (`+ - * /`, `sqr`, powers) | Elementary calls | Arithmetic | Elementary functions | Outside GAOL |
|---|---|---|---:|---:|---:|
| 03 dependency problem | 0.14 M, 26 k | none | 73 % | 0 | 16 % |
| 04 centered form | 10 k | cos 10 k, acos 8 k | 9 % | 82 % | 6 % |
| 06 global optimization | 25 M, 5 M, 0.49 M | none | 31 % | 0 | 63 % |
| 08 SIVIA | 1.0 M, 0.5 M | sin 173 k | 14 % | 12 % | 71 % |
| 09 estimation | 0.5 M, 68 k | exp 247 k | 20 % | 42 % | 31 % |
| 11 ODE | 0.2 M, 8 k, 4.9 k | sin 12 k, cos 3 k, exp 1 k | 29 % | 34 % | 30 % |
| 16 Goldstein-Price | 32 M, 6 M powers | none | 98 % | 0 | 2 % |

- On the six solvers 06 to 11, the arithmetic is 28 % of the time weighted by
  the duration of a run (20 % with equal weights), the elementary functions 4 %
  (20 %), and the code outside GAOL (the priority queue, `malloc` of the
  boxes, the program itself) 62 % (45 %). The weighted figures are those of 06,
  which lasts 80 % of the total. Inside GAOL, at equal weights, about 40 % is
  arithmetic, 34 % the elementary functions, 10 % the reading and writing of
  text and 7 % `mid()`.
- **The code of GAOL around CORE-MATH weighs 21 to 66 % of the time of an
  elementary function** (median about 42 %): the check of the rounding
  direction, the reduction of the argument, the construction of the result. In
  the code of `exp` around the CORE-MATH call, 43 % of the samples fall on the
  line of the check of the direction, a `mrs` of 0.8 ns: the samples probably
  land on the instruction that waits for it. The CORE-MATH calls themselves
  cost 3 to 13 ns on the M2.
- The relations, `hull`, `intersect`, `min` and `max` are 3.6 % or less in
  every program; `mid()` is 6 % of the global optimization, and `midpoint()`
  11 to 12 % of the Krawczyk iteration (21 to 28 ns on the M2: two writes of
  FPCR).
- The check of the direction (one `mrs` of FPCR here), removed in a copy,
  saves 0.8 % (03), 1.4 % (09), 4.7 % (06) and 8.3 % (16).
  `GAOL_PRESERVE_ROUNDING` on the M2, for a program that computes to nearest,
  makes 03 and 16 about 16 times slower, 06 eight times, 09 five times. On x86
  with the AVX-512 path the ratio is 1.0 to 1.5
  ([performance](compare/performance.md)).
- The text is 41 to 69 % of the five demonstration programs that print (01,
  02, 05, 12, 15; 28 to 31 % of 14 and 11 % of 03), at 0.2 to 0.45 µs per bound
  written. Among the solvers it is 0 to 1 % of 06, 08, 09 and 11, 12 % of 10 and
  22 % of 07.

On an Intel i7-1185G7 ([performance](compare/performance.md): a million
operations on arrays of intervals, so partly bound by the memory), the time
of an operation is 2.2 ns for `+` and `-`, 2.30 for `*`, 2.52 for `/`, 1.87
for `sqr`, 6.30 for `sqrt` and 5.12 for `pow(x, 3)`, the same operation on
doubles taking 0.96 ns for `+`. With `GAOL_PRESERVE_ROUNDING` and the AVX-512
path: 2.19, 2.16, 3.42, 3.60, 2.04, 3.75 and 6.18. On the M2
([three builds](three-builds.md#what-the-options-are-worth)) the NEON
intervals take 0.91 ns for `+`, 2.14 for `*`, 2.39 for `/`, 1.48 for `sqr`
and 1.70 for `sqrt`, against 1.18, 3.33, 3.30, 1.82 and 4.60 with the FPU
intervals, whose `*`, `/` and `sqr` take no branch on the signs
([the FPU intervals](using.md#the-fpu-intervals); 1.16, 3.37, 4.00, 2.08 and 4.51
before).

## The rounding direction, by processor

Except on the AVX-512 path, an operation computes upward after a write of the
control register of the processor when the direction of the program is not
upward already. What a write costs:

- **x86**: `LDMXCSR` takes 1 to 6 cycles of throughput on the performance
  cores of Intel and on AMD processors (1.0 on Zen 5, 6.0 on an Arrow Lake P-core)
  and 8 on the efficiency cores, according to uops.info, which measures one
  instruction at a time and so does not count the wait for the operations in
  flight; `STMXCSR`, the read, takes 1 cycle on most Intel cores but **14 to 16
  on Zen 2 to Zen 5**, one more reason, with those given in `gaol/gaol_fpu.h`
  (800 ns under Rosetta 2, 80 ns with 32-bit Visual C++, a single read reused
  by Clang 18), why the check of GAOL on x86 is an addition and not a read. Two
  writes took 6 ns a `sqr` (8.3 ns, 4.4 times) before the AVX-512 path
  ([performance](compare/performance.md)).
- **64-bit ARM**: `mrs` of FPCR takes 0.8 ns, `msr` 4 ns for the value FPCR
  holds and 11 ns when it changes the direction (Apple M2). The reference
  manuals of the Neoverse N1, N2, N3 and of the Cortex-A78 say that a write
  that changes the control fields introduces a barrier or a flush.
- **32-bit ARM, POWER, s390x, LoongArch, MIPS, RISC-V**: no figure found in
  the references.

What each instruction set offers instead, read from its reference (the
versions are in [Sources](#sources)):

| Processor | Rounding direction carried by the instruction | FMA | Register | What a path could be |
|---|---|---|---|---|
| x86-64 SSE2 | no | FMA3, a set of its own: **AVX2 does not imply it** | MXCSR (RC, FTZ, DAZ) | the SSE2 path of today |
| x86-64 AVX-512F | **yes**, `{er}`, for the scalar instructions and the 512-bit ones | yes | MXCSR: RC and the exception flags are ignored, FTZ and DAZ are honoured | the AVX-512 path of today |
| x86-64 AVX-512VL, AVX10.1, AVX10.2 | **no at 128 and 256 bits**: `VADDPD ymm` has no `{er}` in the Intel manual; AVX10.2 had it for 256 bits in revisions 1 to 3 and removed it in revision 4.0 (8 May 2025), and revision 7.0 (6 April 2026) keeps it for 512 bits only | yes | MXCSR | none; a vector of two doubles has to go through the 512-bit form, as the AVX-512 path does |
| AArch64, NEON, SVE, SME | **no**: only `FCVT*`, `FRINT*` and `FCVTX*` name a rounding in the A64 instruction set of March 2026; `FADD`, `FMUL`, `FDIV`, `FSQRT`, `FMLA` use FPCR, scalar, Advanced SIMD and SVE | yes | FPCR (RMode, FZ; FEAT_AFP adds FIZ, AH and NEP) | the NEON path of today; SVE has 128-bit vectors on the Neoverse N2, N3, V2, V3 and the Cortex-A720, 256 on the V1, and adds no rounding |
| ARMv7 | no; NEON has no double, and always rounds to nearest | VFPv4 only | FPSCR (RMode, FZ) | the FPU intervals |
| POWER (VSX) | no: `xsadddp` and `xvadddp` use FPSCR.RN; the `RO` bit exists for the quad precision only (round to odd) | yes | FPSCR (RN, NI) | VSX `<-l, r>` with `mffscrni`, needs hardware to measure |
| s390x | no: `ADB` and `VFA` have no rounding field; the `M3` field of the conversions is not the arithmetic | yes | FPC (BRM) | `VFADB`, needs hardware to measure |
| RISC-V, scalar (D) | **yes**: the `rm` field of `FADD.D`, `FMUL.D`, `FDIV.D`, `FSQRT.D`, `FMADD.D`...; `rup` is 011 | yes | frm (no flush-to-zero mode) | two scalar instructions, no check, no register to restore; inline assembly only |
| RISC-V, vector (V) | **no**: all the vector floating-point operations use frm; the `_rm` intrinsics compile to `fsrmi` and `fsrm` around the operation | yes | frm | none |
| LoongArch | no | yes | FCSR | none |
| MIPS | no (read from the Linux headers and the compiler output, not from the manual) | not verified | FCSR (RM, FS) | none |
| WebAssembly 3.0 | no: "Non-default directed rounding attributes are not supported" | `relaxed_madd` may not be fused | none | none: only the rounding to nearest |

The RISC-V scalar path is the one case of a rounding direction by instruction
that GAOL has not. Its interest is that it needs no check and no restore
(`GAOL_PRESERVE_ROUNDING` would cost nothing), as the AVX-512 path. No figure
exists for it: no RISC-V board is available. The boards read about (secondary
sources): the SpacemiT K1 has the vector extension 1.0, the SiFive P550 none,
the T-Head C910 the draft 0.7.1.

## What the compilers give

- **`#pragma STDC FENV_ROUND`** (C23) would ask for a static rounding. On the six
  targets tried (x86-64, aarch64, riscv64, ppc64le, s390x, loongarch64), GCC 14
  to 16 and its trunk ignore it, with a warning only under `-Wall` (GCC bug
  20785, open since 2005); Clang 18 to its trunk parses it, warns "not
  supported" and emits the ordinary instruction (the IR carries `round.upward`;
  no back end of the three tried, x86-64, aarch64 and riscv64, uses it); Visual
  C++ warns C4068. It does not exist in C++. A program that relies on it
  compiles and rounds in the wrong direction, without an error.
  `-Werror=unknown-pragmas` makes Clang's "not supported" an error, but not its
  other forms of the same pragma ("expected identifier" with the `FE_UPWARD` of
  `<fenv.h>`, which is a number; "not supported on this target"), which belong
  to another group of warnings, and `/we4068` should do it for Visual C++: none
  of these was tried.
- **C++**: P2746R7 (Boehm, January 2025) would replace `fesetround` by a type
  with an explicit mode, without wording. P4231R0 (May 2026, target C++29)
  unifies it with P3864; no revision since. Nothing is in C++26, and none
  proposes a SIMD form.
- **Intrinsics of the embedded rounding**: `_mm_add_round_sd` and
  `_mm512_add_round_pd` (and the other operations) compile with GCC 14 to its
  trunk, Clang 18 to its trunk and Visual C++ 19.20 to 19.51. The mode must be a
  literal, and the compilers do not fold a constant computed with `{ru-sae}`.
  There is no `_mm_add_round_pd`, and no `_mm256_add_round_pd` with GCC 15 and
  16 or Clang 21 and later; **Clang 20 with `-mavx10.2` declares it** and
  compiles it to the encoding of the withdrawn revisions of AVX10.2, which the
  current Intel manual makes `#UD` (never run: no processor with AVX10.2 is on
  sale, as far as we found). **In inline assembly**, the GNU `as` of GCC 16.2 on
  Compiler Explorer assembles `vaddpd {ru-sae}, %ymm1, %ymm0, %ymm2` (bytes
  `62 f1 f9 58 58 d1`, against `62 f1 fd 58 58 d1` for the 512-bit form): no
  assembly of that form may be written.
- **`-frounding-math`** (see [three builds](three-builds.md#interprocedural-optimization-is-not-offered)):
  GCC does not honour it across a change of the direction (bugs 34678 and 123482,
  open): on `fesetround(UP); r = a + b; fesetround(old)`, GCC 14 to its trunk
  moves the addition after the restore, on x86-64 and on AArch64. Clang keeps it
  between. The vector intrinsics are ordinary floating-point operations for
  them: with Apple clang 21 on the M2, at `-O2` with `-frounding-math` or
  `-ffp-model=strict`, `vaddq_f64` and `vfmaq_f64` are right, **`vpaddq_f64`,
  `vaddvq_f64` and `vmulxq_f64` are computed once, in the first direction**, and
  at `-O2` alone everything is. (GAOL uses none of these three.) What held on
  every compiler tried (GCC 14 to its trunk and Clang 18 to its trunk, compiled;
  Apple clang 21, run on the M2) is **an empty assembly statement on each
  operand after the write of the control register, and on the result, or a copy
  of it through volatile memory, before the next one**. GAOL does that:
  `reread_bounds()` and `rnd_reread()` are empty `asm` statements,
  `GAOL_RND_KEEP()` is a copy through volatile memory with SSE2 and the FPU and
  an empty `asm` with NEON, and `GAOL_RND_BARRIER()` is a memory barrier. A path
  written with intrinsics needs the same. Visual C++ for arm64 has no `asm`
  statement: its NEON intervals copy the result through volatile memory,
  `GAOL_RND_BARRIER()` and the writes of FPCR are `_ReadWriteBarrier()`, and
  the operands rely on `/fp:strict` (checked by the generated code and by
  `tests/rounding_direction.cpp`, not by the language).
- **Intrinsics of the other families**: NEON `float64x2_t` with GCC, Clang and
  Visual C++ for arm64 from 19.26; SVE `<arm_sve.h>` with GCC 14 and Clang 18, and
  in Visual C++ 19.50 and 19.51 only partially (`svmla_f64_x` becomes an
  unresolved call without a diagnostic); VSX `vec_add` with `-mvsx`; RVV with GCC
  from 14.1 and Clang from 17.0.1. `std::experimental::simd` is in libstdc++
  (libc++ has the types but no `operator+`), and `std::simd` of C++26 only in
  GCC 16, for x86-64.

## Verdicts

| Track | Verdict | Why | What would change it |
|---|---|---|---|
| SSE4.1, AVX, AVX2 (a 256-bit path) | no path | A batch of two intervals per register needs an interface of arrays, and nothing makes one interval faster; `ROUNDPD` rounds to an integer, not as an operation | An interface of boxes or arrays of intervals, and a program bound by the computation and not by the memory |
| FMA3 | in use (`GAOL_FMA`) | It pays in the elementary functions and the integer powers, none in the basic operations | The exact transformations that would replace the directed rounding are another study (the rounding direction left to nearest) |
| AVX-512 path | keep; try the scalar embedded rounding for `/` | On Intel the 512-bit division takes 16 cycles of throughput against 4 for the 128-bit one (uops.info), and Zen 5 has full-width units; the square root of the path already takes two scalar embedded roots (3.75 ns against 6.30 with SSE2) | Two `vdivsd {er}` and the permutations losing on an Intel processor that halves the registers |
| AVX-512VL, AVX10.2 at 128 and 256 bits | nothing to build | No embedded rounding at these widths | A new revision of the Intel manual restoring it (revision 3.0 of AVX10.2 had it) |
| NEON (arm64) | keep | Done: 1.3 to 4 times faster on an M2 | Hardware with FPCR.AH set (see below) |
| SVE, SVE2, SME | no path | The same FPCR, and 128-bit vectors on most of the processors | An interface of arrays, on a 256 or 512-bit implementation; a static rounding in the instruction set |
| ARMv7 | no vector path | NEON has no double | none |
| POWER (VSX), s390x (vector facility) | measure first | Two doubles per register and a write of the control register, as NEON, but no hardware here and no figure for the write | A machine to run the tests and the benchmark on |
| RISC-V, scalar | measure on a board, then write it | The only other rounding direction by instruction, by inline assembly | A board, and a check that its cores do not slow the static modes |
| RISC-V vector, LoongArch, MIPS, WebAssembly | no path | No static rounding for the vectors, no user, or the rounding to nearest only | An extension with a static rounding; for WebAssembly, a lifting of its limit |

## Cheap gains that need no new instruction set

Candidates seen in the sources and the profiles; none is decided but the first,
which is done, and none needs a new path.

- **`pow(x, 2)` through `sqr`** (done, GAOL v5): `gaol_pown` took 5.2 ns where
  `sqr` takes 1.3 on the M2 (NEON intervals; 5.2 and 1.8 with the FPU ones,
  whose `sqr` has no branch on the signs), and the integer powers are about
  40 % of Goldstein-Price (6 million calls). It takes `sqr` now, for n = 2 in
  every form of the integer power (`gaol_pown()` before its check of the
  rounding direction, `pown_upward()` after the check of its callers): 1.6 ns
  with the NEON intervals and 1.9 with the FPU ones, the call into the library
  accounting for the 0.3 ns of NEON above `sqr` (0.1 with the FPU ones) (the
  medians of 7 passes alternated with the library before the change,
  `tests/performance.cpp`, an Apple M2, AppleClang 21). The other powers cost
  0.3 to 0.5 ns more with the FPU intervals for n = 3, 4, 5 and 10 (+5 %),
  `gaol_pown()` now jumping to `pown_checked()`, and less with the NEON ones
  (15 alternations of a program of its own). On 14 million cases (3 million
  intervals, the special bounds, the empty set and the universe, in the four
  directions of the calling code and with flush-to-zero set) the bounds of
  `pow(x, 2)` are the same as before, the sign of a zero lower bound aside,
  which is that of `sqr(x)` now: +0 to -0 for 3.3 million cases and -0 to +0
  for 0.4 million with the NEON intervals, -0 to +0 for 3.6 million with the
  FPU ones; the old `pow` did not give +0 for every interval holding 0, as the
  first check thought it did (the program of the checks is not in the
  repository).
- **`sqrt` on SSE2** is scalar (6.3 ns on the i7-1185G7, 3.75 with the AVX-512
  path and the direction preserved, 1.7 on the M2 with NEON); the NEON kernel
  ports to `_mm_sqrt_pd`. `sqrt_rel` follows.
- **The integer powers on NEON** (done, GAOL v5, a small gain): the kernel of
  `pown` on two lanes with `vfmaq_f64` (`pown_neon_ops` and `pown_large_inline()`
  in `gaol/gaol_interval_neon.cpp`, 80 lines), as the SSE2 one with `__FMA__`
  (`gaol_interval_sse.cpp`), where it was two scalar lanes with `std::fma`; the
  generic part, `pown_products()`, `pown_powers()` and `pown_point()`, is in
  `gaol_interval_core.cpp`, and each kind of intervals defines the operations
  of its lanes as a class and its `pown_large_inline()`.
  The bounds are the same bit for bit (4 million powers hashed, and
  `tests/tools/pow/diffpow.py` on 70 500 lines). The gain is far below the 1.5
  times that was hoped for: `pow(x, 3)` takes 5.7 ns where it took 6.1 (1.07
  times; 5 passes alternated by `.github/scripts/performance-ab.sh`, an Apple
  M2, AppleClang 21), `gaol::pow(x, 3)` alone 6.0 against 6.4 and
  `gaol_ieee1788::pow(x, [3])` on a base about 0 11.3 against 14.7
  (`tests/tools/pow/bench.cpp`, 6 passes alternated); the other rows move by
  less than 3 %. The two scalar lanes ran side by side already, perhaps, the
  chain of dependent products staying the same (not checked). Making the
  corners from the stored register rather than from two stores and a load did
  not change the ratio.
- **`mid()`** takes 8.5 to 12 ns on the M2 for an addition at 0.9; a two-lane
  kernel would keep the rare branches.
- **The argument reduction of `sin`, `cos` and `tan`** (the quotients by pi and
  the `floor` and `ceil`) is part of the code of GAOL around CORE-MATH, which is
  42 to 66 % of the time of `sin` in the two programs that call it; no profile
  says how much of that is the reduction.
- **`div_rel` and `%=`** are trees of branches; NEON has the cases of
  `divide_by_zero_containing()` as a table for `/`.
- Not for SIMD: `midpoint()` (21 to 28 ns on the M2), `bisect()`, `chi()` and
  `hausdorff()` spend their time in changes of the rounding direction, which is
  the question of computing without changing it; the elementary functions are
  limited by CORE-MATH, and a vector version would be another algorithm.

## What a new path has to show

Before a path is added (the steps the issue lists, with the rules of GAOL):

1. its time against the path it replaces, on the processor and the compiler
   that matter, with the spread, and the operation or the program that needs it;
2. its bounds: the tightest ones, against references computed apart, in the four
   rounding directions of the calling code, with and without
   `GAOL_PRESERVE_ROUNDING`, the subnormals (the flush-to-zero modes cleared),
   the NaN and the infinities, the empty set without an invalid-operation
   exception, and the sign of a zero bound documented where it differs;
3. the three builds alike, a job of the continuous integration that tells which
   path it took (`.github/scripts/check-intervals.sh`), and the processor it
   needs written in [three builds](three-builds.md), with the path it falls back to.

## Things to watch

- **`FPCR.AH`** (FEAT_AFP of Armv8.7, present on the Neoverse V3 of Graviton 5
  and absent from the M2): with it set, `FMAX` and `FMAXP` (the `vmaxq_f64` and
  `vpmaxq_f64` of the NEON kernels) return the second operand for two zeros of
  different signs, and when either is a NaN, in which case they also raise the
  invalid-operation exception. GAOL clears FZ and FIZ and not AH. The kernels
  never give a NaN to a maximum, and the zeros of the maxima of `mul_bounds()`
  differ only in sign (already documented). The square root used to depend on
  the order of the operands of a maximum, of the interval with its left bound
  in its natural sign, `<l, r>`, and of `<+0, +0>`: a compiler that swapped the
  operands would have turned `sqrt([-0, r])` into the empty set under `AH`
  (Apple clang 21 and GCC 14 to its trunk keep the order, but Clang 21 merges
  `vmaxq_f64(x, y)` and `vmaxq_f64(y, x)` into one instruction: it takes the
  intrinsic for commutative). It now clears the lanes whose sign bit is set, a
  negative number or `-0`, with a mask, as the part of the interval in
  `[0, +oo]` (GAOL v5): the same bits as the maximum with `AH` clear, on 20
  million pairs of bounds that are no NaN (the empty set is told before) and
  every pair of the special values (the infinities,
  the zeros, the largest and smallest numbers), and no dependence on `AH` or on
  the order of the operands. The Linux and XNU kernels and the glibc, whose
  sources were read, do not set `AH` by default; a program that does (an
  emulator) is outside what GAOL supports. Modelled in software on 25.8 million
  results: the bounds are those of the kernels, but for the sign of a zero.
  `tests/rounding_direction.cpp` runs the operations with `AH` set where the
  processor retains the bit, and says so where it does not (the M2 and the
  emulators QEMU 7.2 and 8.2 do not; QEMU 10.0 does, with `qemu-aarch64 -cpu
  max`, where the test runs on 441 pairs of operands per operation, and fails
  189 checks with the operands of the former maximum exchanged, the square root
  of `[-0, r]` being then a half-NaN interval).
- **The modes that make a result approximate or flush it** are cleared by GAOL
  where it has a path that reads the control register: FZ and FIZ on 64-bit ARM
  (FZ alone on 32-bit ARM), FTZ and DAZ on x86 (the AVX-512 path does not read
  MXCSR, and clears them where a test shows them set). `NI` of FPSCR on POWER
  (non-IEEE mode, where results "may be approximate") and `FS` of the FCSR on MIPS
  (flush denormalised results to zero) are states of the same kind that GAOL does
  not clear on the FPU intervals of these processors; a path for them would
  have to.
- **The pragma `FENV_ROUND`, the `_mm256_add_round_pd` of Clang 20 and the
  256-bit `{ru-sae}` assembly** compile and are wrong (above).

## What was not measured

- Nothing ran on x86 (the SSE2 and AVX-512 paths), RISC-V, POWER, s390x,
  LoongArch, MIPS or WebAssembly; their timings come from the pages of this
  documentation, uops.info or the references, and every compilation of other
  targets was made on Compiler Explorer, without running the code. The calls
  were counted on the NEON build only.
- GCC never ran: the behaviour of the intrinsics and of the pragma under it is
  read from the assembly.
- The profile has one machine and one compiler, and `sample` is the macOS
  profiler.
- The cost of writing FPSCR (32-bit ARM, POWER), FPC (s390x), FCSR and frm is
  not in the references read, nor is the manual of MIPS.
- Whether `FPCR.AH` changes the kernels was not run: the M2 has no FEAT_AFP.

## Sources

- Intel, Software Developer's Manual, 325462-093US (September 2026), volume 1
  section 15.6.4, volume 2A "Static Rounding Support in EVEX"; Intel AVX10.2
  Architecture Specification 361050, revisions 1.0, 3.0, 4.0 (8 May 2025) and
  7.0 (6 April 2026); the Intel AVX10 Technical Paper 356368-003US; uops.info
  (instructions.xml of 2026-03-29).
- Arm, A64 instruction set XML of 2026-03 and the system register XML (FPCR);
  ARM DDI 0406C.d (ARMv7-A/R); the software optimization guides of the
  Neoverse N1, N2, N3 and the Cortex-A78; the technical reference manual of the
  Cortex-A720 (102530); the A64FX microarchitecture manual; the
  [AWS Graviton getting started](https://github.com/aws/aws-graviton-getting-started)
  guide.
- Power ISA 3.1C; z/Architecture Principles of Operation SA22-7832-14; the
  RISC-V ISA manual of 2026-10-15; LoongArch reference manual volume 1;
  WebAssembly 3.0; `arch/mips/include/asm/mipsregs.h` of Linux. Secondary
  sources, for the RISC-V boards only: the pages of SpacemiT, SiFive and
  T-Head.
- C23 (N3220, 7.6.2); P2746R7, P3864R1 and P4231R0; GCC bugzilla 20785,
  34678, 123482, 118952 and 55752; LLVM pull request 100999; the manuals of
  GCC and of Clang of October 2026.
- Measures of GAOL: [performance](compare/performance.md),
  [three builds](three-builds.md#what-the-options-are-worth), and the
  descriptions of the pull requests
  [#124](https://github.com/Jordan08/GAOL/pull/124) and
  [#146](https://github.com/Jordan08/GAOL/pull/146).
