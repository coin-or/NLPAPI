/*
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory
*/
/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: @(#)Matrix.c	3.8 02/07/30 10:44:25 */
/*      date:   November 11, 1997                     */
/*              February 2, 1999   Ported to C        */
/*              September 28, 2001 Added "wrapped"    */

#include <NLPAPI.h>

void NLSetError(int,char*,char*,int,char*);
static char NLMatrixErrorMsg[256]="";

#define NUMBERTOALLOC 100
#define FULL 0
#define DUMBSPARSE 1
#define WSMPSPARSE 2

int NLVnNonZeros(NLVector);
int *NLVnonZero(NLVector);
double *NLVData(NLVector);
int *NLNEGetElementVariables(NLProblem,NLNonlinearElement);

struct NLGrpPartMat
 {
  int sparse;
  int wrapped;
  int nRows;
  int nCols;
  double *data;
  int nE;
  int mE;
  int *row;
  int *col;
  int nRefs;
 };

void NLMatrixIncreaseSparse(NLMatrix this)
 {
  char RoutineName[]="NLMatrixIncreaseSparse";

  this->mE+=NUMBERTOALLOC;
  this->data=(double*)realloc((void*)this->data,this->mE*sizeof(double));
  if(this->data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate sparse matrix (%d bytes)",this->mE*sizeof(double));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }
  this->row=(int*)realloc((void*)this->row,this->mE*sizeof(int));
  if(this->row==(int*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate sparse matrix (%d bytes)",this->mE*sizeof(int));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }
  this->col=(int*)realloc((void*)this->col,this->mE*sizeof(int));
  if(this->col==(int*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate sparse matrix (%d bytes)",this->mE*sizeof(int));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  return;
 }

NLMatrix NLCreateMatrix(int n,int m)
 {
  char RoutineName[]="NLCreateMatrix";
  int i;

  NLMatrix this=(NLMatrix)malloc(sizeof(struct NLGrpPartMat));
  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartMat));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return((NLMatrix)NULL);
   }
  if(n<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of rows %d (argument 1) is negative.",n);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  if(m<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of columns %d (argument 2) is negative.",m);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  this->sparse=FULL;
  this->wrapped=0;
  this->nRows=n;
  this->nCols=m;
  this->data=(double*)malloc(n*m*sizeof(double));
  if(this->data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %dx%d matrix (%d bytes)",n,m,n*m*sizeof(double));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  this->nE=0;
  this->mE=0;
  this->row=(int*)NULL;
  this->col=(int*)NULL;
  for(i=0;i<n*m;i++)
    (this->data)[i]=0.;
  this->nRefs=1;

  return(this);
 }

NLMatrix NLCreateMatrixWithData(int n,int m,double *data)
 {
  char RoutineName[]="NLCreateMatrixWithData";
  int i;
  NLMatrix this;

  this=(NLMatrix)malloc(sizeof(struct NLGrpPartMat));
  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartMat));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return((NLMatrix)NULL);
   }

  if(n<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of rows %d (argument 1) is negative",n);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  if(m<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of columns %d (argument 2) is negative.",m);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  this->sparse=FULL;
  this->wrapped=0;
  this->nRows=n;
  this->nCols=m;
  this->data=(double*)malloc(n*m*sizeof(double));
  if(this->data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %dx%d matrix (%d bytes)",n,m,n*m*sizeof(double));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  this->nE=0;
  this->mE=0;
  this->row=(int*)NULL;
  this->col=(int*)NULL;

  if(data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Pointer to data (argument 3) is NULL");
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    for(i=0;i<n*m;i++)(this->data)[i]=0.;
    return(this);
   }

  for(i=0;i<n*m;i++)(this->data)[i]=data[i];
  this->nRefs=1;

  return(this);
 }

void NLFreeMatrix(NLMatrix this)
 {
  char RoutineName[]="NLFreeMatrix";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->nRefs--;

  if(this->nRefs<1)
   {
    if(!(this->wrapped) && this->data!=(double*)NULL)free(this->data);
    if(this->row!=(int*)NULL)free(this->row);
    if(this->col!=(int*)NULL)free(this->col);
    free(this);
   }
  return;
 }

int NLMGetNumberOfRows(NLMatrix this)
 {
  char RoutineName[]="NLMGetNumberOfRows";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nRows);
 }

int NLMGetNumberOfCols(NLMatrix this)
 {
  char RoutineName[]="NLMGetNumberOfCols";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return(this->nCols);
 }

double NLMGetElement(NLMatrix this,int i,int j)
 {
  int l;
  char RoutineName[]="NLMGetElement";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }

  if(i<0)sprintf(NLMatrixErrorMsg,"Row index %d (argument 2) is negative.",i);
   else if(j<0)sprintf(NLMatrixErrorMsg,"Column index %d (argument 3) is negative.",j);
   else if(i>=this->nRows)sprintf(NLMatrixErrorMsg,"Row index %d (argument 2) is too large. Must be less than %d.",i,this->nRows);
   else if(j>=this->nCols)sprintf(NLMatrixErrorMsg,"Column index %d (argument 3) is too large. Must be less than %d",j,this->nCols);
   else{
    if(this->sparse==FULL)
     {
      return(this->data[i+this->nRows*j]);
     }else if(this->sparse==DUMBSPARSE)
     {
      for(l=0;l<this->nE;l++)
       if(this->row[l]==i&&this->col[l]==j)return this->data[l];
      return 0.;
     }else if(this->sparse==WSMPSPARSE){
      if(i>j){l=i;i=j;j=l;}
      for(l=this->row[i];l<this->row[i+1];l++)
        if(this->col[l]==j)return this->data[l];
      return 0.;
     }
   }

  sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
  NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
  return DBL_QNAN;
 }

int NLMSetElement(NLMatrix this,int i,int j,double vl)
 {
  int l;
  char RoutineName[]="NLMSetElement";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(i<0)sprintf(NLMatrixErrorMsg,"Row index %d (argument 2) is negative.",i);
  else if(j<0)sprintf(NLMatrixErrorMsg,"Column index %d (argument 3) is negative.",j);
  else if(i>=this->nRows)sprintf(NLMatrixErrorMsg,"Row index %d (argument 2) is too large. Must be less than %d.",i,this->nRows);
  else if(j>=this->nCols)sprintf(NLMatrixErrorMsg,"Column index %d (argument 3) is too large. Must be less than %d",j,this->nCols);
  else{
    if(this->sparse==FULL)
     {
      this->data[i+this->nRows*j]=vl;
      return 1;
     }else if(this->sparse==DUMBSPARSE)
     {
      if(this->nE>=this->mE)NLMatrixIncreaseSparse(this);
      for(l=0;l<this->nE;l++)
       if(this->row[l]==i&&this->col[l]==j)
        {
         this->data[l]=vl;
         return 1;
        }
      this->row[this->nE]=i;
      this->col[this->nE]=j;
      this->data[this->nE]=vl;
      this->nE++;
      return 1;
     }else if(this->sparse==WSMPSPARSE){
      if(i>j){l=i;i=j;j=l;}
      for(l=this->row[i];l<this->row[i+1];l++)
       if(this->col[l]==j){this->data[l]=vl;return 1;}
      sprintf(NLMatrixErrorMsg,"Element (%d,%d) is not an element of WSMP Sparse Matrix (argument 1)",i,j);
      NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
      return 0;
     }
   }

  sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
  NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
  return 0;
 }

