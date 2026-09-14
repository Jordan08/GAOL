# The three builds have to agree

GAOL can be configured with CMake, autotools (`configure`) and meson. On a
given machine with a given compiler, the three have to give GAOL the same
configuration: the same macros in `gaol/gaol_configuration.h`, and the same
compilation flags that bear on the results or on the speed (rounding, contraction,
fast-math, x87 or SSE2 doubles, SSE2 intervals, optimization, C++ standard).

`audit.sh` configures the three builds against an installed mathlib, and
`probe_build.py` preprocesses `make_probe.py`'s program with the flags each build
gives `gaol_interval.cpp`, which shows the macros GAOL sees. `compare.py`
prints what differs, and with `--check` fails when a build did not configure
or when the configurations differ. The continuous integration runs it on each
kind of machine (`.github/workflows/build-systems.yml`).

    sh .github/audit/audit.sh <label> <cc> <cxx> <sources> <mathlib prefix> <output directory> [meson]
    python3 .github/audit/compare.py --check <output directory>/<label>.json
