/*                                                          */
/* @(#)EDtan.c	1.2                   */
/* 02/04/19 16:24:01               */
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
double EDtan(double x)
 {
  double y=1./cos(x);
  return(y*y);
 }
#ifdef __cplusplus
 }
#endif

#ifdef __cplusplus
extern "C" {
#endif
double EDDtan(double x)
 {
  double c=1./cos(x);
  double y=-2*sin(x)*c*c*c;
  return(y*y);
 }
#ifdef __cplusplus
 }
#endif
