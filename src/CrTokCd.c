/*                                                          */
/*  @(#)CrTokCd.c	1.3                                                     */
/*  02/04/19 16:19:44                                                 */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */
/*  Author: Michael E. Henderson, mhender@watson.ibm.com */

#include <ExpCmp.h>
#include <stdlib.h>

long ECBytesForTokenizedCode=0;
int ECNTokenizedCodes=0;

#ifdef __cplusplus
extern "C" {
#endif
struct ECTokenizedCode *ECCreateTokenizedCode()
 {
  struct ECTokenizedCode *result;
  int i;

  ECNTokenizedCodes++;

  ECBytesForTokenizedCode+=sizeof(struct ECTokenizedCode);
  result=(struct ECTokenizedCode *)malloc(sizeof(struct ECTokenizedCode));
  (*result).nToken=0;
  (*result).nSymbols=0;
  (*result).nIntegerConstants=0;
  (*result).nRealConstants=0;
 
  for(i=0;i<4096;i++)
   {
    (*result).tokenTable[i]=0;
    (*result).tokenType[i]=ECUndefined;
    (*result).symbolList[i]=NULL;
    (*result).integerConstants[i]=0;
    (*result).realConstants[i]=0.;
   }
 
  return(result);
 }
#ifdef __cplusplus
}
#endif
