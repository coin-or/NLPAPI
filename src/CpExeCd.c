/*  @(#)CpExeCd.c	1.3
    02/04/19 14:53:08

    PROGRAM NAME:  Manifold

    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory

    author: mhender@watson.ibm.com               */

#include <ExpCmp.h>

#ifdef __cplusplus
extern "C" {
#endif
struct ECExecutableCode ECCopyExecutableCode(struct ECExecutableCode eCode)
 {
  struct ECExecutableCode result={0,ECInvalidOpCode,0,0};

  result.Destination=eCode.Destination;
  result.Opcode=eCode.Opcode;
  result.Operand1=eCode.Operand1;
  result.Operand2=eCode.Operand2;

  return(result);
 }
#ifdef __cplusplus
 }
#endif
