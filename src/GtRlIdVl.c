/*                                                          */
/* @(#)GtRlIdVl.c	1.2                   */
/* 02/04/19 16:26:05               */
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
double ECGetRealIdentifierValue(int i,struct ECObjectCode *object, int *rc)
 {
  int j,k;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetRealIdentifierValue: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return(0.);
   }

  j=0;
  for(k=0;k<(*object).nSymbols;k++)
   {
    if((*object).symbolType[k]==ECReal)
     {
      if(j==i)
       {
        *rc=EC_NO_ERROR;
        j=(*object).symbolValue[k];
        return((*object).realConstants[j]);
/*      return((*object).realConstants[k]);   MEH 3/20/96 */
       }
      j++;
     }
   }

  *rc=EC_IDENTIFIER_NOT_FOUND;
  return(0.);
 }
#ifdef __cplusplus
 }
#endif
