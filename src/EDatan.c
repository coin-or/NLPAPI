/*                                                          */
/* @(#)EDatan.c	1.2                   */
/* 02/04/19 16:23:12               */
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
double EDatan(double x)
 {
  return(1./sqrt(1.+x*x));
 }
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
double EDDatan(double x)
 {
  return(x/sqrt(1.+x*x)/(1.-x*x));
 }
#ifdef __cplusplus
}
#endif
