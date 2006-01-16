/*
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory
*/
/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: @(#)Vector.c	3.5 02/12/04 09:43:22 */
/*      date:   November 11, 1997                     */
/*              February 2, 1999   Ported to C        */
/*              September 15, 2000 Added IncrementC   */
/*              September 28, 2001 Added Dense and Wrapped   */

#include <NLPAPI.h>

void NLSetError(int,char*,char*,int,char*);

struct NLGrpPartVec
 {
  int nC;
  int sparse;
  int wrapped;
  int nNonZeros;
  int mNonZeros;
  int *nonZero;
  double *data;
  int nRefs;
 };
void NLPrintVector(FILE*,NLVector);

static char NLVectorErrorMsg[80]="";

NLVector NLCreateVector(int n)
 {
  static char RoutineName[]="NLCreateVector";
  NLVector this;
#ifndef NL_NOINPUTCHECKS
  if(n<1)
   {
    sprintf(NLVectorErrorMsg,"Length of Vector %d (argument 1) is Illegal. Must be positive.",n);
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return((NLVector)NULL);
   }
#endif

  this=(NLVector)malloc(sizeof(struct NLGrpPartVec));
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartVec));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }
  this->nC=n;
  this->sparse=1;
  this->wrapped=0;

  this->nNonZeros=0;
  this->mNonZeros=0;
  this->nonZero=(int*)NULL;
  this->data=(double*)NULL;
  this->nRefs=1;

  return(this);
 }

NLVector NLCreateVectorWithSparseData(int n,int nz,int *el,double *vl)
 {
  static char RoutineName[]="NLCreateVectorWithSparseData";
  int i;
  NLVector this;

#ifndef NL_NOINPUTCHECKS
  if(n<1)
   {
    sprintf(NLVectorErrorMsg,"Length of Vector %d (argument 1) is Illegal. Must be positive.",n);
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return((NLVector)NULL);
   }

  if(nz<0)
   {
    sprintf(NLVectorErrorMsg,"Number of nonzeros in vector %d (argument 2) is Illegal. Must be nonnegative.",nz);
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return((NLVector)NULL);
   }
#endif

  this=(NLVector)malloc(sizeof(struct NLGrpPartVec));
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartVec));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }

  if(nz>0)
   {
    this->nNonZeros=nz;
    this->mNonZeros=nz;
    this->nonZero=(int*)malloc(nz*sizeof(int));
    if(this->nonZero==(int*)NULL)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",nz*sizeof(int));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
    this->data=(double*)malloc(nz*sizeof(double));
    if(this->data==(double*)NULL)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",nz*sizeof(double));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
#ifndef NL_NOINPUTCHECKS
    if(el==(int*)NULL)
     {
      sprintf(NLVectorErrorMsg,"The pointer to the array of nonZeros (argument 3) is NULL");
      NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      this->nNonZeros=0;
      this->mNonZeros=0;
      this->nonZero=(int*)NULL;
      this->data=(double*)NULL;
      this->nRefs=1;
      return(this);
     }
    if(vl==(double*)NULL)
     {
      sprintf(NLVectorErrorMsg,"The pointer to the array of coordinates (argument 4) is NULL");
      NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      this->nNonZeros=0;
      this->mNonZeros=0;
      this->nonZero=(int*)NULL;
      this->data=(double*)NULL;
      this->nRefs=1;
      return(this);
     }
#endif
    for(i=0;i<nz;i++)
     {
      this->nonZero[i]=el[i];
      this->data[i]=vl[i];
     }
   }else{
    this->nNonZeros=0;
    this->mNonZeros=0;
    this->nonZero=(int*)NULL;
    this->data=(double*)NULL;
   }
  this->sparse=1;
  this->wrapped=0;

  this->nRefs=1;
  return(this);
 }

