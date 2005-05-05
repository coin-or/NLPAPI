/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: %W% %D% %T% */
/*  (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory*/

/*      date:   November 11, 1997                     */
/*              February 2, 1999   Ported to C        */
/*              January 10, 2001   Fixed alloc. problem with Tall Thin */
/*              September 26, 2001 Added support for declaring the degree of a variable */
/*              October 3, 2001    Added rank one updates and differencing */

/*  Please refer to the LICENSE file in the top directory*/

#include <NLPAPI.h>
#include <ExpCmp.h>
#include <time.h>

void NLSetError(int,char*,char*,int,char*);
static char NLEFErrorMsg[256]="";

#define BFGS 0
#define SR1 1
#define EXACT 2

static int NLDefaultElementUpdateType=BFGS;
static double NLDefaultElementUpdateNoise=1.e-3;

struct NLGrpPartEFn
 {
  int nV;
  int *variablePower;
/* Scale for E, Scale for gradE_i, Scale for hessE_ij */
  NLMatrix R;
  int type;
  double (*F)(int,double*,void*);
  double (*dF)(int,int,double*,void*);
  double (*ddF)(int,int,int,double*,void*);

  char *expr;
  char *vars;
  ECFn sF;
  ECFn *sdF;
  ECFn *sddF;

  double *ddF0;
  double *dF0;
  double *x0;
  double *y;
  double *Hs;
  NLProblem P;
  void *data;
  void (*freeData)(void*);
  int DiffType;
  double NoiseLevel;

  int nRefs;
 };

/* These are for replacing tall and thin range transformations with the identity */

static double *NLfcompy=(double*)NULL;
static int lNLfcompy=0;

struct NLfcompdatast {
                    int m;
                    double (*f)(int,double*,void*);
                    double (*df)(int,int,double*,void*);
                    double (*ddf)(int,int,int,double*,void*);
                    NLMatrix xfrm;
                    void *data;
                    void (*freedata)(void*);
                   };

extern NLProblem LANSOLProblem;

double NLEvaluateElementFunctionTime=0.;
int NLEvaluateElementFunctionNCalls=0;
double NLEvaluateElementFunctionDerTime=0.;
int NLEvaluateElementFunctionDerNCalls=0;
double NLEvaluateElementFunctionSecDerTime=0.;
int NLEvaluateElementFunctionSecDerNCalls=0;

double NLfcomp(int n,double *x,void *d)
 {
  struct NLfcompdatast *data;
  double result;

  data=(struct NLfcompdatast*)d;


  NLMVMult(data->xfrm,x,NLfcompy);
  result=(data->f)(data->m,NLfcompy,data->data);
  return result;
 }

double NLdfcomp(int i,int n,double *x,void *d)
 {
  struct NLfcompdatast *data;
  int ii;
  double sum;

  data=(struct NLfcompdatast*)d;

  NLMVMult(data->xfrm,x,NLfcompy);
  sum=0.;
  for(ii=0;ii<data->m;ii++)
    sum+=(data->df)(ii,data->m,NLfcompy,data->data)*
         NLMGetElement(data->xfrm,ii,i);

  return sum;
 }

double NLddfcomp(int i,int j,int n,double *x,void *d)
 {
  struct NLfcompdatast *data;
  int ii,jj;
  double sum;

  data=(struct NLfcompdatast*)d;

  NLMVMult(data->xfrm,x,NLfcompy);
  sum=0.;
  for(ii=0;ii<data->m;ii++)
   for(jj=0;jj<data->m;jj++)
    sum+=(data->ddf)(ii,jj,data->m,NLfcompy,data->data)*NLMGetElement(data->xfrm,ii,i)*NLMGetElement(data->xfrm,jj,j);

  return sum;
 }

void *NLfcompdata( double (*f)(int,double*,void*),
                 double (*df)(int,int,double*,void*),
                 double (*ddf)(int,int,int,double*,void*),
                 NLMatrix xfrm,
                 void *data,
                 void (*freedata)(void*))
 {
  struct NLfcompdatast *result;
  result=(struct NLfcompdatast*)malloc(sizeof(struct NLfcompdatast));
  result->m=NLMGetNumberOfRows(xfrm);
  result->f=f;
  result->df=df;
  result->ddf=ddf;
  result->xfrm=xfrm;
  NLRefMatrix(xfrm);
  result->data=data;
  result->freedata=freedata;
  if(result->m>lNLfcompy)
   {
    lNLfcompy=result->m;
    NLfcompy=(double*)realloc((void*)NLfcompy,result->m*sizeof(double));
   }
  return result;
 }

