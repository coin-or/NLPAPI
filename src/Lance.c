/*  (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory */

/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: %W% %D% %T% */
/*      date:   Nov. 11, 1997                         */
/*              Feb. 2, 1999   Ported to C            */
/*              Sep. 31, 2000  Added fix to output for nvars>999999 */
/*              Sep. 6, 2000   Added fixes to output %6d for >999999 */
/*              Oct. 25, 2000  Added signalling in gminma for returning 
                               obj. group number */
/*              June 12, 2001  Added stuff to make it compatibile with 
                               Vanilla Lancelot */
/*              Jan. 29, 2002  Fixed counting problems with NLNOBJ etc. */
/*                             Fixed problem with default lower simple bounds. */
/*      version: %W% %D% %T% */

#include <NLPAPI.h>
#include <string.h>
#include <ctype.h>

void NLSetError(int,char*,char*,int,char*);
static char LNLanceErrorMsg[256]="";

void F77_FUNC(elfuns,ELFUNS)(F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*);
void F77_FUNC(groups,GROUPS)(F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77LOGICAL*);
void F77_FUNC(ranges,RANGES)(F77INTEGER*,F77LOGICAL*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*);
void F77_FUNC(gminma,GMINMA)(F77INTEGER*,F77DOUBLEPRECISION*);
void F77_FUNC(settyp,SETTYP)(F77INTEGER*,F77INTEGER*);

void LNDumpSpecFile(NLLancelot);
double NLPGetObjectiveLowerBound(NLProblem);
double NLPGetObjectiveUpperBound(NLProblem);
void LNSetMaxMin(NLLancelot,int);
void LNSetProblem(NLProblem);
void LNSetZ0(double*);
void LNSetZGroup(int);

struct NLLancelotSolver
 {
  int MAXIMIZER_SOUGHT;
  int PRINT_LEVEL;
  int START_PRINTING_AT_ITERATION;
  int STOP_PRINTING_AT_ITERATION;
  int ITERATIONS_BETWEEN_PRINTING;
  int MAXIMUM_NUMBER_OF_ITERATIONS;
  int SAVE_DATA_EVERY;
  int CHECK_ALL_DERIVATIVES;
  int CHECK_DERIVATIVES;
  int CHECK_ELEMENT_DERIVATIVES;
  int CHECK_GROUP_DERIVATIVES;
  int IGNORE_DERIVATIVE_WARNINGS;
  int IGNORE_ELEMENT_DERIVATIVE_WARNINGS;
  int IGNORE_GROUP_DERIVATIVE_WARNINGS;
  int FINITE_DIFFERENCE_GRADIENTS;
  int EXACT_SECOND_DERIVATIVES_USED;
  int BFGS_APPROXIMATION_USED;
  int DFP_APPROXIMATION_USED;
  int PSB_APPROXIMATION_USED;
  int SR1_APPROXIMATION_USED;
  int USE_SCALING_FACTORS;
  int USE_CONSTRAINT_SCALING_FACTORS;
  int USE_VARIABLE_SCALING_FACTORS;
  int GET_SCALING_FACTORS;
  int PRINT_SCALING_FACTORS;
  double CONSTRAINT_ACCURACY_REQUIRED;
  double GRADIENT_ACCURACY_REQUIRED;
  double INITIAL_PENALTY_PARAMETER;
  double DECREASE_PENALTY_PARAMETER_UNTIL;
  double FIRST_CONSTRAINT_ACCURACY_REQUIRED;
  double FIRST_GRADIENT_ACCURACY_REQUIRED;
  int INFINITY_NORM_TRUST_REGION_USED;
  int TWO_NORM_TRUST_REGION_USED;
  double TRUST_REGION_RADIUS;
  int SOLVE_BQP_ACCURATELY;
  int EXACT_CAUCHY_POINT_REQUIRED;
  int INEXACT_CAUCHY_POINT_REQUIRED;
  int CG_METHOD_USED;
  int DIAGONAL_PRECONDITIONED_CG_SOLVER_USED;
  int MUNKSGAARDS_PRECONDITIONED_CG_SOLVER_USED;
  int EXPANDING_BAND_PRECONDITIONED_CG_SOLVER_USED;
  int FULL_MATRIX_PRECONDITIONED_CG_SOLVER_USED;
  int GILL_MURRAY_PONCELEON_SAUNDERS_PRECONDITIONED_CG_SOLVER_USED;
  int MODIFIED_MA27_PRECONDITIONED_CG_SOLVER_USED;
  int SCHNABEL_ESKOW_PRECONDITIONED_CG_SOLVER_USED;
  int USERS_PRECONDITIONED_CG_SOLVER_USED;
  int BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED;
  int MULTIFRONT_SOLVER_USED;
  int DIRECT_MODIFIED_MULTIFRONTAL_SOLVER_USED;
  int RESTART_FROM_PREVIOUS_POINT;
  int JIFFYTUNE_TOLERANCE_SET;
  double JIFFYTUNE_TOLERANCE;
 };

/* These will be set by the routine which invokes Lancelot. */

NLProblem LANSOLProblem=(NLProblem)NULL;
double *LANSOLz0=(double*)NULL;
int LANSOLZGroup=0;

NLLancelot NLCreateLancelot()
 {
  char RoutineName[]="NLCreateLancelot";

  NLLancelot this=(NLLancelot)malloc(sizeof(struct NLLancelotSolver));
  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLLancelotSolver));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return (NLLancelot)NULL;
   }

  this->MAXIMIZER_SOUGHT=FALSE;
  this->PRINT_LEVEL=0;
  this->START_PRINTING_AT_ITERATION=0;
  this->STOP_PRINTING_AT_ITERATION=100;
  this->ITERATIONS_BETWEEN_PRINTING=1;
  this->MAXIMUM_NUMBER_OF_ITERATIONS=100;
  this->SAVE_DATA_EVERY=0;
  this->CHECK_ALL_DERIVATIVES=FALSE;
  this->CHECK_DERIVATIVES=TRUE;
  this->CHECK_ELEMENT_DERIVATIVES=FALSE;
  this->CHECK_GROUP_DERIVATIVES=FALSE;
  this->IGNORE_DERIVATIVE_WARNINGS=FALSE;
  this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS=FALSE;
  this->IGNORE_GROUP_DERIVATIVE_WARNINGS=FALSE;
  this->FINITE_DIFFERENCE_GRADIENTS=FALSE;
  this->EXACT_SECOND_DERIVATIVES_USED=FALSE;
  this->BFGS_APPROXIMATION_USED=FALSE;
  this->DFP_APPROXIMATION_USED=FALSE;
  this->PSB_APPROXIMATION_USED=FALSE;
  this->SR1_APPROXIMATION_USED=TRUE;
  this->USE_SCALING_FACTORS=FALSE;
  this->USE_CONSTRAINT_SCALING_FACTORS=FALSE;
  this->USE_VARIABLE_SCALING_FACTORS=FALSE;
  this->GET_SCALING_FACTORS=FALSE;
  this->PRINT_SCALING_FACTORS=FALSE;
  this->CONSTRAINT_ACCURACY_REQUIRED=0.00001;
  this->GRADIENT_ACCURACY_REQUIRED=0.00001;
  this->INITIAL_PENALTY_PARAMETER=0.1;
  this->DECREASE_PENALTY_PARAMETER_UNTIL=0.1;
  this->FIRST_CONSTRAINT_ACCURACY_REQUIRED=0.1;
  this->FIRST_GRADIENT_ACCURACY_REQUIRED=0.1;
  this->INFINITY_NORM_TRUST_REGION_USED=TRUE;
  this->TWO_NORM_TRUST_REGION_USED=FALSE;
  this->TRUST_REGION_RADIUS=0.0;
  this->SOLVE_BQP_ACCURATELY=FALSE;
  this->EXACT_CAUCHY_POINT_REQUIRED=TRUE;
  this->INEXACT_CAUCHY_POINT_REQUIRED=FALSE;
  this->CG_METHOD_USED=FALSE;
  this->DIAGONAL_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->MUNKSGAARDS_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->EXPANDING_BAND_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->FULL_MATRIX_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->GILL_MURRAY_PONCELEON_SAUNDERS_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->MODIFIED_MA27_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->SCHNABEL_ESKOW_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->USERS_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED=5;
  this->MULTIFRONT_SOLVER_USED=FALSE;
  this->DIRECT_MODIFIED_MULTIFRONTAL_SOLVER_USED=FALSE;
  this->RESTART_FROM_PREVIOUS_POINT=FALSE;
  this->JIFFYTUNE_TOLERANCE_SET=0;
  this->JIFFYTUNE_TOLERANCE=1.;

  return(this);
 }

void NLFreeLancelot(NLLancelot Lan)
 {
  char RoutineName[]="NLFreeLancelot";

  if(Lan==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return;
   }

  free(Lan);
 }

int LNDumpOUTSDIF_D(NLLancelot this,NLProblem P,double *initialGuess,double *multipliers)
 {
  char RoutineName[]="LNDumpOUTSDIF_D";

  int i,j,k,l,ii;
  FILE *outsdif;
  char buffer[30]="";
  char varfmt[30]="";
  char plusinf[30]="";
  char minusinf[30]="";
  char *c;
  int ngpvlu;
  int nepvlu;
  int neltyp;
  int ngrtyp;
  char *pname;
  int ialgor;
  int minmax;
  NLNonlinearElementPtr ne;

  int n;
  int ng;
  int nelnum;
  int ngel;
  int nvars;
  int nnza;
  int nslacks;
  int *icna;
  double *a;
  int *kndofc;

  int verbose;
  int DOUBLE=FALSE;
  int NFREE;
  int NFIXED;
  int NLOWER;
  int NUPPER;
  int NBOTH;
  int NLINOB;
  int NNLNOB;
  int NLINEQ;
  int NNLNEQ;
  int NLININ;
  int NNLNIN;
  char *string;
  NLVector A;
#ifdef VANILLALANCELOT
  char intfmt[4]="%8d";
  int bigint=9999999;
  int perline=10;
#else
  char intfmt[4]="%6d";
  int bigint=99999;
  int perline=12;
#endif

  verbose=0;

  if(verbose){NLPrintProblemShort(stdout,P);fflush(stdout);}

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
    return 0;
   }

  if(verbose){printf("-----------START-DUMP---------------------------\n");fflush(stdout);}
  if(verbose){printf("In %s\n",RoutineName);fflush(stdout);}

/* n       number of variables */
/* ng      number of groups */
/* nelnum  number of nonlinear elements */
/* ngel    number of elements in all groups */
/* nvars   number of element variables in all nonlinear elements */
/* neltype number of element functions */
/* ngrtype number of group functions */

  outsdif=fopen("OUTSDIF.d","w");

  minmax=0;
  if(NLPGetNumberOfMinMaxConstraints(P)>0)minmax=1;
  n=NLPGetNumberOfVariables(P);
  if(minmax)n++;
  n+=NLPGetNumberOfInequalityConstraints(P);
  n+=NLPGetNumberOfMinMaxConstraints(P);

  ng=NLPGetNumberOfGroups(P);
  nelnum=NLPGetNumberOfNonlinearElements(P);

  ngel=0;
/*nvars=0;               6/27/2000 PM */
  for(i=0;i<ng;i++)
   {
    ngel+=NLPGetNumberOfElementsInGroup(P,i);
/*  for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
      nvars+=NLPGetElementNumberOfUnknowns(P,i,j);    6/27/2000 PM */
   }

/*                6/27/2000 PM */
  nvars=0;
  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
    nvars+=NLNEGetElementDimension(P,i);


/* Compress list of linear parts of the linear elements of the groups.  */

  nnza=0;
  for(i=0;i<ng;i++)
   {
    if(NLPIsGroupASet(P,i))
     {
      nnza+=NLVGetNumberOfNonZeros(NLPGetGroupA(P,i));
     }
   }
  nnza+=NLPGetNumberOfInequalityConstraints(P);
  nnza+=2*NLPGetNumberOfMinMaxConstraints(P);

  ngpvlu=0;
/*??for(i=0;i<NLPGetNumberOfGroups(P);i++) */
/*??  ngpvlu+=LNPGetNumberOfGroupParameters(P,i); */

  nepvlu=0;
/*??for(i=0;i<NLPGetNumberOfGroups(P);i++) */
/*??  for(j=0;j<NLPGetNumberOfElementsInGroup(P);j++) */
/*??    nepvlu+=LNPGetNumberOfElementParameters(P,i,j); */

  neltyp=NLPGetNumberOfElementTypes(P);
  ngrtyp=NLPGetNumberOfGroupTypes(P);

  pname=NLPGetProblemName(P);
  ialgor=1;
  if( NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P)>0)ialgor=2;

  nslacks=NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P);

  strcpy(varfmt,"%16.8E");
  if(DOUBLE)strcpy(varfmt,"%23.15E");
  strcpy(plusinf,"  1.00000000D+20");
  if(DOUBLE)strcpy(plusinf,"  1.000000000000000D+20");
  strcpy(minusinf," -1.00000000D+20");
  if(DOUBLE)strcpy(minusinf," -1.000000000000000D+20");

/* First Line */

  fprintf(outsdif,intfmt,n);
  if(ng>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,ng);
  if(nelnum>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,nelnum);
  if(ngel>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,ngel);
  if(nvars>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,nvars);
  if(nnza>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,nnza);
  if(ngpvlu>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,ngpvlu);
  if(nepvlu>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,nepvlu);
  if(neltyp>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,neltyp);
  if(ngrtyp-1>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,ngrtyp-1);
  fprintf(outsdif,"\n");

  if(verbose)
   {
#ifdef VANILLALANCELOT
    fprintf(stdout,"       n");
    fprintf(stdout,"      ng");
    fprintf(stdout,"  nelnum");
    fprintf(stdout,"    ngel");
    fprintf(stdout,"   nvars");
    fprintf(stdout,"    nnza");
    fprintf(stdout,"  ngpvlu");
    fprintf(stdout,"  nepvlu");
    fprintf(stdout,"  neltyp");
    fprintf(stdout,"  ngrtyp-1\n");
#else
    fprintf(stdout,"     n");
    fprintf(stdout,"    ng");
    fprintf(stdout,"nelnum");
    fprintf(stdout,"  ngel");
    fprintf(stdout," nvars");
    fprintf(stdout,"  nnza");
    fprintf(stdout,"ngpvlu");
    fprintf(stdout,"nepvlu");
    fprintf(stdout,"neltyp");
    fprintf(stdout,"ngrtyp-1\n");
#endif

    fprintf(stdout,intfmt,n);
    fprintf(stdout,intfmt,ng);
    fprintf(stdout,intfmt,nelnum);
    fprintf(stdout,intfmt,ngel);
    fprintf(stdout,intfmt,nvars);
    fprintf(stdout,intfmt,nnza);
    fprintf(stdout,intfmt,ngpvlu);
    fprintf(stdout,intfmt,nepvlu);
    fprintf(stdout,intfmt,neltyp);
    fprintf(stdout,intfmt,ngrtyp-1);
    fprintf(stdout,"\n");
   }

