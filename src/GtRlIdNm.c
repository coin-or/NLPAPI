/*                                                          */
/* @(#)GtRlIdNm.c	1.2                   */
/* 02/04/19 16:25:57               */
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
char *ECGetRealIdentifierName(int i,struct ECObjectCode *object, int *rc)
 {
  int j,n;
  static char none[1]={0x0};

  if(object==NULL)
   {
    *rc=EC_NULL_OBJECT_CODE;
    return(none);
   }

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetRealIdentifierValue: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return(none);
   }

  if(i<0 || i>ECNumberOfRealIdentifiers(object,rc))
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetRealIdentifierName: RealIdentifier %d does not exist\n",i);
    *rc=EC_IDENTIFIER_NOT_FOUND;
    return(none);
   }

  *rc=EC_NO_ERROR;
  n=0;
  if((*object).nSymbols>0)
   {
    for(j=0;j<(*object).nSymbols;j++)
     {
      if((*object).symbolType[j]==ECReal)
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
