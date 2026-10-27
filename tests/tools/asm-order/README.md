<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-27 by Jordan NININ -->
# The order of the computations and of the writes of the control register

Tool of GAOL v5 for the assembly of the library (task PF.5 of
[TODO.md](../../../TODO.md): the check of #150, made for AArch64 and 32-bit
ARM, made for x86-64).

GCC does not model the rounding direction
([bug 34678](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=34678)): it may move
a floating-point operation on values in registers across the instruction that
writes the control register, and compute a bound in the direction of the
program rather than upward, or a result after the direction has been set back.
[check_order.py](check_order.py) reads the assembly for that. #150 found it
with `width()` on 64-bit and 32-bit ARM (a difference computed before the write
of FPCR), and ran such a check on the assembly of the whole library for
AArch64 and ARM with GCC 14.3: no floating-point instruction that depends on
the direction between a `mrs` or `vmrs` and the `msr` or `vmsr`. That script is
not in the repository; this one is for x86-64, GCC and Clang.

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DGAOL_PRESERVE_ROUNDING=ON
python3 tests/tools/asm-order/check_order.py --build build --preserve
python3 tests/tools/asm-order/check_order.py --selftest
```

`--build` compiles the sources of `gaol/` again with `-S` and the flags of the
build (into `build/asm-order`), and reads the assembly (not the C of CORE-MATH,
whose only writes of MXCSR give back the exception flags that `pow`, `cbrt` and
`hypot` kept, and which sets no direction); `FILE.s` arguments
read the assembly of GCC or Clang that is given. `--preserve` is for the builds
with `GAOL_PRESERVE_ROUNDING`, whose operations write the register back.

## What it looks for

For each function that holds a write of the control register (`ldmxcsr`,
`fldcw`, and a call of `round_upward_if_needed()` or `fesetround()`, which
write it), on the paths of its control flow graph:

- **BEFORE**: an instruction that rounds (add, sub, mul, div, sqrt, fused
  multiply-add, a conversion that rounds, `roundsd` with the direction of the
  register) that the function runs before the write, on a path from its entry
  to a write, and before the probe of the direction. The probe is
  `1 + (2^-1060 + 0)`, whose sums are meant to be computed in the direction
  found: the sums that follow the use of the static `subnormal` or `tiny` up to
  the jump that tells the direction, and that jump, which the instructions after
  it are past. A conversion of a 32-bit integer to a double is exact. An
  instruction with an embedded rounding (`{ru-sae}`) carries its direction.
- **AFTER** (with `--preserve`): an instruction that rounds after the write that
  sets the direction back, that no other write follows and that the entry
  cannot reach without a write. A function whose writes are conditional (the
  registers of ARM and of 32-bit x86 are written only when they differ) reaches
  all of its code without one: the check cannot tell a result computed after
  the write from one computed before it there, and the tests, which compare the
  results in every direction, are all that sees it. The x86-64 builds write
  MXCSR without a condition today.

It does not see what a callee does (the callee is read as well), the
instructions inside an asm statement, nor a computation and a write made in two
functions with the call between them. The loops are cut at their back edge, so
that the instructions of an iteration are not taken for the ones before the
write of the next.

Some findings are right and harmless; `KNOWN` in the script lists them with
their reason, and the check prints them as known findings. An entry that
matches nothing in a run over the whole library (`--build`) is reported, so
that the list does not go out of date unseen.

## What it found

On the code of 2026-10-27, over 40 builds of the library (GCC 15.2 at `-O2` and
`-O3`, Clang 18 and 21 at `-O3` and Clang 21 at `-O2`; the default, the
direction preserved with and without the AVX-512 path, the AVX-512 path alone,
the FPU intervals, without FMA, and the last two preserved): no instruction
that depends on the direction on the wrong side of a write that changes it. The
first run found, as findings to read: the computation of the bound K in the
path of the embedded rounding of the integer powers (`pown_checked()`), before
the write that clears the modes that flush the subnormals, in the direction of
the program (K is exact, or has a factor of 16 of margin), the estimate of
`pow_up_without_overflow_exception()` before the write that masks the
exceptions, and the halving of the bounds of `difference_at_least()` after the
last write (exact). With `GAOL_RND_KEEP()` removed from
`width()`, GCC 15 still kept the subtraction before the write that sets the
direction back, at `-O1` to `-O3`.

The check has teeth on assembly written for it (`--selftest`: a sum before a
write, after the write that sets back, hoisted above the probe, in a loop...).
