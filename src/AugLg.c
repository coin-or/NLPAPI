/*       Author: Mike Henderson, Andy Conn */
/*       version: %W% %D% %T% */
/*       Date: January 7, 2003, modified EqSBProblem.c */

/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES      */
/*   CORPORATION January 7, 2003.  ALL RIGHTS RESERVED.   */

/*  Please refer to the LICENSE file in the top directory*/

#include <NLPAPI.h>

static double NLgSq(double x,void *d){return(x*x);}
static double NLdgSq(double x,void *d){return(2*x);}
static double NLddgSq(double x,void *d){return(2);}

void NLSetError(int,char*,char*,int,char*);
static char NLErrorMsg[256]="";

int NLCreateAugmentedLagrangian(NLProblem P, double mu, double *lambda, int *objGroup, double *constraintB, double *constraintS)
 {
  char RoutineName[]="NLCreateAugmentedLagrangian";

/* Local variables */

  int j;
  int i,n;
  int nc;
  int nObjGroupsAdded;
  int group;
  int cgroup;
  NLGroupFunction gSquare=(NLGroupFunction)NULL;
  char type[128];
  int verbose=0;
  int trace=0;
  int rc;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(4,RoutineName,NLErrorMsg,__LINE__,__FILE__);
    return;
   }

  n=NLPGetNumberOfVariables(P);
  nc=NLPGetNumberOfEqualityConstraints(P);

  if(verbose)
   {
    printf("  This is problem \"%s\"\n",NLPGetProblemName(P));
    fflush(stdout);
   }

/* Introduce quadratic penalty function and Lagrange multipliers */

  if(trace){printf("Introduce quadratic penalty function and Lagrange multipliers\n");fflush(stdout);}

  gSquare=NLCreateGroupFunction(P,"NLSq",NLgSq,NLdgSq,NLddgSq,(void*)NULL,(void (*)(void*))NULL);
  NLGFAssertPolynomialOrder(gSquare,2);

  for(i=0;i<nc;i++)
   {

    if(NLPGetNumberOfEqualityConstraintGroups(P,i)>1)
     {
/*   add an objective group which is trivial and one NE which
                           is the sum of the groups from the constraint. The
                           element variables are the non-zero's from the a's,
                           and the element variables from the NE's. */
      sprintf(NLErrorMsg,"Equality Constraint %d has %d groups. This is not supported yet.\n",i,NLPGetNumberOfEqualityConstraintGroups(P,i));
      NLSetError(4,RoutineName,NLErrorMsg,__LINE__,__FILE__);
      printf("%s",NLErrorMsg);fflush(stdout);
      rc=12;
      goto FreeAndReturn;
     }
    group=NLPGetEqualityConstraintNumberOfGroup(P,i,0);
    if(NLPGetGroupFunction(P,group)!=NULL)
     {
/*   if constraint is a single, nontrivial group, add an objective group which
                                               is g(x)^2 with the NE's from the 
                                               constraint */
      sprintf(NLErrorMsg,"Equality Constraint %d has a nontrivial group function. This is not supported yet.\n",i);fflush(stderr);
      NLSetError(4,RoutineName,NLErrorMsg,__LINE__,__FILE__);
      printf("%s",NLErrorMsg);fflush(stdout);
      rc=12;
      goto FreeAndReturn;
     }

/*   if constraint is a single, trivial group, add an objective group which
                                               is x^2 with the NE's from the 
                                               constraint */

    cgroup=NLPGetEqualityConstraintNumberOfGroup(P,i,0);

    sprintf(type,"(C[%d]+mu*L[%d])**2/(2mu)",i,i);
    objGroup[i]=NLPAddGroupToObjective(P,type);
    group=NLPGetObjectiveGroupNumber(P,objGroup[i]);
    NLPSetGroupFunction(P,group,gSquare);
    if(NLPGetGroupA(P,cgroup)!=(NLVector)NULL)NLPSetGroupA(P,group,NLPGetGroupA(P,cgroup));
    constraintB[i]=0.;
    if(NLPIsGroupBSet(P,cgroup))constraintB[i]=NLPGetGroupB(P,cgroup);
    constraintS[i]=NLPGetGroupScale(P,cgroup);

    NLPSetGroupB(P,group,constraintB[i]-mu*lambda[i]*constraintS[i]);

    for(j=0;j<NLPGetNumberOfElementsInGroup(P,cgroup);j++)
     {
      NLPAddNonlinearElementToObjectiveGroup(P,objGroup[i],
         NLPGetElementWeight(P,cgroup,j),
         NLPGetGroupNonlinearElement(P,cgroup,j));
     }
    NLPSetGroupScale(P,group,2*mu*constraintS[i]*constraintS[i]);
   }

FreeAndReturn:
  return rc;
 }

int NLSetLambaAndMuInAugmentedLagrangian(NLProblem P, int nc, double mu, double *lambda, int *objGroup, double *constraintB, double *constraintS)
 {
  char RoutineName[]="NLSetLambaAndMuInAugmentedLagrangian";
  int i;
  int group;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(4,RoutineName,NLErrorMsg,__LINE__,__FILE__);
    return;
   }

  for(i=0;i<nc;i++)
   {
    group=NLPGetObjectiveGroupNumber(P,objGroup[i]);
    NLPSetGroupB(P,group,constraintB[i]-mu*lambda[i]*constraintS[i]);
    NLPSetGroupScale(P,group,2*mu*constraintS[i]*constraintS[i]);
   }

  return 0;
 }

void NLEliminateFixedVariables(NLProblem P)
 {
  char RoutineName[]="NLEliminateFixedVariables";
  int i,nf,n;
  char type[128];
  double ui,li;
  NLVector a;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLErrorMsg,"Problem (argument 2) is NULL");
    NLSetError(4,RoutineName,NLErrorMsg,__LINE__,__FILE__);
    return;
   }
   
  n=NLPGetNumberOfVariables(P);
  nf=0;
  for(i=0;i<n;i++)
   {
    if(NLPIsUpperSimpleBoundSet(P,i)&&NLPIsLowerSimpleBoundSet(P,i))
     {
      ui=NLPGetUpperSimpleBound(P,i);
      li=NLPGetLowerSimpleBound(P,i);
      if(ui==li)nf++;
     }
   }

  if(nf>0)
   {
    nf=0;
    for(i=0;i<n;i++)
     {
      if(NLPIsUpperSimpleBoundSet(P,i)&&NLPIsLowerSimpleBoundSet(P,i))
       {
        ui=NLPGetUpperSimpleBound(P,i);
        li=NLPGetLowerSimpleBound(P,i);
        if(ui==li)
         {
          NLPUnSetUpperSimpleBound(P,i);
          NLPUnSetLowerSimpleBound(P,i);
          sprintf(type,"FX:%s",NLPGetVariableName(P,i));
          a=NLCreateVector(n);
          NLVSetC(a,i,1.);
          NLPAddLinearEqualityConstraintV(P,type,a,ui);
          NLFreeVector(a);
         }
       }
     }
   }

 }
