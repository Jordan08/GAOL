<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-27 by Jordan NININ -->
# The order of the computations and of the writes of the control register

Tool of GAOL v5 for the assembly of the library: the check of #150 (task PF,
point 5, of `TODO.md`), made for AArch64 and 32-bit ARM, made for x86-64.

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

The self-test runs in the continuous integration (`.github/workflows/linux.yml`,
the job `asm-order-selftest`, Python alone); the check of the builds is not
done there.

`--build` compiles the sources of `gaol/` again with `-S` and the flags of the
build (into `build/asm-order`), and [inline.cpp](inline.cpp) with the flags of
`gaol_interval.cpp`: `width()`, `rad()` and `mid_rad()` are inline in a header,
so that no source of the library holds them. It reads the assembly (not the C
of CORE-MATH, whose writes of MXCSR give back the exception flags that `pow`,
`cbrt`, `hypot`, `exp`, `exp2` and `exp10` kept and which sets no direction);
`FILE.s` arguments read the assembly of GCC or Clang that is given.
`--preserve` is for the builds with `GAOL_PRESERVE_ROUNDING`, whose operations
write the register back. The exit status is 1 for a finding.

## What it looks for

For each function that holds a write of the control register (`ldmxcsr`,
`fldcw`, and a call of `round_upward_if_needed()` or `fesetround()`, which
write it), on the paths of its control flow graph, the jump tables of the
`switch` statements being followed:

- **BEFORE**: an instruction that rounds (add, sub, mul, div, sqrt, fused
  multiply-add, a conversion that rounds, `roundsd` with the direction of the
  register) that the function runs before the write, on a path from its entry
  to a write, and before the probe of the direction. The probe is
  `1 + (2^-1060 + 0)`, whose sums are meant to be computed in the direction
  found: the instructions that round with a register that holds the static
  `subnormal` or `tiny`, or a sum of it, up to the jump that tells the
  direction; a sum of the program placed among them is not one. Past that jump
  the direction is known. The same goes for any conditional write (the code
  after it is reached without it: a write made only if the register has to
  change): the jump that goes around it, after a read of the register
  (`stmxcsr`, `fnstcw`, a call of `fegetround()`), is where the check is made,
  the instructions before that jump are checked, and past it the register
  holds what the write would have put there; a jump that no read precedes
  checks nothing. A conversion of a 32-bit integer to a
  double is exact.
- **AFTER** (with `--preserve`): an instruction that rounds after the write
  that sets the direction back, that no other write follows and that the entry
  cannot reach without a write. A function whose writes are conditional
  (`round_upward_if_needed()`, `flush_guard`, the registers of ARM and of
  32-bit x86) reaches all of its code without one: the check cannot tell a
  result computed after the write from one computed before it there, and the
  tests, which compare the results in every direction, are what sees it.
- **BLIND**: an instruction that rounds or writes the register that no function
  of the file reaches, which a jump table the check could not follow would hide;
  the indirect jumps without a table are counted (most are tail calls through a
  pointer). The landing pads of the exceptions, named by the tables of
  exceptions of the file, are part of the function that holds them, and are not
  reached from its entry: what they write is seen, not analysed.

A store of 1 or of 0 through the x87 unit (`fld1`, `fstpl`: GCC writes a
constant so) is exact and is not one that rounds.

## What it does not see

- What a callee does (the callee is read as well), and an operation and a write
  in two functions, the first calling the second between the two.
- The sections between the first write and the last: the evaluations in nearest
  of `GAOL_RND_NEAREST_ENTER()` and `LEAVE()`, and the writes of
  `round_nearest()` and the others. The tests that call each operation in every
  direction (`tests/rounding_direction.cpp`) are what sees them.
- The AVX-512 path. Its instructions carry their direction (`{ru-sae}`) and read
  no register, but they read the flush-to-zero modes that `flush_guard` clears
  and gives back; the comparisons read denormals-are-zero too (GCC 13 computed
  those of `asinpi()` before the check, `gaol/gaol_fpu.h`). Neither is looked at.
- A value computed in the direction of the program and used where upward is
  meant, which takes a data flow analysis. `difference_at_least()` computes the
  halves of its bounds after its last write (exact: see `KNOWN`), and Clang's
  assembly shows it where GCC's does not, because GCC turns the recursion into
  a loop that leads back to a write.
- The loops are cut at their back edge, so that the instructions of an
  iteration are not taken for the ones before the write of the next.
- The text of an `asm` statement is read as code: the `fldcw` of the library all
  come from one.

Two findings are right and harmless, and `KNOWN` in the script lists them with
their reason; they are printed with `--verbose` and do not make the check fail.
`KNOWN` is for a function and a kind, not for an instruction: another finding
in these functions would be hidden too. With `--build --verbose`, an entry that
matches nothing is reported.

## What it found

On the code of 2026-10-27, in 40 builds of the library: GCC 15.2 at `-O2` and
`-O3`, Clang 18 and 21 at `-O3` and Clang 21 at `-O2`, each with the default,
the direction preserved (with the AVX-512 path, and without), the AVX-512 path
alone, the FPU intervals, no FMA, and the last two preserved: no instruction
that depends on the direction before the first write or after the last one,
but the two harmless findings of `KNOWN`:

- `pow_up_without_overflow_exception()` (and `pow_rounded_up<true>`, which
  inlines it): the write masks the exceptions of MXCSR and leaves the direction;
  what comes before decides whether to mask, with margins far larger than the
  rounding of the estimate;
- `difference_at_least()`, with Clang: the halving of the bounds after the last
  write is exact (their difference overflows, so they are 2^970 or more in
  magnitude).

The bound K of the error of the integer powers in the path of the embedded
rounding (`pown_checked()`, `pown_bound_inverse()`), which the first version of
the tool showed before a write of `flush_guard`, is computed after it in the
builds of this run.

With `GAOL_RND_KEEP()` removed from `width()`, GCC 15 still kept the subtraction
before the write that sets the direction back, at `-O1` to `-O3`, and Clang 21
at `-O1` and `-O3`: `inline.cpp` is what reads `width()` in a build. The check
has teeth on assembly written for it (`--selftest`, 54 cases: a sum before a
write, after the write that sets back, hoisted above the probe, among the
instructions of the probe, in a loop, behind a jump table of GCC and of Clang,
in a cold section, after a tail call...), and 57 of the 63 changes made to the
script, one at a time, make it fail; the six that do not are equivalent (the
`.cfi_endproc` that `.type` follows, the comments after an instruction, the
write that a closure need not stop at...).
