/*                                                          */
/* @(#)GtFnIdNm.c	1.2                   */
/* 02/04/19 16:24:56               */
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
char *ECGetFunctionIdentifierName(int i,struct ECObjectCode *object, int *rc)
 {
  int j,n;
  int N;
  static char none[1]={0x0};

  if(object==NULL)
   {
    *rc=EC_NULL_OBJECT_CODE;
    return(none);
   }

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetFunctionIdentifierName: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return(none);
   }

  if(i<0 || i>ECNumberOfFunctionIdentifiers(object,rc))
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetFunctionIdentifierName: FunctionIdentifier %d does not exist\n",i);
    *rc=EC_IDENTIFIER_NOT_FOUND;
    return(none);
   }

  N=object->nSymbols;

  *rc=EC_NO_ERROR;
  n=0;
  if(N>0)
   {
    for(j=0;j<N;j++)
     {
      if((*object).symbolType[j]==ECFunction)
       {
        if(n==i)return((*object).symbolList[j]);
        n++;
       }
     }
   }

  *rc=EC_IDENTIFIER_NOT_FOUND;
  return(none);
 }
#ifdef __cplusplus
 }
#endif
