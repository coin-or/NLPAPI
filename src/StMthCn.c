/*                                                          */
/* @(#)StMthCn.c	1.2                   */
/* 02/04/19 16:36:04               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

/* April 23, 1996 Turned off error messages when constants not present */

#include <ExpCmp.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
int ECSetStandardMathConstants(struct ECObjectCode *object)
 {
  int result;
  int flag;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);
  flag=ECGetMessagePrint();

  ECSetMessagePrint(0);
  result=ECSetIdentifierToReal("%pi",4.*atan2(1.,1.),object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToReal("%e",exp(1.),object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  return(EC_NO_ERROR);
 }
#ifdef __cplusplus
 }
#endif