void NLRefMatrix(NLMatrix this)
 {
  char RoutineName[]="NLRefMatrix";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->nRefs++;
 }

int NLMGetNumberOfRefs(NLMatrix this)
 {
  return this->nRefs;
 }

void NLPrintMatrix(FILE *fid,NLMatrix this)
 {
  char RoutineName[]="NLPrintMatrix";
  int i,j;

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"File pointer (argument 1), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 2), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  for(i=0;i<this->nRows;i++)
   {
    fprintf(fid,"          [");
    for(j=0;j<this->nCols;j++)
     {
      if(j>0)fprintf(fid,",");
      if(this->sparse==FULL)
        fprintf(fid,"%lf",this->data[i+this->nRows*j]);
      else
        fprintf(fid,"%lf",NLMGetElement(this,i,j));
     }
    fprintf(fid,"]\n");
   }

#ifdef PRINTRAWMATRIX
  if(this->sparse==DUMBSPARSE)
   {
    printf("Raw:\n");
    for(i=0;i<this->nE;i++)
     {
      printf("%4.4d  A(%d,%d) = %le\n",i,this->row[i],this->col[i],this->data[i]);
     }
    fflush(stdout);
   }

  if(this->sparse==WSMPSPARSE)
   {
    printf("Raw:\n");
    for(i=0;i<this->nE;i++)
     {
      if(i<this->nRows+1)printf("   %3.3d",this->row[i]);
        else printf("      ");
      printf("   %3.3d  %le\n",this->col[i],this->data[i]);
     }
    fflush(stdout);
   }
#endif

  return;
 }

