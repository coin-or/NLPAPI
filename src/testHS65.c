#include <NLPAPI.h>
#include <CUTE.h>
#include <stdio.h>

int main(int argc,char *argv[])
 {
  NLProblem P;
  double *x=(double*)NULL;
  double *x0=(double*)NULL;
  double *l0=(double*)NULL;
  NLLancelot Lan;
  double ui,li;
  int i,n;
  int rc;
  int option;
  int lancelot;
  int printproblem;
  int nc;

  lancelot=1;
  printproblem=0;

  while((option=getopt(argc,argv,"pL"))!=EOF)
   {
    switch(option)
     {
      case 'p':
       printproblem=1;
       break;
      case 'L':
       lancelot=1;
       break;
      default:
       printf("\nusage: \n\n%s -gpL\n",argv[0]);
       printf("\noptions:\n\n");
       printf("-g \n        Animate the solution.\n");
       printf("-p \n        Print the problem\n");
       return 0;
       break;
     }
   }

  P=CUTECreateHS65(&x0,&l0);
  if(printproblem){NLPrintProblemShort(stdout,P);printf("\n");fflush(stdout);}

  n=NLPGetNumberOfVariables(P);
  x=(double*)malloc(n*sizeof(double));

  if(!lancelot)
   {
    for(i=0;i<n;i++)
     {
      if(NLPIsLowerSimpleBoundSet(P,i))
       {
        li=NLPGetLowerSimpleBound(P,i);
        if(NLPIsUpperSimpleBoundSet(P,i))
         {
          ui=NLPGetUpperSimpleBound(P,i);
          if(ui==li)x0[i]=li;
          else if(x0[i]<=li)x0[i]=li+.01*(ui-li);
          else if(x0[i]>=ui)x0[i]=ui-.01*(ui-li);
         }else{
          if(x0[i]<=li)x0[i]=li+1.;
         }
       }else{
        if(NLPIsUpperSimpleBoundSet(P,i))
         {
          ui=NLPGetUpperSimpleBound(P,i);
          if(x0[i]>=ui)x0[i]=ui-1.;
         }
       }
     }
   }

  if(n<10)
   {
    printf("Initial Guess is (");
    for(i=0;i<n;i++)
     {
      if(i>0)printf(",");
      printf("%lf",x0[i]);
     }
    printf(")\n");fflush(stdout);
   }else{
    printf("Initial Guess is\n");
    for(i=0;i<n;i++)
     {
      if(n<10||i<10||i>n-12)printf("     %d %lf\n",i,x0[i]);
     }
    fflush(stdout);
   }

  if(lancelot)
   {
    Lan=NLCreateLancelot();
    LNSetPrintLevel(Lan,1);
    LNSetUseExactFirstDerivatives(Lan,1);
    LNSetUseExactSecondDerivatives(Lan,"Exact");
    LNSetSolveBQPAccurately(Lan,1);
    LNSetTrustRegionType(Lan,"two norm");
    LNSetCheckDerivatives(Lan,0);
    LNSetMaximumNumberOfIterations(Lan,100000);
    LNSetPrintStop(Lan,100000);
    rc=LNMinimize(Lan,P,x0,(double*)NULL,(double*)NULL,x);
   }

  if(rc==0)
   {
    printf("Solution is (");
    for(i=0;i<NLPGetNumberOfVariables(P);i++)
     {
      if(i>0)printf(",");
      if(i%10==9)printf("\n");
      printf("%lf",x[i]);
     }
    printf(")\n");
   }else{
    printf("The Minimization failed\n");
   }

  if(NLError())
   {
    printf("There were %d errors\n",NLGetNErrors());
    for(i=0;i<NLGetNErrors();i++)
     {
      printf(" %d line %d, file %s, Sev: %d\n",i,NLGetErrorLine(i),NLGetErrorFile(i),NLGetErrorSev(i));fflush(stdout);
      printf("    Routine: \"%s\"\n",NLGetErrorRoutine(i));fflush(stdout);
      printf("    Msg: \"%s\"\n",NLGetErrorMsg(i));fflush(stdout);
     }
   }

/* Clean up                                                   */

  NLClearErrors();
  if(lancelot)
    NLFreeLancelot(Lan);
  NLFreeProblem(P);
  free(x);
  free(x0);
  free(l0);
  exit(rc);
 }
