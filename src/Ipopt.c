/*  (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory */

/*      author: Andreas Waechter andreasw@watson.ibm.com */
/*      version: %W% %D% %T% */
/*      date:   Sep. 1, 2003                         */

#include <NLPAPI.h>
#include <stdio.h>
#include <string.h>

F77DOUBLEPRECISION F77_FUNC(ipopt,IPOPT)(F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,void *(),void *(),void *(),void *(),void *(),void *(),void *(),void *(),F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,char*,int);
void F77_FUNC_(ev_hlv_dummy,EV_HLV_DUMMY)(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void F77_FUNC_(ev_hov_dummy,EV_HOV_DUMMY)(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void F77_FUNC_(ev_hcv_dummy,EV_HCV_DUMMY)(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);

void EV_F(F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_G(F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_C(F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_A(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_H(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_HLV(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_HOV(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);
void EV_HCV(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*);

static int verbose=0;
static char IPErrorMsg[256]="";

#define CARGSLEN 20
#define ARGSINC 5

struct NLIpoptSolver
{
  F77INTEGER nargs;
  F77INTEGER margs;
  F77DOUBLEPRECISION *args;
  char *cargs;
};

void NLPSetLagrangianMultipliers(NLProblem P, NLVector lambda);
void NLPConvertToLagrangianFunction(NLProblem P);

int *NLNEGetElementVariables(NLProblem P, NLNonlinearElement t);
int* NLMRow(NLMatrix this);
int* NLMCol(NLMatrix this);
double *NLMData(NLMatrix this);

static NLProblem PEq=(NLProblem)NULL;
static NLProblem PLag=(NLProblem)NULL;
static NLMatrix lnH=(NLMatrix)NULL;

/* Store structure of Jacobian and Hessian */
#define JACNZMAXINC 1000
static JACNZMAX = 0;
static int JACNZ=-1;
static int *JACCON=NULL;
static int *JACVAR=NULL;
static int HESSNZ=-1;
static int *HESSROW=NULL;
static int *HESSCOL=NULL;

/* Last point at which something was evaluated */
static double *XLAST=NULL;
static double *LAMLAST=NULL;

void MySort(int *iarray, int *len);

NLIpopt NLCreateIpopt()
{
  char RoutineName[]="NLCreateLancelot";

  NLIpopt this=(NLIpopt)malloc(sizeof(struct NLIpoptSolver));
  if(this==(NLIpopt)NULL)
   {
    sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLIpoptSolver));
    NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
    return (NLIpopt)NULL;
   }

  this->nargs=0;
  this->margs=0;
  this->args=NULL;
  this->cargs=NULL;

  return(this);
}

void NLFreeIpopt(NLIpopt Ip)
{
  char RoutineName[]="NLFreeIpopt";

  if(Ip==(NLIpopt)NULL)
    {
      sprintf(IPErrorMsg,"Solver (argument 1) is NULL");
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return;
    }

  if(Ip->nargs>0)
    {
      free(Ip->args);
      free(Ip->cargs);
    }
  free(Ip);
}

void IPAddOption(NLIpopt this, char *carg, double arg)
{
  char RoutineName[]="IPAddOption";
  size_t len;
  char *p;

  if(this==(NLIpopt)NULL)
    {
      sprintf(IPErrorMsg,"Solver (argument 1) is NULL");
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return;
    }
  if(carg==(char*)NULL)
    {
      sprintf(IPErrorMsg,"Option name (argument 2) is NULL");
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return;
    }
  if(this->nargs==this->margs)
    {
      this->margs+=ARGSINC;
      this->cargs=(char*)realloc((void*)this->cargs,(this->margs)*sizeof(char)*CARGSLEN);
      if(this->cargs==(char*)NULL)
	{
	  sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",(this->margs)*sizeof(char)*CARGSLEN);
	  NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
	  this->margs-=ARGSINC;
	  return;
	}
      this->args=(F77DOUBLEPRECISION*)realloc((void*)this->args,(this->margs)*sizeof(F77DOUBLEPRECISION));
      if(this->args==(F77DOUBLEPRECISION*)NULL)
	{
	  sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",(this->margs)*sizeof(F77DOUBLEPRECISION));
	  NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
	  this->margs-=ARGSINC;
	  return;
	}
    }

  p=&(this->cargs)[(this->nargs)*CARGSLEN];
  len=strlen(carg);
  if(len>CARGSLEN)len=CARGSLEN;
  memcpy(p,carg,len);
  if(len<CARGSLEN)
      memset(p+len,' ',CARGSLEN-len);
  this->args[this->nargs]=(F77DOUBLEPRECISION)arg;
  this->nargs++;
  return;
}

int IPMinimize(NLIpopt this,NLProblem P,double *x0,double *z0,double *l0,double *x)
{
  char RoutineName[]="IPMinimize";

#ifndef HAVE_IPOPT
  sprintf(IPErrorMsg,"IPOPT is not installed on this system (or wasn't when NLPAPI was configured)");
  NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
  return 12;
#else
  int i, nXorig, nMinMax, nIneq, ix;
  F77INTEGER N,M,NLB,NUB,*pILB,*pIUB,IERR,ITER;
  F77DOUBLEPRECISION *pBNDS_L,*pBNDS_U,*pX,*pV_L,*pV_U,*pLAM,*pC;
  F77INTEGER *ibuffer, zero=0;
  F77DOUBLEPRECISION *dbuffer;
  double val, bl;
  double *Slackinit;
  NLVector lnx;

  /* Get the number of variables of the original problem (for setting starting points later...) */
  nXorig=NLPGetNumberOfVariables(P);
  nMinMax=NLPGetNumberOfMinMaxConstraints(P);
  nIneq=NLPGetNumberOfInequalityConstraints(P);
  if(nIneq>0)
    {
      NLPInvalidateGroupAndElementCaches(P);  /* TODO */  
      lnx=NLCreateDenseWrappedVector(nXorig,x0);
      Slackinit=(double*)malloc(nIneq*sizeof(double));
      if(Slackinit==(double*)NULL)
	{
	  sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",nIneq*sizeof(double));
	  NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
	  return -1;
	}
      for(i=0;i<nIneq;i++)
	{
	  if( NLPIsInequalityConstraintLowerBoundSet(P,i) )
	    {
	      val=1./NLPGetGroupScale(P,NLPGetInequalityConstraintGroupNumber(P,i));
	      bl = NLPGetInequalityConstraintLowerBound(P,i);
	      Slackinit[i]=NLPEvaluateInequalityConstraint(P,i,lnx)-val*bl;
	    }
	  else
	    {
	      Slackinit[i]=-NLPEvaluateInequalityConstraint(P,i,lnx);
	    }
	}
      NLFreeVector(lnx);
    }

  /* Reformulate inequality constraints */
  PEq=NLCopyProblem(P);
  NLPConvertToEqualityAndBoundsOnly(PEq);

  /* Create new Problem for the computation of the Hessian */
  PLag=NLCopyProblem(PEq);
  NLPConvertToLagrangianFunction(PLag);

  /* Getting problem size */
  N=NLPGetNumberOfVariables(PEq);
  M=NLPGetNumberOfEqualityConstraints(PEq);

  /* Determine lower and upper bounds */
  dbuffer=(F77DOUBLEPRECISION*)malloc(N*sizeof(F77DOUBLEPRECISION));
  if(dbuffer==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",N*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  ibuffer=(F77INTEGER*)malloc(N*sizeof(F77INTEGER));
  if(ibuffer==(F77INTEGER*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",N*sizeof(F77INTEGER));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }

  NLB=0;
  for(i=0;i<N;i++)
    {
      if(NLPIsLowerSimpleBoundSet(PEq,i))
	{
	  val=NLPGetLowerSimpleBound(PEq,i);
	  ibuffer[NLB]=i+1;
	  dbuffer[NLB]=(F77DOUBLEPRECISION)val;
	  NLB++;
	}
    }
  pBNDS_L=(F77DOUBLEPRECISION*)malloc(NLB*sizeof(F77DOUBLEPRECISION));
  if(pBNDS_L==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",NLB*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  memcpy(pBNDS_L,dbuffer,NLB*sizeof(F77DOUBLEPRECISION));
  pILB=(F77INTEGER*)malloc(NLB*sizeof(F77INTEGER));
  if(pILB==(F77INTEGER*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",NLB*sizeof(F77INTEGER));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  memcpy(pILB,ibuffer,NLB*sizeof(F77INTEGER));

  NUB=0;
  for(i=0;i<N;i++)
    {
      if(NLPIsUpperSimpleBoundSet(PEq,i))
	{
	  val=NLPGetUpperSimpleBound(PEq,i);
	  ibuffer[NUB]=i+1;
	  dbuffer[NUB]=val;
	  NUB++;
	}
    }
  pBNDS_U=(F77DOUBLEPRECISION*)malloc(NUB*sizeof(F77DOUBLEPRECISION));
  if(pBNDS_U==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",NLB*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  memcpy(pBNDS_U,dbuffer,NUB*sizeof(F77DOUBLEPRECISION));
  pIUB=(int*)malloc(NUB*sizeof(F77INTEGER));
  if(pIUB==(F77INTEGER*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",NLB*sizeof(F77INTEGER));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  memcpy(pIUB,ibuffer,NUB*sizeof(F77INTEGER));

  free(ibuffer);
  free(dbuffer);

  /* Get memory for the iterates and multipliers */
  pX=(F77DOUBLEPRECISION*)malloc(N*sizeof(F77DOUBLEPRECISION));
  if(pX==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",N*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  pV_L=(F77DOUBLEPRECISION*)malloc(NLB*sizeof(F77DOUBLEPRECISION));
  if(pV_L==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",NLB*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  pV_U=(F77DOUBLEPRECISION*)malloc(NUB*sizeof(F77DOUBLEPRECISION));
  if(pV_U==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",NUB*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  pLAM=(F77DOUBLEPRECISION*)malloc(M*sizeof(F77DOUBLEPRECISION));
  if(pLAM==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",M*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }
  pC=(F77DOUBLEPRECISION*)malloc(M*sizeof(F77DOUBLEPRECISION));
  if(pC==(F77DOUBLEPRECISION*)NULL)
    {
      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",M*sizeof(F77DOUBLEPRECISION));
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return -1;
    }

  /* Set initial point */
  for(i=0;i<nXorig;i++)
    pX[i]=x0[i];
  if(nMinMax!=0)
    {
      pX[nXorig]=*z0;
      ix=nXorig+1;
    }
  else
    {
      ix=nXorig;
    }

  /* Get values of inequality constraints for initialization of slacks */
  for(i=0;i<nIneq;i++)
    pX[ix+i]=Slackinit[i];

  F77_FUNC(ipopt,IPOPT)(&N,pX,&M,&NLB,pILB,pBNDS_L,&NUB,pIUB,pBNDS_U,
			pV_L,pV_U,pLAM,pC,&zero,NULL,&zero,NULL,&ITER,&IERR,
			(void *(*)())EV_F, (void *(*)())EV_C,
	       (void *(*)())EV_G, (void *(*)())EV_A, (void *(*)())EV_H,
	       (void *(*)())EV_HLV, (void *(*)())EV_HOV, (void *(*)())EV_HCV,
			NULL,NULL,&(this->nargs),this->args,
			this->cargs,CARGSLEN);
  if ( IERR != 0 )
    {
      sprintf(IPErrorMsg,"IPOPT returned IERR = %d\n.",IERR);
      printf("%s",IPErrorMsg);fflush(stdout);
    }

  /* Copy solution */
  for(i=0;i<nXorig;i++)
    x[i]=pX[i];

  /* Free memory */
  free(pC);
  free(pLAM);
  free(pV_U);
  free(pV_L);
  free(pX);
  free(pIUB);
  free(pBNDS_U);
  free(pILB);
  free(pBNDS_L);

  NLFreeProblem(PLag);
  NLFreeProblem(PEq);
  if(lnH!=(NLMatrix)NULL)
    {
      free(lnH);
      lnH=(NLMatrix)NULL;
    }

  if(nIneq>0)free(Slackinit);
  if(JACNZ!=-1)
    {
      free(JACCON);
      free(JACVAR);
      JACNZ=-1;
    }
  if(HESSROW!=NULL)
    {
      free(HESSROW);
      HESSROW=NULL;
    }
  if(HESSCOL!=NULL)
    {
      free(HESSCOL);
      HESSCOL=NULL;
    }

  return IERR;
#endif
}

/* This routine checks whether the cache in the API has to be invalidated */
void checkinv( int *pN, double *pX, int *pM, double *pLAM )
{
  char RoutineName[]="checkinv";

  int i;
  NLVector lnv;

  int changedX=FALSE, changedLAM=FALSE;

  if( !XLAST )
    {
      XLAST=(double*)malloc((*pN)*sizeof(double));
      if(XLAST==(double*)NULL)
	{
	  sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes for XLAST",(*pN)*sizeof(double));
	  NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
	  printf("%s",IPErrorMsg);fflush(stdout);
	  exit(1);
	}
      memcpy(XLAST,pX,(*pN)*sizeof(double));
      changedX=TRUE;
    }
  else
    {
      if(memcmp(pX,XLAST,(*pN)*sizeof(double)))
	{
	  memcpy(XLAST,pX,(*pN)*sizeof(double));
	  changedX=TRUE;
	}
    }

  if( *pM>0 )
    {
      if( !LAMLAST )
	{
	  LAMLAST=(double*)malloc((*pM)*sizeof(double));
	  if(LAMLAST==(double*)NULL)
	    {
	      sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes for LAMLAST",(*pN)*sizeof(double));
	      NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
	      printf("%s",IPErrorMsg);fflush(stdout);
	      exit(1);
	    }
	  memcpy(LAMLAST,pLAM,(*pM)*sizeof(double));
	  changedLAM=TRUE;
	}
      else
	{
	  if(memcmp(pLAM,LAMLAST,(*pM)*sizeof(double)))
	    {
	      memcpy(LAMLAST,pLAM,(*pM)*sizeof(double));
	      changedLAM=TRUE;
	    }
	}
    }

  /* Invalidate caches */
  if( *pM>0 && (changedX || changedLAM) )
    {
      NLPInvalidateGroupAndElementCaches(PLag);
      lnv=NLCreateDenseWrappedVector(*pM,pLAM);
      NLPSetLagrangianMultipliers(PLag,lnv);
      NLFreeVector(lnv);
    } 

  if( changedX )
    {
      NLPInvalidateGroupAndElementCaches(PEq);
    }

}

/* Evaluation of objective function */
void EV_F(F77INTEGER *N, F77DOUBLEPRECISION *X, F77DOUBLEPRECISION *F,
	  F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  NLVector lnx;
  int verbose=0;
  int izero=0;

  checkinv( N, X, &izero, NULL );

  lnx=NLCreateDenseWrappedVector(*N,X);
  if(verbose) NLPrintVector(stdout,lnx);

  *F=NLPEvaluateObjective(PEq,lnx);
  if(verbose) printf("F=%f\n",*F);

  NLFreeVector(lnx);
}

/* Evaluation of gradient of objective function */
void EV_G(F77INTEGER *N, F77DOUBLEPRECISION *X, F77DOUBLEPRECISION *G,
	  F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  NLVector lnx,lng;
  int verbose=0;

  int izero=0;

  checkinv( N, X, &izero, NULL );

  lnx=NLCreateDenseWrappedVector(*N,X);
  lng=NLCreateDenseWrappedVector(*N,G);
  if(verbose){printf("X for G = \n");NLPrintVector(stdout,lnx);printf("\n");fflush(stdout);}

  NLPEvaluateGradientOfObjective(PEq,lnx,lng);
  if(verbose){printf("G = \n");NLPrintVector(stdout,lng);printf("\n");fflush(stdout);}

  NLFreeVector(lng);
  NLFreeVector(lnx);
}

/* Evaluation of constraint functions */
void EV_C(F77INTEGER *N, F77DOUBLEPRECISION *X, F77INTEGER *M, F77DOUBLEPRECISION *C,
	  F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  int i;
  NLVector lnx;
  int izero=0;

  checkinv( N, X, &izero, NULL );

  lnx=NLCreateDenseWrappedVector(*N,X);

  for(i=0;i<*M;i++)
    C[i]=NLPEvaluateEqualityConstraint(PEq,i,lnx);

  NLFreeVector(lnx);
}

void EV_A(F77INTEGER *TASK, F77INTEGER *N,
	  F77DOUBLEPRECISION *X, F77INTEGER *NZ,
	  F77DOUBLEPRECISION *A, F77INTEGER *ACON,
	  F77INTEGER *AVAR,
	  F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  char RoutineName[]="eval_a";
  int verbose=0, izero=0;

  NLVector lnx,lng,vecA;
  int icon,nnz,i,group,iele,nev,inz;
  int *nz;
  int *ev;
  NLNonlinearElement e;
  double *g;

  if(*TASK==0)
    {
      /* Determine sparcity structure of constraint Jacobian */
      JACNZ=0;
      for(icon=0;icon<NLPGetNumberOfEqualityConstraints(PEq);icon++)
	{
	  group=NLPGetEqualityConstraintGroupNumber(PEq,icon);
	  if(NLPIsGroupASet(PEq,group))
	    {
	      vecA=NLPGetGroupA(PEq,group);
	      nnz=NLVGetNumberOfNonZeros(vecA);
	      nz=(int*)malloc(nnz*sizeof(int));
	      if(nz==(int*)NULL){ sprintf(IPErrorMsg,"Out of memory");NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__); return; }
	      for(i=0;i<nnz;i++) nz[i]=NLVGetNonZeroCoord(vecA,i);
	      MySort(nz,&nnz);
	    }
	  else
	    {
	      nnz=0;
	      nz=(int*)NULL;
	    }
	  for(iele=0;iele<NLPGetNumberOfElementsInGroup(PEq,group);iele++)
	    {
	      e=NLPGetNonlinearElementOfGroup(PEq,group,iele);
	      ev=NLNEGetElementVariables(PEq,e);
	      nev=NLNEGetElementDimension(PEq,e);
	      nz=(int*)realloc((void*)nz,(nnz+nev)*sizeof(int));
	      if(nz==(int*)NULL){ sprintf(IPErrorMsg,"Out of memory");NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__); return; }
	      for(i=0;i<nev;i++) nz[nnz+i]=ev[i];
	      nnz=nnz+nev;
	      MySort(nz,&nnz);
	    }
	  if(JACNZ+nnz>=JACNZMAX)
	    {
	      JACNZMAX+=nnz+JACNZMAXINC;
	      JACCON=(int*)realloc((void*)JACCON,JACNZMAX*sizeof(int));
	      if(JACCON==(int*)NULL){ sprintf(IPErrorMsg,"Out of memory for JACCON");NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__); exit(-1); }
	      JACVAR=(int*)realloc((void*)JACVAR,JACNZMAX*sizeof(int));
	      if(JACVAR==(int*)NULL){ sprintf(IPErrorMsg,"Out of memory for JACVAR");NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__); exit(-1); }
	    }
	  for(i=0;i<nnz;i++)
	    {
	      JACCON[JACNZ+i]=icon+1;
	      JACVAR[JACNZ+i]=nz[i]+1;
	    }
          if( nz ) free(nz);
          nz=(int*)NULL;
	  JACNZ+=nnz;
	}
      *NZ=JACNZ;
      return;
    }

  /* Test if Jacobian structure is already known */
  if(JACNZ==-1)
    {
      sprintf(IPErrorMsg,"Jacobian structure not yet known.");
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      exit(-1);
    }
  *NZ=JACNZ;

  /* Check if point has changed */
  checkinv( N, X, &izero, NULL );

  lnx=NLCreateDenseWrappedVector(*N,X);
  g=(double*)malloc((*N)*sizeof(double));
  lng=NLCreateDenseWrappedVector(*N,g);
  NLSetDontInitGradToZero(TRUE);

  inz=0;
  for(icon=0;icon<NLPGetNumberOfEqualityConstraints(PEq);icon++)
    {
      NLPEvaluateGradientOfEqualityConstraint(PEq,icon,lnx,lng);
      while(inz<JACNZ && JACCON[inz]-1==icon)
	{
	  A[inz]=g[JACVAR[inz]-1];
	  g[JACVAR[inz]-1]=0.;
	  inz++;
	}
    }
  if(inz!=*NZ)
    {
      sprintf(IPErrorMsg,"Internal error in eval_a; inz=%d NZ=%d",inz,*NZ);
      NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      abort();
    }
  for(inz=0;inz<*NZ;inz++)
    {
      AVAR[inz]=JACVAR[inz];
      ACON[inz]=JACCON[inz];
    }

  NLFreeVector(lng);
  free(g);
  NLFreeVector(lnx);
  NLSetDontInitGradToZero(FALSE);

  if(verbose)
    {
      for(i=0;i<*NZ;i++)
	{
	  printf("A[%d,%d]=%f\n",ACON[i],AVAR[i],A[i]);
	}
    }
}

void EV_H(F77INTEGER *TASK, F77INTEGER *N,
	  F77DOUBLEPRECISION *X, F77INTEGER *M,
	  F77DOUBLEPRECISION *LAM, F77INTEGER *NNZH, 
	  F77DOUBLEPRECISION *HESS, F77INTEGER *IRNH,
	  F77INTEGER *ICNH,
	  F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  char RoutineName[]="eval_h";
  int verbose=0;

  NLVector lnx;
  int *row,*col;
  int i,j;
  double *data;

  /* Set Lagrangian multipliers and if necessary invalidate cache */
  checkinv( N, X, M, LAM );

  if(*TASK==0) /* Get number of nonzeros */
    {
      if( lnH==(NLMatrix)NULL )
	{
	  lnH=NLCreateWSMPSparseMatrix(*N); /* We store this until the end */
	  lnx=NLCreateDenseWrappedVector(*N,X);
	  NLPEvaluateHessianOfObjective(PLag,lnx,lnH); /* This determines sparcity structure */

	  row=NLMRow(lnH);
	  col=NLMCol(lnH);
	  if(HESSROW!=NULL)free(HESSROW);
	  if(HESSCOL!=NULL)free(HESSCOL);
	  HESSNZ=row[*N];
	  if(HESSNZ>0)
	    {
	      HESSROW=(int*)malloc(HESSNZ*sizeof(int));
	      if(HESSROW==(int*)NULL)
		{
		  sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes for HESSROW",HESSNZ*sizeof(int));
		  NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
		  exit(-1);
		}
	      HESSCOL=(int*)malloc(HESSNZ*sizeof(int));
	      if(HESSCOL==(int*)NULL)
		{
		  sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes for HESSCOL",HESSNZ*sizeof(int));
		  NLSetError(12,RoutineName,IPErrorMsg,__LINE__,__FILE__);
		  exit(-1);
		}
	      HESSNZ=0;
	      for(i=0;i<*N;i++)
		{
		  for(j=row[i];j<row[i+1];j++)
		    {
		      HESSROW[HESSNZ]=i+1;
		      HESSCOL[HESSNZ]=col[HESSNZ]+1;
		      HESSNZ++;
		    }
		}
	    }
	  NLFreeVector(lnx);
	}
      *NNZH=NLMnE(lnH);
    }
  else
    {

      lnx=NLCreateDenseWrappedVector(*N,X);
      NLPEvaluateHessianOfObjective(PLag,lnx,lnH);

      data=NLMData(lnH);
      for(i=0;i<HESSNZ;i++)
	{
	  HESS[i]=data[i];
	  IRNH[i]=HESSROW[i];
	  ICNH[i]=HESSCOL[i];
	  if(verbose){printf("H[%d,%d]=%f\n",IRNH[i],ICNH[i],HESS[i]);}
	}

      *NNZH=NLMnE(lnH);
      NLFreeVector(lnx);
    }
}

void EV_HLV(F77INTEGER *TASK, F77INTEGER *N,
	    F77DOUBLEPRECISION *X, F77INTEGER *M,
            F77DOUBLEPRECISION *LAM, F77DOUBLEPRECISION *VIN, F77DOUBLEPRECISION *VOUT,
            F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  F77_FUNC_(ev_hlv_dummy,EV_HLV_DUMMY)
    (TASK, N, X, M, LAM, VIN, VOUT, DAT, IDAT);
}

void EV_HOV(F77INTEGER *TASK, F77INTEGER *N,
	    F77DOUBLEPRECISION *X, F77INTEGER *M,
            F77DOUBLEPRECISION *VIN, F77DOUBLEPRECISION *VOUT,
            F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  F77_FUNC_(ev_hov_dummy,EV_HOV_DUMMY)
    (TASK, N, X, M, VIN, VOUT, DAT, IDAT);
}

void EV_HCV(F77INTEGER *TASK, F77INTEGER *N,
	    F77DOUBLEPRECISION *X, F77INTEGER *M,
            F77DOUBLEPRECISION *LAM, F77DOUBLEPRECISION *VIN, F77DOUBLEPRECISION *VOUT,
            F77DOUBLEPRECISION *DAT, F77INTEGER *IDAT)
{
  F77_FUNC_(ev_hcv_dummy,EV_HCV_DUMMY)
    (TASK, N, X, M, LAM, VIN, VOUT, DAT, IDAT);
}

void MySort(int *iarray, int *len)
{
  /* Sort integer array and remove double entries */
  int i,j,t;

  for(i=0;i<*len;i++)
    {
      for(j=i+1;j<*len;j++)
	{
	  if(iarray[i]>iarray[j])
	    {
	      t=iarray[i];
	      iarray[i]=iarray[j];
	      iarray[j]=t;
	    }
	}
    }

  j=0;
  for(i=0;i<*len-1;i++)
    {
      if(iarray[j]!=iarray[i+1])
	{
	  j++;
	  iarray[j]=iarray[i+1];
	}
    }
  *len=j+1;
}

static int* lagGroup=NULL;
static int  nlagGroup=0;   /* Store length of previous array */

void NLPConvertToLagrangianFunction(NLProblem P)
 {
  char RoutineName[]="NLPConvertToLagrangianFunction";

/* Local variables */

  int i;
  int j,k;
  int cgroup;
  int group;
  char type[128];
  int verbose=0;

  if(P==(NLProblem)NULL)
   {
    sprintf(IPErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(NLPGetNumberOfInequalityConstraints(P)!=0 || NLPGetNumberOfMinMaxConstraints(P)!=0 )
   {
    sprintf(IPErrorMsg,"Problem has inequality or MinMax constraints");
    NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
    return;
   }
  nlagGroup=NLPGetNumberOfEqualityConstraints(P);
  if(nlagGroup==0) return; /* No equality constraints to be added */

  if(lagGroup!=NULL)
   {
     free(lagGroup);
   }
  lagGroup=(int*)malloc(nlagGroup*sizeof(int));
  if(lagGroup==(int*)NULL)
   {
    sprintf(IPErrorMsg,"Out of memory, trying to allocate %d bytes",(nlagGroup*sizeof(int)));
    NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
    printf("%s",IPErrorMsg);fflush(stdout);
    return;
   }

  for(i=0;i<nlagGroup;i++)
   {
    if(NLPGetNumberOfEqualityConstraintGroups(P,i)>1)
     {
      sprintf(IPErrorMsg,"!!!!Equality Constraint %d has %d groups. This is not supported yet.\n",i,NLPGetNumberOfEqualityConstraintGroups(P,i));
      NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      printf("%s",IPErrorMsg);fflush(stdout);
      return;
     }
    group=NLPGetEqualityConstraintNumberOfGroup(P,i,0);
    if(NLPGetGroupFunction(P,group)!=NULL)
     {
      sprintf(IPErrorMsg,"!!!!Equality Constraint %d has a nontrivial group function. This is not supported yet.\n",i);
      NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      printf("%s",IPErrorMsg);fflush(stdout);
      return;
     }

/*  Add lambda^T c(x) to the objective function  */

    cgroup=NLPGetEqualityConstraintNumberOfGroup(P,i,0);

    sprintf(type,"Lambda[%d]*C[%d]",i,i);
    lagGroup[i]=NLPAddGroupToObjective(P,type);
    group=NLPGetObjectiveGroupNumber(P,lagGroup[i]);
    if(NLPGetGroupA(P,cgroup)!=(NLVector)NULL)
     NLPSetGroupA(P,group,NLPGetGroupA(P,cgroup));
    if(NLPIsGroupBSet(P,cgroup))
     NLPSetGroupB(P,group,NLPGetGroupB(P,cgroup));

    for(j=0;j<NLPGetNumberOfElementsInGroup(P,cgroup);j++)
     {
      NLPAddNonlinearElementToObjectiveGroup(P,lagGroup[i],
         NLPGetElementWeight(P,cgroup,j),
         NLPGetGroupNonlinearElement(P,cgroup,j));
     }
   }

  NLPHideEqualityConstraints(P);

  if(verbose)
   {
    printf("-------------------------------------------------------------\n");
    printf("\n\nAfter converting to Lagrangian function:\n\n");
    NLPrintProblemShort(stdout,P);
    printf("-------------------------------------------------------------\n");
    fflush(stdout);
   }

  return;
 }

void NLPSetLagrangianMultipliers(NLProblem P, NLVector lambda)
{
  char RoutineName[]="NLPSetLagrangianMultipliers";

/* Local variables */

  int i;
  int group;
  int verbose=0;

  if(P==(NLProblem)NULL)
    {
      sprintf(IPErrorMsg,"Problem (argument 1) is NULL");
      NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return;
    }

  if(P==(NLProblem)NULL)
    {
      sprintf(IPErrorMsg,"lambda (argument 2) is NULL");
      NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return;
    }

  if(NLPGetNumberOfEqualityConstraints(P)!=0 || NLPGetNumberOfInequalityConstraints(P)!=0 || NLPGetNumberOfMinMaxConstraints(P)!=0 )
    {
      sprintf(IPErrorMsg,"Lagrangian Problem has constraints");
      NLSetError(4,RoutineName,IPErrorMsg,__LINE__,__FILE__);
      return;
    }

  for(i=0;i<nlagGroup;i++)
    {
      if(verbose)printf("lambda[%d]=%g\n",i,NLVGetC(lambda,i));
      group=NLPGetObjectiveGroupNumber(P,lagGroup[i]);
      NLPSetInvGroupScale(P,group,NLVGetC(lambda,i));
    }
}