NLVector NLCreateVectorWithFullData(int n,double *vl)
 {
  static char RoutineName[]="NLCreateVectorWithFullData";
  int i,j;
  NLVector this;

#ifndef NL_NOINPUTCHECKS
  if(n<1)
   {
    sprintf(NLVectorErrorMsg,"Length of Vector %d (argument 1) is Illegal. Must be positive.",n);
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return((NLVector)NULL);
   }
#endif

  this=(NLVector)malloc(sizeof(struct NLGrpPartVec));
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartVec));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }
  this->nC=n;

  if(n>0)
   {
#ifndef NL_NOINPUTCHECKS
    if(vl==(double*)NULL)
     {
      sprintf(NLVectorErrorMsg,"The pointer to the array of coordinates (argument 2) is NULL");
      NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      this->nNonZeros=0;
      this->mNonZeros=0;
      this->nonZero=(int*)NULL;
      this->data=(double*)NULL;
      this->nRefs=1;
      return(this);
     }
#endif

    this->nNonZeros=0;
    this->mNonZeros=0;
    for(i=0;i<n;i++)
     {
      if(fabs(vl[i])>1.e-30)
       {
        this->mNonZeros++;
       }
     }

    this->nonZero=(int*)malloc(this->mNonZeros*sizeof(int));
    if(this->nonZero==(int*)NULL)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(int));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
    this->data=(double*)malloc(this->mNonZeros*sizeof(double));
    if(this->data==(double*)NULL)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(double));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
    for(i=0;i<n;i++)
     {
      if(fabs(vl[i])>1.e-30)
       {
        this->nonZero[this->nNonZeros]=i;
        this->data[this->nNonZeros]=vl[i];
        this->nNonZeros++;
       }
     }
   }else{
    this->nNonZeros=0;
    this->mNonZeros=0;
    this->nonZero=(int*)NULL;
    this->data=(double*)NULL;
   }
  this->sparse=1;
  this->wrapped=0;

  this->nRefs=1;
  return(this);
 }

void NLFreeVector(NLVector this)
 {
  static char RoutineName[]="NLFreeVector";
#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  this->nRefs--;

  if(this->nRefs<1)
   {
    if(!this->wrapped&&this->data!=(double*)NULL)free(this->data);
    if(this->nonZero!=(int*)NULL)free(this->nonZero);
    if(this!=(NLVector)NULL)free(this);
   }
  return;
 }

int NLVGetNC(NLVector this)
 {
  static char RoutineName[]="NLVGetNC";
#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return -1;
   }
#endif

  return(this->nC);
 }

