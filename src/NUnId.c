/*                                                          */
/* @(#)NUnId.c	1.2                   */
/* 02/04/19 16:34:14               */
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
int ECNumberOfUnsetIdentifiers(struct ECObjectCode *object, int *rc)
 {
  int i;
  int n;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);

  if((*object).successful)
   {
    printf("ECNumberOfUnsetIdentifiers: compilation was unsuccessful\n");
    *rc=(*object).successful;
    return(0);
   }

  n=0;
  if((*object).nSymbols>0)
   {
    for(i=0;i<(*object).nSymbols;i++)
     {
      if((*object).symbolType[i]==ECUnset)n++;
     }
   }

  *rc=EC_NO_ERROR;
  return(n);
 }
#ifdef __cplusplus
 }
#endif
