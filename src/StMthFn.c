/*                                                          */
/* @(#)StMthFn.c	1.2                   */
/* 02/04/19 16:36:10               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory */

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

/* April 23, 1996 Turned off error messages when functions not present */

#include <ExpCmp.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
ECSetStandardMathFunctions(struct ECObjectCode *object)
 {
  extern double ECsin(double);
  extern double ECsinh(double);
  extern double ECasin(double);
  extern double ECcos(double);
  extern double ECcosh(double);
  extern double ECacos(double);
  extern double ECtan(double);
  extern double ECtanh(double);
  extern double ECatan(double);
  extern double ECsqrt(double);
  extern double ECfabs(double);
  extern double ECexp(double);
  extern double EClog(double);
  extern double EClog10(double);

  extern double EDasin(double);
  extern double EDcos(double);
  extern double EDacos(double);
  extern double EDtan(double);
  extern double EDtanh(double);
  extern double EDatan(double);
  extern double EDsqrt(double);
  extern double EDfabs(double);
  extern double EDlog(double);
  extern double EDlog10(double);

  extern double EDDasin(double);
  extern double EDDcos(double);
  extern double EDDacos(double);
  extern double EDDtan(double);
  extern double EDDtanh(double);
  extern double EDDatan(double);
  extern double EDDsqrt(double);
  extern double EDDfabs(double);
  extern double EDDlog(double);
  extern double EDDlog10(double);

  int    result;
  int    flag;

  if(object==NULL)return(EC_NULL_OBJECT_CODE);
  flag=ECGetMessagePrint();

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("sin",ECsin,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("asin",ECasin,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("sinh",ECsinh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("cos",ECcos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("acos",ECacos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("cosh",ECcosh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("tan",ECtan,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("atan",ECatan,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("tanh",ECtanh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("sqrt",ECsqrt,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("abs",ECfabs,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("exp",ECexp,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("ln",EClog,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("log",EClog,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("log10",EClog10,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

/* Now the derivatives of these */

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dsin",ECcos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dasin",EDasin,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dsinh",ECcosh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dcos",EDcos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dacos",EDacos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dcosh",ECsinh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dtan",EDtan,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Datan",EDatan,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dtanh",EDtanh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dsqrt",EDsqrt,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dabs",EDfabs,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dexp",ECexp,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dln",EDlog,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dlog",EDlog,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dlog10",EDlog10,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);


/* And the second derivatives of these */

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDsin",EDcos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDasin",EDDasin,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDsinh",ECsinh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDcos",EDDcos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDacos",EDDacos,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDcosh",ECcosh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDtan",EDDtan,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDatan",EDDatan,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDtanh",EDDtanh,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDsqrt",EDDsqrt,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDabs",EDDfabs,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDexp",ECexp,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDln",EDDlog,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("DDlog",EDDlog,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);
  ECSetMessagePrint(0);
  result=ECSetIdentifierToFunction("Dlog10",EDlog10,object);
  ECSetMessagePrint(flag);
  if(result!=EC_NO_ERROR && result!=EC_IDENTIFIER_NOT_FOUND)return(result);

  return(EC_NO_ERROR);
 }
#ifdef __cplusplus
 }
#endif
