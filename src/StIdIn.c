/*                                                          */
/* @(#)StIdIn.c	1.2                   */
/* 02/04/19 16:35:36               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
int ECSetIdentifierToInteger(char *Identifier, int Value, struct ECObjectCode *object)
 {
  int i,j;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);

  if((*object).successful)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifierToInteger: compilation was unsuccessful\n");
    return((*object).successful);
   }

  for(i=0;i<(*object).nSymbols;i++)
   {
    if(!strcmp(Identifier,(*object).symbolList[i]))
     {
      if((*object).symbolType[i]!=ECUnset)
       {
        j=(*object).symbolValue[i];
        (*object).integerConstants[j]=Value;
        return(EC_NO_ERROR);
       }
      (*object).symbolType[i]=ECInteger;
      (*object).integerConstants[(*object).nIntegerConstants]=Value;
      (*object).symbolValue[i]=(*object).nIntegerConstants;
      (*object).nIntegerConstants++;
      return(EC_NO_ERROR);
     }
   }

/*printf("ECSetIdentifierToInteger: Identifier \"%s\" not found\n",Identifier);*/
  return(EC_NO_ERROR);
 }
#ifdef __cplusplus
 }
#endif