/* Second Line */

  fprintf(outsdif,"%2d",ialgor);
  for(i=0;i<strlen(pname)&&i<8;i++)fprintf(outsdif,"%c",pname[i]);
  for(i=strlen(pname);i<8;i++)fprintf(outsdif," ");
  if(DOUBLE)fprintf(outsdif,"D");
   else fprintf(outsdif," ");
  fprintf(outsdif,"\n");

  if(verbose)
   {
    fprintf(stdout,"%2d",ialgor);
    for(i=0;i<strlen(pname)&&i<8;i++)fprintf(stdout,"%c",pname[i]);
    for(i=strlen(pname);i<8;i++)fprintf(stdout," ");
    fprintf(stdout,"\n");
   }

/* Third Line */

  if(ialgor==2)
   {
    fprintf(outsdif,intfmt,nslacks);
#ifdef VANILLALANCELOT
/* This conforms to the sif decoder distributed with Vanilla Lancelot. File inlanc.f. Says 03/03/00 */
    fprintf(outsdif,intfmt,0);  
#endif
    fprintf(outsdif,"\n");
   }

  if(verbose && ialgor==2)
   {
    fprintf(stdout,intfmt,nslacks);
#ifdef VANILLALANCELOT
    fprintf(stdout,intfmt,0);
#endif
    fprintf(stdout,"\n");
   }

/* Fourth Line - Starting Addresses of the Elements in Each Group ng+1 */
  if(verbose)fprintf(stdout,"ISTADG - Starting Addresses of the Elements in Each Group. perline=%d\n",perline);

  k=0;
  l=0;
  for(i=0;i<ng;i++)
   {
    if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
     else if(k+1>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,k+1);   /* istadg[i] */
    if(verbose)
     {
      if(l%perline==0 && l!=0)fprintf(stdout,"\n");
       else if(k+1>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,k+1);   /* istadg[i] */
     }
    k+=NLPGetNumberOfElementsInGroup(P,i);
    l++;
   }
  if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
   else if(k+1>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,k+1);   /* istadg[ng+1] */
  fprintf(outsdif,"\n");
  if(verbose)
   {
    if(l%perline==0 && l!=0)fprintf(stdout,"\n");
     else if(k+1>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,k+1);   /* istadg[ng+1] */
    fprintf(stdout,"\n");
   }

/* Fifth Line - Starting Addresses of the Parameters Used for Each Group ng+1 */
  if(verbose)fprintf(stdout,"ISTGP - Starting Addresses of the Parameters Used for Each Group.\n");

  k=0;
  for(i=0;i<ng;i++)
   {
    if(i%perline==0 && i!=0)fprintf(outsdif,"\n");
     else if(k+1>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,k+1);  /* istgp[i] */
    if(verbose)
     {
      if(i%perline==0 && i!=0)fprintf(stdout,"\n");
       else if(k+1>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,k+1);  /* istgp[i] */
     }
/*??k+=LNPGetNumberOfGroupParameters(P,i); */
   }
  if(i%perline==0 && i!=0)fprintf(outsdif,"\n");
   else if(k+1>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,k+1);  /* istgp[ng+1] */
  fprintf(outsdif,"\n");
  if(verbose)
   {
    if(i%perline==0 && i!=0)fprintf(stdout,"\n");
     else if(k+1>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,k+1);  /* istgp[ng+1] */
    fprintf(stdout,"\n");
   }

/* Sixth Line - Starting Addresses of the Nonzeros of the Linear Element */
/*              in Each Group ng+1 */
  if(verbose)fprintf(stdout,"ISTADA - Starting Addresses of the Nonzeros of the Linear Element in Each Group\n");

  icna=(int*)malloc(nnza*sizeof(int));
  if(icna==(int*)NULL&&nnza>0)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nnza*sizeof(int));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
    return(0);
   }
  a=(double*)malloc(nnza*sizeof(double));
  if(a==(double*)NULL&&nnza>0)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nnza*sizeof(double));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
    return(0);
   }
  k=0;
  for(i=0;i<ng;i++)
   {
    if(i%perline==0 && i!=0)fprintf(outsdif,"\n");
     else if(k+1>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,k+1); /* istada[i] */
    if(verbose)
     {
      if(i%perline==0 && i!=0)fprintf(stdout,"\n");
       else if(k+1>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,k+1); /* istada[i] */
     }
    if(NLPIsGroupASet(P,i))
     {
      A=NLPGetGroupA(P,i);
      for(j=0;j<NLVGetNumberOfNonZeros(A);j++)
       {
        icna[k]=NLVGetNonZeroCoord(A,j)+1;
        a[k]=NLVGetNonZero(A,j);
        k++;
       }
     }
    j=-1;
    for(l=0;l<NLPGetNumberOfInequalityConstraints(P);l++)
     {
      if(NLPGetInequalityConstraintGroupNumber(P,l)==i)
       {
        j=l;
        break;
       }
     }
    if(j!=-1) /* group i is inequality constraint j */
     {

/* Slack Coefficient */

      icna[k]=NLPGetNumberOfVariables(P)+j+1;
      if(minmax)icna[k]++;
      if(NLPGetInequalityConstraintLowerBound(P,j)<=-1.e20&&NLPGetInequalityConstraintUpperBound(P,j)<1.e20)a[k]=1.;
       else a[k]=-1.;
      k++;
     }
    j=-1;
    for(l=0;l<NLPGetNumberOfMinMaxConstraints(P);l++)
     {
      if(NLPGetMinMaxConstraintGroupNumber(P,l)==i)
       {
        j=l;
        break;
       }
     }
    if(j!=-1) /* group i is minmax constraint j */
     {

/* MinMax Slack Coefficient */

      icna[k]=NLPGetNumberOfVariables(P)+1;   /* z variable */
      a[k]=-1.;
      k++;
      icna[k]=NLPGetNumberOfVariables(P)+1
               +NLPGetNumberOfInequalityConstraints(P)
               +j+1;   /* slack variable */
      a[k]= 1.;
      k++;
     }
   }
  if(i%perline==0 && i!=0)fprintf(outsdif,"\n");
   else if(k+1>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,k+1); /* istada[ng+1] */
  fprintf(outsdif,"\n");
  if(verbose)
   {
    if(i%perline==0 && i!=0)fprintf(stdout,"\n");
     else if(k+1>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,k+1); /* istada[ng+1] */
    fprintf(stdout,"\n");
   }

/* Seventh Line - Starting Addresses of the Variables in Each Element nel+1 */
  if(verbose)fprintf(stdout,"ISTAEV - Starting Addresses of the Variables in Each Element\n");
  k=0;
  l=0;
  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
   {
    ne=NLPGetNonlinearElement(P,i);
    if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
     else if(k+1>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,k+1); /* istaev[l]=k+1; */
    if(verbose)
     {
      if(l%perline==0 && l!=0)fprintf(stdout,"\n");
       else if(k+1>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,k+1); /* istaev[l]=k+1; */
     }
    l++;
    k+=NLNEPGetElementDimension(ne);
   }
  if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
   else if(k+1>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,k+1); /* istaev[nel+1] */
  fprintf(outsdif,"\n");

  if(verbose)
   {
    if(l%perline==0 && l!=0)fprintf(stdout,"\n");
     else if(k+1>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,k+1); /* istaev[nel+1] */
    fprintf(stdout,"\n");
   }

/* Eighth Line - Starting Addresses of the Parameters in Each Element nel+1 */
  if(verbose)fprintf(stdout,"ISTEP - Starting Addresses of the Parameters in Each Element\n");

  k=0;
  l=0;
  for(i=0;i<NLPGetNumberOfNonlinearElements(P);i++)
   {
    ne=NLPGetNonlinearElement(P,i);
    if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
     else if(k+1>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,k+1);  /* istep[l]; */
    if(verbose)
     {
      if(l%perline==0 && l!=0)fprintf(stdout,"\n");
       else if(k+1>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,k+1);  /* istep[l]; */
     }
    l++;
/*??k+=LNPGetNumberOfElementParameters(ne); */
   }
  if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
   else if(k+1>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,k+1);  /* istep[nel+1]; */
  fprintf(outsdif,"\n");
  if(verbose)
   {
    if(l%perline==0 && l!=0)fprintf(stdout,"\n");
     else if(k+1>bigint)fprintf(outsdif," ");
    fprintf(stdout,intfmt,k+1);  /* istep[nel+1]; */
    fprintf(stdout,"\n");
   }

