/*                                                          */
/* @(#)PrtSymTb.c	1.2                   */
/* 02/04/19 16:34:52               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>

#ifdef __cplusplus
extern "C" {
#endif
void ECPrintSymbolTable(struct ECObjectCode *object)
{
 int i;

 printf("\nSymbol Table:\n\n");
 if((*object).nSymbols>0)
  {
   for(i=0;i<(*object).nSymbols;i++)
    {
     printf(" Identifier \"%s\",",(*object).symbolList[i]);
     if((*object).symbolType[i]==ECUnset)
      {
       printf(" is Undefined \n");
      }else{
       if((*object).symbolType[i]==ECReal)printf(" is the Float %f\n",(*object).realConstants[(*object).symbolValue[i]]);
       if((*object).symbolType[i]==ECInteger)printf(" is the Integer %d\n",(*object).integerConstants[(*object).symbolValue[i]]);
       if((*object).symbolType[i]==ECFunction)printf(" is a Function\n");
      }
    }
  }
 return;
}
#ifdef __cplusplus
 }
#endif
