/*                                                          */
/* @(#)ECfabs.c	1.2                   */
/* 02/04/19 16:21:42               */
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
double ECfabs(double x)
 {
  return(fabs(x));
 }
#ifdef __cplusplus
}
#endif
