/*
    @(#)ExpCmp.h	1.4
    02/04/19 14:44:20
   
    PROGRAM NAME:  Manifold
   
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.
   
    Please refer to the LICENSE file in the top directory

        author: Mike Henderson mhender@watson.ibm.com 
*/

#ifndef __ECExpressionCompilerH__
#define __ECExpressionCompilerH__

/* Comment out the following line if your fortran compiler does not */
/* Add an underscore to external names */

/* #define ECFORTRANUNDERSCORE */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef double (*ECPointerToFunction)(double);

typedef enum {	ECUndefined	=0,
		ECComma		=1,
		ECLeftParen	=2,
		ECRightParen	=3,
		ECPlus		=4,
		ECMinus		=5,
		ECStar		=6,
		ECSlash		=7,
		ECExponentiate	=8,
		ECIdentifier	=9,
		ECIntegerConstant=10,
		ECRealConstant	=11,
		ECAtSign	=12 } ECTokentype;

typedef enum {	ECLookingForToken		=0,
		ECLookingForMultiplyExponentiate=1,
		ECLookingForIdentifier		=2,
		ECLookingForConstant		=3 } ECState;

typedef enum {	ECInvalidOpCode      		=0,
              	ECLoadIdentifierValue		=1,
		ECLoadRealConstantValue		=2,
		ECLoadIntegerConstantValue	=3,
		ECNegate			=4,
		ECAdd				=5,
		ECSubtract			=6,
		ECMultiply			=7,
		ECDivide			=8,
		ECPower				=9,
		ECCallWithNoArguments		=10,
		ECCallWithOneArgument		=11,
		ECCompose			=12
             } ECOpcode;

typedef enum {	ECUnset  	=0,
              	ECInteger	=1,
		ECReal	        =2,
		ECFunction	=3,
		ECNAObject	=4 } ECConstanttype;

struct ECTokenizedCode {
	 	 int successful;
	 	 int nToken;
	 	 ECTokentype tokenType[4096];
	 	 int tokenTable[4096];
	 	 int nSymbols;
	 	 char *symbolList[4096];
	 	 int nIntegerConstants;
	 	 int integerConstants[4096];
	 	 int nRealConstants;
	 	 double realConstants[4096];
		};

struct ECExecutableCode {
	 	 int Destination;
	 	 ECOpcode Opcode;
	 	 int Operand1;
	 	 int Operand2;
		};

struct ECObjectCode {
	 	 int successful;
	 	 int returnRegister;
                 int nRegisters;
                 int nStatements;
	 	 struct ECExecutableCode *ExecutableCode;
	 	 int nSymbols;
	 	 char *symbolList[4096];
	 	 ECConstanttype symbolType[4096];
	 	 int symbolValue[4096];
	 	 int nIntegerConstants;
	 	 int integerConstants[4096];
	 	 int nRealConstants;
	 	 double realConstants[4096];
	 	 int nFunctions;
		 ECPointerToFunction functions[4096];
	 	 int nNAObjects;
	 	 void *NAObjects[4096];
		};

#define ECBLANK ' '
#define ECCOMMA ','
#define ECLPAREN '('
#define ECRPAREN ')'
#define ECPLUS '+'
#define ECMINUS '-'
#define ECSTAR '*'
#define ECATSIGN '@'
#define ECSLASH '/'
#define ECDECIMALPOINT '.'
#ifndef NULL
#define NULL 0x0
#endif

#ifdef __cplusplus
extern "C" {
#endif
void   ECFreeObjectCode(struct ECObjectCode **);
void   ECPrintObjectCode(struct ECObjectCode *object);
void   ECPrintSymbolTable(struct ECObjectCode *object);
int    ECSetIdentifierToReal(char *Identifier, double Value, struct ECObjectCode *object);
int    ECSetIdentifierToInteger(char *Identifier, int Value, struct ECObjectCode *object);
int    ECSetIdentifierToFunction(char *Identifier, ECPointerToFunction Value, struct ECObjectCode *object);
int    ECSetIdentifierToNAObject(char *Identifier, void *Value, struct ECObjectCode *object);
int    ECSetIdentifier(char *Assignment, struct ECObjectCode *object);
double ECEvaluateExpression(struct ECObjectCode *object,int *ierr);
int    ECCompileExpression(char *sourceCode,struct ECObjectCode **object);
int    ECCreateExpressionDerivative(struct ECObjectCode *exp, char *variable,struct ECObjectCode **object);

int    ECNumberOfIdentifiers(struct ECObjectCode*,int *);
char  *ECGetIdentifierName(int,struct ECObjectCode*,int *);
char  *ECGetIdentifierType(int,struct ECObjectCode*,int *);
int    ECIsIdentifierSet(int,struct ECObjectCode*,int *);

int    ECNumberOfUnsetIdentifiers(struct ECObjectCode*,int *);
char  *ECGetUnsetIdentifierName(int i,struct ECObjectCode *object,int *);

int    ECNumberOfRealIdentifiers(struct ECObjectCode*,int *);
double ECGetRealIdentifierValue(int i,struct ECObjectCode *object,int *);
char  *ECGetRealIdentifierName(int i,struct ECObjectCode *object,int *);

int    ECNumberOfIntegerIdentifiers(struct ECObjectCode*,int *);
char  *ECGetIntegerIdentifierName(int i,struct ECObjectCode *object,int *);
int    ECGetIntegerIdentifierValue(int i,struct ECObjectCode *object,int *);

int    ECNumberOfNAObjectIdentifiers(struct ECObjectCode*,int *);
char  *ECGetNAObjectIdentifierName(int i,struct ECObjectCode *object,int *);
void  *ECGetNAObjectIdentifierValue(int i,struct ECObjectCode *object,int *);

int    ECNumberOfFunctionIdentifiers(struct ECObjectCode*,int *);
char  *ECGetFunctionIdentifierName(int i,struct ECObjectCode *object,int *);
ECPointerToFunction ECGetFunctionIdentifierValue(int i,struct ECObjectCode *object,int *);
char  *ECGetErrorMessage(int rc);
void ECSetMessagePrint(int);
int  ECGetMessagePrint(void);

extern int ECPrintErrorMessages;

int ECSetStandardMathConstants(struct ECObjectCode *);
int ECSetStandardMathFunctions(struct ECObjectCode *);

struct ECFunctionSt;
typedef struct ECFunctionSt *ECFn;
ECFn ECCreateFunction(char*,char*);
ECFn ECCreateDerivativeOfFunction(ECFn,int);
void ECEvaluateFunction(ECFn,double*,double*);
void ECFreeFunction(ECFn);
int ECFunctionN(ECFn);
int ECFunctionM(ECFn);

#ifdef __cplusplus
 }
#endif

#include <stdio.h> 

#include <ECMsg.h> 
#endif
