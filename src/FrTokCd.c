/*                                                          */
/* @(#)FrTokCd.c	1.3                   */
/* 02/04/19 16:24:40               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <stdlib.h>
#include <string.h>

extern long ECBytesForTokenizedCode;
extern int ECNTokenizedCodes;

#ifdef __cplusplus
extern "C" {
#endif
void ECFreeTokenizedCode(struct ECTokenizedCode *code)
 {
  int i;

  ECNTokenizedCodes--;

  if((*code).nSymbols>0)
   {
    for(i=0;i<(*code).nSymbols;i++)
     {
      if((*code).symbolList[i]!=(char *)NULL)
       {
        ECBytesForTokenizedCode-=(strlen(code->symbolList[i])+1)*sizeof(char);
        free((void*)(*code).symbolList[i]);
       }
     }
   }
  ECBytesForTokenizedCode-=sizeof(struct ECTokenizedCode);
  free((void*)code);
 
  return;
 }
#ifdef __cplusplus
 }
#endif