/* Ninth Line - The Group Type of each Group ng */
  if(verbose)fprintf(stdout,"ITYPEG - The Group Type of each Group, ng=%d\n",ng);

  l=0;
  for(i=0;i<ng;i++)
   {
    if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
     else if(NLPGetTypeOfGroup(P,i)>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,NLPGetTypeOfGroup(P,i));
    if(verbose)
     {
      if(l%perline==0 && l!=0)fprintf(stdout,"\n");
       else if(NLPGetTypeOfGroup(P,i)>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,NLPGetTypeOfGroup(P,i));
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

  if(ialgor==2)
   {
    kndofc=(int*)malloc(ng*sizeof(int));
    if(kndofc==(int*)NULL)
     {
      sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",ng*sizeof(int));
      NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
      if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
      return(0);
     }
    for(i=0;i<NLPGetNumberOfGroupsInObjective(P);i++)
     {
      kndofc[NLPGetObjectiveGroupNumber(P,i)]=1;
     }
    for(i=0;i<NLPGetNumberOfEqualityConstraints(P);i++)
     {
      kndofc[NLPGetEqualityConstraintGroupNumber(P,i)]=2;
     }
    for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
     {
      kndofc[NLPGetMinMaxConstraintGroupNumber(P,i)]=9;
     }
    for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
     {
      if(NLPGetInequalityConstraintLowerBound(P,i)<=-1.e20&&NLPGetInequalityConstraintUpperBound(P,i)<1.e20)
        kndofc[NLPGetInequalityConstraintGroupNumber(P,i)]=3;
      else 
        kndofc[NLPGetInequalityConstraintGroupNumber(P,i)]=4;
     }

/* Ninth Line - The Group kind of each Group - kndofc ng */
    if(verbose)fprintf(stdout,"KNDOFC - The kind of each group\n");

    for(i=0;i<ng;i++)
     {
      if(i%perline==0 && i!=0)fprintf(outsdif,"\n");
      fprintf(outsdif,intfmt,kndofc[i]);
      if(verbose)
       {
        if(i%perline==0 && i!=0)fprintf(stdout,"\n");
        fprintf(stdout,intfmt,kndofc[i]);
       }
     }
    fprintf(outsdif,"\n");
    if(verbose)fprintf(stdout,"\n");
    free(kndofc);
   }

/* Tenth Line - The Element Type of each Element - nelnum */
  if(verbose)fprintf(stdout,"ITYPEE - The Element Type of each Element\n");

  l=0;
  for(i=0;i<nelnum;i++)
   {
    if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
     else if(NLEGetType(NLNEGetElementFunction(P,i))+1>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,NLEGetType(NLNEGetElementFunction(P,i))+1);
    if(verbose)
     {
      if(l%perline==0 && l!=0)fprintf(stdout,"\n");
       else if(NLEGetType(NLNEGetElementFunction(P,i))+1>bigint)fprintf(stdout," ");
       fprintf(stdout,intfmt,NLEGetType(NLNEGetElementFunction(P,i))+1);
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Eleventh Line - The Number of Internal Variables for each Element nelnum */
  if(verbose)fprintf(stdout,"INTVAR - The Number of Internal Variables for each Element\n");

  l=0;
  for(i=0;i<nelnum;i++)
   {
    if(l%perline==0 && l!=0)fprintf(outsdif,"\n");
     else if(NLNEPGetInternalDimension(NLPGetNonlinearElement(P,i))>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,NLNEPGetInternalDimension(NLPGetNonlinearElement(P,i)));
    if(verbose)
     {
      if(l%perline==0 && l!=0)fprintf(stdout,"\n");
       else if(NLNEPGetInternalDimension(NLPGetNonlinearElement(P,i))>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,NLNEPGetInternalDimension(NLPGetNonlinearElement(P,i)));
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 12 - The Identity of each Individual Element in each group ngel */
  if(verbose)fprintf(stdout,"IELING - The Identity of each Individual Element in each group.\n");

  k=0;
  for(i=0;i<ng;i++)
   {
    for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
     {
      if(k%perline==0 && k!=0)fprintf(outsdif,"\n");
       else if(NLPGetGroupNonlinearElement(P,i,j)+1>bigint)fprintf(outsdif," ");
      fprintf(outsdif,intfmt,NLPGetGroupNonlinearElement(P,i,j)+1);
      if(verbose)
       {
        if(k%perline==0 && k!=0)fprintf(stdout,"\n");
         else if(NLPGetGroupNonlinearElement(P,i,j)+1>bigint)fprintf(stdout," ");
        fprintf(stdout,intfmt,NLPGetGroupNonlinearElement(P,i,j)+1);
       }
      k++;
     }
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 13 - The Variables in Each Element nvars */
  if(verbose)fprintf(stdout,"IELVAR - The Variables in Each Element.\n");

  k=0;
  for(i=0;i<nelnum;i++)
   {
    ne=NLPGetNonlinearElement(P,i);
    for(l=0;l<NLNEPGetElementDimension(ne);l++)
     {
/*    if(!(NLNEPGetIndex(ne,l)>-1&&NLNEPGetIndex(ne,l)<nvars))
       {
        sprintf(LNLanceErrorMsg,"Element index into whole is illegal, element %d, unknown %d  has index %d. Must be in range 0 to %d.",i,l,NLNEPGetIndex(ne,l),nvars-1);
        NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
        if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
        return 0;
       }
*/
      if(k%perline==0 && k!=0)fprintf(outsdif,"\n");
       else if(NLNEPGetIndex(ne,l)+1>bigint)fprintf(outsdif," ");
      fprintf(outsdif,intfmt,NLNEPGetIndex(ne,l)+1);
      if(verbose)
       {
        if(k%perline==0 && k!=0)fprintf(stdout,"\n");
         else if(NLNEPGetIndex(ne,l)+1>bigint)fprintf(stdout," ");
        fprintf(stdout,intfmt,NLNEPGetIndex(ne,l)+1);
       }
      k++;
     }
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 14 - The Column Addresses of the Nonzeros in Each Linear Element */
  if(verbose)fprintf(stdout,"ICNA - The Column Addresses of the Nonzeros in Each Linear Element.\n");

  for(i=0;i<nnza;i++)
   {
    if(i%perline==0 && i!=0)fprintf(outsdif,"\n");
     else if(icna[i]>bigint)fprintf(outsdif," ");
    fprintf(outsdif,intfmt,icna[i]);
    if(verbose)
     {
      if(i%perline==0 && i!=0)fprintf(stdout,"\n");
       else if(icna[i]>bigint)fprintf(stdout," ");
      fprintf(stdout,intfmt,icna[i]);
     }
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");
  free(icna);

/* Line 15 - The Values of the Nonzeros in Each Linear Element.  nnza */
  if(verbose)fprintf(stdout,"A - The Values of the Nonzeros in Each Linear Element.\n");

  l=0;
  for(i=0;i<nnza;i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,a[i]);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");
  free(a);

/* Line 16 - The Constant Term in Each Group. ng */
  if(verbose)fprintf(stdout,"B - The Constant Term in Each Group.\n");

  l=0;
  for(i=0;i<ng;i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;

    j=-1;
    for(ii=0;ii<NLPGetNumberOfInequalityConstraints(P);ii++)
     {
      if(NLPGetInequalityConstraintGroupNumber(P,ii)==i)
       {
        j=ii;
        break;
       }
     }
    if(j!=-1) /* group i is inequality constraint j */
     {
      if(NLPGetInequalityConstraintLowerBound(P,j)<=-1.e20&&NLPGetInequalityConstraintUpperBound(P,j)<1.e20) /* c(x)<= b */
       {
        if(NLPIsGroupBSet(P,i))
         {
          sprintf(buffer,varfmt,NLPGetGroupB(P,i)
              +NLPGetInequalityConstraintUpperBound(P,j));
          if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
          fprintf(outsdif,"%s",buffer);
          if(verbose)fprintf(stdout,"%s",buffer);
         }else{
          sprintf(buffer,varfmt,
              NLPGetInequalityConstraintUpperBound(P,j));
          if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
          fprintf(outsdif,"%s",buffer);
          if(verbose)fprintf(stdout,"%s",buffer);
         }
       }else if(NLPGetInequalityConstraintLowerBound(P,j)>-1.e20&&NLPGetInequalityConstraintUpperBound(P,j)>=1.e20) /* a<=c(x) */
       {
        if(NLPIsGroupBSet(P,i))
         {
          sprintf(buffer,varfmt,NLPGetGroupB(P,i)
             +NLPGetInequalityConstraintLowerBound(P,j));
          if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
          fprintf(outsdif,"%s",buffer);
          if(verbose)fprintf(stdout,"%s",buffer);
         }else{
          sprintf(buffer,varfmt,
              NLPGetInequalityConstraintLowerBound(P,j));
          if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
          fprintf(outsdif,"%s",buffer);
          if(verbose)fprintf(stdout,"%s",buffer);
         }
       }else{   /* a<=c(x)<=b */
        if(NLPIsGroupBSet(P,i))
         {
          sprintf(buffer,varfmt,NLPGetGroupB(P,i)
             +NLPGetInequalityConstraintLowerBound(P,j));
          if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
          fprintf(outsdif,"%s",buffer);
          if(verbose)fprintf(stdout,"%s",buffer);
         }else{
          sprintf(buffer,varfmt,
             NLPGetInequalityConstraintLowerBound(P,j));
          if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
          fprintf(outsdif,"%s",buffer);
          if(verbose)fprintf(stdout,"%s",buffer);
         }
       }
     }else{ /* group i is not an inequality constraint */
      if(NLPIsGroupBSet(P,i))
       {
        sprintf(buffer,varfmt,NLPGetGroupB(P,i));
        if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
        fprintf(outsdif,"%s",buffer);
        if(verbose)fprintf(stdout,"%s",buffer);
       }else{
        sprintf(buffer,varfmt,0.);
        if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
        fprintf(outsdif,"%s",buffer);
        if(verbose)fprintf(stdout,"%s",buffer);
       }
     }
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 17 - The Lower Bounds on the Variables n */
  if(verbose)fprintf(stdout,"BL - The Lower Bounds on the Variables.\n");

  l=0;
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    if(NLPIsLowerSimpleBoundSet(P,i))
      sprintf(buffer,varfmt,NLPGetLowerSimpleBound(P,i));
     else
      strcpy(buffer,minusinf);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  if(minmax)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    if(NLPGetLowerMinMaxBound(P)>-DBL_MAX)
      sprintf(buffer,varfmt,NLPGetLowerMinMaxBound(P));
     else
      strcpy(buffer,minusinf);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }

/* Lower Bounds on the Slacks */

  for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,0.);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,0.);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 18 - The Upper Bounds on the Variables n */
  if(verbose)fprintf(stdout,"BU - The Upper Bounds on the Variables.\n");

  l=0;
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    if(NLPIsUpperSimpleBoundSet(P,i))
      sprintf(buffer,varfmt,NLPGetUpperSimpleBound(P,i));
     else
      strcpy(buffer,plusinf);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  if(minmax)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    if(NLPGetUpperMinMaxBound(P)>=1.e20)
      strcpy(buffer,plusinf);
     else
      sprintf(buffer,varfmt,NLPGetUpperMinMaxBound(P));
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }

/* Upper Bounds on the Slacks */

  for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    if(NLPGetInequalityConstraintLowerBound(P,i)>-1.e20 && NLPGetInequalityConstraintUpperBound(P,i)<1.e20)
     {
      sprintf(buffer,varfmt,
        NLPGetInequalityConstraintUpperBound(P,i)
       -NLPGetInequalityConstraintLowerBound(P,i));
      if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
      fprintf(outsdif,"%s",buffer);
      if(verbose)fprintf(stdout,"%s",buffer);
     }else{
      fprintf(outsdif,plusinf);
      if(verbose)fprintf(stdout,plusinf);
     }
   }
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    strcpy(buffer,plusinf);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,buffer);
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 19 - The Starting Point for the Minimization n */
  if(verbose)fprintf(stdout,"X - The Starting Point for the Minimization.\n");

  l=0;
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,initialGuess[i]);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  if(minmax)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,LANSOLz0[0]);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }

  for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,0.);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,0.);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }

  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

  if(ialgor==2)
   {
    if(verbose)fprintf(stdout,"U\n");
    if(multipliers!=(double*)NULL)
     {
      l=0;
      for(i=0;i<ng;i++)
       {
        if(l%4==0 && l!=0)fprintf(outsdif,"\n");
        if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
        l++;
        sprintf(buffer,varfmt,multipliers[i]);
        if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
        fprintf(outsdif,"%s",buffer);
        if(verbose){fprintf(stdout,"%s",buffer);fflush(stdout);};
       }
     }else{
      l=0;
      for(i=0;i<ng;i++)
       {
        if(l%4==0 && l!=0)fprintf(outsdif,"\n");
        if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
        l++;
        sprintf(buffer,varfmt,0.);
        if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
        fprintf(outsdif,"%s",buffer);
        if(verbose)fprintf(stdout,"%s",buffer);
       }
     }
    fprintf(outsdif,"\n");
    if(verbose)fprintf(stdout,"\n");
   }

/* Line 20 - The Parameters in Each Group. ngpvlu */
    if(verbose)fprintf(stdout,"GPVALU - The Parameters in Each Group.\n");

/*??  for(i=0;i<ngpvlu;i++) */
/*??   { */
/*??    if(i%4==0 && i!=0)fprintf(outsdif,"\n"); */
/*??    sprintf(buffer,varfmt,gpvalu[i]); */
/*??    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D'; */
/*??    fprintf(outsdif,"%s",buffer); */
/*??   } */
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 21 - The Parameters in Each Individual Element. nepvlu */
    if(verbose)fprintf(stdout,"EPVALU - The Parameters in Each Individual Element.\n");

/*??  for(i=0;i<nepvlu;i++) */
/*??   { */
/*??    if(i%4==0 && i!=0)fprintf(outsdif,"\n"); */
/*??    sprintf(buffer,varfmt,epvalu[i]); */
/*??    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D'; */
/*??    fprintf(outsdif,"%s",buffer); */
/*??   } */
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 22 - The Scale Factors for the Nonlinear Elements. ngel */
    if(verbose)fprintf(stdout,"ESCALE - The Scale Factors for the Nonlinear Elements.\n");

  l=0;
  for(i=0;i<ng;i++)
   {
    for(j=0;j<NLPGetNumberOfElementsInGroup(P,i);j++)
     {
      if(l%4==0 && l!=0)fprintf(outsdif,"\n");
      if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
      if(NLPIsElementWeightSet(P,i,j))
       {
        sprintf(buffer,varfmt,NLPGetElementWeight(P,i,j));
        if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
        fprintf(outsdif,"%s",buffer);
        if(verbose)fprintf(stdout,"%s",buffer);
       }else{
        sprintf(buffer,varfmt,1.);
        if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
        fprintf(outsdif,"%s",buffer);
        if(verbose)fprintf(stdout,"%s",buffer);
       }
      l++;
     }
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 23 - The Scale Factors for the Groups. ng */
  if(verbose)fprintf(stdout,"GSCALE - The Scale Factors for the Groups.\n");

  for(i=0;i<ng;i++)
   {
    if(i%4==0 && i!=0)fprintf(outsdif,"\n");
    if(verbose)if(i%4==0 && i!=0)fprintf(stdout,"\n");
    sprintf(buffer,varfmt,1./NLPGetGroupScale(P,i));
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 24 - The Scale Factors for the Variables. n */
  if(verbose)fprintf(stdout,"VSCALE - The Scale Factors for the Variables. n=%d\n",n);

  l=0;
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,NLPGetVariableScale(P,i));
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  if(minmax)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,1.);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
/*  if(NLPGetInequalityConstraintLowerBound(P,i)==0. || NLPGetInequalityConstraintLowerBound(P,i)==DBL_MIN)
     {  6/27/2000 */
      sprintf(buffer,varfmt,1.);
      if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
      fprintf(outsdif,"%s",buffer);
      if(verbose)fprintf(stdout,"%s",buffer);
