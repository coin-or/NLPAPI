/*  @(#)CpObjCd.c	1.2
    02/04/19 16:17:25

    PROGRAM NAME:  Manifold

    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory

    author: mhender@watson.ibm.com               */

#include <ExpCmpI.h>
#include <string.h>

extern long ECBytesForObjectCode;

#ifdef __cplusplus
extern "C" {
#endif
struct ECObjectCode *ECCopyObjectCode(struct ECObjectCode *oCode)
 {
  struct ECObjectCode *result;
  int                  i;
  size_t               length;

  result=ECCreateObjectCode();
  (*result).successful=(*oCode).successful;
  (*result).returnRegister=(*oCode).returnRegister;

  (*result).nRegisters=oCode->nRegisters;
 
  (*result).nSymbols=(*oCode).nSymbols;
  for(i=0;i<(*oCode).nSymbols;i++)
   {
    length=strlen((*oCode).symbolList[i])+1;
    ECBytesForObjectCode+=length*sizeof(char);
    (*result).symbolList[i]=(char *)malloc(length*sizeof(char));
    strcpy((*result).symbolList[i],(*oCode).symbolList[i]);
    (*result).symbolType[i]=(*oCode).symbolType[i];
   }

  (*result).nRealConstants=(*oCode).nRealConstants;
  for(i=0;i<(*oCode).nRealConstants;i++)
   {
    (*result).realConstants[i]=(*oCode).realConstants[i];
   }

  (*result).nIntegerConstants=(*oCode).nIntegerConstants;
  for(i=0;i<(*oCode).nIntegerConstants;i++)
   {
    (*result).integerConstants[i]=(*oCode).integerConstants[i];
   }

  (*result).nFunctions=(*oCode).nFunctions;
  for(i=0;i<(*oCode).nFunctions;i++)
   {
    (*result).functions[i]=(*oCode).functions[i];
   }

  (*result).nStatements=(*oCode).nStatements;
  for(i=0;i<(*oCode).nStatements;i++)
   {
    (*result).ExecutableCode[i]=ECCopyExecutableCode((*oCode).ExecutableCode[i]);
   }

  return(result);
 }
#ifdef __cplusplus
}
#endif
