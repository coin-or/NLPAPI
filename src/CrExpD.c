/*  @(#)CrExpD.c	1.4
    02/05/03 10:17:06

    PROGRAM NAME:  Manifold

    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory

    author: mhender@watson.ibm.com               */

#include <ExpCmpI.h>
#include <ExpCmp.h>

extern long ECBytesForObjectCode;

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
int ECCreateExpressionDerivative(struct ECObjectCode *object,char *variable,struct ECObjectCode **result)
 {
  int i,n;
  int m;
  int newRegister;
  int r1,r2,r3,r4,r5,r6,r7,r8,r9;
  int s1,s2,s3,s4,s5,s6,s7,s8,s9;
  int zero,one;
  ECOpcode Opcode;
  int Destination;
  int Operand1;
  int Operand2;
  int derivative[256]={0};
  int function;
  int j,found;
  char *name;
  int   rc;
  double realValue;
  int   integerValue;
  ECPointerToFunction functionValue;
  extern double EClog(double);

  int ilog;

  *result=NULL;
  if(object==NULL)return(EC_NULL_OBJECT_CODE);

/* Copy the symbol tables from the expressions ECObjectCode (duplicate symbols) */
/*    and external functions */

  *result=ECCreateObjectCode();
  (*result)->successful=EC_NO_ERROR;

  n=object->nSymbols;
  for(i=0;i<n;i++)
   {
    ECBytesForObjectCode+=(strlen(object->symbolList[i])+1)*sizeof(char);
    (*result)->symbolList[i]=
           (char *)malloc((strlen(object->symbolList[i])+1)*sizeof(char));
    if((*result)->symbolList[i]==(char*)NULL)
     {
      fprintf(stderr,"ECCreateExpressionDerivative: %d problem mallocing a string of length %d\n",__LINE__,strlen(object->symbolList[i])+1);
      exit(8);
     }
    strcpy((*result)->symbolList[i],object->symbolList[i]);
    (*result)->symbolType[i]=object->symbolType[i];
    (*result)->symbolValue[i]=object->symbolValue[i];
   }
  m=n;
  n=object->nStatements;
  for(i=0;i<n;i++)
   {
    Opcode=(object->ExecutableCode[i]).Opcode;
    if(Opcode==ECCallWithOneArgument)
     {
      Operand1=(object->ExecutableCode[i]).Operand1;
      name=(char *)malloc((strlen(object->symbolList[Operand1])+2)*sizeof(char));
      if(name==(char*)NULL)
       {
        fprintf(stderr,"ECCreateExpressionDerivative: %d problem mallocing a string of length %d\n",__LINE__,strlen(object->symbolList[Operand1])+2);
        exit(8);
       }
      strcpy(name,"D");
      strcat(name,object->symbolList[Operand1]);
      found=FALSE;
      for(j=0;j<m;j++)
       {
        if(!strcmp(name,(*result)->symbolList[j]))
         {
          found=TRUE;
          derivative[Operand1]=j;
         }
       }
      if(!found)
       {
        ECBytesForObjectCode+=(strlen(name)+1)*sizeof(char);
        (*result)->symbolList[m]=(char *)malloc((strlen(name)+1)*sizeof(char));
        if((*result)->symbolList[m]==(char*)NULL)
         {
          fprintf(stderr,"ECCreateExpressionDerivative: %d problem mallocing a string of length %d\n",__LINE__,strlen(name)+1);
          exit(8);
         }
        strcpy((*result)->symbolList[m],name);
        (*result)->symbolType[m]=ECUnset;
        (*result)->symbolValue[m]=0;
        derivative[Operand1]=m;
        m++;
       }
      free((void*)name);
     }
   }
  (*result)->nSymbols=m;

  n=object->nIntegerConstants;
  for(i=0;i<n;i++)
   {
    (*result)->integerConstants[i]=object->integerConstants[i];
   }
  (*result)->nIntegerConstants=n;

  n=object->nRealConstants;
  for(i=0;i<n;i++)
   {
    (*result)->realConstants[i]=object->realConstants[i];
   }
  (*result)->realConstants[n]=0.;
  zero=n;
  (*result)->realConstants[n+1]=1.;
  one=n+1;
  (*result)->nRealConstants=n+2;

  n=object->nFunctions;
  for(i=0;i<(*result)->nFunctions;i++)
   {
    (*result)->functions[i]=object->functions[i];
   }
  (*result)->functions[n]=EClog;
  (*result)->nFunctions=n+1;
  m=(*result)->nSymbols;
  ECBytesForObjectCode+=4*sizeof(char);
  (*result)->symbolList[m]=(char *)malloc(4*sizeof(char));
  if((*result)->symbolList[m]==(char*)NULL)
   {
    fprintf(stderr,"ECCreateExpressionDerivative: %d problem mallocing a string of length %d\n",__LINE__,4);
    exit(8);
   }
  strcpy((*result)->symbolList[m],"log");
  (*result)->symbolType[m]=ECFunction;
  (*result)->symbolValue[m]=n;
  ilog=m;
  (*result)->nSymbols=m+1;

  n=object->nStatements;
  (*result)->nStatements=0;
  (*result)->returnRegister=object->returnRegister+n;
  newRegister=(*result)->returnRegister+1;
  for(i=0;i<n;i++)
   {
    Opcode=(object->ExecutableCode[i]).Opcode;
    Destination=(object->ExecutableCode[i]).Destination;
    Operand1=(object->ExecutableCode[i]).Operand1;
    Operand2=(object->ExecutableCode[i]).Operand2;

    m=(*result)->nStatements;
    if(m>1024)
     {
      fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
      exit(8);
     }
    (*result)->ExecutableCode[m].Opcode=Opcode;
    (*result)->ExecutableCode[m].Destination=Destination;
    (*result)->ExecutableCode[m].Operand1=Operand1;
    (*result)->ExecutableCode[m].Operand2=Operand2;
    (*result)->nStatements=(*result)->nStatements+1;

    switch(Opcode)
     {
      case ECLoadIdentifierValue:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECLoadRealConstantValue;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       if(!strcmp(variable,(*result)->symbolList[Operand1]))
        {
         (*result)->ExecutableCode[m].Operand1=one;
        }else{
         (*result)->ExecutableCode[m].Operand1=zero;
        }
       (*result)->nStatements++;
       break;

      case ECLoadRealConstantValue:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECLoadRealConstantValue;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=zero;
       (*result)->nStatements++;
       break;

      case ECLoadIntegerConstantValue:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECLoadRealConstantValue;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=zero;
       (*result)->nStatements++;
       break;

      case ECNegate:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECNegate;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=Operand1+n;
       (*result)->nStatements++;
       break;

      case ECAdd:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECAdd;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=Operand1+n;
       (*result)->ExecutableCode[m].Operand2=Operand2+n;
       (*result)->nStatements++;
       break;

      case ECSubtract:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECSubtract;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=Operand1+n;
       (*result)->ExecutableCode[m].Operand2=Operand2+n;
       (*result)->nStatements++;
       break;

      case ECMultiply:     /* dD=(dO1*O2)+(O1*dO2) */
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       r1=newRegister;
       (*result)->ExecutableCode[m].Opcode=ECMultiply;
       (*result)->ExecutableCode[m].Destination=r1;
       (*result)->ExecutableCode[m].Operand1=Operand1;
       (*result)->ExecutableCode[m].Operand2=Operand2+n;
       (*result)->nStatements++;
       newRegister++;

       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECMultiply;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=Operand1+n;
       (*result)->ExecutableCode[m].Operand2=Operand2;
       (*result)->nStatements++;

       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECAdd;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=r1;
       (*result)->ExecutableCode[m].Operand2=Destination+n;
       (*result)->nStatements++;
       break;

      case ECDivide:     /* dD=(dO1/O2)-(O1/O2)*(dO2/O2) */
       r1=newRegister;
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECDivide;
       (*result)->ExecutableCode[m].Destination=r1;
       (*result)->ExecutableCode[m].Operand1=Operand1+n;
       (*result)->ExecutableCode[m].Operand2=Operand2;
       (*result)->nStatements++;
       newRegister++;

       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECDivide;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=Operand2+n;
       (*result)->ExecutableCode[m].Operand2=Operand2;
       (*result)->nStatements++;

       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECMultiply;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=Destination;
       (*result)->ExecutableCode[m].Operand2=Destination+n;
       (*result)->nStatements++;

       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECSubtract;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=r1;
       (*result)->ExecutableCode[m].Operand2=Destination+n;
       (*result)->nStatements++;

       break;

      case ECPower:   /* dD=dO2*ln(O1)*O1**O2+O2*dO1*O1**(O2-1)        */
/*                              |r1  |                                 */
/*                          |   r2   |                                 */
/*                                     |Dest|                          */
/*                          |       r3      |                          */
/*                                            | r4 |                   */
/*                                                           r5        */
/*                                                       | r6 |        */
/*                                                   |   r7   |        */
/*                                            |     r8        |        */
/*                          |             r9                  |        */

/* Allocate new Registers */

       r1=newRegister;
       r2=newRegister+1;
       r3=newRegister+2;
       r4=newRegister+3;
       r5=newRegister+4;
       r6=newRegister+5;
       r7=newRegister+6;
       r8=newRegister+7;
       newRegister+=8;

/* Allocate new Statements */

       s1=(*result)->nStatements;
       s2=(*result)->nStatements+1;
       s3=(*result)->nStatements+2;
       s4=(*result)->nStatements+3;
       s5=(*result)->nStatements+4;
       s6=(*result)->nStatements+5;
       s7=(*result)->nStatements+6;
       s8=(*result)->nStatements+7;
       s9=(*result)->nStatements+8;
       (*result)->nStatements+=9;

       if(s9>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,s9+1);
         exit(8);
        }

/* r1=ln(O1) */

       (*result)->ExecutableCode[s1].Opcode=ECCallWithOneArgument;
       (*result)->ExecutableCode[s1].Destination=r1;
       (*result)->ExecutableCode[s1].Operand1=ilog;
       (*result)->ExecutableCode[s1].Operand2=Operand1;

/* r2=dO2*r1 */

       (*result)->ExecutableCode[s2].Opcode=ECMultiply;
       (*result)->ExecutableCode[s2].Destination=r2;
       (*result)->ExecutableCode[s2].Operand1=Operand2+n;
       (*result)->ExecutableCode[s2].Operand2=r1;

/* r3=r2*Dest */

       (*result)->ExecutableCode[s3].Opcode=ECMultiply;
       (*result)->ExecutableCode[s3].Destination=r3;
       (*result)->ExecutableCode[s3].Operand1=r2;
       (*result)->ExecutableCode[s3].Operand2=Destination;

/* r4=O2*dO1 */

       (*result)->ExecutableCode[s4].Opcode=ECMultiply;
       (*result)->ExecutableCode[s4].Destination=r4;
       (*result)->ExecutableCode[s4].Operand1=Operand2;
       (*result)->ExecutableCode[s4].Operand2=Operand1+n;

/* r5=1 */

       (*result)->ExecutableCode[s5].Opcode=ECLoadRealConstantValue;
       (*result)->ExecutableCode[s5].Destination=r5;
       (*result)->ExecutableCode[s5].Operand1=one;
       (*result)->ExecutableCode[s5].Operand2=one;

/* r6=O2-r5 */

       (*result)->ExecutableCode[s6].Opcode=ECSubtract;
       (*result)->ExecutableCode[s6].Destination=r6;
       (*result)->ExecutableCode[s6].Operand1=Operand2;
       (*result)->ExecutableCode[s6].Operand2=r5;

/* r7=O1**r6 */

       (*result)->ExecutableCode[s7].Opcode=ECPower;
       (*result)->ExecutableCode[s7].Destination=r7;
       (*result)->ExecutableCode[s7].Operand1=Operand1;
       (*result)->ExecutableCode[s7].Operand2=r6;

/* r8=r4*r7 */

       (*result)->ExecutableCode[s8].Opcode=ECMultiply;
       (*result)->ExecutableCode[s8].Destination=r8;
       (*result)->ExecutableCode[s8].Operand1=r4;
       (*result)->ExecutableCode[s8].Operand2=r7;

/* Dest+n=r3*r8 */

       (*result)->ExecutableCode[s9].Opcode=ECAdd;
       (*result)->ExecutableCode[s9].Destination=Destination+n;
       (*result)->ExecutableCode[s9].Operand1=r3;
       (*result)->ExecutableCode[s9].Operand2=r8;
 
       break;

      case ECCallWithNoArguments:
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECLoadRealConstantValue;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=zero;
       (*result)->nStatements++;
       break;

      case ECCallWithOneArgument:
       function=Operand1;

       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       r1=newRegister;
       newRegister+=1;
       (*result)->ExecutableCode[m].Opcode=ECCallWithOneArgument;
       (*result)->ExecutableCode[m].Destination=r1;
       (*result)->ExecutableCode[m].Operand1=derivative[function];
       (*result)->ExecutableCode[m].Operand2=Operand2;
       (*result)->nStatements++;
 
       m=(*result)->nStatements;
       if(m>1024)
        {
         fprintf(stderr,"ECCreateExpressionDerivative: %d. Too many statements in derivative code! n=%d\n",__LINE__,m);
         exit(8);
        }
       (*result)->ExecutableCode[m].Opcode=ECMultiply;
       (*result)->ExecutableCode[m].Destination=Destination+n;
       (*result)->ExecutableCode[m].Operand1=r1;
       (*result)->ExecutableCode[m].Operand2=Operand2+n;
       (*result)->nStatements++;
       break;

      default:
       if(ECPrintErrorMessages)fprintf(stderr," ECCreateExpressionDerivative: Invalid Opcode in object Code\n");
       (*result)->successful=EC_INVALID_OPCODE_IN_DERIVATIVE;
       return((*result)->successful);
     }
   }

