/*   PROGRAM NAME:  ExpressionCompiler v1.0             */

/*  @(#)ExpCmpI.h	1.3
    02/04/19 14:44:37                              

    PROGRAM NAME:  Manifold
   
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.
   
    Please refer to the LICENSE file in the top directory

    author: mhender@watson.ibm.com               */

#ifndef __ECExpressionCompilerInternalH__
#define __ECExpressionCompilerInternalH__

#include <stdlib.h>
#include <ExpCmp.h>

#ifdef __cplusplus
extern "C" {
#endif
struct ECObjectCode    *ECCreateObjectCode(void);
struct ECTokenizedCode *ECCreateTokenizedCode(void);
struct ECTokenizedCode *ECTokenizeExpression(char *);
void   ECFreeTokenizedCode(struct ECTokenizedCode *);
void   ECPrintTokenizedCode(struct ECTokenizedCode *);
void   ECPrintTokenType(FILE*,ECTokentype);
struct ECObjectCode *ECParseTokenizedCode(struct ECTokenizedCode *);
struct ECObjectCode *ECCopyObjectCode(struct ECObjectCode *);
struct ECExecutableCode ECCopyExecutableCode(struct ECExecutableCode);
int    ECParseE1(struct ECTokenizedCode *,struct ECObjectCode *object);
int    ECParseE2(struct ECTokenizedCode *,struct ECObjectCode *object);
int    ECParseE3(struct ECTokenizedCode *,struct ECObjectCode *object);
int    ECParseE4(struct ECTokenizedCode *,struct ECObjectCode *object);
void   ECAddOp(struct ECObjectCode *,ECOpcode, int, int , int );
void ECPrintState(ECState);
#ifdef __cplusplus
}
#endif

#endif