/*   }else{
      sprintf(buffer,varfmt,-1.);
      if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
      fprintf(outsdif,"%s",buffer);
      if(verbose)fprintf(stdout,"%s",buffer);
     } 6/27/2000 */
   }
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
   {
    if(l%4==0 && l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%4==0 && l!=0)fprintf(stdout,"\n");
    l++;
    sprintf(buffer,varfmt,1.);
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 25 - The Lower and Upper Bounds on the Objective Function. */
    if(verbose)fprintf(stdout,"OBFBND - The Lower and Upper Bounds on the Objective Function.\n");

  if(NLPGetObjectiveLowerBound(P)!=DBL_MIN)
   {
    sprintf(buffer,varfmt,NLPGetObjectiveLowerBound(P));
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }else{
    fprintf(outsdif,minusinf);
    if(verbose)fprintf(stdout,minusinf);
   }
  if(NLPGetObjectiveUpperBound(P)!=DBL_MAX)
   {
    sprintf(buffer,varfmt,NLPGetObjectiveUpperBound(P));
    if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
    fprintf(outsdif,"%s",buffer);
    if(verbose)fprintf(stdout,"%s",buffer);
   }else{
    fprintf(outsdif,plusinf);
    if(verbose)fprintf(stdout,plusinf);
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 26 - An Array Stating If There Are Internal Variables. nelnum */
      if(verbose)if(l%72==0&&l!=0)fprintf(stdout,"\n");
    if(verbose)fprintf(stdout,"INTREP - An Array Stating If There Are Internal Variables. nelnum=%d\n",nelnum);

  l=0;
/*for(i=0;i<neltyp;i++)   6/27/2000 */
  for(i=0;i<nelnum;i++)
   {
    if(l%72==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%72==0&&l!=0)fprintf(stdout,"\n");
/*  if(NLPIsElementRangeSet(P,i))  6/27/2000 */
    if(NLEGetRangeXForm(NLNEGetElementFunction(P,i))!=(NLMatrix)NULL)
     {
      fprintf(outsdif,"%c",'T');
      if(verbose)fprintf(stdout,"%c",'T');
     }else{
      fprintf(outsdif,"%c",'F');
      if(verbose)fprintf(stdout,"%c",'F');
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 27 - An Array Stating If Group Function is Trivial  ng */
  if(verbose)fprintf(stdout,"GXEQX - An Array Stating If Group Function is Trivial.\n");

  l=0;
  for(i=0;i<ng;i++)
   {
    if(l%72==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%72==0&&l!=0)fprintf(stdout,"\n");
    if(NLPIsGroupFunctionSet(P,i))
     {
      fprintf(outsdif,"%c",'F');
      if(verbose)fprintf(stdout,"%c",'F');
     }else{
      fprintf(outsdif,"%c",'T');
      if(verbose)fprintf(stdout,"%c",'T');
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 28 - Names Given to the Groups. ng  */
  if(verbose)fprintf(stdout,"GNAMES - Names Given to the Groups.\n");

  l=0;
  for(i=0;i<ng;i++)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    string=NLPGetGroupName(P,i);
    for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
    for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
    if(verbose)
     {
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(stdout," ");
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 29 - Names Given to the Variables. n  */
  if(verbose)fprintf(stdout,"VNAMES - Names Given to the Variables.\n");

  l=0;
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    string=NLPGetVariableName(P,i);
    if(string==(char*)NULL)
     {
      string=(char*)malloc(9*sizeof(char));
      if(string==(char*)NULL)
       {
        sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",9*sizeof(char));
        NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
        if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
        return(0);
       }
      sprintf(string,"X%d",i+1);
      if((c=strstr(buffer,"E"))!=(char*)NULL)c[0]='D';
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
      if(verbose)
       {
        for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
        for(j=strlen(string);j<10;j++)fprintf(stdout," ");
       }
      free(string);
     }else{
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
      if(verbose)
       {
        for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
        for(j=strlen(string);j<10;j++)fprintf(stdout," ");
       }
     }
    l++;
   }
  if(minmax)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    fprintf(outsdif,"Z         ");
    if(verbose)fprintf(stdout,"Z         ");
    l++;
   }
  for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    string=NLPGetGroupName(P,NLPGetInequalityConstraintGroupNumber(P,i));
    if(string==(char*)NULL)
     {
      string=(char*)malloc(9*sizeof(char));
      if(string==(char*)NULL)
       {
        sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",9*sizeof(char));
        NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
        if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
        return(0);
       }
      sprintf(string,"C%d",i);
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
      if(verbose)
       {
        for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
        for(j=strlen(string);j<10;j++)fprintf(stdout," ");
       }
      free(string);
     }else{
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
      if(verbose)
       {
        for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
        for(j=strlen(string);j<10;j++)fprintf(stdout," ");
       }
     }
    l++;
   }
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    string=NLPGetGroupName(P,NLPGetMinMaxConstraintGroupNumber(P,i));
    if(string==(char*)NULL)
     {
      string=(char*)malloc(9*sizeof(char));
      if(string==(char*)NULL)
       {
        sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",9*sizeof(char));
        NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
        if(verbose){printf("\n %s -- %s\n",RoutineName,LNLanceErrorMsg);fflush(stdout);}
        return(0);
       }
      sprintf(string,"J%d",i);
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
      if(verbose)
       {
        for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
        for(j=strlen(string);j<10;j++)fprintf(stdout," ");
       }
      free(string);
     }else{
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
      if(verbose)
       {
        for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
        for(j=strlen(string);j<10;j++)fprintf(stdout," ");
       }
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 30 - Names Given to the Nonlinear Element Types. neltype  */
  if(verbose)fprintf(stdout,"ETYPES - Names Given to the Nonlinear Elements.\n");

  l=0;
/*for(i=0;i<neltyp;i++) 6/27/2000 */
  for(i=0;i<NLPGetNumberOfElementTypes(P);i++)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    string=NLPGetElementType(P,i);
    for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
    for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
    if(verbose)
     {
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(stdout," ");
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Line 31 - Names Given to the Group Types. ngrtype  */
  if(verbose)fprintf(stdout,"GTYPES - Names Given to the Group Types. ngrtyp=%d\n",ngrtyp);

  l=0;
  for(i=1;i<ngrtyp;i++)
   {
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    if(verbose)if(l%8==0&&l!=0)fprintf(stdout,"\n");
    string=NLPGetGroupType(P,i);
    for(j=0;j<strlen(string)&&j<10;j++)fprintf(outsdif,"%c",string[j]);
    for(j=strlen(string);j<10;j++)fprintf(outsdif," ");
    if(verbose)
     {
      for(j=0;j<strlen(string)&&j<10;j++)fprintf(stdout,"%c",string[j]);
      for(j=strlen(string);j<10;j++)fprintf(stdout," ");
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");

/* Extra line (added 2/12/99???) */

  l=0;
  for(i=0;i<n;i++)
   {
    fprintf(outsdif,intfmt,0);
    if(l%8==0&&l!=0)fprintf(outsdif,"\n");
    l++;
   }
  fprintf(outsdif,"\n");

/* Last line - SOLUTION SUMMARY    */

  NFREE=0;
  NFIXED=0;
  NLOWER=0;
  NUPPER=0;
  NBOTH=0;
  for(i=0;i<NLPGetNumberOfVariables(P);i++)
   {
    if(NLPGetLowerSimpleBound(P,i)>-1.e20)
     {
      if(!NLPIsUpperSimpleBoundSet(P,i))NLOWER++;
       else{
        if(NLPGetLowerSimpleBound(P,i)==NLPGetUpperSimpleBound(P,i))NFIXED++;
         else NBOTH++;
       }
     }else{
      if(NLPIsUpperSimpleBoundSet(P,i))NUPPER++;
       else NFREE++;
     }
   }
  if(minmax)
   {
    if(NLPGetLowerMinMaxBound(P)>-1.e20)
     {
      if(NLPGetUpperMinMaxBound(P)>1.e20)NLOWER++;
       else{
        if(NLPGetLowerMinMaxBound(P)==NLPGetUpperMinMaxBound(P))NFIXED++;
         else NBOTH++;
       }
     }else{
      if(NLPGetUpperMinMaxBound(P)<=1.e20)NUPPER++;
       else NFREE++;
     }
   }

  NLINOB=0;
  NNLNOB=0;
  for(i=0;i<NLPGetNumberOfGroupsInObjective(P);i++)
   {
    if(NLPIsGroupFunctionSet(P,NLPGetObjectiveGroupNumber(P,i))||NLPGetNumberOfElementsInGroup(P,NLPGetObjectiveGroupNumber(P,i))>0)
      NNLNOB++;
     else
      NLINOB++;
   }

  NLINEQ=0;
  NNLNEQ=0;
  for(i=0;i<NLPGetNumberOfEqualityConstraints(P);i++)
   {
    if(NLPIsGroupFunctionSet(P,NLPGetEqualityConstraintGroupNumber(P,i))||NLPGetNumberOfElementsInGroup(P,NLPGetEqualityConstraintGroupNumber(P,i))>0)
      NNLNEQ++;
     else
      NLINEQ++;
   }

  NLININ=0;
  NNLNIN=0;
  for(i=0;i<NLPGetNumberOfInequalityConstraints(P);i++)
   {
    if(NLPIsGroupFunctionSet(P,NLPGetInequalityConstraintGroupNumber(P,i))||NLPGetNumberOfElementsInGroup(P,NLPGetInequalityConstraintGroupNumber(P,i))>0)
      NNLNIN++;
     else
      NLININ++;
   }
  for(i=0;i<NLPGetNumberOfMinMaxConstraints(P);i++)
   {
    if(NLPIsGroupFunctionSet(P,NLPGetMinMaxConstraintGroupNumber(P,i))||NLPGetNumberOfElementsInGroup(P,NLPGetMinMaxConstraintGroupNumber(P,i))>0)
      NNLNIN++;
     else
      NLININ++;
   }

#ifdef VANILLALANCELOT
  l=0;
  for(i=0;i<n;i++)
   {
    if(l%perline==0&&l!=0)fprintf(outsdif,"\n");
    fprintf(outsdif,intfmt,0);
    if(verbose)
     {
      if(l%perline==0&&l!=0)fprintf(stdout,"\n");
      fprintf(stdout,intfmt,0);
     }
    l++;
   }
  fprintf(outsdif,"\n");
  if(verbose)fprintf(stdout,"\n");
#endif

  for(i=0;i<strlen(pname)&&i<8;i++)fprintf(outsdif,"%c",pname[i]);
  for(i=strlen(pname);i<8;i++)fprintf(outsdif," ");
  if(NFREE>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NFREE);
  if(NFIXED>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NFIXED);
  if(NLOWER>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NLOWER);
  if(NUPPER>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NUPPER);
  if(NBOTH>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NBOTH );
  if(nslacks>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,nslacks);
  if(NLINOB>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NLINOB);
  if(NNLNOB>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NNLNOB);
  if(NLINEQ>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NLINEQ);
  if(NNLNEQ>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NNLNEQ);
  if(NLININ>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NLININ);
  if(NNLNIN>bigint)fprintf(outsdif," ");
  fprintf(outsdif,intfmt,NNLNIN);
  fprintf(outsdif,"\n");

  if(verbose)
   {
    fprintf(stdout,"%s","   PNAME");
#ifdef VANILLALANCELOT
    fprintf(stdout,"%s","   NFREE");
    fprintf(stdout,"%s","  NFIXED");
    fprintf(stdout,"%s","  NLOWER");
    fprintf(stdout,"%s","  NUPPER");
    fprintf(stdout,"%s","   NBOTH");
    fprintf(stdout,"%s","  NSLACK");
    fprintf(stdout,"%s","  NLINOB");
    fprintf(stdout,"%s","  NNLNOB");
    fprintf(stdout,"%s","  NLINEQ");
    fprintf(stdout,"%s","  NNLNEQ");
    fprintf(stdout,"%s","  NLININ");
    fprintf(stdout,"%s","  NNLNIN");
#else
    fprintf(stdout,"%s"," NFREE");
    fprintf(stdout,"%s","NFIXED");
    fprintf(stdout,"%s","NLOWER");
    fprintf(stdout,"%s","NUPPER");
    fprintf(stdout,"%s"," NBOTH");
    fprintf(stdout,"%s","NSLACK");
    fprintf(stdout,"%s","NLINOB");
    fprintf(stdout,"%s","NNLNOB");
    fprintf(stdout,"%s","NLINEQ");
    fprintf(stdout,"%s","NNLNEQ");
    fprintf(stdout,"%s","NLININ");
    fprintf(stdout,"%s","NNLNIN");
#endif
    fprintf(stdout,"\n");
    for(i=0;i<strlen(pname)&&i<8;i++)fprintf(stdout,"%c",pname[i]);
    for(i=strlen(pname);i<8;i++)fprintf(stdout," ");
    if(NFREE>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NFREE);
    if(NFIXED>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NFIXED);
    if(NLOWER>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NLOWER);
    if(NUPPER>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NUPPER);
    if(NBOTH>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NBOTH );
    if(nslacks>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,nslacks);
    if(NLINOB>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NLINOB);
    if(NNLNOB>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NNLNOB);
    if(NLINEQ>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NLINEQ);
    if(NNLNEQ>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NNLNEQ);
    if(NLININ>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NLININ);
    if(NNLNIN>bigint)fprintf(stdout," ");
    fprintf(stdout,intfmt,NNLNIN);
    fprintf(stdout,"\n");
   }

  fclose(outsdif);
  if(verbose){printf("------------END--DUMP---------------------------\n");fflush(stdout);}
  return 1;
 }

/* These will be called by Lancelot.   */

void F77_FUNC(elfuns,ELFUNS)(F77DOUBLEPRECISION *result,F77DOUBLEPRECISION *variables,F77DOUBLEPRECISION *epvalu,F77INTEGER *nElements,F77INTEGER *itypee,F77INTEGER *istaev,F77INTEGER *ielvar,F77INTEGER *intvar,F77INTEGER *istadh,F77INTEGER *istepa,F77INTEGER *elementList,F77INTEGER *derivs)
 {
  char RoutineName[]="elfuns";

  int element;
  double *x,*l;
  double *f,*dfdx,*d2fdx2;
  int i,j,k,m;
  int jk;
  NLElementFunction elementFunction;
  void *fdata;
  int nx,nl;
  double *X,*L,*F;
  int verbose;
  int trace;
  static int mx=0;
  static double *pX=(double*)NULL;
  static int mu;
  static double *pU=(double*)NULL;
  int nu;
  int false=0;

  verbose=0;
  trace=0;

  if(verbose){printf("in ELFUNS nelements=%d deriv = %d\n",*nElements,*derivs);fflush(stdout);}

  for(i=0;i<*nElements;i++)
   {
    element=elementList[i]-1;

/* element is a nonlinear element */

/*  printf("%d ielemn %d ieltyp %d\n",i+1,element+1,itypee[element]);fflush(stdout);*/
    if(verbose){printf(" ELFUNS: Nonlinear Element[%d] %d, element function=%d(%s)\n",i,element+1,itypee[element]-1,NLPGetElementType(LANSOLProblem,itypee[element]-1));fflush(stdout);}
    x=variables+ielvar[istaev[element]-1]-1;
    f=result+elementList[i]- 1;
    dfdx=result+intvar[element]-1;
    d2fdx2=result+istadh[element]-1;
    l=variables+istepa[element]-1;

    if(verbose)
     {
      printf("           Element Type   = %s\n",NLPGetElementType(LANSOLProblem,itypee[element]-1));fflush(stdout);
      printf("           F   = FUVALS[%d]\n",elementList[i]);fflush(stdout);
      printf("           F'  = FUVALS[%d]\n",intvar[element]);fflush(stdout);
      if(*derivs==3){printf("           F'' = FUVALS[%d]\n",istadh[element]);fflush(stdout);}
      printf("           X   = XVALUE[%d]\n",ielvar[istaev[element]-1]);fflush(stdout);
      printf("           L   = XVALUE[%d]\n",istepa[element]);fflush(stdout);
     }

    elementFunction=NLNEGetElementFunction(LANSOLProblem,element);
    fdata=NLPGetNonlinearElementData(LANSOLProblem,element);
    if(NLEGetRangeXForm(elementFunction)==(NLMatrix)NULL)
     {
      nx=NLEGetDimension(elementFunction);
      nl=0;
      if(verbose)
       {
        printf("          nx   =        %d \n",nx);fflush(stdout);
        printf("          nl   =        %d \n",nl);fflush(stdout);
       }

      if(nx>mx)
       {
        pX=(double*)realloc((void*)pX,nx*sizeof(double));
        if(pX==(double*)NULL)
         {
          sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nx*sizeof(char));
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          return;
         }
       }
      for(j=0;j<nx;j++)pX[j]=variables[ielvar[istaev[element]-1+j]-1];
/*    printf(" Evaluating Element Type %2.2d %s (",itypee[element],NLPGetElementType(LANSOLProblem,itypee[element]-1));fflush(stdout);
      for(j=0;j<nx;j++){if(j>0)printf(",");printf("%10.7lf",pX[j]);}
      printf(")\n");fflush(stdout);*/
     }else{
      nu=NLMGetNumberOfCols(NLEGetRangeXForm(elementFunction));
      nx=NLMGetNumberOfRows(NLEGetRangeXForm(elementFunction));
      nl=0;
      if(verbose)
       {
        printf("          nu   =        %d \n",nu);fflush(stdout);
        printf("          nv   =        %d \n",nx);fflush(stdout);
        printf("          nl   =        %d \n",nl);fflush(stdout);
       }

      if(nu>mu)
       {
        pU=(double*)realloc((void*)pU,nu*sizeof(double));
        if(pU==(double*)NULL)
         {
          sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nu*sizeof(char));
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          return;
         }
       }
      for(j=0;j<nu;j++)pU[j]=variables[ielvar[istaev[element]-1+j]-1];
      if(nx>mx)
       {
        pX=(double*)realloc((void*)pX,nx*sizeof(double));
        if(pX==(double*)NULL)
         {
          sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nx*sizeof(char));
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          return;
         }
       }
      NLMVMult(NLEGetRangeXForm(elementFunction),pU,pX);
/*    printf(" Evaluating Element Type %2.2d %s (",itypee[element],NLPGetElementType(LANSOLProblem,itypee[element]-1));fflush(stdout);
      for(j=0;j<nu;j++){if(j>0)printf(",");printf("%10.7lf",pU[j]);}
      printf(")->(");fflush(stdout);
      for(j=0;j<nx;j++){if(j>0)printf(",");printf("%10.7lf",pX[j]);}
      printf(")\n");fflush(stdout);*/
     }

    if(*derivs==1)
     {
      f[0]=NLEEval(elementFunction,nx,pX,fdata);
      if(verbose||trace)
       {
        printf("  elF[%d](",element+1);
        for(j=0;j<nx;j++)
         {
          if(j>0)printf(",");
          printf("%21.14lf",pX[j]);
         }
        printf(";");
        for(j=0;j<nl;j++)
         {
          if(j>0)printf(",");
          printf("%21.14lf",l[j]);
         }
        printf(")=%21.14lf\n",f[0]);fflush(stdout);
       }
/*    printf("                  value  %10.7lf\n",f[0]);fflush(stdout);*/
     }else{
      for(j=0;j<nx;j++)
       {
        dfdx[j]=NLEEvalDer(elementFunction,j,nx,pX,fdata);
        if(!(dfdx[j]==dfdx[j]))
         {
          sprintf(LNLanceErrorMsg,"Nan returned from derivative of element function %d",j);
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          fprintf(stderr,"%s\n",LNLanceErrorMsg);fflush(stderr);
          abort();
         }
        if(verbose)
         {
          printf("  d elF[%d]/dx_%d(",element+1,j+1);
          for(m=0;m<nx;m++)
           {
            if(m>0)printf(",");
            printf("%21.14lf",pX[m]);
           }
          printf(";");
          for(m=0;m<nl;m++)
           {
            if(m>0)printf(",");
            printf("%21.14lf",l[m]);
           }
          printf(")=%21.14lf\n",dfdx[j]);fflush(stdout);
         }
       }
      if(*derivs==3)
       {
        jk=0;
        for(k=0;k<nx;k++)
         {
          for(j=0;j<=k;j++)
           {
            d2fdx2[jk]=NLEEvalSecDer(elementFunction,j,k,nx,pX,fdata);
            if(!(d2fdx2[jk]==d2fdx2[jk]))
             {
              sprintf(LNLanceErrorMsg,"Nan returned from second derivative of element function %d, %d",i,k);
              NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
              fprintf(stderr,"%s\n",LNLanceErrorMsg);fflush(stderr);
              abort();
             }
            if(verbose)
             {
              printf("  d2 elF[%d]/dx_%d/dx_%d(",element+1,j+1,k+1);
              for(m=0;m<nx;m++)
               {
                if(m>0)printf(",");
                printf("%21.14lf",pX[m]);
               }
              printf(";");
              for(m=0;m<nl;m++)
               {
                if(m>0)printf(",");
                printf("%21.14lf",l[m]);
               }
              printf(")=%21.14lf\n",d2fdx2[jk]);fflush(stdout);
             }
            jk++;
           }
         }
       }
/*    printf("                  value  %10.7lf, (",f[0]);fflush(stdout);
      for(j=0;j<nx;j++)
       {
        if(j>0)printf(",");
        printf("%10.7lf",dfdx[j]);
       } 
      printf(")\n");fflush(stdout);*/
     }
   }
  if(verbose){printf("done ELFUNS:\n");fflush(stdout);}

  return;
 }

void F77_FUNC(groups,GROUPS)(F77DOUBLEPRECISION *result,F77INTEGER *lgvalu,F77DOUBLEPRECISION *fvalue,F77DOUBLEPRECISION *gpvalu,F77INTEGER *nGroups,F77INTEGER *itypeg,F77INTEGER *istgpa,F77INTEGER *groupList,F77LOGICAL* derivs)
 {
  char RoutineName[]="groups";

  int group;
  double *x=(double*)NULL;
  double *l=(double*)NULL;
  double *g=(double*)NULL;
  double *dgdx=(double*)NULL;
  double *d2gdx2=(double*)NULL;
  void *gdata;
  int i,j;
  NLGroupFunction groupFunction;
  int nl=0;
  double *X=(double*)NULL;
  double *L=(double*)NULL;
  double *G=(double*)NULL;
  int verbose;
  int trace;

  verbose=0;
  trace=0;

  if(verbose){printf("in GROUPS: deriv=%d\n",*derivs);fflush(stdout);}
  if(verbose){printf("grouplist is:[");for(i=0;i<*nGroups;i++){if(i>0)printf(",");printf("%d",groupList[i]);}printf("]\n");fflush(stdout);}

  g=result;
  dgdx=result+(*lgvalu);
  d2gdx2=result+2*(*lgvalu);
  x=fvalue;
  for(i=0;i<*nGroups;i++)
   {
    group=groupList[i]-1;
    if(verbose){printf("  Group  %d\n",group+1);fflush(stdout);}
    if(itypeg[group]>0)
     {
      if(!(LANSOLProblem!=NULL))
       {
        sprintf(LNLanceErrorMsg,"Problem (LANSOLProblem) is NULL");
        NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
        fprintf(stderr,"%s\n",LNLanceErrorMsg);fflush(stderr);
        return;
       }
      groupFunction=NLPGetGroupFunction(LANSOLProblem,group);
      gdata=NLPGetGroupFunctionData(LANSOLProblem,group);
     
      if(groupFunction==(NLGroupFunction)NULL)
       {
        if(verbose)printf(" g[%d] is the trivial group\n",group+1);fflush(stdout);
        if(itypeg[group]!=0)fprintf(stderr,"Lancelot API -- Warning. Trivial group %d has type %d. Line %d of file %s\n",group,itypeg[group],__LINE__,__FILE__);
       }

      if(verbose)
       {
        if(!*derivs)
         {
          printf("           G   = GVALUE[%d,%d]=%d\n",group+1,1,group+1);fflush(stdout);
         }else{
          printf("           G'  = GVALUE[%d,%d]=%d\n",group+1,2,*lgvalu+group+1);fflush(stdout);
          printf("           G'' = GVALUE[%d,%d]=%d\n",group+1,3,2*(*lgvalu)+group+1);fflush(stdout);
         }
        printf("           X   = FVALUE[%d]\n",group+1);fflush(stdout);
        printf("           L   = FVALUE[%d]\n",istgpa[group]);fflush(stdout);
       }
  
      if(!*derivs)
       {
        if(groupFunction!=(NLGroupFunction)NULL)
         {
          g[group]=NLGEval(groupFunction,x[group],gdata);
         }else{ /* Trivial group */
          g[group]=x[group];
         }
        if(!(g[group]==g[group]))
         {
          sprintf(LNLanceErrorMsg,"Nan returned from group function");
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          fprintf(stderr,"%s\n",LNLanceErrorMsg);fflush(stderr);
          abort();
         }
        if(verbose || trace)
         {
          printf("  grF[%d](%21.14lf;",group+1,x[group]);
          for(j=0;j<nl;j++)
           {
            if(j>0)printf(",");
            printf("%21.14lf",l[j]);
           }
          if(groupFunction!=(NLGroupFunction)NULL)
            {printf(")=%21.14lf\n",g[group]);fflush(stdout);}
           else
            {printf(")=%21.14lf  (trivial group)\n",g[group]);fflush(stdout);}
         }
       }else{
        if(groupFunction!=(NLGroupFunction)NULL)
         {
          dgdx[group]=NLGEvalDer(groupFunction,x[group],gdata);
         }else{ /* Trivial group */
          dgdx[group]=1.;
         }
        if(!(dgdx[group]==dgdx[group]))
         {
          sprintf(LNLanceErrorMsg,"Nan returned from derivative of group function");
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          fprintf(stderr,"%s\n",LNLanceErrorMsg);fflush(stderr);
          abort();
         }
        if(verbose)
         {
          printf("  grF'[%d](%21.14lf;",group+1,x[group]);
          for(j=0;j<nl;j++)
           {
            if(j>0)printf(",");
            printf("%21.14lf",l[j]);
           }
          printf(")=%21.14lf\n",dgdx[group]);fflush(stdout);
         }

        if(groupFunction!=(NLGroupFunction)NULL)
         {
          d2gdx2[group]=NLGEvalSecDer(groupFunction,x[group],gdata);
         }else{ /* Trivial group */
          d2gdx2[group]=0.;
         }
        if(!(d2gdx2[group]==d2gdx2[group]))
         {
          sprintf(LNLanceErrorMsg,"Nan returned from second derivative of group function");
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          fprintf(stderr,"%s\n",LNLanceErrorMsg);fflush(stderr);
          abort();
         }
        if(verbose)
         {
          printf("  grF''[%d](%21.14lf;",group+1,x[group]);
          for(j=0;j<nl;j++)
           {
            if(j>0)printf(",");
            printf("%21.14lf",l[j]);
           }
          printf(")=%21.14lf\n",d2gdx2[group]);fflush(stdout);
         }
       }
     }
   }
  if(verbose){printf("done GROUPS:\n");fflush(stdout);}
  return;
 }

void F77_FUNC(ranges,RANGES)(F77INTEGER *ielemn,F77LOGICAL *transp,F77DOUBLEPRECISION *w1,F77DOUBLEPRECISION *w2,F77INTEGER *nelvar,F77INTEGER *ninvar)
 {
  char RoutineName[]="ranges";

  int i,j,group,ng;
  int n,m;
  int verbose;
  NLMatrix xfrm;

  verbose=0;

/* ielemn in [0,nelnum-1] */
/* ielemn is a nonlinear element */

  if(verbose){printf("in RANGES:\n");fflush(stdout);}

  xfrm=NNLEGetRangeXForm(LANSOLProblem,*ielemn-1);

#ifdef UNKNOWN
/* ITYPE = ITYPEE( IELEMN )*/

  j=*ielemn-1;
  ng=NLPGetNumberOfGroups(LANSOLProblem);
  for(group=0;group<ng;group++)
   {
    if(j<NLPGetNumberOfElementsInGroup(LANSOLProblem,group))
     {
      i=group;
      ng=-1;
     }else{
      j-=NLPGetNumberOfElementsInGroup(LANSOLProblem,group);
     }
   }

  xfrm=NLEGetRangeXForm(NLPGetElementFunctionOfGroup(LANSOLProblem,i,j));
#endif

  if(xfrm==(NLMatrix)NULL)
   {
    for(i=0;i<*nelvar;i++)w2[i]=w1[i];
   }else{
    n=NLMGetNumberOfRows(xfrm);
    m=NLMGetNumberOfCols(xfrm);

/* w2 < w1 */

    if(*transp==0)
     {
      if(verbose){printf("ranges: !T n=%d, ninvar=%d   m=%d, nelvar=%d element %d\n",n,*ninvar,m,*nelvar,*ielemn-1);fflush(stdout);}
      NLMVMult(xfrm,w1,w2);
#ifdef UNKNOWN
      for(i=0;i<n;i++)
       {
        w2[i]=0.;
        for(j=0;j<m;j++)
         {
          w2[i]+=NLMGetElement(xfrm,i,j)*w1[j];
         }
       }
#endif
      if(verbose)
       {
        for(i=0;i<*ninvar;i++)
         {
          printf("[ %10.7lf ] = [",w2[i]);
          for(j=0;j<m;j++)
           {
            if(j>0)printf(" ");
            printf("%10.7lf",NLMGetElement(xfrm,i,j));
           } 
          printf(" ][ %10.7lf ]\n",w1[i]);
         }
        for(i=*ninvar;i<*nelvar;i++)
         {
          printf("                  ");
          for(j=0;j<m;j++)
           {
            if(j>0)printf(" ");
            printf("          ");
           } 
          printf("  [ %10.7lf ]\n",w1[i]);
         }
        fflush(stdout);
       }
     }else{
      if(verbose){printf("ranges: T n=%d, ninvar=%d   m=%d, nelvar=%d element %d\n",n,*ninvar,m,*nelvar,*ielemn-1);fflush(stdout);}

      NLMVMultT(xfrm,w1,w2);
#ifdef UNKNOWN
/* w1 < w2 */

      for(j=0;j<m;j++)
       {
        w2[j]=0.;
        for(i=0;i<n;i++)
         {
          w2[j]+=NLMGetElement(xfrm,i,j)*w1[i];
         }
       }
#endif
      if(verbose)
       {
        for(i=0;i<*nelvar;i++)
         {
          printf("[ %10.7lf ] = [",w2[i]);
          for(j=0;j<*ninvar;j++)
           {
            if(j>0)printf(" ");
            printf("%10.7lf",NLMGetElement(xfrm,j,i));
           } 
          printf(" ]");
          if(i<*ninvar)printf("[ %10.7lf ]",w1[i]);
          printf("\n");
         }
        fflush(stdout);
       }
     }
   }

  if(verbose){printf("done RANGES:\n");fflush(stdout);}

  return;
 }

void F77_FUNC(gminma,GMINMA)(F77INTEGER *nz,F77DOUBLEPRECISION *z0)
 {
  char RoutineName[]="gminma";

  int i;
  int verbose;

  verbose=0;
  if(verbose){printf("in GMINMA %d:\n",NLPGetNumberOfMinMaxConstraints(LANSOLProblem));fflush(stdout);}
  if(NLPGetNumberOfMinMaxConstraints(LANSOLProblem)>0)
   {
    if(*nz<0)
     {
      *nz=LANSOLZGroup+1;
      for(i=0;i<1;i++)
       {
        z0[i]=LANSOLz0[i];
        if(verbose){printf("  z0[%d]=%lf\n",i,z0[i]);fflush(stdout);}
       }
     }else{
      *nz=-1;          /* (to tell Lancelot that I have the group number for the objective) */
     }
    if(verbose){printf("done GMINMA:\n");fflush(stdout);}
   }else
    *nz=0;

  return;
 }

void F77_FUNC(settyp,SETTYP)(F77INTEGER *nelnum, F77INTEGER *itype)
 {
/* This is used in Lancelot to copy itypee to a common block so that RANGES.f can use it */
/*printf("in SETTYP:\n");fflush(stdout);*/
  return;
 }

void LNDumpSpecFile(NLLancelot this)
 {
  char RoutineName[]="LNDumpSpecFile";

  FILE *specfile;
  int verbose;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return;
   }

  verbose=0;

  specfile=fopen("SPEC.SPC","w");

  fprintf(specfile,"BEGIN\n");

  if(this->MAXIMIZER_SOUGHT)fprintf(specfile," MAXIMIZER-SOUGHT\n");

  fprintf(specfile," PRINT-LEVEL %d\n",this->PRINT_LEVEL);
  if(this->PRINT_LEVEL>0)
   {
    fprintf(specfile," START-PRINTING-AT-ITERATION %d\n",this->START_PRINTING_AT_ITERATION);
    fprintf(specfile," STOP-PRINTING-AT-ITERATION %d\n",this->STOP_PRINTING_AT_ITERATION);
    fprintf(specfile," ITERATIONS-BETWEEN-PRINTING %d\n",this->ITERATIONS_BETWEEN_PRINTING);
   }
  fprintf(specfile," MAXIMUM-NUMBER-OF-ITERATIONS %d\n",this->MAXIMUM_NUMBER_OF_ITERATIONS);
  fprintf(specfile," SAVE-DATA-EVERY %d\n",this->SAVE_DATA_EVERY);

  if(this->CHECK_ALL_DERIVATIVES)fprintf(specfile," CHECK-ALL-DERIVATIVES\n");
  if(this->CHECK_DERIVATIVES)fprintf(specfile," CHECK-DERIVATIVES\n");
  if(this->CHECK_ELEMENT_DERIVATIVES)fprintf(specfile," CHECK-ELEMENT-DERIVATIVES\n");
  if(this->CHECK_GROUP_DERIVATIVES)fprintf(specfile," CHECK-GROUP-DERIVATIVES\n");
  if(this->IGNORE_DERIVATIVE_WARNINGS)fprintf(specfile," IGNORE-DERIVATIVE-WARNINGS\n");
  if(this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS)fprintf(specfile," IGNORE-ELEMENT-DERIVATIVE-WARNINGS\n");
  if(this->IGNORE_GROUP_DERIVATIVE_WARNINGS)fprintf(specfile," IGNORE-GROUP-DERIVATIVE-WARNINGS\n");
  if(this->FINITE_DIFFERENCE_GRADIENTS)fprintf(specfile," FINITE-DIFFERENCE-GRADIENTS\n");
  if(this->EXACT_SECOND_DERIVATIVES_USED)fprintf(specfile," EXACT-SECOND-DERIVATIVES-USED\n");
  if(this->BFGS_APPROXIMATION_USED)fprintf(specfile," BFGS-APPROXIMATION-USED\n");
  if(this->DFP_APPROXIMATION_USED)fprintf(specfile," DFP-APPROXIMATION-USED\n");
  if(this->PSB_APPROXIMATION_USED)fprintf(specfile," PSB-APPROXIMATION-USED\n");
  if(this->SR1_APPROXIMATION_USED)fprintf(specfile," SR1-APPROXIMATION-USED\n");
  if(this->USE_SCALING_FACTORS)fprintf(specfile," USE-SCALING-FACTORS\n");
  if(this->USE_CONSTRAINT_SCALING_FACTORS)fprintf(specfile," USE-CONSTRAINT-SCALING-FACTORS\n");
  if(this->USE_VARIABLE_SCALING_FACTORS)fprintf(specfile," USE-VARIABLE-SCALING-FACTORS\n");
  if(this->GET_SCALING_FACTORS)fprintf(specfile," GET-SCALING-FACTORS\n");
  if(this->PRINT_SCALING_FACTORS)fprintf(specfile," PRINT-SCALING-FACTORS\n");
  fprintf(specfile," CONSTRAINT-ACCURACY-REQUIRED %lf\n",this->CONSTRAINT_ACCURACY_REQUIRED);
  fprintf(specfile," GRADIENT-ACCURACY-REQUIRED %lf\n",this->GRADIENT_ACCURACY_REQUIRED);
  fprintf(specfile," INITIAL-PENALTY-PARAMETER %lf\n",this->INITIAL_PENALTY_PARAMETER);
  fprintf(specfile," DECREASE-PENALTY-PARAMETER-UNTIL< %lf\n",this->DECREASE_PENALTY_PARAMETER_UNTIL);
  fprintf(specfile," FIRST-CONSTRAINT-ACCURACY-REQUIRED %lf\n",this->FIRST_CONSTRAINT_ACCURACY_REQUIRED);
  fprintf(specfile," FIRST-GRADIENT-ACCURACY-REQUIRED %lf\n",this->FIRST_GRADIENT_ACCURACY_REQUIRED);
  if(this->INFINITY_NORM_TRUST_REGION_USED)fprintf(specfile," INFINITY-NORM-TRUST-REGION-USED\n");
  if(this->TWO_NORM_TRUST_REGION_USED)fprintf(specfile," TWO-NORM-TRUST-REGION-USED\n");
  if(this->TRUST_REGION_RADIUS>0.)fprintf(specfile," TRUST-REGION-RADIUS %lf\n",this->TRUST_REGION_RADIUS);
  if(this->SOLVE_BQP_ACCURATELY)fprintf(specfile," SOLVE-BQP-ACCURATELY\n");
  if(this->EXACT_CAUCHY_POINT_REQUIRED)fprintf(specfile," EXACT-CAUCHY-POINT-REQUIRED\n");
  if(this->INEXACT_CAUCHY_POINT_REQUIRED)fprintf(specfile," INEXACT-CAUCHY-POINT-REQUIRED\n");
  if(this->CG_METHOD_USED)fprintf(specfile," CG-METHOD-USED\n");
  if(this->DIAGONAL_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," DIAGONAL-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->MUNKSGAARDS_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," MUNKSGAARDS-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->EXPANDING_BAND_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," EXPANDING-BAND-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->FULL_MATRIX_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," FULL-MATRIX-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->GILL_MURRAY_PONCELEON_SAUNDERS_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," GILL-MURRAY-PONCELEON-SAUNDERS-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->MODIFIED_MA27_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," MODIFIED-MA27-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->SCHNABEL_ESKOW_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," SCHNABEL-ESKOW-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->USERS_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," USERS-PRECONDITIONED-CG-SOLVER-USED\n");
  if(this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED)fprintf(specfile," BANDSOLVER-PRECONDITIONED-CG-SOLVER-USED %d\n",this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED);
  if(this->MULTIFRONT_SOLVER_USED)fprintf(specfile," MULTIFRONT-SOLVER-USED\n");
  if(this->DIRECT_MODIFIED_MULTIFRONTAL_SOLVER_USED)fprintf(specfile," DIRECT-MODIFIED-MULTIFRONTAL-SOLVER-USED\n");
  if(this->RESTART_FROM_PREVIOUS_POINT)fprintf(specfile," RESTART-FROM-PREVIOUS-POINT\n");
  if(this->JIFFYTUNE_TOLERANCE_SET)fprintf(specfile," JIFFYTUNE_TOLERANCE %lf\n",this->JIFFYTUNE_TOLERANCE);

  fprintf(specfile,"END\n");

  fclose(specfile);
  return;
 }

int LNSetCheckDerivatives(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetCheckDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->CHECK_ALL_DERIVATIVES=FALSE;
  this->CHECK_DERIVATIVES=FALSE;
  this->CHECK_ELEMENT_DERIVATIVES=FALSE;
  this->CHECK_GROUP_DERIVATIVES=FALSE;
  switch(i)
   {
    case 0:
     break;
    case 1:
     this->CHECK_ALL_DERIVATIVES=TRUE;
     break;
    case 2:
     this->CHECK_DERIVATIVES=TRUE;
     break;
    case 3:
     this->CHECK_ELEMENT_DERIVATIVES=TRUE;
     break;
    case 4:
     this->CHECK_GROUP_DERIVATIVES=TRUE;
     break;
    default:
     sprintf(LNLanceErrorMsg,"Option %d (argument 2) is invalid, must be in range 0-4",i);
     NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
     return 0;
     break;
   }
  return 1;
 }

int LNGetCheckDerivatives(NLLancelot this)
 {
  char RoutineName[]="LNGetCheckDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!this->CHECK_ALL_DERIVATIVES&&
     !this->CHECK_DERIVATIVES&&
     !this->CHECK_ELEMENT_DERIVATIVES&&
     !this->CHECK_GROUP_DERIVATIVES)return(0);
  if( this->CHECK_ALL_DERIVATIVES&&
     !this->CHECK_DERIVATIVES&&
     !this->CHECK_ELEMENT_DERIVATIVES&&
     !this->CHECK_GROUP_DERIVATIVES)return(1);
  if(!this->CHECK_ALL_DERIVATIVES&&
      this->CHECK_DERIVATIVES&&
     !this->CHECK_ELEMENT_DERIVATIVES&&
     !this->CHECK_GROUP_DERIVATIVES)return(2);
  if(!this->CHECK_ALL_DERIVATIVES&&
     !this->CHECK_DERIVATIVES&&
      this->CHECK_ELEMENT_DERIVATIVES&&
     !this->CHECK_GROUP_DERIVATIVES)return(3);
  if(!this->CHECK_ALL_DERIVATIVES&&
     !this->CHECK_DERIVATIVES&&
     !this->CHECK_ELEMENT_DERIVATIVES&&
      this->CHECK_GROUP_DERIVATIVES)return(4);

  return(-1);
 }

int LNSetStopOnBadDerivatives(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetStopOnBadDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->IGNORE_DERIVATIVE_WARNINGS=FALSE;
  this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS=FALSE;
  this->IGNORE_GROUP_DERIVATIVE_WARNINGS=FALSE;
  switch(i)
   {
    case 0:
     break;
    case 1:
     this->IGNORE_DERIVATIVE_WARNINGS=TRUE;
     break;
    case 2:
     this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS=TRUE;
     break;
    case 3:
     this->IGNORE_GROUP_DERIVATIVE_WARNINGS=TRUE;
     break;
    default:
     sprintf(LNLanceErrorMsg,"Option %d (argument 2) is invalid, must be in range 0-2",i);
     NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
     return 0;
     break;
   }
  return 1;
 }

int LNGetStopOnBadDerivatives(NLLancelot this)
 {
  char RoutineName[]="LNGetStopOnBadDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  if(!this->IGNORE_DERIVATIVE_WARNINGS&&
     !this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS&&
     !this->IGNORE_GROUP_DERIVATIVE_WARNINGS)return(0);
  if( this->IGNORE_DERIVATIVE_WARNINGS&&
     !this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS&&
     !this->IGNORE_GROUP_DERIVATIVE_WARNINGS)return(1);
  if(!this->IGNORE_DERIVATIVE_WARNINGS&&
      this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS&&
     !this->IGNORE_GROUP_DERIVATIVE_WARNINGS)return(2);
  if(!this->IGNORE_DERIVATIVE_WARNINGS&&
     !this->IGNORE_ELEMENT_DERIVATIVE_WARNINGS&&
      this->IGNORE_GROUP_DERIVATIVE_WARNINGS)return(3);

  return(-1);
 }

int LNSetPrintLevel(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetPrintLevel";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->PRINT_LEVEL=i;
  if(i<0)
   {
    sprintf(LNLanceErrorMsg,"PrintLevel %d (argument 2) is invalid, must be nonnegative",i);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  return 1;
 }

int LNGetPrintLevel(NLLancelot this)
 {
  char RoutineName[]="LNGetPrintLevel";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->PRINT_LEVEL);
 }

int LNSetPrintStart(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetPrintStart";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->START_PRINTING_AT_ITERATION=i;
  return 1;
 }

int LNGetPrintStart(NLLancelot this)
 {
  char RoutineName[]="LNGetPrintStart";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->START_PRINTING_AT_ITERATION);
 }

int LNSetPrintStop(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetPrintStop";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->STOP_PRINTING_AT_ITERATION=i;
  return 1;
 }

int LNGetPrintStop(NLLancelot this)
 {
  char RoutineName[]="LNGetPrintStop";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->STOP_PRINTING_AT_ITERATION);
 }

