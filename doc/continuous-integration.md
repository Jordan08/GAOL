# Continuous integration

Part of the documentation of [this fork of GAOL](../README.md#documentation).

The workflows of `.github/workflows/` build GAOL with CMake and run the tests
on:

- **Linux:** Ubuntu 22.04, 24.04 and 26.04 on x86_64 and arm64, with GCC and
  Clang, also with the address and undefined behaviour sanitizers, and with
  CMake 3.14.
- **Linux containers:**
  - Debian 12 and 13 on amd64, arm64 and armhf, and Debian 12 on i386;
  - manylinux_2_28 on x86_64 and aarch64;
  - Alpine (musl) on x86_64 and aarch64;
  - Debian 13 under qemu on s390x, ppc64le and riscv64.
- **macOS:** 14, 15 and 26, on arm64 and x86_64 (natively or under Rosetta), and
  with the sanitizers with AppleClang, LLVM's Clang and GCC.
- **Windows:**
  - Visual Studio 2022 and 2026, on x86, x64 and arm64, Release and Debug;
  - MinGW-w64 15, on x86 and x64;
  - MSYS2 UCRT64 (GCC) and CLANG64 (Clang).

They also build GAOL with autotools and meson, against an installed mathlib,
on Ubuntu (x86_64, arm64), Debian (i386,
armhf), macOS (arm64, x86_64) and MSYS2, and the tests with the GAOL they
install; check that the three builds agree on each of these machines; build
GAOL with CMake against an installed mathlib (`MATHLIB_DIR`); check
that Clang is refused on 32-bit ARM, Clang 14 on 64-bit ARM, and MinGW-w64 11
to 14 (13 and 14 with autotools and meson too), and Visual C++ without
`/fp:strict`. Jobs of each build restore the
rounding direction (`GAOL_PRESERVE_ROUNDING`): Ubuntu x86_64 GCC and arm64
Clang, Debian i386 and armhf, macOS arm64, Visual Studio x64, autotools and
meson. The jobs built in Release print the time per operation in their summary.
