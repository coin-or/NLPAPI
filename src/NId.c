/*                                                          */
/* @(#)NId.c	1.2                   */
/* 02/04/19 16:33:46               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <ECMsg.h>

#ifdef __cplusplus
extern "C" {
#endif
int ECNumberOfIdentifiers(struct ECObjectCode *object, int *rc)
 {
  if(object==NULL)return(EC_NULL_OBJECT_CODE);

  if((*object).successful!=EC_NO_ERROR)
   {;
    if(ECPrintErrorMessages)fprintf(stderr,"ECNumberOfIdentifiers: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return(0);
   }

  *rc=EC_NO_ERROR;
  return((*object).nSymbols);
 }
#ifdef __cplusplus
 }
#endif