int LNSetPrintEvery(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetPrintEvery";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->ITERATIONS_BETWEEN_PRINTING=i;
  return 1;
 }

int LNGetPrintEvery(NLLancelot this)
 {
  char RoutineName[]="LNGetPrintEvery";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->ITERATIONS_BETWEEN_PRINTING);
 }

int LNSetMaximumNumberOfIterations(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetMaximumNumberOfIterations";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->MAXIMUM_NUMBER_OF_ITERATIONS=i;
  return 1;
 }

int LNGetMaximumNumberOfIterations(NLLancelot this)
 {
  char RoutineName[]="LNGetMaximumNumberOfIterations";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->MAXIMUM_NUMBER_OF_ITERATIONS);
 }

int LNSetSaveDataEvery(NLLancelot this, int i)
 {
  char RoutineName[]="LNSetSaveDataEvery";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->SAVE_DATA_EVERY=i;
  return 1;
 }

int LNGetSaveDataEvery(NLLancelot this)
 {
  char RoutineName[]="LNGetSaveDataEvery";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->SAVE_DATA_EVERY);
 }


int LNSetUseExactFirstDerivatives(NLLancelot this, int v)
 {
  char RoutineName[]="LNSetUseExactFirstDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->FINITE_DIFFERENCE_GRADIENTS=!v;
  return 1;
 }

