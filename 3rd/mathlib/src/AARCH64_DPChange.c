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
  FESETENV(FE_DFL_ENV); // round to nearest, all except. cleared, nonstop
  return 0; // We do not try to save the fpu reg. before the call to Init_Lib()
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
  	return;
#else 
#   error "fenv.h not found and no replacement available to deinitialize the library"
#endif /* HAVE_FENV_H */
}













































