/*                                                          */
/* @(#)EDfabs.c	1.2                   */
/* 02/04/19 16:23:27               */
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
double EDfabs(double x)
 {
  if(x>0.)
   {
    return(1.);
   }else{
    if(x<0.)
     {
      return(-1.);
     }else{
      return(0.);
     }
   }
 }
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
double EDDfabs(double x)
 {
  return(0.);
 }
#ifdef __cplusplus
}
#endif