double NLVGetC(NLVector this,int i)
 {
  static char RoutineName[]="NLVGetC";
  int j;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(i<0|| !(i<this->nC))
   {
    sprintf(NLVectorErrorMsg,"Coordinate %d (argument 2) is illegal. Must be in 0 to %d",i,this->nC-1);
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

  if(this->sparse)
   {
    for(j=0;j<this->nNonZeros;j++)
     {
      if(this->nonZero[j]==i)
       return(this->data[j]);
     }
    return(0.);
   }else return this->data[i];
 }

int NLVSetC(NLVector this,int i,double vl)
 {
  static char RoutineName[]="NLVSetC";
  int rc;
  int j;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  if(i<0|| !(i<this->nC))
   {
    sprintf(NLVectorErrorMsg,"Coordinate %d (argument 2) is illegal. Must be in 0 to %d",i,this->nC-1);
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
#endif

  if(!(this->sparse))
   {
    this->data[i]=vl;
    return 1;
   }

  for(j=0;j<this->nNonZeros;j++)
   {
    if(this->nonZero[j]==i)
     {
      this->data[j]=vl;
      return 1;
     }
   }
  if(fabs(vl)<1.e-30)return 1;
  if(this->nNonZeros>=this->mNonZeros)
   {
    if(this->mNonZeros==0)
     {
      this->mNonZeros=10;
      this->nonZero=(int*)malloc((this->mNonZeros)*sizeof(int));
      if(this->nonZero==(int*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(int));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros=0;
        return 0;
       }
      this->data=(double*)malloc((this->mNonZeros)*sizeof(double));
      if(this->data==(double*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(double));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros=0;
        free(this->nonZero);
        this->nonZero=(int*)NULL;
        return 0;
       }
     }else{
      this->mNonZeros+=10;
      this->nonZero=(int*)realloc((void*)this->nonZero,(this->mNonZeros)*sizeof(int));
      if(this->nonZero==(int*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(int));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros-=10;
        return 0;
       }
      this->data=(double*)realloc((void*)this->data,(this->mNonZeros)*sizeof(double));
      if(this->data==(double*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(double));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros-=10;
        free(this->nonZero);
        this->nonZero=(int*)NULL;
        return 0;
       }
     }
   }
  this->nonZero[this->nNonZeros]=i;
  this->data[this->nNonZeros]=vl;
  (this->nNonZeros)++;
  return 1;
 }

void NLRefVector(NLVector this)
 {
  static char RoutineName[]="NLRefVector";
#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  (this->nRefs)++;
  return;
 }

int NLVGetNumberOfNonZeros(NLVector v)
 {
  static char RoutineName[]="NLVGetNumberOfNonZeros";

#ifndef NL_NOINPUTCHECKS
  if(v==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return -1;
   }
#endif

  if(v->sparse)
    return(v->nNonZeros);
   else
    return v->nC;
 }

int NLVGetNonZeroCoord(NLVector v,int n)
 {
  static char RoutineName[]="NLVGetNonZeroCoord";
#ifndef NL_NOINPUTCHECKS
  if(v==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return -1;
   }
#endif
  if(v->sparse)
   {
#ifndef NL_NOINPUTCHECKS
    if(n<0|| !(n<v->nNonZeros))
     {
      sprintf(NLVectorErrorMsg,"NonZero Coordinate %d (argument 2) is illegal. Must be in 0 to %d",n,v->nNonZeros-1);
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return -1;
     }
#endif
   }else{
#ifndef NL_NOINPUTCHECKS
    if(n<0|| !(n<v->nC))
     {
      sprintf(NLVectorErrorMsg,"NonZero Coordinate %d (argument 2) is illegal. Must be in 0 to %d",n,v->nC-1);
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return -1;
     }
#endif
   }

  if(v->sparse)
    return((v->nonZero)[n]);
   else
    return((v->data)[n]);
 }

double NLVGetNonZero(NLVector v,int n)
 {
  static char RoutineName[]="NLVGetNonZero";
#ifndef NL_NOINPUTCHECKS
  if(v==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
  if(n<0|| !(n<v->nNonZeros))
   {
    sprintf(NLVectorErrorMsg,"NonZero Coordinate %d (argument 2) is illegal. Must be in 0 to %d",n,v->nNonZeros-1);
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return DBL_QNAN;
   }
#endif

  return((v->data)[n]);
 }

void NLPrintVector(FILE *fid, NLVector v)
 {
  static char RoutineName[]="NLPrintVector";
  int i;

#ifndef NL_NOINPUTCHECKS
  if(fid==(FILE*)NULL)
   {
    sprintf(NLVectorErrorMsg,"File identifier (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(v==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  fprintf(fid," [",v->nC);fflush(stdout);
  for(i=0;i<v->nC;i++)
   {
    if(i>0)fprintf(fid,",");
    fprintf(fid,"%lf",NLVGetC(v,i));
   }
  fprintf(fid,"]");
  return;
 }

int NLVIncrementC(NLVector this,int i,double vl)
 {
  static char RoutineName[]="NLVSetC";
  int rc;
  int j=0;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
  if(i<0|| !(i<this->nC))
   {
    sprintf(NLVectorErrorMsg,"Coordinate %d (argument 2) is illegal. Must be in 0 to %d",i,this->nC-1);
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
#endif

  if(!(this->sparse))
   {
    this->data[j]+=vl;
    return 1;
   }

  for(j=0;j<this->nNonZeros;j++)
   {
    if(this->nonZero[j]==i)
     {
      this->data[j]+=vl;
      return 1;
     }
   }
  if(fabs(vl)<1.e-30)return 1;
  if(this->nNonZeros>=this->mNonZeros)
   {
    if(this->mNonZeros==0)
     {
      this->mNonZeros=10;
      this->nonZero=(int*)malloc((this->mNonZeros)*sizeof(int));
      if(this->nonZero==(int*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(int));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros=0;
        return 0;
       }
      this->data=(double*)malloc((this->mNonZeros)*sizeof(double));
      if(this->data==(double*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(double));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros=0;
        free(this->nonZero);
        this->nonZero=(int*)NULL;
        return 0;
       }
     }else{
      this->mNonZeros+=10;
      this->nonZero=(int*)realloc((void*)this->nonZero,(this->mNonZeros)*sizeof(int));
      if(this->nonZero==(int*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(int));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros-=10;
        return 0;
       }
      this->data=(double*)realloc((void*)this->data,(this->mNonZeros)*sizeof(double));
      if(this->data==(double*)NULL)
       {
        sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",(this->mNonZeros)*sizeof(double));
        NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
        this->mNonZeros-=10;
        free(this->nonZero);
        this->nonZero=(int*)NULL;
        return 0;
       }
     }
   }
  this->nonZero[this->nNonZeros]=i;
  this->data[this->nNonZeros]=vl;
  (this->nNonZeros)++;
  return 1;
 }

double NLVInnerProd(NLVector u, NLVector v)
 {
  double p;
  int i,j;
  int verbose=0;

  if(verbose){printf("<");NLPrintVector(stdout,u);
              printf(",");NLPrintVector(stdout,v);
              printf(">=");fflush(stdout);}

/* Andreas' performance improvement */

  p=0.;
  if( u->wrapped && v->wrapped )
   {
    for(i=0;i<u->nC;i++)
     p+=u->data[i]*v->data[i];
   }
  else if( u->wrapped && v->sparse )
   {
    for(i=0;i<v->nNonZeros;i++)
     p+=(u->data[v->nonZero[i]])*(v->data[i]);
   }
  else if( u->sparse && v->wrapped )
   {
    for(i=0;i<u->nNonZeros;i++)
     p+=(v->data[u->nonZero[i]])*(u->data[i]);
   }
  else
   {
    for(i=0;i<u->nNonZeros;i++)
     for(j=0;j<v->nNonZeros;j++)
      if(u->nonZero[i]==v->nonZero[j])
       p+=u->data[i]*v->data[j];
   }

  if(verbose){printf("%lf\n",p);fflush(stdout);}
  return p;
 }

double NLVInnerFull(NLVector u, double *v)
 {
  double p;
  int i;

  p=0.;
  for(i=0;i<u->nC;i++)
   {
    p+=NLVGetC(u,i)*v[i];
   }
  return p;
 }

void NLVectorIncreaseLength(NLVector this,int n)
 {
  static char RoutineName[]="NLVectorIncreaseLength";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  if(!this->sparse && this->wrapped)
   {
    sprintf(NLVectorErrorMsg,"Cannot increase the length of a wrapped vector");
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->nC+=n;
  if(!(this->sparse)&&!(this->wrapped))
    this->data=(double*)realloc(this->data,(this->nC)*sizeof(double));

  return;
 }

void NLVectorDecreaseLength(NLVector this,int n)
 {
  static char RoutineName[]="NLVectorDecreaseLength";
  int i;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  if(!this->sparse && this->wrapped)
   {
    sprintf(NLVectorErrorMsg,"Cannot increase the length of a wrapped vector");
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->nC-=n;
  if(this->sparse)
   {
    for(i=0;i<this->nNonZeros;i++)
     {
      if(this->nonZero[i]>=this->nC)
       {
        if(this->nNonZeros-1>i)
         {
          while(this->nonZero[this->nNonZeros-1]>=this->nC && this->nNonZeros>0)
           {
            this->nNonZeros--;
            this->nonZero[i]=this->nonZero[this->nNonZeros];
            this->data[i]=this->data[this->nNonZeros];
           }
         }else this->nNonZeros--;
       }
     }
   }

  return;
 }

NLVector NLCreateDenseWrappedVector(int n,double *data)
 {
  static char RoutineName[]="NLCreateDenseWrappedVector";
  NLVector this;

#ifndef NL_NOINPUTCHECKS
  if(n<1)
   {
    sprintf(NLVectorErrorMsg,"Length of Vector %d (argument 1) is Illegal. Must be positive.",n);
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return((NLVector)NULL);
   }
#endif

  this=(NLVector)malloc(sizeof(struct NLGrpPartVec));
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartVec));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }
  this->nC=n;
  this->sparse=0;
  this->wrapped=1;

  this->nNonZeros=n;
  this->mNonZeros=0;
  this->nonZero=(int*)NULL;
  this->data=data;
  this->nRefs=1;

  return this;
 }

NLVector NLCreateDenseVector(int n)
 {
  static char RoutineName[]="NLCreateDenseVector";
  NLVector this;

#ifndef NL_NOINPUTCHECKS
  if(n<1)
   {
    sprintf(NLVectorErrorMsg,"Length of Vector %d (argument 1) is Illegal. Must be positive.",n);
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return((NLVector)NULL);
   }
#endif

  this=(NLVector)malloc(sizeof(struct NLGrpPartVec));
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartVec));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }
  this->nC=n;
  this->sparse=1;
  this->wrapped=0;

  this->nNonZeros=n;
  this->mNonZeros=0;
  this->nonZero=(int*)NULL;
  this->data=(double*)malloc(n*sizeof(double));
  if(this->data==(double*)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(double));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }
  this->nRefs=1;

  return this;
 }

int NLVSetToZero(NLVector this)
 {
  static char RoutineName[]="NLVSetToZero";
  int i;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
#endif

  if(!(this->sparse))
   {
    for(i=0;i<this->nC;i++)this->data[i]=0.;
    return 1;
   }

  this->nNonZeros=0;

  return 1;
 }

int NLZeroVector(NLVector this)
 {
  static char RoutineName[]="NLZeroVector";
  int i;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
#endif

  if(!this->sparse)
   {
    memset((void*)(this->data),0,this->nC*sizeof(double));
   }else{
    memset((void*)(this->data),0,this->nNonZeros*sizeof(double));
   }

  return 1;
 }

int NLNegateVector(NLVector this)
 {
  static char RoutineName[]="NLNegateVector";
  int i;

#ifndef NL_NOINPUTCHECKS
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return 0;
   }
#endif

  if(!this->sparse)
   {
    for(i=0;i<this->nC;i++)(this->data)[i]=-(this->data)[i];
   }else{
    for(i=0;i<this->nNonZeros;i++)(this->data)[i]=-(this->data)[i];
   }

  return 1;
 }

int NLVSparse(NLVector this)
 {
  return this->sparse;
 }

int NLVWrapped(NLVector this)
 {
  return this->wrapped;
 }

int NLVnNonZeros(NLVector this)
 {
  return this->nNonZeros;
 }

int *NLVnonZero(NLVector this)
 {
  return this->nonZero;
 }

double *NLVData(NLVector this)
 {
  return this->data;
 }

NLVector NLCopyVector(NLVector that)
 {
  static char RoutineName[]="NLCopyVector";
  NLVector this;
  int i;

#ifndef NL_NOINPUTCHECKS
  if(that==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Argument 1, the vector to copy, is NULL");
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return (NLVector)NULL;
   }
#endif

  this=(NLVector)malloc(sizeof(struct NLGrpPartVec));
  if(this==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartVec));
    NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return(this);
   }
  this->nC=that->nC;
  this->sparse=that->sparse;
  this->wrapped=that->wrapped;

  this->nNonZeros=that->nNonZeros;
  this->mNonZeros=that->mNonZeros;
  if(this->wrapped)
   {
    this->data=that->data;
   }else if(this->sparse)
   {
    this->nNonZeros=that->nNonZeros;
    this->mNonZeros=that->mNonZeros;
    this->nonZero=(int*)malloc(this->mNonZeros*sizeof(int));
    if(this->nNonZeros>0 && this->nonZero==(int*)NULL && this->mNonZeros>0)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonZeros*sizeof(int));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
    this->data=(double*)malloc(this->mNonZeros*sizeof(double));
    if(this->data==(double*)NULL && this->mNonZeros>0)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",this->mNonZeros*sizeof(double));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
    for(i=0;i<this->nNonZeros;i++)
     {
      this->nonZero[i]=that->nonZero[i];
      this->data[i]=that->data[i];
     }
   }else{
    this->nNonZeros=that->nC;
    this->mNonZeros=that->nC;
    this->nonZero=(int*)NULL;
    this->data=(double*)malloc(this->nC*sizeof(double));
    if(this->data==(double*)NULL && this->nC>0)
     {
      sprintf(NLVectorErrorMsg,"Out of memory, trying to allocate %d bytes",this->nC*sizeof(double));
      NLSetError(12,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
      return(this);
     }
    for(i=0;i<this->nC;i++)this->data[i]=that->data[i];
   }

  this->nRefs=1;

  return(this);
 }

