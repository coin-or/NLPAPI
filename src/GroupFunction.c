/*  (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 11/11/1997.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory*/

/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: @(#)GroupFunction.c	3.2 02/08/02 14:52:05 */
/*      date:   November 11, 1997                     */
/*              February 2, 1999   Ported to C        */
/*              September 26, 2001 Added support for the polynomial order */
/*              October 3, 2001    Added differencing */

#include <NLPAPI.h>
#include <ExpCmp.h>
#include <time.h>

void NLSetError(int,char*,char*,int,char*);
static char NLGFuncErrorMsg[256]="";

typedef double (*NLScalarFn)(double,void*);

struct NLGrpPartGFn
 {
  double (*G)(double,void*);
  double (*dG)(double,void*);
  double (*ddG)(double,void*);
  int polynomialOrder;
/* Scale for G, for dG, and for ddG */
  void *data;
  void (*freeData)(void*);
  int type;

  char *expr;
  char *var;
  ECFn sG;
  ECFn sdG;
  ECFn sddG;

  int nRefs;
 };

double NLEvaluateGroupFunctionTime=0.;
int NLEvaluateGroupFunctionNCalls=0;
double NLEvaluateGroupFunctionDerTime=0.;
int NLEvaluateGroupFunctionDerNCalls=0;
double NLEvaluateGroupFunctionSecDerTime=0.;
int NLEvaluateGroupFunctionSecDerNCalls=0;

void NLRefGroupFunction(NLGroupFunction this)
 {
  static char RoutineName[]="NLRefGroupFunction";
#ifndef NL_NOINPUTCHECKS
  if(this==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
#endif
  this->nRefs++;
 }

void NLFreeGroupFunction(NLGroupFunction this)
 {
  static char RoutineName[]="NLFreeGroupFunction";
#ifndef NL_NOINPUTCHECKS
  if(this==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
#endif

  this->nRefs--;

  if(this->nRefs<1)
   {
    if(this->freeData!=(void (*)(void*))NULL)this->freeData(this->data);
    free(this);
   }
  return;
 }

NLGroupFunction NLCreateGroupFunction(NLProblem P, char *type, double (*G)(double,void*),double (*dG)(double,void*),double (*ddG)(double,void*),void *data,void (*freeData)(void*))
 {
  static char RoutineName[]="NLCreateGroupFunction";
  NLGroupFunction result;
  static int verbose=0;

  result=(NLGroupFunction)malloc(sizeof(struct NLGrpPartGFn));
  if(result==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartGFn));
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
  result->type=NLPAddGroupType(P,type);
  result->G=G;
  result->dG=dG;
  result->ddG=ddG;
  result->var=(char*)NULL;
  result->expr=(char*)NULL;
  result->sG=(ECFn)NULL;
  result->sdG=(ECFn)NULL;
  result->sddG=(ECFn)NULL;
  result->polynomialOrder=NLVARIABLEDEPENDENCENOTSET;
  result->data=data;
  result->freeData=freeData;
  result->nRefs=1;
  if(verbose){printf("%s(type %d=\"%s\")\n",RoutineName,result->type,type);fflush(stdout);}

  return(result);
 }

double NLGEval(NLGroupFunction G,double x, void *data)
 {
  static char RoutineName[]="NLGEval";
  clock_t tin;
  double result;

  tin=clock();

#ifndef NL_NOINPUTCHECKS
  if(G==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
  if(G->G==(NLScalarFn)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function function is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
#endif

  if(G->G!=(NLScalarFn)NULL)
   {
    result=G->G(x,data);
   }else{
    ECEvaluateFunction(G->sG,&x,&result);
   }

  NLEvaluateGroupFunctionTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateGroupFunctionNCalls++;

  return result;
 }

double NLGEvalDer(NLGroupFunction G,double x, void *data)
 {
  static char RoutineName[]="NLGEvalDer";
  static double gplus,gminus;
  clock_t tin;
  double result;

  tin=clock();

#ifndef NL_NOINPUTCHECKS
  if(G==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
#endif

  if(G->G!=(NLScalarFn)NULL)
   {
    if(G->dG==(NLScalarFn)NULL)
     {
      gplus=NLGEval(G,x+.5e-5,data);
      gminus=NLGEval(G,x-.5e-5,data);
      result=(gplus-gminus)*1.e5;
     }else{
      result=G->dG(x,data);
     }
   }else{
    ECEvaluateFunction(G->sdG,&x,&result);
   }
   
  NLEvaluateGroupFunctionDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateGroupFunctionDerNCalls++;
  return result;
 }

double NLGEvalSecDer(NLGroupFunction G,double x, void *data)
 {
  static char RoutineName[]="NLGEvalSecDer";
  static double gplus,gzero,gminus;
  clock_t tin;
  double result;

  tin=clock();

#ifndef NL_NOINPUTCHECKS
  if(G==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
#endif
  if(G->G!=(NLScalarFn)NULL)
   {
    if(G->ddG==(NLScalarFn)NULL)
     {
      if(G->dG==(NLScalarFn)NULL)
       {
        gplus=NLGEval(G,x+1.e-5,data);
        gzero=NLGEval(G,x,data);
        gminus=NLGEval(G,x-1.e-5,data);
        result=(gplus-2*gzero+gminus)*1.e5;
       }else{
        gplus=NLGEvalDer(G,x+.5e-5,data);
        gminus=NLGEvalDer(G,x-.5e-5,data);
        result=(gplus-gminus)*1.e5;
       }
     }else{
      result=G->ddG(x,data);
     }
   }else{
    ECEvaluateFunction(G->sddG,&x,&result);
   }

  NLEvaluateGroupFunctionSecDerTime+=(clock()-tin)*1./CLOCKS_PER_SEC;
  NLEvaluateGroupFunctionSecDerNCalls++;

  return result;
 }

int NLGGetType(NLGroupFunction G)
 {
  static char RoutineName[]="NLGGetType";

#ifndef NL_NOINPUTCHECKS
  if(G==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
#endif

  return(G->type);
 }

int NLGFAssertPolynomialOrder(NLGroupFunction this,int p)
 {
  static char RoutineName[]="NLGFAssertPolynomialOrder";

#ifndef NL_NOINPUTCHECKS
  if(this==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Group Function (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return 12;
   }
#endif

  this->polynomialOrder=p;
  return 0;
 }

int NLGFQueryPolynomialOrder(NLGroupFunction this)
 {
  static char RoutineName[]="NLGFQueryPolynomialOrder";

  if(this==(NLGroupFunction)NULL)return 1;
   else return this->polynomialOrder;
 }

NLGroupFunction NLCreateGroupFunctionByString(NLProblem P, char *type, char *var, char *expr)
 {
  static char RoutineName[]="NLCreateGroupFunctionByString";
  NLGroupFunction result;
  static int verbose=0;
  int n;

#ifndef NL_NOINPUTCHECKS
  if(P==(NLProblem)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }

  if(type==(char*)NULL)
   {
    sprintf(NLGFuncErrorMsg,"type (argument 2) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }

  if(var==(char*)NULL)
   {
    sprintf(NLGFuncErrorMsg,"var (argument 3) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }

  if(expr==(char*)NULL)
   {
    sprintf(NLGFuncErrorMsg,"expr (argument 4) is NULL");
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }
#endif

  result=(NLGroupFunction)malloc(sizeof(struct NLGrpPartGFn));
  if(result==(NLGroupFunction)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartGFn));
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
   }
  result->type=NLPAddGroupType(P,type);
  result->G=(NLScalarFn)NULL;
  result->dG=(NLScalarFn)NULL;
  result->ddG=(NLScalarFn)NULL;

  result->sG=ECCreateFunction(var,expr);

  result->expr=(char*)malloc((strlen(expr)+1)*sizeof(char));
  if(result->expr==(char*)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(expr)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }
  strcpy(result->expr,expr);

  n=ECFunctionM(result->sG);
#ifndef NL_NOINPUTCHECKS
  if(ECFunctionN(result->sG)!=1||n!=1)
   {
    sprintf(NLGFuncErrorMsg,"String %s is not a scalar valued function of a scalar.",expr);
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }
#endif

  result->var=(char*)malloc((strlen(var)+1)*sizeof(char));
  if(result->var==(char*)NULL)
   {
    sprintf(NLGFuncErrorMsg,"Out of memory, trying to allocate %d bytes",(strlen(var)+1)*sizeof(char));
    NLSetError(12,RoutineName,NLGFuncErrorMsg,__LINE__,__FILE__);
    return (NLGroupFunction)NULL;
   }
  strcpy(result->var,var);

  result->sdG=ECCreateDerivativeOfFunction(result->sG,0);
  result->sddG=ECCreateDerivativeOfFunction(result->sdG,0);
  result->polynomialOrder=NLVARIABLEDEPENDENCENOTSET;
  result->data=(void*)NULL;
  result->freeData=(void (*)(void*))NULL;
  result->nRefs=1;
  if(verbose){printf("%s(type %d=\"%s\")\n",RoutineName,result->type,type);fflush(stdout);}

  return(result);
 }
