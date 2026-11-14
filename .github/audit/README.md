<!-- Copyright (c) 2026 ENSTA, France

     By: Jordan NININ (created 2026-09-20)

     GAOL is a software distributed WITHOUT ANY WARRANTY. Read the associated
     COPYING.LIB file for information. -->
# The three builds have to agree

GAOL can be configured with CMake, autotools (`configure`) and meson. On a
given machine with a given compiler, the three have to give GAOL the same
configuration: the same macros in `gaol/gaol_configuration.h`, and the same
compilation flags that bear on the results or on the speed (rounding, contraction,
fast-math, x87 or SSE2 doubles, SSE2 or NEON intervals, optimization, C++ standard).

`audit.sh` configures the three builds, with the default options and then with
each of five sets of options that change the configuration (the rounding
direction preserved, with and without the AVX-512 path, the FPU intervals, no
fused multiply-add instruction, no exceptions and the verbose mode; GAOL v5: the
default options alone), and `probe_build.py` preprocesses
`make_probe.py`'s program with the flags each build gives
`gaol_interval.cpp`, which shows the macros GAOL sees, and a C program with the
flags each gives a source of CORE-MATH (`sin.c`). `compare.py`
prints what differs, and with `--check` fails when a build did not configure
or when the configurations differ. The continuous integration runs it on each
kind of machine (`.github/workflows/build-systems.yml`).

    sh .github/audit/audit.sh <label> <cc> <cxx> <sources> <output directory> [meson]
    python3 .github/audit/compare.py --check <output directory>/<label>*.json

`<label>.json` is the default options and `<label>-<variant>.json` each set of
options (`GAOL_AUDIT_VARIANTS` lists the sets to audit, none for the default
options alone). A build that did not configure, a probe of the C or C++ sources
that failed, or builds that differ, fail `compare.py --check`;
`python3 .github/audit/test_compare.py` checks that on made-up results.
