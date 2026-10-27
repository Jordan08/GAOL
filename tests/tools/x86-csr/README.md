<!-- Copyright (c) 2026 ENSTA, France
     Created 2026-10-27 by Jordan NININ -->
# Tools for the control registers of a 32-bit x86 processor

Tools of GAOL v5 for `gaol_core::x86_*()` of [gaol/gaol_fpu.h](../../../gaol/gaol_fpu.h),
the functions that decide what to write in the control word of the x87 unit
and in MXCSR (point AI.108 of [TODO_DONE.md](../../../TODO_DONE.md)). They
were written for [#156](https://github.com/Jordan08/GAOL/pull/156) on a Mac
and kept out of the repository; their files were lost, and these are written
again.

| File | What it is for |
|---|---|
| [mutants.py](mutants.py) | 46 mutants of `gaol/gaol_fpu.h`, for `../pow/mutate.py`: the test `rounding_direction` has to fail on each. |
| [amalgamate.py](amalgamate.py) | One source file from the sources of GAOL, to give to a compiler that is not here and takes one file (Compiler Explorer). |

## The mutants

`../pow/mutate.py` copies the sources, configures and builds the copy with
CMake, puts one error of the list in the copy of the file, rebuilds and runs
the tests; see its header. For these mutants:

```sh
python3 tests/tools/pow/mutate.py --mutants tests/tools/x86-csr/mutants.py \
        --test rounding_direction --no-diffpow --jobs 4
```

The whole library is built again for each mutant, `gaol_fpu.h` being included
everywhere: the 46 mutants took 713 s with 4 cores. Each mutant is an error in
one of the functions `x86_is_upward()`,
`x86_set_modes()`, `x86_set_upward()`, `x86_enter_registers()`,
`x86_get_state()`, `x86_enter_upward()`, `x86_set_rounding()`,
`x86_set_rounding_and_flush_modes()` and `x86_leave()`: a wrong mask or value,
`&&` for `||`, a write that is always or never made, a register overwritten
whole, the exception flags cleared, the flush modes forgotten, the state read
in the wrong register. These functions are templates over a type for the
registers, and `x86_registers_simulated()` of `tests/rounding_direction.cpp`
runs them on every platform, on two variables, so that the mutants are killed
on the machine of a developer who has no 32-bit x86 processor. The test counts
the reads of the registers as well as their final state: `x86_leave()` that
writes always, with no change of the state, is killed by the count.

One mutant is listed as equivalent: `x87_rc_upward = FE_UPWARD`, which has the
value 0x0800 of the hardware with glibc and is another only where `<fenv.h>`
says otherwise (macOS on arm64, Windows). The constants `x86_flush_mask`,
`sse_rc_mask` and `sse_rc_upward` are not mutated: a `static_assert` compares
them with those of `<xmmintrin.h>`.

What only the processor runs, `x86_registers` and `gaol_read_x86_control()`
of `gaol/gaol_fpu_fenv.h`, is checked by `x86_registers_on_the_processor()`
of the same test, on a 32-bit x86 processor (an i386 image of Docker runs it on
an x86-64 machine, as does the continuous integration with Debian i386), in the
six directions, with each mode that flushes the subnormals and with four sets
of the other bits (the precision and the exception masks of the control word,
the masks and the flags of MXCSR, with one exception unmasked in each, whose
flags the test clears first). Mutations made by hand, and killed by it: a read
of the wrong register, a control word truncated to 8 bits, a write of the
control word with the wrong field RC or without the precision or with the
masks forced, a write of MXCSR that clears the field RC or the flags or forces
the masks, a read that drops the flags or the masks.

## The amalgamation

```sh
python3 tests/tools/x86-csr/amalgamate.py --define GAOL_USING_SSE2_INSTRUCTIONS \
        --define GAOL_USING_SSE3_INSTRUCTIONS --output gaol-all.cpp
```

writes the three units `gaol/core/gaol_interval_core.cpp`,
`gaol/gaol_common.cpp` and `tests/rounding_direction.cpp`, with their
includes, and the `gaol/gaol_configuration.h` that the options say
(`--define GAOL_PRESERVE_ROUNDING`, and so on), in one file (33 000 lines
with the two options above), which compiles with the flags of the builds (`-std=c++17 -O2 -frounding-math -ffp-contract=off -msse2 -mfpmath=sse`
with GCC and Clang for x86, `/std:c++17 /O2 /fp:strict` with Visual C++; C++17
because the test writes hexadecimal floating literals; add `-faligned-new=8`
for GCC and Clang on 32-bit Windows). GCC 15 compiles it in 10 to 15 seconds
with `-Wall -Wextra` and no warning; Clang 21 warns 4 times of an unused
constant (`version_major`... of `gaol_version.h`, which it sees as the main
file). Compiling is all it does for a
target that cannot be run; and a Compiler Explorer stops a compilation after
about 20 seconds, which #156 met with `GAOL_PRESERVE_ROUNDING` and GCC at
`-O2`. Another list of units, the other `--define`s and `--root` are in the
header of the script.

The emulator of i386 (Unicorn) that #156 used to run an extract of the library
on a Mac is not here: an i386 image of Docker, or the continuous integration,
runs the real library on an x86-64 machine.
