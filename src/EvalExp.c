/*                                                          */
/* @(#)EvalExp.c	1.3                   */
/* 03/02/21 09:15:24               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <ECMsg.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
/*#define VERBOSEEVALUATE 1*/
#define ERROUT stdout

#ifdef __cplusplus
extern "C" {
#endif
double ECEvaluateExpression(struct ECObjectCode *object,int *rc)
 {
  static int i;

  static ECOpcode Opcode;
  static int Destination;
  static int Operand1;
  static int Operand2;

  static double r1,r2;
 
  static double Register[256];
  static ECPointerToFunction   function;
  static struct ECExecutableCode *instruction;
  static ECConstanttype type;

  if(errno>0){printf("ECEvaluateExpression %s line %d: %s\n",__FILE__,__LINE__,strerror(errno));fflush(stdout);}

/* Check input */

 if(object==NULL)
  {
   if(ECPrintErrorMessages)fprintf(ERROUT,"ECEvaluateExpression: object code is null.\n");
   *rc=EC_NULL_OBJECT_CODE;
#ifdef FLT_QNAN
   return(FLT_QNAN);
#else
   return(0.);
#endif
  }

 if((*object).successful!=EC_NO_ERROR)
  {
   if(ECPrintErrorMessages)fprintf(ERROUT,"ECEvaluateExpression: compilation was unsuccessful\n");
   *rc=(*object).successful;
#ifdef FLT_QNAN
   return(FLT_QNAN);
#else
   return(0.);
#endif
  }

 if(ECNumberOfUnsetIdentifiers(object,rc)>0)
  {

   if(ECPrintErrorMessages)fprintf(ERROUT,"ECEvaluateExpression: Some identifiers have not been given values\n");
   ECPrintSymbolTable(object);
   *rc=EC_IDENTIFIERS_NOT_SET;
#ifdef FLT_QNAN
   return(FLT_QNAN);
#else
   return(0.);
#endif
  }

/* Execute */

 for(i=0;i<(*object).nStatements;i++)
  {
   instruction=(*object).ExecutableCode+i;
   Opcode=instruction->Opcode;
   Destination=instruction->Destination;
   Operand1=instruction->Operand1;
   Operand2=instruction->Operand2;
   if(errno>0)
    {
     printf("ECEvaluateExpression %s line %d: %s, statment %d\n",__FILE__,__LINE__,strerror(errno),i);
     ECPrintSymbolTable(object);fflush(stdout);
     ECPrintObjectCode(object);fflush(stdout);
     if(i>0)
      {
       instruction=(*object).ExecutableCode+i-1;
       Opcode=instruction->Opcode;
       Destination=instruction->Destination;
       Operand1=instruction->Operand1;
       Operand2=instruction->Operand2;
       printf("The previous statment was opcode %d, Operand1=%d, Operand2=%d, Destination %d\n",Opcode,Destination,Operand1,Operand2);
      }
     fflush(stdout);
     abort();
    }

   switch(Opcode)
    {
     case ECLoadIdentifierValue:
      type=(*object).symbolType[Operand1];
      switch(type)
       {
        case ECReal:
         Register[Destination]=(*object).realConstants[(*object).symbolValue[Operand1]];
#ifdef VERBOSEEVALUATE
         printf("  LoadIdentifierValue(real): (Register %d = %f)\n",Destination,Register[Destination]);
#endif
         break;

        case ECInteger:
         Register[Destination]=(*object).integerConstants[(*object).symbolValue[Operand1]];
#ifdef VERBOSEEVALUATE
         printf("  LoadIdentifierValue(integer): (Register %d = %f)\n",Destination,Register[Destination]);
#endif
         break;

        default:
         if(ECPrintErrorMessages)fprintf(ERROUT,"ECEvaluateExpression: Bad constant type in object code %d\n",type);
         *rc=EC_INVALID_CONSTANT_TYPE;
#ifdef FLT_QNAN
         return(FLT_QNAN);
#else
         return(0.);
#endif
       }
      break;

     case ECLoadRealConstantValue:
      Register[Destination]=(*object).realConstants[Operand1];
#ifdef VERBOSEEVALUATE
      printf("  LoadRealConstantValue: (Register %d = %f)\n",Destination,Register[Destination]);
#endif
      break;

     case ECLoadIntegerConstantValue:
      Register[Destination]=(*object).integerConstants[Operand1];
#ifdef VERBOSEEVALUATE
      printf("  LoadIntegerConstantValue: (Register %d = %f)\n",Destination,Register[Destination]);
#endif
      break;

     case ECNegate:
      Register[Destination]=-Register[Operand1];
#ifdef VERBOSEEVALUATE
      printf("  Negate: (Register %d = -(%f) = %f)\n",Destination,Register[Operand1],Register[Destination]);
#endif
      break;

     case ECAdd:
      Register[Destination]=Register[Operand1]+Register[Operand2];
#ifdef VERBOSEEVALUATE
      printf("  Add: (Register %d = %f+%f = %f)\n",Destination,Register[Operand1],Register[Operand2],Register[Destination]);
#endif
      break;

     case ECSubtract:
      Register[Destination]=Register[Operand1]-Register[Operand2];
#ifdef VERBOSEEVALUATE
      printf("  Subtract: (Register %d = %f-%f = %f)\n",Destination,Register[Operand1],Register[Operand2],Register[Destination]);
#endif
      break;

     case ECMultiply:
      r1=Register[Operand1];
      r2=Register[Operand2];
      if(r1==0. || r2==0.)
       {
        Register[Destination]=0.;
       }else{
        Register[Destination]=r1*r2;
       }
#ifdef VERBOSEEVALUATE
      printf("  Multiply: (Register %d = %f*%f = %f)\n",Destination,Register[Operand1],Register[Operand2],Register[Destination]);
#endif
      break;

     case ECDivide:
      Register[Destination]=Register[Operand1]/Register[Operand2];
#ifdef VERBOSEEVALUATE
      printf("  (Register %d = %f/%f = %f)\n",Destination,Register[Operand1],Register[Operand2],Register[Destination]);
#endif
      break;

     case ECPower:
      Register[Destination]=pow(Register[Operand1],Register[Operand2]);
#ifdef VERBOSEEVALUATE
      printf("  (Register %d = pow(%f,%f) = %f)\n",Destination,Register[Operand1],Register[Operand2],Register[Destination]);
#endif
      break;

     case ECCallWithNoArguments:
      Register[Destination]=(*(*object).functions[(*object).symbolValue[Operand1]])(0.);
#ifdef VERBOSEEVALUATE
      printf("  (Register %d = %s() = %f)\n",Destination,(*object).symbolList[Operand1],Register[Destination]);
#endif
      break;

     case ECCallWithOneArgument:
      function=(*object).functions[(*object).symbolValue[Operand1]];
      Register[Destination]=(*function)(Register[Operand2]);
#ifdef VERBOSEEVALUATE
      printf("  (Register %d = %s(%f) = %f)\n",Destination,(*object).symbolList[Operand1],Register[Operand2],Register[Destination]);
#endif
      break;

     default:
      if(ECPrintErrorMessages)fprintf(ERROUT,"ECEvaluateExpression: Bad opcode in object code %d\n",Opcode);
      *rc=EC_INVALID_OPCODE;
#ifdef FLT_QNAN
      return(FLT_QNAN);
#else
      return(0.);
#endif
    }
  }
#ifdef VERBOSEEVALUATE
 printf(" Return(Register %d)\n",(*object).returnRegister);
 printf("  (%f)\n",Register[(*object).returnRegister]);
#endif
 *rc=EC_NO_ERROR;
 return(Register[(*object).returnRegister]);
}
#ifdef __cplusplus
}
#endif
