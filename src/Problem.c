/*
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory
*/
/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: %W% %D% %T% */
/*      date:   November 11, 1997                     */
/*              February 2, 1999   Ported to C        */
/*              September 24, 2001 Added multiple groups for constraints */
/*                                 add and remove variables */
/*                                 remove groups and constraints */
/*                                 hide constraints */
/*              September 26, 2001 Added query for polynomial order of variables*/

#include <NLPAPI.h>
#include <string.h>
#include <time.h>

#define NUMBEROFGROUPSTOALLOCATE 5000
#define NUMBEROFGROUPSINOBJECTIVETOALLOCATE 10
#define NUMBEROFINEQUALITYCONSTRAINTSTOALLOCATE 5000
#define NUMBEROFINEQUALITYCONSTRAINTGROUPSTOALLOCATE 5
#define NUMBEROFEQUALITYCONSTRAINTSTOALLOCATE 5000
#define NUMBEROFEQUALITYCONSTRAINTGROUPSTOALLOCATE 5
#define NUMBEROFMINMAXCONSTRAINTSTOALLOCATE 5000
#define NUMBEROFMINMAXCONSTRAINTGROUPSTOALLOCATE 5
#define NUMBEROFELEMENTSINGROUPTOALLOCATE 10
#define NUMBEROFGROUPTYPESTOALLOCATE 5000
#define NUMBEROFELEMENTTYPESTOALLOCATE 10

/* These control the caching of groups and elements. If =1 then caching */

#define NLPCACHEELEMENTS 0;
#define NLPCACHEGROUPS 0;

/* These are internal routines - probably should go in their own include file. */

void NLSetError(int,char*,char*,int,char*);
static char NLProblemErrorMsg[256]="";
int NLPAddGroupType(NLProblem,char*);
int NLPAddGroup(NLProblem,char*);
int NLPAddElementType(NLProblem,char*,int);
void NLPrintVector(FILE*,NLVector);
void NLPPrintVector(FILE*,int*,NLProblem,NLVector);
void NLPrintNonlinearElement(FILE*,NLProblem,NLNonlinearElement);
void NLPrintNonlinearElementOld(FILE*,NLProblem,NLNonlinearElement);
void NLPrintGroup(FILE*,NLProblem,int);
void NLPrintGroupOld(FILE*,NLProblem,int);
void NLPrintGroupShort(FILE*,NLProblem,int,int);
void NLPrintElement(FILE*,NLProblem,int);
NLNonlinearElementPtr NLPGetNonlinearElementPtrOfGroup(NLProblem,int,int);
NLNonlinearElement    NLPGetNonlinearElementOfGroup(NLProblem,int,int);
void NLVectorIncreaseLength(NLVector,int);
void NLVectorDecreaseLength(NLVector,int);
void NLPDeleteGroup(NLProblem,int);
char *NLEGetExpr(NLElementFunction);

double NLPEvaluateGroup(NLProblem,int,NLVector);
double NLPEvaluateElement(NLProblem,int,NLVector);

int *NLNEGetElementVariables(NLProblem,NLNonlinearElement);

typedef void (*groupFunctionDataFreer)(void*);

int ttmpflag=0; /* REMOVEME */
int nttmp=100000;
int ttmp1=0;
int ttmp2=0;
int ttmp3=1;

struct NLGrpPart
 {
   char *problemName;
   int nVariables;
   double *variableScale;
   char **variableName;

   int cache;

   int nGroups;
   int mGroups;
   char **groupName;
   NLGroupFunction *groupFunction;
   void **groupFunctionData;
   groupFunctionDataFreer *freeGroupFunctionData;
   NLVector *groupA;
   int *groupBSet;
   double *groupB;
   double *groupScale;
   int *nElementsInGroup;
   int *mElementsInGroup;
   int **elementWeightSet;
   double **elementWeight;
   int **element;
   int *groupCached;
   double *groupValue;
   double *groupGradient;
   double *groupHessian;

   int nNonlinearElements;
   int mNonlinearElements;
   NLNonlinearElementPtr *nonlinearElement;
   int *elementCached;
   double **internalVariables;
   double **elementVariables;
   double *elementValue;
   double **elementGradient;
   double **elementHessian;

   int nGroupTypes;
   int mGroupTypes;
   char **groupTypeName;

   int nElementTypes;
   int mElementTypes;
   char **elementTypeName;
   int *elementRangeSet;

   int nGroupsInObjective;
   int mGroupsInObjective;
   int *groupsInObjective;
   double lowerBoundOnObjective;
   double upperBoundOnObjective;
   int zgroupnumber;

   double *simpleConstraintLowerBound; 
   double *simpleConstraintUpperBound; 

   int hideEqualityConstraints;
   int nEqualityConstraints;
   int mEqualityConstraints;
   int *nEqualityConstraintGroups;
   int *mEqualityConstraintGroups;
   int **equalityConstraintGroups;

   int hideMinMaxConstraints;
   int nMinMaxConstraints;
   int mMinMaxConstraints;
   int *nMinMaxConstraintGroups;
   int *mMinMaxConstraintGroups;
   int **minMaxConstraintGroups;
   double lowerBoundOnMinMaxVar;
   double upperBoundOnMinMaxVar;

   int hideInequalityConstraints;
   int nInequalityConstraints;
   int mInequalityConstraints;
   int *nInequalityConstraintGroups;
   int *mInequalityConstraintGroups;
   int **inequalityConstraintGroups;
   double *inequalityConstraintLowerBound; 
   double *inequalityConstraintUpperBound; 
 };

int NLPInternalExtendArray(int*,int*,int**,int,int);
int NLPInternalExtendIndirectArrays(int*,int*,int***,int*,int**,int**,double**,double,double**,double,int);

double NLEvaluateObjectiveTime=0.;
int NLEvaluateObjectiveNCalls=0;
double NLEvaluateObjectiveDerTime=0.;
int NLEvaluateObjectiveDerNCalls=0;
double NLEvaluateObjectiveSecDerTime=0.;
int NLEvaluateObjectiveSecDerNCalls=0;

double NLEvaluateEqualityConstraintTime=0.;
int NLEvaluateEqualityConstraintNCalls=0;
double NLEvaluateEqualityConstraintDerTime=0.;
int NLEvaluateEqualityConstraintDerNCalls=0;
double NLEvaluateEqualityConstraintSecDerTime=0.;
int NLEvaluateEqualityConstraintSecDerNCalls=0;

double NLEvaluateInequalityConstraintTime=0.;
int NLEvaluateInequalityConstraintNCalls=0;
double NLEvaluateInequalityConstraintDerTime=0.;
int NLEvaluateInequalityConstraintDerNCalls=0;
double NLEvaluateInequalityConstraintSecDerTime=0.;
int NLEvaluateInequalityConstraintSecDerNCalls=0;

double NLEvaluateMinMaxConstraintTime=0.;
int NLEvaluateMinMaxConstraintNCalls=0;
double NLEvaluateMinMaxConstraintDerTime=0.;
int NLEvaluateMinMaxConstraintDerNCalls=0;
double NLEvaluateMinMaxConstraintSecDerTime=0.;
int NLEvaluateMinMaxConstraintSecDerNCalls=0;

double NLEvaluateGroupTime=0.;
int NLEvaluateGroupNCalls=0;
double NLEvaluateElementTime=0.;
int NLEvaluateElementNCalls=0;

