#include <NLPAPI.h>
#include <stdio.h>
#include <float.h>

double fSq(int n,double *x,void *d){return(x[0]*x[0]);}
double dfSq(int i,int n,double *x,void *d){return(2*x[0]);}
double ddfSq(int i,int j,int n,double *x,void *d){return(2);}

double fCb(int n,double *x,void *d){return(x[0]*x[0]*x[0]);}
double dfCb(int i,int n,double *x,void *d){return(3*x[0]*x[0]);}
double ddfCb(int i,int j,int n,double *x,void *d){return(6*x[0]);}

int main(int argc,char *argv[])
 {
  NLProblem P;
  NLElementFunction Sq1;
  NLElementFunction Sq2;
  NLElementFunction Cb;
  NLNonlinearElement ne;
  NLMatrix xfrm;
  int objective;
  NLVector a;
  double x0[4];
  NLLancelot Lan;
  double x[3];
  int constraint;
  int element;
  int v[3];
  int i;
  int rc;

/* HS32 */

  P=NLCreateProblem("HS32",3);

/* Objective                                                  */

  xfrm=NLCreateMatrix(1,3);
  NLMSetElement(xfrm,0,0,1.);
  NLMSetElement(xfrm,0,1,3.);
  NLMSetElement(xfrm,0,2,1.);
  Sq1=NLCreateElementFunction(P,"efSq1",1,xfrm,fSq,dfSq,ddfSq,(void*)NULL,(void (*)(void*))NULL);
  NLFreeMatrix(xfrm);

  xfrm=NLCreateMatrix(1,2);
  NLMSetElement(xfrm,0,0,1.);
  NLMSetElement(xfrm,0,1,-1.);
  Sq2=NLCreateElementFunction(P,"efSq2",1,xfrm,fSq,dfSq,ddfSq,(void*)NULL,(void (*)(void*))NULL);
  NLFreeMatrix(xfrm);

  Cb=NLCreateElementFunction(P,"efCb",1,(NLMatrix)NULL,fCb,dfCb,ddfCb,(void*)NULL,(void (*)(void*))NULL);

  objective=NLPAddGroupToObjective(P,"OBJ");

  v[0]=0;
  v[1]=1;
  v[2]=2;
  ne=NLCreateNonlinearElement(P,"Sq1",Sq1,v);
  element=NLPAddNonlinearElementToObjectiveGroup(P,objective,1.,ne);
  NLFreeNonlinearElement(P,ne);

  v[0]=0;
  v[1]=1;
  ne=NLCreateNonlinearElement(P,"Sq2",Sq2,v);
  element=NLPAddNonlinearElementToObjectiveGroup(P,objective,4.,ne);
  NLFreeNonlinearElement(P,ne);

/* Nonlinear Inequality Constraints  */

  constraint=NLPAddNonlinearInequalityConstraint(P,"C1");
  a=NLCreateVector(3);
  rc=NLVSetC(a,0, 0.);
  rc=NLVSetC(a,1, 6.);
  rc=NLVSetC(a,2, 4.);
  rc=NLPSetInequalityConstraintA(P,constraint,a);
  NLFreeVector(a);
  rc=NLPSetInequalityConstraintB(P,constraint,3.);
  v[0]=0;
  ne=NLCreateNonlinearElement(P,"CB",Cb,v);
  element=NLPAddNonlinearElementToInequalityConstraint(P,constraint,-1.,ne);
  NLFreeNonlinearElement(P,ne);
  NLPSetInequalityConstraintLowerBound(P,constraint,0.);

  constraint=NLPAddNonlinearEqualityConstraint(P,"C2");
  a=NLCreateVector(3);
  rc=NLVSetC(a,0,-1.);
  rc=NLVSetC(a,1,-1.);
  rc=NLVSetC(a,2,-1.);
  rc=NLPSetEqualityConstraintA(P,constraint,a);
  NLFreeVector(a);
  rc=NLPSetEqualityConstraintB(P,constraint,-1.);

  NLPrintProblem(stdout,P);

  x0[0]=.1;
  x0[1]=.7;
  x0[2]=.2;
  x0[3]=-2.001;

  Lan=NLCreateLancelot();
  rc=LNSetPrintLevel(Lan,1);

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
  NLFreeElementFunction(Sq1);
  NLFreeElementFunction(Sq2);
  NLFreeElementFunction(Cb);
  NLFreeLancelot(Lan);
  NLFreeProblem(P);
  return(0);
 }
