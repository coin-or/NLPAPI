/*                                                          */
/*  @(#)CrObjCd.c	1.3                                                     */
/*  02/04/19 16:18:56                                                 */
/*                                                          */


/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
/*   Please refer to the LICENSE file in the top directory*/

/*   Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <stdlib.h>

long ECBytesForObjectCode=0;
int ECNObjectCodes=0;

#ifdef __cplusplus
extern "C" {
#endif
struct ECObjectCode *ECCreateObjectCode()
 {
  struct ECObjectCode *result;
  int i;
 
  ECNObjectCodes++;

  ECBytesForObjectCode+=sizeof(struct ECObjectCode);
  result=(struct ECObjectCode *)malloc(sizeof(struct ECObjectCode));
  ECBytesForObjectCode+=1024*sizeof(struct ECExecutableCode);
  result->ExecutableCode=(struct ECExecutableCode *)malloc(1024*sizeof(struct ECExecutableCode));

  result->successful=0;
  result->nRegisters=0;
  result->nStatements=0;
  result->nSymbols=0;
  result->nIntegerConstants=0;
  result->nRealConstants=0;
  result->nFunctions=0;
  result->nNAObjects=0;


  for(i=0;i<4096;i++)
   {
    result->symbolList[i]=NULL;
    result->symbolType[i]=ECUnset;
    result->symbolValue[i]=0;
    result->integerConstants[i]=0;
    result->realConstants[i]=0.;
    result->functions[i]=NULL;
    result->NAObjects[i]=NULL;
   }

  result->nStatements=0;
  for(i=0;i<1024;i++)
   {
    (result->ExecutableCode[i]).Destination=0;
    (result->ExecutableCode[i]).Opcode=ECInvalidOpCode;
    (result->ExecutableCode[i]).Operand1=0;
    (result->ExecutableCode[i]).Operand2=0;
   }

  return(result);
 }
#ifdef __cplusplus
}
#endif
