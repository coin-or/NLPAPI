/*
    @(#)ECMsg.h	1.3
    02/04/19 14:44:10

    PROGRAM NAME:  Manifold
   
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2001.  ALL RIGHTS RESERVED.
   
    Please refer to the LICENSE file in the top directory

        author: Mike Henderson mhender@watson.ibm.com 
*/

#ifndef __ECMessagesH__
#define __ECMessagesH__

#define EC_NO_ERROR			0	/* No Error */
#define EC_INVALID_OPCODE_IN_DERIVATIVE	1	/* ECCreateExpressionDerivative: Invalid opcode */
#define EC_INVALID_EXPRESSION		2	/* ECParseTokenizedCode: Expression is invalid */
#define EC_EXTRA_CHARACTERS		3	/* ECParseTokenizedCode: Extra characters following valid expression */
#define EC_LONG_SOURCE			4	/* ECTokenizeExpression: sourceCode is longer than 256 characters */
#define EC_NULL_SOURCE			5	/* ECTokenizeExpression: sourceCode is NULL */
#define EC_TOO_MANY_TOKENS			6	/* ECTokenizeExpression: sourceCode contains more than 256 tokens */
#define EC_INVALID_CHARACTER		7	/* ECTokenizeExpression: Invalid character in source. */
#define EC_TOO_MANY_IDENTIFIERS		8	/* ECTokenizeExpression: sourceCode contains more than 256 Identifiers */
#define EC_TOO_MANY_INTEGERS		9	/* ECTokenizeExpression: sourceCode contains more than 256 integer constants */
#define EC_TOO_MANY_REALS			10	/* ECTokenizeExpression: sourceCode contains more than 256 real constants */
#define EC_BAD_CONSTANT_TYPE		11	/* ECTokenizeExpression: Bad value for ConstantType */
#define EC_IDENTIFIER_NOT_FOUND		12	/* ECSetIdentifierToFloat: Identifier not found */
#define EC_IDENTIFIERS_NOT_SET		13	/* ECEvaluateExpression: Some Identifier not set */
#define EC_INVALID_OPCODE			14	/* ECEvaluateExpression: Bad opcode in object code */
#define EC_NO_STATEMENTS			15	/* ECEvaluateExpression: No Statements in object code */
#define EC_IDENTIFIER_NOT_FUNCTION		16	/* ECSetIdentifierToFunction: Identifier not a Function */
#define EC_INVALID_ASSIGNMENT		17	/* setIdentifier: Invalid assignment string */
#define EC_NULL_OBJECT_CODE		18	/*  objectCode is NULL */
#define EC_INVALID_CONSTANT_TYPE		19	/*  Invalid constant type */
#define EC_INVALID_IDENTIFIER_TYPE		20	/*  Invalid Identifer type */
#endif