void NLMVMult(NLMatrix A, double *x, double *b)
 {
  int i,j;
  char RoutineName[]="NLMVMult";

  if(A==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"A (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(x==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"x (second argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(b==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"b (third argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(A->sparse==FULL)
   {
    for(i=0;i<A->nRows;i++)
     {
      b[i]=0.;
      for(j=0;j<A->nCols;j++)
        b[i]+=A->data[i+A->nRows*j]*x[j];
     }
    return;
   }else if(A->sparse==DUMBSPARSE)
   {
    for(i=0;i<A->nRows;i++)b[i]=0.;
    for(i=0;i<A->nE;i++)b[A->row[i]]+=A->data[i]*x[A->col[i]];
    return;
   }else if(A->sparse==WSMPSPARSE)
   {
    for(i=0;i<A->nRows;i++)b[i]=0.;
    for(i=0;i<A->nRows;i++)
     {
      for(j=A->row[i];j<A->row[i+1];j++)
       {
        b[i]+=A->data[j]*x[A->col[j]];
        b[A->col[j]]+=A->data[j]*x[i];
       }
     }
    return;
   }

  sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",A->sparse);
  NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
  return;
 }

void NLMVMultT(NLMatrix A, double *x, double *b)
 {
  int i,j;
  char RoutineName[]="NLMVMult";

  if(A==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"A (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(x==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"x (second argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(b==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"b (third argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(A->sparse==FULL)
   {
    for(j=0;j<A->nCols;j++)
     {
      b[j]=0.;
      for(i=0;i<A->nRows;i++)
        b[j]+=A->data[i+A->nRows*j]*x[i];
     }
   }else if(A->sparse==DUMBSPARSE)
   {
    for(i=0;i<A->nCols;i++)b[i]=0.;
    for(i=0;i<A->nE;i++)b[A->col[i]]+=A->data[i]*x[A->row[i]];
   }else if(A->sparse==WSMPSPARSE)
   {
    for(i=0;i<A->nCols;i++)b[i]=0.;
    for(i=0;i<A->nRows;i++)
     {
      for(j=A->row[i];j<A->row[i+1];j++)
       {
        b[A->col[j]]+=A->data[j]*x[i];
        b[i]+=A->data[j]*x[A->col[j]];
       }
     }
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",A->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  return;
 }

NLMatrix NLCreateSparseMatrix(int n,int m)
 {
  char RoutineName[]="NLCreateSparseMatrix";
  int i;

  NLMatrix this=(NLMatrix)malloc(sizeof(struct NLGrpPartMat));
  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartMat));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return((NLMatrix)NULL);
   }
  if(n<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of rows %d (argument 1) is negative.",n);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  if(m<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of columns %d (argument 2) is negative.",m);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  this->sparse=DUMBSPARSE;
  this->wrapped=0;
  this->nRows=n;
  this->nCols=m;
  this->nE=0;
  this->mE=NUMBERTOALLOC;
  this->data=(double*)malloc(this->mE*sizeof(double));
  if(this->data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %dx%d sparse matrix (%d bytes)",n,m,this->mE*sizeof(double));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  this->row=(int*)malloc(this->mE*sizeof(int));
  if(this->row==(int*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %dx%d sparse matrix (%d bytes)",n,m,this->mE*sizeof(int));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  this->col=(int*)malloc(this->mE*sizeof(int));
  if(this->col==(int*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %dx%d sparse matrix (%d bytes)",n,m,this->mE*sizeof(int));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  for(i=0;i<this->mE;i++)
   {
    (this->data)[i]=0.;
    (this->row)[i]=-1;
    (this->col)[i]=-1;
   }
  this->nRefs=1;

  return(this);
 }

int NLMIncrementElement(NLMatrix this,int i,int j,double vl)
 {
  int l;
  char RoutineName[]="NLMIncrementElement";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  if(i<0)sprintf(NLMatrixErrorMsg,"Row index %d (argument 2) is negative.",i);
  else if(j<0)sprintf(NLMatrixErrorMsg,"Column index %d (argument 3) is negative.",j);
  else if(i>=this->nRows)sprintf(NLMatrixErrorMsg,"Row index %d (argument 2) is too large. Must be less than %d.",i,this->nRows);
  else if(j>=this->nCols)sprintf(NLMatrixErrorMsg,"Column index %d (argument 3) is too large. Must be less than %d",j,this->nCols);
  else{
    if(this->sparse==FULL)
     {
      this->data[i+this->nRows*j]+=vl;
      return 1;
     }else if(this->sparse==DUMBSPARSE){
      if(this->nE>=this->mE)NLMatrixIncreaseSparse(this);
      for(l=0;l<this->nE;l++)
       if(this->row[l]==i&&this->col[l]==j)
        {
         this->data[l]+=vl;
         return 1;
        }
      this->row[this->nE]=i;
      this->col[this->nE]=j;
      this->data[this->nE]=vl;
      this->nE++;
      return 1;
     }else if(this->sparse==WSMPSPARSE){
      if(i>j){l=i;i=j;j=l;}
      for(l=this->row[i];l<this->row[i+1];l++)
       if(this->col[l]==j){this->data[l]+=vl;return 1;}
      sprintf(NLMatrixErrorMsg,"Element (%d,%d) is not an element of WSMP Sparse Matrix (argument 1)",i,j);
      NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
      return 0;
     }
   }

  sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
  NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
  return 0;
 }

NLMatrix NLCreateDenseWrappedMatrix(int n,int m, double *data)
 {
  char RoutineName[]="NLCreateDenseWrappedMatrix";
  int i;

  NLMatrix this=(NLMatrix)malloc(sizeof(struct NLGrpPartMat));
  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartMat));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return((NLMatrix)NULL);
   }
  if(n<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of rows %d (argument 1) is negative.",n);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }
  if(m<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of columns %d (argument 2) is negative.",m);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  this->sparse=FULL;
  this->wrapped=1;
  this->nRows=n;
  this->nCols=m;
  this->data=data;
  this->nE=0;
  this->mE=0;
  this->row=(int*)NULL;
  this->col=(int*)NULL;
  for(i=0;i<n*m;i++)
    (this->data)[i]=0.;
  this->nRefs=1;

  return(this);
 }

int NLMSetToZero(NLMatrix this)
 {
  int i,j;
  char RoutineName[]="NLMSetToZero";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  switch(this->sparse)
   {
    case FULL:
     for(i=0;i<this->nCols*this->nRows;i++)this->data[i]=0.;
     break;

    case DUMBSPARSE:
     for(i=0;i<this->nE;i++)this->data[i]=0.;
     break;

    case WSMPSPARSE:
     for(i=0;i<this->nE;i++)this->data[i]=0.;
     break;
   }

  return 1;
 }

NLMatrix NLMatrixClone(NLMatrix this)
 {
  char RoutineName[]="NLMatrixClone";
  NLMatrix clone;

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return (NLMatrix)NULL;
   }

  if(!this->sparse)
   {
    clone=NLCreateMatrixWithData(this->nRows,this->nCols,this->data);
   }else{
    clone=NLCreateSparseMatrix(this->nRows,this->nCols);
    if(this->mE>clone->mE)
     {
      clone->mE=this->mE;
      clone->data=(double*)realloc((void*)clone->data,clone->mE*sizeof(double));
      if(clone->data==(double*)NULL)
       {
        sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate sparse matrix (%d bytes)",clone->mE*sizeof(double));
        NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
        return;
       }
      clone->row=(int*)realloc((void*)clone->row,clone->mE*sizeof(int));
      if(clone->row==(int*)NULL)
       {
        sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate sparse matrix (%d bytes)",clone->mE*sizeof(int));
        NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
        return;
       }
      clone->col=(int*)realloc((void*)clone->col,clone->mE*sizeof(int));
      if(clone->col==(int*)NULL)
       {
        sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate sparse matrix (%d bytes)",clone->mE*sizeof(int));
        NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
        return;
       }
     }
    clone->nE=this->nE;
    clone->sparse=this->sparse;
    memcpy((void*)(clone->data),(void*)(this->data),this->mE*sizeof(double));
    memcpy((void*)(clone->col),(void*)(this->col),this->mE*sizeof(int));
    memcpy((void*)(clone->row),(void*)(this->row),this->mE*sizeof(int));
   }

  return clone;
 }

int NLMSparse(NLMatrix this)
 {
  char RoutineName[]="NLMSparse";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  return this->sparse;
 }

double *NLMData(NLMatrix this)
 {
  char RoutineName[]="NLMData";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return (double*)NULL;
   }

  return this->data;
 }

int NLMnE(NLMatrix this)
 {
  char RoutineName[]="NLMnE";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return -1;
   }

  switch(this->sparse)
   {
    case FULL:
     return this->nRows*this->nCols;
     break;

    case DUMBSPARSE:
     return this->nE;
     break;

    case WSMPSPARSE:
     return this->nE;
     break;
   }
  return -1;
 }

int *NLMRow(NLMatrix this)
 {
  char RoutineName[]="NLMRow";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return (int*)NULL;
   }

  return this->row;
 }

int *NLMCol(NLMatrix this)
 {
  char RoutineName[]="NLMCol";

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument) is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return (int*)NULL;
   }

  return this->col;
 }

void NLMMMMProd(NLMatrix R,double *M,double *B)
 {
  char RoutineName[]="NLMMMMProd";
  int i,j,k,l;
  int n,m;

/* B= R^T M R */

  if(R==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(M==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (second argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(B==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Result (third argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  n=R->nCols;
  m=R->nRows;
  if(R->sparse==FULL||R->sparse==DUMBSPARSE)
   {
    for(j=0;j<n;j++)
     {
      for(i=0;i<n;i++)
       {
        B[i+n*j]=0.;
        if(R->sparse==FULL)
         {
          for(l=0;l<m;l++)
           {
            for(k=0;k<m;k++)
             {
              B[i+n*j]+=R->data[k+m*i]*M[k+m*l]*R->data[l+m*j];
             }
           }
         }else if(R->sparse==DUMBSPARSE)
         {
          for(i=0;i<n*n;i++)B[i]=0.;
          for(l=0;l<R->nE;l++)
           {
            for(k=0;k<R->nE;k++)
             {
              B[R->col[k]+n*R->col[l]]+=R->data[k]*M[R->row[k]+m*R->row[l]]*R->data[l];
             }
           }
         }
       }
     }
   }else if(R->sparse==WSMPSPARSE)
   {
    for(j=0;j<n;j++)
      for(i=0;i<n;i++)B[i+n*j]=0.;
 
    for(l=0;l<m;l++)
     {
      for(k=0;k<m;k++)
       {
        for(j=R->row[l];j<R->row[l+1];j++)
         {
          for(i=R->row[k];i<R->row[k+1];i++)
           {
            B[R->col[i]+n*R->col[j]]+=R->data[i]*M[k+m*l]*R->data[j];
           }
         }
       }
     }
 
    for(j=0;j<n;j++)
     {
      for(i=0;i<n;i++)
       {
        for(l=R->row[j];l<R->row[j+1]-1;l++)
         {
          for(k=R->row[i];k<R->row[i+1]-1;k++)
           {
            B[i+n*j]+=R->data[k]*M[R->col[l]+m*R->col[k]]*R->data[l];
           }
         }
       }
     }
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",R->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  return;
 }

#ifndef MAX
#define MAX(X, Y) ((X) < (Y) ? (Y) : (X))
#endif
#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#include <time.h>

double NLGetMaxScaledDiagonalTime=0.;
int NLGetMaxScaledDiagonalNCalls=0;
double NLGetMinScaledDiagonalTime=0.;
int NLGetMinScaledDiagonalNCalls=0;
double NLGetGershgorinBoundsTime=0.;
int NLGetGershgorinBoundsNCalls=0;
double NLMatrixOneNormTime=0.;
int NLMatrixOneNormNCalls=0;
double NLMatrixDoubleProductTime=0.;
int NLMatrixDoubleProductNCalls=0;

double NLDetermineSparsityTime=0.;
int NLDetermineSparsityNCalls=0.;
double NLSumIntoTime=0.;
int NLSumIntoNCalls=0.;

double NLGetMinScaledDiagonal(NLMatrix this, double *M)
 {
  char RoutineName[]="NLGetMinScaledDiagonal";
  double result;
  int i;
  int flag;
  clock_t tin;

  tin=clock();
  if(this->sparse==FULL)
   {
    result=this->data[0]/M[0];
    for(i=1;i<this->nRows;i++)result=MIN(result,this->data[i+this->nRows*i]/M[i]);
   }else if(this->sparse==DUMBSPARSE)
   {
    flag=0;
    for(i=0;i<this->nE;i++)
     {
      if(this->row[i]==this->col[i])
       {
        if(flag==0)
         {
          result=this->data[i]/M[this->row[i]];
          flag=1;
         }else{
          result=MIN(result,this->data[i]/M[this->row[i]]);
         }
       }
     }
   }else if(this->sparse==WSMPSPARSE)
   {
    result=this->data[this->row[0]]/M[0];
    for(i=1;i<this->nRows;i++)result=MIN(result,this->data[this->row[i]]/M[i]);
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLGetMinScaledDiagonalTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLGetMinScaledDiagonalNCalls++;
  return result;
 }

double NLGetMaxScaledDiagonal(NLMatrix this, double *M)
 {
  char RoutineName[]="NLGetMaxScaledDiagonal";
  double result;
  int i;
  int flag;
  clock_t tin;

  tin=clock();
  if(this->sparse==FULL)
   {
    result=this->data[0]/M[0];
    for(i=1;i<this->nRows;i++)result=MAX(result,this->data[i+this->nRows*i]/M[i]);
   }else if(this->sparse==DUMBSPARSE){
    flag=0;
    for(i=0;i<this->nE;i++)
     {
      if(this->row[i]==this->col[i])
       {
        if(flag==0)
         {
          result=this->data[i]/M[this->row[i]];
          flag=1;
         }else{
          result=MAX(result,this->data[i]/M[this->row[i]]);
         }
       }
     }
   }else if(this->sparse==WSMPSPARSE)
   {
    result=this->data[this->row[0]]/M[0];
    for(i=1;i<this->nRows;i++)result=MAX(result,this->data[this->row[i]]/M[i]);
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLGetMaxScaledDiagonalTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLGetMaxScaledDiagonalNCalls++;
  return result;
 }

static double *NLrowsum=(double*)NULL;
static double *NLdiag=(double*)NULL;

void NLGetGershgorinBounds(NLMatrix this,double *M,double *L,double *U)
 {
  char RoutineName[]="NLGetGershgorinBounds";
  int i,j;
  int flag;
  double rowsum;
  clock_t tin;

  tin=clock();

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(L==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Address for the lower bound (third argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(U==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Address for the upper bound (fourth argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(this->sparse==FULL)
   {
    for(i=0;i<this->nRows;i++)
     {
      rowsum=0.;
      for(j=0;j<this->nCols;j++)
       {
        if(M!=(double*)NULL)
         {
          if(j!=i)rowsum+=fabs(this->data[i+this->nRows*j])/sqrt(M[i])/sqrt(M[j]);
         }else{
          if(j!=i)rowsum+=fabs(this->data[i+this->nRows*j]);
         }
       }
      if(i==0)
       {
        if(M!=(double*)NULL)
         {
          *L= this->data[i+this->nRows*i]/M[i]+rowsum;
          *U=-this->data[i+this->nRows*i]/M[i]+rowsum;
         }else{
          *L= this->data[i+this->nRows*i]+rowsum;
          *U=-this->data[i+this->nRows*i]+rowsum;
         }
       }else{
        if(M!=(double*)NULL)
         {
          *L=MAX(*L, this->data[i+this->nRows*i]/M[i]+rowsum);
          *U=MAX(*U,-this->data[i+this->nRows*i]/M[i]+rowsum);
         }else{
          *L=MAX(*L, this->data[i+this->nRows*i]+rowsum);
          *U=MAX(*U,-this->data[i+this->nRows*i]+rowsum);
         }
       }
     }
   }else if(this->sparse==DUMBSPARSE)
   {
    NLrowsum=(double*)realloc((void*)NLrowsum,(this->nRows)*sizeof(double));
    if(NLrowsum==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    NLdiag=(double*)realloc((void*)NLdiag,(this->nRows)*sizeof(double));
    if(NLdiag==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<this->nRows;i++){NLrowsum[i]=0.;NLdiag[i]=0.;}
    for(i=0;i<this->nE;i++)
     {
      if(this->row[i]==this->col[i])
       {
        if(M!=(double*)NULL)NLdiag[this->row[i]]=this->data[i]/M[this->row[i]];
         else NLdiag[this->row[i]]=this->data[i];
 
       }else{
        if(M!=(double*)NULL)NLrowsum[this->row[i]]+=fabs(this->data[i])/sqrt(M[this->row[i]])/sqrt(M[this->col[i]]);
         else NLrowsum[this->row[i]]+=fabs(this->data[i]);
       }
     }
    *L= NLdiag[0]+NLrowsum[0];
    *U=-NLdiag[0]+NLrowsum[0];
    for(i=1;i<this->nRows;i++)
     {
      *L=MAX(*L, NLdiag[i]+NLrowsum[i]);
      *U=MAX(*U,-NLdiag[i]+NLrowsum[i]);
     }
   }else if(this->sparse==WSMPSPARSE)
   {
    NLrowsum=(double*)realloc((void*)NLrowsum,(this->nRows)*sizeof(double));
    if(NLrowsum==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    NLdiag=(double*)realloc((void*)NLdiag,(this->nRows)*sizeof(double));
    if(NLdiag==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<this->nRows;i++){NLrowsum[i]=0.;NLdiag[i]=0.;}
    for(i=0;i<this->nRows;i++)
     {
      for(j=this->row[i]+1;j<this->row[i+1];j++)
       {
        if(M!=(double*)NULL)
         {
          NLrowsum[i]+=fabs(this->data[j])/sqrt(M[i])/sqrt(M[this->col[j]]);
          NLrowsum[this->col[j]]+=fabs(this->data[j])/sqrt(M[i])/sqrt(M[this->col[j]]);
         }else{
          NLrowsum[i]+=fabs(this->data[j]);
          NLrowsum[this->col[j]]+=fabs(this->data[j]);
         }
       }
      if(M!=(double*)NULL)NLdiag[i]=this->data[this->row[i]]/M[i];
       else NLdiag[i]=this->data[this->row[i]];
     }
    *L= NLdiag[0]+NLrowsum[0];
    *U=-NLdiag[0]+NLrowsum[0];
    for(i=1;i<this->nRows;i++)
     {
      *L=MAX(*L, NLdiag[i]+NLrowsum[i]);
      *U=MAX(*U,-NLdiag[i]+NLrowsum[i]);
     }
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  NLGetGershgorinBoundsTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLGetGershgorinBoundsNCalls++;
  return;
 }

double NLMatrixOneNorm(NLMatrix this,double *M)
 {
  char RoutineName[]="NLMatrixOneNorm";
  static double result,row;
  static int i,j;
  clock_t tin;

  tin=clock();

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(this->sparse==FULL)
   {
    result=0.;
    for(j=0;j<this->nCols;j++)
     {
      row=0.;
      for(i=0;i<this->nRows;i++)
        {
       if(M!=(double*)NULL)
          row+=fabs(this->data[i+this->nRows*j])/sqrt(M[i])/sqrt(M[j]);
         else
          for(i=0;i<this->nRows;i++)row+=fabs(this->data[i+this->nRows*j]);
       }
      if(result<row)result=row;
     }
   }else if(this->sparse==DUMBSPARSE)
   {
    NLrowsum=(double*)realloc((void*)NLrowsum,(this->nRows)*sizeof(double));
    if(NLrowsum==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<this->nRows;i++)NLrowsum[i]=0.;
    for(i=0;i<this->nE;i++)
     {
      if(M!=(double*)NULL)NLrowsum[this->row[i]]+=fabs(this->data[i])/sqrt(M[this->row[i]])/sqrt(M[this->col[i]]);
       else NLrowsum[this->row[i]]+=fabs(this->data[i]);
     }
    result=0.;
    for(i=0;i<this->nRows;i++)if(result<NLrowsum[i])result=NLrowsum[i];
   }else if(this->sparse==WSMPSPARSE)
   {
    NLrowsum=(double*)realloc((void*)NLrowsum,(this->nRows)*sizeof(double));
    if(NLrowsum==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<this->nRows;i++)NLrowsum[i]=0.;
    for(i=0;i<this->nRows;i++)
     {
      j=this->row[i];
      if(M!=(double*)NULL)NLrowsum[i]+=fabs(this->data[j])/sqrt(M[i])/sqrt(M[this->col[j]]);
       else NLrowsum[i]+=fabs(this->data[j]);
      for(j=this->row[i]+1;j<this->row[i+1];j++)
       {
        if(M!=(double*)NULL)
         {
          NLrowsum[i]+=fabs(this->data[j])/sqrt(M[i])/sqrt(M[this->col[j]]);
          NLrowsum[this->col[j]]+=fabs(this->data[j])/sqrt(M[i])/sqrt(M[this->col[j]]);
         }else{
          NLrowsum[i]+=fabs(this->data[j]);
          NLrowsum[this->col[j]]+=fabs(this->data[j]);
         }
       }
     }
    result=0.;
    for(i=0;i<this->nRows;i++)if(result<NLrowsum[i])result=NLrowsum[i];
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLMatrixOneNormTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLMatrixOneNormNCalls++;
  return result;
 }

double NLMatrixDoubleProduct(NLVector u,NLMatrix this,NLVector v)
 {
  static char RoutineName[]="NLMatrixDoubleProduct";
  static int i,j,n,m;
  static double row;
  static double result;
  clock_t tin;

  tin=clock();

  if(u==(NLVector)NULL)
   {
    sprintf(NLMatrixErrorMsg,"left vector (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (second argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(v==(NLVector)NULL)
   {
    sprintf(NLMatrixErrorMsg,"right vector (third argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  n=NLVGetNC(u);
  m=NLVGetNC(v);
  if(n!=this->nRows || m!=this->nCols)
   {
    sprintf(NLMatrixErrorMsg,"Cannot find u^TAv for a %d vector a %dx%d matrix and a %d vector.",n,this->nRows,this->nCols,m);
    printf("Cannot find u^TAv for a %d vector a %dx%d matrix and a %d vector.",n,this->nRows,this->nCols,m);
    printf("A is\n");
    NLPrintMatrix(stdout,this);
    printf("u is\n");
    NLPrintVector(stdout,u);
    printf("v is\n");
    NLPrintVector(stdout,v);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0.;
   }

  if(this->sparse==FULL)
   {
    result=0;
    for(i=0;i<this->nRows;i++)
     {
      row=0.;
      for(j=0;j<this->nCols;j++)
       {
        row+=this->data[i+this->nRows*j]*NLVGetC(v,j);
       }
      result+=NLVGetC(u,i)*row;
     }
   }else if(this->sparse==DUMBSPARSE)
   {
    result=0;
    for(i=0;i<this->nE;i++)
     {
      result+=this->data[i]*NLVGetC(u,this->row[i])*NLVGetC(v,this->col[i]);
     }
   }else if(this->sparse==WSMPSPARSE)
   {
    result=0;
    for(i=0;i<this->nRows;i++)
     {
      j=this->row[i];
      result+=this->data[j]*NLVGetC(u,i)*NLVGetC(v,this->col[j]);
      for(j=this->row[i]+1;j<this->row[i+1];j++)
       {
        result+=this->data[j]*NLVGetC(u,i)*NLVGetC(v,this->col[j]);
        result+=this->data[j]*NLVGetC(u,this->col[j])*NLVGetC(v,i);
       }
     }
   }else{
    sprintf(NLMatrixErrorMsg,"Matrix (argument 1) is unknown format (%d)",this->sparse);
    NLSetError(4,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return 0;
   }

  NLMatrixDoubleProductTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLMatrixDoubleProductNCalls++;
  return result;
 }

void NLMSumSubMatrixInto(NLMatrix M, double s, int n, int *r, double *data)
 {
  static char RoutineName[]="NLMSumSubMatrixInto";
  int i,j;
  int ir,ic;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(M==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(r==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Array of row indices (fourth argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Array of submatrix elements (fifth argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

/* Put in code for DUMBSPARSE and WSMPMATRIX */

  if(verbose)
   {
    printf("%s, (%le)*[%d",RoutineName,s,r[0]);
    for(i=1;i<n;i++)printf(",%d",r[i]);
    printf("]\n");fflush(stdout);
   }

  if(s==0.)
   {
    NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
    NLSumIntoNCalls++;
    return;
   }
  switch(M->sparse)
   {
    case FULL:
     if(verbose){printf(" type FULL\n");fflush(stdout);}
     for(j=0;j<n;j++)
       for(i=0;i<n;i++)
         M->data[r[i]+M->nRows*r[j]]+=s*data[i+n*j];
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
     break;
    case DUMBSPARSE:
     if(verbose){printf(" type DUMBSPARSE\n");fflush(stdout);}
     for(i=0;i<M->nE;i++)
      {
       if(M->row[i]>=r[0]&&M->row[i]<=r[n-1] && M->col[i]>=r[0]&&M->col[i]<=r[n-1])
        {
         ir=0;while(r[ir]<M->row[i])ir++;
         ic=0;while(r[ic]<M->col[i])ic++;
         if(r[ir]==M->row[i] && r[ic]==M->col[i])M->data[i]+=s*data[ir+n*ic];
        }
      }
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
     break;
    case WSMPSPARSE:
     if(verbose){printf(" type WSMPSPARSE\n");fflush(stdout);}
     for(i=0;i<n;i++)
      {
       ir=r[i];
       ic=M->row[ir];
       for(j=i;j<n;j++)
        {
         if(data[i+n*j]!=0.)
          {
           while(M->col[ic]<r[j] && ic<M->nE)ic++;
           if(!(ic<M->nE))abort();
           (M->data[ic])+=s*data[i+n*j];
          }
        }
      }
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
    default:
     if(verbose){printf(" type default\n");fflush(stdout);}
     for(j=0;j<n;j++)
       for(i=0;i<n;i++)
         if(data[i+n*j]!=0.)NLMSetElement(M,r[i],r[j],NLMGetElement(M,r[i],r[j])+s*data[i+n*j]);
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
     break;
   }
 }

void NLMSumRankOneInto(NLMatrix M, double s, double *data)
 {
  static char RoutineName[]="NLMSumRankOneInto";
  int i,j;
  int verbose=0;
  clock_t tin;

  tin=clock();

  if(M==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(data==(double*)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Vector (third argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

/* Put in code for DUMBSPARSE and WSMPMATRIX */

  if(verbose)
   {
    printf("%s, (%le)*[",RoutineName,s);
    j=0;
    for(i=0;i<M->nCols;i++){if(data[i]!=0.){if(j==1)printf(",");printf("%d",i);j=1;}}
    printf("]\n");fflush(stdout);
   }

  if(s==0.)
   {
    NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
    NLSumIntoNCalls++;
    return;
   }

  switch(M->sparse)
   {
    case FULL:
     for(j=0;j<M->nCols;j++)
       if(data[j]!=0)
        {
         for(i=0;i<M->nRows;i++)M->data[i+M->nRows*j]+=s*data[i]*data[j];
        }
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
     break;
    case DUMBSPARSE:
     for(i=0;i<M->nE;i++)
       M->data[i]+=s*data[M->row[i]]*data[M->col[i]];
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
     break;
    case WSMPSPARSE:
     for(i=0;i<M->nRows;i++)
      {
       if(data[i]!=0.)
        {
         for(j=M->row[i];j<M->row[i+1];j++)
          {
           if(data[M->col[j]]!=0.)M->data[j]+=s*data[i]*data[M->col[j]];
          }
        }
      }
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
    default:
     for(j=0;j<M->nCols;j++)
      {
       if(data[j]!=0.)
        {
         for(i=0;i<M->nRows;i++)
          {
           if(data[i]!=0.)NLMSetElement(M,i,j,NLMGetElement(M,i,j)+s*data[i]*data[j]);
          }
        }
      }
     NLSumIntoTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
     NLSumIntoNCalls++;
     return;
     break;
   }
 }

void NLMInsertNonzeros(int**,int*,int,int*,int,int);
void NLMPrintSparsityStructure(int n,int *nCols,int **rowIndex)
 {
  char RoutineName[]="NLMPrintSparsityStructure";
  int i,j;

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"File pointer (argument 1), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Matrix (argument 2), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  printf("Sparsity structure:\n");
  for(i=0;i<n;i++)
   {
    for(j=0;j<nCols[i];j++)
     {
      printf(" (%3d,%3d)",i,(rowIndex[i])[j]);
     }
    if(nCols[i]>0){printf("\n");fflush(stdout);}
   }

  return;
 }

void NLMDetermineHessianSparsityStructure(NLProblem P,char f, int constraint, NLMatrix H)
 {
  static char RoutineName[]="NLMDetermineHessianSparsityStructure";
  int **rowIndex;
  int *nCols;
  int i,j,n,m,t;
  int I;
  int ig,ng,g;
  int ie,ne;
  NLVector a;
  int nev;
  int *ev;
  int *nz=(int*)NULL;
  int *ez=(int*)NULL;
  int nlv;
  int *lv=(int*)NULL;
  double *v;
  NLNonlinearElement e;
  int sym;
  int verbose=0;
  clock_t tin;
  int diag[1];

  tin=clock();

  if(P==(NLProblem)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Problem (first argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(f!='O'&&f!='I'&&f!='E'&&f!='M')
   {
    sprintf(NLMatrixErrorMsg,"type %c (second argument), is not valid. Must be 'O', 'I', 'E', or 'M'",f);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(H==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Hessian (fourth argument), is NULL");
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return;
   }

/* f='O', Objective */
/* f='I', Inequality i */
/* f='E', Equality i */
/* f='M', MinMax */

/* Each Element in the function contributes a square of the element v's, plus a a^t of the linear parts */

  if(verbose){printf("%s H is 0x%8.8x, # nonzeros in H is %d\n",RoutineName,H,NLMnE(H));fflush(stdout);}

  if(H->sparse==FULL)
   {
    NLDetermineSparsityTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
    NLDetermineSparsityNCalls++;
    return;
   }

  n=H->nRows;
  m=H->nCols;
  rowIndex=(int**)malloc(n*sizeof(int*));
  if(rowIndex==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
  for(i=0;i<n;i++)rowIndex[i]=(int*)NULL;
  nCols=(int*)malloc(n*sizeof(int*));
  if(nCols==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
  for(i=0;i<n;i++)nCols[i]=0;

  sym=0;
  if(H->sparse==WSMPSPARSE)sym=1;

  for(i=0;i<n;i++)
   {
    diag[0]=i;
    NLMInsertNonzeros(rowIndex,nCols,1,diag,sym,n);
   }

  if(f=='O')
    ng=NLPGetNumberOfGroupsInObjective(P);
   else if(f=='I')
    ng=NLPGetNumberOfGroupsInInequalityConstraint(P,constraint);
   else if(f=='E')
    ng=NLPGetNumberOfGroupsInEqualityConstraint(P,constraint);
   else if(f=='M')
    ng=NLPGetNumberOfGroupsInMinMaxConstraint(P,constraint);

  for(ig=0;ig<ng;ig++)
   {
    if(f=='O')
      g=NLPGetObjectiveGroupNumber(P,ig);
     else if(f=='I')
      g=NLPGetInequalityConstraintGroupNumber(P,constraint);
     else if(f=='E')
      g=NLPGetEqualityConstraintGroupNumber(P,constraint);
     else if(f=='M')
      g=NLPGetMinMaxConstraintGroupNumber(P,constraint);

    if(verbose){printf("\nGroup %d:\n",g);fflush(stdout);}

    if(NLPIsGroupASet(P,g))
     {
      a=NLPGetGroupA(P,g);
      if(NLVSparse(a))
       {
        nz=NLVnonZero(a);
        nlv=NLVnNonZeros(a);
        lv=(int*)malloc(nlv*sizeof(int));
        if(nlv>0&&lv==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory trying to allocate %d bytes",nlv*sizeof(int));NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
        for(i=0;i<nlv;i++)lv[i]=nz[i];
        nz=(int*)NULL;
        for(i=0;i<nlv;i++)
         {
          for(j=i+1;j<nlv;j++)
           {
            if(lv[i]>lv[j])
             {
              t=lv[i];
              lv[i]=lv[j];
              lv[j]=t;
             }
           }
         }
       }else{
        v=NLVData(a);
        nlv=0;
        for(i=0;i<n;i++)if(v[i]!=0.);nlv++;
        lv=(int*)malloc(nlv*sizeof(int));
        if(lv==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
        t=0;
        for(i=0;i<n;i++)if(v[i]!=0.){lv[t]=i;t++;}
       }
     }else{
      nlv=0;
      lv=(int*)NULL;
     }
    if(verbose)
     {
      if(nlv>0){printf("lv's: [%d",lv[0]);for(i=1;i<nlv;i++)printf(",%d",lv[i]);printf("]\n");fflush(stdout);}
        else {printf("no lv's\n");fflush(stdout);}
     }

/* Nonlinear Elements */

    ne=NLPGetNumberOfElementsInGroup(P,g);

    for(ie=0;ie<ne;ie++)
     {
      e=NLPGetNonlinearElementOfGroup(P,g,ie);
      ev=NLNEGetElementVariables(P,e);
      nev=NLNEGetElementDimension(P,e);
      if(verbose){printf("Element %d\n",ie);fflush(stdout);}
      if(verbose){printf("unsorted ev's: [%d",ev[0]);for(i=1;i<nev;i++)printf(",%d",ev[i]);printf("]\n");fflush(stdout);}
  
/* Sort the element variables */

      ez=(int*)realloc((void*)ez,(nev+nlv)*sizeof(int));
      if(ez==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
      for(i=0;i<nev;i++)ez[i]=ev[i];
      for(i=0;i<nev;i++)
       {
        for(j=i+1;j<nev;j++)
         {
          if(ez[i]>ez[j])
           {
            t=ez[i];
            ez[i]=ez[j];
            ez[j]=t;
           }
         }
       }

/* Remove duplicates */

      if(0&&verbose){printf("Remove duplicate ev's\n");fflush(stdout);}
      j=0;
      for(i=0;i<nev-1;i++)
       {
        if(0&&verbose){printf("Test ev[%d]=%d and ev[%d]=%d\n",j,ez[j],i+1,ez[i+1]);fflush(stdout);}
        if(ez[i+1]!=ez[j]){j++;ez[j]=ez[i+1];}
/*       else { printf(" ev's %d and %d are %d\n",i+1,j,ez[j]);fflush(stdout);}*/
       }
      nev=j+1;
  
/* f'' */

      if(verbose){printf("ev's: [%d",ez[0]);for(i=1;i<nev;i++)printf(",%d",ez[i]);printf("]\n");fflush(stdout);}
      NLMInsertNonzeros(rowIndex,nCols,nev,ez,sym,n);

/* Merge ev into lv */

      if(verbose){printf("lv's: [%d",lv[0]);for(i=1;i<nlv;i++)printf(",%d",lv[i]);printf("]\n");fflush(stdout);}
      NLMMergeIntoArray(nev,ez,&nlv,&lv);
     }

/* (f'+a)(f'+a)^T */

    if(verbose){printf("Linear Element: [%d",lv[0]);for(i=1;i<nlv;i++)printf(",%d",lv[i]);printf("]\n");fflush(stdout);}
    NLMInsertNonzeros(rowIndex,nCols,nlv,lv,sym,n);
  
    if(lv!=(int*)NULL)free(lv);
   }

/* Now have a list of the nonzeros in the lower part of H */

  if(verbose)NLMPrintSparsityStructure(n,nCols,rowIndex);

  H->nE=0;
  for(i=0;i<n;i++)H->nE+=nCols[i];

  H->mE=H->nE;
  H->data=(double*)malloc((H->mE)*sizeof(double));
  if(H->data==(double*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }

  if(H->sparse==DUMBSPARSE)
   {
    if(verbose){printf(" type DUMBSPARSE\n");fflush(stdout);}
    H->row=(int*)realloc((void*)(H->row),(H->mE)*sizeof(int));
    if(H->row==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    H->col=(int*)realloc((void*)(H->col),(H->mE)*sizeof(int));
    if(H->col==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    t=0;
    for(i=0;i<n;i++)
     {
      for(j=0;j<nCols[i];j++)
       {
        H->row[t]=i;
        H->col[t]=(rowIndex[i])[j];
        if(verbose){printf("    %d (%d,%d)\n",t,H->row[t],H->col[t]);fflush(stdout);}
        t++;
       }
     }
   }else if(H->sparse==WSMPSPARSE)
   {
    H->row=(int*)realloc((void*)(H->row),(n+1)*sizeof(int));
    if(H->row==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    H->col=(int*)realloc((void*)(H->col),(H->mE)*sizeof(int));
    if(H->col==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }

    t=0;
    for(i=0;i<n+1;i++)
     {
      (H->row)[i]=t;
      if(i<n)t+=nCols[i];
     }

    t=0;
    for(i=0;i<n;i++)
     {
      for(j=0;j<nCols[i];j++)
       {
        (H->col)[t]=(rowIndex[i])[j];
        H->data[t]=0.;
        t++;
       }
     }
   }

  if(verbose){NLPrintMatrix(stdout,H);printf("Done %s\n",RoutineName);fflush(stdout);}

  free(nCols);
  free(rowIndex);

  NLDetermineSparsityTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLDetermineSparsityNCalls++;
  return;
 }

NLMatrix NLCreateWSMPSparseMatrix(int n)
 {
  char RoutineName[]="NLCreateWSMPSparseMatrix";
  int i;

  NLMatrix this=(NLMatrix)malloc(sizeof(struct NLGrpPartMat));
  if(this==(NLMatrix)NULL)
   {
    sprintf(NLMatrixErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartMat));
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    return((NLMatrix)NULL);
   }
  if(n<0)
   {
    sprintf(NLMatrixErrorMsg,"Number of rows %d (argument 1) is negative.",n);
    NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__);
    free(this);
    return((NLMatrix)NULL);
   }

  this->sparse=WSMPSPARSE;
  this->wrapped=0;
  this->nRows=n;
  this->nCols=n;
  this->nE=0;
  this->mE=-1;
  this->data=(double*)NULL;
  this->row=(int*)NULL;
  this->col=(int*)NULL;

  this->nRefs=1;

  return(this);
 }

void NLMInsertNonzeros(int **rows,int *nCols,int nnz,int *nz, int sym, int n)
 {
  char RoutineName[]="NLMInsertNonzeros";
  int i,j,I,J,nnew;
  int *newrow;
  int iz,jz,nJ,*rw;
  int verbose=0;

  if(nnz==0)return;

  if(verbose)
   {
    printf("Insert Nonzeroes, [%d",nz[0]);
    for(i=1;i<nnz;i++)printf(",%d",nz[i]);
    printf("]\n");fflush(stdout);
   }

  for(i=0;i<nnz;i++)
   {
    if(rows[nz[i]]==(int*)NULL)
     {
      nnew=0;
      for(j=0;j<nnz;j++)if(!sym||nz[j]>=nz[i])nnew++;
      nCols[nz[i]]=nnew;
 
      rows[nz[i]]=(int*)malloc(nnew*sizeof(int));
      if(rows[nz[i]]==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }

      J=0;
      for(j=0;j<nnz;j++)
       {
        if(!sym || nz[j]>=nz[i])
         {
          (rows[nz[i]])[J]=nz[j];
          J++;
         }
       }
     }else{
      nnew=0;
      j=0;
      J=0;
      iz=nz[i];
      nJ=nCols[iz];
      rw=rows[iz];
      while(j<nnz || J<nJ)
       {
        if(j<nnz)jz=nz[j];
        if(j<nnz && sym && jz<iz)j++;
         else if( (j>=nnz&&J<nJ) || (j<nnz&&J<nJ&&rw[J]<jz))J++;
         else if( (j<nnz&&J>=nJ) || (j<nnz&&J<nJ&&rw[J]>jz)){nnew++;j++;}
         else {j++;J++;}
       }
      newrow=(int*)malloc((nnew+nCols[nz[i]]+1)*sizeof(int));
      if(newrow==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
      j=0;
      J=0;
      I=0;
      while(j<nnz || J<nJ)
       {
        if(j<nnz)jz=nz[j];
        if(j<nnz && sym && jz<iz)j++;
         else if((j>=nnz&&J<nJ) || (j<nnz&&J<nJ&&rw[J]<jz)){newrow[I]=rw[J];I++;J++;}
         else if((j<nnz&&J>=nJ) || (j<nnz&&J<nJ&&rw[J]>jz)){newrow[I]=jz;I++;j++;}
         else {newrow[I]=nz[j];I++;j++;J++;}
       }
      free(rows[nz[i]]);
      rows[nz[i]]=newrow;
      nCols[nz[i]]=nCols[nz[i]]+nnew;
     }
   }

  return;
 }

void NLMMergeIntoArray(int na,int *a,int *nb,int **b)
 {
  static char RoutineName[]="NLMMergeIntoArray";
  int i,j,nnew,t;
  int *c;
  int nc;
  int verbose=0;
  int error;

  error=0;
  for(i=1;i<*nb;i++)if((*b)[i-1]>=(*b)[i])error=1;
  if(error)
   {
    printf("Error in %s, input array b is not correctly sorted\n",RoutineName);
    for(i=1;i<*nb;i++)if((*b)[i-1]>=(*b)[i])printf("   Entry %d>=%d\n",(*b)[i-1],(*b)[i]);
    printf("b: ");for(i=0;i<*nb;i++)printf("  %d",(*b)[i]);printf("\n");
    fflush(stdout);
    abort();
   }
  error=0;
  for(i=1;i<na;i++)if(a[i-1]>=a[i])error=1;
  if(error)
   {
    printf("Error in %s, input array a is not correctly sorted\n",RoutineName);
    for(i=1;i<na;i++)if(a[i-1]>=a[i])printf("   Entry %d>=%d\n",a[i-1],a[i]);
    printf("a: ");for(i=0;i<na;i++)printf("  %d",a[i]);printf("\n");
    fflush(stdout);
    abort();
   }

  if(*nb==0)
   {
    if(verbose){printf("%s, A:[%d",RoutineName,a[0]);for(i=1;i<na;i++)printf(",%d",a[i]);printf("]\n");
                printf("                   B: empty\n");
                fflush(stdout);}
    *nb=na;
    (*b)=(int*)realloc((void*)(*b),(*nb)*sizeof(int));
    if(*b==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<*nb;i++)(*b)[i]=a[i];
    return;
   }

  if(na==0)
   {
    if(verbose){printf("%s, A: empty\n");
                printf("                   B:[%d",(*b)[0]);for(i=1;i<*nb;i++){printf(",%d",(*b)[i]);if(i%20==19)printf("\n");}printf("]\n");
                fflush(stdout);}
    *nb=na;
    (*b)=(int*)realloc((void*)(*b),(*nb)*sizeof(int));
    if(*b==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<*nb;i++)(*b)[i]=a[i];
    return;
   }

  if(verbose){printf("%s, A:[%d",RoutineName,a[0]);for(i=1;i<na;i++)printf(",%d",a[i]);printf("]\n");
              printf("                   B:[%d",(*b)[0]);for(i=1;i<*nb;i++){printf(",%d",(*b)[i]);if(i%20==19)printf("\n");}printf("]\n");
              fflush(stdout);}

  if(a[0]>(*b)[(*nb)-1])
   {
    if(verbose){printf("Append A to B\n");fflush(stdout);}
    (*b)=(int*)realloc((void*)(*b),(*nb+na)*sizeof(int));
    if(*b==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<na;i++)(*b)[*nb+i]=a[i];
    *nb=*nb+na;
    if(verbose){printf(" Result            B:[%d",(*b)[0]);for(i=1;i<*nb;i++){printf(",%d",(*b)[i]);if(i%20==19)printf("\n");}printf("]\n");}
    return;
   }

  if(a[na-1]<(*b)[0])
   {
    if(verbose){printf("prepend A to B\n");fflush(stdout);}
    (*b)=(int*)realloc((void*)(*b),(*nb+na)*sizeof(int));
    if(*b==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }
    for(i=0;i<*nb;i++)(*b)[na+*nb-i-1]=(*b)[*nb-i-1];
    for(i=0;i<na;i++)(*b)[i]=a[i];
    *nb=*nb+na;
    if(verbose){printf(" Result            B:[%d",(*b)[0]);for(i=1;i<*nb;i++){printf(",%d",(*b)[i]);if(i%20==19)printf("\n");}printf("]\n");}
    return;
   }

  nnew=0;
  j=0;i=0;
  while(i<na||j<(*nb))
   {
    if( (i>=na&&j<(*nb)) || i<na&&j<(*nb)&&(*b)[j]<a[i])j++;
     else if((i<na&&j>=(*nb)) || i<na&&j<(*nb)&&(*b)[j]>a[i]){nnew++;i++;}
     else {i++;j++;}
    if(i<na&&j<(*nb))
     if((*b)[j]<a[i])       {           j++;}
     else if((*b)[j]>a[i])  {nnew++;i++;    }
     else                   {       i++;j++;}
    else if(i>=na&&j<(*nb)) {           j++;}
    else if(i<na&&j>=(*nb)) {nnew++;i++;    }
   }
  if(verbose){printf("   There are %d entries of A to be merged into B\n");fflush(stdout);}

  nc=(*nb)+nnew;
  c=(int*)malloc(nc*sizeof(int));
  if(c==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }

  j=0;i=0;t=0;
  while(i<na||j<(*nb))
   {
    if(verbose){printf("   i=%d/%d j=%d/%d\n",i,na,j,*nb);fflush(stdout);}
    if(i<na&&j<(*nb)&&verbose){printf("   A[%d]=%d, B[%d]=%d\n",i,a[i],j,(*b)[j]);fflush(stdout);}

    if(i<na&&j<(*nb))
     if((*b)[j]<a[i])       {c[t]=(*b)[j];    j++;t++;}
     else if((*b)[j]>a[i])  {c[t]=a[i];   i++;    t++;}
     else                   {c[t]=a[i];   i++;j++;t++;}
    else if(i>=na&&j<(*nb)) {c[t]=(*b)[j];    j++;t++;}
    else if(i<na&&j>=(*nb)) {c[t]=a[i];   i++;    t++;}
   }

  (*nb)=nc;
  (*b)=(int*)realloc((void*)(*b),(*nb)*sizeof(int));
  if(*b==(int*)NULL){ sprintf(NLMatrixErrorMsg,"Out of memory");NLSetError(12,RoutineName,NLMatrixErrorMsg,__LINE__,__FILE__); return; }

  for(i=0;i<nc;i++)(*b)[i]=c[i];

  if(verbose){printf(" Result            B:[%d",(*b)[0]);for(i=1;i<*nb;i++){printf(",%d",(*b)[i]);if(i%20==19)printf("\n");}printf("]\n");}

  error=0;
  for(i=1;i<nc;i++)if(c[i-1]>=c[i])error=1;
  if(error)
   {
    printf("Error in %s, result arrcy c is not correctly sorted\n",RoutineName);
    for(i=1;i<nc;i++)if(c[i-1]>=c[i])printf("   Entry %d>=%d\n",c[i-1],c[i]);
    printf("c: ");for(i=0;i<nc;i++)printf("  %d",c[i]);printf("\n");
    fflush(stdout);
    abort();
   }

  free(c);

  return;
 }
