/*                                                          */
/* @(#)StMsgPrt.c	1.2                   */
/* 02/04/19 16:35:59               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>

int ECPrintErrorMessages=0;

#ifdef __cplusplus
extern "C" {
#endif
void ECSetMessagePrint(int value)
 {
  ECPrintErrorMessages=value;
  return;
 }
#ifdef __cplusplus
 }
#endif
