/*                                                      */
/* @(#)PrsTokCd.c	1.2                   */
/* 02/04/19 16:34:26               */
/*                                                      */
/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */


/* April 23, 1996  Fixed ECParseE4 to return error when E4 starts with */
/*                    a wrong token (/,*...)                           */
/*                 Fixed several error messages not being directed to  */
/*                    stderr                                           */
/* May 13, 1996    Fixed Grammar to make negation and + lower priority */

#include <ExpCmpI.h>
#include <string.h>

extern long ECBytesForTokenizedCode;

int token;

#ifdef __cplusplus
extern "C" {
#endif
struct ECObjectCode *ECParseTokenizedCode(struct ECTokenizedCode *tokenized)
{
 struct ECObjectCode *object;
 int               i;
 size_t            length;
 int               expr;

 object=ECCreateObjectCode();
 if(((*tokenized).successful))
  {
   (*object).successful=(*tokenized).successful;
   return(object);
  }
 (*object).returnRegister=0;

 (*object).nSymbols=(*tokenized).nSymbols;
 for(i=0;i<(*tokenized).nSymbols;i++)
  {
   length=strlen((*tokenized).symbolList[i])+1;
   ECBytesForTokenizedCode+=length*sizeof(char);
   (*object).symbolList[i]=(char *)malloc(length*sizeof(char));
   strcpy((*object).symbolList[i],(*tokenized).symbolList[i]);
  }

 (*object).nRealConstants=(*tokenized).nRealConstants;
 for(i=0;i<(*tokenized).nRealConstants;i++)
  {
   (*object).realConstants[i]=(*tokenized).realConstants[i];
  }

 (*object).nIntegerConstants=(*tokenized).nIntegerConstants;
 for(i=0;i<(*tokenized).nIntegerConstants;i++)
  {
   (*object).integerConstants[i]=(*tokenized).integerConstants[i];
  }

/* Grammar:								 */
/*	    								 */
/* E1:          E1 + E2 | E1 - E2 | E2					 */
/* E2:          E2 * E3 | E2 / E3 |  -E3 | +E3 | E3                      */
/* E3:          E3**E4 | E4						 */
/* E4:          (E1) | id(E1) | id() | id | realC | integerC             */


 token=0;
 if((expr=ECParseE1(tokenized,object))==0||expr==-1)
  {
   if(ECPrintErrorMessages)fprintf(stderr," ECParseTokenizedCode: Expression is invalid\n");
   (*object).successful=EC_INVALID_EXPRESSION;
   return(object);
  }else{
   (*object).successful=EC_NO_ERROR;
  }

 if(token<(*tokenized).nToken)
  {
   if(ECPrintErrorMessages)
    {
     fprintf(stderr," ECParseTokenizedCode: Extra Characters following valid expression:\n");
     for(i=token;i<(*tokenized).nToken;i++)
      {
       fprintf(stderr," %d",i);
       ECPrintTokenType(stderr,(*tokenized).tokenType[i]);
       if((*tokenized).tokenType[i]==ECIdentifier)fprintf(stderr," \"%s\"",(*tokenized).symbolList[(*tokenized).tokenTable[i]]);
       fprintf(stderr,"\n");
      }
     }
   (*object).successful=EC_EXTRA_CHARACTERS;
  }

 return(object);
}

int ECParseE1(struct ECTokenizedCode *tokenized,struct ECObjectCode *object)
 {
  int expl,expr;
  int oldToken;

#ifdef VERBOSEPARSE
  printf("ECParseE1:\n");
#endif

  oldToken=token;

  if(token>(*tokenized).nToken)
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE1: 0\n");
#endif
    return(0);
   }

  if(!(expl=ECParseE2(tokenized,object)))
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE1: 0\n");
#endif
    return(0);
   }
  if(expl==-1)return(-1);

  while(TRUE)
   {
  switch((*tokenized).tokenType[token])
   {
    case ECPlus:
#ifdef VERBOSEPARSE
     printf(" ECParseE1: first Token is +\n");
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr,"ECParseE1: + cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE1: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE2(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE1: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     (*object).returnRegister++;
     ECAddOp(object,ECAdd,(*object).returnRegister,expl,expr);
     expl=(*object).returnRegister;
     break;
    case ECMinus:
#ifdef VERBOSEPARSE
     printf(" ECParseE1: first Token is -\n");
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr,"ECParseE1: - cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE1: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE2(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE1: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     (*object).returnRegister++;
     ECAddOp(object,ECSubtract,(*object).returnRegister,expl,expr);
     expl=(*object).returnRegister;
     break;
    default:
#ifdef VERBOSEPARSE
     printf(" done ECParseE1:\n");
#endif
     return(expl);
   }
   }
 }

