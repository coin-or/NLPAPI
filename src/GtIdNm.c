/*                                                          */
/* @(#)GtIdNm.c	1.2                   */
/* 02/04/19 16:25:12               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <ECMsg.h>

#ifdef __cplusplus
extern "C" {
#endif
char *ECGetIdentifierName(int i,struct ECObjectCode *object, int *rc)
 {
  static char none[1];

  if(object==NULL)
   {
    *rc=EC_NULL_OBJECT_CODE;
    return((char *)NULL);
   }

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetIdentifierName: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return((char *)NULL);
   }

  *rc=EC_NO_ERROR;
  if(i>=0 && i<(*object).nSymbols)return((*object).symbolList[i]);

  *rc=EC_IDENTIFIER_NOT_FOUND;
  none[0]=0x0;
  return(none);
 }
#ifdef __cplusplus
 }
#endif
