/*                                                          */
/* @(#)StIdFn.c	1.2                   */
/* 02/04/19 16:35:25               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <ECMsg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
int ECSetIdentifierToFunction(char *Identifier, ECPointerToFunction Value, struct ECObjectCode *object)
 {
  int i,j;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifierToFunction: compilation was unsuccessful\n");
    return((*object).successful);
   }

  for(i=0;i<(*object).nSymbols;i++)
   {
    if(!strcmp(Identifier,(*object).symbolList[i]))
     {
      if((*object).symbolType[i]!=ECUnset&&(*object).symbolType[i]!=ECFunction)
       {
        j=(*object).symbolValue[i];
        (*object).functions[j]=Value;
        if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifierToFunction: Identifier \"%s\" not a Function\n",Identifier);
        return(EC_IDENTIFIER_NOT_FUNCTION);
       }
      (*object).symbolType[i]=ECFunction;
      (*object).functions[(*object).nFunctions]=Value;
      (*object).symbolValue[i]=(*object).nFunctions;
      (*object).nFunctions++;
      return(EC_NO_ERROR);
     }
   }

  if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifierToFunction: Identifier \"%s\" not found\n",Identifier);
  return(EC_IDENTIFIER_NOT_FOUND);
 }
#ifdef __cplusplus
 }
#endif