int LNGetUseExactFirstDerivatives(NLLancelot this)
 {
  char RoutineName[]="LNGetUseExactFirstDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(!this->FINITE_DIFFERENCE_GRADIENTS);
 }

int LNSetUseExactSecondDerivatives(NLLancelot this, char *choice)
 {
  char RoutineName[]="LNSetUseExactSecondDerivatives";

  int i,n;
  char *t;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  n=strlen(choice);
  t=(char*)malloc((n+1)*sizeof(char));
  if(t==(char*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",(n+1)*sizeof(char));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  strcpy(t,choice);
  for(i=0;i<n;i++)if(isupper(choice[i]))t[i]=tolower(choice[i]);

  this->EXACT_SECOND_DERIVATIVES_USED=FALSE;
  this->BFGS_APPROXIMATION_USED=FALSE;
  this->DFP_APPROXIMATION_USED=FALSE;
  this->PSB_APPROXIMATION_USED=FALSE;
  this->SR1_APPROXIMATION_USED=FALSE;
  if(!strcmp(t,"exact"))this->EXACT_SECOND_DERIVATIVES_USED=TRUE;
  else if(!strcmp(t,"bfgs"))this->BFGS_APPROXIMATION_USED=TRUE;
  else if(!strcmp(t,"dfp"))this->DFP_APPROXIMATION_USED=TRUE;
  else if(!strcmp(t,"psb"))this->PSB_APPROXIMATION_USED=TRUE;
  else if(!strcmp(t,"sr1"))this->SR1_APPROXIMATION_USED=TRUE;
  else
   {
    sprintf(LNLanceErrorMsg,"Option \"%s\" is invalid, must be one of \"Exact\", \"BFGS\",\"DFP\",\"PSB\", \"SR1\",",choice);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    free(t);
    return 0;
   }

  free(t);
  return 1;
 }

char *LNGetUseExactSecondDerivatives(NLLancelot this)
 {
  char RoutineName[]="LNGetUseExactSecondDerivatives";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(this->EXACT_SECOND_DERIVATIVES_USED)return("Exact");
  else if(this->BFGS_APPROXIMATION_USED)return("BFGS");
  else if(this->DFP_APPROXIMATION_USED)return("DFP");
  else if(this->PSB_APPROXIMATION_USED)return("PSB");
  else if(this->SR1_APPROXIMATION_USED)return("SR1");
  else return("Unknown");
 }

int LNSetConstraintAccuracy(NLLancelot this, double a)
 {
  char RoutineName[]="LNSetConstraintAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->CONSTRAINT_ACCURACY_REQUIRED=a;
  return 1;
 }

double LNGetConstraintAccuracy(NLLancelot this)
 {
  char RoutineName[]="LNGetConstraintAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->CONSTRAINT_ACCURACY_REQUIRED);
 }

int LNSetGradientAccuracy(NLLancelot this, double a)
 {
  char RoutineName[]="LNSetGradientAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  this->GRADIENT_ACCURACY_REQUIRED=a;
  return 1;
 }

double LNGetGradientAccuracy(NLLancelot this)
 {
  char RoutineName[]="LNGetGradientAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->GRADIENT_ACCURACY_REQUIRED);
 }

int LNSetInitialPenalty(NLLancelot this, double p)
 {
  char RoutineName[]="LNSetInitialPenalty";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->INITIAL_PENALTY_PARAMETER=p;
  return 1;
 }

double LNGetInitialPenalty(NLLancelot this)
 {
  char RoutineName[]="LNGetInitialPenalty";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->INITIAL_PENALTY_PARAMETER);
 }

int LNSetPenaltyBound(NLLancelot this, double p)
 {
  char RoutineName[]="LNSetPenaltyBound";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->DECREASE_PENALTY_PARAMETER_UNTIL=p;
  return 1;
 }

double LNGetPenaltyBound(NLLancelot this)
 {
  char RoutineName[]="LNGetPenaltyBound";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->DECREASE_PENALTY_PARAMETER_UNTIL);
 }