int ECParseE2(struct ECTokenizedCode *tokenized,struct ECObjectCode *object)
 {
  int expl,expr;
  int oldToken;

#ifdef VERBOSEPARSE
  printf("ECParseE2:\n");
#endif

  oldToken=token;

  if(token>=(*tokenized).nToken)
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE2: 0\n");
#endif
    return(0);
   }

  switch((*tokenized).tokenType[token])
   {
    case ECPlus:
#ifdef VERBOSEPARSE
     printf(" ECParseE2: first Token is + %d\n",token);
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseE2: + cannot end an expression\n");
#ifdef VERBOSEPARSE
    printf(" done ECParseE2: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE2(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: 0\n");
#endif
       return(0);
      }
#ifdef VERBOSEPARSE
     printf(" done ECParseE2: %d\n",expr);
#endif
     return(expr);

    case ECMinus:
#ifdef VERBOSEPARSE
     printf(" ECParseE2: first Token is - %d\n",token);
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseE2: - cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE2(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     (*object).returnRegister++;
     ECAddOp(object,ECNegate,(*object).returnRegister,expr,0);
#ifdef VERBOSEPARSE
     printf(" done ECParseE2:\n");
#endif
     return((*object).returnRegister);
   }

  if(!(expl=ECParseE3(tokenized,object)))
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE2: 0\n");
#endif
    return(0);
   }
  if(expl==-1)return(-1);

  while(TRUE)
   {
  switch((*tokenized).tokenType[token])
   {
    case ECStar:
#ifdef VERBOSEPARSE
     printf(" ECParseE2: first Token is *\n");
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr,"ECParseE2: * cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE2(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     (*object).returnRegister++;
     ECAddOp(object,ECMultiply,(*object).returnRegister,expl,expr);
     expl=(*object).returnRegister;
     break;

    case ECAtSign:
#ifdef VERBOSEPARSE
     printf(" ECParseE2: first Token is @\n");
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr,"ECParseE2: @ cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE2(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     (*object).returnRegister++;
     ECAddOp(object,ECCompose,(*object).returnRegister,expl,expr);
     expl=(*object).returnRegister;
     break;

    case ECSlash:
#ifdef VERBOSEPARSE
     printf(" ECParseE2: first Token is /\n");
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr,"ECParseE2: / cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE3(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE2: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     (*object).returnRegister++;
     ECAddOp(object,ECDivide,(*object).returnRegister,expl,expr);
     expl=(*object).returnRegister;
     break;
    default:
#ifdef VERBOSEPARSE
     printf(" done ECParseE2:\n");
#endif
     return(expl);
   }
   }
 }

int ECParseE3(struct ECTokenizedCode *tokenized,struct ECObjectCode *object)
 {
  int expl,expr;
  int oldToken;

#ifdef VERBOSEPARSE
  printf("ECParseE3:\n");
#endif

  oldToken=token;

  if(token>=(*tokenized).nToken)
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE3: 0\n");
#endif
    return(0);
   }

  if(!(expl=ECParseE4(tokenized,object)))
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE3: 0\n");
#endif
    return(0);
   }
  if(expl==-1)return(-1);

  while(TRUE)
   {
  switch((*tokenized).tokenType[token])
   {
    case ECExponentiate:
#ifdef VERBOSEPARSE
     printf(" ECParseE3: first Token is **\n");
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseE3: ** cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE3: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE4(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE3: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);

     (*object).returnRegister++;
     ECAddOp(object,ECPower,(*object).returnRegister,expl,expr);
     expl=(*object).returnRegister;
     break;
    default:
#ifdef VERBOSEPARSE
     printf(" done ECParseE3 %d:\n",expl);
#endif
     return(expl);
   }
   }
 }

int ECParseE4(struct ECTokenizedCode *tokenized,struct ECObjectCode *object)
 {
  int expr;
  int oldToken;
  int firstToken;
  int table;

#ifdef VERBOSEPARSE
  printf("ECParseE4:\n");
#endif

  oldToken=token;

  if(token>(*tokenized).nToken)
   {
    token=oldToken;
#ifdef VERBOSEPARSE
    printf(" done ECParseE4: 0\n");
#endif
    return(0);
   }

  switch((*tokenized).tokenType[token])
   {
    case ECLeftParen:
#ifdef VERBOSEPARSE
     printf(" ECParseE4: first Token is ( %d\n",token);
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseE4: ( cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);
      }
     if((*tokenized).tokenType[token]==ECRightParen)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseTokenizedCode: () is invalid in an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);
      }
     if(!(expr=ECParseE1(tokenized,object)))
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: 0\n");
#endif
       return(0);
      }
     if(expr==-1)return(-1);
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseTokenizedCode: Unmatched (\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);
      }
     if((*tokenized).tokenType[token]!=ECRightParen)
      {
       token=oldToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: 0\n");
#endif
       return(0);
      }
     token++;
#ifdef VERBOSEPARSE
     printf(" done ECParseE4: %d\n",expr);
