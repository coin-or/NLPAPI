/*                                                          */
/* @(#)GtIdTy.c	1.2                   */
/* 02/04/19 16:25:20               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <ECMsg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
char *ECGetIdentifierType(int i,struct ECObjectCode *object, int *rc)
 {
  static char none[1];
  static char real[5];
  static char integer[8];
  static char function[9];
  static char naobject[9];
  static char undefined[10];

  if(object==NULL)
   {
    *rc=EC_NULL_OBJECT_CODE;
    return((char *)NULL);
   }

  if((*object).successful!=EC_NO_ERROR)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECGetIdentifierType: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return((char *)NULL);
   }

  if(i>=0 && i<(*object).nSymbols)
   {
    switch((*object).symbolType[i])
     {
      case ECReal:
        strcpy(real,"Real");
        *rc=EC_NO_ERROR;
	return(real);
       
      case ECInteger:
        strcpy(integer,"Integer");
        *rc=EC_NO_ERROR;
	return(integer);

      case ECFunction:
        strcpy(function,"Function");
        *rc=EC_NO_ERROR;
	return(function);

      case ECNAObject:
        strcpy(naobject,"NAObject");
        *rc=EC_NO_ERROR;
	return(naobject);

      default:
        strcpy(undefined,"Undefined");
        *rc=EC_NO_ERROR;
	return(undefined);
     }
   }

  *rc=EC_IDENTIFIER_NOT_FOUND;
  none[0]=0x0;
  return(none);
 }
#ifdef __cplusplus
 }
#endif