int LNSetFirstConstraintAccuracy(NLLancelot this, double a)
 {
  char RoutineName[]="LNSetFirstConstraintAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->FIRST_CONSTRAINT_ACCURACY_REQUIRED=a;
  return 1;
 }

double LNGetFirstConstraintAccuracy(NLLancelot this)
 {
  char RoutineName[]="LNGetFirstConstraintAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->FIRST_CONSTRAINT_ACCURACY_REQUIRED);
 }

int LNSetFirstGradientAccuracy(NLLancelot this, double a)
 {
  char RoutineName[]="LNSetFirstGradientAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->FIRST_GRADIENT_ACCURACY_REQUIRED=a;
  return 1;
 }

double LNGetFirstGradientAccuracy(NLLancelot this)
 {
  char RoutineName[]="LNGetFirstGradientAccuracy";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->FIRST_GRADIENT_ACCURACY_REQUIRED);
 }

int LNSetTrustRegionType(NLLancelot this, char *choice)
 {
  char RoutineName[]="LNSetTrustRegionType";

  int i,n;
  char *t;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  n=strlen(choice);
  t=(char*)malloc((n+1)*sizeof(char));
  if(t==(char*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",(n+1)*sizeof(char));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  strcpy(t,choice);
  for(i=0;i<n;i++)if(isupper(choice[i]))t[i]=tolower(choice[i]);

  this->INFINITY_NORM_TRUST_REGION_USED=FALSE;
  this->TWO_NORM_TRUST_REGION_USED=FALSE;
  if(!strcmp(t,"two norm"))this->TWO_NORM_TRUST_REGION_USED=TRUE;
  else if(!strcmp(t,"infinity norm"))this->INFINITY_NORM_TRUST_REGION_USED=TRUE;
  else
   {
    sprintf(LNLanceErrorMsg,"TrustRegionType \"%s\" (argument 2) is invalid, must be one of \"Two Norm\" \"Infinity Norm\"",choice);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    free(t);
    return 0;
   }

  free(t);
  return 1;
 }

char *LNGetTrustRegionType(NLLancelot this)
 {
  char RoutineName[]="LNGetTrustRegionType";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(this->TWO_NORM_TRUST_REGION_USED)return("Two Norm");
  else if(this->INFINITY_NORM_TRUST_REGION_USED)return("Infinity Norm");
  else return("Unknown");
 }

int LNSetTrustRegionRadius(NLLancelot this, double r)
 {
  char RoutineName[]="LNSetTrustRegionRadius";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->TRUST_REGION_RADIUS=r;
  return 1;
 }

double LNGetTrustRegionRadius(NLLancelot this)
 {
  char RoutineName[]="LNGetTrustRegionRadius";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  return(this->TRUST_REGION_RADIUS);
 }

int LNSetSolveBQPAccurately(NLLancelot this, int v)
 {
  char RoutineName[]="LNSetSolveBQPAccurately";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->SOLVE_BQP_ACCURATELY=v;
  return 1;
 }

int LNGetSolveBQPAccurately(NLLancelot this)
 {
  char RoutineName[]="LNGetSolveBQPAccurately";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->SOLVE_BQP_ACCURATELY);
 }

int LNSetRequireExactCauchyPoint(NLLancelot this, int v)
 {
  char RoutineName[]="LNSetRequireExactCauchyPoint";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->EXACT_CAUCHY_POINT_REQUIRED=v;
  this->INEXACT_CAUCHY_POINT_REQUIRED=!v;

  return 1;
 }

int LNGetRequireExactCauchyPoint(NLLancelot this)
 {
  char RoutineName[]="LNGetRequireExactCauchyPoint";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->EXACT_CAUCHY_POINT_REQUIRED);
 }

int LNSetLinearSolverMethod(NLLancelot this, char *choice,int bandwidth)
 {
  char RoutineName[]="LNSetLinearSolverMethod";
  int i,n;
  char *t;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  n=strlen(choice);
  t=(char*)malloc((n+1)*sizeof(char));
  if(t==(char*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",(n+1)*sizeof(char));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  strcpy(t,choice);
  for(i=0;i<n;i++)if(isupper(choice[i]))t[i]=tolower(choice[i]);

  this->CG_METHOD_USED=FALSE;
  this->DIAGONAL_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->MUNKSGAARDS_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->EXPANDING_BAND_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->FULL_MATRIX_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->GILL_MURRAY_PONCELEON_SAUNDERS_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->MODIFIED_MA27_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->SCHNABEL_ESKOW_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->USERS_PRECONDITIONED_CG_SOLVER_USED=FALSE;
  this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED=0;
  this->MULTIFRONT_SOLVER_USED=FALSE;
  this->DIRECT_MODIFIED_MULTIFRONTAL_SOLVER_USED=FALSE;

  if(!strcmp(t,"diagonal preconditioned"))this->DIAGONAL_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"munksgaards preconditioned"))this->MUNKSGAARDS_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"expanding band preconditioned"))this->EXPANDING_BAND_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"full matrix preconditioned"))this->FULL_MATRIX_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"gill-murray-ponceleon-saunders preconditioned"))this->GILL_MURRAY_PONCELEON_SAUNDERS_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"modified ma27 preconditioned"))this->MODIFIED_MA27_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"schnabel-eskow preconditioned"))this->SCHNABEL_ESKOW_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"users preconditioned"))this->USERS_PRECONDITIONED_CG_SOLVER_USED=TRUE;
  else if(!strcmp(t,"bandsolver preconditioned"))this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED=bandwidth;
  else if(!strcmp(t,"multifront"))this->MULTIFRONT_SOLVER_USED=TRUE;
  else if(!strcmp(t,"direct modified"))this->DIRECT_MODIFIED_MULTIFRONTAL_SOLVER_USED=TRUE;
  else if(!strcmp(t,"cg method used"))this->CG_METHOD_USED=TRUE;
  else
   {
    sprintf(LNLanceErrorMsg,"Linear Solver Type \"%s\" (argument 2) is invalid",choice);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    free(t);
    return 0;
   }

  free(t);
  return 1;
 }

char *LNGetLinearSolverMethod(NLLancelot this)
 {
  char RoutineName[]="LNGetLinearSolverMethod";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(this->DIAGONAL_PRECONDITIONED_CG_SOLVER_USED)return("Diagonal Preconditioned");
  else if(this->MUNKSGAARDS_PRECONDITIONED_CG_SOLVER_USED)return("Munksgaards Preconditioned");
  else if(this->EXPANDING_BAND_PRECONDITIONED_CG_SOLVER_USED)return("Expanding Band Preconditioned");
  else if(this->FULL_MATRIX_PRECONDITIONED_CG_SOLVER_USED)return("Full Matrix Preconditioned");
  else if(this->GILL_MURRAY_PONCELEON_SAUNDERS_PRECONDITIONED_CG_SOLVER_USED)return("Gill-Murray-Ponceleon-Saunders Preconditioned");
  else if(this->MODIFIED_MA27_PRECONDITIONED_CG_SOLVER_USED)return("Modified MA27 Preconditioned");
  else if(this->SCHNABEL_ESKOW_PRECONDITIONED_CG_SOLVER_USED)return("Schnabel-Eskow Preconditioned");
  else if(this->USERS_PRECONDITIONED_CG_SOLVER_USED)return("Users Preconditioned");
  else if(this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED!=0)return("Bandsolver Preconditioned");
  else if(this->MULTIFRONT_SOLVER_USED)return("Multifront");
  else if(this->DIRECT_MODIFIED_MULTIFRONTAL_SOLVER_USED)return("Direct Modified");
  else return("Unknown");
 }

int LNGetLinearSolverBandwidth(NLLancelot this)
 {
  char RoutineName[]="LNGetLinearSolverBandwidth";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->BANDSOLVER_PRECONDITIONED_CG_SOLVER_USED);
 }