void NLfreeNLfcompdata(void *d)
 {
  struct NLfcompdatast *data;

  data=(struct NLfcompdatast*)d;
  NLRefMatrix(data->xfrm);
  data->freedata(data->data);
  free(data);
  return;
 }

int NLEGetDimension(NLElementFunction this)
 {
  char RoutineName[]="NLEGetDimension";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return -1;
   }
#endif

  return(this->nV);
 }

void NLRefElementFunction(NLElementFunction this)
 {
  char RoutineName[]="NLRefElementFunction";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  this->nRefs++;
  return;
 }

void NLFreeElementFunction(NLElementFunction this)
 {
  char RoutineName[]="NLFreeElementFunction";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  this->nRefs--;

  if(this->nRefs<1)
   {
    if(this->freeData!=(void (*)(void*))NULL)this->freeData(this->data);
    if(this->ddF0!=(double*)NULL)free(this->ddF0);
    if(this->dF0!=(double*)NULL)free(this->dF0);
    if(this->x0!=(double*)NULL)free(this->x0);
    if(this->y!=(double*)NULL)free(this->y);
    if(this->Hs!=(double*)NULL)free(this->Hs);
    if(this->variablePower!=(int*)NULL)free(this->variablePower);
    free(this);
   }
  return;
 }

int NLMatrixGetNumberOfRefs(NLMatrix);

NLElementFunction NLCreateElementFunction(NLProblem P,char *type,int n,NLMatrix
R,double (*F)(int,double*,void*),double (*dF)(int,int,double*,void*),double (*ddF)(int,int,int,double*,void*),void *data,void (*freedata)(void*))
 {
  char RoutineName[]="NLCreateElementFunction";
  NLElementFunction result;
  int set;
  int i;
  int verbose;

  verbose=0;
  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(R!=(NLMatrix)NULL && NLMGetNumberOfCols(R)<NLMGetNumberOfRows(R))
   {
    result=NLCreateElementFunction(P,type,NLMGetNumberOfCols(R),(NLMatrix)NULL,NLfcomp,NLdfcomp,NLddfcomp,NLfcompdata(F,dF,ddF,R,data,freedata),NLfreeNLfcompdata);
    return result;
   }

  result=(NLElementFunction)malloc(sizeof(struct NLGrpPartEFn));
  if(result==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartEFn));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  result->nV=n;
#ifndef NL_NOINPUTCHECKS
  if(!(n>0))
   {
    sprintf(NLEFErrorMsg,"Number of coordinates is not positive %d",n);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    free(result);
    return (NLElementFunction)NULL;
   }
#endif
  result->variablePower=(int*)malloc(n*sizeof(int));
  if(result->variablePower==(int*)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(int));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  for(i=0;i<n;i++)result->variablePower[i]=NLVARIABLEDEPENDENCENOTSET;
  set=(R!=(NLMatrix)NULL);
  result->type=NLPAddElementType(P,type,set);
  result->R=R;
  if(set)NLRefMatrix(R);
  result->F=F;
  result->ddF0=(double*)NULL;
  result->dF0=(double*)NULL;
  result->x0=(double*)NULL;
  result->y=(double*)NULL;
  result->Hs=(double*)NULL;
  result->dF=dF;
  result->ddF=ddF;
  result->data=data;
  result->freeData=freedata;
  result->nRefs=1;
  result->P=P;
  if(ddF==NULL)
    result->DiffType=NLDefaultElementUpdateType;
   else
    result->DiffType=EXACT;
  result->NoiseLevel=NLDefaultElementUpdateNoise;

  result->expr=(char*)NULL;
  result->vars=(char*)NULL;
  result->sF=(ECFn)NULL;
  result->sdF=(ECFn*)NULL;
  result->sddF=(ECFn*)NULL;

  return(result);
 }

