/*                                                          */
/* @(#)EDlog10.c	1.2                   */
/* 03/02/21 09:18:00               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#ifdef __cplusplus
extern "C" {
#endif
double EDlog10(double x)
 {
  return(.434294/x);
 }
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
double EDDlog10(double x)
 {
  return(-.434294/x/x);
 }
#ifdef __cplusplus
}
#endif