/* Andreas' fix for speed */

void NLVPlusV(NLVector u, NLVector v, double a)
 {
  /* computes u = u + a * v */
  static char RoutineName[]="NLVPlusV";
  int i;
  double tmp;

#ifndef NL_NOINPUTCHECKS
  if(u==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to first Vector (argument 1) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(v==(NLVector)NULL)
   {
    sprintf(NLVectorErrorMsg,"Pointer to second Vector (argument 2) is NULL");
    NLSetError(4,RoutineName,NLVectorErrorMsg,__LINE__,__FILE__);
    return;
   }
#endif

  if( u->wrapped && v->wrapped )
   {
    for(i=0;i<u->nC;i++) u->data[i]+=a*v->data[i];
   }
  else if( u->wrapped && v->sparse )
   {
    for(i=0;i<v->nNonZeros;i++) u->data[v->nonZero[i]]+=a*v->data[i];
   }
  else if( u->sparse && v->wrapped )
   {
    for(i=0;i<v->nC;i++)
     {
      tmp=v->data[i];
      if(tmp>1.e-30)
       NLVSetC(u,i,NLVGetC(u,i)+a*tmp);
     }
   }
  else if( u->sparse && v->sparse )
   {
    for(i=0;i<v->nNonZeros;i++)
     NLVSetC(u,v->nonZero[i],NLVGetC(u,v->nonZero[i])+a*v->data[i]);
   }
 }
