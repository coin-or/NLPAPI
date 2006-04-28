/*                                                          */
/* @(#)TokExpr.c	1.4                   */
/* 02/04/19 16:36:43               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmpI.h>
#include <ctype.h>
#include <string.h>

extern long ECBytesForTokenizedCode;

#ifdef __cplusplus
extern "C" {
#endif
struct ECTokenizedCode *ECTokenizeExpression(char *sourceCode)
{
 struct ECTokenizedCode *result;
 ECState              State;
 ECConstanttype       constantType=ECUnset;
 size_t               ichar;
 size_t               tokenStart=(size_t)0;
 int                  Continue;
 int                  symbolTableId;
 int                  integerTableId;
 int                  realTableId;
 int                  symbolFound;
 int                  integerValue=0;
 double               doubleValue=0.;
 int                  i;
 size_t               n;
 size_t               nchar;
 char                 *tempString;

 result=ECCreateTokenizedCode();
 nchar=strlen(sourceCode);

 if(nchar>4096)
  {
   if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode is longer than 4096 characters\n");
   (*result).successful=EC_LONG_SOURCE;
   return(result);
  }

 if(nchar<=0)
  {
   if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode is NULL\n");
   (*result).successful=EC_NULL_SOURCE;
   return(result);
  }

 ichar=0;

 State=ECLookingForToken;
 Continue=1;
 while(Continue)
  {
#ifdef VERBOSETOKENIZE
  ECPrintState(State);
  printf(" ichar is %d ==>%c<==\n",ichar,sourceCode[ichar]);
#endif
  switch(State)
   {
    case ECLookingForToken:
#ifdef VERBOSETOKENIZE
     printf("case LookingForToken\n");
#endif
     if(ichar>=nchar)
      {
#ifdef VERBOSETOKENIZE
        printf(" character is END OF STRING\n");
#endif
        Continue=0;
      }else{
       switch(sourceCode[ichar])
        {
         case ECBLANK:
#ifdef VERBOSETOKENIZE
          printf(" character is ECBLANK \n");
#endif
          ichar++;
          break;

/*    Single character tokens */
         case ECCOMMA:
#ifdef VERBOSETOKENIZE
          printf(" character is ECCOMMA \n");
#endif
          (*result).tokenType[(*result).nToken]=ECComma;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

         case ECLPAREN:
#ifdef VERBOSETOKENIZE
          printf(" character is ECLPAREN \n");
#endif
          (*result).tokenType[(*result).nToken]=ECLeftParen;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

         case ECRPAREN:
#ifdef VERBOSETOKENIZE
          printf(" character is ECRPAREN \n");
#endif
          (*result).tokenType[(*result).nToken]=ECRightParen;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

         case ECATSIGN:
#ifdef VERBOSETOKENIZE
          printf(" character is ECATSIGN \n");
#endif
          (*result).tokenType[(*result).nToken]=ECAtSign;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          ((*result).nToken)++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

         case ECPLUS:
#ifdef VERBOSETOKENIZE
          printf(" character is ECPLUS \n");
#endif
          (*result).tokenType[(*result).nToken]=ECPlus;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          ((*result).nToken)++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

         case ECMINUS:
#ifdef VERBOSETOKENIZE
          printf(" character is ECMINUS \n");
#endif
          (*result).tokenType[(*result).nToken]=ECMinus;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

         case ECSLASH:
#ifdef VERBOSETOKENIZE
          printf(" character is ECSLASH \n");
#endif
          (*result).tokenType[(*result).nToken]=ECSlash;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          ichar++;
          break;

/* Multiple character tokens */
         case ECSTAR:
#ifdef VERBOSETOKENIZE
          printf(" character is ECSTAR \n");
#endif
          tokenStart=ichar;
          State=ECLookingForMultiplyExponentiate;
          ichar++;
          break;

/* Multiple character tokens */

         case 0x0:
#ifdef VERBOSETOKENIZE
          printf(" character is END OF STRING\n");
