/*                                                          */
/* @(#)PrtObjCd.c	1.2                   */
/* 02/04/19 16:34:35               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
 /*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ExpCmp.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
void ECPrintObjectCode(struct ECObjectCode *object)
{
 int i;
 int Destination;
 int Operand1;
 int Operand2;

 printf("Object Code: \n\n");
 printf("             %d statements\n",(*object).nStatements);
 printf("             %d identifiers\n",(*object).nSymbols);
 printf("             %d real constants\n",(*object).nRealConstants);
 printf("             %d integer constants\n",(*object).nIntegerConstants);
 printf("\nExecutable Code: \n\n");
 if((*object).nStatements>0)
  {
   for(i=0;i<(*object).nStatements;i++)
    {
     Destination=((*object).ExecutableCode[i]).Destination;
     Operand1=((*object).ExecutableCode[i]).Operand1;
     Operand2=((*object).ExecutableCode[i]).Operand2;

     switch(((*object).ExecutableCode[i]).Opcode)
      {
       case ECLoadIdentifierValue:
        printf(" Register %d = Identifier \"%s\"\n",Destination,(*object).symbolList[Operand1]);
        break;

       case ECLoadRealConstantValue:
        printf(" Register %d = RealConstant %f\n",Destination,(*object).realConstants[Operand1]);
        break;

       case ECLoadIntegerConstantValue:
        printf(" Register %d = IntegerConstant %d\n",Destination,(*object).integerConstants[Operand1]);
        break;

       case ECNegate:
        printf(" Register %d = - Register %d\n",Destination,Operand1);
        break;

       case ECAdd:
        printf(" Register %d = Register %d + Register %d\n",Destination,Operand1,Operand2);
        break;

       case ECSubtract:
        printf(" Register %d = Register %d - Register %d\n",Destination,Operand1,Operand2);
        break;

       case ECMultiply:
        printf(" Register %d = Register %d * Register %d\n",Destination,Operand1,Operand2);
        break;

       case ECDivide:
        printf(" Register %d = Register %d / Register %d\n",Destination,Operand1,Operand2);
        break;

       case ECCompose:
        printf(" Register %d = Register %d o Register %d\n",Destination,Operand1,Operand2);
        break;

       case ECPower:
        printf(" Register %d = Register %d ** Register %d\n",Destination,Operand1,Operand2);
        break;

       case ECCallWithNoArguments:
        printf(" Register %d = Call Identifier \"%s\" ()\n",Destination,(*object).symbolList[Operand1]);
        break;

       case ECCallWithOneArgument:
        printf(" Register %d = Call Identifier \"%s\" ( Register %d )\n",Destination,(*object).symbolList[Operand1],Operand2);
        break;

       default:
        printf(" Invalid Opcode\n");
        exit(8);
      }
    }
  }
 printf(" Return(Register %d)\n",(*object).returnRegister);
 return;
}
#ifdef __cplusplus
 }
#endif
