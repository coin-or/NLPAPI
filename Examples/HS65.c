#include <NLPAPI.h>
#include <stdio.h>

double gSq(double x,void *d){return(x*x);}
double dgSq(double x,void *d){return(2*x);}
double ddgSq(double x,void *d){return(2);}

double fSq(int n,double *x,void *d){return(x[0]*x[0]);}
double dfSq(int i,int n,double *x,void *d){return(2*x[0]);}
double ddfSq(int i,int j,int n,double *x,void *d){return(2);}

int main(int argc,char *argv[])
 {
  NLProblem P;
  NLGroupFunction g;
  NLElementFunction f;
  NLNonlinearElement ne;
  int group;
  NLVector a;
  double x0[3];
  NLLancelot Lan;
  double x[3];
  int constraint;
  int element;
  int v[1];
  int i;
  int rc;

/* HS65 */

  P=NLCreateProblem("HS65",3);
  NLPSetVariableName(P,0,"X1");
  NLPSetVariableName(P,1,"X2");
  NLPSetVariableName(P,2,"X3");

/* Objective                                                  */

/*           2             2          2                       */
/*    (x -x )  + (x +x -10) /9 +(x -5)                        */
/*      1  2       1  2           3                           */

  g=NLCreateGroupFunction(P,"L2",gSq,dgSq,ddgSq,(void*)NULL,(void (*)(void*))NULL);
  f=NLCreateElementFunction(P,"SQ",1,(NLMatrix)NULL,fSq,dfSq,ddfSq,(void*)NULL,(void (*)(void*))NULL);

  group=NLPAddGroupToObjective(P,"OBJ1");
  rc=NLPSetObjectiveGroupFunction(P,group,g);
  a=NLCreateVector(3);
  rc=NLVSetC(a,0,1.);
  rc=NLVSetC(a,1,-1.);
  rc=NLVSetC(a,2,0.);
  rc=NLPSetObjectiveGroupA(P,group,a);
  NLFreeVector(a);

  group=NLPAddGroupToObjective(P,"OBJ2");
  rc=NLPSetObjectiveGroupFunction(P,group,g);
  a=NLCreateVector(3);
  rc=NLVSetC(a,0,1.);
  rc=NLVSetC(a,1,1.);
  rc=NLVSetC(a,2,0.);
  rc=NLPSetObjectiveGroupScale(P,group,9.);
  rc=NLPSetObjectiveGroupA(P,group,a);
  rc=NLPSetObjectiveGroupB(P,group,10.);
  NLFreeVector(a);

  group=NLPAddGroupToObjective(P,"OBJ3");
  rc=NLPSetObjectiveGroupFunction(P,group,g);
  a=NLCreateVector(3);
  rc=NLVSetC(a,0,0.);
  rc=NLVSetC(a,1,0.);
  rc=NLVSetC(a,2,1.);
  rc=NLPSetObjectiveGroupA(P,group,a);
  rc=NLPSetObjectiveGroupB(P,group,5.);
  NLFreeVector(a);

/* Simple Bounds                                              */

/*  -4.5 <= x <= 4.5                                          */
/*           1                                                */

/*  -4.5 <= x <= 4.5                                          */
/*           2                                                */

/*  -5. <= x <= 5.                                            */
/*           3                                                */

  rc=NLPSetSimpleBounds(P,0,-4.5,4.5);
  rc=NLPSetSimpleBounds(P,1,-4.5,4.5);
  rc=NLPSetSimpleBounds(P,2,-5.,5.);

/* Nonlinear Inequality Constraint                                                  */

/*          2  2  2                                           */
/* 0 <= 48-x -x -x                                            */
/*          1  2  3                                           */

  constraint=NLPAddNonlinearInequalityConstraint(P,"C1");
  group=0;

  rc=NLPSetInequalityConstraintGroupB(P,constraint,group,-48.);
  v[0]=0;
  ne=NLCreateNonlinearElement(P,"Sq1",f,v);
  element=NLPAddNonlinearElementToInequalityConstraintGroup(P,constraint,group,-1.,ne);
  NLFreeNonlinearElement(P,ne);

  v[0]=1;
  ne=NLCreateNonlinearElement(P,"Sq2",f,v);
  element=NLPAddNonlinearElementToInequalityConstraintGroup(P,constraint,group,-1.,ne);
  NLFreeNonlinearElement(P,ne);

  v[0]=2;
  ne=NLCreateNonlinearElement(P,"Sq3",f,v);
  element=NLPAddNonlinearElementToInequalityConstraintGroup(P,constraint,group,-1.,ne);
  NLFreeNonlinearElement(P,ne);

  NLPrintProblem(stdout,P);

  x0[0]=-5.;
  x0[1]=5.;
  x0[2]=0.;

/*x0[0]=3.65046;
  x0[1]=3.65046;
  x0[2]=4.62042;*/

  Lan=NLCreateLancelot();
  rc=LNSetPrintLevel(Lan,1);
/*rc=LNSetInitialPenalty(Lan,1.e-4);
  rc=LNSetPenaltyBound(Lan,1.e-4);*/

  rc=LNMinimize(Lan,P,x0,(double*)NULL,(double*)NULL,x);

  printf("Solution is (");
  for(i=0;i<3;i++)
   {
    if(i>0)printf(",");
    printf("%lf",x[i]);
   }
  printf(")\n");
  printf("There were %d errors\n",NLGetNErrors());
  if(NLError())
   {
    for(i=0;i<NLGetNErrors();i++)
     {
      printf(" %d line %d, file %s, Sev: %d\n",i,NLGetErrorLine(i),NLGetErrorFile(i),NLGetErrorSev(i));fflush(stdout);
      printf("    Routine: \"%s\"\n",NLGetErrorRoutine(i));fflush(stdout);
      printf("    Msg: \"%s\"\n",NLGetErrorMsg(i));fflush(stdout);
     }
   }

/* Clean up                                                   */

  NLClearErrors();
  NLFreeGroupFunction(g);
  NLFreeElementFunction(f);
  NLFreeLancelot(Lan);
  NLFreeProblem(P);
  return(0);
 }