#endif
          Continue=0;
          break;

         default:
          if(isalpha(sourceCode[ichar])||sourceCode[ichar]=='%')
           {
#ifdef VERBOSETOKENIZE
            printf(" character is alphabetic\n");
#endif
            tokenStart=ichar;
            State=ECLookingForIdentifier;
            ichar++;
           }else{
            if(isdigit(sourceCode[ichar])||sourceCode[ichar]==ECDECIMALPOINT)
             {
#ifdef VERBOSETOKENIZE
              printf(" character is numeric\n");
#endif
              tokenStart=ichar;
              State=ECLookingForConstant;
              constantType=ECInteger;
              if(sourceCode[ichar]==ECDECIMALPOINT)constantType=ECReal;
              ichar++;
             }else{
              if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: tokenize: Invalid character in source. 0x%2.2x \"%c\"\n",sourceCode[ichar],sourceCode[ichar]);
              (*result).successful=EC_INVALID_CHARACTER;
              return(result);
             }
           }
        }
      }
     break;

    case ECLookingForMultiplyExponentiate:
#ifdef VERBOSETOKENIZE
     printf("case LookingForMultiplyExponentiate\n");
#endif
     if(sourceCode[ichar]==ECSTAR)
      { 
#ifdef VERBOSETOKENIZE
       printf("character is ECSTAR\n");
#endif
       (*result).tokenType[(*result).nToken]=ECExponentiate;
#ifdef VERBOSETOKENIZE
       printf(" Token %d\n",(*result).nToken);
       {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
       (*result).nToken++;
       if((*result).nToken>4096)
        {
         if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
         (*result).successful=EC_TOO_MANY_TOKENS;
         return(result);
        }
       ichar++;
       State=ECLookingForToken;
      }else{
#ifdef VERBOSETOKENIZE
       printf("character is not ECSTAR\n");
#endif
       (*result).tokenType[(*result).nToken]=ECStar;
#ifdef VERBOSETOKENIZE
       printf(" Token %d\n",(*result).nToken);
       {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
       (*result).nToken++;
       if((*result).nToken>4096)
        {
         if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
         (*result).successful=EC_TOO_MANY_TOKENS;
         return(result);
        }
       State=ECLookingForToken;
       if(sourceCode[ichar]=='\0')Continue=0;
      }
     break;

    case ECLookingForIdentifier:
#ifdef VERBOSETOKENIZE
     printf("case LookingForIdentifier\n");
#endif
     if(isalpha(sourceCode[ichar])||isdigit(sourceCode[ichar])||sourceCode[ichar]=='%')
      {
#ifdef VERBOSETOKENIZE
       printf("character is alphanumeric\n");
#endif
       ichar++;
      }else{
       if(sourceCode[ichar]=='\0')Continue=0;
#ifdef VERBOSETOKENIZE
       printf("character is not alphanumeric\n");
#endif
       n=ichar-tokenStart;
       (*result).tokenType[(*result).nToken]=ECIdentifier;
       tempString=(char *)malloc((n+1)*sizeof(char));
       strncpy(tempString,sourceCode+tokenStart,n);
       tempString[n]=0x0;
#ifdef VERBOSETOKENIZE
       printf(" Token %d\n",(*result).nToken);
       {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf(" \"%s\" \n",tempString);}
#endif

/* Check to see if it's there already! */
       symbolTableId=0;
       symbolFound=0;
       if((*result).nSymbols>0)
        {
         for(i=0;i<(*result).nSymbols;i++)
          {
           if(!strcmp(tempString,(*result).symbolList[i]))
            {
             symbolTableId=i;
             symbolFound=1;
             break;
            }
          }
        }
       if(!symbolFound)
        {
         symbolTableId=(*result).nSymbols;
         (*result).nSymbols++;
         if((*result).nSymbols>4096)
          {
           if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 Identifiers\n");
           (*result).successful=EC_TOO_MANY_IDENTIFIERS;
           return(result);
          }
         ECBytesForTokenizedCode+=(n+1)*sizeof(char);
         (*result).symbolList[symbolTableId]=(char *)malloc((n+1)*sizeof(char));
         strcpy((*result).symbolList[symbolTableId],tempString);
        }
       (*result).tokenTable[(*result).nToken]=symbolTableId;
       free(tempString);

       (*result).nToken++;
       if((*result).nToken>4096)
        {
         if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
         (*result).successful=EC_TOO_MANY_TOKENS;
         return(result);
        }
       State=ECLookingForToken;
      }
     break;

    case ECLookingForConstant:
#ifdef VERBOSETOKENIZE
     printf("case LookingForConstant\n");
#endif
     if(isdigit(sourceCode[ichar]) || sourceCode[ichar]==ECDECIMALPOINT)
      {
#ifdef VERBOSETOKENIZE
       printf("character is numeric or ECDECIMALPOINT\n");
#endif
       if(sourceCode[ichar]==ECDECIMALPOINT)constantType=ECReal;
       ichar++;
      }else{
       if(sourceCode[ichar]=='\0')Continue=0;
#ifdef VERBOSETOKENIZE
       printf("character is not numeric or ECDECIMALPOINT\n");
#endif
       n=ichar-tokenStart;
       switch(constantType)
        {
         case ECInteger:
          (*result).tokenType[(*result).nToken]=ECIntegerConstant;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          tempString=(char *)malloc((n+1)*sizeof(char));
          strncpy(tempString,sourceCode+tokenStart,n);
          tempString[n]=0x0;
          sscanf(tempString,"%d",&integerValue);
          free(tempString);
#ifdef VERBOSETOKENIZE
          printf(" Value of Integer Constant is %d\n",integerValue);
#endif
/* Check to see if it's there already! */
          integerTableId=(*result).nIntegerConstants+10;
          if((*result).nIntegerConstants>0)
           {
            for(i=0;i<(*result).nIntegerConstants;i++)
             {
              if(integerValue==(*result).integerConstants[i])
               {
                integerTableId=i;
                break;
               }
             }
           }
          if(integerTableId==(*result).nIntegerConstants+10)
           {
            integerTableId=(*result).nIntegerConstants;
            (*result).nIntegerConstants++;
            if((*result).nIntegerConstants>4096)
             {
              if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 integer constants\n");
              (*result).successful=EC_TOO_MANY_INTEGERS;
              return(result);
             }
           }
 
          (*result).integerConstants[integerTableId]=integerValue;
          (*result).tokenTable[(*result).nToken]=integerTableId;
 
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          State=ECLookingForToken;
          break;
         case ECReal:
          (*result).tokenType[(*result).nToken]=ECRealConstant;
#ifdef VERBOSETOKENIZE
          printf(" Token %d\n",(*result).nToken);
          {ECPrintTokenType(stdout,(*result).tokenType[(*result).nToken]);printf("\n");}
#endif
          tempString=(char *)malloc((n+1)*sizeof(char));
          strncpy(tempString,sourceCode+tokenStart,n);
          tempString[n]=0x0;
          sscanf(tempString,"%le",&doubleValue);
          free(tempString);
#ifdef VERBOSETOKENIZE
          printf(" Value of Real Constant is %lf\n",doubleValue);
#endif
/* Check to see if it's there already! */
          realTableId=(*result).nRealConstants+10;
          if((*result).nRealConstants>0)
           {
            for(i=0;i<(*result).nRealConstants;i++)
             {
              if(doubleValue==(*result).realConstants[i])
               {
                realTableId=i;
                break;
               }
             }
           }
          if(realTableId==(*result).nRealConstants+10)
           {
            realTableId=(*result).nRealConstants;
            (*result).nRealConstants++;
            if((*result).nRealConstants>4096)
             {
              if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 real constants\n");
              (*result).successful=EC_TOO_MANY_REALS;
              return(result);
             }
           }
          ((*result).realConstants)[realTableId]=doubleValue;
          (*result).tokenTable[(*result).nToken]=realTableId;
 
          (*result).nToken++;
          if((*result).nToken>4096)
           {
            if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: sourceCode contains more than 4096 tokens\n");
            (*result).successful=EC_TOO_MANY_TOKENS;
            return(result);
           }
          State=ECLookingForToken;
          break;
         default:
          if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: Bad value for ConstantType\n");
          (*result).successful=EC_BAD_CONSTANT_TYPE;
          return(result);
        }
       State=ECLookingForToken;
       if(sourceCode[ichar]=='\0')Continue=0;
      }
     break;

    default:
     if(ECPrintErrorMessages)fprintf(stderr,"ECTokenizeExpression: Invalid character in source. 0x%2.2x \"%c\"\n",sourceCode[ichar],sourceCode[ichar]);
#ifdef VERBOSETOKENIZE
     printf(" %s\n",sourceCode);
     printf(" Invalid character in position %d\n",ichar);
     printf(" Character %c %2.2x \n",sourceCode[ichar],sourceCode[ichar]);
#endif
     (*result).successful=EC_INVALID_CHARACTER;
     return(result);
   }

  }
 (*result).successful=EC_NO_ERROR;
 return(result);
}
#ifdef __cplusplus
 }
#endif
