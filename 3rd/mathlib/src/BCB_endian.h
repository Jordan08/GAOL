#undef  BIG_ENDI
#define   LITTLE_ENDI

#ifdef BIG_ENDI
#define HIGH_HALF 0
#define  LOW_HALF 1
#else 
#ifdef LITTLE_ENDI
#define HIGH_HALF 1
#define  LOW_HALF 0
#endif
#endif

/* fabs() rather than ((x) <  0  ? -(x) : (x)) (fork of GAOL): the same value
   but for the sign of -0 and of NaN, which mathlib's results do not depend on,
   and no branch, where compilers made one of the conditional expression. Its
   direction depends on the bits of the argument (the sign of log(ui) and
   log(vj) in the exact additions of ulog), and was mispredicted. fabs() is
   declared rather than taken from <math.h>, whose NAN urem.h names a constant. */
double fabs(double);
#define ABS(x)   fabs(x)
