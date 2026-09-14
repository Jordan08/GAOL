/* @(#)s_nextafter.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* IEEE functions
 *	nextafter(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

/*
 * Fork of GAOL: the words of the doubles are read and written through
 * memcpy() and a 64-bit integer, rather than through int pointers, which broke
 * the strict aliasing rules and took the low word first whatever the byte
 * order. The cases, the results and the exceptions raised are those of fdlibm.
 * GAOL only uses s_nextafter() with a Visual C++ lacking nextafter().
 */

#include <string.h>

#define SIGN_BIT      0x8000000000000000ULL
#define EXPONENT_BITS 0x7ff0000000000000ULL

#if defined (_MSC_VER)
__declspec(dllexport)
#elif defined (__GNUC__)
__attribute__ ((visibility("default")))
#endif
double s_nextafter(double x, double y)

{
	unsigned long long ux,uy,ax,ay;
	/* volatile: the product x*x stored in it only raises the underflow flag,
	   and GCC would drop it, both branches returning the same double */
	volatile double t;

	memcpy(&ux,&x,sizeof ux);
	memcpy(&uy,&y,sizeof uy);
	ax = ux&~SIGN_BIT;		/* |x| */
	ay = uy&~SIGN_BIT;		/* |y| */

	if(ax>EXPONENT_BITS ||		/* x is nan */
	   ay>EXPONENT_BITS)		/* y is nan */
	   return x+y;
	if(x==y) return x;		/* x=y, return x */
	if(ax==0) {			/* x == 0 */
	    ux = (uy&SIGN_BIT)|1;	/* return +-minsubnormal */
	    memcpy(&x,&ux,sizeof x);
	    t = x*x;
	    if(t==x) return t; else return x;	/* raise underflow flag */
	}
	if((x>y)==((ux&SIGN_BIT)==0))	/* |x| decreases toward y */
	    ux -= 1;
	else				/* |x| increases toward y */
	    ux += 1;
	if((ux&EXPONENT_BITS)==EXPONENT_BITS) return x+x;	/* overflow  */
	if((ux&EXPONENT_BITS)==0) {	/* underflow */
	    t = x*x;
	    if(t!=x) {		/* raise underflow flag */
		memcpy(&y,&ux,sizeof y);
		return y;
	    }
	}
	memcpy(&x,&ux,sizeof x);
	return x;
}