NLProblem NLCreateProblem(char *probName, int nV)
 {
  char RoutineName[]="NLCreateProblem";
  int i;
  NLProblem this;
  int verbose;

  verbose=0;

  if(verbose){printf("%s(%s,%d)\n",RoutineName,probName,nV);fflush(stdout);}

  if(nV<1)
   {
    sprintf(NLProblemErrorMsg,"Number of Variables %d (argument 2) must be positive",nV);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }

  this=(NLProblem)malloc(sizeof(struct NLGrpPart));
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPart));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }

  if(probName!=(char*)NULL)
   {
    this->problemName=(char*)malloc((strlen(probName)+1)*sizeof(char));
    if(this->problemName==(char*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(probName)+1)*sizeof(char));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    strcpy(this->problemName,probName);
   }else{
    sprintf(NLProblemErrorMsg,"Problem name (argument 1) is NULL");
    NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    if(verbose){printf("%s\n",NLProblemErrorMsg);fflush(stdout);}
    this->problemName=(char*)NULL;
   }

  this->nVariables=nV;
  this->variableScale=(double*)malloc(sizeof(double)*nV);
  if(this->variableScale==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nV*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  this->variableName=(char**)malloc(nV*sizeof(char*));
  if(this->variableName==(char**)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nV*sizeof(char*));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  for(i=0;i<nV;i++)
   {
    this->variableScale[i]=1.;
    this->variableName[i]=(char*)malloc(sizeof(char)*40);
    if(this->variableName[i]==(char*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",40*sizeof(char));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
     }
/*  sprintf(this->variableName[i],"X%7.7x",i+1);*/
    sprintf(this->variableName[i],"X%x",i+1);
   }

  this->cache=0;

  this->nGroups=0;
  this->mGroups=-1;
  this->groupName=(char**)NULL;
  this->groupFunction=(NLGroupFunction*)NULL;
  this->groupFunctionData=(void**)NULL;
  this->freeGroupFunctionData=(groupFunctionDataFreer*)NULL;
  this->groupA=(NLVector*)NULL;
  this->groupB=(double*)NULL;
  this->groupBSet=(int*)NULL;
  this->groupScale=(double*)NULL;
  this->nElementsInGroup=(int*)NULL;
  this->mElementsInGroup=(int*)NULL;
  this->elementWeight=(double**)NULL;
  this->elementWeightSet=(int**)NULL;
  this->element=(int**)NULL;
  this->groupCached=(int*)NULL;
  this->groupValue=(double*)NULL;
  this->groupGradient=(double*)NULL;
  this->groupHessian=(double*)NULL;

  this->nGroupTypes=0;
  this->mGroupTypes=-1;
  this->groupTypeName=(char**)NULL;

  this->nNonlinearElements=0;
  this->mNonlinearElements=-1;
  this->nonlinearElement=(NLNonlinearElementPtr*)NULL;
  this->elementCached=(int*)NULL;
  this->internalVariables=(double**)NULL;
  this->elementVariables=(double**)NULL;
  this->elementValue=(double*)NULL;
  this->elementGradient=(double**)NULL;
  this->elementHessian=(double**)NULL;

  this->nElementTypes=0;
  this->mElementTypes=-1;
  this->elementTypeName=(char**)NULL;

  this->nGroupsInObjective=0;
  this->mGroupsInObjective=-1;
  this->groupsInObjective=(int*)NULL;
  this->lowerBoundOnObjective=DBL_MIN;
  this->upperBoundOnObjective=DBL_MAX;

  this->simpleConstraintLowerBound=(double*)malloc(sizeof(double)*this->nVariables);
  if(this->simpleConstraintLowerBound==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  this->simpleConstraintUpperBound=(double*)malloc(sizeof(double)*this->nVariables);
  if(this->simpleConstraintUpperBound==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  for(i=0;i<this->nVariables;i++)
   {
    this->simpleConstraintLowerBound[i]=-DBL_MAX;
    this->simpleConstraintUpperBound[i]=DBL_MAX;
   }

  this->hideEqualityConstraints=0;
  this->nEqualityConstraints=0;
  this->mEqualityConstraints=-1;
  this->nEqualityConstraintGroups=(int*)NULL;
  this->mEqualityConstraintGroups=(int*)NULL;
  this->equalityConstraintGroups=(int**)NULL;

  this->hideMinMaxConstraints=0;
  this->nMinMaxConstraints=0;
  this->mMinMaxConstraints=-1;
  this->zgroupnumber=-1;
  this->nMinMaxConstraintGroups=(int*)NULL;
  this->mMinMaxConstraintGroups=(int*)NULL;
  this->minMaxConstraintGroups=(int**)NULL;
  this->lowerBoundOnMinMaxVar=0.;
  this->upperBoundOnMinMaxVar=DBL_MAX;

  this->hideInequalityConstraints=0;
  this->nInequalityConstraints=0;
  this->mInequalityConstraints=-1;
  this->nInequalityConstraintGroups=(int*)NULL;
  this->mInequalityConstraintGroups=(int*)NULL;
  this->inequalityConstraintGroups=(int**)NULL;
  this->inequalityConstraintLowerBound=(double*)NULL;
  this->inequalityConstraintUpperBound=(double*)NULL; 

  NLPAddGroupType(this,"TRIVIAL GROUP");

  return(this);
 }

void NLFreeProblem(NLProblem this)
 {
  char RoutineName[]="NLFreeProblem";
  int i,j;
  int verbose;

  verbose=FALSE;

  if(verbose)printf("NLFreeProblem()\n");

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(verbose)printf("  delete problem name \n");
  if(this->problemName!=(char*)NULL)free(this->problemName);

  if(this->variableScale!=(double*)NULL)free(this->variableScale);
  
  if(this->variableName!=(char**)NULL)
   {
    for(i=0;i<this->nVariables;i++)
     if(this->variableName[i]!=(char*)NULL)free(this->variableName[i]);
    free(this->variableName);
   }
  
  if(this->nGroups>0)
   {
    for(i=0;i<this->nGroups;i++)
     {
      if(verbose)printf("  delete group %d\n",i);
      if(this->groupFunction!=(NLGroupFunction*)NULL && this->groupFunction[i]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[i]);
      if(this->groupA!=(NLVector*)NULL && this->groupA[i]!=(NLVector)NULL)NLFreeVector(this->groupA[i]);
      if(this->elementWeight!=(double**)NULL&& this->elementWeight[i]!=(double*)NULL)free(this->elementWeight[i]);
      if(this->elementWeightSet!=(int**)NULL&& this->elementWeightSet[i]!=(int*)NULL)free(this->elementWeightSet[i]);
     }
    if(this->groupName!=(char**)NULL)free(this->groupName);
    if(this->groupFunction!=(NLGroupFunction*)NULL)free(this->groupFunction);
    if(this->groupA!=(NLVector*)NULL)free(this->groupA);
    if(this->groupB!=(double*)NULL)free(this->groupB);
    if(this->groupBSet!=(int*)NULL)free(this->groupBSet);
    if(this->groupScale!=(double*)NULL)free(this->groupScale);
    if(this->nElementsInGroup!=(int*)NULL)free(this->nElementsInGroup);
    if(this->mElementsInGroup!=(int*)NULL)free(this->mElementsInGroup);
    if(this->element!=(int**)NULL)free(this->element);
    if(this->elementWeight!=(double**)NULL)free(this->elementWeight);
    if(this->elementWeightSet!=(int**)NULL)free(this->elementWeightSet);
    if(this->groupCached!=(int*)NULL)free(this->groupCached);
    if(this->groupValue!=(double*)NULL)free(this->groupValue);
    if(this->groupGradient!=(double*)NULL)free(this->groupGradient);
    if(this->groupHessian!=(double*)NULL)free(this->groupHessian);
   }

  for(i=0;i<this->nNonlinearElements;i++)
   {
    NLFreeNonlinearElement(this,i);
    if(this->internalVariables[i]!=(double*)NULL)free(this->internalVariables[i]);
    if(this->elementVariables[i]!=(double*)NULL)free(this->elementVariables[i]);
    if(this->elementGradient[i]!=(double*)NULL)free(this->elementGradient[i]);
    if(this->elementHessian[i]!=(double*)NULL)free(this->elementHessian[i]);
   }
  if(this->nonlinearElement!=(NLNonlinearElementPtr*)NULL)free(this->nonlinearElement);
  if(this->elementCached!=(int*)NULL)free(this->elementCached);
  if(this->internalVariables!=(double**)NULL)free(this->internalVariables);
  if(this->elementVariables!=(double**)NULL)free(this->elementVariables);
  if(this->elementValue!=(double*)NULL)free(this->elementValue);
  if(this->elementGradient!=(double**)NULL)free(this->elementGradient);
  if(this->elementHessian!=(double**)NULL)free(this->elementHessian);

  if(this->nGroupTypes>0)
   {
    for(i=0;i<this->nGroupTypes;i++)
     {
      if(verbose)printf("  delete group type %d\n",i);
      if(this->groupTypeName!=(char**)NULL && this->groupTypeName[i]!=(char*)NULL)free(this->groupTypeName[i]);
     }
    if(this->groupTypeName!=(char**)NULL)free(this->groupTypeName);
   }

  if(this->nElementTypes>0)
   {
    for(i=0;i<this->nElementTypes;i++)
     {
      if(verbose)printf("  delete element type %d",i);
      if(this->elementTypeName!=(char**)NULL && this->elementTypeName[i]!=(char*)NULL)free(this->elementTypeName[i]);
     }
    if(verbose)printf("  delete element label list %d\n",i);
    if(this->elementTypeName!=(char**)NULL)free(this->elementTypeName);
   }

  if(verbose)printf("  delete Objective Lists\n");
  if(this->groupsInObjective!=(int*)NULL)free(this->groupsInObjective);
 
  if(verbose)printf("  delete Simple Constraint Lists\n");
  if(this->simpleConstraintLowerBound!=(double*)NULL)free(this->simpleConstraintLowerBound);
  if(this->simpleConstraintUpperBound!=(double*)NULL)free(this->simpleConstraintUpperBound);
 
  if(verbose)printf("  delete Equality Constraint Lists\n");
  for(i=0;i<this->mEqualityConstraints;i++)
     if(this->equalityConstraintGroups[i]!=(int*)NULL)free(this->equalityConstraintGroups[i]);
  if(this->equalityConstraintGroups!=(int**)NULL)free(this->equalityConstraintGroups);
  if(this->nEqualityConstraintGroups!=(int*)NULL)free(this->nEqualityConstraintGroups);
  if(this->mEqualityConstraintGroups!=(int*)NULL)free(this->mEqualityConstraintGroups);
 
  if(verbose)printf("  delete MinMax Constraint Lists\n");
  for(i=0;i<this->mMinMaxConstraints;i++)
    if(this->minMaxConstraintGroups[i]=(int*)NULL)free(this->minMaxConstraintGroups[i]);
  if(this->minMaxConstraintGroups!=(int**)NULL)free(this->minMaxConstraintGroups);
  if(this->nMinMaxConstraintGroups!=(int*)NULL)free(this->nMinMaxConstraintGroups);
  if(this->mMinMaxConstraintGroups!=(int*)NULL)free(this->mMinMaxConstraintGroups);

  if(verbose)printf("  delete Inequality Constraint Lists\n");
  for(i=0;i<this->mInequalityConstraints;i++)
    if(this->inequalityConstraintGroups[i]!=(int*)NULL)free(this->inequalityConstraintGroups[i]);
  if(this->inequalityConstraintGroups!=(int**)NULL)free(this->inequalityConstraintGroups);
  if(this->nInequalityConstraintGroups!=(int*)NULL)free(this->nInequalityConstraintGroups);
  if(this->mInequalityConstraintGroups!=(int*)NULL)free(this->mInequalityConstraintGroups);
  if(this->inequalityConstraintLowerBound!=(double*)NULL)free(this->inequalityConstraintLowerBound);
  if(this->inequalityConstraintUpperBound!=(double*)NULL)free(this->inequalityConstraintUpperBound);

  if(verbose)printf("done NLFreeProblem()\n");
  return;
 }

int NLPAddGroup(NLProblem this,char *name)
 {
  char RoutineName[]="NLPAddGroup";
  int i;
  char **tmpgroupName;
  NLGroupFunction *tmpgroupFunction;
  void **tmpgroupFunctionData;
  groupFunctionDataFreer *tmpfreeGroupFunctionData;
  NLVector *tmpgroupA;
  double *tmpgroupB;
  int *tmpgroupBSet;
  double *tmpgroupScale;
  int *tmpnElementsInGroup;
  int *tmpmElementsInGroup;
  double **tmpelementWeight;
  int **tmpelementWeightSet;
  int **tmpelement;
  int *tmpgroupCached;
  double *tmpgroupValue;
  double *tmpgroupGradient;
  double *tmpgroupHessian;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(this->nGroups>=this->mGroups)
   {
    if(this->mGroups==-1)
     {
      this->mGroups=NUMBEROFGROUPSTOALLOCATE;

      this->groupName=(char**)malloc(this->mGroups*sizeof(char*));
      if(this->groupName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupFunction=(NLGroupFunction*)malloc(this->mGroups*sizeof(NLGroupFunction));
      if(this->groupFunction==(NLGroupFunction*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLGroupFunction));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupFunctionData=(void**)malloc(this->mGroups*sizeof(void*));
      if(this->groupFunctionData==(void**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(void**));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->freeGroupFunctionData=(groupFunctionDataFreer*)malloc(this->mGroups*sizeof(groupFunctionDataFreer));
      if(this->freeGroupFunctionData==(groupFunctionDataFreer*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(groupFunctionDataFreer));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupA=(NLVector*)malloc(this->mGroups*sizeof(NLVector));
      if(this->groupA==(NLVector*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLVector*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupB=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupB==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupBSet=(int*)malloc(this->mGroups*sizeof(int));
      if(this->groupBSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupScale=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupScale==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->nElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(this->nElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->mElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(this->mElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->elementWeight=(double**)malloc(sizeof(double*)*this->mGroups);
      if(this->elementWeight==(double**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->elementWeightSet=(int**)malloc(sizeof(int*)*this->mGroups);
      if(this->elementWeightSet==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->element=(int**)malloc(sizeof(int*)*this->mGroups);
      if(this->element==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupCached=(int*)malloc(sizeof(int)*this->mGroups);
      if(this->groupCached==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupValue=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupValue==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupGradient=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupGradient==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->groupHessian=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupHessian==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }


      for(i=0;i<this->mGroups;i++)
       {
        this->groupName[i]=(char*)NULL;
        this->groupFunction[i]=(NLGroupFunction)NULL;
        this->groupA[i]=(NLVector)NULL;
        this->groupB[i]=0.;
        this->groupBSet[i]=FALSE;
        this->groupScale[i]=1.;
        this->nElementsInGroup[i]=0;
        this->mElementsInGroup[i]=-1;
        this->elementWeight[i]=(double*)NULL;
        this->elementWeightSet[i]=(int*)NULL;
        this->element[i]=(int*)NULL;
        this->groupCached[i]=0;
        this->groupValue[i]=0.;
        this->groupGradient[i]=0.;
        this->groupHessian[i]=0.;
       }
     }else{
      this->mGroups+=NUMBEROFGROUPSTOALLOCATE;

      tmpgroupName=(char**)malloc(this->mGroups*sizeof(char*));
      if(tmpgroupName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupFunction=(NLGroupFunction*)malloc(sizeof(NLGroupFunction)*this->mGroups);
      if(tmpgroupFunction==(NLGroupFunction*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLGroupFunction));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupFunctionData=(void**)malloc(this->mGroups*sizeof(void*));
      if(tmpgroupFunctionData==(void**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(void**));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpfreeGroupFunctionData=(groupFunctionDataFreer*)malloc(this->mGroups*sizeof(groupFunctionDataFreer));
      if(tmpfreeGroupFunctionData==(groupFunctionDataFreer*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(groupFunctionDataFreer));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupA=(NLVector*)malloc(sizeof(NLVector)*this->mGroups);
      if(tmpgroupA==(NLVector*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLVector));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupB=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupB==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupBSet=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpgroupBSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupScale=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupScale==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpnElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpnElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpmElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpmElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpelementWeight=(double**)malloc(sizeof(double*)*this->mGroups);
      if(tmpelementWeight==(double**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpelementWeightSet=(int**)malloc(sizeof(int*)*this->mGroups);
      if(tmpelementWeightSet==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpelement=(int**)malloc(sizeof(int*)*this->mGroups);
      if(tmpelement==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupCached=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpgroupCached==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupValue=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupValue==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupGradient=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupGradient==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpgroupHessian=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupHessian==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }


      for(i=0;i<this->nGroups;i++)
       {
        tmpgroupName[i]=this->groupName[i];
        tmpgroupFunction[i]=this->groupFunction[i];
        tmpgroupFunctionData[i]=this->groupFunctionData[i];
        tmpfreeGroupFunctionData[i]=this->freeGroupFunctionData[i];
        tmpgroupA[i]=this->groupA[i];
        tmpgroupB[i]=this->groupB[i];
        tmpgroupBSet[i]=this->groupBSet[i];
        tmpgroupScale[i]=this->groupScale[i];
        tmpnElementsInGroup[i]=this->nElementsInGroup[i];
        tmpmElementsInGroup[i]=this->mElementsInGroup[i];
        tmpelementWeight[i]=this->elementWeight[i];
        tmpelementWeightSet[i]=this->elementWeightSet[i];
        tmpelement[i]=this->element[i];
        tmpgroupCached[i]=this->groupCached[i];
        tmpgroupValue[i]=this->groupValue[i];
        tmpgroupGradient[i]=this->groupValue[i];
        tmpgroupHessian[i]=this->groupValue[i];
       }
      for(i=this->nGroups;i<this->mGroups;i++)
       {
        tmpgroupName[i]=(char*)NULL;
        tmpgroupFunction[i]=(NLGroupFunction)NULL;
        tmpgroupFunctionData[i]=(void*)NULL;
        tmpfreeGroupFunctionData[i]=(groupFunctionDataFreer)NULL;
        tmpgroupA[i]=(NLVector)NULL;
        tmpgroupB[i]=0.;
        tmpgroupBSet[i]=FALSE;
        tmpgroupScale[i]=1.;
        tmpnElementsInGroup[i]=0;
        tmpmElementsInGroup[i]=-1;
        tmpelementWeight[i]=(double*)NULL;
        tmpelementWeightSet[i]=(int*)NULL;
        tmpelement[i]=(int*)NULL;
        tmpgroupCached[i]=0;
        tmpgroupValue[i]=0.;
        tmpgroupGradient[i]=0.;
        tmpgroupHessian[i]=0.;
       }
      free(this->groupName);
      free(this->groupFunction);
      free(this->groupA);
      free(this->groupB);
      free(this->groupBSet);
      free(this->groupScale);
      free(this->nElementsInGroup);
      free(this->mElementsInGroup);
      free(this->elementWeight);
      free(this->elementWeightSet);
      free(this->element);
      free(this->groupCached);
      free(this->groupValue);
      free(this->groupGradient);
      free(this->groupHessian);

      this->groupName=tmpgroupName;
      this->groupFunction=tmpgroupFunction;
      this->groupFunctionData=tmpgroupFunctionData;
      this->freeGroupFunctionData=tmpfreeGroupFunctionData;
      this->groupA=tmpgroupA;
      this->groupB=tmpgroupB;
      this->groupBSet=tmpgroupBSet;
      this->groupScale=tmpgroupScale;
      this->nElementsInGroup=tmpnElementsInGroup;
      this->mElementsInGroup=tmpmElementsInGroup;
      this->elementWeight=tmpelementWeight;
      this->elementWeightSet=tmpelementWeightSet;
      this->element=tmpelement;
      this->groupCached=tmpgroupCached;
      this->groupValue=tmpgroupValue;
      this->groupGradient=tmpgroupGradient;
      this->groupHessian=tmpgroupHessian;
     }
   }

  for(i=0;i<this->nGroups;i++)
   {
    if(!strcmp(this->groupName[i],name))
     {
      sprintf(NLProblemErrorMsg,"You are creating a group named \"%s\" which is the same name given to group %d.",name,i);
      NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
     }
   }

  if(name!=(char*)NULL)
   {
    this->groupName[this->nGroups]=(char*)malloc(sizeof(char)*(strlen(name)+1));
    if(this->groupName[this->nGroups]==(char*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+1)*sizeof(char));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
     }

    strcpy(this->groupName[this->nGroups],name);
   }else
    this->groupName[this->nGroups]=(char*)NULL;

  this->groupFunction[this->nGroups]=(NLGroupFunction)NULL;
  this->groupFunctionData[this->nGroups]=(void*)NULL;
  this->freeGroupFunctionData[this->nGroups]=(groupFunctionDataFreer)NULL;
  this->groupA[this->nGroups]=(NLVector)NULL;
  this->groupB[this->nGroups]=0.;
  this->groupBSet[this->nGroups]=FALSE;
  this->groupScale[this->nGroups]=1.;
  this->nElementsInGroup[this->nGroups]=0;
  this->mElementsInGroup[this->nGroups]=-1;
  this->elementWeight[this->nGroups]=(double*)NULL;
  this->elementWeightSet[this->nGroups]=(int*)NULL;
  this->element[this->nGroups]=(int*)NULL;
  this->groupCached[this->nGroups]=0;
  this->groupValue[this->nGroups]=0.;
  this->groupGradient[this->nGroups]=0.;
  this->groupHessian[this->nGroups]=0.;

  this->nGroups++;

  return(this->nGroups-1);
 }

int NLPAddGroupToObjective(NLProblem this,char *name)
 {
  char RoutineName[]="NLPAddGroupToObjective";
  int i;
  int *tmpgroupsInObjective;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(this->nGroupsInObjective>=this->mGroupsInObjective)
   {
    if(this->mGroupsInObjective==-1)
     {
      this->mGroupsInObjective=NUMBEROFGROUPSINOBJECTIVETOALLOCATE;
      this->groupsInObjective=(int*)malloc(sizeof(int)*this->mGroupsInObjective);
      if(this->groupsInObjective==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroupsInObjective*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->mGroupsInObjective;i++)this->groupsInObjective[i]=-1;
     }else{
      this->mGroupsInObjective+=NUMBEROFGROUPSINOBJECTIVETOALLOCATE;
      tmpgroupsInObjective=(int*)malloc(sizeof(int)*this->mGroupsInObjective);
      if(tmpgroupsInObjective==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroupsInObjective*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->nGroupsInObjective;i++)tmpgroupsInObjective[i]=this->groupsInObjective[i];
      for(i=this->nGroupsInObjective;i<this->mGroupsInObjective;i++)tmpgroupsInObjective[i]=-1;
      free(this->groupsInObjective);
      this->groupsInObjective=tmpgroupsInObjective;
     }
   }
  this->groupsInObjective[this->nGroupsInObjective]=NLPAddGroup(this,name);

  this->nGroupsInObjective++;
  return(this->nGroupsInObjective-1);
 }

int NLPAddNonlinearEqualityConstraint(NLProblem this, char *name)
 {
  char RoutineName[]="NLPAddNonlinearEqualityConstraint";
  int i;
  int result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(NLPInternalExtendIndirectArrays(&(this->nEqualityConstraints),
                                     &(this->mEqualityConstraints),
                                     &(this->equalityConstraintGroups),(int*)NULL,
                                     &(this->nEqualityConstraintGroups),
                                     &(this->mEqualityConstraintGroups),
                                     (double**)NULL,0.,
                                     (double**)NULL,0.,
                                     NUMBEROFEQUALITYCONSTRAINTSTOALLOCATE)!=0)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to add an Equality Constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
   }

  this->nEqualityConstraintGroups[this->nEqualityConstraints]=0;
  this->mEqualityConstraintGroups[this->nEqualityConstraints]=-1;
  this->equalityConstraintGroups[this->nEqualityConstraints]=(int*)NULL;
  result=this->nEqualityConstraints;

  this->nEqualityConstraints++;

  if(name!=(char*)NULL)NLPAddGroupToEqualityConstraint(this,result,name);
  return(result);
 }

int NLPAddNonlinearInequalityConstraint(NLProblem this, char *name)
 {
  char RoutineName[]="NLPAddNonlinearInequalityConstraint";
  int i;
  int result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(NLPInternalExtendIndirectArrays(&(this->nInequalityConstraints),
                                     &(this->mInequalityConstraints),
                                     &(this->inequalityConstraintGroups),(int*)NULL,
                                     &(this->nInequalityConstraintGroups),
                                     &(this->mInequalityConstraintGroups),
                                     &(this->inequalityConstraintLowerBound),-DBL_MAX,
                                     &(this->inequalityConstraintUpperBound),DBL_MAX,
                                     NUMBEROFEQUALITYCONSTRAINTSTOALLOCATE)!=0)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to add an Inequality Constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
   }

  this->nInequalityConstraintGroups[this->nInequalityConstraints]=0;
  this->mInequalityConstraintGroups[this->nInequalityConstraints]=-1;
  this->inequalityConstraintGroups[this->nInequalityConstraints]=(int*)NULL;
  this->inequalityConstraintLowerBound[this->nInequalityConstraints]=0.;
  this->inequalityConstraintUpperBound[this->nInequalityConstraints]=DBL_MAX;
  result=this->nInequalityConstraints;

  this->nInequalityConstraints++;

  if(name!=(char*)NULL)NLPAddGroupToInequalityConstraint(this,result,name);
  return(result);
 }


int NLPSetGroupFunction(NLProblem this,int group,NLGroupFunction g)
 {
  return NLPSetGroupFunctionParm(this,group,g,NULL,NULL);
 }

int NLPSetGroupFunctionParm(NLProblem this,int group,NLGroupFunction g, void *data, void (*freeData)(void*))
 {
  char RoutineName[]="NLPSetGroupFunction";
  int verbose=0;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }
  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefGroupFunction(g);
  if(this->groupFunction[group]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[group]);
  this->groupFunction[group]=g;

  if(this->groupFunctionData[group]!=(void*)NULL && this->freeGroupFunctionData[group]!=(groupFunctionDataFreer)NULL)(this->freeGroupFunctionData[group])(this->groupFunctionData[group]);
  this->groupFunctionData[group]=data;
  this->freeGroupFunctionData[group]=freeData;

  return 1;
 }

int NLPSetGroupA(NLProblem this,int group,NLVector a)
 {
  char RoutineName[]="NLPSetGroupA";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefVector(a);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetGroupB(NLProblem this,int group,double b)
 {
  char RoutineName[]="NLPSetGroupB";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPAddNonlinearElementToGroup(NLProblem this,int group,double weight,NLNonlinearElement E)
 {
  char RoutineName[]="NLPAddNonlinearElementToGroup";
  int i,n,m;
  int verbose;
  double *tmpelementWeight;
  int *tmpelementWeightSet;
  int *tmpelement;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  verbose=FALSE;

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(verbose)printf("addNonlinearElementToGroup, next element is %d and there is space for %d\n",this->nElementsInGroup[group],this->mElementsInGroup[group]);

  if(this->nElementsInGroup[group]>=this->mElementsInGroup[group])
   {
    if(verbose)printf("  need to allocate more space\n");
    if(this->mElementsInGroup[group]==-1)
     {
      if(verbose)printf("  allocating space for first time.\n");
      this->mElementsInGroup[group]=NUMBEROFELEMENTSINGROUPTOALLOCATE;
      this->elementWeight[group]=(double*)malloc(sizeof(double)*this->mElementsInGroup[group]);
      if(this->elementWeight[group]==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementsInGroup[group]*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s\n",NLProblemErrorMsg);fflush(stdout);
        abort();
       }

      this->elementWeightSet[group]=(int*)malloc(sizeof(int)*this->mElementsInGroup[group]);
      if(this->elementWeightSet[group]==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementsInGroup[group]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s\n",NLProblemErrorMsg);fflush(stdout);
        abort();
       }

      this->element[group]=(int*)malloc(sizeof(int)*this->mElementsInGroup[group]);
      if(this->element[group]==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementsInGroup[group]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s\n",NLProblemErrorMsg);fflush(stdout);
        abort();
       }
      for(i=0;i<this->mElementsInGroup[group];i++)
       {
        (this->element[group])[this->nElementsInGroup[group]]=-1;
       }
     }else{
      if(verbose)printf("  reallocating space.\n");
      this->mElementsInGroup[group]+=NUMBEROFELEMENTSINGROUPTOALLOCATE;
      tmpelementWeight=(double*)malloc(sizeof(double)*this->mElementsInGroup[group]);
      if(tmpelementWeight==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementsInGroup[group]*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpelementWeightSet=(int*)malloc(sizeof(int)*this->mElementsInGroup[group]);
      if(tmpelementWeightSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementsInGroup[group]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpelement=(int*)malloc(sizeof(int)*this->mElementsInGroup[group]);
      if(tmpelement==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementsInGroup[group]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->nElementsInGroup[group];i++)
       {
        tmpelementWeight[i]=(this->elementWeight[group])[i];
        tmpelementWeightSet[i]=(this->elementWeightSet[group])[i];
        tmpelement[i]=(this->element[group])[i];
       }
      for(i=this->nElementsInGroup[group];i<this->mElementsInGroup[group];i++)
       {
        tmpelementWeight[i]=1.;
        tmpelementWeightSet[i]=FALSE;
        tmpelement[i]=-1;
       }
      this->elementWeight[group]=tmpelementWeight;
      this->elementWeightSet[group]=tmpelementWeightSet;
      this->element[group]=tmpelement;
     }
   }

  if(verbose)printf(" copy new values into arrays\n");
  (this->elementWeight[group])[this->nElementsInGroup[group]]=weight;
  (this->elementWeightSet[group])[this->nElementsInGroup[group]]=TRUE;
  (this->element[group])[this->nElementsInGroup[group]]=E;
  NLRefNonlinearElement(this->nonlinearElement[E]);

  this->nElementsInGroup[group]++;
  if(verbose)printf(" done addNonlinearElementToGroup\n");
  return(this->nElementsInGroup[group]-1);
 }

int NLPSetLowerSimpleBound(NLProblem this,int variable,double l)
 {
  char RoutineName[]="NLPSetLowerSimpleBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(variable>-1&&variable<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",variable,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->simpleConstraintLowerBound[variable]=l; 
  if(0&&l>-DBL_MAX){printf("Set Lower Bound on variable %d to %lf\n",variable,l);fflush(stdout);}
  return 1;
 }

int NLPSetUpperSimpleBound(NLProblem this,int variable,double r)
 {
  char RoutineName[]="NLPSetUpperSimpleBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(variable>-1&&variable<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",variable,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->simpleConstraintUpperBound[variable]=r; 
  if(0&&r<DBL_MAX)printf("Set Upper Bound on variable %d to %lf\n",variable,r);fflush(stdout);
  return 1;
 }

int NLPSetSimpleBounds(NLProblem this,int variable,double l,double r)
 {
  char RoutineName[]="NLPSetSimpleBounds";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(variable>-1&&variable<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",variable,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->simpleConstraintLowerBound[variable]=l; 
  this->simpleConstraintUpperBound[variable]=r; 
  if(0&&r<DBL_MAX)printf("Set Upper Bound on variable %d to %lf\n",variable,r);fflush(stdout);
  if(0&&l>-DBL_MAX)printf("Set Lower Bound on variable %d to %lf\n",variable,l);fflush(stdout);
  return 1;
 }

int NLPUnSetUpperSimpleBound(NLProblem this,int variable)
 {
  char RoutineName[]="NLPUnSetUpperSimpleBound";
  
  if(0){printf("Remove Upper Bound on variable %d\n",variable);fflush(stdout);}
  NLPSetUpperSimpleBound(this,variable,DBL_MAX);
  return 1;
 }

int NLPUnSetLowerSimpleBound(NLProblem this,int variable)
 {
  char RoutineName[]="NLPUnSetLowerSimpleBound";

  if(0){printf("Remove Lower Bound on variable %d\n",variable);fflush(stdout);}
  NLPSetLowerSimpleBound(this,variable,-DBL_MAX);
  return 1;
 }

int NLPUnSetSimpleBounds(NLProblem this,int variable)
 {
  char RoutineName[]="NLPUnSetSimpleBounds";

  NLPSetSimpleBounds(this,variable,-DBL_MAX,DBL_MAX);
  return 1;
 }

int NLPGetNumberOfGroups(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfGroups";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nGroups);
 }

int NLPGetNumberOfGroupTypes(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfGroupTypes";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nGroupTypes);
 }

char *NLPGetGroupType(NLProblem this,int type)
 {
  char RoutineName[]="NLPGetGroupType";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(!(type>-1&&type<this->nGroupTypes))
   {
    sprintf(NLProblemErrorMsg,"Type %d is illegal (argument 2). Must be in range 0 to %d",type,this->nGroupTypes-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return((char*)NULL);
   }

  return(this->groupTypeName[type]);
 }

char *NLPGetGroupTypeName(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetGroupTypeName";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  return(this->groupTypeName[NLGGetType(this->groupFunction[group])]);
 }

int NLPGetTypeOfGroup(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetTypeOfGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(this->groupFunction[group]!=(NLGroupFunction)NULL)
    return(NLGGetType(this->groupFunction[group]));
   else
    return 0;
 }

int NLPGetNumberOfElementsInGroup(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetNumberOfElementsInGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nElementsInGroup[group]);
 }

double NLPGetElementWeight(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPGetElementWeight";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(!(element>-1&&element<this->nElementsInGroup[group]))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,this->nElementsInGroup[group]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return((this->elementWeight[group])[element]);
 }

NLElementFunction NLPGetElementFunctionOfGroup(NLProblem this,int group, int element)
 {
  char RoutineName[]="NLPGetElementFunctionOfGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  if(!(element>-1&&element<this->nElementsInGroup[group]))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,this->nElementsInGroup[group]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  return(NLNEGetElementFunction(this,(this->element[group])[element]));
 }

NLElementFunction NLPGetElementFunction(NLProblem this,int element)
 {
  char RoutineName[]="NLPGetElementFunction";
  int group,n,m,telement;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  if(!(element>-1&&element<NLPGetNumberOfElements(this)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 2). Must be in range 0 to %d",element,NLPGetNumberOfElements(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

/* element is an element number, while xfrm is indexed on element type. */

  n=NLPGetNumberOfGroups(this);
  m=0;
  telement=element;
  for(group=0;group<n;group++)
   {
    if(telement<NLPGetNumberOfElementsInGroup(this,group))
     {
      return(NLPGetElementFunctionOfGroup(this,group,telement));
     }
    telement-=NLPGetNumberOfElementsInGroup(this,group);
    m+=NLPGetNumberOfElementsInGroup(this,group);
   }
  return((NLElementFunction)NULL);
 }

NLGroupFunction NLPGetGroupFunction(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetGroupFunction";
  int verbose=0;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }

  if(verbose)
   {
    printf("NLPGetGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %8.8x\n",this->groupFunction);
   }
  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }

  return(this->groupFunction[group]);
 }

NLVector NLPGetGroupA(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetGroupA";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  return(this->groupA[group]);
 }

double NLPGetGroupB(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetGroupB";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->groupB[group]);
 }

int NLPGetNumberOfElements(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfElements";
  int result;
  int i,n;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=0;
  n=NLPGetNumberOfGroups(this);
  for(i=0;i<n;i++)result+=NLPGetNumberOfElementsInGroup(this,i);

  return(result);
 }

int NLPGetNumberOfElementsO(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfElementsO";
  int result;
  int i,n;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=0;
  n=NLPGetNumberOfGroupsInObjective(this);
  for(i=0;i<n;i++)result+=NLPGetNumberOfElementsInGroup(this,NLPGetObjectiveGroupNumber(this,i));

  return(result);
 }

int NLPGetObjectiveGroupNumber(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetObjectiveGroupNumber";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1 && group<this->nGroupsInObjective))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  return(this->groupsInObjective[group]);
 }

int NLPGetNumberOfElementsE(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfElementsE";
  int i,result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=0;
  for(i=0;i<NLPGetNumberOfEqualityConstraints(this);i++)
   result+=NLPGetNumberOfElementsInGroup(this,NLPGetEqualityConstraintGroupNumber(this,i));

  return(result);
 }

int NLPGetNumberOfElementsI(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfElementsI";
  int i,result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=0;
  for(i=0;i<NLPGetNumberOfInequalityConstraints(this);i++)
   result+=NLPGetNumberOfElementsInGroup(this,NLPGetInequalityConstraintGroupNumber(this,i));

  return(result);
 }

int NLPGetNumberOfElementTypes(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfElementType2";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nElementTypes);
 }

char *NLPGetElementType(NLProblem this,int element)
 {
  char RoutineName[]="NLPGetElementType";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(!(element>-1 && element<this->nElementTypes))
   {
    sprintf(NLProblemErrorMsg,"Element type %d is illegal (argument 2). Must be in range 0 to %d",element,this->nElementTypes-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  return(this->elementTypeName[element]);
 }

char *NLPGetElementTypeName(NLProblem this,int group, int element)
 {
  char RoutineName[]="NLPGetElementTypeName";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(!(group>-1 && group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  return(this->elementTypeName[NLEGetType(NLNEGetElementFunction(this,(this->element[group])[element]))]);
 }

int NLPGetTypeOfElement(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPGetTypeOfElement";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1 && group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return NLEGetType(NLNEGetElementFunction(this,(this->element[group])[element]));
 }

int NLPGetElementNumberOfUnknowns(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPGetElementNumberOfUnknowns";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1 && group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(NLNEGetElementDimension(this,(this->element[group])[element]));
 }

int NLPGetElementIndexIntoWhole(NLProblem this,int group,int element, int i)
 {
  char RoutineName[]="NLPGetElementIndexIntoWhole";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1 && group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(i>-1 && i<NLPGetElementNumberOfUnknowns(this,group,element)))
   {
    sprintf(NLProblemErrorMsg,"Element Internal Variable index %d is illegal (argument 4). Must be in range 0 to %d",element,NLPGetElementNumberOfUnknowns(this,group,element)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(NLNEGetIndex(this,(this->element[group])[element],i));
 }

int NLPGetNumberOfGroupsInObjective(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfGroupsInObjective";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nGroupsInObjective);
 }

int NLPGetNumberOfEqualityConstraints(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfEqualityConstraints";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!this->hideEqualityConstraints)return(this->nEqualityConstraints);
   else return 0;
 }

int NLPGetNumberOfInequalityConstraints(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfInequalityConstraints";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!this->hideInequalityConstraints)return(this->nInequalityConstraints);
   else return 0;
 }

double NLPGetLowerSimpleBound(NLProblem this,int variable)
 {
  char RoutineName[]="NLPGetLowerSimpleBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(variable>-1 && variable<NLPGetNumberOfVariables(this)))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",variable,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  return(this->simpleConstraintLowerBound[variable]); 
 }

double NLPGetUpperSimpleBound(NLProblem this,int variable)
 {
  char RoutineName[]="NLPGetUpperSimpleBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(variable>-1 && variable<NLPGetNumberOfVariables(this)))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",variable,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  return(this->simpleConstraintUpperBound[variable]); 
 }

int NLPSetInequalityConstraintLowerBound(NLProblem this,int constraint,double l)
 {
  char RoutineName[]="NLPSetInequalityConstraintLowerBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfInequalityConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfInequalityConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  this->inequalityConstraintLowerBound[constraint]=l; 
  return 1;
 }

int NLPSetInequalityConstraintUpperBound(NLProblem this,int constraint,double u)
 {
  char RoutineName[]="NLPSetInequalityConstraintUpperBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfInequalityConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfInequalityConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  this->inequalityConstraintUpperBound[constraint]=u; 
  return 1;
 }

int NLPSetInequalityConstraintBounds(NLProblem this,int constraint,double l,double u)
 {
  char RoutineName[]="NLPSetInequalityConstraintBounds";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfInequalityConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfInequalityConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  this->inequalityConstraintLowerBound[constraint]=l; 
  this->inequalityConstraintUpperBound[constraint]=u; 
  return 1;
 }

double NLPGetInequalityConstraintLowerBound(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetInequalityConstraintLowerBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfInequalityConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfInequalityConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  return(this->inequalityConstraintLowerBound[constraint]); 
 }

double NLPGetInequalityConstraintUpperBound(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetInequalityConstraintUpperBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 1;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfInequalityConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfInequalityConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  return(this->inequalityConstraintUpperBound[constraint]); 
 }

int NLPGetInequalityConstraintGroupNumber(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetInequalityConstraintGroupNumber";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(constraint>-1 && constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,this->nInequalityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return((this->inequalityConstraintGroups[constraint])[0]);
 }

int NLPGetEqualityConstraintGroupNumber(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetEqualityConstraintGroupNumber";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(constraint>-1 && constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Equality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,this->nEqualityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return((this->equalityConstraintGroups[constraint])[0]);
 }

char *NLPGetProblemName(NLProblem this)
 {
  char RoutineName[]="NLPGetProblemName";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  return(this->problemName);
 }

int NLPGetNumberOfVariables(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfVariables";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 1;
   }

  return(this->nVariables);
 }

int NLPAddGroupType(NLProblem this,char *label)
 {
  char RoutineName[]="NLPAddGroupType";
  int i;
  char **tmpgroupTypeName;
  static int verbose=0;

  if(verbose){printf("%s(%s)\n",RoutineName,label);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(label==(char*)NULL)return(-1);

  for(i=0;i<this->nGroupTypes;i++)
   if(!strcmp(label,this->groupTypeName[i]))return(i);

  if(this->nGroupTypes>=this->mGroupTypes)
   {
    if(this->mGroupTypes==-1)
     {
      this->mGroupTypes=NUMBEROFGROUPTYPESTOALLOCATE;
      this->groupTypeName=(char**)malloc(sizeof(char*)*this->mGroupTypes);
      if(this->groupTypeName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroupTypes*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->mGroupTypes;i++)
         this->groupTypeName[i]=(char*)NULL;
     }else{
      this->mGroupTypes+=NUMBEROFGROUPTYPESTOALLOCATE;
      tmpgroupTypeName=(char**)malloc(sizeof(char*)*this->mGroupTypes);
      if(tmpgroupTypeName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroupTypes*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->nGroupTypes;i++)
         tmpgroupTypeName[i]=this->groupTypeName[i];
      for(i=this->nGroupTypes;i<this->mGroupTypes;i++)
         tmpgroupTypeName[i]=(char*)NULL;
      free(this->groupTypeName);
      this->groupTypeName=tmpgroupTypeName;
     }
   }

  this->groupTypeName[this->nGroupTypes]=(char*)malloc(sizeof(char)*(strlen(label)+1));
  if(this->groupTypeName[this->nGroupTypes]==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(label)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
   }

  strcpy(this->groupTypeName[this->nGroupTypes],label);

  this->nGroupTypes++;

  return(this->nGroupTypes-1);
 }

int NLPAddElementType(NLProblem this,char *label,int rangeSet)
 {
  char RoutineName[]="NLPAddElementType";
  int i;
  char **tmpelementTypeName;
  int *tmpelementRangeSet;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(label==(char*)NULL)return(-1);

  for(i=0;i<this->nElementTypes;i++)
   if(!strcmp(label,this->elementTypeName[i]))
     return(i);

  if(this->nElementTypes>=this->mElementTypes)
   {
    if(this->mElementTypes==-1)
     {
      this->mElementTypes=NUMBEROFGROUPTYPESTOALLOCATE;
      this->elementTypeName=(char**)malloc(sizeof(char*)*this->mElementTypes);
      if(this->elementTypeName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementTypes*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->elementRangeSet=(int*)malloc(sizeof(int)*this->mElementTypes);
      if(this->elementRangeSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementTypes*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->mElementTypes;i++)
       {
        this->elementTypeName[i]=(char*)NULL;
        this->elementRangeSet[i]=0;
       }
     }else{
      this->mElementTypes+=NUMBEROFGROUPTYPESTOALLOCATE;
      tmpelementTypeName=(char**)malloc(sizeof(char*)*this->mElementTypes);
      if(tmpelementTypeName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementTypes*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpelementRangeSet=(int*)malloc(sizeof(int)*this->mElementTypes);
      if(tmpelementRangeSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mElementTypes*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->nElementTypes;i++)
       {
        tmpelementTypeName[i]=this->elementTypeName[i];
        tmpelementRangeSet[i]=this->elementRangeSet[i];
       }
      for(i=this->nElementTypes;i<this->mElementTypes;i++)
       {
        tmpelementTypeName[i]=(char*)NULL;
        tmpelementRangeSet[i]=0;
       }
      free(this->elementTypeName);
      free(this->elementRangeSet);
      this->elementTypeName=tmpelementTypeName;
      this->elementRangeSet=tmpelementRangeSet;
     }
   }

  this->elementTypeName[this->nElementTypes]=(char*)malloc(sizeof(char)*(strlen(label)+1));
  if(this->elementTypeName[this->nElementTypes]==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(label)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
   }

  strcpy(this->elementTypeName[this->nElementTypes],label);
  this->elementRangeSet[this->nElementTypes]=rangeSet;

  this->nElementTypes++;

  return(this->nElementTypes-1);
}

int NLPSetVariableScale(NLProblem this,int i,double scale)
 {
  char RoutineName[]="NLPSetVariableScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(i>-1 && i<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",i,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->variableScale[i]=scale;

  return 1;
 }

double NLPGetVariableScale(NLProblem this,int i)
 {
  char RoutineName[]="NLPGetVariableScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(i>-1 && i<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",i,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->variableScale[i]);
 }

int NLPSetVariableName(NLProblem this,int i,char *name)
 {
  char RoutineName[]="NLPSetVariableName";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(i>-1 && i<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",i,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  if(!(name!=(char*)NULL))
   {
    sprintf(NLProblemErrorMsg,"The pointer to the variable name (argument 3) is NULL.");
    NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(this->variableName[i]!=(char*)NULL)free(this->variableName[i]);
  this->variableName[i]=(char*)malloc(sizeof(char)*(strlen(name)+1));
  if(this->variableName[i]==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
   }
  strcpy(this->variableName[i],name);

  return 1;
 }

char *NLPGetVariableName(NLProblem this,int i)
 {
  char RoutineName[]="NLPGetVariableName";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(!(i>-1 && i<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",i,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  return(this->variableName[i]);
 }

/* This is currently disabled

int NLPSetObjectiveLowerBound(NLProblem this,double l)
 {
  char RoutineName[]="NLPSetObjectiveLowerBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->lowerBoundOnObjective=l;
  return 1;
 }
*/

double NLPGetObjectiveLowerBound(NLProblem this)
 {
  char RoutineName[]="NLPGetObjectiveLowerBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->lowerBoundOnObjective);
 }

/* This is currently disabled

int NLPSetObjectiveUpperBound(NLProblem this,double r)
 {
  char RoutineName[]="NLPSetObjectiveUpperBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->upperBoundOnObjective=r;

  return 1;
 }
*/

double NLPGetObjectiveUpperBound(NLProblem this)
 {
  char RoutineName[]="NLPGetObjectiveUpperBound";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->upperBoundOnObjective);
 }

/* This is currently disabled

int NLPSetObjectiveBounds(NLProblem this,double l,double r)
 {
  char RoutineName[]="NLPSetObjectiveBounds";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->lowerBoundOnObjective=l;
  this->upperBoundOnObjective=r;

  return 1;
 }
*/

int NLPSetGroupScale(NLProblem this,int group,double s)
 {
  char RoutineName[]="NLPSetGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->groupScale[group]=1./s;
  return 1;
 }

double NLPGetGroupScale(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(1./this->groupScale[group]);
 }

int NLPIsGroupFunctionSet(NLProblem this,int group)
 {
  char RoutineName[]="NLPIsGroupFunctionSet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->groupFunction[group]!=(NLGroupFunction)NULL);
 }

int NLPIsGroupASet(NLProblem this,int group)
 {
  char RoutineName[]="NLPIsGroupASet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->groupA[group]!=(NLVector)NULL);
 }

int NLPIsGroupBSet(NLProblem this,int group)
 {
  char RoutineName[]="NLPIsGroupBSet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->groupBSet[group]);
 }

int NLPIsElementFunctionSet(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPIsElementFunctionSet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(NLNEGetElementFunction(this,(this->element[group])[element])!=(NLElementFunction)NULL);
 }

int NLPIsElementWeightSet(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPIsElementWeightSet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return((this->elementWeightSet[group])[element]);
 }

int NLPIsUpperSimpleBoundSet(NLProblem this,int i)
 {
  char RoutineName[]="NLPIsUpperSimpleBoundSet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(i>-1&&i<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",i,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->simpleConstraintUpperBound[i]!=DBL_MAX);
 }

int NLPIsLowerSimpleBoundSet(NLProblem this,int i)
 {
  char RoutineName[]="NLPIsLowerSimpleBoundSet";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(i>-1&&i<this->nVariables))
   {
    sprintf(NLProblemErrorMsg,"Variable number %d (argument 2) is illegal. Must be in range 0 to %d",i,this->nVariables-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->simpleConstraintLowerBound[i]!=-DBL_MAX);
 }

int NLPSetElementWeight(NLProblem this,int group,int element,double w)
 {
  char RoutineName[]="NLPSetElementWeight";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  if(element<0 || element>=NLPGetNumberOfElementsInGroup(this,group))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  (this->elementWeight[group])[element]=w;
  (this->elementWeightSet[group])[element]=TRUE;

  return 1;
 }

char *NLPGetGroupName(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetGroupName";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  return(this->groupName[group]);
 }

void NLPrintProblem(FILE *fid,NLProblem P)
 {
  char RoutineName[]="NLPrintProblem";
  int i,I,j,J,k;
  double s;

  if(fid==(FILE*)NULL)
   {
    sprintf(NLProblemErrorMsg,"File pointer (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(P==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  fprintf(fid,"NAGroupPartialSeparableNonlinearOptimizationProblem %s\n",NLPGetProblemName(P));
  fprintf(fid,"\nVariables: %d\n\n",NLPGetNumberOfVariables(P));
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(NLPGetVariableName(P,i)!=(char*)NULL)
      fprintf(fid," %d  name=\"%s\" scale %lf",i,NLPGetVariableName(P,i),NLPGetVariableScale(P,i));
     else
      fprintf(fid," %d  name=\"X%d\" scale %lf",i,i+1,NLPGetVariableName(P,i),NLPGetVariableScale(P,i));
    if((NLPIsLowerSimpleBoundSet(P,i)&&NLPGetLowerSimpleBound(P,i)>-1.e20)||NLPIsUpperSimpleBoundSet(P,i))
     {
      if(NLPIsLowerSimpleBoundSet(P,i) && NLPGetLowerSimpleBound(P,i)>-1.e20)fprintf(fid,"    %lf <=",NLPGetLowerSimpleBound(P,i));
      fprintf(fid,"x[%d]",i);
      if(NLPIsUpperSimpleBoundSet(P,i))fprintf(fid," <= %lf",NLPGetUpperSimpleBound(P,i));
      fprintf(fid,"\n");
     }else 
      fprintf(fid,"\n");
   }

  fprintf(fid,"\nObjective Function: \n\n");
  fprintf(fid,"  Bounds: [");
  if(NLPGetObjectiveLowerBound(P)!=DBL_MIN)
    fprintf(fid,"%lf",NLPGetObjectiveLowerBound(P));
   else
    fprintf(fid,"-infinity");
  fprintf(fid,",");
  if(NLPGetObjectiveUpperBound(P)!=DBL_MAX)
    fprintf(fid,"%lf",NLPGetObjectiveUpperBound(P));
   else
    fprintf(fid,"infinity");
  fprintf(fid,"]\n");
  fprintf(fid,"  Number Of Groups: %d\n",NLPGetNumberOfGroupsInObjective(P));
  for(I=0;I<NLPGetNumberOfGroupsInObjective(P);I++)
   {
    i=NLPGetObjectiveGroupNumber(P,I);
    fprintf(fid,"   Group: %d number %d\n",I,i);
    NLPrintGroup(fid,P,i);
    fprintf(fid,"\n");
   }

  fprintf(fid,"\nConstraints: \n\n");

  fprintf(fid,"Equality Constraints: %d\n\n",NLPGetNumberOfEqualityConstraints(P));
  for(I=0;I<NLPGetNumberOfEqualityConstraints(P);I++)
   {
    fprintf(fid,"  Equality Constraint: %d, %d Groups:\n",I,NLPGetNumberOfEqualityConstraintGroups(P,I));
    for(J=0;J<NLPGetNumberOfEqualityConstraintGroups(P,I);J++)
     {
      i=NLPGetEqualityConstraintNumberOfGroup(P,I,J);
      fprintf(fid,"   Group: %d number %d\n",J,i);
      NLPrintGroup(fid,P,i);
      fprintf(fid,"\n");
     }
   }
  if(NLPGetNumberOfEqualityConstraints(P)>0)fprintf(fid,"\n");

  fprintf(fid,"Inequality Constraints: %d\n\n",NLPGetNumberOfInequalityConstraints(P));
  for(I=0;I<NLPGetNumberOfInequalityConstraints(P);I++)
   {
    fprintf(fid,"  Inequality Constraint: %d, %d Groups:\n",I,NLPGetNumberOfInequalityConstraintGroups(P,I));
    for(J=0;J<NLPGetNumberOfInequalityConstraintGroups(P,I);J++)
     {
      i=NLPGetInequalityConstraintNumberOfGroup(P,I,J);
      fprintf(fid,"   Group: %d number %d\n",J,i);
      NLPrintGroup(fid,P,i);
      s=1./NLPGetGroupScale(P,i);
      fprintf(fid,"\n");
     }

    fprintf(fid,"       Bounds: [");
    if(NLPIsInequalityConstraintLowerBoundSet(P,I))
      fprintf(fid,"%lf",NLPGetInequalityConstraintLowerBound(P,I)*s);
     else
      fprintf(fid,"-infinity");
    fprintf(fid,",");
    if(NLPIsInequalityConstraintUpperBoundSet(P,I))
      fprintf(fid,"%lf",NLPGetInequalityConstraintUpperBound(P,I)*s);
     else
      fprintf(fid,"infinity");
    fprintf(fid,"]\n");
    fprintf(fid,"\n");
   }
  if(NLPGetNumberOfInequalityConstraints(P)>0)fprintf(fid,"\n");

  fprintf(fid,"MinMax Constraints: %d\n\n",NLPGetNumberOfMinMaxConstraints(P));
  for(I=0;I<NLPGetNumberOfMinMaxConstraints(P);I++)
   {
    fprintf(fid,"      MinMax Constraint: %d, %d Groups:\n",I,NLPGetNumberOfMinMaxConstraintGroups(P,I));
    for(J=0;J<NLPGetNumberOfMinMaxConstraintGroups(P,I);J++)
     {
      i=NLPGetMinMaxConstraintNumberOfGroup(P,I,J);
      fprintf(fid,"   Group: %d number %d\n",J,i);
      NLPrintGroup(fid,P,i);
      fprintf(fid,"\n");
     }
   }
  if(NLPGetNumberOfMinMaxConstraints(P)>0)fprintf(fid,"\n");

  fprintf(fid,"Groups: \n\n",NLPGetNumberOfGroups(P));

  for(i=0;i<NLPGetNumberOfGroups(P);i++)
   {
    fprintf(fid," Group %d\n",i);
    NLPrintGroup(fid,P,i);
   }

  fprintf(fid,"Nonlinear Elements: \n\n",NLPGetNumberOfNonlinearElements(P));

  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
   {
    fprintf(fid," Nonlinear Element %d",i);
    NLPrintNonlinearElement(fid,P,i);
   }

  fprintf(fid," Summary: \n\n");
  fprintf(fid," Total Number of Groups: %d\n",NLPGetNumberOfGroups(P));
  for(i=0;i<NLPGetNumberOfGroups(P);i++)
   {
    if(NLPGetTypeOfGroup(P,i)>-1)
      fprintf(fid,"  Group %d type %d (%s)\n",i,NLPGetTypeOfGroup(P,i),NLPGetGroupType(P,NLPGetTypeOfGroup(P,i)));
     else
      fprintf(fid,"  Group %d no type ",i);
   }
  fprintf(fid,"\n Total Number of Group Functions: %d\n",NLPGetNumberOfGroupTypes(P));
  for(i=0;i<NLPGetNumberOfGroupTypes(P);i++)
   {
    fprintf(fid,"  Group Function %d %s\n",i,NLPGetGroupType(P,i));
   }
  fprintf(fid,"\n Total Number of Nonlinear Elements: %d\n",NLPGetNumberOfNonlinearElements(P));
  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
   {
    fprintf(fid,"  Nonlinear Element %d %s\n",i,NLNEGetName(P,i));
   }

  fprintf(fid,"\n Total Number of Element Functions: %d\n",NLPGetNumberOfElementTypes(P));
  for(i=0;i<NLPGetNumberOfElementTypes(P);i++)
   {
    fprintf(fid,"  Element Function %d %s\n",i,NLPGetElementType(P,i));
   }

  fflush(stdout);
  return;
 }

NLMatrix NLPGetElementRangeTransformationOfGroup(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPGetElementRangeTransformationOfGroup";
  int i,n;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLMatrix)NULL;
   }
  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return NNLEGetRangeXForm(this,(this->element[group])[element]);
 }

NLMatrix NLPGetElementRangeTransformation(NLProblem this,int element)
 {
  char RoutineName[]="NLPGetElementRangeTransformation";
  int group,n;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLMatrix)NULL;
   }

  if(!(element>-1&&element<NLPGetNumberOfElements(this)))
   {
    sprintf(NLProblemErrorMsg,"The global element index %d (argument 2) is illegal, must be in range 0 to %d.",element,NLPGetNumberOfElements(this)-1);
    NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLMatrix)NULL;
   }

  return NNLEGetRangeXForm(this,element);
 }

int NLPGetNumberOfInternalVariablesInElement(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPGetNumberOfInternalVariablesInElement";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1 && group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1 && element<NLPGetNumberOfElementsInGroup(this,group)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementsInGroup(this,group)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return NLNEGetInternalDimension(this,(this->element[group])[element]);
 }

int NLAddGroup(NLProblem this,char *name,char *type)
 {
  char RoutineName[]="NLAddGroup";
  int i;
  int *tmpgroupType;
  char **tmpgroupName;
  NLGroupFunction *tmpgroupFunction;
  NLVector *tmpgroupA;
  double *tmpgroupB;
  int *tmpgroupBSet;
  double *tmpgroupScale;
  int *tmpnElementsInGroup;
  int *tmpmElementsInGroup;
  double **tmpelementWeight;
  int **tmpelementWeightSet;
  int **tmpelement;
  int **tmpelementType;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(this->nGroups>=this->mGroups)
   {
    if(this->mGroups==-1)
     {
      this->mGroups=NUMBEROFGROUPSTOALLOCATE;
      this->groupName=(char**)malloc(sizeof(char*)*this->mGroups);
      if(this->groupName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->groupFunction=(NLGroupFunction*)malloc(sizeof(NLGroupFunction)*this->mGroups);
      if(this->groupFunction==(NLGroupFunction*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLGroupFunction));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->groupA=(NLVector*)malloc(sizeof(NLVector)*this->mGroups);
      if(this->groupA==(NLVector*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLVector));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->groupB=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupB==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->groupBSet=(int*)malloc(sizeof(int)*this->mGroups);
      if(this->groupBSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->groupScale=(double*)malloc(sizeof(double)*this->mGroups);
      if(this->groupScale==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->nElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(this->nElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->mElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(this->mElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->elementWeight=(double**)malloc(sizeof(double*)*this->mGroups);
      if(this->elementWeight==(double**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->elementWeightSet=(int**)malloc(sizeof(int*)*this->mGroups);
      if(this->elementWeightSet==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      this->element=(int**)malloc(sizeof(int*)*this->mGroups);
      if(this->element==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      for(i=0;i<this->mGroups;i++)
       {
        this->groupName[i]=(char*)NULL;
        this->groupFunction[i]=(NLGroupFunction)NULL;
        this->groupA[i]=(NLVector)NULL;
        this->groupB[i]=0.;
        this->groupBSet[i]=FALSE;
        this->groupScale[i]=1.;
        this->nElementsInGroup[i]=0;
        this->mElementsInGroup[i]=-1;
        this->elementWeight[i]=(double*)NULL;
        this->elementWeightSet[i]=(int*)NULL;
        this->element[i]=(int*)NULL;
       }
     }else{
      this->mGroups+=NUMBEROFGROUPSTOALLOCATE;
      tmpgroupType=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpgroupType==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpgroupName=(char**)malloc(sizeof(char*)*this->mGroups);
      if(tmpgroupName==(char**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(char*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpgroupFunction=(NLGroupFunction*)malloc(sizeof(NLGroupFunction)*this->mGroups);
      if(tmpgroupFunction==(NLGroupFunction*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLGroupFunction));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpgroupA=(NLVector*)malloc(sizeof(NLVector)*this->mGroups);
      if(tmpgroupA==(NLVector*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLVector));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpgroupB=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupB==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpgroupBSet=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpgroupBSet==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpgroupScale=(double*)malloc(sizeof(double)*this->mGroups);
      if(tmpgroupScale==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpnElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpnElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpmElementsInGroup=(int*)malloc(sizeof(int)*this->mGroups);
      if(tmpmElementsInGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpelementWeight=(double**)malloc(sizeof(double*)*this->mGroups);
      if(tmpelementWeight==(double**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpelementWeightSet=(int**)malloc(sizeof(int*)*this->mGroups);
      if(tmpelementWeightSet==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(int*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      tmpelement=(int**)malloc(sizeof(int*)*this->mGroups);
      if(tmpelement==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mGroups*sizeof(NLElementFunction*));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }
      for(i=0;i<this->nGroups;i++)
       {
        tmpgroupName[i]=this->groupName[i];
        tmpgroupFunction[i]=this->groupFunction[i];
        tmpgroupA[i]=this->groupA[i];
        tmpgroupB[i]=this->groupB[i];
        tmpgroupBSet[i]=this->groupBSet[i];
        tmpgroupScale[i]=this->groupScale[i];
        tmpnElementsInGroup[i]=this->nElementsInGroup[i];
        tmpmElementsInGroup[i]=this->mElementsInGroup[i];
        tmpelementWeight[i]=this->elementWeight[i];
        tmpelementWeightSet[i]=this->elementWeightSet[i];
        tmpelement[i]=this->element[i];
       }
      for(i=this->nGroups;i<this->mGroups;i++)
       {
        tmpgroupType[i]=-1;
        tmpgroupName[i]=(char*)NULL;
        tmpgroupFunction[i]=(NLGroupFunction)NULL;
        tmpgroupA[i]=(NLVector)NULL;
        tmpgroupB[i]=0.;
        tmpgroupBSet[i]=FALSE;
        tmpgroupScale[i]=1.;
        tmpnElementsInGroup[i]=0;
        tmpmElementsInGroup[i]=-1;
        tmpelementWeight[i]=(double*)NULL;
        tmpelementWeightSet[i]=(int*)NULL;
        tmpelement[i]=(int*)NULL;
       }
      free(this->groupName);
      free(this->groupFunction);
      free(this->groupA);
      free(this->groupB);
      free(this->groupBSet);
      free(this->groupScale);
      free(this->nElementsInGroup);
      free(this->mElementsInGroup);
      free(this->elementWeight);
      free(this->elementWeightSet);
      free(this->element);
      this->groupName=tmpgroupName;
      this->groupFunction=tmpgroupFunction;
      this->groupA=tmpgroupA;
      this->groupB=tmpgroupB;
      this->groupBSet=tmpgroupBSet;
      this->groupScale=tmpgroupScale;
      this->nElementsInGroup=tmpnElementsInGroup;
      this->mElementsInGroup=tmpmElementsInGroup;
      this->elementWeight=tmpelementWeight;
      this->elementWeightSet=tmpelementWeightSet;
      this->element=tmpelement;
     }
   }

  for(i=0;i<this->nGroups;i++)
   {
    if(!strcmp(this->groupName[i],name))
     {
      fprintf(stderr,"Lancelot API -- Warning! You are creating a group named %s which is the same name given to group %d. I don't care, but someone else might.\n",name,i);
     }
   }
  if(name!=(char*)NULL)
   {
    this->groupName[this->nGroups]=(char*)malloc(sizeof(char)*(strlen(name)+1));
    if(this->groupName[this->nGroups]==(char*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+1)*sizeof(char));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
     }
    strcpy(this->groupName[this->nGroups],name);
   }else
    this->groupName[this->nGroups]=(char*)NULL;

  this->groupFunction[this->nGroups]=(NLGroupFunction)NULL;
  this->groupA[this->nGroups]=(NLVector)NULL;
  this->groupB[this->nGroups]=0.;
  this->groupBSet[this->nGroups]=FALSE;
  this->groupScale[this->nGroups]=1.;
  this->nElementsInGroup[this->nGroups]=0;
  this->mElementsInGroup[this->nGroups]=-1;
  this->elementWeight[this->nGroups]=(double*)NULL;
  this->elementWeightSet[this->nGroups]=(int*)NULL;
  this->element[this->nGroups]=(int*)NULL;

  this->nGroups++;

  return(this->nGroups-1);
 }

int NLPAddMinMaxConstraint(NLProblem this,char *name)
 {
  char RoutineName[]="NLPAddMinMaxConstraint";
  int i;
  int result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(NLPInternalExtendIndirectArrays(&(this->nMinMaxConstraints),
                                     &(this->mMinMaxConstraints),
                                     &(this->minMaxConstraintGroups),(int*)NULL,
                                     &(this->nMinMaxConstraintGroups),
                                     &(this->mMinMaxConstraintGroups),
                                     (double**)NULL,0.,
                                     (double**)NULL,0.,
                                     NUMBEROFEQUALITYCONSTRAINTSTOALLOCATE)!=0)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to add an MinMax Constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
   }

  this->nMinMaxConstraintGroups[this->nMinMaxConstraints]=0;
  this->mMinMaxConstraintGroups[this->nMinMaxConstraints]=-1;
  this->minMaxConstraintGroups[this->nMinMaxConstraints]=(int*)NULL;
  result=this->nMinMaxConstraints;

  this->nMinMaxConstraints++;

  if(name!=(char*)NULL)NLPAddGroupToMinMaxConstraint(this,result,name);
  return(result);
 }

int NLPGetNumberOfMinMaxConstraints(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfMinMaxConstraints";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!this->hideMinMaxConstraints)return(this->nMinMaxConstraints);
   else return 0;
 }

int NLPGetMinMaxConstraintGroupNumber(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetMinMaxConstraintGroupNumber";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfMinMaxConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"InminMax constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfMinMaxConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return((this->minMaxConstraintGroups[constraint])[0]);
 }

int NLPGetNumberOfElementsM(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfElementsM";
  int i,result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=0;
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(this);i++)
   result+=NLPGetNumberOfElementsInGroup(this,NLPGetMinMaxConstraintGroupNumber(this,i));

  return(result);
 }

int NLPSetMinMaxBounds(NLProblem this,double l,double r)
 {
  char RoutineName[]="NLPSetMinMaxBounds";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->lowerBoundOnMinMaxVar=l;
  this->upperBoundOnMinMaxVar=r;
  return 1;
 }

int NLPSetLowerMinMaxBound(NLProblem this,double l)
 {
  char RoutineName[]="NLPSetLowerMinMaxBound";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->lowerBoundOnMinMaxVar=l;
  return 1;
 }

double NLPGetLowerMinMaxBound(NLProblem this)
 {
  char RoutineName[]="NLPGetLowerMinMaxBound";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->lowerBoundOnMinMaxVar;
 }

int NLPSetUpperMinMaxBound(NLProblem this,double r)
 {
  char RoutineName[]="NLPSetUpperMinMaxBound";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->upperBoundOnMinMaxVar=r;
  return 1;
 }

double NLPGetUpperMinMaxBound(NLProblem this)
 {
  char RoutineName[]="NLPGetUpperMinMaxBound";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->upperBoundOnMinMaxVar;
 }

int NLPGetNumberOfNonlinearElements(NLProblem this)
 {
  char RoutineName[]="NLPGetNumberOfNonlinearElements";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->nNonlinearElements;
 }

NLNonlinearElementPtr NLPGetNonlinearElement(NLProblem this,int e)
 {
  char RoutineName[]="NLPGetNonlinearElement";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->nonlinearElement[e];
 }

int NLPAddNonlinearElement(NLProblem this,NLNonlinearElementPtr E)
 {
  char RoutineName[]="NLPAddNonlinearElement";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(this->nNonlinearElements>=this->mNonlinearElements)
   {
    this->mNonlinearElements+=100;
    this->nonlinearElement=(NLNonlinearElementPtr*)realloc((void*)this->nonlinearElement,this->mNonlinearElements*sizeof(NLNonlinearElementPtr));
    if(this->nonlinearElement==(NLNonlinearElementPtr*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(NLNonlinearElementPtr));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

    this->elementCached=(int*)realloc((void*)this->elementCached,this->mNonlinearElements*sizeof(int));
    if(this->elementCached==(int*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

    this->internalVariables=(double**)realloc((void*)this->internalVariables,this->mNonlinearElements*sizeof(double*));
    if(this->internalVariables==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

    this->elementVariables=(double**)realloc((void*)this->elementVariables,this->mNonlinearElements*sizeof(double*));
    if(this->elementVariables==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

    this->elementValue=(double*)realloc((void*)this->elementValue,this->mNonlinearElements*sizeof(double));
    if(this->elementValue==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

    this->elementGradient=(double**)realloc((void*)this->elementGradient,this->mNonlinearElements*sizeof(double*));
    if(this->elementGradient==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

    this->elementHessian=(double**)realloc((void*)this->elementHessian,this->mNonlinearElements*sizeof(double*));
    if(this->elementHessian==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonlinearElements*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return -1;
     }

   }
  this->nonlinearElement[this->nNonlinearElements]=E;
  NLRefNonlinearElement(this->nonlinearElement[this->nNonlinearElements]);
  this->elementCached[this->nNonlinearElements]=0;
  this->internalVariables[this->nNonlinearElements]=(double*)NULL;
  this->elementVariables[this->nNonlinearElements]=(double*)NULL;
  this->elementValue[this->nNonlinearElements]=0.;
  this->elementGradient[this->nNonlinearElements]=(double*)NULL;
  this->elementHessian[this->nNonlinearElements]=(double*)NULL;
  this->nNonlinearElements++;
  return this->nNonlinearElements-1;
 }

int NLPAddNonlinearElementToObjectiveGroup(NLProblem this,int group,double weight,NLNonlinearElement E)
 {
  char RoutineName[]="NLPAddNonlinearElementToObjective";

  group=NLPGetObjectiveGroupNumber(this,group);
  return NLPAddNonlinearElementToGroup(this,group,weight,E);
 }

int NLPAddNonlinearElementToInequalityConstraint(NLProblem this,int constraint,double weight,NLNonlinearElement E)
 {
  char RoutineName[]="NLPAddNonlinearElementToInequalityConstraint";
  int group;

  group=NLPGetInequalityConstraintGroupNumber(this,constraint);
  return NLPAddNonlinearElementToGroup(this,group,weight,E);
 }

int NLPAddNonlinearElementToEqualityConstraint(NLProblem this,int constraint,double weight,NLNonlinearElement E)
 {
  char RoutineName[]="NLPAddNonlinearElementToEqualityConstraint";
  int group;

  group=NLPGetEqualityConstraintGroupNumber(this,constraint);
  return NLPAddNonlinearElementToGroup(this,group,weight,E);
 }

int NLPAddNonlinearElementToMinMaxConstraint(NLProblem this,int constraint,double weight,NLNonlinearElement E)
 {
  char RoutineName[]="NLPAddNonlinearElementToMinMaxConstraint";
  int group;

  group=NLPGetMinMaxConstraintGroupNumber(this,constraint);
  return NLPAddNonlinearElementToGroup(this,group,weight,E);
 }

void NLPrintNonlinearElement(FILE *fid,NLProblem P,NLNonlinearElement i)
 {
  int k;
  int j,n;
  double *H0;

  fprintf(fid,"\n");
  fprintf(fid,"          Name: %s\n",NLNEGetName(P,i));
  fprintf(fid,"          Element Function: ");
  if(NLNEGetElementFunction(P,i)!=(NLElementFunction)NULL)
    fprintf(fid,"type %d (%s)\n",NLEGetType(NLNEGetElementFunction(P,i)),NLPGetElementType(P,NLEGetType(NLNEGetElementFunction(P,i))));
   else
    fprintf(fid," default\n");
  fprintf(fid,"          Number of Element Variables: %d\n",NLNEGetElementDimension(P,i));
  fprintf(fid,"          Number of Internal Variables: %d\n",NLNEGetInternalDimension(P,i));
  fprintf(fid,"          Number of Unknowns: %d\n",NLNEGetElementDimension(P,i));
  fprintf(fid,"          Unknowns: (");
  for(k=0;k<NLNEGetElementDimension(P,i);k++)
   {
    if(k>0)fprintf(fid,",");
    fprintf(fid,"%d",NLNEGetIndex(P,i,k));
   }
  fprintf(fid,")  (");
  for(k=0;k<NLNEGetElementDimension(P,i);k++)
   {
    if(k>0)fprintf(fid,",");
    fprintf(fid,"%s",NLPGetVariableName(P,NLNEGetIndex(P,i,k)));
   }
  fprintf(fid,")\n");
  fprintf(fid,"          Range Transformation:");
  if(NNLEGetRangeXForm(P,i)!=(NLMatrix)NULL)
   {
    fprintf(fid,"\n");
    NLPrintMatrix(fid,NNLEGetRangeXForm(P,i));
   }else
    fprintf(fid," Identity\n");
  H0=NLElementFunctionGetInitialHessianMatrix(NLNEGetElementFunction(P,i));
  if(H0!=(double*)NULL)
   {
    fprintf(fid,"          Initial Hessian\n");
    n=NLNEGetInternalDimension(P,i);
    for(j=0;j<n;j++)
     {
      fprintf(fid,"          [");
      for(k=0;k<n;k++)
       {
        if(k>0)fprintf(fid,",");
        fprintf(fid,"%lf",H0[j+n*k]);
       }
      fprintf(fid,"]\n");
     }
   }else
    fprintf(fid,"          No Initial Hessian\n");

  fprintf(fid,"\n");
  return;
 }

NLNonlinearElementPtr NLPGetNonlinearElementPtrOfGroup(NLProblem this,int group, int element)
 {
  char RoutineName[]="NLPGetNonlinearElementPtrOfGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLNonlinearElementPtr)NULL;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLNonlinearElementPtr)NULL;
   }
  if(!(element>-1&&element<this->nElementsInGroup[group]))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,this->nElementsInGroup[group]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLNonlinearElementPtr)NULL;
   }

  return(NLPGetNonlinearElement(this,(this->element[group])[element]));
 }

NLNonlinearElement NLPGetNonlinearElementOfGroup(NLProblem this,int group, int element)
 {
  char RoutineName[]="NLPGetNonlinearElementOfGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }
  if(!(element>-1&&element<this->nElementsInGroup[group]))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,this->nElementsInGroup[group]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return((this->element[group])[element]);
 }

NLNonlinearElement NLPGetGroupNonlinearElement(NLProblem this,int group,int element)
 {
  char RoutineName[]="NLPGetGroupNonlinearElement";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(!(element>-1&&element<this->nElementsInGroup[group]))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,this->nElementsInGroup[group]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return((this->element[group])[element]);
 }

int NLPIsElementRangeSet(NLProblem this,int element)
 {
  char RoutineName[]="NLPIsElementRangeSet";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(element>-1 && element<NLPGetNumberOfElementTypes(this)))
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 3). Must be in range 0 to %d",element,NLPGetNumberOfElementTypes(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return(this->elementRangeSet[element]);
 }

int NLPSetObjectiveGroupFunction(NLProblem this,int group,NLGroupFunction g)
 {
  char RoutineName[]="NLPSetObjectiveGroupFunction";
  int verbose=0;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetObjectiveGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }
  if(!(group>-1&&group<this->nGroupsInObjective))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefGroupFunction(g);
  group=NLPGetObjectiveGroupNumber(this,group);
  if(this->groupFunction[group]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[group]);
  this->groupFunction[group]=g;

  return 1;
 }

int NLPSetObjectiveGroupA(NLProblem this,int group,NLVector a)
 {
  char RoutineName[]="NLPSetObjectiveGroupA";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroupsInObjective))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefVector(a);
  group=NLPGetObjectiveGroupNumber(this,group);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetEqualityConstraintA(NLProblem this,int constraint,NLVector a)
 {
  char RoutineName[]="NLPSetEqualityConstraint";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefVector(a);
  group=NLPGetEqualityConstraintGroupNumber(this,constraint);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetInequalityConstraintA(NLProblem this,int constraint,NLVector a)
 {
  char RoutineName[]="NLPSetInequalityConstraint";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",constraint,this->nInequalityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefVector(a);
  group=NLPGetInequalityConstraintGroupNumber(this,constraint);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetMinMaxConstraintA(NLProblem this,int constraint,NLVector a)
 {
  char RoutineName[]="NLPSetMinMaxConstraint";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nMinMaxConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",constraint,this->nMinMaxConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefVector(a);
  group=NLPGetMinMaxConstraintGroupNumber(this,constraint);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetObjectiveGroupB(NLProblem this,int group,double b)
 {
  char RoutineName[]="NLPSetObjectiveGroupB";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroupsInObjective))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetObjectiveGroupNumber(this,group);
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPSetEqualityConstraintB(NLProblem this,int constraint,double b)
 {
  char RoutineName[]="NLPSetEqualityConstraintB";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetEqualityConstraintGroupNumber(this,constraint);
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPSetInequalityConstraintB(NLProblem this,int constraint,double b)
 {
  char RoutineName[]="NLPSetInequalityConstraintB";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetInequalityConstraintGroupNumber(this,constraint);
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPSetMinMaxConstraintB(NLProblem this,int constraint,double b)
 {
  char RoutineName[]="NLPSetMinMaxConstraintB";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nMinMaxConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetMinMaxConstraintGroupNumber(this,constraint);
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

void NLPrintGroup(FILE *fid,NLProblem P,int group)
 {
  int j;

  fprintf(fid,"      Name: %s\n",NLPGetGroupName(P,group));
  fprintf(fid,"      Scale: %lf\n",1./NLPGetGroupScale(P,group));
  fprintf(fid,"      Group Function: ");
  fprintf(fid,"%d %s\n",NLPGetTypeOfGroup(P,group),NLPGetGroupType(P,NLPGetTypeOfGroup(P,group)));
  fprintf(fid,"      Element: ");NLPrintElement(fid,P,group);printf("\n");fflush(stdout);

  return;
 }

int NLPSetObjectiveGroupScale(NLProblem this,int group,double s)
 {
  char RoutineName[]="NLPSetObjectiveGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroupsInObjective))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetObjectiveGroupNumber(this,group);
  this->groupScale[group]=1./s;
  return 1;
 }

int NLPSetEqualityConstraintScale(NLProblem this,int constraint,double s)
 {
  char RoutineName[]="NLPSetEqualityConstraintScale";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",constraint,this->nEqualityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetEqualityConstraintGroupNumber(this,constraint);
  this->groupScale[group]=1./s;
  return 1;
 }

int NLPSetInequalityConstraintScale(NLProblem this,int constraint,double s)
 {
  char RoutineName[]="NLPSetInequalityConstraintScale";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",constraint,this->nInequalityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetInequalityConstraintGroupNumber(this,constraint);
  this->groupScale[group]=1./s;
  return 1;
 }

int NLPSetMinMaxConstraintScale(NLProblem this,int constraint,double s)
 {
  char RoutineName[]="NLPSetMinMaxConstraintScale";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(constraint>-1&&constraint<this->nMinMaxConstraints))
   {
    sprintf(NLProblemErrorMsg,"Constraint %d is illegal (argument 2). Must be in range 0 to %d",constraint,this->nMinMaxConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=NLPGetMinMaxConstraintGroupNumber(this,constraint);
  this->groupScale[group]=1./s;
  return 1;
 }

int NLPAddLinearEqualityConstraintV(NLProblem this,char *name,NLVector a,double b)
 {
  int constraint;

  constraint=NLPAddNonlinearEqualityConstraint(this,name);
  NLPSetEqualityConstraintA(this,constraint,a);
  NLPSetEqualityConstraintB(this,constraint,b);

  return constraint;
 }

int NLPAddLinearInequalityConstraintV(NLProblem this,char *name,NLVector a,double b)
 {
  int constraint;

  constraint=NLPAddNonlinearInequalityConstraint(this,name);
  NLPSetInequalityConstraintA(this,constraint,a);
  NLPSetInequalityConstraintB(this,constraint,b);

  return constraint;
 }

int NLPAddLinearMinMaxConstraintV(NLProblem this,char *name,NLVector a,double b)
 {
  int constraint;

  constraint=NLPAddMinMaxConstraint(this,name);
  NLPSetMinMaxConstraintA(this,constraint,a);
  NLPSetMinMaxConstraintB(this,constraint,b);

  return constraint;
 }

int NLPAddLinearMinMaxConstraint(NLProblem this,char *name,double *a,double b)
 {
  int constraint;
  NLVector va;

  constraint=NLPAddMinMaxConstraint(this,name);
  va=NLCreateVectorWithFullData(this->nVariables,a);
  NLPSetMinMaxConstraintA(this,constraint,va);
  NLFreeVector(va);
  NLPSetMinMaxConstraintB(this,constraint,b);

  return constraint;
 }

int NLPAddLinearEqualityConstraint(NLProblem this,char *name,double *a, double b)
 {
  int constraint;
  NLVector va;

  constraint=NLPAddNonlinearEqualityConstraint(this,name);
  va=NLCreateVectorWithFullData(this->nVariables,a);
  NLPSetEqualityConstraintA(this,constraint,va);
  NLFreeVector(va);
  NLPSetEqualityConstraintB(this,constraint,b);

  return constraint;
 }

int NLPAddLinearInequalityConstraint(NLProblem this,char *name, double *a, double b)
 {
  int constraint;
  NLVector va;

  constraint=NLPAddNonlinearInequalityConstraint(this,name);
  va=NLCreateVectorWithFullData(this->nVariables,a);
  NLPSetInequalityConstraintA(this,constraint,va);
  NLFreeVector(va);
  NLPSetInequalityConstraintB(this,constraint,b);

  return constraint;
 }

int NLPGetZGroupNumber(NLProblem this)
 {
  return this->zgroupnumber;
 }

void NLPSetZGroupNumber(NLProblem this,int group)
 {
  this->zgroupnumber = NLPGetObjectiveGroupNumber(this,group);
 }

#ifdef JUNK
int NLPAddLinearEqualityConstraint(NLProblem this,char *name,double *a, double b)
 {
  char RoutineName[]="NLPAddNonlinearEqualityConstraint";
  int i;
  int *tmpequalityConstraintGroup;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(this->nEqualityConstraints>=this->mEqualityConstraints)
   {
    if(this->mEqualityConstraints==-1)
     {
      this->mEqualityConstraints=NUMBEROFEQUALITYCONSTRAINTSTOALLOCATE;
      this->equalityConstraintGroup=(int*)malloc(sizeof(int)*this->mEqualityConstraints);
      if(this->equalityConstraintGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mEqualityConstraints*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->mEqualityConstraints;i++)this->equalityConstraintGroup[i]=-1;
     }else{
      this->mEqualityConstraints+=NUMBEROFEQUALITYCONSTRAINTSTOALLOCATE;
      tmpequalityConstraintGroup=(int*)malloc(sizeof(int)*this->mEqualityConstraints);
      if(tmpequalityConstraintGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mEqualityConstraints*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->nEqualityConstraints;i++)tmpequalityConstraintGroup[i]=this->equalityConstraintGroup[i];
      for(i=this->nEqualityConstraints;i<this->mEqualityConstraints;i++)tmpequalityConstraintGroup[i]=-1;
      free(this->equalityConstraintGroup);
      this->equalityConstraintGroup=tmpequalityConstraintGroup;
     }
   }
  this->equalityConstraintGroup[this->nEqualityConstraints]=NLPAddGroup(this,name);

  i=this->equalityConstraintGroup[this->nEqualityConstraints];
  if(this->groupA[i]!=(NLVector)NULL)
     NLFreeVector(this->groupA[i]);
  this->groupA[i]=NLCreateVectorWithFullData(this->nVariables,a);
  this->groupB[i]=b;
  this->groupBSet[i]=1;

  this->nEqualityConstraints++;
  return(this->nEqualityConstraints-1);
 }

int NLPAddLinearInequalityConstraint(NLProblem this,char *name, double *a, double b)
 {
  char RoutineName[]="NLPAddLinearInequalityConstraint";
  int i;
  int *tmpinequalityConstraintGroup;
  double *tmpinequalityConstraintLowerBound;
  double *tmpinequalityConstraintUpperBound;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(this->nInequalityConstraints>=this->mInequalityConstraints)
   {
    if(this->mInequalityConstraints==-1)
     {
      this->mInequalityConstraints=NUMBEROFINEQUALITYCONSTRAINTSTOALLOCATE;
      this->inequalityConstraintGroup=(int*)malloc(sizeof(int)*this->mInequalityConstraints);
      if(this->inequalityConstraintGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->inequalityConstraintLowerBound=(double*)malloc(sizeof(double)*this->mInequalityConstraints);
      if(this->inequalityConstraintLowerBound==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      this->inequalityConstraintUpperBound=(double*)malloc(sizeof(double)*this->mInequalityConstraints);
      if(this->inequalityConstraintUpperBound==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->mGroups;i++)
       {
        this->inequalityConstraintGroup[i]=-1;
        this->inequalityConstraintLowerBound[i]=0.;
        this->inequalityConstraintUpperBound[i]=DBL_MAX;
       }
     }else{
      this->mInequalityConstraints+=NUMBEROFINEQUALITYCONSTRAINTSTOALLOCATE;
      tmpinequalityConstraintGroup=(int*)malloc(sizeof(int)*this->mInequalityConstraints);
      if(this->inequalityConstraintGroup==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpinequalityConstraintLowerBound=(double*)malloc(sizeof(double)*this->mInequalityConstraints);
      if(this->inequalityConstraintLowerBound==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      tmpinequalityConstraintUpperBound=(double*)malloc(sizeof(double)*this->mInequalityConstraints);
      if(this->inequalityConstraintUpperBound==(double*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(double));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
       }

      for(i=0;i<this->nInequalityConstraints;i++)
       {
        tmpinequalityConstraintGroup[i]=this->inequalityConstraintGroup[i];
        tmpinequalityConstraintLowerBound[i]=this->inequalityConstraintLowerBound[i];
        tmpinequalityConstraintUpperBound[i]=this->inequalityConstraintUpperBound[i];
       }
      for(i=this->nInequalityConstraints;i<this->mInequalityConstraints;i++)
       {
        tmpinequalityConstraintGroup[i]=-1;
        tmpinequalityConstraintLowerBound[i]=DBL_MIN;
        tmpinequalityConstraintUpperBound[i]=DBL_MAX;
       }
      free(this->inequalityConstraintGroup);
      free(this->inequalityConstraintLowerBound);
      free(this->inequalityConstraintUpperBound);
      this->inequalityConstraintGroup=tmpinequalityConstraintGroup;
      this->inequalityConstraintLowerBound=tmpinequalityConstraintLowerBound;
      this->inequalityConstraintUpperBound=tmpinequalityConstraintUpperBound;
     }
   }
  this->inequalityConstraintGroup[this->nInequalityConstraints]=NLPAddGroup(this,name);

  i=this->inequalityConstraintGroup[this->nInequalityConstraints];
  if(this->groupA[i]!=(NLVector)NULL)
     NLFreeVector(this->groupA[i]);
  this->groupA[i]=NLCreateVectorWithFullData(this->nVariables,a);
  this->groupB[i]=b;
  this->groupBSet[i]=1;

  this->nInequalityConstraints++;
  return(this->nInequalityConstraints-1);
 }
#endif

/* New June 2001 */

double NLPGetGroupValue(NLProblem,int,double*);
void NLPGetGroupGradient(NLProblem,int,double*,double*);
void NLPGetGroupHessian(NLProblem,int,double*,double*);

double NLPGetNonlinearElementValue(NLProblem,int,double*);
void NLPGetNonlinearElementGradient(NLProblem,int,double*,double*);
void NLPGetNonlinearElementHessian(NLProblem,int,double*,double*);

int NLPInternalExtendIndirectArrays(int *nItems,  int *mItems, 
                                    int ***items, int *initialItem,
                                    int **nitems, int **mitems, 
                                    double **itemB, double initialItemB,
                                    double **itemC, double initialItemC,
                                    int chunk)
 {
  char RoutineName[]="NLPInternalExtendIndirectArrays";
  int i;
  int **tmpIntPtr;
  int *tmpInt;
  double *tmpDble;

  if(*nItems>=(*mItems))
   {
    if((*mItems)==-1)
     {
      (*mItems)=chunk;
      (*items)=(int**)malloc(sizeof(int*)*(*mItems));
      if((*items)==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int*));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      *nitems=(int*)malloc(sizeof(int)*(*mItems));
      if(*nitems==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      (*mitems)=(int*)malloc(sizeof(int)*(*mItems));
      if((*mitems)==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      if(itemB!=(double**)NULL)
       {
        (*itemB)=(double*)malloc(sizeof(double)*(*mItems));
        if((*itemB)==(double*)NULL)
         {
          sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(double));
          NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
          return 12;
         }
       }
      if(itemC!=(double**)NULL)
       {
        (*itemC)=(double*)malloc(sizeof(double)*(*mItems));
        if((*itemC)==(double*)NULL)
         {
          sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(double));
          NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
          return 12;
         }
       }

      for(i=0;i<(*mItems);i++)
       {
        (*nitems)[i]=0;
        (*mitems)[i]=-1;
        (*items)[i]=initialItem;
        if(itemB!=(double**)NULL)(*itemB)[i]=initialItemB;
        if(itemC!=(double**)NULL)(*itemC)[i]=initialItemC;
       }
     }else{
      (*mItems)+=chunk;

      tmpIntPtr=(int**)malloc(sizeof(int*)*(*mItems));
      if(tmpIntPtr==(int**)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int*));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      for(i=0;i<(*nItems);i++)tmpIntPtr[i]=(*items)[i];
      for(i=(*nItems);i<(*mItems);i++)tmpIntPtr[i]=initialItem;
      free((*items));
      (*items)=tmpIntPtr;

      tmpInt=(int*)malloc(sizeof(int)*(*mItems));
      if(tmpInt==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      for(i=0;i<(*nItems);i++)tmpInt[i]=(*nitems)[i];
      for(i=(*nItems);i<(*mItems);i++)tmpInt[i]=0;
      free((*nitems));
      (*nitems)=tmpInt;

      tmpInt=(int*)malloc(sizeof(int)*(*mItems));
      if(tmpInt==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      for(i=0;i<(*nItems);i++)tmpInt[i]=(*mitems)[i];
      for(i=(*nItems);i<(*mItems);i++)tmpInt[i]=0;
      free((*mitems));
      (*mitems)=tmpInt;

      if(itemB!=(double**)NULL)
       {
        tmpDble=(double*)malloc(sizeof(double)*(*mItems));
        if(tmpDble==(double*)NULL)
         {
          sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(double));
          NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
          return 12;
         }
        for(i=0;i<(*nItems);i++)tmpDble[i]=(*itemB)[i];
        for(i=(*nItems);i<(*mItems);i++)tmpDble[i]=initialItemB;
        free((*itemB));
        (*itemB)=tmpDble;
       }

      if(itemC!=(double**)NULL)
       {
        tmpDble=(double*)malloc(sizeof(double)*(*mItems));
        if(tmpDble==(double*)NULL)
         {
          sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(double));
          NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
          return 12;
         }
        for(i=0;i<(*nItems);i++)tmpDble[i]=(*itemC)[i];
        for(i=(*nItems);i<(*mItems);i++)tmpDble[i]=initialItemC;
        free((*itemC));
        (*itemC)=tmpDble;
       }
     }
   }

  return 0;
 }

int NLPInternalExtendArray(int *nItems, int *mItems, int **items, int chunk, int initialItem)
 {
  char RoutineName[]="NLPInternalExtendArray";
  int i;
  int *tmpInt;

  if(*nItems>=(*mItems))
   {
    if((*mItems)==-1)
     {
      (*mItems)=chunk;
      (*items)=(int*)malloc(sizeof(int)*(*mItems));
      if((*items)==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }

      for(i=0;i<(*mItems);i++)(*items)[i]=initialItem;
     }else{
      (*mItems)+=chunk;

      tmpInt=(int*)malloc(sizeof(int)*(*mItems));
      if(tmpInt==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(*mItems)*sizeof(int));
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return 12;
       }
      for(i=0;i<(*nItems);i++)tmpInt[i]=(*items)[i];
      for(i=(*nItems);i<(*mItems);i++)tmpInt[i]=initialItem;
      free((*items));
      (*items)=tmpInt;
     }
   }

  return 0;
 }

void NLPAddVariables(NLProblem this,int n)
 {
  char RoutineName[]="NLPAddVariables";
  int i,i0;
  char *name;

/* Adds an additional n variables to the problem. Each linear part has to be resized. */

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  i0=this->nVariables;
  this->nVariables+=n;

  this->variableScale=(double*)realloc(this->variableScale,this->nVariables*sizeof(double));
  if(this->variableScale==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return;
   }
  for(i=0;i<n;i++)this->variableScale[i0+i]=1.;

  this->variableName=(char**)realloc(this->variableName,this->nVariables*sizeof(char**));
  if(this->variableName==(char**)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(char*));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return;
   }
  for(i=0;i<n;i++)
   {
    name=(char*)malloc(16*sizeof(char));
    sprintf(name,"newvar%d",i);
    this->variableName[i0+i]=name;
   }

  for(i=0;i<this->nGroups;i++)
   {
    if(this->groupA[i]!=(NLVector)NULL)
     {
      NLVectorIncreaseLength(this->groupA[i],n);
     }
   }

  this->simpleConstraintLowerBound=(double*)realloc(this->simpleConstraintLowerBound,this->nVariables*sizeof(double));
  if(this->simpleConstraintLowerBound==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return;
   }
  for(i=0;i<n;i++)this->simpleConstraintLowerBound[i0+i]=0.;

  this->simpleConstraintUpperBound=(double*)realloc(this->simpleConstraintUpperBound,this->nVariables*sizeof(double));
  if(this->simpleConstraintUpperBound==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return;
   }
  for(i=0;i<n;i++)this->simpleConstraintUpperBound[i0+i]=DBL_MAX;

  return;
 }

void NLPRemoveVariables(NLProblem this,int n)
 {
  char RoutineName[]="NLPRemoveVariables";
  int i;

/* Removes the last n variables from the problem. Each linear part has to be resized. 
   The assumption is made that none of the element variables include one which is dropped. */

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->nVariables-=n;
  for(i=0;i<this->nGroups;i++)
   {
    if(this->groupA[i]!=(NLVector)NULL)
     {
      NLVectorDecreaseLength(this->groupA[i],n);
     }
   }
  return;
 }

void NLPHideMinMaxConstraints(NLProblem this)
 {
  char RoutineName[]="NLPHideMinMaxConstraints";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->hideMinMaxConstraints=1;
  return;
 }

void NLPUnHideMinMaxConstraints(NLProblem this)
 {
  char RoutineName[]="NLPUnHideMinMaxConstraints";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->hideMinMaxConstraints=0;
  return;
 }

void NLPHideEqualityConstraints(NLProblem this)
 {
  char RoutineName[]="NLPHideEqualityConstraints";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->hideEqualityConstraints=1;
  return;
 }

void NLPUnHideEqualityConstraints(NLProblem this)
 {
  char RoutineName[]="NLPUnHideEqualityConstraints";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->hideEqualityConstraints=0;
  return;
 }

void NLPHideInequalityConstraints(NLProblem this)
 {
  char RoutineName[]="NLPHideInequalityConstraints";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->hideInequalityConstraints=1;
  return;
 }

void NLPUnHideInequalityConstraints(NLProblem this)
 {
  char RoutineName[]="NLPUnHideInequalityConstraints";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->hideInequalityConstraints=0;
  return;
 }

int  NLPGetNumberOfGroupsInMinMaxConstraint(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetNumberOfGroupsInMinMaxConstraint";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->nMinMaxConstraintGroups[constraint];
 }

int  NLPGetNumberOfGroupsInInequalityConstraint(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetNumberOfGroupsInInequalityConstraint";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->nInequalityConstraintGroups[constraint];
 }

int  NLPGetNumberOfGroupsInEqualityConstraint(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetNumberOfGroupsInEqualityConstraint";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return this->nEqualityConstraintGroups[constraint];
 }

int NLPSetInequalityConstraintGroupB(NLProblem this,int constraint,int group,double b)
 {
  char RoutineName[]="NLPSetInequalityConstraintGroupB";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nInequalityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPSetEqualityConstraintGroupB(NLProblem this,int constraint,int group,double b)
 {
  char RoutineName[]="NLPSetEqualityConstraintGroupB";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nEqualityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPSetMinMaxConstraintGroupB(NLProblem this,int constraint,int group,double b)
 {
  char RoutineName[]="NLPSetMinMaxConstraintGroupB";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nMinMaxConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];
  this->groupB[group]=b;
  this->groupBSet[group]=TRUE;

  return 1;
 }

int NLPSetInequalityConstraintGroupA(NLProblem this,int constraint,int group,NLVector a)
 {
  char RoutineName[]="NLPSetInequalityConstraintGroupA";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nInequalityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];

  NLRefVector(a);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetEqualityConstraintGroupA(NLProblem this,int constraint,int group,NLVector a)
 {
  char RoutineName[]="NLPSetEqualityConstraintGroupA";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nEqualityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];

  NLRefVector(a);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

int NLPSetMinMaxConstraintGroupA(NLProblem this,int constraint,int group,NLVector a)
 {
  char RoutineName[]="NLPSetMinMaxConstraintGroupA";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nMinMaxConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];

  NLRefVector(a);
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=a;

  return 1;
 }

NLVector NLPGetMinMaxConstraintGroupA(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetMinMaxConstraintGroupA";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(!(group>-1&&group<this->nMinMaxConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];

  return this->groupA[group];
 }

double NLPGetMinMaxConstraintGroupB(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetMinMaxConstraintGroupB";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(!(group>-1&&group<this->nMinMaxConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];

  return this->groupB[group];
 }

NLVector NLPGetEqualityConstraintGroupA(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetEqualityConstraintGroupA";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(!(group>-1&&group<this->nEqualityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  group=(this->equalityConstraintGroups[constraint])[group];

  return this->groupA[group];
 }

double NLPGetEqualityConstraintGroupB(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetEqualityConstraintGroupB";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(!(group>-1&&group<this->nEqualityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  group=(this->equalityConstraintGroups[constraint])[group];

  return this->groupB[group];
 }

NLVector NLPGetInequalityConstraintGroupA(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetInequalityConstraintGroupA";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  if(!(group>-1&&group<this->nInequalityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];

  return this->groupA[group];
 }

double NLPGetInequalityConstraintGroupB(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetInequalityConstraintGroupB";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(!(group>-1&&group<this->nInequalityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];

  return this->groupB[group];
 }

int NLPAddGroupToInequalityConstraint(NLProblem this,int constraint, char *name)
 {
  char RoutineName[]="NLPAddGroupToInequalityConstraint";
  int result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(NLPInternalExtendArray(&(this->nInequalityConstraintGroups[constraint]),
                            &(this->mInequalityConstraintGroups[constraint]),
                            &(this->inequalityConstraintGroups[constraint]),
                            NUMBEROFINEQUALITYCONSTRAINTGROUPSTOALLOCATE,-1)!=0)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to add an Inequality Constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=this->nInequalityConstraintGroups[constraint];
  (this->inequalityConstraintGroups[constraint])[this->nInequalityConstraintGroups[constraint]]=NLPAddGroup(this,name);
  (this->nInequalityConstraintGroups[constraint])++;

  return result;
 }

int NLPAddGroupToEqualityConstraint(NLProblem this,int constraint, char *name)
 {
  char RoutineName[]="NLPAddGroupToEqualityConstraint";
  int result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(NLPInternalExtendArray(&(this->nEqualityConstraintGroups[constraint]),
                            &(this->mEqualityConstraintGroups[constraint]),
                            &(this->equalityConstraintGroups[constraint]),
                            NUMBEROFEQUALITYCONSTRAINTGROUPSTOALLOCATE,-1)!=0)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to add an Equality Constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=this->nEqualityConstraintGroups[constraint];
  (this->equalityConstraintGroups[constraint])[this->nEqualityConstraintGroups[constraint]]=NLPAddGroup(this,name);
  (this->nEqualityConstraintGroups[constraint])++;

  return result;
 }

int NLPAddGroupToMinMaxConstraint(NLProblem this,int constraint, char *name)
 {
  char RoutineName[]="NLPAddGroupToMinMaxConstraint";
  int result;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(NLPInternalExtendArray(&(this->nMinMaxConstraintGroups[constraint]),
                            &(this->mMinMaxConstraintGroups[constraint]),
                            &(this->minMaxConstraintGroups[constraint]),
                            NUMBEROFMINMAXCONSTRAINTGROUPSTOALLOCATE,-1)!=0)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to add an MinMax Constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  result=this->nMinMaxConstraintGroups[constraint];
  (this->minMaxConstraintGroups[constraint])[this->nMinMaxConstraintGroups[constraint]]=NLPAddGroup(this,name);
  (this->nMinMaxConstraintGroups[constraint])++;

  return result;
 }

void NLPDeleteEqualityConstraintGroup(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPDeleteEqualityConstraintGroup";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(group<0 || group>=this->nEqualityConstraintGroups[constraint])
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nEqualityConstraintGroups[constraint]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  NLPDeleteGroup(this,(this->equalityConstraintGroups[constraint])[group]);
  (this->equalityConstraintGroups[constraint])[group]=-1;
  while(this->nEqualityConstraintGroups[constraint]>0 && (this->equalityConstraintGroups[constraint])[this->nEqualityConstraintGroups[constraint]-1]==-1)this->nEqualityConstraintGroups[constraint]--;

  return;
 }

void NLPDeleteInequalityConstraintGroup(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPDeleteInequalityConstraintGroup";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(group<0 || group>=this->nInequalityConstraintGroups[constraint])
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nInequalityConstraintGroups[constraint]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  NLPDeleteGroup(this,(this->inequalityConstraintGroups[constraint])[group]);
  (this->inequalityConstraintGroups[constraint])[group]=-1;
  while(this->nInequalityConstraintGroups[constraint]>0 && (this->inequalityConstraintGroups[constraint])[this->nInequalityConstraintGroups[constraint]-1]==-1)this->nInequalityConstraintGroups[constraint]--;

  return;
 }

void NLPDeleteMinMaxConstraintGroup(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPDeleteMinMaxConstraintGroup";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(group<0 || group>=this->nMinMaxConstraintGroups[constraint])
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nMinMaxConstraintGroups[constraint]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  NLPDeleteGroup(this,(this->minMaxConstraintGroups[constraint])[group]);
  (this->minMaxConstraintGroups[constraint])[group]=-1;
  while(this->nMinMaxConstraintGroups[constraint]>0 && (this->minMaxConstraintGroups[constraint])[this->nMinMaxConstraintGroups[constraint]-1]==-1)this->nMinMaxConstraintGroups[constraint]--;

  return;
 }

void NLPDeleteObjectiveGroup(NLProblem this,int group)
 {
  char RoutineName[]="NLPDeleteObjectiveGroup";
  int verbose=0;

  if(verbose){printf("%s(%d)\n",RoutineName,group);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(group<0 || group>=this->nGroupsInObjective)
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nGroupsInObjective);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  NLPDeleteGroup(this,this->groupsInObjective[group]);
  this->groupsInObjective[group]=-1;
  while(this->nGroupsInObjective>0 && this->groupsInObjective[this->nGroupsInObjective-1]==-1)this->nGroupsInObjective--;
  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}

  return;
 }

void NLPDeleteEqualityConstraint(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPDeleteEqualityConstraint";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  while(this->nEqualityConstraintGroups[constraint]>0)
   {
    NLPDeleteEqualityConstraintGroup(this,constraint,this->nEqualityConstraintGroups[constraint]-1);
   }

  free(this->equalityConstraintGroups[constraint]);
  this->equalityConstraintGroups[constraint]=this->equalityConstraintGroups[this->nEqualityConstraints-1];
  this->nEqualityConstraintGroups[constraint]=this->nEqualityConstraintGroups[this->nEqualityConstraints-1];
  this->mEqualityConstraintGroups[constraint]=this->mEqualityConstraintGroups[this->nEqualityConstraints-1];
  this->equalityConstraintGroups[this->nEqualityConstraints-1]=(int*)NULL;
  this->nEqualityConstraintGroups[this->nEqualityConstraints-1]=0;
  this->mEqualityConstraintGroups[this->nEqualityConstraints-1]=-1;
  this->nEqualityConstraints--;

  return;
 }

void NLPDeleteInequalityConstraint(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPDeleteInequalityConstraint";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  while(this->nInequalityConstraintGroups[constraint]>0)
    NLPDeleteInequalityConstraintGroup(this,constraint,this->nInequalityConstraintGroups[constraint]-1);

  free(this->inequalityConstraintGroups[constraint]);
  this->inequalityConstraintGroups[constraint]=this->inequalityConstraintGroups[this->nInequalityConstraints-1];
  this->nInequalityConstraintGroups[constraint]=this->nInequalityConstraintGroups[this->nInequalityConstraints-1];
  this->mInequalityConstraintGroups[constraint]=this->mInequalityConstraintGroups[this->nInequalityConstraints-1];
  this->inequalityConstraintGroups[this->nInequalityConstraints-1]=(int*)NULL;
  this->nInequalityConstraintGroups[this->nInequalityConstraints-1]=0;
  this->mInequalityConstraintGroups[this->nInequalityConstraints-1]=-1;
  this->nInequalityConstraints--;

  return;
 }

void NLPDeleteMinMaxConstraint(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPDeleteMinMaxConstraint";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  while(this->nMinMaxConstraintGroups[constraint]>0)
    NLPDeleteMinMaxConstraintGroup(this,constraint,this->nMinMaxConstraintGroups[constraint]-1);

  free(this->minMaxConstraintGroups[constraint]);
  this->minMaxConstraintGroups[constraint]=this->minMaxConstraintGroups[this->nMinMaxConstraints-1];
  this->nMinMaxConstraintGroups[constraint]=this->nMinMaxConstraintGroups[this->nMinMaxConstraints-1];
  this->mMinMaxConstraintGroups[constraint]=this->mMinMaxConstraintGroups[this->nMinMaxConstraints-1];
  this->minMaxConstraintGroups[this->nMinMaxConstraints-1]=(int*)NULL;
  this->nMinMaxConstraintGroups[this->nMinMaxConstraints-1]=0;
  this->mMinMaxConstraintGroups[this->nMinMaxConstraints-1]=-1;
  this->nMinMaxConstraints--;

  return;
 }

void NLPDeleteGroup(NLProblem this,int group)
 {
  char RoutineName[]="NLPDeleteGroup";
  static char DELETEDGROUP[]="DeletedGroup";
  int element;
  int i;
  int verbose=0;

  if(verbose){printf("%s(%d)\n",RoutineName,group);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(group<0 || group>=this->nGroups)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",group,this->nGroups);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(this->groupName[group]!=(char*)NULL)free(this->groupName[group]);
  this->groupName[group]=DELETEDGROUP;
  if(this->groupFunction[group]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[group]);
  this->groupFunction[group]=(NLGroupFunction)NULL;
  if(this->groupA[group]!=(NLVector)NULL)NLFreeVector(this->groupA[group]);
  this->groupA[group]=(NLVector)NULL;
  this->groupB[group]=0.;
  this->groupBSet[group]=0;
  this->groupScale[group]=1.;
  for(i=0;i<this->nElementsInGroup[group];i++)
   {
    element=(this->element[group])[i];
    if(NLFreeNonlinearElement(this,element))
     {
      this->nonlinearElement[element]=(NLNonlinearElementPtr)NULL;
      while(this->nNonlinearElements>-1 && this->nonlinearElement[this->nNonlinearElements-1]==(NLNonlinearElementPtr)NULL)
           this->nNonlinearElements--;
     }
    (this->elementWeight[group])[i]=1.;
    (this->elementWeightSet[group])[i]=0;
   }
  if(this->element[group]!=(int*)NULL)free(this->element[group]);
  this->element[group]=(int*)NULL;
  if(this->elementWeightSet[group]!=(int*)NULL)free(this->elementWeightSet[group]);
  this->elementWeightSet[group]=(int*)NULL;
  if(this->elementWeight[group]!=(double*)NULL)free(this->elementWeight[group]);
  this->elementWeight[group]=(double*)NULL;
  this->nElementsInGroup[group]=0;
  this->mElementsInGroup[group]=-1;

  while(this->nGroups>-1&&this->groupName[this->nGroups-1]==DELETEDGROUP)this->nGroups--;
  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}

  return;
 }

int NLPGetNumberOfMinMaxConstraintGroups(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetNumberOfMinMaxConstraintGroups";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  return this->nMinMaxConstraintGroups[constraint];
 }

int NLPGetNumberOfEqualityConstraintGroups(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetNumberOfEqualityConstraintGroups";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  return this->nEqualityConstraintGroups[constraint];
 }

int NLPGetNumberOfInequalityConstraintGroups(NLProblem this,int constraint)
 {
  char RoutineName[]="NLPGetNumberOfInequalityConstraintGroups";
  int group;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  return this->nInequalityConstraintGroups[constraint];
 }

int NLPSetEqualityConstraintGroupFunction(NLProblem this,int constraint,int group,NLGroupFunction g)
 {
  char RoutineName[]="NLPSetEqualityConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetEqualityConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefGroupFunction(g);
  group=(this->equalityConstraintGroups[constraint])[group];
  if(this->groupFunction[group]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[group]);
  this->groupFunction[group]=g;

  return 1;
 }

int NLPSetInequalityConstraintGroupFunction(NLProblem this,int constraint,int group,NLGroupFunction g)
 {
  char RoutineName[]="NLPSetInequalityConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetInequalityConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefGroupFunction(g);
  group=(this->inequalityConstraintGroups[constraint])[group];
  if(this->groupFunction[group]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[group]);
  this->groupFunction[group]=g;

  return 1;
 }

int NLPSetMinMaxConstraintGroupFunction(NLProblem this,int constraint,int group,NLGroupFunction g)
 {
  char RoutineName[]="NLPSetMinMaxConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetMinMaxConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nMinMaxConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLRefGroupFunction(g);
  group=(this->minMaxConstraintGroups[constraint])[group];
  if(this->groupFunction[group]!=(NLGroupFunction)NULL)NLFreeGroupFunction(this->groupFunction[group]);
  this->groupFunction[group]=g;

  return 1;
 }

int NLPSetEqualityConstraintGroupScale(NLProblem this,int constraint,int group,double s)
 {
  char RoutineName[]="NLPSetEqualityConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetEqualityConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];
  NLPSetGroupScale(this,group,s);

  return 1;
 }

int NLPSetInequalityConstraintGroupScale(NLProblem this,int constraint,int group,double s)
 {
  char RoutineName[]="NLPSetInequalityConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetInequalityConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];
  NLPSetGroupScale(this,group,s);

  return 1;
 }

int NLPSetMinMaxConstraintGroupScale(NLProblem this,int constraint,int group,double s)
 {
  char RoutineName[]="NLPSetMinMaxConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetMinMaxConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nMinMaxConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];
  NLPSetGroupScale(this,group,s);

  return 1;
 }

int NLPAddNonlinearElementToEqualityConstraintGroup(NLProblem this,int constraint,int group,double w,NLNonlinearElement ef)
 {
  char RoutineName[]="NLPSetEqualityConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetEqualityConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];
  NLPAddNonlinearElementToGroup(this,group,w,ef);

  return 1;
 }

int NLPAddNonlinearElementToInequalityConstraintGroup(NLProblem this,int constraint,int group,double w,NLNonlinearElement ef)
 {
  char RoutineName[]="NLPSetInequalityConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetInequalityConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];
  NLPAddNonlinearElementToGroup(this,group,w,ef);

  return 1;
 }

int NLPAddNonlinearElementToMinMaxConstraintGroup(NLProblem this,int constraint,int group,double w,NLNonlinearElement ef)
 {
  char RoutineName[]="NLPSetMinMaxConstraintGroupFunction";
  int verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(verbose)
   {
    printf("NLPSetMinMaxConstraintGroupFunction(%d)\n",group);
    printf(" this->groupFunction = %d\n",this->groupFunction);
   }

  if(!(constraint>-1&&constraint<this->nMinMaxConstraints))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroupsInObjective-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];
  NLPAddNonlinearElementToGroup(this,group,w,ef);

  return 1;
 }

int NLPGetInequalityConstraintNumberOfGroup(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetInequalityConstraintNumberOfGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(constraint>-1 && constraint<this->nInequalityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Inequality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,this->nInequalityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(group<0 || group>=this->nInequalityConstraintGroups[constraint])
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nInequalityConstraintGroups[constraint]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  return((this->inequalityConstraintGroups[constraint])[group]);
 }

int NLPGetEqualityConstraintNumberOfGroup(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetEqualityConstraintNumberOfGroup";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(constraint>-1 && constraint<this->nEqualityConstraints))
   {
    sprintf(NLProblemErrorMsg,"Equality constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,this->nEqualityConstraints-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(group<0 || group>=this->nEqualityConstraintGroups[constraint])
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nEqualityConstraintGroups[constraint]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  return((this->equalityConstraintGroups[constraint])[group]);
 }

int NLPGetMinMaxConstraintNumberOfGroup(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetMinMaxConstraintNumberOfGroup";

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!(constraint>-1 && constraint<NLPGetNumberOfMinMaxConstraints(this)))
   {
    sprintf(NLProblemErrorMsg,"MinMax constraint number %d (argument 2) is illegal. Must be in range 0 to %d",constraint,NLPGetNumberOfMinMaxConstraints(this)-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(group<0 || group>=this->nMinMaxConstraintGroups[constraint])
   {
    sprintf(NLProblemErrorMsg,"Group %d (argument 3) is Invalid, must be in [0,%d).",group,this->nMinMaxConstraintGroups[constraint]);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  return((this->minMaxConstraintGroups[constraint])[group]);
 }

int NLPQueryPolynomialOrderOfVariablesInObjective(NLProblem this,int *order)
 {
  char RoutineName[]="NLPQueryPolynomialOrderOfVariablesInObjective";
  int i,j,n;
  int nev;
  int *ev;
  int group,ng,element,ne;
  int v,o,gO;
  NLVector gA;
  NLNonlinearElement nel;
  NLElementFunction ef;
  static int verbose=0;
  NLMatrix R;
  int u,niv;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(order==(int*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Array for result (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  n=NLPGetNumberOfVariables(this);
  for(i=0;i<n;i++)order[i]=0;

  ng=NLPGetNumberOfGroupsInObjective(this);
  for(i=0;i<ng;i++)
   {
    if(verbose){printf("  Objective group %d/%d\n",i,ng);
      for(v=0;v<n;v++)printf("    Variable %d - %s is currently order %d\n",v,NLPGetVariableName(this,v),order[v]);
      fflush(stdout);}
    group=NLPGetObjectiveGroupNumber(this,i);
    gO=NLGFQueryPolynomialOrder(NLPGetGroupFunction(this,group));
    if(verbose){printf("     group is order %d\n",gO);fflush(stdout);}
    gA=NLPGetGroupA(this,group);
    if(gA!=(NLVector)NULL)
     {
      for(v=0;v<n;v++)
       {
        if(NLVGetC(gA,v)!=0.&& gO>order[v])
         {
          if(verbose){printf("       variable %d %s appears in linear element old order %d, new %d\n",v,NLPGetVariableName(this,v),order[v],gO);fflush(stdout);}
          order[v]=gO;
         }
       }
     }
    ne=NLPGetNumberOfElementsInGroup(this,group);
    for(element=0;element<ne;element++)
     {
      if(verbose){printf("            group element %d/%d\n",element,ne);fflush(stdout);}
      nel=NLPGetNonlinearElementOfGroup(this,group,element);
      nev=NLNEGetElementDimension(this,nel);
      ev=NLNEGetElementVariables(this,nel);
      ef=NLNEGetElementFunction(this,nel);
      R=NNLEGetRangeXForm(this,nel);
      if(R!=(NLMatrix)NULL)
       {
        if(verbose){printf("               element has a range transform\n");fflush(stdout);}
        niv=NLEGetDimension(ef);
        for(v=0;v<niv;v++)
         {
          o=NLEFQueryPolynomialOrderOfElementVariable(ef,v);
          j=NLNEGetIndex(this,nel,v);
          for(u=0;u<nev;u++)
           {
            if(NLMGetElement(R,v,u)!=0. && gO*o>order[j])order[j]=gO*o;
           }
         }
       }else{
        if(verbose){printf("               element does not have a range transform\n");fflush(stdout);}
        for(v=0;v<nev;v++)
         {
          o=NLEFQueryPolynomialOrderOfElementVariable(ef,v);
          u=NLNEGetIndex(this,nel,v);
          if(verbose){printf("       ev %d (variable %d %s) is order %d in NE\n",v,u,NLPGetVariableName(this,u),o);fflush(stdout);}
          if(verbose){printf("       variable %d appears in nonliner element old order %d, new %d\n",u,order[u],gO*o);fflush(stdout);}
          if(gO*o>order[u])order[u]=gO*o;
         }
       }
     }
   }

  for(i=0;i<n;i++)
   if(order[i]>NLVARIABLEDEPENDENCENOTSET)order[i]=NLVARIABLEDEPENDENCENOTSET;

  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}
  return 0;
 
}

double NLPEvaluateObjective(NLProblem this,NLVector x)
 {
  char RoutineName[]="NLPEvaluateObjective";
  int i,n;
  double o;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  o=0.;
  for(i=0;i<this->nGroupsInObjective;i++)
   {
    NLPEvaluateGroup(this,this->groupsInObjective[i],x);
    o+=this->groupValue[this->groupsInObjective[i]];
    if(verbose){if(i>0)printf("+");
                printf("%le",this->groupValue[this->groupsInObjective[i]]);fflush(stdout);}
   }
  if(verbose){printf("=%le\n",o);fflush(stdout);}

  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}

  NLEvaluateObjectiveTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateObjectiveNCalls++;
  return o;
 }

int NLPEvaluateGradientOfObjective(NLProblem this,NLVector x,NLVector g)
 {
  char RoutineName[]="NLPEvaluateGradientOfObjective";
  int i,j,k,nev;
  int group,element;
  double dg;
  static double *b=(double*)NULL;
  static int nb=0;
  NLMatrix R;
  int *ev;
  int verbose=0;
  double w;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(g==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"g (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

/* sum g' * ( sum w f' R + a ) */
  NLVSetToZero(g);
  for(i=0;i<this->nGroupsInObjective;i++)
   {
    group=this->groupsInObjective[i];

    if(verbose){printf(" Group %d (%d) - current grad is",group,i);NLPrintVector(stdout,g);printf("\n");fflush(stdout);}
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    if(this->groupA[group]!=(NLVector)NULL)
     {
/*    for(k=0;k<this->nVariables;k++)NLVSetC(g,k,NLVGetC(g,k)+dg*NLVGetC(this->groupA[group],k));*/
      NLVPlusV(g,this->groupA[group],dg); /* Andreas' fix */
     }
    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      if(verbose){printf("    element %d\n",j);fflush(stdout);}
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      nev=NLNEGetElementDimension(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];
      if(verbose){printf("    element weight=%lf\n",w);fflush(stdout);}

      R=NNLEGetRangeXForm(this,element);
      if(R!=(NLMatrix)NULL)
       {
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }

         }
        ev=NLNEGetElementVariables(this,element);

        NLMVMultT(R,this->elementGradient[element],b);
        for(k=0;k<nev;k++)NLVSetC(g,ev[k],NLVGetC(g,ev[k])+w*dg*b[k]);
       }else{
        ev=NLNEGetElementVariables(this,element);
        for(k=0;k<nev;k++)
         {
          NLVSetC(g,ev[k],NLVGetC(g,ev[k])
                  +w*dg*(this->elementGradient[element])[k]);
         }
       }
     }
   }
  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}


  NLEvaluateObjectiveDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateObjectiveDerNCalls++;
  return 0;
 }

int NLPEvaluateHessianOfObjective(NLProblem this,NLVector x,NLMatrix H)
 {
  char RoutineName[]="NLPEvaluateHessianOfObjective";
  int i,j,k,l,nev;
  double dg,ddg;
  double *ddf,*df;
  int element,group;
  NLMatrix R;
  int *ev;
  int niv;
  static double *b=(double*)NULL;
  static double *c=(double*)NULL;
  static double *d=(double*)NULL;
  static int nb=0;
  static int nc=0;
  static int nd=0;
  static int verbose=0;
  double w;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(H==(NLMatrix)NULL)
   {
    sprintf(NLProblemErrorMsg,"H (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

/* sum ( sum w g' * R^T f'' R + w g'' * [ (f'R)^T f'R + (f'R)^T a + a^T f' R ] ) + g'' a^T a */

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this->nVariables>nc)
   {
    nc=this->nVariables;
    c=(double*)realloc(c,nc*sizeof(double));
    if(c==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
    d=(double*)realloc(d,nc*sizeof(double));
    if(d==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
   }

  if(verbose){printf("Set H to zero\n");fflush(stdout);}
  if(NLMnE(H)==0)NLMDetermineHessianSparsityStructure(this,'O',0,H);
  NLMSetToZero(H);

  for(i=0;i<this->nGroupsInObjective;i++)
   {
    group=this->groupsInObjective[i];

    if(verbose){printf("Group %d/%d is number %d\n",i,this->nGroupsInObjective,group);fflush(stdout);}
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    ddg=this->groupHessian[group];
    if(verbose){printf("    dg=%lf\n",dg);fflush(stdout);}
    if(verbose){printf("    ddg=%lf\n",ddg);fflush(stdout);}

    if(this->groupA[group]!=(NLVector)NULL)
     {
      for(j=0;j<this->nVariables;j++)c[j]=NLVGetC(this->groupA[group],j);
     }else{
      for(j=0;j<this->nVariables;j++)c[j]=0.;
     }

    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      element=(this->element[group])[j];
      if(verbose){printf("    element %d/%d is number %d\n",j,this->nElementsInGroup[group],element);fflush(stdout);}
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      df=this->elementGradient[element];
      ddf=this->elementHessian[element];
      niv=NLNEGetInternalDimension(this,element);
      nev=NLNEGetElementDimension(this,element);
      R=NNLEGetRangeXForm(this,element);
      ev=NLNEGetElementVariables(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];
      if(verbose){printf("    element weight=%lf\n",w);fflush(stdout);}
      if(R!=(NLMatrix)NULL)
       {
        if(verbose){printf("    Range transformation niv=%d, nev=%d\n",niv,nev);fflush(stdout);}
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }

/* sum g' * ( R^T f'' R ) + g'' * (f'R+a)^T (f'R+a)*/

        NLMVMultT(R,df,b); /* was d instead of b */
        for(k=0;k<nev;k++)c[ev[k]]+=w*b[k]; /* was d instead of b */
        if(verbose){printf("  done f'R \n");fflush(stdout);}

        if(dg!=0.&&w!=0.)
         {
          NLMMMMProd(R,ddf,b);
          NLMSumSubMatrixInto(H,w*dg,nev,ev,b);
         }
       }else{

/* sum g' * f'' + g'' * (f'+a)^T (f'+a)*/

        if(verbose){printf("    No range transformation\n");fflush(stdout);}

        for(k=0;k<nev;k++)c[ev[k]]+=w*df[k];
        NLMSumSubMatrixInto(H,w*dg,nev,ev,ddf);
       }
     }
    if(verbose){printf("Linear ELement\n");fflush(stdout);}
    NLMSumRankOneInto(H,ddg,c);
   }

  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}
  NLEvaluateObjectiveSecDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateObjectiveSecDerNCalls++;
  return 0;
 }

double NLPEvaluateEqualityConstraint(NLProblem this,int constraint,NLVector x)
 {
  char RoutineName[]="NLPEvaluateEqualityConstraint";
  int i,n;
  double c;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  c=0.;
  for(i=0;i<this->nEqualityConstraintGroups[constraint];i++)
   {
    NLPEvaluateGroup(this,(this->equalityConstraintGroups[constraint])[i],x);
    c+=this->groupValue[(this->equalityConstraintGroups[constraint])[i]];
    if(verbose){if(i>0)printf("+");
                printf("%le",this->groupValue[(this->equalityConstraintGroups[constraint])[i]]);fflush(stdout);}
   }
  if(verbose){printf("\n");fflush(stdout);}

  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}

  NLEvaluateEqualityConstraintTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateEqualityConstraintNCalls++;
  return c;
 }

int NLPEvaluateGradientOfEqualityConstraint(NLProblem this,int constraint,NLVector x,NLVector g)
 {
  char RoutineName[]="NLPEvaluateGradientOfEqualityConstraint";
  int i,j,k,nev;
  int group,element;
  double dg;
  static double *b=(double*)NULL;
  static int nb=0;
  double w;
  NLMatrix R;
  int *ev;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(g==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"g (argument 4) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  NLVSetToZero(g);
  for(i=0;i<this->nEqualityConstraintGroups[constraint];i++)
   {
    group=(this->equalityConstraintGroups[constraint])[i];
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    if(this->groupA[group]!=(NLVector)NULL)
     {
/*    for(k=0;k<this->nVariables;k++)NLVSetC(g,k,NLVGetC(g,k)+dg*NLVGetC(this->groupA[group],k));*/
      NLVPlusV(g,this->groupA[group],dg); /* Andreas' fix */
     }
    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      nev=NLNEGetElementDimension(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];

      R=NNLEGetRangeXForm(this,element);
      if(R!=(NLMatrix)NULL)
       {
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }
        ev=NLNEGetElementVariables(this,element);
  
        NLMVMultT(R,this->elementGradient[element],b);
        for(k=0;k<nev;k++)NLVSetC(g,ev[k],NLVGetC(g,ev[k])+w*dg*b[k]);
       }else{
        ev=NLNEGetElementVariables(this,element);
        for(k=0;k<nev;k++)
         {
          NLVSetC(g,ev[k],NLVGetC(g,ev[k])
                         +w*dg*(this->elementGradient[element])[k]);
         }
       }
     }
   }

  NLEvaluateEqualityConstraintDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateEqualityConstraintDerNCalls++;
  return 0;
 }

int NLPEvaluateHessianOfEqualityConstraint(NLProblem this,int constraint,NLVector x,NLMatrix H)
 {
  char RoutineName[]="NLPEvaluateHessianOfEqualityConstraint";
  int i,j,k,l,nev;
  double dg,ddg;
  double *ddf,*df;
  int element,group;
  double w;
  NLMatrix R;
  int *ev;
  int niv;
  static double *b=(double*)NULL;
  static double *c=(double*)NULL;
  static double *d=(double*)NULL;
  static int nb=0;
  static int nc=0;
  static int nd=0;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(H==(NLMatrix)NULL)
   {
    sprintf(NLProblemErrorMsg,"H (argument 4) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

/* sum ( sum g' * R^T f'' R + g'' * [ (f'R)^T f'R + (f'R)^T a + a^T f' R ] ) + g'' a^T a */

  if(this->nVariables>nc)
   {
    nc=this->nVariables;
    c=(double*)realloc(c,nc*sizeof(double));
    if(c==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
    d=(double*)realloc(d,nc*sizeof(double));
    if(d==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
   }

  if(NLMnE(H)==0)NLMDetermineHessianSparsityStructure(this,'E',constraint,H);
  NLMSetToZero(H);
  for(i=0;i<this->nEqualityConstraintGroups[constraint];i++)
   {
    group=(this->equalityConstraintGroups[constraint])[i];
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    ddg=this->groupHessian[group];

    if(this->groupA[group]!=(NLVector)NULL)
     {
      for(j=0;j<this->nVariables;j++)c[j]=NLVGetC(this->groupA[group],j);
     }else{
      for(j=0;j<this->nVariables;j++)c[j]=0.;
     }

    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      df=this->elementGradient[element];
      ddf=this->elementHessian[element];
      niv=NLNEGetInternalDimension(this,element);
      nev=NLNEGetElementDimension(this,element);
      R=NNLEGetRangeXForm(this,element);
      ev=NLNEGetElementVariables(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];

      if(R!=(NLMatrix)NULL)
       {
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }

/* sum g' * ( R^T f'' R ) + g'' * (f'R+a)^T (f'R+a)*/

        NLMVMultT(R,df,b);  /* was d instead of b */
        for(k=0;k<nev;k++)c[ev[k]]+=w*b[k];  /* was d instead of b */

        if(dg!=0.&&w!=0.)
         {
          NLMMMMProd(R,ddf,b);
          NLMSumSubMatrixInto(H,w*dg,nev,ev,b);
         }
       }else{

/* sum g' * f'' + g'' * (f'+a)^T (f'+a)*/

        for(k=0;k<nev;k++)c[ev[k]]+=w*df[k];
        NLMSumSubMatrixInto(H,w*dg,nev,ev,ddf);
       }
     }
    NLMSumRankOneInto(H,ddg,c);
   }

  NLEvaluateEqualityConstraintSecDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateEqualityConstraintSecDerNCalls++;
  return 0;
 }

double NLPEvaluateInequalityConstraint(NLProblem this,int constraint,NLVector x)
 {
  char RoutineName[]="NLPEvaluateInequalityConstraint";
  int i,n;
  double c;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  c=0.;
  for(i=0;i<this->nInequalityConstraintGroups[constraint];i++)
   {
    NLPEvaluateGroup(this,(this->inequalityConstraintGroups[constraint])[i],x);
    c+=this->groupValue[(this->inequalityConstraintGroups[constraint])[i]];
    if(verbose){if(i>0)printf("+");
                printf("%le",this->groupValue[(this->inequalityConstraintGroups[constraint])[i]]);fflush(stdout);}
   }

  if(verbose){printf("\n");fflush(stdout);}

  if(verbose){printf("done %s\n",RoutineName);fflush(stdout);}

  NLEvaluateInequalityConstraintTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateInequalityConstraintNCalls++;
  return c;
 }

int NLPEvaluateGradientOfInequalityConstraint(NLProblem this,int constraint,NLVector x,NLVector g)
 {
  char RoutineName[]="NLPEvaluateGradientOfInequalityConstraint";
  int i,j,k,nev;
  int group,element;
  double dg;
  static double *b=(double*)NULL;
  static int nb=0;
  double w;
  NLMatrix R;
  int *ev;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(g==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"g (argument 4) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  NLVSetToZero(g);
  for(i=0;i<this->nInequalityConstraintGroups[constraint];i++)
   {
    group=(this->inequalityConstraintGroups[constraint])[i];
    if(verbose){printf(" Group %d (%d) - current grad is",group,i);NLPrintVector(stdout,g);printf("\n");fflush(stdout);}
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    if(this->groupA[group]!=(NLVector)NULL)
     {
/*    for(k=0;k<this->nVariables;k++)NLVSetC(g,k,NLVGetC(g,k)+dg*NLVGetC(this->groupA[group],k));*/
      NLVPlusV(g,this->groupA[group],dg); /* Andreas' fix */
     }
    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      if(verbose){printf("    element %d\n",j);fflush(stdout);}
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      nev=NLNEGetElementDimension(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];
      if(verbose){printf("    element weight=%lf\n",w);fflush(stdout);}

      R=NNLEGetRangeXForm(this,element);
      if(R!=(NLMatrix)NULL)
       {
        R=NNLEGetRangeXForm(this,element);
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }
        ev=NLNEGetElementVariables(this,element);

        NLMVMultT(R,this->elementGradient[element],b);
        for(k=0;k<nev;k++)NLVSetC(g,ev[k],NLVGetC(g,ev[k])+w*dg*b[k]);
       }else{
        ev=NLNEGetElementVariables(this,element);
        for(k=0;k<nev;k++)NLVSetC(g,ev[k],NLVGetC(g,ev[k])+w*dg*(this->elementGradient[element])[k]);
       }
     }
   }

  NLEvaluateInequalityConstraintDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateInequalityConstraintDerNCalls++;
  return 0;
 }

int NLPEvaluateHessianOfInequalityConstraint(NLProblem this,int constraint,NLVector x,NLMatrix H)
 {
  char RoutineName[]="NLPEvaluateHessianOfInequalityConstraint";
  int i,j,k,l,nev;
  double dg,ddg;
  double *ddf,*df;
  int element,group;
  NLMatrix R;
  int *ev;
  int niv;
  static double *b=(double*)NULL;
  static double *c=(double*)NULL;
  static double *d=(double*)NULL;
  static int nb=0;
  static int nc=0;
  static int nd=0;
  double w;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(H==(NLMatrix)NULL)
   {
    sprintf(NLProblemErrorMsg,"H (argument 4) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

/* sum ( sum g' * R^T f'' R + g'' * [ (f'R)^T f'R + (f'R)^T a + a^T f' R ] ) + g'' a^T a */

  if(this->nVariables>nc)
   {
    nc=this->nVariables;
    c=(double*)realloc(c,nc*sizeof(double));
    if(c==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
    d=(double*)realloc(d,nc*sizeof(double));
    if(d==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
   }

  if(NLMnE(H)==0)NLMDetermineHessianSparsityStructure(this,'I',constraint,H);
  NLMSetToZero(H);
  for(i=0;i<this->nInequalityConstraintGroups[constraint];i++)
   {
    group=(this->inequalityConstraintGroups[constraint])[i];
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    ddg=this->groupHessian[group];

    if(this->groupA[group]!=(NLVector)NULL)
     {
      for(j=0;j<this->nVariables;j++)c[j]=NLVGetC(this->groupA[group],j);
     }else{
      for(j=0;j<this->nVariables;j++)c[j]=0.;
     }

    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      df=this->elementGradient[element];
      ddf=this->elementHessian[element];
      niv=NLNEGetInternalDimension(this,element);
      nev=NLNEGetElementDimension(this,element);
      R=NNLEGetRangeXForm(this,element);
      ev=NLNEGetElementVariables(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];

      if(R!=(NLMatrix)NULL)
       {
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }

/* sum g' * ( R^T f'' R ) + g'' * (f'R+a)^T (f'R+a)*/

        NLMVMultT(R,df,b);  /* was d instead of b */
        for(k=0;k<nev;k++)c[ev[k]]+=w*b[k];  /* was d instead of b */
        if(dg!=0.&&w!=0.)
         {
          NLMMMMProd(R,ddf,b);
          NLMSumSubMatrixInto(H,w*dg,nev,ev,b);
         }
       }else{

/* sum g' * f'' + g'' * (f'+a)^T (f'+a)*/

        for(k=0;k<nev;k++)c[ev[k]]+=w*df[k];
        NLMSumSubMatrixInto(H,w*dg,nev,ev,ddf);
       }
     }
    NLMSumRankOneInto(H,ddg,c);
   }

  NLEvaluateInequalityConstraintSecDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateInequalityConstraintSecDerNCalls++;
  return 0;
 }

double NLPEvaluateMinMaxConstraint(NLProblem this,int constraint,NLVector x)
 {
  char RoutineName[]="NLPEvaluateMinMaxConstraint";
  int i,n;
  double c;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  c=0.;
  for(i=0;i<this->nMinMaxConstraintGroups[constraint];i++)
   {
    NLPEvaluateGroup(this,(this->minMaxConstraintGroups[constraint])[i],x);
    c+=this->groupValue[(this->minMaxConstraintGroups[constraint])[i]];
   }

  NLEvaluateMinMaxConstraintTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateMinMaxConstraintNCalls++;
  return c;
 }

int NLPEvaluateGradientOfMinMaxConstraint(NLProblem this,int constraint,NLVector x,NLVector g)
 {
  char RoutineName[]="NLPEvaluateGradientOfMinMaxConstraint";
  int i,j,k,nev;
  int group,element;
  double dg;
  static double *b=(double*)NULL;
  static int nb=0;
  double w;
  NLMatrix R;
  int *ev;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

  if(g==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"g (argument 4) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  NLVSetToZero(g);
  for(i=0;i<this->nMinMaxConstraintGroups[constraint];i++)
   {
    group=(this->minMaxConstraintGroups[constraint])[i];
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    if(this->groupA[group]!=(NLVector)NULL)
     {
/*    for(k=0;k<this->nVariables;k++)NLVSetC(g,k,NLVGetC(g,k)+dg*NLVGetC(this->groupA[group],k));*/
      NLVPlusV(g,this->groupA[group],dg); /* Andreas' fix */
     }
    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      nev=NLNEGetElementDimension(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];

      R=NNLEGetRangeXForm(this,element);
      if(R!=(NLMatrix)NULL)
       {
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }
        ev=NLNEGetElementVariables(this,element);

        NLMVMultT(R,this->elementGradient[element],b);
        for(k=0;k<nev;k++)NLVSetC(g,ev[k],NLVGetC(g,ev[k])+w*dg*b[k]);
       }else{
        ev=NLNEGetElementVariables(this,element);
        for(k=0;k<nev;k++)NLVSetC(g,ev[k],NLVGetC(g,ev[k])+w*dg*(this->elementGradient[element])[k]);
       }
     }
   }

  NLEvaluateMinMaxConstraintDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateMinMaxConstraintDerNCalls++;
  return 0;
 }

int NLPEvaluateHessianOfMinMaxConstraint(NLProblem this,int constraint,NLVector x,NLMatrix H)
 {
  char RoutineName[]="NLPEvaluateHessianOfMinMaxConstraint";
  int i,j,k,l,nev;
  double dg,ddg;
  double *ddf,*df;
  int element,group;
  NLMatrix R;
  int *ev;
  int niv;
  static double *b=(double*)NULL;
  static double *c=(double*)NULL;
  static double *d=(double*)NULL;
  static int nb=0;
  static int nc=0;
  static int nd=0;
  double w;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(x==(NLVector)NULL)
   {
    sprintf(NLProblemErrorMsg,"x (argument 3) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(H==(NLMatrix)NULL)
   {
    sprintf(NLProblemErrorMsg,"H (argument 4) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

/* sum ( sum g' * R^T f'' R + g'' * [ (f'R)^T f'R + (f'R)^T a + a^T f' R ] ) + g'' a^T a */

  if(this->nVariables>nc)
   {
    nc=this->nVariables;
    c=(double*)realloc(c,nc*sizeof(double));
    if(c==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
    d=(double*)realloc(d,nc*sizeof(double));
    if(d==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nc*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return 12;
     }
   }

  if(NLMnE(H)==0)NLMDetermineHessianSparsityStructure(this,'M',constraint,H);
  NLMSetToZero(H);
  for(i=0;i<this->nMinMaxConstraintGroups[constraint];i++)
   {
    group=(this->minMaxConstraintGroups[constraint])[i];
    NLPEvaluateGroup(this,group,x);
    dg=this->groupGradient[group];
    ddg=this->groupHessian[group];

    if(this->groupA[group]!=(NLVector)NULL)
     {
      for(j=0;j<this->nVariables;j++)c[j]=NLVGetC(this->groupA[group],j);
     }else{
      for(j=0;j<this->nVariables;j++)c[j]=0.;
     }

    for(j=0;j<this->nElementsInGroup[group];j++)
     {
      element=(this->element[group])[j];
      if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
      NLPEvaluateElement(this,element,x);
      df=this->elementGradient[element];
      ddf=this->elementHessian[element];
      niv=NLNEGetInternalDimension(this,element);
      nev=NLNEGetElementDimension(this,element);
      R=NNLEGetRangeXForm(this,element);
      ev=NLNEGetElementVariables(this,element);
      w=1.;
      if((this->elementWeightSet[group])[j])w=(this->elementWeight[group])[j];

      if(R!=(NLMatrix)NULL)
       {
        if(nev>nb)
         {
          nb=nev;
          b=(double*)realloc(b,nb*nb*sizeof(double));
          if(b==(double*)NULL)
           {
            sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nb*sizeof(double));
            NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
            return 12;
           }
         }

/* sum g' * ( R^T f'' R ) + g'' * (f'R+a)^T (f'R+a)*/

        NLMVMultT(R,df,b); /* was d instead of b */
        for(k=0;k<nev;k++)c[ev[k]]+=w*b[k]; /* was d instead of b */
        if(dg!=0.&&w!=0.)
         {
          NLMMMMProd(R,ddf,b);
          NLMSumSubMatrixInto(H,w*dg,nev,ev,b);
         }
       }else{

/* sum g' * f'' + g'' * (f'+a)^T (f'+a)*/

        for(k=0;k<nev;k++)c[ev[k]]+=w*df[k];
        NLMSumSubMatrixInto(H,w*dg,nev,ev,ddf);
       }
     }
    NLMSumRankOneInto(H,ddg,c);
   }

  NLEvaluateMinMaxConstraintSecDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateMinMaxConstraintSecDerNCalls++;
  return 0;
 }

int NLPInvalidateGroupAndElementCaches(NLProblem this)
 {
  char RoutineName[]="NLPInvalidateGroupAndElementCaches";
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 12;
   }

/*if(this->cache==0)this->cache=1;
   else this->cache=0;*/
  this->cache++;
  ttmpflag=1;

  for(i=0;i<this->nGroups;i++)
   {
    this->groupCached[i]=0;
   }
  for(i=0;i<this->nNonlinearElements;i++)
   {
    this->elementCached[i]=0;
   }

  return 0;
 }

int *NLPGetCacheFlag(NLProblem this)
 {
  char RoutineName[]="NLPGetCacheFlag";
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return (int*)NULL;
   }

  return &(this->cache);
 }

double NLPEvaluateGroup(NLProblem this,int group,NLVector x)
 {
  char RoutineName[]="NLPEvaluateGroup";
  double arg,elem;
  int i,element;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(group<0||group>=this->nGroups)
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }
  if(verbose)
   {
   printf("%s(%d,",RoutineName,group);
   NLPrintVector(stdout,x);
   printf(")\n");
   fflush(stdout);
   }

  if(this->groupCached[group]==1)
   {
    if(verbose){printf("  value %lf is cached\n",this->groupValue[group]);fflush(stdout);}
    return this->groupValue[group];
   }

  arg=0.;
  if(this->groupBSet[group])arg-=this->groupB[group];
  if(verbose){printf("  -b=%lf \n",arg);fflush(stdout);}
  if(this->groupA[group]!=(NLVector)NULL)
   {
    arg+=NLVInnerProd(this->groupA[group],x);
    if(verbose){printf("  <a,x>=%lf \n",NLVInnerProd(this->groupA[group],x));fflush(stdout);}
   }else
    if(verbose){printf("  <a,x>=0.\n");fflush(stdout);}
  if(verbose){printf("  <a,x>-b=%lf \n",arg);fflush(stdout);}
  for(i=0;i<this->nElementsInGroup[group];i++)
   {
    element=(this->element[group])[i];
    if(ttmp2){
       int I,mttmp;
       clock_t ttmp;

       ttmp=clock();
       mttmp=nttmp;
       if(ttmpflag){mttmp=100;printf("This is the first element evalute after an InvalidateCache\n");}
       for(I=0;I<mttmp;I++)
        {
         NLPEvaluateElement(this,element,x);
         if(ttmpflag)this->cache++;
        }
       ttmpflag=0;
       printf("Overall Time to evaluate Element %d is %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);fflush(stdout);
      }else
    NLPEvaluateElement(this,element,x);
    elem=this->elementValue[element];
    if(verbose){printf("  group %d element %d has value %lf\n",group,i,elem);fflush(stdout);}
    if((this->elementWeightSet[group])[i])
     {
      elem=(this->elementWeight[group])[i]*elem;
      if(verbose){printf("  w= %lf\n",(this->elementWeight[group])[i]);fflush(stdout);}
     }
    if(verbose){printf("  w*element value = %lf\n",elem);fflush(stdout);}
    arg=arg+elem;
   }
  if(verbose){printf("  sum ef's + <a,x>+b=%lf \n",arg);fflush(stdout);}

  if(this->groupFunction[group]!=(NLGroupFunction)NULL)
   {
    if(verbose){printf("  Nontrivial group function\n");fflush(stdout);}
    this->groupValue[group]=NLGEval(this->groupFunction[group],arg,this->groupFunctionData[group])*this->groupScale[group];
    this->groupGradient[group]=NLGEvalDer(this->groupFunction[group],arg,this->groupFunctionData[group])*this->groupScale[group];
    this->groupHessian[group]=NLGEvalSecDer(this->groupFunction[group],arg,this->groupFunctionData[group])*this->groupScale[group];
   }else{
    if(verbose){printf("  trivial group function\n");fflush(stdout);}
    this->groupValue[group]=arg*this->groupScale[group];
    this->groupGradient[group]=this->groupScale[group];
    this->groupHessian[group]=0.;
   }
  if(verbose){printf("  group Scale is %lf\n",this->groupScale[group]);fflush(stdout);}
  if(verbose){printf("  group value %lf, der %lf, 2nd %lf \n",this->groupValue[group],this->groupGradient[group],this->groupHessian[group]);fflush(stdout);}
  this->groupCached[group]=NLPCACHEGROUPS;

  NLEvaluateGroupTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateGroupNCalls++;
  return this->groupValue[group];
 }
 
double NLPEvaluateElement(NLProblem this,NLNonlinearElement element,NLVector x)
 {
  char RoutineName[]="NLPEvaluateElement";
  int i,j;
  int niv;
  int nev,*ev;
  NLMatrix R;
  NLElementFunction F;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(element<0||element>=this->nNonlinearElements)
   {
    sprintf(NLProblemErrorMsg,"Element %d is illegal (argument 2). Must be in range 0 to %d",element,this->nNonlinearElements-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(verbose)
   {
   printf("%s(%d,",RoutineName,element);
   NLPrintVector(stdout,x);
   printf(")\n");
   fflush(stdout);
   }

  if(this->elementCached[element]==1)
   {
    if(verbose){printf("  value %lf is cached\n",this->elementValue[element]);fflush(stdout);}
    return this->elementValue[element];
   }

  F=NLNEGetElementFunction(this,element);
  R=NNLEGetRangeXForm(this,element);
  niv=NLNEGetInternalDimension(this,element);
  nev=NLNEGetElementDimension(this,element);
  ev=NLNEGetElementVariables(this,element);

  if(this->internalVariables[element]==(double*)NULL)
   {
    this->internalVariables[element]=(double*)malloc(niv*sizeof(double));
    if(this->internalVariables[element]==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",niv*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return 0.;
     }
   }

  if(R!=(NLMatrix)NULL && this->elementVariables[element]==(double*)NULL)
   {
    this->elementVariables[element]=(double*)malloc(nev*sizeof(double));
    if(this->elementVariables[element]==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",niv*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return 0.;
     }
   }

  if(this->elementGradient[element]==(double*)NULL)
   {
    this->elementGradient[element]=(double*)malloc(niv*sizeof(double));
    if(this->elementGradient[element]==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",niv*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return 0.;
     }
   }

  if(this->elementHessian[element]==(double*)NULL)
   {
    this->elementHessian[element]=(double*)malloc(niv*niv*sizeof(double));
    if(this->elementHessian[element]==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",niv*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return 0.;
     }
   }

  if(R!=(NLMatrix)NULL)
   {
    for(i=0;i<nev;i++)(this->elementVariables[element])[i]=NLVGetC(x,ev[i]);
    if(verbose)
     {
      printf("Element variables are (%f",(this->elementVariables[element])[0]);
      for(i=1;i<nev;i++)
        printf(",%lf",(this->elementVariables[element])[i]);
      printf(")\n");
      fflush(stdout);
     }
    NLMVMult(R,this->elementVariables[element],this->internalVariables[element]);
   }else{
    for(i=0;i<niv;i++)(this->internalVariables[element])[i]=NLVGetC(x,ev[i]);
   }

  if(verbose)
   {
    printf("Internal variables are (%f",(this->internalVariables[element])[0]);
    for(i=1;i<niv;i++)
      printf(",%lf",(this->internalVariables[element])[i]);
    printf(")\n");
    fflush(stdout);
   }
   if(ttmp1)
    {
     clock_t ttmp;
     int I,mttmp;

     ttmp=clock();
     mttmp=nttmp;
     if(ttmpflag){mttmp=100;printf("This is the first evaluate after an InvalidateCache\n");}
     for(I=0;I<mttmp;I++)
      {
       if(ttmpflag)this->cache++;
       this->elementValue[element]=NLEEval(F,niv,this->internalVariables[element],
                                  NLPGetNonlinearElementData(this,element));
      }
     printf("Time to evaluate Element Function %d, %le secs\n",element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/mttmp);
     ttmpflag=0;
    }else
    this->elementValue[element]=NLEEval(F,niv,this->internalVariables[element],
                                  NLPGetNonlinearElementData(this,element));
  if(verbose){printf("  element evaluates as %lf\n",this->elementValue[element]);fflush(stdout);}
  for(i=0;i<niv;i++)
   {
    if(ttmp1)
     {
      clock_t ttmp;
      int I;

      ttmp=clock();
      for(I=0;I<nttmp;I++)
       {
        (this->elementGradient[element])[i]=NLEEvalDer(F,i,niv,this->internalVariables[element],
                                  NLPGetNonlinearElementData(this,element));
       }
      printf("Time to evaluate grad[%d] Element Function %d, %le secs\n",i,element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/nttmp);
     }else
    (this->elementGradient[element])[i]=NLEEvalDer(F,i,niv,this->internalVariables[element],
                                  NLPGetNonlinearElementData(this,element));
    for(j=0;j<=i;j++)
     {
      if(ttmp1)
       {
        clock_t ttmp;
        int I;

        ttmp=clock();
        for(I=0;I<nttmp;I++)
         {
          (this->elementHessian[element])[i+j*niv]
             =NLEEvalSecDer(F,i,j,niv,this->internalVariables[element],
                                      NLPGetNonlinearElementData(this,element));
         }
        printf("Time to evaluate hess[%d,%d] Element Function %d, %le secs\n",i,j,element,1.*(clock()-ttmp)/CLOCKS_PER_SEC/nttmp);
       }else
      (this->elementHessian[element])[i+j*niv]
         =NLEEvalSecDer(F,i,j,niv,this->internalVariables[element],
                                  NLPGetNonlinearElementData(this,element));
      (this->elementHessian[element])[j+i*niv]=(this->elementHessian[element])[i+j*niv];
     }
   }

  if(verbose)
   {
    printf("  gradient of element %d is (%f",element,(this->elementGradient[element])[0]);
    for(i=1;i<niv;i++)
      printf(",%lf",(this->elementGradient[element])[i]);
    printf(")\n");
    printf("  Hessian of element %d is \n",element);
    for(i=0;i<niv;i++)
     {
      printf("       [%lf",(this->elementHessian[element])[i+niv*0]);
      for(j=1;j<niv;j++)
        printf(" %lf",(this->elementHessian[element])[i+niv*j]);
      printf("]\n");
     }
    fflush(stdout);
   }


  NLEvaluateElementTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateElementNCalls++;

  this->elementCached[element]=NLPCACHEGROUPS;
  return this->elementValue[element];
 }

int NLPUnSetInequalityConstraintLowerBound(NLProblem this,int c)
 {
  char RoutineName[]="NLPUnSetInequalityConstraintLowerBound";
  return NLPSetInequalityConstraintLowerBound(this,c,-DBL_MAX);
 }

int NLPUnSetInequalityConstraintUpperBound(NLProblem this,int c)
 {
  char RoutineName[]="NLPUnSetInequalityConstraintUpperBound";
  return NLPSetInequalityConstraintUpperBound(this,c,DBL_MAX);
 }

int NLPUnSetInequalityConstraintBounds(NLProblem this,int c)
 {
  char RoutineName[]="NLPUnSetInequalityConstraintBounds";
  return NLPSetInequalityConstraintBounds(this,c,-DBL_MAX,DBL_MAX);
 }

int NLPIsInequalityConstraintLowerBoundSet(NLProblem this,int i)
 {
  return NLPGetInequalityConstraintLowerBound(this,i)>-DBL_MAX;
 }

int NLPIsInequalityConstraintUpperBoundSet(NLProblem this,int i)
 {
  return NLPGetInequalityConstraintUpperBound(this,i)<DBL_MAX;
 }

double *NLPGetGroupScales(NLProblem this)
 {
  return this->groupScale;
 }

double *NLPGetGroupValues(NLProblem this)
 {
  return this->groupValue;
 }

void NLPrintElementValues(NLProblem this, FILE *fid)
 {
  int i;
 
  fprintf(fid,"Element values\n\n");fflush(fid);
  for(i=0;i<this->nNonlinearElements;i++)
   {
    fprintf(fid,"  %d  %le\n",i,this->elementValue[i]);
   }
  fflush(fid);
  return;
 }

void NLPPrintVector(FILE *fid,int *j, NLProblem P,NLVector a)
 {
  int i,n;
  double v;

  n=NLVGetNC(a);
  for(i=0;i<n;i++)
   {
    v=NLVGetC(a,i);
    if(fabs(v)>1.e-14)
     {
      if(fabs(fabs(v)-1)<1.e-14)
       {
        if((*j)>0&&v>0)fprintf(fid,"+");
        if(v<0)fprintf(fid,"-");
        fprintf(fid,"%s",NLPGetVariableName(P,i));
        (*j)++;
       }else{
        if((*j)>0&&v>0)fprintf(fid,"+");
        if(itrunc(v)==v)fprintf(fid,"%d",itrunc(v));
         else if(itrunc(v*10)==10*v)fprintf(fid,"%.1lf",v);
         else if(itrunc(v*100)==100*v)fprintf(fid,"%.2lf",v);
         else fprintf(fid,"%lf",v);
        fprintf(fid,"*(%s)",NLPGetVariableName(P,i));
        (*j)++;
       }
     }
   }
  return;
 }

void NLPrintElement(FILE *fid,NLProblem P,int i)
 {
  int j,k,l;
  int ne;
  double w,b;

  k=0;
  for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
   {
    if(NLPIsElementFunctionSet(P,i,j))w=NLPGetElementWeight(P,i,j);
     else w=0;
    ne=NLPGetNonlinearElementOfGroup(P,i,j);
    if(j%8==7)fprintf(fid,"\n");

    if(fabs(w)>1.e-14)
     {
      if(k>0&&w>0)fprintf(fid,"+");
      if(fabs(fabs(w)-1)>1.e-14)
       {
        if(itrunc(w)==w)fprintf(fid,"%d*",itrunc(w));
         else if(itrunc(w*10)==10*w)fprintf(fid,"%.1lf*",w);
         else if(itrunc(w*100)==100*w)fprintf(fid,"%.2lf*",w);
         else fprintf(fid,"%21.14le*",w);
       }else{
        if(w<0)fprintf(fid,"-");
       }
      if(NLEGetExpr(NLNEGetElementFunction(P,ne))!=(char*)NULL)
       {
        fprintf(fid,"%s",NLEGetExpr(NLNEGetElementFunction(P,ne)));
       }else{
        fprintf(fid,"\"%s\"(",NLPGetElementType(P,NLEGetType(NLNEGetElementFunction(P,ne))));
        for(l=0;l<NLNEGetElementDimension(P,ne);l++)
         {
          if(l>0)fprintf(fid,",");
          fprintf(fid,"%s",NLPGetVariableName(P,NLNEGetIndex(P,ne,l)));
         }
        fprintf(fid,")");
       }
      k++;
     }
   }
  if(NLPIsGroupASet(P,i))
    NLPPrintVector(fid,&k,P,NLPGetGroupA(P,i));
  if(NLPIsGroupBSet(P,i)&&fabs(NLPGetGroupB(P,i))>1.e-14)
   {
    if(NLPGetGroupB(P,i)<0&&k>0)fprintf(fid,"+",-NLPGetGroupB(P,i));
    b=-NLPGetGroupB(P,i);
    if(itrunc(b)==b)fprintf(fid,"%d",itrunc(b));
     else if(itrunc(b*10)==10*b)fprintf(fid,"%.1lf",b);
     else if(itrunc(b*100)==100*b)fprintf(fid,"%.2lf",b);
     else fprintf(fid,"%lf",b);
   }

  return;
 }

void NLPrintProblemShort(FILE *fid,NLProblem P)
 {
  char RoutineName[]="NLPrintProblem";
  int i,I,j,J,k;

  if(fid==(FILE*)NULL)
   {
    sprintf(NLProblemErrorMsg,"File pointer (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(P==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  fprintf(fid,"%s\n",NLPGetProblemName(P));
  fprintf(fid,"\nSimple Bounds:\n\n",NLPGetNumberOfVariables(P));
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    fprintf(fid,"      ");
    if(NLPIsLowerSimpleBoundSet(P,i))fprintf(fid,"%10.7lf <=",NLPGetLowerSimpleBound(P,i));
     else
      fprintf(fid,"             ");
    fprintf(fid,"%s",NLPGetVariableName(P,i));
    if(NLPIsUpperSimpleBoundSet(P,i))fprintf(fid," <= %10.7lf",NLPGetUpperSimpleBound(P,i));
    fprintf(fid,"\n");
   }

  fprintf(fid,"\nObjective Function: \n\n");
  fprintf(fid,"      ");
  for(I=0;I<NLPGetNumberOfGroupsInObjective(P);I++)
   {
    if(I%3==2)fprintf(fid,"\n    ");
    i=NLPGetObjectiveGroupNumber(P,I);
    NLPrintGroupShort(fid,P,i,I);
   }
  fprintf(fid,"\n\n");

  if(NLPGetNumberOfEqualityConstraints(P)>0)fprintf(fid,"  Equality Constraints:\n\n");
  for(I=0;I<NLPGetNumberOfEqualityConstraints(P);I++)
   {
    fprintf(fid,"  %3d   %s\n",I,NLPGetGroupName(P,NLPGetEqualityConstraintNumberOfGroup(P,I,0)));
    fprintf(fid,"        ");
    for(J=0;J<NLPGetNumberOfEqualityConstraintGroups(P,I);J++)
     {
      i=NLPGetEqualityConstraintNumberOfGroup(P,I,J);
      if(J%3==2)fprintf(fid,"\n    ");
      NLPrintGroupShort(fid,P,i,J);
     }
    fprintf(fid,"=0\n");
   }
  if(NLPGetNumberOfEqualityConstraints(P)>0)fprintf(fid,"\n");

  if(NLPGetNumberOfInequalityConstraints(P)>0)fprintf(fid,"  Inequality Constraints:\n\n");
  for(I=0;I<NLPGetNumberOfInequalityConstraints(P);I++)
   {
    fprintf(fid,"  %3d   %s\n",I,NLPGetGroupName(P,NLPGetInequalityConstraintNumberOfGroup(P,I,0)));
    fprintf(fid,"        ");
    if(NLPIsInequalityConstraintLowerBoundSet(P,I))
      fprintf(fid,"%lf<",NLPGetInequalityConstraintLowerBound(P,I));
    for(J=0;J<NLPGetNumberOfInequalityConstraintGroups(P,I);J++)
     {
      i=NLPGetInequalityConstraintNumberOfGroup(P,I,J);
      if(J%3==2)fprintf(fid,"\n    ");
      NLPrintGroupShort(fid,P,i,J);
     }
    if(NLPIsInequalityConstraintUpperBoundSet(P,I))
      fprintf(fid,"<%lf",NLPGetInequalityConstraintUpperBound(P,I));
    fprintf(fid,"\n");
   }
  if(NLPGetNumberOfInequalityConstraints(P)>0)fprintf(fid,"\n");

  if(NLPGetNumberOfMinMaxConstraints(P)>0)fprintf(fid,"  MinMax Constraints:\n\n");
  for(I=0;I<NLPGetNumberOfMinMaxConstraints(P);I++)
   {
    fprintf(fid,"  %3d   %s\n",I,NLPGetGroupName(P,NLPGetMinMaxConstraintNumberOfGroup(P,I,0)));
    fprintf(fid,"        ");
    for(J=0;J<NLPGetNumberOfMinMaxConstraintGroups(P,I);J++)
     {
      i=NLPGetMinMaxConstraintNumberOfGroup(P,I,J);
      if(J%3==2)fprintf(fid,"\n    ");
      NLPrintGroupShort(fid,P,i,J);
     }
    fprintf(fid,"\n");
   }
  if(NLPGetNumberOfMinMaxConstraints(P)>0)fprintf(fid,"\n");

  fflush(stdout);
  return;
 }

void NLPrintGroupShort(FILE *fid,NLProblem P,int group,int first)
 {
  int j;
  double s;
  int nonTrivalGroup;

  s=1./NLPGetGroupScale(P,group);
  nonTrivalGroup=strcmp("TRIVIAL GROUP",NLPGetGroupType(P,NLPGetTypeOfGroup(P,group)) );

  if(first>0&&s>0)fprintf(fid,"+");

  if(fabs(s-1)>1.e-14)
   {
    if(itrunc(s)==s)fprintf(fid,"%d*",itrunc(s));
     else if(itrunc(s*10)==10*s)fprintf(fid,"%.1lf*",s);
     else if(itrunc(s*100)==100*s)fprintf(fid,"%.2lf*",s);
     else fprintf(fid,"%lf*(",s);
   }

  if(!nonTrivalGroup&&fabs(s-1)>1.e-14)fprintf(fid,"(");
  if(nonTrivalGroup)fprintf(fid,"\"%s\"(",NLPGetGroupType(P,NLPGetTypeOfGroup(P,group)));
  NLPrintElement(fid,P,group);
  if(nonTrivalGroup||fabs(s-1)>1.e-14)fprintf(fid,")");
  fflush(fid);

  return;
 }

void *NLPGetGroupFunctionData(NLProblem P, int group)
 {
  return P->groupFunctionData[group];
 }

void *NLPGetNonlinearElementData(NLProblem P, NLNonlinearElement element)
 {
  return NNLEGetData(P->nonlinearElement[element]);
 }

void NLPrintProblemOld(FILE *fid,NLProblem P)
 {
  char RoutineName[]="NLPrintProblemOld";
  int i,I,j,k;

  if(fid==(FILE*)NULL)
   {
    sprintf(NLProblemErrorMsg,"File pointer (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(P==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  fprintf(fid,"NAGroupPartialSeparableNonlinearOptimizationProblem %s\n",NLPGetProblemName(P));
  fprintf(fid,"\nVariables: %d\n\n",NLPGetNumberOfVariables(P));
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(NLPGetVariableName(P,i)!=(char*)NULL)
      fprintf(fid," %d  name=\"%s\" scale %lf",i,NLPGetVariableName(P,i),NLPGetVariableScale(P,i));
     else
      fprintf(fid," %d  name=\"X%d\" scale %lf",i,i+1,NLPGetVariableName(P,i),NLPGetVariableScale(P,i));
    if((NLPIsLowerSimpleBoundSet(P,i)&&NLPGetLowerSimpleBound(P,i)>-1.e20)||NLPIsUpperSimpleBoundSet(P,i))
     {
      if(NLPIsLowerSimpleBoundSet(P,i) && NLPGetLowerSimpleBound(P,i)>-1.e20)fprintf(fid,"    %lf <=",NLPGetLowerSimpleBound(P,i));
      fprintf(fid,"x[%d]",i);
      if(NLPIsUpperSimpleBoundSet(P,i))fprintf(fid," <= %lf",NLPGetUpperSimpleBound(P,i));
      fprintf(fid,"\n");
     }else 
      fprintf(fid,"\n");
   }

  fprintf(fid,"\nObjective Function: \n\n");
  fprintf(fid,"  Bounds: [");
  if(NLPGetObjectiveLowerBound(P)!=DBL_MIN)
    fprintf(fid,"%lf",NLPGetObjectiveLowerBound(P));
   else
    fprintf(fid,"-infinity");
  fprintf(fid,",");
  if(NLPGetObjectiveUpperBound(P)!=DBL_MAX)
    fprintf(fid,"%lf",NLPGetObjectiveUpperBound(P));
   else
    fprintf(fid,"infinity");
  fprintf(fid,"]\n");
  fprintf(fid,"  Number Of Groups: %d\n",NLPGetNumberOfGroupsInObjective(P));
  for(I=0;I<NLPGetNumberOfGroupsInObjective(P);I++)
   {
    i=NLPGetObjectiveGroupNumber(P,I);
    fprintf(fid,"\n   Group: %d number %d\n",I,i);
    fprintf(fid,"    Name: %s\n",NLPGetGroupName(P,i));
    fprintf(fid,"    Scale: %lf\n",1./NLPGetGroupScale(P,i));
    fprintf(fid,"    Group Function: ");
      fprintf(fid,"%d %s\n",NLPGetTypeOfGroup(P,i),NLPGetGroupType(P,NLPGetTypeOfGroup(P,i)));
/*  if(NLPIsGroupFunctionSet(P,i))
     else
      fprintf(fid,"default\n");*/
    fprintf(fid,"    a: ",NLPGetGroupA(P,i));fflush(stdout);
    if(NLPIsGroupASet(P,i))
      NLPrintVector(fid,NLPGetGroupA(P,i));
     else
      fprintf(fid,"default");
    fprintf(fid,"\n");
    if(NLPIsGroupBSet(P,i))
      fprintf(fid,"    b: %lf\n",NLPGetGroupB(P,i));
     else
      fprintf(fid,"    b: default\n");
    fprintf(fid,"    Number Of Elements: %d\n",NLPGetNumberOfElementsInGroup(P,i));
    for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
     {
      fprintf(fid,"        Element: %d is nonlinear element %d (%s), ",j,NLPGetNonlinearElementOfGroup(P,i,j),NLNEGetName(P,NLPGetNonlinearElementOfGroup(P,i,j)));
      if(NLPIsElementFunctionSet(P,i,j))
        fprintf(fid,"Weight: %lf\n",NLPGetElementWeight(P,i,j));
       else
        fprintf(fid,"Weight: default\n");
     }
   }

  fprintf(fid,"\nConstraints: \n\n");

  fprintf(fid,"Nonlinear Equality Constraints: %d\n\n",NLPGetNumberOfEqualityConstraints(P));
  for(I=0;I<NLPGetNumberOfEqualityConstraints(P);I++)
   {
    i=NLPGetEqualityConstraintGroupNumber(P,I);
    fprintf(fid,"      Equality Constraint: %d (group %d)\n",I,i);
    fprintf(fid,"       Name: %s\n",NLPGetGroupName(P,i));
    fprintf(fid,"       Scale: %lf\n",1./NLPGetGroupScale(P,i));
    fprintf(fid,"       Group Function: ");
    fprintf(fid,"%d %s\n",NLPGetTypeOfGroup(P,i),NLPGetGroupType(P,NLPGetTypeOfGroup(P,i)));
    fprintf(fid,"       a: ");
    if(NLPIsGroupASet(P,i))
      NLPrintVector(fid,NLPGetGroupA(P,i));
     else
      fprintf(fid,"default");
    fprintf(fid,"\n");
    if(NLPIsGroupBSet(P,i))
      fprintf(fid,"       b: %lf\n",NLPGetGroupB(P,i));
     else
      fprintf(fid,"       b: default\n");
    fprintf(fid,"       Number Of Elements: %d\n",NLPGetNumberOfElementsInGroup(P,i));
    for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
     {
      fprintf(fid,"        Element: %d is nonlinear element %d, ",j,NLPGetNonlinearElementOfGroup(P,i,j));
      if(NLPIsElementFunctionSet(P,i,j))
        fprintf(fid,"Weight: %lf\n",NLPGetElementWeight(P,i,j));
       else
        fprintf(fid,"Weight: default\n");
     }
    fprintf(fid,"\n");
   }

  fprintf(fid,"Nonlinear Inequality Constraints: %d\n\n",NLPGetNumberOfInequalityConstraints(P));
  for(I=0;I<NLPGetNumberOfInequalityConstraints(P);I++)
   {
    i=NLPGetInequalityConstraintGroupNumber(P,I);
    fprintf(fid,"      Inequality Constraint: %d (group %d)\n",I,i);
    fprintf(fid,"       Name: %s\n",NLPGetGroupName(P,i));
    fprintf(fid,"       Scale: %lf\n",1./NLPGetGroupScale(P,i));
    fprintf(fid,"       Group Function: ");
    fprintf(fid,"%d %s\n",NLPGetTypeOfGroup(P,i),NLPGetGroupType(P,NLPGetTypeOfGroup(P,i)));
    fprintf(fid,"       a: ");
    if(NLPIsGroupASet(P,i))
      NLPrintVector(fid,NLPGetGroupA(P,i));
     else
      fprintf(fid,"default");
    fprintf(fid,"\n");
    if(NLPIsGroupBSet(P,i))
      fprintf(fid,"       b: %lf\n",NLPGetGroupB(P,i));
     else
      fprintf(fid,"       b: default\n");
    fprintf(fid,"       Number Of Elements: %d\n",NLPGetNumberOfElementsInGroup(P,i));
    for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
     {
      fprintf(fid,"        Element: %d is nonlinear element %d, ",j,NLPGetNonlinearElementOfGroup(P,i,j));
      if(NLPIsElementWeightSet(P,i,j))
        fprintf(fid,"Weight: %lf\n",NLPGetElementWeight(P,i,j));
       else
        fprintf(fid,"Weight: Default\n");
     }
    fprintf(fid,"       Bounds: [");
    if(NLPGetInequalityConstraintLowerBound(P,I)!=DBL_MIN)
      fprintf(fid,"%lf",NLPGetInequalityConstraintLowerBound(P,I));
     else
      fprintf(fid,"-infinity");
    fprintf(fid,",");
    if(NLPGetInequalityConstraintUpperBound(P,I)!=DBL_MAX)
      fprintf(fid,"%lf",NLPGetInequalityConstraintUpperBound(P,I));
     else
      fprintf(fid,"infinity");
    fprintf(fid,"]\n");
    fprintf(fid,"\n");
   }
  fprintf(fid,"MinMax Constraints: %d\n\n",NLPGetNumberOfMinMaxConstraints(P));
  for(I=0;I<NLPGetNumberOfMinMaxConstraints(P);I++)
   {
    i=NLPGetMinMaxConstraintGroupNumber(P,I);
    fprintf(fid,"      MinMax Constraint: %d (group %d)\n",I,i);
    fprintf(fid,"       Name: %s\n",NLPGetGroupName(P,i));
    fprintf(fid,"       Scale: %lf\n",1./NLPGetGroupScale(P,i));
    fprintf(fid,"       Group Function: ");
    fprintf(fid,"%d %s\n",NLPGetTypeOfGroup(P,i),NLPGetGroupType(P,NLPGetTypeOfGroup(P,i)));
    fprintf(fid,"       a: ");
    if(NLPIsGroupASet(P,i))
      NLPrintVector(fid,NLPGetGroupA(P,i));
     else
      fprintf(fid,"default");
    fprintf(fid,"\n");
    if(NLPIsGroupBSet(P,i))
      fprintf(fid,"       b: %lf\n",NLPGetGroupB(P,i));
     else
      fprintf(fid,"       b: default\n");
    fprintf(fid,"       Number Of Elements: %d\n",NLPGetNumberOfElementsInGroup(P,i));
    for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
     {
      fprintf(fid,"        Element: %d is nonlinear element %d, ",j,NLPGetNonlinearElementOfGroup(P,i,j));
      if(NLPIsElementWeightSet(P,i,j))
        fprintf(fid,"Weight: %lf\n",NLPGetElementWeight(P,i,j));
       else
        fprintf(fid,"Weight: Default\n");
     }
    fprintf(fid,"\n");
   }

  fprintf(fid,"Groups: \n\n",NLPGetNumberOfGroups(P));

  for(i=0;i<NLPGetNumberOfGroups(P);i++)
   {
    fprintf(fid," Group %d",i);
    NLPrintGroupOld(fid,P,i);
   }

  fprintf(fid,"Nonlinear Elements: \n\n",NLPGetNumberOfNonlinearElements(P));

  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
   {
    fprintf(fid," Nonlinear Element %d",i);
    NLPrintNonlinearElementOld(fid,P,i);
   }

  fprintf(fid," Summary: \n\n");
  fprintf(fid," Total Number of Groups: %d\n",NLPGetNumberOfGroups(P));
  for(i=0;i<NLPGetNumberOfGroups(P);i++)
   {
    if(NLPGetTypeOfGroup(P,i)>-1)
      fprintf(fid,"  Group %d type %d (%s)\n",i,NLPGetTypeOfGroup(P,i),NLPGetGroupType(P,NLPGetTypeOfGroup(P,i)));
     else
      fprintf(fid,"  Group %d no type ",i);
   }
  fprintf(fid,"\n Total Number of Group Functions: %d\n",NLPGetNumberOfGroupTypes(P));
  for(i=0;i<NLPGetNumberOfGroupTypes(P);i++)
   {
    fprintf(fid,"  Group Function %d %s\n",i,NLPGetGroupType(P,i));
   }
  fprintf(fid,"\n Total Number of Nonlinear Elements: %d\n",NLPGetNumberOfNonlinearElements(P));
  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
   {
    fprintf(fid,"  Nonlinear Element %d %s\n",i,NLNEGetName(P,i));
   }

  fprintf(fid,"\n Total Number of Element Functions: %d\n",NLPGetNumberOfElementTypes(P));
  for(i=0;i<NLPGetNumberOfElementTypes(P);i++)
   {
    fprintf(fid,"  Element Function %d %s\n",i,NLPGetElementType(P,i));
   }

  fflush(stdout);
  return;
 }

void NLPrintGroupOld(FILE *fid,NLProblem P,int group)
 {
  int j;

  fprintf(fid,"\n");
  fprintf(fid,"      Name: %s\n",NLPGetGroupName(P,group));
  fprintf(fid,"      Scale: %lf\n",1./NLPGetGroupScale(P,group));
  fprintf(fid,"      Group Function: ");
  fprintf(fid,"%d %s\n",NLPGetTypeOfGroup(P,group),NLPGetGroupType(P,NLPGetTypeOfGroup(P,group)));
  fprintf(fid,"      a: ");
  if(NLPIsGroupASet(P,group))
    NLPrintVector(fid,NLPGetGroupA(P,group));
   else
    fprintf(fid,"default");
  fprintf(fid,"\n");
  if(NLPIsGroupBSet(P,group))
    fprintf(fid,"      b: %lf\n",NLPGetGroupB(P,group));
   else
    fprintf(fid,"      b: default\n");
  fprintf(fid,"      Number Of Elements: %d\n",NLPGetNumberOfElementsInGroup(P,group));
  for(j=0;j<NLPGetNumberOfElementsInGroup(P,group);j++)
   {
    fprintf(fid,"       Element: %d is nonlinear element %d, ",j,NLPGetNonlinearElementOfGroup(P,group,j));
    if(NLPIsElementWeightSet(P,group,j))
      fprintf(fid,"Weight: %lf\n",NLPGetElementWeight(P,group,j));
     else
      fprintf(fid,"Weight: Default\n");
   }
  fprintf(fid,"\n");

  return;
 }

void NLPrintNonlinearElementOld(FILE *fid,NLProblem P,NLNonlinearElement i)
 {
  int j,n;
  double *H0;
  int k;

  fprintf(fid,"\n");
  fprintf(fid,"          Name: %s\n",NLNEGetName(P,i));
  fprintf(fid,"          Element Function: ");
  if(NLNEGetElementFunction(P,i)!=(NLElementFunction)NULL)
    fprintf(fid,"type %d (%s)\n",NLEGetType(NLNEGetElementFunction(P,i)),NLPGetElementType(P,NLEGetType(NLNEGetElementFunction(P,i))));
   else
    fprintf(fid," default\n");
  fprintf(fid,"          Number of Element Variables: %d\n",NLNEGetElementDimension(P,i));
  fprintf(fid,"          Number of Internal Variables: %d\n",NLNEGetInternalDimension(P,i));
  fprintf(fid,"          Number of Unknowns: %d\n",NLNEGetElementDimension(P,i));
  fprintf(fid,"          Unknowns: (");
  for(k=0;k<NLNEGetElementDimension(P,i);k++)
   {
    if(k>0)fprintf(fid,",");
    fprintf(fid,"%d",NLNEGetIndex(P,i,k));
   }
  fprintf(fid,")\n");
  fprintf(fid,"          Range Transformation:");
  if(NNLEGetRangeXForm(P,i)!=(NLMatrix)NULL)
   {
    fprintf(fid,"\n");
    NLPrintMatrix(fid,NNLEGetRangeXForm(P,i));
   }else
    fprintf(fid," Identity\n");
  H0=NLElementFunctionGetInitialHessianMatrix(NLNEGetElementFunction(P,i));
  if(H0!=(double*)NULL)
   {
    fprintf(fid,"          Initial Hessian\n");
    n=NLNEGetInternalDimension(P,i);
    for(j=0;j<n;j++)
     {
      fprintf(fid,"          [");
      for(k=0;k<n;k++)
       {
        if(k>0)fprintf(fid,",");
        fprintf(fid,"%lf",H0[j+n*k]);
       }
      fprintf(fid,"]\n");
     }
   }else
    fprintf(fid,"          No Initial Hessian\n");

  fprintf(fid,"\n");
  return;
 }

void NLPSetAllElementUpdateTypes(NLProblem P, char *type, int override)
 {
  int ie;

  for(ie=0;ie<P->nNonlinearElements;ie++)
   {
    NLSetElementUpdateType(NLNEPGetElementFunction((P->nonlinearElement)[ie]),type,override);
   }
  return;
 }

void NLPSetAllElementUpdateNoises(NLProblem P, double NoiseLevel)
 {
  int ie;

  for(ie=0;ie<P->nNonlinearElements;ie++)
   {
    NLSetElementUpdateNoise(NLNEPGetElementFunction((P->nonlinearElement)[ie]),NoiseLevel);
   }
  return;
 }

void NLPConvertToEqualityAndBoundsOnly(NLProblem P)
 {
  char RoutineName[]="NLPConvertToEqualityAndBoundsOnly";

/* Local variables */

  int i,n;
  int z,s0,nSlacks,nMinMax,nInEq,nEq;
  int nV;
  int objGroup;
  NLVector a=(NLVector)NULL;
  int c;
  int m;
  int j,k;
  char constrName[1024];
  int g;
  int cgroup;
  double u;
  double l;
  double b;
  double scl;
  int rc;
  int verbose=0;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return;
   }

  n=NLPGetNumberOfVariables(P);
  nEq=NLPGetNumberOfEqualityConstraints(P);
  nInEq=NLPGetNumberOfInequalityConstraints(P);
  nMinMax=NLPGetNumberOfMinMaxConstraints(P);
  nSlacks=nInEq+nMinMax;
  if(verbose){printf(" Problem %s has %d Equality Constraints, %d InEq\n",NLPGetProblemName(P),nEq,nInEq);fflush(stdout);}

  if(nMinMax>0)
   {
    z=n;
    s0=n+1;
    nV=1+nSlacks;
   }else{
    s0=n;
    z=-1;
    nV=nSlacks;
   }

/* Add variables - this is rather expensive, since each linear part has to
     be extended. I'm doing all the variables together, so it is only done
     once per minimize */

  NLPAddVariables(P,nV);

/* Add the minmax term to the objective and convert the minmax constraints to Inequalities. */

  if(nMinMax>0)
   {
    NLPSetVariableName(P,z,"MnMxZ");
    NLPSetVariableScale(P,z,1.);

    objGroup=NLPAddGroupToObjective(P,"minmax");
    a=NLCreateVector(n+nV);
    NLVSetC(a,z,1.);
    NLPSetObjectiveGroupA(P,objGroup,a);
    NLFreeVector(a);
    NLPUnSetLowerSimpleBound(P,z);
    NLPUnSetUpperSimpleBound(P,z);

    for(i=0;i<nMinMax;i++)
     {
      c=NLPAddNonlinearInequalityConstraint(P,(char*)NULL);
      m=NLPGetNumberOfGroupsInMinMaxConstraint(P,i);
      if(m>1)
       {
        sprintf(NLProblemErrorMsg,"MinMax Constraint %d has %d groups,only one is allowed currently and it must be the trivial group function",i,m);
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return;
       }

      cgroup=NLPGetMinMaxConstraintNumberOfGroup(P,i,0);
      if(NLPIsGroupFunctionSet(P,cgroup))
       {
        sprintf(NLProblemErrorMsg,"MinMax Constraint %d group must currently have the trivial group function",i);
        NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        return;
       }

      for(j=0;j<m;j++)
       {
        sprintf(constrName,"MnMx%s",NLPGetGroupName(P,cgroup));
        g=NLPAddGroupToInequalityConstraint(P,c,constrName);

        scl=NLPGetMinMaxConstraintGroupScale(P,i,0);
        NLPSetInequalityConstraintGroupScale(P,c,g,scl);

        if(NLPGetMinMaxConstraintGroupA(P,i,j)!=(NLVector)NULL)
         {
          a=NLPGetMinMaxConstraintGroupA(P,i,j);
          NLVSetC(a,z,-scl);
          NLPSetInequalityConstraintGroupA(P,c,g,a);
         }else{
          a=NLCreateVector(n+nV);
          NLVSetC(a,z,-scl);
          NLPSetInequalityConstraintGroupA(P,c,g,a);
          NLFreeVector(a);
         }

        if(NLPIsGroupBSet(P,NLPGetMinMaxConstraintNumberOfGroup(P,i,j)))
          NLPSetInequalityConstraintGroupB(P,c,g,NLPGetMinMaxConstraintGroupB(P,i,j));
        cgroup=NLPGetMinMaxConstraintNumberOfGroup(P,i,j);
        for(k=0;k<NLPGetNumberOfElementsInGroup(P,cgroup);k++)
         {
          NLPAddNonlinearElementToInequalityConstraintGroup(P,c,j,
             NLPGetElementWeight(P,cgroup,k),
             NLPGetGroupNonlinearElement(P,cgroup,k));
         }
       }
      sprintf(constrName,"minmaxConstraint%dMinusZ",i);

      NLPSetInequalityConstraintUpperBound(P,c,0.);
      NLPUnSetInequalityConstraintLowerBound(P,c);
     }
    NLPHideMinMaxConstraints(P);
   }

/* Add the slacks to the inequalities and convert them to equalities */

  for(i=0;i<nInEq+nMinMax;i++)
   {

/* If constraint is a single trivial group, just change a. */

    m=NLPGetNumberOfGroupsInInequalityConstraint(P,i);
    if(m>1)
     {
      sprintf(NLProblemErrorMsg,"Inequality Constraint %d has %d groups, only one is allowed currently and it must be the trivial group function",i,m);
      NLSetError(4,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return;
     }

    if(i<nInEq)
     {
      sprintf(constrName,"InEqConstr%d",i);
     }else{
      sprintf(constrName,"MinMaxConstr%d",i-nInEq);
     }

    cgroup=NLPGetInequalityConstraintNumberOfGroup(P,i,0);
    sprintf(constrName,"InEq%s",NLPGetGroupName(P,cgroup));
    c=NLPAddNonlinearEqualityConstraint(P,constrName);
    sprintf(constrName,"Sl%s",NLPGetGroupName(P,cgroup));

    if(z>0)
     {
      NLPSetVariableName(P,n+i+1,constrName);
      NLPSetVariableScale(P,n+i+1,1.);
     }else{
      NLPSetVariableName(P,n+i,constrName);
      NLPSetVariableScale(P,n+i,1.);
     }

    if(NLPIsGroupFunctionSet(P,cgroup))
     {
      sprintf(NLProblemErrorMsg,"Inequality Constraint %d group must currently have the trivial group function",i);
      NLSetError(0,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      return;
     }
    g=0;
    if(NLPGetGroupA(P,cgroup)==(NLVector)NULL)
     {
      a=NLCreateVector(n+nV);
      NLPSetEqualityConstraintGroupA(P,c,g,a);
      NLFreeVector(a);
     }else{
      NLPSetEqualityConstraintGroupA(P,c,g,NLPGetGroupA(P,cgroup));
     }
    if(NLPIsGroupBSet(P,cgroup))
     {
      NLPSetEqualityConstraintGroupB(P,c,g,NLPGetGroupB(P,cgroup));
     }

    scl=NLPGetInequalityConstraintGroupScale(P,i,0);
    NLPSetEqualityConstraintGroupScale(P,c,g,scl);

    for(j=0;j<NLPGetNumberOfElementsInGroup(P,cgroup);j++)
     {
      NLPAddNonlinearElementToEqualityConstraintGroup(P,c,g,
         NLPGetElementWeight(P,cgroup,j),
         NLPGetGroupNonlinearElement(P,cgroup,j));
     }

    u=NLPGetInequalityConstraintUpperBound(P,i);
    l=NLPGetInequalityConstraintLowerBound(P,i);
    if(u>1.e19)NLPUnSetInequalityConstraintUpperBound(P,i);
    if(l<-1.e19)NLPUnSetInequalityConstraintLowerBound(P,i);

    if(NLPIsInequalityConstraintUpperBoundSet(P,i) && !NLPIsInequalityConstraintLowerBoundSet(P,i))
     {
      b=NLPGetEqualityConstraintGroupB(P,c,g);
      NLPSetEqualityConstraintGroupB(P,c,g,b+u);
      NLVSetC(NLPGetEqualityConstraintGroupA(P,c,g),s0+i,scl);
      NLPSetLowerSimpleBound(P,s0+i,0.);
      NLPUnSetUpperSimpleBound(P,s0+i);
     }else if(!NLPIsInequalityConstraintUpperBoundSet(P,i) && NLPIsInequalityConstraintLowerBoundSet(P,i))
     {
      b=NLPGetEqualityConstraintGroupB(P,c,g);
      NLPSetEqualityConstraintGroupB(P,c,g,b+l);
      NLVSetC(NLPGetEqualityConstraintGroupA(P,c,g),s0+i,-scl);
      NLPSetLowerSimpleBound(P,s0+i,0.);
      NLPUnSetUpperSimpleBound(P,s0+i);
     }else{
      b=NLPGetEqualityConstraintGroupB(P,c,g);
      NLPSetEqualityConstraintGroupB(P,c,g,b+l);
      NLVSetC(NLPGetEqualityConstraintGroupA(P,c,g),s0+i,-scl);
      NLPSetSimpleBounds(P,s0+i,0.,(u-l)/scl);
     }
   }

  NLPHideInequalityConstraints(P);

  if(verbose)
   {
    printf("-------------------------------------------------------------\n");
    printf("\n\nAfter converting equalities and simple bounds\n\n");
    NLPrintProblemShort(stdout,P);
    printf("-------------------------------------------------------------\n");
    fflush(stdout);
   }

  return;
 }

int NLPSetInvGroupScale(NLProblem this,int group,double s)
 {
  char RoutineName[]="NLPSetInvGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->groupScale[group]=s;
  return 1;
 }

double NLPGetInvGroupScale(NLProblem this,int group)
 {
  char RoutineName[]="NLPGetInvGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(!(group>-1&&group<this->nGroups))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nGroups-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->groupScale[group]);
 }

double NLPGetInequalityConstraintGroupScale(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetInequalityConstraintGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nInequalityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];

  return NLPGetGroupScale(this,group);
 }

double NLPGetEqualityConstraintGroupScale(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetEqualityConstraintGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nEqualityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];

  return NLPGetGroupScale(this,group);
 }

double NLPGetMinMaxConstraintGroupScale(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetMinMaxConstraintGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nMinMaxConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];

  return NLPGetGroupScale(this,group);
 }

double NLPGetInequalityConstraintInvGroupScale(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetInequalityConstraintInvGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nInequalityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nInequalityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nInequalityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nInequalityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->inequalityConstraintGroups[constraint])[group];

  return NLPGetInvGroupScale(this,group);
 }

double NLPGetEqualityConstraintInvGroupScale(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetEqualityConstraintInvGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nEqualityConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nEqualityConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nEqualityConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nEqualityConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->equalityConstraintGroups[constraint])[group];

  return NLPGetInvGroupScale(this,group);
 }

double NLPGetMinMaxConstraintInvGroupScale(NLProblem this,int constraint,int group)
 {
  char RoutineName[]="NLPGetMinMaxConstraintInvGroupScale";
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(constraint<0 || constraint>=this->nMinMaxConstraints)
   {
    sprintf(NLProblemErrorMsg,"Constraint %d (argument 2) is Invalid, must be in [0,%d).",constraint,this->nMinMaxConstraints);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(!(group>-1&&group<this->nMinMaxConstraintGroups[constraint]))
   {
    sprintf(NLProblemErrorMsg,"Group %d is illegal (argument 2). Must be in range 0 to %d",group,this->nMinMaxConstraintGroups[constraint]-1);
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  group=(this->minMaxConstraintGroups[constraint])[group];

  return NLPGetInvGroupScale(this,group);
 }

NLProblem NLCopyProblem(NLProblem P)
 {
  char RoutineName[]="NLCopyProblem";
  int i,j;
  int nV;
  NLProblem this;
  int verbose;

  verbose=0;

  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  this=(NLProblem)malloc(sizeof(struct NLGrpPart));
  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPart));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }

  this->problemName=(char*)malloc((strlen(P->problemName)+1)*sizeof(char));
  if(this->problemName==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(P->problemName)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  strcpy(this->problemName,P->problemName);

  nV=P->nVariables;
  this->nVariables=P->nVariables;
  this->variableScale=(double*)malloc(sizeof(double)*this->nVariables);
  if(this->variableScale==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nV*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }

  this->variableName=(char**)malloc(nV*sizeof(char*));
  if(this->variableName==(char**)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",nV*sizeof(char*));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  for(i=0;i<nV;i++)
   {
    this->variableScale[i]=P->variableScale[i];
    this->variableName[i]=(char*)malloc(sizeof(char)*40);
    if(this->variableName[i]==(char*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",40*sizeof(char));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    strcpy(this->variableName[i],P->variableName[i]);
   }

  this->cache=P->cache;

  this->nGroups=P->nGroups;
  this->mGroups=P->mGroups;
  if(this->mGroups>0)
   {
    this->groupName=(char**)malloc((this->mGroups)*sizeof(char*));
    if(this->groupName==(char**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mGroups)*sizeof(char*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
  
    this->groupFunction=(NLGroupFunction*)malloc((this->mGroups)*sizeof(NLGroupFunction));
    if(this->groupFunction==(NLGroupFunction*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mGroups)*sizeof(NLGroupFunction));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
  
    this->groupFunctionData=(void**)malloc((this->mGroups)*sizeof(void*));
    this->freeGroupFunctionData=(groupFunctionDataFreer*)malloc((this->mGroups)*sizeof(groupFunctionDataFreer));
    this->groupA=(NLVector*)malloc((this->mGroups)*sizeof(NLVector));
    this->groupB=(double*)malloc((this->mGroups)*sizeof(double));
    this->groupBSet=(int*)malloc((this->mGroups)*sizeof(int));
    this->groupScale=(double*)malloc((this->mGroups)*sizeof(double));
    this->nElementsInGroup=(int*)malloc((this->mGroups)*sizeof(int));
    this->mElementsInGroup=(int*)malloc((this->mGroups)*sizeof(int));
    this->elementWeight=(double**)malloc((this->mGroups)*sizeof(double*));
    this->elementWeightSet=(int**)malloc((this->mGroups)*sizeof(int*));
    this->element=(int**)malloc((this->mGroups)*sizeof(int*));
    this->groupCached=(int*)malloc((this->mGroups)*sizeof(int));
    this->groupValue=(double*)malloc((this->mGroups)*sizeof(double));
    this->groupGradient=(double*)malloc((this->mGroups)*sizeof(double));
    this->groupHessian=(double*)malloc((this->mGroups)*sizeof(double));
  
    for(i=0;i<this->nGroups;i++)
     {
      this->groupName[i]=(char*)malloc((strlen(P->groupName[i])+1)*sizeof(char*));
      if(this->groupName[i]==(char*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(P->groupName[i])+1)*sizeof(char));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s",NLProblemErrorMsg);fflush(stdout);
        return (NLProblem)NULL;
       }
      strcpy(this->groupName[i],P->groupName[i]);
      this->groupFunction[i]=P->groupFunction[i];if(P->groupFunction[i]!=(NLGroupFunction)NULL)NLRefGroupFunction(P->groupFunction[i]);
      this->groupFunctionData[i]=P->groupFunctionData[i];
      this->freeGroupFunctionData[i]=P->freeGroupFunctionData[i];
      if(P->groupA[i]!=(NLVector)NULL)
        this->groupA[i]=NLCopyVector(P->groupA[i]);
       else
        this->groupA[i]=(NLVector)NULL;
      this->groupB[i]=P->groupB[i];
      this->groupBSet[i]=P->groupBSet[i];
      this->groupScale[i]=P->groupScale[i];
      this->nElementsInGroup[i]=P->nElementsInGroup[i];
      this->mElementsInGroup[i]=P->mElementsInGroup[i];
      this->elementWeight[i]=(double*)malloc((this->mElementsInGroup[i])*sizeof(double));
      this->elementWeightSet[i]=(int*)malloc((this->mElementsInGroup[i])*sizeof(int));
      this->element[i]=(int*)malloc((this->mElementsInGroup[i])*sizeof(int));
      for(j=0;j<this->nElementsInGroup[i];j++)
       {
        (this->element[i])[j]=(P->element[i])[j];
        (this->elementWeight[i])[j]=(P->elementWeight[i])[j];
        (this->elementWeightSet[i])[j]=(P->elementWeightSet[i])[j];
       }
      this->groupCached[i]=P->groupCached[i];
      this->groupValue[i]=P->groupValue[i];
      this->groupGradient[i]=P->groupGradient[i];
      this->groupHessian[i]=P->groupHessian[i];
     }
    }else{
     this->groupName=(char**)NULL;
    }

  this->nGroupTypes=P->nGroupTypes;
  this->mGroupTypes=P->mGroupTypes;
  if(this->mGroupTypes>0)
   {
    this->groupTypeName=(char**)malloc((this->mGroupTypes)*sizeof(char*));
    for(i=0;i<this->nGroupTypes;i++)
     {
      this->groupTypeName[i]=(char*)malloc((strlen(P->groupTypeName[i])+1)*sizeof(char));
      if(this->groupTypeName[i]==(char*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(P->groupTypeName[i])+1)*sizeof(char));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s",NLProblemErrorMsg);fflush(stdout);
        return (NLProblem)NULL;
       }
      strcpy(this->groupTypeName[i],P->groupTypeName[i]);
     }
   }else{
    this->groupTypeName=(char**)NULL;
   }

  this->nNonlinearElements=P->nNonlinearElements;
  this->mNonlinearElements=P->mNonlinearElements;
  if(this->mNonlinearElements>0)
   {
    this->nonlinearElement=(NLNonlinearElementPtr*)malloc((this->mNonlinearElements)*sizeof(NLNonlinearElementPtr));
    if(this->nonlinearElement==(NLNonlinearElementPtr*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(NLNonlinearElementPtr));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->elementCached=(int*)malloc((this->mNonlinearElements)*sizeof(int));
    if(this->elementCached==(int*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->internalVariables=(double**)malloc((this->mNonlinearElements)*sizeof(double*));
    if(this->internalVariables==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->elementVariables=(double**)malloc((this->mNonlinearElements)*sizeof(double*));
    if(this->elementVariables==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->elementValue=(double*)malloc((this->mNonlinearElements)*sizeof(double));
    if(this->elementValue==(double*)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->elementGradient=(double**)malloc((this->mNonlinearElements)*sizeof(double*));
    if(this->elementGradient==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->elementHessian=(double**)malloc((this->mNonlinearElements)*sizeof(double*));
    if(this->elementHessian==(double**)NULL)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonlinearElements)*sizeof(double*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
  
    for(i=0;i<this->nNonlinearElements;i++)
     {
      this->nonlinearElement[i]=P->nonlinearElement[i];NLRefNonlinearElement(P->nonlinearElement[i]);
      this->elementCached[i]=P->elementCached[i];
      this->internalVariables[i]=(double*)NULL;
      this->elementValue[i]=P->elementValue[i];
      this->elementVariables[i]=(double*)NULL;
      this->elementGradient[i]=(double*)NULL;
      this->elementHessian[i]=(double*)NULL;
     }
   }else{
    this->nonlinearElement=(NLNonlinearElementPtr*)NULL;
    this->elementCached=(int*)NULL;
    this->internalVariables=(double**)NULL;
    this->elementVariables=(double**)NULL;
    this->elementValue=(double*)NULL;
    this->elementGradient=(double**)NULL;
    this->elementHessian=(double**)NULL;
   }
  
  this->nElementTypes=P->nElementTypes;
  this->mElementTypes=P->mElementTypes;
  if(this->mElementTypes>0)
   {
    this->elementTypeName=(char**)malloc((this->mElementTypes)*sizeof(char*));
    for(i=0;i<this->nElementTypes;i++)
     {
      this->elementTypeName[i]=(char*)malloc((strlen(P->elementTypeName[i])+1)*sizeof(char));
      if(this->elementTypeName[i]==(char*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(P->elementTypeName[i])+1)*sizeof(char));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s",NLProblemErrorMsg);fflush(stdout);
        return (NLProblem)NULL;
       }
      strcpy(this->elementTypeName[i],P->elementTypeName[i]);
     }
   }else{
    this->elementTypeName=(char**)NULL;
   }

  this->nGroupsInObjective=P->nGroupsInObjective;
  this->mGroupsInObjective=P->mGroupsInObjective;
  if(this->mGroupsInObjective>0)
   {
    this->groupsInObjective=(int*)malloc((this->mGroupsInObjective)*sizeof(int));
    if(this->groupsInObjective==(int*)NULL&&this->mGroupsInObjective>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mGroupsInObjective)*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    for(i=0;i<this->nGroupsInObjective;i++)(this->groupsInObjective)[i]=(P->groupsInObjective)[i];
   }else{
    this->groupsInObjective=(int*)NULL;
   }
  this->lowerBoundOnObjective=P->lowerBoundOnObjective;
  this->upperBoundOnObjective=P->upperBoundOnObjective;

  this->simpleConstraintLowerBound=(double*)malloc(sizeof(double)*this->nVariables);
  if(this->simpleConstraintLowerBound==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  this->simpleConstraintUpperBound=(double*)malloc(sizeof(double)*this->nVariables);
  if(this->simpleConstraintUpperBound==(double*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->nVariables*sizeof(double));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return (NLProblem)NULL;
   }
  for(i=0;i<this->nVariables;i++)
   {
    this->simpleConstraintLowerBound[i]=P->simpleConstraintLowerBound[i];
    this->simpleConstraintUpperBound[i]=P->simpleConstraintUpperBound[i];
   }

  this->hideEqualityConstraints=P->hideEqualityConstraints;
  this->nEqualityConstraints=P->nEqualityConstraints;
  this->mEqualityConstraints=P->mEqualityConstraints;
  if(this->mEqualityConstraints>0)
   {
    this->nEqualityConstraintGroups=(int*)malloc((this->mEqualityConstraints)*sizeof(int));
    if(this->nEqualityConstraintGroups==(int*)NULL && this->mEqualityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mEqualityConstraints*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->mEqualityConstraintGroups=(int*)malloc((this->mEqualityConstraints)*sizeof(int));
    if(this->mEqualityConstraintGroups==(int*)NULL && this->mEqualityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mEqualityConstraints*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->equalityConstraintGroups=(int**)malloc((this->mEqualityConstraints)*sizeof(int*));
    if(this->equalityConstraintGroups==(int**)NULL && this->mEqualityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mEqualityConstraints*sizeof(int*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    for(i=0;i<this->nEqualityConstraints;i++)
     {
      (this->nEqualityConstraintGroups)[i]=(P->nEqualityConstraintGroups)[i];
      (this->mEqualityConstraintGroups)[i]=(P->mEqualityConstraintGroups)[i];
      this->equalityConstraintGroups[i]=(int*)malloc((this->mEqualityConstraintGroups)[i]*sizeof(int));
      if(this->equalityConstraintGroups[i]==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mEqualityConstraintGroups)[i]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s",NLProblemErrorMsg);fflush(stdout);
        return (NLProblem)NULL;
       }
      for(j=0;j<this->nEqualityConstraintGroups[i];j++)
       (this->equalityConstraintGroups[i])[j]=(P->equalityConstraintGroups[i])[j];
     }
   }else{
    this->nEqualityConstraintGroups=(int*)NULL;
    this->mEqualityConstraintGroups=(int*)NULL;
    this->equalityConstraintGroups=(int**)NULL;
   }

  this->hideMinMaxConstraints=P->hideMinMaxConstraints;
  this->nMinMaxConstraints=P->nMinMaxConstraints;
  this->mMinMaxConstraints=P->mMinMaxConstraints;
  this->zgroupnumber=P->zgroupnumber;
  if(this->mMinMaxConstraints>0)
   {
    this->nMinMaxConstraintGroups=(int*)malloc((this->mMinMaxConstraints)*sizeof(int));
    if(this->nMinMaxConstraintGroups==(int*)NULL && this->mMinMaxConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mMinMaxConstraints*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->mMinMaxConstraintGroups=(int*)malloc((this->mMinMaxConstraints)*sizeof(int));
    if(this->mMinMaxConstraintGroups==(int*)NULL && this->mMinMaxConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mMinMaxConstraints*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->minMaxConstraintGroups=(int**)malloc((this->mMinMaxConstraints)*sizeof(int*));
    if(this->minMaxConstraintGroups==(int**)NULL && this->mMinMaxConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mMinMaxConstraints*sizeof(int*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    for(i=0;i<this->nMinMaxConstraints;i++)
     {
      (this->nMinMaxConstraintGroups[i])=(P->nMinMaxConstraintGroups[i]);
      (this->mMinMaxConstraintGroups[i])=(P->mMinMaxConstraintGroups[i]);
      this->minMaxConstraintGroups[i]=(int*)malloc((this->mMinMaxConstraintGroups[i])*sizeof(int));
      if(this->minMaxConstraintGroups[i]==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mMinMaxConstraintGroups[i]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s",NLProblemErrorMsg);fflush(stdout);
        return (NLProblem)NULL;
       }
      for(j=0;j<this->nMinMaxConstraintGroups[i];j++)
       (this->minMaxConstraintGroups[i])[j]=(P->minMaxConstraintGroups[i])[j];
     }
   }else{
    this->nMinMaxConstraintGroups=(int*)NULL;
    this->mMinMaxConstraintGroups=(int*)NULL;
    this->minMaxConstraintGroups=(int**)NULL;
   }
  this->lowerBoundOnMinMaxVar=P->lowerBoundOnMinMaxVar;
  this->upperBoundOnMinMaxVar=P->upperBoundOnMinMaxVar;

  this->hideInequalityConstraints=P->hideInequalityConstraints;
  this->nInequalityConstraints=P->nInequalityConstraints;
  this->mInequalityConstraints=P->mInequalityConstraints;
  if(this->mInequalityConstraints>0)
   {
    this->nInequalityConstraintGroups=(int*)malloc((this->mInequalityConstraints)*sizeof(int));
    if(this->nInequalityConstraintGroups==(int*)NULL && this->mInequalityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->mInequalityConstraintGroups=(int*)malloc((this->mInequalityConstraints)*sizeof(int));
    if(this->mInequalityConstraintGroups==(int*)NULL && this->mInequalityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mMinMaxConstraints*sizeof(int));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->inequalityConstraintGroups=(int**)malloc((this->mInequalityConstraints)*sizeof(int*));
    if(this->inequalityConstraintGroups==(int**)NULL && this->mInequalityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(int*));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->inequalityConstraintLowerBound=(double*)malloc((this->mInequalityConstraints)*sizeof(double));
    if(this->inequalityConstraintLowerBound==(double*)NULL && this->mInequalityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    this->inequalityConstraintUpperBound=(double*)malloc((this->mInequalityConstraints)*sizeof(double)); 
    if(this->inequalityConstraintUpperBound==(double*)NULL && this->mInequalityConstraints>0)
     {
      sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraints*sizeof(double));
      NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
      printf("%s",NLProblemErrorMsg);fflush(stdout);
      return (NLProblem)NULL;
     }
    for(i=0;i<this->nInequalityConstraints;i++)
     {
      (this->nInequalityConstraintGroups[i])=(P->nInequalityConstraintGroups[i]);
      (this->mInequalityConstraintGroups[i])=(P->mInequalityConstraintGroups[i]);
      this->inequalityConstraintGroups[i]=(int*)malloc((this->mInequalityConstraintGroups[i])*sizeof(int));
      if(this->inequalityConstraintGroups[i]==(int*)NULL)
       {
        sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",this->mInequalityConstraintGroups[i]*sizeof(int));
        NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
        printf("%s",NLProblemErrorMsg);fflush(stdout);
        return (NLProblem)NULL;
       }
      for(j=0;j<this->nInequalityConstraintGroups[i];j++)
       (this->inequalityConstraintGroups[i])[j]=(P->inequalityConstraintGroups[i])[j];
      this->inequalityConstraintLowerBound[i]=P->inequalityConstraintLowerBound[i];
      this->inequalityConstraintUpperBound[i]=P->inequalityConstraintUpperBound[i];
     }
   }else{
    this->nInequalityConstraintGroups=(int*)NULL;
    this->mInequalityConstraintGroups=(int*)NULL;
    this->inequalityConstraintGroups=(int**)NULL;
    this->inequalityConstraintLowerBound=(double*)NULL;
    this->inequalityConstraintUpperBound=(double*)NULL;
   }

  return(this);
 }

int NLPAddEqualityConstraint(NLProblem this, char *name, int nvars, int *vars, double (*F)(int,double*,void*),double (*dF)(int,int,double*,void*),double (*ddF)(int,int,int,double*,void*),void *data,void (*freedata)(void*))
 {
  char RoutineName[]="NLPAddEqualityConstraint";
  int constraint;
  int group;
  NLElementFunction ef;
  NLNonlinearElement ne;
  char *tname;
  char tmpname[1024];
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (first arg.) is NULL, you must supply a problem.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(name==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"name (second arg.) is NULL, you must supply a name for the constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  tname=tmpname;
  if(strlen(name)+9>1024)tname=(char*)malloc((strlen(name)+9)*sizeof(char));

  if(tname==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+9)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  constraint=NLPAddNonlinearEqualityConstraint(this, name);
  group=0;

  sprintf(tname,"%sElement",name);
  ef=NLCreateElementFunction(this,tname,nvars,(NLMatrix)NULL,F,dF,ddF,data,freedata);
  sprintf(tname,"%sNE",name);
  ne=NLCreateNonlinearElement(this,tname,ef,vars);
  NLPAddNonlinearElementToEqualityConstraintGroup(this,constraint,group,1.,ne);

  if(tname!=tmpname)free(tname);

  return constraint;
 }

int NLPAddInequalityConstraint(NLProblem this, char *name, double l, double u, int nvars, int *vars, double (*F)(int,double*,void*),double (*dF)(int,int,double*,void*),double (*ddF)(int,int,int,double*,void*),void *data,void (*freedata)(void*))
 {
  char RoutineName[]="NLPAddInequalityConstraint";
  int constraint;
  int group;
  NLElementFunction ef;
  NLNonlinearElement ne;
  char *tname;
  char tmpname[1024];
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (first arg.) is NULL, you must supply a problem.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(name==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"name (second arg.) is NULL, you must supply a name for the constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  tname=tmpname;
  if(strlen(name)+9>1024)tname=(char*)malloc((strlen(name)+9)*sizeof(char));

  if(tname==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+9)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  constraint=NLPAddNonlinearInequalityConstraint(this, name);
  group=0;

  sprintf(tname,"%sElement",name);
  ef=NLCreateElementFunction(this,tname,nvars,(NLMatrix)NULL,F,dF,ddF,data,freedata);
  sprintf(tname,"%sNE",name);
  ne=NLCreateNonlinearElement(this,tname,ef,vars);
  NLPAddNonlinearElementToInequalityConstraintGroup(this,constraint,group,1.,ne);

  NLPSetInequalityConstraintUpperBound(this,constraint,u);
  NLPSetInequalityConstraintLowerBound(this,constraint,l);

  if(tname!=tmpname)free(tname);

  return constraint;
 }

int NLPSetObjective(NLProblem this, char *name, int nvars, int *vars, double (*F)(int,double*,void*),double (*dF)(int,int,double*,void*),double (*ddF)(int,int,int,double*,void*),void *data,void (*freedata)(void*))
 {
  char RoutineName[]="NLPSetObjective";
  int constraint;
  int group;
  NLElementFunction ef;
  NLNonlinearElement ne;
  char *tname;
  char tmpname[1024];
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (first arg.) is NULL, you must supply a problem.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(name==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"name (second arg.) is NULL, you must supply a name for the constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(NLPGetNumberOfGroupsInObjective(this)>0)
   {
    sprintf(NLProblemErrorMsg,"This problem already has an objective.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  tname=tmpname;
  if(strlen(name)+9>1024)tname=(char*)malloc((strlen(name)+9)*sizeof(char));

  if(tname==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+9)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  sprintf(tname,"%sGroup",name);
  group=NLPAddGroupToObjective(this,tname);

  sprintf(tname,"%sElement",name);
  ef=NLCreateElementFunction(this,tname,nvars,(NLMatrix)NULL,F,dF,ddF,data,freedata);
  sprintf(tname,"%sNE",name);
  ne=NLCreateNonlinearElement(this,tname,ef,vars);
  NLPAddNonlinearElementToObjectiveGroup(this,group,1.,ne);

  if(tname!=tmpname)free(tname);

  return 0;
 }

int NLPAddEqualityConstraintByString(NLProblem this, char *name, int nvars, int *vars, char *varlist, char *expr)
 {
  char RoutineName[]="NLPAddEqualityConstraintByString";
  int constraint;
  int group;
  NLElementFunction ef;
  NLNonlinearElement ne;
  char *tname;
  char tmpname[1024];
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (first arg.) is NULL, you must supply a problem.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(name==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"name (second arg.) is NULL, you must supply a name for the constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  tname=tmpname;
  if(strlen(name)+9>1024)tname=(char*)malloc((strlen(name)+9)*sizeof(char));

  if(tname==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+9)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  constraint=NLPAddNonlinearEqualityConstraint(this, name);
  group=0;

  sprintf(tname,"%sElement",name);
  ef=NLCreateElementFunctionByString(this,tname,nvars,(NLMatrix)NULL,varlist,expr);
  sprintf(tname,"%sNE",name);
  ne=NLCreateNonlinearElement(this,tname,ef,vars);
  NLPAddNonlinearElementToEqualityConstraintGroup(this,constraint,group,1.,ne);

  if(tname!=tmpname)free(tname);

  return constraint;
 }

int NLPAddInequalityConstraintByString(NLProblem this, char *name, double l, double u, int nvars, int *vars, char *varlist, char *expr)
 {
  char RoutineName[]="NLPAddInequalityConstraintByString";
  int constraint;
  int group;
  NLElementFunction ef;
  NLNonlinearElement ne;
  char *tname;
  char tmpname[1024];
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (first arg.) is NULL, you must supply a problem.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(name==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"name (second arg.) is NULL, you must supply a name for the constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  tname=tmpname;
  if(strlen(name)+9>1024)tname=(char*)malloc((strlen(name)+9)*sizeof(char));

  if(tname==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+9)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  constraint=NLPAddNonlinearInequalityConstraint(this, name);
  group=0;

  sprintf(tname,"%sElement",name);
  ef=NLCreateElementFunctionByString(this,tname,nvars,(NLMatrix)NULL,varlist,expr);
  sprintf(tname,"%sNE",name);
  ne=NLCreateNonlinearElement(this,tname,ef,vars);
  NLPAddNonlinearElementToInequalityConstraintGroup(this,constraint,group,1.,ne);

  if(u<1.e19)NLPSetInequalityConstraintUpperBound(this,constraint,u);
  if(l>-1.e19)NLPSetInequalityConstraintLowerBound(this,constraint,l);

  if(tname!=tmpname)free(tname);

  return constraint;
 }

int NLPSetObjectiveByString(NLProblem this, char *name, int nvars, int *vars, char *varlist, char *expr)
 {
  char RoutineName[]="NLPSetObjectiveByString";
  int constraint;
  int group;
  NLElementFunction ef;
  NLNonlinearElement ne;
  char *tname;
  char tmpname[1024];
  int i;

  if(this==(NLProblem)NULL)
   {
    sprintf(NLProblemErrorMsg,"Problem (first arg.) is NULL, you must supply a problem.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(name==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"name (second arg.) is NULL, you must supply a name for the constraint.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  if(NLPGetNumberOfGroupsInObjective(this)>0)
   {
    sprintf(NLProblemErrorMsg,"This problem already has an objective.");
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  tname=tmpname;
  if(strlen(name)+9>1024)tname=(char*)malloc((strlen(name)+9)*sizeof(char));

  if(tname==(char*)NULL)
   {
    sprintf(NLProblemErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(name)+9)*sizeof(char));
    NLSetError(12,RoutineName,NLProblemErrorMsg,__LINE__,__FILE__);
    printf("%s",NLProblemErrorMsg);fflush(stdout);
    return -12;
   }

  sprintf(tname,"%sGroup",name);
  group=NLPAddGroupToObjective(this,tname);

  sprintf(tname,"%sElement",name);
  ef=NLCreateElementFunctionByString(this,tname,nvars,(NLMatrix)NULL,varlist,expr);
  sprintf(tname,"%sNE",name);
  ne=NLCreateNonlinearElement(this,tname,ef,vars);
  NLPAddNonlinearElementToObjectiveGroup(this,group,1.,ne);

  if(tname!=tmpname)free(tname);

  return 0;
 }