int LNSetScalings(NLLancelot this, char *choice)
 {
  char RoutineName[]="LNSetScalings";

  int i,n;
  char *t;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  n=strlen(choice);
  t=(char*)malloc((n+1)*sizeof(char));
  if(t==(char*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",(n+1)*sizeof(char));
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  strcpy(t,choice);
  for(i=0;i<n;i++)if(isupper(choice[i]))t[i]=tolower(choice[i]);

  this->USE_SCALING_FACTORS=FALSE;
  this->USE_CONSTRAINT_SCALING_FACTORS=FALSE;
  this->USE_VARIABLE_SCALING_FACTORS=FALSE;
  this->GET_SCALING_FACTORS=FALSE;
  this->PRINT_SCALING_FACTORS=FALSE;

  if(!strcmp(t,"no scaling"))this->USE_SCALING_FACTORS=FALSE;
   else if(!strcmp(t,"scale constraints")) this->USE_CONSTRAINT_SCALING_FACTORS=TRUE;
   else if(!strcmp(t,"scale variables")) this->USE_VARIABLE_SCALING_FACTORS=TRUE;
   else if(!strcmp(t,"scale both")) this->USE_SCALING_FACTORS=TRUE;
   else if(!strcmp(t,"print but don't use")) this->PRINT_SCALING_FACTORS=TRUE;
   else
   {
    sprintf(LNLanceErrorMsg,"Scaling \"%s\" (argument 2) is invalid, must be one of \"No Scaling\",\"Scale Constraints\",\"Scale Variables\",\"Scale Both\",\"Print but don't use\"",choice);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    free(t);
    return 0;
   }

  free(t);
  return 1;
 }

char *LNGetScalings(NLLancelot this)
 {
  char RoutineName[]="LNGetScalings";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return (char*)NULL;
   }

  if(this->USE_SCALING_FACTORS)return("Scale Both");
  else if(this->USE_CONSTRAINT_SCALING_FACTORS)return("Scale Constraints");
  else if(this->USE_VARIABLE_SCALING_FACTORS)return("Scale Variables");
  else if(this->PRINT_SCALING_FACTORS)return("Print but don't use");
  else if(!this->PRINT_SCALING_FACTORS
       && !this->USE_SCALING_FACTORS
       && !this->USE_CONSTRAINT_SCALING_FACTORS
       && !this->USE_VARIABLE_SCALING_FACTORS)return("No Scaling");
  else return("Unknown");
 }

void LNSetMaxMin(NLLancelot this,int flag)
 {
  this->MAXIMIZER_SOUGHT=flag;
 }

void LNSetProblem(NLProblem P)
 {
  LANSOLProblem=P;
 }

void LNSetZ0(double *z0)
 {
  LANSOLz0=z0;
 }

void LNSetZGroup(int zgroup)
 {
  LANSOLZGroup=zgroup;
 }

int LNSetJiffyTuneTolerance(NLLancelot this, double tol)
 {
  char RoutineName[]="LNSetJiffyTuneTolerance";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(4,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  this->JIFFYTUNE_TOLERANCE_SET=1;
  this->JIFFYTUNE_TOLERANCE=tol;

  return 1;
 }

int LNGetJiffyTuneTolerance(NLLancelot this, double *tol)
 {
  char RoutineName[]="LNGetJiffyTuneTolerance";

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(4,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(tol!=(double*)NULL&&this->JIFFYTUNE_TOLERANCE_SET)*tol=this->JIFFYTUNE_TOLERANCE;

  return this->JIFFYTUNE_TOLERANCE_SET;
 }

/*      author: Mike Henderson mhender@watson.ibm.com */
/*      date:   Nov. 11, 1997                         */
/*              Feb. 2, 1999   Ported to C            */

#include <NLPAPI.h>
#include <float.h>
#include <strings.h>
#include <stdlib.h>

/*
void F77NAME(elfuns)(F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*);
void F77NAME(groups)(F77DOUBLEPRECISION*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77INTEGER*,F77LOGICAL*);
void F77NAME(ranges)(F77INTEGER*,F77LOGICAL*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77INTEGER*,F77INTEGER*);
void F77NAME(gminma)(F77INTEGER*,F77DOUBLEPRECISION*);*/

void LNDumpSpecFile(NLLancelot);
void NLSetError(int,char*,char*,int,char*);
double NLPGetObjectiveLowerBound(NLProblem);
double NLPGetObjectiveUpperBound(NLProblem);
void LNSetMaxMin(NLLancelot,int);
void LNSetProblem(NLProblem);
void LNSetZ0(double*);
int LNDumpOUTSDIF_D(NLLancelot,NLProblem,double*,double*);

extern int F77_FUNC(mainl,MAINL)(void);

int LNMinimize(NLLancelot this,NLProblem P,double *x0,double *z0,double *l0,double *x)
 {
  char RoutineName[]="LNMinimize";

#ifndef HAVE_LANCELOT
  sprintf(LNLanceErrorMsg,"LANCELOT is not installed on this system (or wasn't when NLPAPI was configured)");
  NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
  return 12;
#else
  int i,j,k;
  int verbose;
  int rc;
  char *aliveFile;
  int removeALIVE;
  FILE *alive;
  int c;
  char buf[133]="";
  char *s;
  char *D;
  char *token,*varName,*varValue,*name;
  double d;
  FILE *solution;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(x0==(double*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Pointer to initial guess (argument 2) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(x==(double*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Pointer to area to store the solution (argument 6) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  verbose=0;

  if(verbose){printf("In LNMinimize\n");
  printf("  NumberOfMinMaxConstraints=%d, NumberOfGroupsInObjective=%d\n",NLPGetNumberOfMinMaxConstraints(P),NLPGetNumberOfGroupsInObjective(P));fflush(stdout);
  printf("x0: [%lf",x0[0]);for(i=1;i<NLPGetNumberOfVariables(P);i++)printf(",%lf",x0[i]);printf("]\n");fflush(stdout);}

  if(this->BFGS_APPROXIMATION_USED)NLPSetAllElementUpdateTypes(P,"BFGS",0);
   else if(this->SR1_APPROXIMATION_USED)NLPSetAllElementUpdateTypes(P,"SR1",0);

  if(NLPGetNumberOfMinMaxConstraints(P)>0&&NLPGetZGroupNumber(P)<0)
   {
    NLVector a=(NLVector)NULL;
    int objGrp=-1;

    objGrp=NLPAddGroupToObjective(P,"OBJ");
    NLPSetZGroupNumber(P,objGrp);
    a=NLCreateVector(NLPGetNumberOfVariables(P)+1);
    NLVSetC(a,NLPGetNumberOfVariables(P),1.);
    NLPSetObjectiveGroupA(P,objGrp,a);
    NLFreeVector(a);
   }


  LNSetMaxMin(this,FALSE);

  LNSetProblem(P);
  LNSetZ0(z0);
  LNSetZGroup(NLPGetZGroupNumber(P));
  k=0;
  if(verbose)printf("There are %d groups\n",NLPGetNumberOfGroups(P));

  if(verbose){printf("LNMinimize entered, dumping OUTSDIF.d ... nvars=%d\n",NLPGetNumberOfVariables(P));fflush(stdout);}
  rc=LNDumpOUTSDIF_D(this,P,x0,l0);
  if(!rc)
   {
    sprintf(LNLanceErrorMsg,"Dump of OUTSDIF.d file failed");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  LNDumpSpecFile(this);
  if(verbose)printf("LNMinimize entered, done dumping OUTSDIF.d ... rc=%d\n",rc);

/*------------------------------------------------------------------------   */

  aliveFile="ALIVE.d";
  removeALIVE=FALSE;

  if(verbose)printf("LNMinimize - Creating -->%s<-- \n",aliveFile);
  alive=fopen(aliveFile,"w");
  if(alive==(FILE*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Error opening file %s for writing in current directory",aliveFile);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }else
    if(verbose)printf("-->%s<-- successfully opened.\n",aliveFile);
  rc=fprintf(alive," LANCELOT rampages onwards \n");
  if(verbose)printf("%d characters written to -->%s<--\n",rc,aliveFile);
  rc=fclose(alive);
  if(rc!=0)
   {
    sprintf(LNLanceErrorMsg,"Error closing file %s",aliveFile);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }else 
    if(verbose)printf("-->%s<-- successfully closed.\n",aliveFile);
  removeALIVE=TRUE;

  if(verbose)printf("LNMinimize - Invoking mainl\n");
  rc=F77_FUNC(mainl,MAINL)();
  if(verbose)printf("LNMinimize - done Invoking mainl, rc=%d\n",rc);

/*------------Set things back to the default values-----------------------   */

  LNSetProblem((NLProblem)NULL);
  LNSetZ0((double*)NULL);
  LNSetZGroup(0);

  if(verbose)printf("done LNMinimize\n");

 /* Here we read the SOLUTION.d file, and maybe the SUMMARY.d */

  if(verbose)printf("LNMinimize open SOLUTION.d\n");
  solution=fopen("SOLUTION.d","r");
  if(solution==(FILE*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Error opening file SOLUTION.d for reading in current directory");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  while((c=fgetc(solution))!=EOF)
   {
    i=0;
    buf[i]=c;i++;
    if(c=='\n')continue;
    while((c=fgetc(solution))!='\n' && c!=EOF && i<132){buf[i]=c;i++;}
    buf[i]=0x0;i++;

    if((s=strstr(buf,"SOLUTION "))!=(char*)NULL)
     {
      if(verbose)printf("Found SOLUTION string in -->%s<---\n",buf);
      token=strtok(s," ");
      varName=strtok((char*)NULL," ");
      varValue=strtok((char*)NULL," ");
      if(verbose)printf("tokenized is -->%s<-- -->%s<-- -->%s<--\n",token,varName,varValue);
      for(i=0;i<NLPGetNumberOfVariables(P);i++)
       {
        name=NLPGetVariableName(P,i);
        if(verbose)printf("Checking variable %d -->%s<--\n",i,name);
        if(!strcmp(name,varName))
         {
          if(verbose)printf("This is variable %d\n",i);
          if((D=strstr(varValue,"D"))!=(char*)NULL)D[0]='E';
          d=-111.;
          sscanf(varValue,"%lf",&d);
          if(verbose){printf("  value %lf in string %s\n",d,varValue);fflush(stdout);}
          x[i]=d;
         }
       }
     }
   }

  return rc;
#endif
 }

int LNMaximize(NLLancelot this,NLProblem P,double *x0,double *z0,double *l0,double *x)
 {
  char RoutineName[]="LNMaximize";

#ifdef NLPAPI_NO_LANCELOT
  sprintf(LNLanceErrorMsg,"LANCELOT is not installed on this system (or wasn't when NLPAPI was configured)");
  NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
  return 12;
#else
  int i,j,k;
  int rc;
  char *aliveFile;
  int removeALIVE;
  FILE *alive;
  int c;
  char buf[133]="";
  char *s;
  char *D;
  char *token,*varName,*varValue,*name;
  double d;
  double *result;
  FILE *solution;

  int verbose;

  if(this==(NLLancelot)NULL)
   {
    sprintf(LNLanceErrorMsg,"Solver (argument 1) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(x0==(double*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Pointer to initial guess (argument 2) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(x==(double*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Pointer to area to store the solution (argument 3) is NULL");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  verbose=0;
  rc=1;

  if(verbose){printf("In LNMaximize\n");
  printf("  NumberOfMinMaxConstraints=%d, NumberOfGroupsInObjective=%d\n",NLPGetNumberOfMinMaxConstraints(P),NLPGetNumberOfGroupsInObjective(P));fflush(stdout);}

  if(this->BFGS_APPROXIMATION_USED)NLPSetAllElementUpdateTypes(P,"BFGS",0);
   else if(this->SR1_APPROXIMATION_USED)NLPSetAllElementUpdateTypes(P,"SR1",0);

  if(NLPGetNumberOfMinMaxConstraints(P)>0&&NLPGetZGroupNumber(P)<0)
   {
    NLVector a=(NLVector)NULL;
    int objGrp=-1;

    objGrp=NLPAddGroupToObjective(P,"OBJ");
    NLPSetZGroupNumber(P,objGrp);
    a=NLCreateVector(NLPGetNumberOfVariables(P)+1);
    NLVSetC(a,NLPGetNumberOfVariables(P),1.);
    NLPSetObjectiveGroupA(P,objGrp,a);
    NLFreeVector(a);
   }

  LNSetMaxMin(this,TRUE);

  LNSetProblem(P);
  LNSetZ0(z0);
  LNSetZGroup(NLPGetZGroupNumber(P));
  k=0;
  if(verbose)printf("There are %d groups\n",NLPGetNumberOfGroups(P));

  if(verbose)printf("LNMaximize entered, dumping OUTSDIF.d ... \n");
  rc=LNDumpOUTSDIF_D(this,P,x0,l0);
  if(!rc)
   {
    sprintf(LNLanceErrorMsg,"Dump of OUTSDIF.d file failed");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  LNDumpSpecFile(this);
  if(verbose)printf("LNMaximize entered, done dumping OUTSDIF.d ... rc=%d\n",rc);

/*------------------------------------------------------------------------   */
  aliveFile="ALIVE.d";
  removeALIVE=FALSE;

  if(verbose)printf("LNMaximize - Creating -->%s<-- \n",aliveFile);
  alive=fopen(aliveFile,"w");
  if(alive==(FILE*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Error opening file %s for writing in current directory",aliveFile);
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }else
    if(verbose)printf("-->%s<-- successfully opened.\n",aliveFile);
  rc=fprintf(alive," LANCELOT rampages onwards \n");
  if(verbose)printf("%d characters written to -->%s<--\n",rc,aliveFile);
  rc=fclose(alive);
  if(rc!=0)
   {
    sprintf(LNLanceErrorMsg,"Error closing file %s",aliveFile);
    return 0;
   }else 
    if(verbose)printf("-->%s<-- successfully closed.\n",aliveFile);
  removeALIVE=TRUE;

  if(verbose)printf("LNMaximize - Invoking mainl\n");
  rc=F77_FUNC(mainl,MAINL)();
  if(verbose)printf("LNMaximize - done Invoking mainl, rc=%d\n",rc);

/*--------------Set things back to defaults-------------------------------   */

  LNSetProblem((NLProblem)NULL);
  LNSetZ0((double*)NULL);
  LNSetZGroup(0);

  if(verbose)printf("done LNMaximize\n");

 /* Here we read the SOLUTION.d file, and maybe the SUMMARY.d */

  if(verbose)printf("LNMaximize open SOLUTION.d\n");
  solution=fopen("SOLUTION.d","r");
  if(solution==(FILE*)NULL)
   {
    sprintf(LNLanceErrorMsg,"Error opening file SOLUTION.d for reading in current directory");
    NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  while((c=fgetc(solution))!=EOF)
   {
    i=0;
    buf[i]=c;i++;
    if(c=='\n')continue;
    while((c=fgetc(solution))!='\n' && c!=EOF && i<132){buf[i]=c;i++;}
    buf[i]=0x0;i++;

    if((s=strstr(buf,"SOLUTION "))!=(char*)NULL)
     {
      if(verbose)printf("Found SOLUTION string in -->%s<--\n",buf);
      token=strtok(s," ");
      varName=strtok((char*)NULL," ");
      varValue=strtok((char*)NULL," ");
      if(verbose)printf("tokenized is -->%s<-- -->%s<-- -->%s<--\n",token,varName,varValue);
      for(i=0;i<NLPGetNumberOfVariables(P);i++)
       {
        name=NLPGetVariableName(P,i);
        if(verbose)printf("Checking variable %d -->%s<--\n",i,name);
        if(!strcmp(name,varName))
         {
          if(verbose)printf("This is variable %d\n",i);
          if((D=strstr(varValue,"D"))!=(char*)NULL)D[0]='E';
          d=-111.;
          sscanf(varValue,"%lf",&d);
          if(verbose)printf("  value %lf in string %s\n",d,varValue);
          x[i]=d;
         }
       }
     }
   }

  return rc;
#endif
 }

void F77_FUNC(iniths,INITHS)(F77DOUBLEPRECISION *variables,F77DOUBLEPRECISION *fuvals,F77INTEGER *nElements,F77INTEGER *istaev,F77INTEGER *ielvar,
             F77INTEGER *istadh,F77INTEGER *elementList)
 {
  char RoutineName[]="iniths";

  int element;
  double *x,*l;
  double *d2fdx2;
  double *Hessian;
  int i,j,k,m;
  int jk;
  NLElementFunction elementFunction=(NLElementFunction)NULL;
  int nx,nl;
  double *X,*L,*F;
  int verbose;
  int trace;
  static int mx;
  static double *pX=(double*)NULL;
  static int mu;
  static double *pU=(double*)NULL;
  int nu;
  int false=0;

  verbose=0;
  trace=0;

  if(verbose){printf("in INITHS nelements=%d\n",*nElements);fflush(stdout);}

  for(i=0;i<*nElements;i++)
   {
    element=elementList[i]-1;
    elementFunction=NLNEGetElementFunction(LANSOLProblem,element);

/* element is a nonlinear element */

    x=variables+ielvar[istaev[element]-1]-1;
    Hessian=fuvals+istadh[element]-1;

    if(NLEGetRangeXForm(elementFunction)==(NLMatrix)NULL)
     {
      nx=NLEGetDimension(elementFunction);

      if(nx>mx)
       {
        pX=(double*)realloc((void*)pX,nx*sizeof(double));
        if(pX==(double*)NULL)
         {
          sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nx*sizeof(char));
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          return;
         }
       }
      for(j=0;j<nx;j++)pX[j]=variables[ielvar[istaev[element]-1+j]-1];
     }else{
      nu=NLMGetNumberOfCols(NLEGetRangeXForm(elementFunction));
      nx=NLMGetNumberOfRows(NLEGetRangeXForm(elementFunction));

      if(nu>mu)
       {
        pU=(double*)realloc((void*)pU,nu*sizeof(double));
        if(pU==(double*)NULL)
         {
          sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nu*sizeof(char));
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          return;
         }
       }
      for(j=0;j<nu;j++)pU[j]=variables[ielvar[istaev[element]-1+j]-1];
      if(nx>mx)
       {
        pX=(double*)realloc((void*)pX,nx*sizeof(double));
        if(pX==(double*)NULL)
         {
          sprintf(LNLanceErrorMsg,"Out of memory, trying to allocate %d bytes",nx*sizeof(char));
          NLSetError(12,RoutineName,LNLanceErrorMsg,__LINE__,__FILE__);
          return;
         }
       }
      NLMVMult(NLEGetRangeXForm(elementFunction),pU,pX);
     }

    d2fdx2=NLElementFunctionGetInitialHessianMatrix(elementFunction);
    if(d2fdx2!=(double*)NULL)
     {
      jk=0;
      for(k=0;k<nx;k++)
       {
        for(j=0;j<=k;j++)
         {
          Hessian[jk]=d2fdx2[jk];jk++;
         }
       }
     }
   }
  if(verbose){printf("done INITHS:\n");fflush(stdout);}

  return;
 }

void F77_FUNC(lnpgetinitialhessian,LNPGETINITIALHESSIAN)(F77INTEGER *e, F77DOUBLEPRECISION *h)
 {
  NLElementFunction ef;

  ef=NLNEGetElementFunction(LANSOLProblem,*e-1);
  NLElementFunctionGetInitialHessian(ef,h);
  return;
 }

/* #include <../src/LanceCUTER.c>*/
