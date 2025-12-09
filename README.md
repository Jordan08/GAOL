
# GAOL
<em>Not Just Another Interval Library</em>

GAOL is a C++ [Interval Arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic) library that strives to offer fast and reliable operators for constraint solvers. 

## Building GAOL

### Pre-requisites

A supported math library: [apmathlib](https://frederic.goualard.net/software/mathlib-2.1.1.tar.gz) or [crlibm](https://github.com/taschini/crlibm) (since crlibm github repo seems to have missing files, you can take get it here: [pycrlibm](https://github.com/taschini/pycrlibm))

### Linux users

Look at INSTALL file to use autotools

### MacOS ARM users

[Meson build system](https://mesonbuild.com/index.html) can be used:

```bash
meson setup build -Dwith-mathlib=crlibm
cd build
meson compile
```

If you want to run tests setup the build folder with option `with-test` to `true`

If you want to install gaol to a specify folder use the meson argument `--prefix`

For instance you can run:

```bash
meson setup build --prefix=/opt/homebrew/Cellar/gaol/4.2.2 -Dwith-mathlib=apmathlib -Dwith-test=true
```
