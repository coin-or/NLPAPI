/* @(#)CmpExpr.c	1.2
   02/05/03 09:47:19 
 
    PROGRAM NAME:  Manifold
   
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.
   
    Please refer to the LICENSE file in the top directory

    author: mhender@watson.ibm.com               */

#include <ExpCmpI.h>
 
#ifdef __cplusplus
extern "C" {
#endif
int ECCompileExpression(char *sourceCode,struct ECObjectCode **object)
 {
  struct ECTokenizedCode *tokenized;
  int rc;
 
  tokenized=ECTokenizeExpression(sourceCode);
  if((*tokenized).successful!=0)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECCompileExpression: tokenization failed\n");
    rc=(*tokenized).successful;
    ECFreeTokenizedCode(tokenized);
    return(rc);
   }

  *object=ECParseTokenizedCode(tokenized);
  if((*object)->successful!=0)
   {
    if(ECPrintErrorMessages)fprintf(stderr,"ECCompileExpression: parsing failed\n");
    rc=(*object)->successful;
    ECFreeTokenizedCode(tokenized);
    ECFreeObjectCode(object);
    return(rc);
   }
  ECFreeTokenizedCode(tokenized);

  return((*object)->successful);
 }
#ifdef __cplusplus
 }
#endif
