/*                                                          */
/* @(#)GtUnIdNm.c	1.2                   */
/* 02/04/19 16:26:13               */
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
char *ECGetUnsetIdentifierName(int i,struct ECObjectCode *object, int *rc)
 {
  static char Invalid[1]={0x0};
  int j;
  int n;

  if(object==NULL)
   {
    *rc=EC_NULL_OBJECT_CODE;
    return((char *)NULL);
   }

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetUnsetIdentifierName: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return((char *)NULL);
   }

  if(i<0 || i>ECNumberOfUnsetIdentifiers(object,rc))
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetUnsetIdentifierName: UnsetIdentifier %d does not exist\n",i);
    *rc=EC_IDENTIFIER_NOT_FOUND;
    return(Invalid);
   }

  *rc=EC_NO_ERROR;
  n=0;
  if((*object).nSymbols>0)
   {
    for(j=0;j<(*object).nSymbols;j++)
     {
      if((*object).symbolType[j]==ECUnset)
       {
        if(n==i)return((*object).symbolList[j]);
        n++;
       }
     }
   }

  if(ECPrintErrorMessages)fprintf(stderr,"ECGetUnsetIdentifierName: UnsetIdentifier %d does not exist\n",i);
  *rc=EC_IDENTIFIER_NOT_FOUND;
  return(Invalid);
 }
#ifdef __cplusplus
 }
#endif
