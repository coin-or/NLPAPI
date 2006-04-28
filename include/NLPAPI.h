/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: @(#)NLPAPI.h	3.24 03/01/07 11:35:26 */
/*      date:   November 11, 1997                     */
/*              February 2, 1999   Ported to C        */
/*              September 14, 2000 Added NLVIncrementC*/

#ifndef __NLPAPI_H__
#define __NLPAPI_H__

#include <NLPAPIConfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef DBL_QNAN
#define DBL_QNAN (sqrt(-1.L))
#endif

struct NLGrpPart;
struct NLGrpPartVec;
struct NLGrpPartMat;
struct NLGrpPartGFn;
struct NLGrpPartEFn;
struct NLLancelotSolver;
struct NLIpoptSolver;

typedef struct NLGrpPart *NLProblem;
typedef struct NLGrpPartVec *NLVector;
typedef struct NLGrpPartMat *NLMatrix;
typedef struct NLGrpPartGFn *NLGroupFunction;
typedef struct NLGrpPartEFn *NLElementFunction;
typedef int NLNonlinearElement;
typedef struct NLGrpPartNEl *NLNonlinearElementPtr;
typedef struct NLLancelotSolver *NLLancelot;
typedef struct NLIpoptSolver *NLIpopt;

#ifdef __cplusplus
 extern "C" {
#endif

NLProblem NLCreateProblem(char*,int);
void NLFreeProblem(NLProblem);

char *NLPGetProblemName(NLProblem);
int NLPGetNumberOfVariables(NLProblem);
int NLPSetVariableScale(NLProblem,int,double);
double NLPGetVariableScale(NLProblem,int);
int NLPSetVariableName(NLProblem,int,char*);
char *NLPGetVariableName(NLProblem,int);

/* Group Information */

int NLPGetNumberOfGroups(NLProblem);

/* Per Group Info. */

int NLPGetTypeOfGroup(NLProblem,int);
char *NLPGetGroupTypeName(NLProblem,int);
char *NLPGetGroupName(NLProblem,int);

int NLPSetGroupFunction(NLProblem,int,NLGroupFunction);
int NLPSetGroupFunctionParm(NLProblem,int,NLGroupFunction,void*,void (*)(void*));
int NLPSetObjectiveGroupFunction(NLProblem,int,NLGroupFunction);

NLGroupFunction NLPGetGroupFunction(NLProblem,int);
int NLPIsGroupFunctionSet(NLProblem,int);

int NLPSetGroupA(NLProblem,int,NLVector);
int NLPSetObjectiveGroupA(NLProblem,int,NLVector);
int NLPSetEqualityConstraintA(NLProblem,int,NLVector);
int NLPSetInequalityConstraintA(NLProblem,int,NLVector);
int NLPSetMinMaxConstraintA(NLProblem,int,NLVector);
int NLPIsGroupASet(NLProblem,int);
NLVector NLPGetGroupA(NLProblem,int);

int NLPSetGroupB(NLProblem,int,double);
int NLPSetObjectiveGroupB(NLProblem,int,double);
int NLPSetEqualityConstraintB(NLProblem,int,double);
int NLPSetInequalityConstraintB(NLProblem,int,double);
int NLPSetMinMaxConstraintB(NLProblem,int,double);
int NLPIsGroupBSet(NLProblem,int);
double NLPGetGroupB(NLProblem,int);

int NLPSetObjectiveGroupScale(NLProblem,int,double);
int NLPSetEqualityConstraintScale(NLProblem,int,double);
int NLPSetInequalityConstraintScale(NLProblem,int,double);
int NLPSetMinMaxConstraintScale(NLProblem,int,double);

int NLPGetNumberOfElementsInGroup(NLProblem,int);
double NLPGetElementWeight(NLProblem,int,int);
NLNonlinearElement NLPGetGroupNonlinearElement(NLProblem,int,int);
int NLPSetElementWeight(NLProblem,int,int,double);
int NLPIsElementWeightSet(NLProblem,int,int);
NLElementFunction NLPGetElementFunctionOfGroup(NLProblem,int,int);
NLElementFunction NLPGetElementFunction(NLProblem,int);
int NLPSetElementFunction(NLProblem,int,int,NLElementFunction,int*);
int NLPSetElementFunctionWithRange(NLProblem,int,int,NLElementFunction,int*,NLMatrix);
int NLPIsElementFunctionSet(NLProblem,int,int);
int NLPSetElementTypeName(NLProblem,int,int);
int NLPGetTypeOfElement(NLProblem,int,int);
int NLPGetElementNumberOfUnknowns(NLProblem,int,int);
int NLPGetNumberOfInternalVariablesInElement(NLProblem,int,int);
int NLPGetElementIndexIntoWhole(NLProblem,int,int,int);
int NLPSetGroupScale(NLProblem,int,double);
double NLPGetGroupScale(NLProblem,int);
int NLPSetInvGroupScale(NLProblem,int,double);
double NLPGetInvGroupScale(NLProblem,int);

NLMatrix NLPGetElementRangeTransformationOfGroup(NLProblem,int,int);
NLMatrix NLPGetElementRangeTransformation(NLProblem,int);

/* Element Information */

char *NLPGetElementTypeName(NLProblem,int,int);
int NLPGetNumberOfElements(NLProblem);
int NLPGetNumberOfElementsO(NLProblem);
int NLPGetNumberOfElementsE(NLProblem);
int NLPGetNumberOfElementsI(NLProblem);

/* Objective Info. */

int NLPGetNumberOfGroupsInObjective(NLProblem);
int NLPGetObjectiveGroupNumber(NLProblem,int);
/* int NLPSetObjectiveLowerBound(NLProblem,double); */
/* double NLPGetObjectiveLowerBound(NLProblem); */
/* int NLPSetObjectiveUpperBound(NLProblem,double); */
/* double NLPGetObjectiveUpperBound(NLProblem); */
/* int NLPSetObjectiveBounds(NLProblem,double,double); */

double NLPEvaluateObjective(NLProblem,NLVector);
int NLPEvaluateGradientOfObjective(NLProblem,NLVector,NLVector);
int NLPEvaluateHessianOfObjective(NLProblem,NLVector,NLMatrix);

/* Constraint Info. */


/*   Equality Constraints. */

double NLPEvaluateEqualityConstraint(NLProblem,int,NLVector);
int NLPEvaluateGradientOfEqualityConstraint(NLProblem,int,NLVector,NLVector);
int NLPEvaluateHessianOfEqualityConstraint(NLProblem,int,NLVector,NLMatrix);
int NLPAddNonlinearEqualityConstraint(NLProblem,char*);
int NLPAddLinearEqualityConstraint(NLProblem,char*,double*,double); /* 9/27/99 */
int NLPAddLinearEqualityConstraintV(NLProblem,char*,NLVector,double); /* 6/18/00 */
int NLPGetNumberOfEqualityConstraints(NLProblem);
int NLPGetEqualityConstraintGroupNumber(NLProblem,int);
int NLPGetEqualityConstraintNumberOfGroup(NLProblem,int,int);
int NLPGetNumberOfEqualityConstraintGroups(NLProblem,int);
int NLPSetEqualityConstraintGroupFunction(NLProblem,int,int,NLGroupFunction);
int NLPSetEqualityConstraintGroupScale(NLProblem,int,int,double);
double NLPGetEqualityConstraintGroupScale(NLProblem,int,int);
double NLPGetEqualityConstraintInvGroupScale(NLProblem,int,int);
int NLPAddNonlinearElementToEqualityConstraintGroup(NLProblem,int,int,double,NLNonlinearElement);
int NLPAddNonlinearElementToEqualityConstraint(NLProblem P,int constraint,double w,NLNonlinearElement E);

/*   Inequality Constraints. */

double NLPEvaluateInequalityConstraint(NLProblem,int,NLVector);
int NLPEvaluateGradientOfInequalityConstraint(NLProblem,int,NLVector,NLVector);
int NLPEvaluateHessianOfInequalityConstraint(NLProblem,int,NLVector,NLMatrix);
int NLPGetNumberOfInequalityConstraints(NLProblem);
int NLPAddNonlinearInequalityConstraint(NLProblem,char*);
int NLPAddLinearInequalityConstraint(NLProblem,char*,double*,double); /* 6/18/00 */
int NLPAddLinearInequalityConstraintV(NLProblem,char*,NLVector,double); /* 6/18/00 */
int NLPGetInequalityConstraintGroupNumber(NLProblem,int);
int NLPGetInequalityConstraintNumberOfGroup(NLProblem,int,int);
double NLPGetInequalityConstraintLowerBound(NLProblem,int);
double NLPGetInequalityConstraintUpperBound(NLProblem,int);
int NLPIsInequalityConstraintLowerBoundSet(NLProblem,int);
int NLPIsInequalityConstraintUpperBoundSet(NLProblem,int);
int NLPSetInequalityConstraintLowerBound(NLProblem,int,double);
int NLPSetInequalityConstraintUpperBound(NLProblem,int,double);
int NLPSetInequalityConstraintBounds(NLProblem,int,double,double);
int NLPGetNumberOfInequalityConstraintGroups(NLProblem,int);
int NLPSetInequalityConstraintGroupFunction(NLProblem,int,int,NLGroupFunction);
int NLPSetInequalityConstraintGroupScale(NLProblem,int,int,double);
double NLPGetInequalityConstraintGroupScale(NLProblem,int,int);
double NLPGetInequalityConstraintInvGroupScale(NLProblem,int,int);
int NLPAddNonlinearElementToInequalityConstraint(NLProblem P,int constraint,double w,NLNonlinearElement E);
int NLPAddNonlinearElementToInequalityConstraintGroup(NLProblem P,int constraint,int group,double w,NLNonlinearElement E);
int NLPUnSetInequalityConstraintLowerBound(NLProblem,int);
int NLPUnSetInequalityConstraintUpperBound(NLProblem,int);
int NLPUnSetInequalityConstraintBounds(NLProblem,int);

/*   MinMax Constraints 9/27/99 */

double NLPEvaluateMinMaxConstraint(NLProblem,int,NLVector);
int NLPEvaluateGradientOfMinMaxConstraint(NLProblem,int,NLVector,NLVector);
int NLPEvaluateHessianOfMinMaxConstraint(NLProblem,int,NLVector,NLMatrix);
int NLPAddMinMaxConstraint(NLProblem,char*);
int NLPAddLinearMinMaxConstraint(NLProblem,char*,double*,double); /* 6/18/00 */
int NLPAddLinearMinMaxConstraintV(NLProblem,char*,NLVector,double); /* 6/18/00 */
int NLPGetNumberOfMinMaxConstraints(NLProblem);
int NLPGetMinMaxConstraintGroupNumber(NLProblem,int);
int NLPGetMinMaxConstraintNumberOfGroup(NLProblem,int,int);
int NLPGetNumberOfElementsM(NLProblem);
int NLPGetNumberOfMinMaxConstraintGroups(NLProblem,int);
int NLPAddNonlinearElementToMinMaxConstraint(NLProblem P,int constraint,double w,NLNonlinearElement);
int NLPAddNonlinearElementToMinMaxConstraintGroup(NLProblem P,int constraint,int group, double w,NLNonlinearElement);

int NLPSetMinMaxBounds(NLProblem,double,double);
int NLPSetLowerMinMaxBound(NLProblem,double);
double NLPGetLowerMinMaxBound(NLProblem);
int NLPSetUpperMinMaxBound(NLProblem,double);
double NLPGetUpperMinMaxBound(NLProblem);

/*   Simple Bounds */

int NLPSetSimpleBounds(NLProblem,int,double,double);
int NLPSetLowerSimpleBound(NLProblem,int,double);
double NLPGetLowerSimpleBound(NLProblem,int);
int NLPIsLowerSimpleBoundSet(NLProblem,int);
double NLPGetUpperSimpleBound(NLProblem,int);
int NLPIsUpperSimpleBoundSet(NLProblem,int);
int NLPUnSetUpperSimpleBound(NLProblem,int);
int NLPUnSetLowerSimpleBound(NLProblem,int);
int NLPUnSetSimpleBounds(NLProblem,int);

/* Element Types */

int NLPGetNumberOfElementTypes(NLProblem);
char *NLPGetElementType(NLProblem,int);

/* Group Types */

int NLPGetNumberOfGroupTypes(NLProblem);
char *NLPGetGroupType(NLProblem,int);

void NLPrintProblem(FILE*,NLProblem);

/* Vectors */

NLVector NLCreateVector(int);
NLVector NLCreateVectorWithSparseData(int,int,int*,double*);
NLVector NLCreateVectorWithFullData(int,double*);
NLVector NLCreateDenseWrappedVector(int,double*);
NLVector NLCreateDenseVector(int);
void NLFreeVector(NLVector);
int NLVGetNC(NLVector);
double NLVGetC(NLVector,int);
int NLVSetC(NLVector,int,double);
int NLVIncrementC(NLVector,int,double);
void NLRefVector(NLVector);
int NLVGetNumberOfNonZeros(NLVector);
int NLVGetNonZeroCoord(NLVector,int);
double NLVGetNonZero(NLVector,int);
int NLVSetToZero(NLVector);
double NLVInnerProd(NLVector,NLVector);

/* Matrices */

NLMatrix NLCreateMatrix(int,int);
NLMatrix NLCreateSparseMatrix(int,int);
NLMatrix NLCreateMatrixWithData(int,int,double*);
NLMatrix NLCreateDenseWrappedMatrix(int,int,double*);
void NLFreeMatrix(NLMatrix);
int NLMGetNumberOfRows(NLMatrix);
int NLMGetNumberOfCols(NLMatrix);
double NLMGetElement(NLMatrix,int,int);
int NLMSetElement(NLMatrix,int,int,double);
int NLMIncrementElement(NLMatrix,int,int,double);
void NLRefMatrix(NLMatrix);
void NLPrintMatrix(FILE*,NLMatrix);
int NLMSetToZero(NLMatrix);

/* Group Functions */

typedef double (*NLGroupF)(double,void*);
typedef double (*NLGroupDF)(double,void*);
typedef double (*NLGroupDDF)(double,void*);

int NLGGetType(NLGroupFunction);
void NLRefGroupFunction(NLGroupFunction);
void NLFreeGroupFunction(NLGroupFunction);
double NLGEval(NLGroupFunction,double,void*);
double NLGEvalDer(NLGroupFunction,double,void*);
double NLGEvalSecDer(NLGroupFunction,double,void*); 

/* Element Functions */

typedef double (*NLElementF)(int,double*,void*);
typedef double (*NLElementDF)(int,int,double*,void*);
typedef double (*NLElementDDF)(int,int,int,double*,void*);

int NLEGetDimension(NLElementFunction);
void NLRefElementFunction(NLElementFunction);
void NLFreeElementFunction(NLElementFunction);
double NLEEval(NLElementFunction,int,double*,void*);
double NLEEvalDer(NLElementFunction,int,int,double*,void*);
double NLEEvalSecDer(NLElementFunction,int,int,int,double*,void*);

/* Lancelot Solver*/

NLLancelot NLCreateLancelot();
void NLFreeLancelot(NLLancelot);
int LNMinimize(NLLancelot,NLProblem,double*,double*,double*,double*);
int LNMaximize(NLLancelot,NLProblem,double*,double*,double*,double*);
int LNMinimizeDLL(NLLancelot,NLProblem,double*,double*,double*,double*);
int LNMaximizeDLL(NLLancelot,NLProblem,double*,double*,double*,double*);
int LNSetCheckDerivatives(NLLancelot,int);
int LNGetCheckDerivatives(NLLancelot);
int LNSetStopOnBadDerivatives(NLLancelot,int);
int LNGetStopOnBadDerivatives(NLLancelot);
int LNSetPrintLevel(NLLancelot,int);
int LNGetPrintLevel(NLLancelot);
int LNSetPrintStart(NLLancelot,int);
int LNGetPrintStart(NLLancelot);
int LNSetPrintStop(NLLancelot,int);
int LNGetPrintStop(NLLancelot);
int LNSetPrintEvery(NLLancelot,int);
int LNGetPrintEvery(NLLancelot);
int LNSetMaximumNumberOfIterations(NLLancelot,int);
int LNGetMaximumNumberOfIterations(NLLancelot);
int LNSetSaveDataEvery(NLLancelot,int);
int LNGetSaveDataEvery(NLLancelot);
int LNSetUseExactFirstDerivatives(NLLancelot,int);
int LNGetUseExactFirstDerivatives(NLLancelot);
int LNSetUseExactSecondDerivatives(NLLancelot,char*);
char *LNGetUseExactSecondDerivatives(NLLancelot);
int LNSetConstraintAccuracy(NLLancelot,double);
double LNGetConstraintAccuracy(NLLancelot);
int LNSetGradientAccuracy(NLLancelot,double);
double LNGetGradientAccuracy(NLLancelot);
int LNSetInitialPenalty(NLLancelot,double);
double LNGetInitialPenalty(NLLancelot);
int LNSetPenaltyBound(NLLancelot,double);
double LNGetPenaltyBound(NLLancelot);
int LNSetFirstConstraintAccuracy(NLLancelot,double);
double LNGetFirstConstraintAccuracy(NLLancelot);
int LNSetFirstGradientAccuracy(NLLancelot,double);
double LNGetFirstGradientAccuracy(NLLancelot);
int LNSetTrustRegionType(NLLancelot,char*);
char *LNGetTrustRegionType(NLLancelot);
int LNSetTrustRegionRadius(NLLancelot,double);
double LNGetTrustRegionRadius(NLLancelot);
int LNSetSolveBQPAccurately(NLLancelot,int);
int LNGetSolveBQPAccurately(NLLancelot);
int LNSetRequireExactCauchyPoint(NLLancelot,int);
int LNGetRequireExactCauchyPoint(NLLancelot);
int LNSetLinearSolverMethod(NLLancelot,char*,int);
char *LNGetLinearSolverMethod(NLLancelot);
int LNGetLinearSolverBandwidth(NLLancelot);
int LNSetScalings(NLLancelot,char*);
char *LNGetScalings(NLLancelot);
int LNGetJiffyTuneTolerance(NLLancelot,double*);
int LNSetJiffyTuneTolerance(NLLancelot,double);

/* Ipopt Solver */

NLIpopt NLCreateIpopt();
void NLFreeIpopt(NLIpopt);
int IPMinimize(NLIpopt,NLProblem,double*,double*,double*,double*);
int IPMaximize(NLIpopt,NLProblem,double*,double*,double*,double*);
void IPAddOption(NLIpopt,char*,double);

int NLGetNErrors();
int NLGetErrorSev(int);
char *NLGetErrorRoutine(int);
char *NLGetErrorMsg(int);
int NLGetErrorLine(int);
char *NLGetErrorFile(int);
int NLError();
void NLClearErrors();

/* Changes */

int NLPAddGroupToObjective(NLProblem,char*);

NLGroupFunction NLCreateGroupFunction(NLProblem,char*,double (*)(double,void*),double (*)(double,void*),double (*)(double,void*),void*,void (*)(void*));
NLGroupFunction NLCreateGroupFunctionByString(NLProblem,char*,char*,char*);

int NLPAddNonlinearElementToObjectiveGroup(NLProblem P,int group,double w,NLNonlinearElement E);

NLNonlinearElement NLCreateNonlinearElement(NLProblem P,char *type,NLElementFunction fn,int *vars);
NLNonlinearElement NLCreateNonlinearElementParm(NLProblem P,char *type,NLElementFunction fn,int *vars,void*,void (*)(void*));
int NLFreeNonlinearElement(NLProblem P,NLNonlinearElement E);
void NLRefNonlinearElement(NLNonlinearElementPtr E);
NLElementFunction NLNEGetElementFunction(NLProblem P,NLNonlinearElement E);
NLMatrix NNLEGetRangeXForm(NLProblem P,NLNonlinearElement E);
int NLNEGetIndex(NLProblem P,NLNonlinearElement E,int);
int NLNEGetInternalDimension(NLProblem P,NLNonlinearElement E);
int NLNEGetElementDimension(NLProblem P,NLNonlinearElement E);
char *NLNEGetName(NLProblem P,NLNonlinearElement E);
NLElementFunction NLNEPGetElementFunction(NLNonlinearElementPtr E);
NLMatrix NLNEPGetRangeXForm(NLNonlinearElementPtr E);
int NLNEPGetIndex(NLNonlinearElementPtr E,int);
int NLNEPGetInternalDimension(NLNonlinearElementPtr E);
int NLNEPGetElementDimension(NLNonlinearElementPtr E);
char *NLNEPGetName(NLNonlinearElementPtr E);
int NLPIsElementRangeSet(NLProblem,int);

NLElementFunction NLCreateElementFunction(NLProblem P,char *type,int n,NLMatrix R,
double (*f)(int,double*,void*),double (*df)(int,int,double*,void*),double (*ddf)(int,int,int,double*,void*),
void *data,void (*freedata)(void*));
NLElementFunction NLCreateElementFunctionByString(NLProblem P,char *type,int n,NLMatrix R,char*,char*);

NLElementFunction NLCreateElementFunctionWithInitialHessian(NLProblem P,char *type,int n,NLMatrix R,
double (*f)(int,double*,void*),double (*df)(int,int,double*,void*),double (*ddf)(int,int,int,double*,void*),
void *data,void (*freedata)(void*),NLMatrix);

void NLElementFunctionGetInitialHessian(NLElementFunction,double*);
double *NLElementFunctionGetInitialHessianMatrix(NLElementFunction);

/* Add to Problem */

NLNonlinearElementPtr NLPGetNonlinearElement(NLProblem,int);
int NLPAddNonlinearElement(NLProblem,NLNonlinearElementPtr);
int NLPGetNumberOfNonlinearElements(NLProblem);
double NLPEvaluateGroup(NLProblem,int,NLVector);
double NLPEvaluateElement(NLProblem,int,NLVector);

/* Add to Element */

NLMatrix NLEGetRangeXForm(NLElementFunction);
int NLEGetType(NLElementFunction);

void NLMVMult(NLMatrix,double*,double*);
void NLMVMultT(NLMatrix,double*,double*);
int NLPGetZGroupNumber(NLProblem);
void NLPSetZGroupNumber(NLProblem,int);

typedef int F77INTEGER;
typedef float F77REAL;
typedef double F77DOUBLEPRECISION;
typedef int F77LOGICAL;

/* Added for Elaine */

void NLPAddVariables(NLProblem,int);
void NLPRemoveVariables(NLProblem,int);

void NLPHideMinMaxConstraints(NLProblem);
void NLPUnHideMinMaxConstraints(NLProblem);
int  NLPGetNumberOfGroupsInMinMaxConstraint(NLProblem,int);
int NLPAddGroupToMinMaxConstraint(NLProblem,int,char*);
int NLPSetInequalityConstraintGroupA(NLProblem,int,int,NLVector);
int NLPSetInequalityConstraintGroupB(NLProblem,int,int,double);
NLVector NLPGetMinMaxConstraintGroupA(NLProblem,int,int);
double NLPGetMinMaxConstraintGroupB(NLProblem,int,int);
double NLPGetMinMaxConstraintGroupScale(NLProblem,int,int);
double NLPGetMinMaxConstraintInvGroupScale(NLProblem,int,int);
int NLPSetMinMaxConstraintGroupScale(NLProblem,int,int,double);

void NLPHideInequalityConstraints(NLProblem);
void NLPUnHideInequalityConstraints(NLProblem);
int  NLPGetNumberOfGroupsInInequalityConstraint(NLProblem,int);
int NLPAddGroupToInequalityConstraint(NLProblem,int,char*);
int NLPSetInequalityConstraintGroupA(NLProblem,int,int,NLVector);
int NLPSetInequalityConstraintGroupB(NLProblem,int,int,double);
NLVector NLPGetInequalityConstraintGroupA(NLProblem,int,int);
double NLPGetInequalityConstraintGroupB(NLProblem,int,int);

void NLPHideEqualityConstraints(NLProblem);
void NLPUnHideEqualityConstraints(NLProblem);
int  NLPGetNumberOfGroupsInEqualityConstraint(NLProblem,int);
int  NLPAddGroupToEqualityConstraint(NLProblem,int,char*);
int NLPSetEqualityConstraintGroupA(NLProblem,int,int,NLVector);
int NLPSetEqualityConstraintGroupB(NLProblem,int,int,double);
NLVector NLPGetEqualityConstraintGroupA(NLProblem,int,int);
double NLPGetEqualityConstraintGroupB(NLProblem,int,int);

void NLPDeleteMinMaxConstraint(NLProblem,int);
void NLPDeleteEqualityConstraint(NLProblem,int);
void NLPDeleteInequalityConstraint(NLProblem,int);
void NLPDeleteObjectiveGroup(NLProblem,int);
void NLPDeleteMinMaxConstraintGroup(NLProblem,int,int);
void NLPDeleteEqualityConstraintGroup(NLProblem,int,int);
void NLPDeleteInequalityConstraintGroup(NLProblem,int,int);

int NLEFAssertPolynomialOrderOfElementVariable(NLElementFunction,int,int);
int NLEFQueryPolynomialOrderOfElementVariable(NLElementFunction,int);
int NLGFAssertPolynomialOrder(NLGroupFunction,int);
int NLGFQueryPolynomialOrder(NLGroupFunction);
#define NLVARIABLEDEPENDENCENOTSET 999
int NLPQueryPolynomialOrderOfVariablesInObjective(NLProblem,int*);

int NLPInvalidateGroupAndElementCaches(NLProblem);
int *NLPGetCacheFlag(NLProblem);

NLMatrix NLMatrixClone(NLMatrix);
int NLZeroVector(NLVector);
void NLPrintElementValues(NLProblem,FILE*);
void NLPrintProblemShort(FILE*,NLProblem);

void *NLPGetNonlinearElementData(NLProblem,NLNonlinearElement);
void *NLPGetGroupFunctionData(NLProblem,int);
void *NNLEGetData(NLNonlinearElementPtr);

double NLGetMaxScaledDiagonal(NLMatrix,double*);
double NLGetMinScaledDiagonal(NLMatrix,double*);
void NLGetGershgorinBounds(NLMatrix,double*,double*,double*);
double NLMatrixOneNorm(NLMatrix,double*);
double NLMatrixDoubleProduct(NLVector,NLMatrix,NLVector);
void NLPrintProblemOld(FILE*,NLProblem);

extern double NLGetMaxScaledDiagonalTime;
extern int NLGetMaxScaledDiagonalNCalls;
extern double NLGetMinScaledDiagonalTime;
extern int NLGetMinScaledDiagonalNCalls;
extern double NLGetGershgorinBoundsTime;
extern int NLGetGershgorinBoundsNCalls;
extern double NLMatrixOneNormTime;
extern int NLMatrixOneNormNCalls;
extern double NLMatrixDoubleProductTime;
extern int NLMatrixDoubleProductNCalls;

extern double NLDetermineSparsityTime;
extern int NLDetermineSparsityNCalls;
extern double NLSumIntoTime;
extern int NLSumIntoNCalls;

void *NLEGetData(NLElementFunction);

extern double NLEvaluateElementFunctionTime;
extern int NLEvaluateElementFunctionNCalls;
extern double NLEvaluateElementFunctionDerTime;
extern int NLEvaluateElementFunctionDerNCalls;
extern double NLEvaluateElementFunctionSecDerTime;
extern int NLEvaluateElementFunctionSecDerNCalls;
extern double NLEvaluateGroupFunctionTime;
extern int NLEvaluateGroupFunctionNCalls;
extern double NLEvaluateGroupFunctionDerTime;
extern int NLEvaluateGroupFunctionDerNCalls;
extern double NLEvaluateGroupFunctionSecDerTime;
extern int NLEvaluateGroupFunctionSecDerNCalls;

extern double NLEvaluateObjectiveTime;
extern int NLEvaluateObjectiveNCalls;
extern double NLEvaluateObjectiveDerTime;
extern int NLEvaluateObjectiveDerNCalls;
extern double NLEvaluateObjectiveSecDerTime;
extern int NLEvaluateObjectiveSecDerNCalls;

extern double NLEvaluateEqualityConstraintTime;
extern int NLEvaluateEqualityConstraintNCalls;
extern double NLEvaluateEqualityConstraintDerTime;
extern int NLEvaluateEqualityConstraintDerNCalls;
extern double NLEvaluateEqualityConstraintSecDerTime;
extern int NLEvaluateEqualityConstraintSecDerNCalls;

extern double NLEvaluateInequalityConstraintTime;
extern int NLEvaluateInequalityConstraintNCalls;
extern double NLEvaluateInequalityConstraintDerTime;
extern int NLEvaluateInequalityConstraintDerNCalls;
extern double NLEvaluateInequalityConstraintSecDerTime;
extern int NLEvaluateInequalityConstraintSecDerNCalls;

extern double NLEvaluateMinMaxConstraintTime;
extern int NLEvaluateMinMaxConstraintNCalls;
extern double NLEvaluateMinMaxConstraintDerTime;
extern int NLEvaluateMinMaxConstraintDerNCalls;
extern double NLEvaluateMinMaxConstraintSecDerTime;
extern int NLEvaluateMinMaxConstraintSecDerNCalls;

extern double NLEvaluateGroupTime;
extern int NLEvaluateGroupNCalls;
extern double NLEvaluateElementTime;
extern int NLEvaluateElementNCalls;

void NLMSumSubMatrixInto(NLMatrix,double,int,int*,double*);
void NLMSumRankOneInto(NLMatrix,double,double*);
void NLMDetermineHessianSparsityStructure(NLProblem,char,int,NLMatrix);
NLMatrix NLCreateWSMPSparseMatrix(int);
void NLSetDefaultElementUpdateType(char*);
int NLGetDefaultElementUpdateType();
void NLSetElementUpdateType(NLElementFunction,char*,int);
int NLGetElementUpdateType(NLElementFunction);
void NLPSetAllElementUpdateTypes(NLProblem,char*,int);
void NLSetDefaultElementUpdateNoise(double);
double NLGetDefaultElementUpdateNoise();
void NLSetElementUpdateNoise(NLElementFunction,double);
double NLGetElementUpdateNoise(NLElementFunction);
void NLPSetAllElementUpdateNoises(NLProblem,double);

NLVector NLCopyVector(NLVector);
void NLPConvertToEqualityAndBoundsOnly(NLProblem);
NLProblem NLCopyProblem(NLProblem);

extern int NLNumberOfUpdatesSkipped;

void NLMMergeIntoArray(int,int*,int*,int**);
void NLReportTimes();

void NLVPlusV(NLVector,NLVector,double); /* Andreas' fix for speed */

int NLCreateAugmentedLagrangian(NLProblem,double,double*,int*,double*,double*);
int NLSetLambaAndMuInAugmentedLagrangian(NLProblem,int,double,double*,int*,double*,double*);
void NLEliminateFixedVariables(NLProblem);

int NLPAddEqualityConstraint(NLProblem,char*,int,int*,double (*)(int,double*,void*),double (*)(int,int,double*,void*),double (*)(int,int,int,double*,void*),void*,void (*)(void*));
int NLPAddInequalityConstraint(NLProblem,char*,double,double,int,int*,double (*)(int,double*,void*),double (*)(int,int,double*,void*),double (*)(int,int,int,double*,void*),void*,void (*)(void*));
int NLPSetObjective(NLProblem,char*,int,int*,double (*)(int,double*,void*),double (*)(int,int,double*,void*),double (*)(int,int,int,double*,void*),void*,void (*)(void*));

int NLPAddEqualityConstraintByString(NLProblem,char*,int,int*,char*,char*);
int NLPAddInequalityConstraintByString(NLProblem,char*,double,double,int,int*,char*,char*);
int NLPSetObjectiveByString(NLProblem,char*,int,int*,char*,char*);

void NLSetDontInitGradToZero(int i);

#ifdef __cplusplus
 }
#endif

#endif
