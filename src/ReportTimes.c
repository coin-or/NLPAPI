/*
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 5/1/2001.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory

    author: Mike Henderson mhender@watson.ibm.com
*/
#include <NLPAPI.h>

extern double NLEvaluateObjectiveUTime;
extern double NLEvaluateObjectiveSTime;
extern double NLEvaluateEqualityConstraintUTime;
extern double NLEvaluateEqualityConstraintSTime;
extern double NLEvaluateInequalityConstraintUTime;
extern double NLEvaluateInequalityConstraintSTime;

void NLReportTimes()
 {
  if(NLEvaluateElementFunctionNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Element Function\n",NLEvaluateElementFunctionTime,NLEvaluateElementFunctionNCalls);
  NLEvaluateElementFunctionTime=0.;
  NLEvaluateElementFunctionNCalls=0;

  if(NLEvaluateElementFunctionDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Gradient of Element Function\n",NLEvaluateElementFunctionDerTime,NLEvaluateElementFunctionDerNCalls);
  NLEvaluateElementFunctionDerTime=0.;
  NLEvaluateElementFunctionDerNCalls=0;

  if(NLEvaluateElementFunctionSecDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Hessian of Element Function\n",NLEvaluateElementFunctionSecDerTime,NLEvaluateElementFunctionSecDerNCalls);
  NLEvaluateElementFunctionSecDerTime=0.;
  NLEvaluateElementFunctionSecDerNCalls=0;

  if(NLEvaluateGroupFunctionNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Group Function\n",NLEvaluateGroupFunctionTime,NLEvaluateGroupFunctionNCalls);
  NLEvaluateGroupFunctionTime=0.;
  NLEvaluateGroupFunctionNCalls=0;

  if(NLEvaluateGroupFunctionDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Gradient of Group Function\n",NLEvaluateGroupFunctionDerTime,NLEvaluateGroupFunctionDerNCalls);
  NLEvaluateGroupFunctionDerTime=0.;
  NLEvaluateGroupFunctionDerNCalls=0;

  if(NLEvaluateGroupFunctionSecDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Hessian of Group Function\n",NLEvaluateGroupFunctionSecDerTime,NLEvaluateGroupFunctionSecDerNCalls);
  NLEvaluateGroupFunctionSecDerTime=0.;
  NLEvaluateGroupFunctionSecDerNCalls=0;

  if(NLEvaluateObjectiveNCalls>0)
   {
    printf("     %10.2f secs in %10d calls to Evaluate Objective",NLEvaluateObjectiveTime,NLEvaluateObjectiveNCalls);
    printf(" User %lf secs System %lf\n",NLEvaluateObjectiveUTime,NLEvaluateObjectiveSTime);
   }
  NLEvaluateObjectiveTime=0.;
  NLEvaluateObjectiveUTime=0.;
  NLEvaluateObjectiveSTime=0.;
  NLEvaluateObjectiveNCalls=0;

  if(NLEvaluateObjectiveDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Gradient of Objective\n",NLEvaluateObjectiveDerTime,NLEvaluateObjectiveDerNCalls);
  NLEvaluateObjectiveDerTime=0.;
  NLEvaluateObjectiveDerNCalls=0;

  if(NLEvaluateObjectiveSecDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Hessian of Objective\n",NLEvaluateObjectiveSecDerTime,NLEvaluateObjectiveSecDerNCalls);
  NLEvaluateObjectiveSecDerTime=0.;
  NLEvaluateObjectiveSecDerNCalls=0;

  if(NLEvaluateEqualityConstraintNCalls>0)
   {
    printf("     %10.2f secs in %10d calls to Evaluate Equality Constraint",NLEvaluateEqualityConstraintTime,NLEvaluateEqualityConstraintNCalls);
    printf(" User %lf secs System %lf\n",NLEvaluateEqualityConstraintUTime,NLEvaluateEqualityConstraintSTime);
   }
  NLEvaluateEqualityConstraintTime=0.;
  NLEvaluateEqualityConstraintUTime=0.;
  NLEvaluateEqualityConstraintSTime=0.;
  NLEvaluateEqualityConstraintNCalls=0;

  if(NLEvaluateEqualityConstraintDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Gradient of Equality Constraint\n",NLEvaluateEqualityConstraintDerTime,NLEvaluateEqualityConstraintDerNCalls);
  NLEvaluateEqualityConstraintDerTime=0.;
  NLEvaluateEqualityConstraintDerNCalls=0;

  if(NLEvaluateEqualityConstraintSecDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Hessian of Equality Constraint\n",NLEvaluateEqualityConstraintSecDerTime,NLEvaluateEqualityConstraintSecDerNCalls);
  NLEvaluateEqualityConstraintSecDerTime=0.;
  NLEvaluateEqualityConstraintSecDerNCalls=0;

  if(NLEvaluateInequalityConstraintNCalls>0)
   {
    printf("     %10.2f secs in %10d calls to Evaluate Inequality Constraint\n",NLEvaluateInequalityConstraintTime,NLEvaluateInequalityConstraintNCalls);
    printf(" User %lf secs System %lf\n",NLEvaluateInequalityConstraintUTime,NLEvaluateInequalityConstraintSTime);
   }
  NLEvaluateInequalityConstraintTime=0.;
  NLEvaluateInequalityConstraintUTime=0.;
  NLEvaluateInequalityConstraintSTime=0.;
  NLEvaluateInequalityConstraintNCalls=0;

  if(NLEvaluateInequalityConstraintDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Gradient of Inequality Constraint\n",NLEvaluateInequalityConstraintDerTime,NLEvaluateInequalityConstraintDerNCalls);
  NLEvaluateInequalityConstraintDerTime=0.;
  NLEvaluateInequalityConstraintDerNCalls=0;

  if(NLEvaluateInequalityConstraintSecDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Hessian of Inequality Constraint\n",NLEvaluateInequalityConstraintSecDerTime,NLEvaluateInequalityConstraintSecDerNCalls);
  NLEvaluateInequalityConstraintSecDerTime=0.;
  NLEvaluateInequalityConstraintSecDerNCalls=0;

  if(NLEvaluateMinMaxConstraintNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate MinMax Constraint\n",NLEvaluateMinMaxConstraintTime,NLEvaluateMinMaxConstraintNCalls);
  NLEvaluateMinMaxConstraintTime=0.;
  NLEvaluateMinMaxConstraintNCalls=0;

  if(NLEvaluateMinMaxConstraintDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Gradient of MinMax Constraint\n",NLEvaluateMinMaxConstraintDerTime,NLEvaluateMinMaxConstraintDerNCalls);
  NLEvaluateMinMaxConstraintDerTime=0.;
  NLEvaluateMinMaxConstraintDerNCalls=0;

  if(NLEvaluateMinMaxConstraintSecDerNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Hessian of MinMax Constraint\n",NLEvaluateMinMaxConstraintSecDerTime,NLEvaluateMinMaxConstraintSecDerNCalls);
  NLEvaluateMinMaxConstraintSecDerTime=0.;
  NLEvaluateMinMaxConstraintSecDerNCalls=0;

  if(NLEvaluateGroupNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Group\n",NLEvaluateGroupTime,NLEvaluateGroupNCalls);
  NLEvaluateGroupTime=0.;
  NLEvaluateGroupNCalls=0;

  if(NLEvaluateElementNCalls>0)
    printf("     %10.2f secs in %10d calls to Evaluate Element\n",NLEvaluateElementTime,NLEvaluateElementNCalls);
  NLEvaluateElementTime=0.;
  NLEvaluateElementNCalls=0;

  if(NLDetermineSparsityNCalls>0)
    printf("     %10.2f secs in %10d calls to Determine Sparsity of Hessians\n",NLDetermineSparsityTime,NLDetermineSparsityNCalls);
  NLDetermineSparsityTime=0.;
  NLDetermineSparsityNCalls=0;

  if(NLSumIntoNCalls>0)
    printf("     %10.2f secs in %10d calls to Fill in Hessians\n",NLSumIntoTime,NLSumIntoNCalls);
  NLSumIntoTime=0.;
  NLSumIntoNCalls=0;

  if(NLNumberOfUpdatesSkipped>0)
    printf("     %d Element Hessian Updates skipped\n",NLNumberOfUpdatesSkipped);
  NLNumberOfUpdatesSkipped=0;

  return;
 }