/* Now copy the values of identifers that have been set */

  n=ECNumberOfRealIdentifiers(object,&rc);
  if(rc!=EC_NO_ERROR)return(rc);
  if(n>0)
   {
    for(i=0;i<n;i++)
     {
      name=ECGetRealIdentifierName(i,object,&rc);
      if(rc!=EC_NO_ERROR)return(rc);

      realValue=ECGetRealIdentifierValue(i,object,&rc);
      if(rc!=EC_NO_ERROR)return(rc);

      rc=ECSetIdentifierToReal(name,realValue,*result);
      if(rc!=EC_NO_ERROR)return(rc);
     }
   }

  n=ECNumberOfIntegerIdentifiers(object,&rc);
  if(rc!=EC_NO_ERROR)return(rc);
  if(n>0)
   {
    for(i=0;i<n;i++)
     {
      name=ECGetIntegerIdentifierName(i,object,&rc);
       if(rc!=EC_NO_ERROR)return(rc);

      integerValue=ECGetIntegerIdentifierValue(i,object,&rc);
       if(rc!=EC_NO_ERROR)return(rc);

      rc=ECSetIdentifierToInteger(name,integerValue,*result);
       if(rc!=EC_NO_ERROR)return(rc);
     }
   }

  n=ECNumberOfFunctionIdentifiers(object,&rc);
  if(rc!=EC_NO_ERROR)return(rc);
  if(n>0)
   {
    for(i=0;i<n;i++)
     {
      name=ECGetFunctionIdentifierName(i,object,&rc);
      if(rc!=EC_NO_ERROR)return(rc);

      functionValue=ECGetFunctionIdentifierValue(i,object,&rc);
      if(rc!=EC_NO_ERROR)return(rc);

      rc=ECSetIdentifierToFunction(name,functionValue,*result);
      if(rc!=EC_NO_ERROR)return(rc);
     }
   }

  (*result)->successful=EC_NO_ERROR;
  return((*result)->successful);
 }
#ifdef __cplusplus
}
#endif
