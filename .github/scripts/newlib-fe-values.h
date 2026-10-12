/* The FE_* rounding values of newlib, the C library of Cygwin (0 to nearest,
   1 downward, 2 upward, 3 toward zero, newlib/libc/machine/shared_x86/sys/
   fenv.h), put over those of glibc for the sources of CORE-MATH, which the job
   cygwin-fe-values of linux.yml compiles with -include and this header
   (GAOL v5). GCC and Clang for Cygwin define __x86_64__ but not __WIN32__,
   which Linux mimics: upstream's get_rounding_mode() of cbrt.c, rsqrt.c and
   asinpi.c then took the downward and upward roundings for toward zero there,
   and tests/core_math.cpp failed 26,558 of its checks; GAOL's copies read the
   rounding field of MXCSR whatever the values of FE_* (3rd/README.md, item 6).
   Only the C sources take this header: GAOL's own, C++, keep glibc's. */
#include <fenv.h>
#undef FE_TONEAREST
#undef FE_DOWNWARD
#undef FE_UPWARD
#undef FE_TOWARDZERO
#define FE_TONEAREST 0
#define FE_DOWNWARD 1
#define FE_UPWARD 2
#define FE_TOWARDZERO 3
