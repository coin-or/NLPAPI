/*                                                          */
/* @(#)StId.c	1.2                   */
/* 02/04/19 16:35:19               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmpI.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
int ECSetIdentifier(char *Assignment,struct ECObjectCode *object)
 {
  static size_t i;
  static int I,j;
  static size_t n;
  static char *Identifier;
  static double Value;
  static int   iValue;
  static struct ECObjectCode *valueObject;
  static int rc;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);

/* find the equals sign */
  n=strlen(Assignment)+10;
  for(i=0;i<strlen(Assignment);i++)
   {
    if(Assignment[i]=='=')n=i;
   }

#ifdef VERBOSESET
  printf("ECSetIdentifier: position of \"=\" is %d\n",n);
#endif

  if(n==strlen(Assignment)+10)
   {
    if(ECPrintErrorMessages)fprintf(stderr, "ECSetIdentifier: \"=\" sign not found in assignment\n");
    return(EC_INVALID_ASSIGNMENT);
   }

/* Get the identifier name */

  Identifier=(char *)malloc((n+1)*sizeof(char));
  strncpy(Identifier,Assignment,n);
  Identifier[n]=0x0;

/* Get the identifier value */

  rc=ECCompileExpression(Assignment+n+1,&valueObject);
  if(rc!=EC_NO_ERROR)return(rc);

  rc=ECSetStandardMathConstants(valueObject);
  if(rc!=EC_NO_ERROR)return(rc);

  rc=ECSetStandardMathFunctions(valueObject);
  if(rc!=EC_NO_ERROR)return(rc);

  Value=ECEvaluateExpression(valueObject,&rc);
  if(rc!=EC_NO_ERROR)return(rc);

  ECFreeObjectCode(&valueObject);

#ifdef VERBOSESET
  printf("ECSetIdentifier: identifier \"%s\" value %f\n",Identifier,Value);
#endif

  if((*object).successful)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifier: compilation was unsuccessful\n");
    free(Identifier);
    return((*object).successful);
   }

  for(I=0;I<(*object).nSymbols;I++)
   {
    if(!strcmp(Identifier,(*object).symbolList[I]))
     {
      if((*object).symbolType[I]!=ECUnset)
       {
        j=(*object).symbolValue[I];
        if((*object).symbolType[I]==ECReal)(*object).realConstants[j]=Value;
        sscanf(Assignment+n+1,"%d",&iValue);
        if((*object).symbolType[I]==ECInteger)(*object).integerConstants[j]=iValue;
        if((*object).symbolType[I]==ECFunction)
         {
          if(ECPrintErrorMessages)fprintf(stderr,"ECSetIdentifier: Attempt to assign a function\n");
          free(Identifier);
          return(EC_INVALID_IDENTIFIER_TYPE);
         }
        free(Identifier);
        return(EC_NO_ERROR);
       }
      (*object).symbolType[I]=ECReal;
      (*object).realConstants[(*object).nRealConstants]=Value;
      (*object).symbolValue[I]=(*object).nRealConstants;
      (*object).nRealConstants++;
      free(Identifier);
      return(EC_NO_ERROR);
     }
   }

  free(Identifier);
  return(EC_NO_ERROR);
 }
#ifdef __cplusplus
 }
#endif