#endif
     return(expr);

    case ECIdentifier:
     table=(*tokenized).tokenTable[token];
     if(token>(*tokenized).nToken-1)
      {
#ifdef VERBOSEPARSE
       printf(" ECParseE4: first Token is Identifier %d EOL\n",token);
#endif
       token++;
       (*object).returnRegister++;
       ECAddOp(object,ECLoadIdentifierValue,(*object).returnRegister,table,0);
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
       return((*object).returnRegister);
      }
     token++;
     firstToken=token;
     if((*tokenized).tokenType[token]!=ECLeftParen)
      {
#ifdef VERBOSEPARSE
       printf(" ECParseE4: first Token is Identifier %d non (\n",token);
#endif
       (*object).returnRegister++;
       ECAddOp(object,ECLoadIdentifierValue,(*object).returnRegister,table,0);
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
       return((*object).returnRegister);
      }
#ifdef VERBOSEPARSE
     printf(" ECParseE4: first Token is Identifier %d(\n",token);
#endif
     token++;
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseE4: ( cannot end an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);
      }
     if((*tokenized).tokenType[token]==ECRightParen)
      {
       token++;
       (*object).returnRegister++;
       ECAddOp(object,ECCallWithNoArguments,(*object).returnRegister,table,0);
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
       return((*object).returnRegister);
      }
     if(!(expr=ECParseE1(tokenized,object)))
      {
       (*object).returnRegister++;
       ECAddOp(object,ECLoadIdentifierValue,(*object).returnRegister,table,0);
       token=firstToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
       return((*object).returnRegister);
      }
     if(expr==-1)return(-1);
     if(token>(*tokenized).nToken-1)
      {
       token=oldToken;
       if(ECPrintErrorMessages)
         fprintf(stderr," ECParseTokenizedCode: Unmatched (\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);
      }
     if((*tokenized).tokenType[token]!=ECRightParen)
      {
       (*object).returnRegister++;
       ECAddOp(object,ECLoadIdentifierValue,(*object).returnRegister,table,0);
       token=firstToken;
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
       return((*object).returnRegister);
      }
     token++;
     (*object).returnRegister++;
     ECAddOp(object,ECCallWithOneArgument,(*object).returnRegister,table,expr);
#ifdef VERBOSEPARSE
     printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
     return((*object).returnRegister);

    case ECRealConstant:
     table=(*tokenized).tokenTable[token];
#ifdef VERBOSEPARSE
     printf(" ECParseE4: first Token is RealConstant %d\n",token);
#endif
     (*object).returnRegister++;
     ECAddOp(object,ECLoadRealConstantValue,(*object).returnRegister,table,0);
     token++;
#ifdef VERBOSEPARSE
     printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
     return((*object).returnRegister);

    case ECIntegerConstant:
     table=(*tokenized).tokenTable[token];
#ifdef VERBOSEPARSE
     printf(" ECParseE4: first Token is IntegerConstant %d\n",token);
#endif
     (*object).returnRegister++;
     ECAddOp(object,ECLoadIntegerConstantValue,(*object).returnRegister,table,0);
     token++;
#ifdef VERBOSEPARSE
     printf(" done ECParseE4: %d\n",(*object).returnRegister);
#endif
     return((*object).returnRegister);

    case ECExponentiate:
     if(ECPrintErrorMessages)
       fprintf(stderr," ECParseE4: ** cannot begin an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);

    case ECComma:
     if(ECPrintErrorMessages)
       fprintf(stderr," ECParseE4: , cannot begin an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);

    case ECSlash:
     if(ECPrintErrorMessages)
       fprintf(stderr," ECParseE4: / cannot begin an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);

    case ECStar:
     if(ECPrintErrorMessages)
       fprintf(stderr," ECParseE4: * cannot begin an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);

    case ECAtSign:
     if(ECPrintErrorMessages)
       fprintf(stderr," ECParseE4: @ cannot begin an expression\n");
#ifdef VERBOSEPARSE
       printf(" done ECParseE4: -1\n");
#endif
       return(-1);

    default:
     if(ECPrintErrorMessages)
       fprintf(stderr," ECParseTokenizedCode: expression begins with unknown token\n");
#ifdef VERBOSEPARSE
     printf(" done ECParseE4: -1\n");
#endif
     return(-1);
   }
 }

void ECAddOp(struct ECObjectCode *object,ECOpcode Opcode, int Destination, int Operand1, int Operand2)
 {
  int statement;
  struct ECExecutableCode *code;

  statement=(*object).nStatements;
  code=(*object).ExecutableCode+statement;
  (*code).Opcode=Opcode;
  (*code).Destination=Destination;
  (*code).Operand1=Operand1;
  (*code).Operand2=Operand2;
  (*object).nStatements++;
  if(Destination>object->nRegisters)object->nRegisters=Destination;
  if(Operand1>object->nRegisters)object->nRegisters=Operand1;
  if(Operand2>object->nRegisters)object->nRegisters=Operand2;
  return;
 }
#ifdef __cplusplus
 }
#endif
