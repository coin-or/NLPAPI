/*                                                          */
/* @(#)PrtTokCd.c	1.2                   */
/* 02/04/19 16:35:00               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmpI.h>

#ifdef __cplusplus
extern "C" {
#endif
void ECPrintTokenizedCode(struct ECTokenizedCode *tokenized)
{
 int i;

 printf("Tokens:\n");
 if((*tokenized).nToken>0)
  {
   for(i=0;i<(*tokenized).nToken;i++)
    {
     ECPrintTokenType(stdout,(*tokenized).tokenType[i]);
     if((*tokenized).tokenTable[i]!=-1)
      {
       printf(" %d",(*tokenized).tokenTable[i]);
       if((*tokenized).tokenType[i]==ECIdentifier)printf(" \"%s\"",(*tokenized).symbolList[(*tokenized).tokenTable[i]]);
       if((*tokenized).tokenType[i]==ECIntegerConstant)printf(" %d",(*tokenized).integerConstants[(*tokenized).tokenTable[i]]);
       if((*tokenized).tokenType[i]==ECRealConstant)printf(" %f",(*tokenized).realConstants[(*tokenized).tokenTable[i]]);
      }
     printf("\n");
    }
  }
 printf("\n");
/*
 printf("Symbol Table:\n");
 if((*tokenized).nSymbols>0)
  {
   for(i=0;i<(*tokenized).nSymbols;i++)
    {
     printf(" %d ==>%s<==\n",i,(*tokenized).symbolList[i]);
    }
  }
 printf("\n");
 printf("Integer Constant Table:\n");
 if((*tokenized).nIntegerConstants>0)
  {
   for(i=0;i<(*tokenized).nIntegerConstants;i++)
    {
     printf(" %d %d\n",i,(*tokenized).integerConstants[i]);
    }
  }
 printf("\n");
 printf("Real Constant Table:\n");
 if((*tokenized).nRealConstants>0)
  {
   for(i=0;i<(*tokenized).nRealConstants;i++)
    {
     printf(" %d %f (address %d)\n",i,(*tokenized).realConstants[i],(*tokenized).realConstants+i);
    }
  }
*/
 return;
}
#ifdef __cplusplus
 }
#endif
