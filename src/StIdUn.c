/*                                                          */
/* @(#)StIdUn.c	1.2                   */
/* 02/04/19 16:35:52               */
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
int ECSetIdentifierToUndefined(char *Identifier, struct ECObjectCode *object)
 {
  int i;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);

  if((*object).successful!=0)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifierToUndefined: compilation was unsuccessful\n");
    return((*object).successful);
   }

  for(i=0;i<(*object).nSymbols;i++)
   {
    if(!strcmp(Identifier,(*object).symbolList[i]))
     {
      (*object).symbolType[i]=ECUnset;
      return(EC_NO_ERROR);
     }
   }

  if(ECPrintErrorMessages)
     if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifierToUndefined: Identifier \"%s\" not found\n",Identifier);
  return(EC_IDENTIFIER_NOT_FOUND);
 }
#ifdef __cplusplus
 }
#endif
