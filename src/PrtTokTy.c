/*                                                          */
/* @(#)PrtTokTy.c	1.2                   */
/* 02/04/19 16:35:10               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

/*     April 23, 1996 changed to write to FILE                */

#include <ExpCmpI.h>

#ifdef __cplusplus
extern "C" {
#endif
void ECPrintTokenType(FILE *fid,ECTokentype T)
{
 switch(T)
  {
   case ECUndefined:
    fprintf(fid," Undefined");
    break;
   case ECComma:
    fprintf(fid," Comma");
    break;
   case ECLeftParen:
    fprintf(fid," Left Parenthesis");
    break;
   case ECRightParen:
    fprintf(fid," Right Parenthesis");
    break;
   case ECPlus:
    fprintf(fid," Plus");
    break;
   case ECMinus:
    fprintf(fid," Minus");
    break;
   case ECStar:
    fprintf(fid," Star");
    break;
   case ECAtSign:
    fprintf(fid," At Sign");
    break;
   case ECSlash:
    fprintf(fid," Slash");
    break;
   case ECExponentiate:
    fprintf(fid," Exponentiate");
    break;
   case ECIdentifier:
    fprintf(fid," Identifier");
    break;
   case ECIntegerConstant:
    fprintf(fid," IntegerConstant");
    break;
   case ECRealConstant:
    fprintf(fid," RealConstant");
    break;
   default:
    fprintf(fid," TokenType invalid");
  }
 return;
}
#ifdef __cplusplus
 }
#endif
