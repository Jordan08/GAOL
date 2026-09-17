/**************************************************************************/
/*                                                                        */ 
/*MODULE_NAME:DPChange                                                    */
/*                                                                        */
/*FUNCTIONS:Init_Lib                                                      */
/*          Exit_Lib                                                      */
/*                                                                        */
/*  common data & function prototypes                                     */ 
/**************************************************************************/
#ifndef DP
#define DP
/* Round Control    : bits 10 and 11 of the control word  */                  
/* Precision Control: bits  8 and  9 of the control word  */



extern unsigned short Init_Lib();


extern void Exit_Lib(unsigned short);

#endif /*DP*/
