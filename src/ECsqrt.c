/*                                                          */
/* @(#)ECsqrt.c	1.3                   */
/* 03/02/21 09:21:47               */
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
double ECsqrt(double x)
 {
  return(sqrt(x));
 }
#ifdef __cplusplus
}
#endif
