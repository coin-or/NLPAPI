/*                                                          */
/* @(#)FrObjCd.c	1.3                   */
/* 02/04/19 16:24:32               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <stdlib.h>
#include <string.h>

extern long ECBytesForObjectCode;
extern int ECNObjectCodes;

#ifdef __cplusplus
extern "C" {
#endif
void ECFreeObjectCode(struct ECObjectCode **object)
 {
  int i;

  if((*object)==NULL)return;

  ECNObjectCodes--;
 
  ECBytesForObjectCode-=1024*sizeof(struct ECExecutableCode);
  free((*object)->ExecutableCode);

  for(i=0;i<257;i++)
   {
    if((*object)->symbolList[i]!=(char *)NULL)
     {
      ECBytesForObjectCode-=(strlen((*object)->symbolList[i])+1)*sizeof(char);
      free((*object)->symbolList[i]);
     }
   }

  ECBytesForObjectCode-=sizeof(struct ECObjectCode);
  free((*object));

  return;
 }
#ifdef __cplusplus
 }
#endif