double NLEEval(NLElementFunction F,int n,double *x,void *data)
 {
  char RoutineName[]="NLEEval";
  double result;
  static int verbose=0;
  clock_t tin;

  tin=clock();

  if(verbose){printf("%s, type %s\n",RoutineName,NLPGetElementType(F->P,F->type));
              printf("    f=0x%8.8x, df=0x%8.8x, ddf=0x%8.8x, data=0x%8.8x\n",F->F,F->dF,F->ddF,data);fflush(stdout);}

#ifndef NL_NOINPUTCHECKS
  if(F==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

#ifndef NL_NOINPUTCHECKS
  if(n!=F->nV)
   {
    sprintf(NLEFErrorMsg,"Number of arguments to Element Function (%d) is illegal (argument 2). Must be %d. Argument 1 is %8.8x",n,F->nV,F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

#ifndef NL_NOINPUTCHECKS
  if(x==(double*)NULL)
   {
    sprintf(NLEFErrorMsg,"Pointer to x (argument 3) is NULL. F is %8.8x",F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

  if(F->F!=NULL)
   {
    if(verbose){int i;
                printf(" calling users function @(%le",x[0]);
                for(i=0;i<n;i++)printf(",%le",x[i]);
                printf(")\n");
                fflush(stdout);}
    result=F->F(n,x,data);
   }else{
    if(verbose){printf("evaluating: %s\n",F->expr);fflush(stdout);}
    ECEvaluateFunction(F->sF,x,&result);
   }
  if(verbose){printf("  result=%le\n",result);fflush(stdout);}

  NLEvaluateElementFunctionTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateElementFunctionNCalls++;
  return result;
 }

double NLEEvalDer(NLElementFunction F,int i,int n,double *x, void *data)
 {
  static char RoutineName[]="NLEEvalDer";
  static double fplus,fminus,t;
  static int verbose=0;
  static int ii;
  static double result;
  clock_t tin;

  tin=clock();

  if(verbose)
   {
    printf("%s, type %s entry %d at x=(%lf",RoutineName,NLPGetElementType(F->P,F->type),i,x[0]);
    for(ii=1;ii<n;ii++)printf(",%lf",x[ii]);
    printf(")\n");fflush(stdout);
    printf("    f=0x%8.8x, df=0x%8.8x, ddf=0x%8.8x\n",F->F,F->dF,F->ddF);fflush(stdout);
   }

#ifndef NL_NOINPUTCHECKS
  if(F==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(n!=F->nV)
   {
    sprintf(NLEFErrorMsg,"Number of arguments to Element Function %d is illegal (argument 3). Must be %d. Argument 1 is %8.8x",n,F->nV,F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(x==(double*)NULL)
   {
    sprintf(NLEFErrorMsg,"Pointer to x (argument 4) is NULL. F is %8.8x",F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(i<0||!(i<F->nV))
   {
    sprintf(NLEFErrorMsg,"Direction %d (argument 2) is illegal. Must be in range 0 to %d Argument 1 is %8.8x",i,F->nV,F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

  if(F->F!=NULL)
   {
    if(F->dF==(double (*)(int,int,double*,void*))NULL) /* i.e. no gradient given by user */
     {
      if(verbose){printf("  differencing\n");fflush(stdout);}
      t=x[i];
      x[i]=t+.5e-5;
      fplus=F->F(n,x,data);
      x[i]=t-.5e-5;
      fminus=F->F(n,x,data);
        x[i]=t;
      return (fplus-fminus)*1.e5;
     }
  
    if(verbose){printf("  calling user's function\n");fflush(stdout);}
    result=F->dF(i,n,x,data);
   }else{
    if(verbose){printf("evaluating derivative of: %s\n",F->expr);fflush(stdout);}
    ECEvaluateFunction(F->sdF[i],x,&result);
   }
  if(verbose){printf("  result=%le\n",result);fflush(stdout);}
  NLEvaluateElementFunctionDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateElementFunctionDerNCalls++;
  return result;
 }

int NLNumberOfUpdatesSkipped=0;

double NLEEvalSecDer(NLElementFunction F,int i,int j,int n,double *x, void *data)
 {
  static char RoutineName[]="NLEEvalSecDer";
  static int ii,jj,m;
  static double sHs,ys,t;
  static int verbose=0;
  clock_t tin;
  double result;
  double skipBound=1.e-8;
  double yy,ss;
  double invsHs;
  double invys;

  tin=clock();

  if(verbose)
   {
    printf("%s, type %s entry (%d,%d) at x=(%lf",RoutineName,NLPGetElementType(F->P,F->type),i,j,x[0]);
    for(ii=1;ii<n;ii++)printf(",%lf",x[ii]);
    printf(")\n");fflush(stdout);
    printf("    f=0x%8.8x, df=0x%8.8x, ddf=0x%8.8x\n",F->F,F->dF,F->ddF);fflush(stdout);
    printf("DiffType=%d (EXACT=%d, BFGS=%d, SR1=%d)\n",F->DiffType,EXACT,BFGS,SR1);fflush(stdout);

   }

#ifndef NL_NOINPUTCHECKS
  if(F==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(i<0||!(i<F->nV))
   {
    sprintf(NLEFErrorMsg,"Direction %d (argument 2) is illegal. Must be in range 0 to %d Argument 1 is %8.8x",i,F->nV,F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(j<0||!(j<F->nV))
   {
    sprintf(NLEFErrorMsg,"Direction %d (argument 3) is illegal. Must be in range 0 to %d Argument 1 is %8.8x",j,F->nV,F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(n!=F->nV)
   {
    sprintf(NLEFErrorMsg,"Number of arguments to Element Function %d is illegal (argument 4). Must be %d. Argument 1 is %8.8x",n,F->nV,F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(x==(double*)NULL)
   {
    sprintf(NLEFErrorMsg,"Pointer to x (argument 5) is NULL. F is %8.8x",F);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

  if(F->F!=NULL)
   {
    if(F->ddF==NULL||F->DiffType!=EXACT) /* i.e. no Hessian given by user */
     {
      if(verbose){printf("   No Hessian Function given by user\n");fflush(stdout);}
      if(F->dF0==(double*)NULL) /* First time */
       {
        if(verbose){printf("   First call\n");fflush(stdout);}
        m=F->nV;
  
        F->x0=(double*)malloc(m*sizeof(double));
        if(F->x0==(double*)NULL)
         {
          sprintf(NLEFErrorMsg,"Out of memory trying to allocate %d bytes",m*sizeof(double));
          NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
          return DBL_QNAN;
         }
        for(ii=0;ii<m;ii++)(F->x0)[ii]=x[ii];
  
        F->y=(double*)malloc(m*sizeof(double));
        if(F->y==(double*)NULL)
         {
          sprintf(NLEFErrorMsg,"Out of memory trying to allocate %d bytes",m*sizeof(double));
          NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
          return DBL_QNAN;
         }
        for(ii=0;ii<m;ii++)(F->y)[ii]=0.;
  
        F->Hs=(double*)malloc(m*sizeof(double));
        if(F->Hs==(double*)NULL)
         {
          sprintf(NLEFErrorMsg,"Out of memory trying to allocate %d bytes",m*sizeof(double));
          NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
          return DBL_QNAN;
         }
        for(ii=0;ii<m;ii++)(F->Hs)[ii]=0.;
  
        F->dF0=(double*)malloc(m*sizeof(double));
        if(F->dF0==(double*)NULL)
         {
          sprintf(NLEFErrorMsg,"Out of memory trying to allocate %d bytes",m*sizeof(double));
          NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
          return DBL_QNAN;
         }
        for(ii=0;ii<m;ii++)(F->dF0)[ii]=NLEEvalDer(F,ii,n,x,data);
  
        if(verbose)
         {
          printf("   Current point x0=(%lf",(F->x0)[0]);for(ii=1;ii<m;ii++)printf(",%lf",(F->x0)[ii]);printf(")\n");fflush(stdout);
          printf("   Current gradient dF0=(%lf",(F->dF0)[0]);for(ii=1;ii<m;ii++)printf(",%lf",(F->dF0)[ii]);printf(")\n");fflush(stdout);
         }
  
        if(F->ddF0==(double*)NULL) /* i.e. no Initial Hessian given */
         {
          if(verbose){printf("   no Initial Hessian given\n");fflush(stdout);}
          m=F->nV*F->nV;
          F->ddF0=(double*)malloc(m*sizeof(double));
          if(F->ddF0==(double*)NULL)
           {
            sprintf(NLEFErrorMsg,"Out of memory trying to allocate %d bytes",m*sizeof(double));
            NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
            return DBL_QNAN;
           }
          for(ii=0;ii<m;ii++)(F->ddF0)[ii]=0.;
  /*      for(ii=0;ii<F->nV;ii++)(F->ddF0)[ii+F->nV*ii]=0.;*/
          for(ii=0;ii<F->nV;ii++)(F->ddF0)[ii+F->nV*ii]=1.;
         }else
          if(verbose){printf("   Initial Hessian given\n");fflush(stdout);}
  
        if(verbose)
         {
          if(verbose){printf("   Initial Hessian:\n");fflush(stdout);}
          for(ii=0;ii<F->nV;ii++)
           {
            printf("       [");
            for(jj=0;jj<F->nV;jj++)printf(" %14.7le",(F->ddF0)[ii+F->nV*jj]);
            printf("]\n");fflush(stdout);
           }
         }
  
        result=(F->ddF0)[i+n*j];
  
        NLEvaluateElementFunctionDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
        NLEvaluateElementFunctionDerNCalls++;
  
        return result;
       }
  
      m=F->nV;
      t=0.;
      for(ii=0;ii<m;ii++)t+=fabs(x[ii]-(F->x0)[ii]);
      if(verbose)
       {
        printf("   Previous point x0=(%lf",(F->x0)[0]);
        for(ii=1;ii<m;ii++)printf(",%lf",(F->x0)[ii]);
        printf(")\n");fflush(stdout);
       }
      if(t>1.e-5) /* New point, update Hessian */
       {
        if(verbose){printf("%s, type %d entry (%d,%d)\n",RoutineName,F->type,i,j);}
        if(verbose){printf("   New point, update Hessian ");if(F->DiffType==BFGS)printf(" - BFGS\n");if(F->DiffType==SR1)printf(" - SR1\n");fflush(stdout);}
  
        for(ii=0;ii<m;ii++)(F->y)[ii]=NLEEvalDer(F,ii,n,x,data);
  
        if(verbose)
         {
          printf("   Gradient at x0=(%lf",(F->x0)[0]);
          for(ii=1;ii<m;ii++)printf(",%lf",(F->x0)[ii]);
          printf(")=(");fflush(stdout);
          printf("%lf",(F->dF0)[0]);
          for(ii=1;ii<m;ii++)printf(",%lf",(F->dF0)[ii]);
          printf(")\n");fflush(stdout);
          printf("   Gradient at x =(%lf",x[0]);
          for(ii=1;ii<m;ii++)printf(",%lf",x[ii]);
          printf(")=(");fflush(stdout);
          printf("%lf",(F->y)[0]);
          for(ii=1;ii<m;ii++)printf(",%lf",(F->y)[ii]);
          printf(")\n");fflush(stdout);
         }
  
        if(verbose)
         {
          printf("   Previous Hessian:\n");fflush(stdout);
          for(ii=0;ii<F->nV;ii++)
           {
            printf("       [");
            for(jj=0;jj<F->nV;jj++)printf(" %14.7le",(F->ddF0)[ii+F->nV*jj]);
            printf("]\n");fflush(stdout);
           }
         }
  
        if(F->DiffType==BFGS)
         {
          sHs=0.;
          ys=0.;
          yy=0.;
          ss=0.;
          for(ii=0;ii<m;ii++)
           {
            (F->Hs)[ii]=0.;
            for(jj=0;jj<m;jj++)(F->Hs)[ii]+=(F->ddF0)[ii+m*jj]*(x[jj]-(F->x0)[jj]);
            sHs+=(x[ii]-(F->x0)[ii])*(F->Hs)[ii];
            ys+=((F->y)[ii]-(F->dF0)[ii])*(x[ii]-(F->x0)[ii]);
            yy+=((F->y)[ii]-(F->dF0)[ii])*((F->y)[ii]-(F->dF0)[ii]);
            ss+=(x[ii]-(F->x0)[ii])*(x[ii]-(F->x0)[ii]);
            if(0&&fabs((F->dF0)[ii]-(F->y)[ii])<2*F->NoiseLevel)
              printf("WARNING, Hessian update, derivative component %d being subtracted are %le,%le, while noise is %le\n",ii,(F->dF0)[ii],(F->y)[ii],F->NoiseLevel);
           }
    
          if(verbose)
           {
            printf("   Hs =(%lf",(F->Hs)[0]);for(ii=1;ii<m;ii++)printf(",%lf",(F->Hs)[ii]);printf(")\n");fflush(stdout);
            printf("   <s,Hs>=%le\n",sHs);
            printf("   <y,s>=%le\n",ys);fflush(stdout);
           }
    
          if(ss>F->NoiseLevel*F->NoiseLevel&&fabs(sHs)>skipBound*ss&&fabs(ys)>skipBound*yy)
           {
            invsHs=1./sHs;
            invys=1./ys;
    
            for(ii=0;ii<m;ii++)
             {
              for(jj=0;jj<m;jj++)
               {
                (F->ddF0)[ii+m*jj]-=invsHs*(F->Hs)[ii]*(F->Hs)[jj];
                (F->ddF0)[ii+m*jj]+=invys*((F->y)[ii]-(F->dF0)[ii])*((F->y)[jj]-(F->dF0)[jj]);
               }
             }
           }else{
            NLNumberOfUpdatesSkipped++;
           }
         }
  
        if(F->DiffType==SR1)
         {
          ys=0.;
          ss=0.;
          for(ii=0;ii<m;ii++)
           {
            (F->Hs)[ii]=0.;for(jj=0;jj<m;jj++)(F->Hs)[ii]+=(F->ddF0)[ii+m*jj]*(x[jj]-(F->x0)[jj]);
            ys+=((F->y)[ii]-(F->dF0)[ii]-(F->Hs)[ii])*(x[ii]-(F->x0)[ii]);
            ss+=((F->y)[ii]-(F->dF0)[ii]-(F->Hs)[ii])*((F->y)[ii]-(F->dF0)[ii]-(F->Hs)[ii]);
            if(0&&fabs((F->dF0)[ii]-(F->y)[ii])<2*F->NoiseLevel)
              printf("WARNING, Hessian update, derivative component %d being subtracted are %le,%le, while noise is %le\n",ii,(F->dF0)[ii],(F->y)[ii],F->NoiseLevel);
           }
          if(ss>F->NoiseLevel*F->NoiseLevel&&fabs(ys)>skipBound*ss)
           {
            invys=1./ys;
            for(ii=0;ii<m;ii++)
             {
              for(jj=0;jj<m;jj++)
               {
                (F->ddF0)[ii+m*jj]+=((F->y)[ii]-(F->dF0)[ii]-(F->Hs)[ii])*((F->y)[jj]-(F->dF0)[jj]-(F->Hs)[jj])*invys;
               }
             }
           }else{
            NLNumberOfUpdatesSkipped++;
           }
         }
  
        if(verbose)
         {
          printf("   Updated Hessian:\n");fflush(stdout);
          for(ii=0;ii<F->nV;ii++)
           {
            printf("       [");
            for(jj=0;jj<F->nV;jj++)printf(" %14.7le",(F->ddF0)[ii+F->nV*jj]);
            printf("]\n");fflush(stdout);
           }
         }
  
        for(ii=0;ii<m;ii++)
         {
          (F->x0)[ii]=x[ii];
          (F->dF0)[ii]=(F->y)[ii];
         }
       }else
        if(verbose){printf("   Same point, return Hessian entry\n");fflush(stdout);}
  
      result=(F->ddF0)[i+n*j];

      NLEvaluateElementFunctionDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
      NLEvaluateElementFunctionDerNCalls++;
  
      return result;
     }
  
    if(0&&i==0&&j==0)
     {
      printf("%s, type %d entry (%d,%d)\n",RoutineName,F->type,i,j);
      printf("   True Hessian:\n");fflush(stdout);
      for(ii=0;ii<F->nV;ii++)
       {
        printf("       [");
        for(jj=0;jj<F->nV;jj++)printf(" %14.7le",F->ddF(ii,jj,n,x,data));
        printf("]\n");fflush(stdout);
       }
     }
  
    if(verbose){printf("   Hessian given by user\n");fflush(stdout);}
    result=F->ddF(i,j,n,x,data);
   }else{
    if(verbose){printf("evaluating 2nd derivative (%d,%d) of: %s\n",i,j,F->expr);fflush(stdout);}
    ECEvaluateFunction(F->sddF[i+n*j],x,&result);
   }

  NLEvaluateElementFunctionDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateElementFunctionDerNCalls++;

  return result;
 }

NLMatrix NLEGetRangeXForm(NLElementFunction this)
 {
  char RoutineName[]="NLEGetRangeXForm";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif
  return this->R;
 }

int NLEGetType(NLElementFunction this)
 {
  char RoutineName[]="NLEGetType";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return -1;
   }
#endif
  return this->type;
 }

NLElementFunction NLCreateElementFunctionWithInitialHessian(NLProblem P,char *type,int n,
      NLMatrix R,double (*F)(int,double*,void*),double (*dF)(int,int,double*,void*),
      double (*ddF)(int,int,int,double*,void*),void *data,void (*freedata)(void*),NLMatrix ddF0)
 {
  char RoutineName[]="NLCreateElementFunctionWithInitialHessian";
  NLElementFunction result;
  int set;
  int i,j;
  static int verbose=0;

  if(R!=(NLMatrix)NULL && NLMGetNumberOfCols(R)<NLMGetNumberOfRows(R))
   {
    result=NLCreateElementFunction(P,type,NLMGetNumberOfCols(R),(NLMatrix)NULL,
      NLfcomp,NLdfcomp,NLddfcomp,NLfcompdata(F,dF,ddF,R,data,freedata),NLfreeNLfcompdata);
    return result;
   }

  result=(NLElementFunction)malloc(sizeof(struct NLGrpPartEFn));
  if(result==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartEFn));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  result->nV=n;
#ifndef NL_NOINPUTCHECKS
  if(!(n>0))
   {
    sprintf(NLEFErrorMsg,"Number of coordinates is not positive %d",n);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    free(result);
    return (NLElementFunction)NULL;
   }
#endif
  result->variablePower=(int*)malloc(n*sizeof(int));
  if(result->variablePower==(int*)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(int));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  for(i=0;i<n;i++)result->variablePower[i]=NLVARIABLEDEPENDENCENOTSET;
  set=R!=(NLMatrix)NULL;
  result->type=NLPAddElementType(P,type,set);
  result->R=R;
  if(set)NLRefMatrix(R);
  result->P=P;
  result->F=F;
  result->ddF0=(double*)NULL;
  if(ddF0!=(NLMatrix)NULL)
   {
    result->ddF0=(double*)malloc(n*n*sizeof(double));
    for(j=0;j<n;j++)
     {
      for(i=0;i<n;i++)
       {
        result->ddF0[i+n*j]=NLMGetElement(ddF0,i,j);
       }
     }
   }
  result->dF0=(double*)NULL;
  result->x0=(double*)NULL;
  result->y=(double*)NULL;
  result->Hs=(double*)NULL;
  result->dF=dF;
  result->ddF=ddF;
  result->data=data;
  result->freeData=freedata;
  result->nRefs=1;
  if(ddF==NULL)
    result->DiffType=NLDefaultElementUpdateType;
   else
    result->DiffType=EXACT;
  result->NoiseLevel=NLDefaultElementUpdateNoise;

  if(verbose){printf("%s: type %s, F=0x%8.8x, dF=0x%8.8x, ddF=0x%8.8x, data=0x%8.8x\n",RoutineName,type,F,dF,ddF,data);fflush(stdout);}

  result->expr=(char*)NULL;
  result->vars=(char*)NULL;
  result->sF=(ECFn)NULL;
  result->sdF=(ECFn*)NULL;
  result->sddF=(ECFn*)NULL;

  return(result);
 }

void NLElementFunctionGetInitialHessian(NLElementFunction this, double *h)
 {
  int i,j,n,offset;

  if(this->ddF0==(double*)NULL)return;

  n=this->nV;

  offset=0;
  for(i=0;i<n;i++)
   {
    for(j=0;j<=i;j++)
     {
      h[offset]=this->ddF0[i+n*j];
      offset++;
     }
   }

  return;
 }

double *NLElementFunctionGetInitialHessianMatrix(NLElementFunction this)
 {
  char RoutineName[]="NLElementFunctionGetInitialHessianMatrix";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (double*)NULL;
   }
#endif

  return this->ddF0;
 }

int NLEFAssertPolynomialOrderOfElementVariable(NLElementFunction this,int i,int p)
 {
  char RoutineName[]="NLEFAssertPolynomialOrderOfElementVariable";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return 12;
   }
  if(i<0 || i>=this->nV)
   {
    sprintf(NLEFErrorMsg,"variable %d (argument 2) is invalid, must be in [0,%d)",i,this->nV);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return 12;
   }
#endif

  this->variablePower[i]=p;
  return 0;
 }

int NLEFQueryPolynomialOrderOfElementVariable(NLElementFunction this,int i)
 {
  char RoutineName[]="NLEFQueryPolynomialOrderOfElementVariable";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Element Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return NLVARIABLEDEPENDENCENOTSET;
   }

  if(i<0 || i>=this->nV)
   {
    sprintf(NLEFErrorMsg,"variable %d (argument 2) is invalid, must be in [0,%d)",i,this->nV);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return NLVARIABLEDEPENDENCENOTSET;
   }
#endif

  return this->variablePower[i];
 }

void *NLEGetData(NLElementFunction this)
 {
  if(this==(NLElementFunction)NULL)abort();
  return this->data;
 }

void NLSetDefaultElementUpdateType(char *type)
 {
  char RoutineName[]="NLSetDefaultElementUpdateType";

  printf("%s to \"%s\"\n",RoutineName,type);
  if(!strcmp(type,"SR1"))
    NLDefaultElementUpdateType=SR1;
   else if(!strcmp(type,"BFGS"))
    NLDefaultElementUpdateType=BFGS;
   else if(!strcmp(type,"EXACT"))
    NLDefaultElementUpdateType=EXACT;
  return;
 }

int NLGetDefaultElementUpdateType()
 {
  return NLDefaultElementUpdateType;
 }

void NLSetDefaultElementUpdateNoise(double NoiseLevel)
 {
  NLDefaultElementUpdateNoise=NoiseLevel;

  return;
 }

double NLGetDefaultElementUpdateNoise()
 {
  return NLDefaultElementUpdateNoise;
 }

void NLSetElementUpdateType(NLElementFunction ef,char *type, int override)
 {
  if(ef->ddF==NULL||override)
   {
    if(!strcmp(type,"SR1"))
      ef->DiffType=SR1;
     else if(!strcmp(type,"BFGS"))
      ef->DiffType=BFGS;
     else if(!strcmp(type,"EXACT")&&ef->ddF!=NULL)
      ef->DiffType=EXACT;
   }

  return;
 }

int NLGetElementUpdateType(NLElementFunction ef)
 {
  return ef->DiffType;
 }

void NLSetElementUpdateNoise(NLElementFunction ef,double NoiseLevel)
 {
  ef->NoiseLevel=NoiseLevel;

  return;
 }

double NLGetElementUpdateNoise(NLElementFunction ef)
 {
  return ef->NoiseLevel;
 }

NLElementFunction NLCreateElementFunctionByString(NLProblem P,char *type,int n,NLMatrix
R,char *vars,char *expr)
 {
  char RoutineName[]="NLCreateElementFunctionByString";
  NLElementFunction result;
  int set;
  int i,j;
  int verbose;

#ifndef NL_NOINPUTCHECKS
  if(P==(NLProblem)NULL)
   {
    sprintf(NLEFErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  if(type==(char*)NULL)
   {
    sprintf(NLEFErrorMsg,"type (argument 1) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  if(vars==(char*)NULL)
   {
    sprintf(NLEFErrorMsg,"vars (argument 5) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  if(expr==(char*)NULL)
   {
    sprintf(NLEFErrorMsg,"expr (argument 6) is NULL");
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
#endif

  verbose=0;
  if(verbose){printf("%s\n",RoutineName);fflush(stdout);}

  if(R!=(NLMatrix)NULL && NLMGetNumberOfCols(R)<NLMGetNumberOfRows(R))
   {
    result=NLCreateElementFunctionByString(P,type,NLMGetNumberOfCols(R),(NLMatrix)NULL,vars,expr);
    return result;
   }

  result=(NLElementFunction)malloc(sizeof(struct NLGrpPartEFn));
  if(result==(NLElementFunction)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartEFn));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  result->nV=n;
#ifndef NL_NOINPUTCHECKS
  if(!(n>0))
   {
    sprintf(NLEFErrorMsg,"Number of coordinates is not positive %d",n);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    free(result);
    return (NLElementFunction)NULL;
   }
#endif
  result->variablePower=(int*)malloc(n*sizeof(int));
  if(result->variablePower==(int*)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(int));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  for(i=0;i<n;i++)result->variablePower[i]=NLVARIABLEDEPENDENCENOTSET;
  set=(R!=(NLMatrix)NULL);
  result->type=NLPAddElementType(P,type,set);
  result->R=R;
  if(set)NLRefMatrix(R);
  result->F=(double (*)(int,double*,void*))NULL;
  result->dF=(double (*)(int,int,double*,void*))NULL;
  result->ddF=(double (*)(int,int,int,double*,void*))NULL;

  result->ddF0=(double*)NULL;
  result->dF0=(double*)NULL;
  result->x0=(double*)NULL;
  result->y=(double*)NULL;
  result->Hs=(double*)NULL;
  result->data=(void*)NULL;
  result->freeData=(void (*)(void*))NULL;
  result->nRefs=1;
  result->P=P;
  result->DiffType=EXACT;
  result->NoiseLevel=NLDefaultElementUpdateNoise;

  result->sF=ECCreateFunction(vars,expr);

  result->expr=(char*)malloc((strlen(expr)+1)*sizeof(char));
#ifndef NL_NOINPUTCHECKS
  if(result->expr==(char*)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(expr)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
#endif
  strcpy(result->expr,expr);

  n=ECFunctionM(result->sF);
#ifndef NL_NOINPUTCHECKS
  if(ECFunctionN(result->sF)!=1)
   {
    sprintf(NLEFErrorMsg,"String %s represents a vector valued function. Must be a scalar.",expr);
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
#endif

  result->vars=(char*)malloc((strlen(vars)+1)*sizeof(char));
  if(result->vars==(char*)NULL)
   {
    sprintf(NLEFErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(vars)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLEFErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }
  strcpy(result->vars,vars);

  result->sdF=(ECFn*)malloc(n*sizeof(ECFn));
  for(i=0;i<n;i++)
    result->sdF[i]=ECCreateDerivativeOfFunction(result->sF,i);

  result->sddF=(ECFn*)malloc(n*n*sizeof(ECFn));
  for(i=0;i<n;i++)
   for(j=0;j<n;j++)
    result->sddF[i+n*j]=ECCreateDerivativeOfFunction(result->sdF[i],j);

  return(result);
 }

char *NLEGetExpr(NLElementFunction ef)
 {
  return ef->expr;
 }
