/*                                                          */
/* @(#)PrtSt.c	1.2                   */
/* 02/04/19 16:34:44               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>

#ifdef __cplusplus
extern "C" {
#endif
void ECPrintState(ECState S)
{
 switch(S)
  {
   case ECLookingForToken:
    printf(" State is LookingForToken\n");
    break;
   case ECLookingForMultiplyExponentiate:
    printf(" State is LookingForMultiplyExponentiate\n");
    break;
   case ECLookingForIdentifier:
    printf(" State is LookingForIdentifier\n");
    break;
   case ECLookingForConstant:
    printf(" State is LookingForConstant\n");
    break;
   default:
    printf(" State is invalid\n");
  }
 return;
}
#ifdef __cplusplus
 }
#endif
