/*                                                          */
/* @(#)EDtanh.c	1.2                   */
/* 02/04/19 16:24:12               */
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
double EDtanh(double x)
 {
  double y=tanh(x);
  return(1.-y*y);
 }
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
double EDDtanh(double x)
 {
  double y=tanh(x);
  double dy=1-y*y;
  return(-2*y*dy);
 }
#ifdef __cplusplus
}
#endif
