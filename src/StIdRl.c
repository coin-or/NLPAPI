/*                                                          */
/* @(#)StIdRl.c	1.3                   */
/* 02/04/19 16:35:45               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <string.h>
#define ERROUT stdout
#ifdef __cplusplus
extern "C" {
#endif
int ECSetIdentifierToReal(char *Identifier, double Value, struct ECObjectCode *object)
 {
  int i,j;

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(ERROUT,"ECSetIdentifierToReal: compilation was unsuccessful\n");
    return((*object).successful);
   }

  for(i=0;i<(*object).nSymbols;i++)
   {
    if(!strcmp(Identifier,(*object).symbolList[i]))
     {
      if((*object).symbolType[i]!=ECUnset)
       {
        j=(*object).symbolValue[i];
        (*object).realConstants[j]=Value;
        return(EC_NO_ERROR);
       }
      (*object).symbolType[i]=ECReal;
      (*object).realConstants[(*object).nRealConstants]=Value;
      (*object).symbolValue[i]=(*object).nRealConstants;
      (*object).nRealConstants++;
      return(EC_NO_ERROR);
     }
   }

  if(ECPrintErrorMessages)
    if(ECPrintErrorMessages)fprintf(ERROUT,"ECSetIdentifierToReal: Identifier \"%s\" not found\n",Identifier);
  return(EC_IDENTIFIER_NOT_FOUND);
 }
#ifdef __cplusplus
 }
#endif
