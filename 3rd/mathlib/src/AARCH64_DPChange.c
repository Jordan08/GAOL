/****************************************************************************/
/*                                                                          */
/*MODULE_NAME:DPChange                                                      */
/*                                                                          */
/*FUNCTIONS:Init_Lib                                                        */
/*          Exit_Lib                                                        */
/*                                                                          */
/* Init_lib must be called once prior to any usage of the mathlib routines. */
/* Exit_lib must be called once after the last usage of the mathlib         */
/* routines.                                                                */
/* Init_Lib changes the control word to IEEE double precision so that the   */
/* math routines will work properly. It returns the original status         */
/* as unsigned short. The returned value should be handed to Exit_lib       */
/* in order to restore the original status of the control word after the    */
/* math routines are no longer needed                                       */
/****************************************************************************/
#include "DPChange.h"
#include <stdio.h>
#if HAVE_FENV_H
#  include <fenv.h>
#endif /* HAVE_FENV_H */

#define FESETENV fesetenv
#define FEGETENV fegetenv

unsigned  short OrgDPStatus=0;
unsigned  short NewDPStatus=0;

#if MATHLIB_MINGW
#  define ORG_DP_STATUS "_OrgDPStatus"
#  define NEW_DP_STATUS "_NewDPStatus"
#else
#  define ORG_DP_STATUS "OrgDPStatus"
#  define NEW_DP_STATUS "NewDPStatus"
#endif


/* Function to change precision control to double and round mode to nearest */
/* or even. Function returns unsigned short between 0 and 15 that indicates */
/* the original round control and precision mode before the change.         */
/* The two LSB bits of the returned value are the precision mode, and the   */
/* next two bits are the round control.                                     */

unsigned short Init_Lib()
{
#if HAVE_FENV_H
  /* The rounding direction found, which Exit_Lib() restores: as the comment
     above says, in the two bits above the two of the precision mode, which is
     not touched here (the doubles of 32-bit x86 are computed in SSE2). */
  unsigned short round_control;
  switch (fegetround()) {
    case FE_DOWNWARD:   round_control = 0x01; break;
    case FE_UPWARD:     round_control = 0x02; break;
    case FE_TOWARDZERO: round_control = 0x03; break;
    default:            round_control = 0x00; break; /* FE_TONEAREST */
  }
  fesetround(FE_TONEAREST); /* what mathlib's algorithms need */
  return (unsigned short)(round_control << 2);
#else 
#   error "fenv.h not found and no replacement available to initialize the library"
#endif /* HAVE_FENV_H */
}


/* Function that receives an unsigned short argument in the range 0 - 15    */
/* and changes the precision control and round mode according to the        */
/* explanation above. If the value is more than 15 it prints to stdout an   */
/* error message and changes nothing.                                       */

void Exit_Lib(unsigned short status)
{
#if HAVE_FENV_H
  /* The rounding direction Init_Lib() found, set again */
  int round_mode;
  switch ((status >> 2) & 0x03) {
    case 0x01: round_mode = FE_DOWNWARD; break;
    case 0x02: round_mode = FE_UPWARD; break;
    case 0x03: round_mode = FE_TOWARDZERO; break;
    default:   round_mode = FE_TONEAREST; break;
  }
  (void)fesetround(round_mode);
  	return;
#else 
#   error "fenv.h not found and no replacement available to deinitialize the library"
#endif /* HAVE_FENV_H */
}













































