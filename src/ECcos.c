/*                                                          */
/* %W%                   */
/* %D% %T%               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
double ECcos(double x)
 {
  double y;
  double r;

  y=x;
  r=cos(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECexp(double x)
 {
  double y;
  double r;

  y=x;
  r=exp(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double EClog(double x)
 {
  double y;
  double r;

  y=fabs(x);
  if(y<1.e-70)return log(1.e-70);
  r=log(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double EClog10(double x)
 {
  double y;
  double r;

  y=x;
  y=fabs(x);
  r=log10(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECsinh(double x)
 {
  double y;
  double r;

  y=x;
  r=sinh(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECtan(double x)
 {
  double y;
  double r;

  y=x;
  r=tan(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECatan(double x)
 {
  double y;
  double r;

  y=x;
  r=atan(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECtanh(double x)
 {
  double y;
  double r;

  y=x;
  r=tanh(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECcosh(double x)
 {
  double y;
  double r;

  y=x;
  r=cosh(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECasin(double x)
 {
  double y;
  double r;

  y=x;
  r=asin(y);
  return(r);
 }

#ifdef __cplusplus
extern "C"
#endif
double ECacos(double x)
 {
  double y;
  double r;

  y=x;
  r=acos(y);
  return(r);
 }
#ifdef __cplusplus
}
#endif
