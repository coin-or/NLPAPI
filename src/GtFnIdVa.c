/*                                                          */
/* @(#)GtFnIdVa.c	1.2                   */
/* 02/04/19 16:25:05               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>

#ifdef __cplusplus
extern "C" {
#endif
ECPointerToFunction ECGetFunctionIdentifierValue(int i,struct ECObjectCode *object, int *rc)
 {
  int j,n;
  int N;

  if(object==NULL)
   {
    *rc=EC_NULL_OBJECT_CODE;
    return(NULL);
   }

  if((*object).successful!=EC_NO_ERROR)
   {
    fprintf(stderr,"ECGetFunctionIdentifierValue: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return(NULL);
   }

  if(i<0 || i>ECNumberOfFunctionIdentifiers(object,rc))
   {
    fprintf(stderr,"ECGetFunctionIdentifierValue: FunctionIdentifier %d does not exist\n",i);
    *rc=EC_IDENTIFIER_NOT_FOUND;
    return(NULL);
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
        if(n==i)
         {
          j=(*object).symbolValue[j];
          return((*object).functions[j]);
         }
        n++;
       }
     }
   }

  *rc=EC_IDENTIFIER_NOT_FOUND;
  return(NULL);
 }
#ifdef __cplusplus
 }
#endif
