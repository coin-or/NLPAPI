/*                                                          */
/* @(#)GtErMsg.c	1.2                   */
/* 02/04/19 16:24:49               */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0             */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.   */
 
/*  Please refer to the LICENSE file in the top directory*/

/*  Author: Michael E. Henderson mhender@watson.ibm.com               */

#include <ECMsg.h>

#ifdef __cplusplus
extern "C" {
#endif
char *ECGetErrorMessage(int rc)
 {
  static char *no_error={"No error"};
  static char *invalid_opcode_in_derivative={"Invalid opcode in derivative object code"};
  static char *invalid_expression={"Expression is invalid"};
  static char *extra_characters={"Extra characters following a valid expression"};
  static char *long_source={"Source code is longer than 256 characters"};
  static char *null_source={"Source code is empty"};
  static char *too_many_tokens={"Source code contains more than 256 tokens"};
  static char *invalid_character={"Source code contains an illegal character"};
  static char *too_many_identifiers={"Source code contains more than 256 identifiers"};
  static char *too_many_integers={"Source code contains more than 256 integer constants"};
  static char *too_many_reals={"Source code contains more than 256 real constants"};
  static char *bad_constant_type={"Bad constant type"};
  static char *identifier_not_found={"Specified identifier is not in symbol table"};
  static char *identifiers_not_set={"There are Undefined identifiers in the symbol table"};
  static char *invalid_opcode={"Invalid opcode in object code"};
  static char *no_statements={"Object code contains no executable"};
  static char *identifier_not_function={"Identifier is not a function"};
  static char *invalid_assignment={"Assignment does not contain an equals sign"};
  static char *null_object_code={"Object code has been deleted, or is the result of a failed compilation"};
  static char *unknown={"Unknown return code"};

  switch(rc)
   {

    case EC_NO_ERROR:
     return(no_error);

    case EC_INVALID_OPCODE_IN_DERIVATIVE:
     return(invalid_opcode_in_derivative);

    case EC_INVALID_EXPRESSION:
     return(invalid_expression);

    case EC_EXTRA_CHARACTERS:
     return(extra_characters);

    case EC_LONG_SOURCE:
     return(long_source);

    case EC_NULL_SOURCE:
     return(null_source);

    case EC_TOO_MANY_TOKENS:
     return(too_many_tokens);

    case EC_INVALID_CHARACTER:
     return(invalid_character);

    case EC_TOO_MANY_IDENTIFIERS:
     return(too_many_identifiers);

    case EC_TOO_MANY_INTEGERS:
     return(too_many_integers);

    case EC_TOO_MANY_REALS:
     return(too_many_reals);

    case EC_BAD_CONSTANT_TYPE:
     return(bad_constant_type);

    case EC_IDENTIFIER_NOT_FOUND:
     return(identifier_not_found);

    case EC_IDENTIFIERS_NOT_SET:
     return(identifiers_not_set);

    case EC_INVALID_OPCODE:
     return(invalid_opcode);

    case EC_NO_STATEMENTS:
     return(no_statements);

    case EC_IDENTIFIER_NOT_FUNCTION:
     return(identifier_not_function);

    case EC_INVALID_ASSIGNMENT:
     return(invalid_assignment);

    case EC_NULL_OBJECT_CODE:
     return(null_object_code);

    default:
     return(unknown);
   }
 }
#ifdef __cplusplus
 }
#endif
