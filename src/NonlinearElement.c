/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: %W% %D% %T% */
/*      date:   May 4, 2000                           */

#include <NLPAPI.h>

void NLSetError(int,char*,char*,int,char*);
static char NLNEErrorMsg[256]="";

struct NLGrpPartNEl
 {
  NLElementFunction F;
  char *name;
  int nElementVariables;
  int nInternalVariables;
  int *ElVars;
  void *data;
  void (*freeData)(void*);
  int nRefs;
 };

int *NLNEGetElementVariables(NLProblem P, NLNonlinearElement t)
 {
  char RoutineName[]="NLNEGetElementVariables";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 2) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return (int*)NULL;
   }
  this=NLPGetNonlinearElement(P,t);

  return(this->ElVars);
 }

NLElementFunction NLNEGetElementFunction(NLProblem P, NLNonlinearElement t)
 {
  char RoutineName[]="NLNEGetElementFunction";
  NLNonlinearElementPtr this;

  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is invalid 0<%d<%d",t,NLPGetNumberOfNonlinearElements(P));
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return (NLElementFunction)NULL;
   }

  this=NLPGetNonlinearElement(P,t);

  return(this->F);
 }

void NLRefNonlinearElement(NLNonlinearElementPtr this)
 {
  char RoutineName[]="NLRefNonlinearElement";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  this->nRefs++;

  return;
 }

int NLFreeNonlinearElement(NLProblem P, NLNonlinearElement t)
 {
  char RoutineName[]="NLFreeNonlinearElement";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element %d (argument 1) is invalid. Must be in [0,%d)",t,NLPGetNumberOfNonlinearElements(P));
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  this=NLPGetNonlinearElement(P,t);

  this->nRefs--;

  if(this->nRefs<1)
   {
    if(this->F!=(NLElementFunction)NULL)NLFreeElementFunction(this->F);
    if(this->name!=(char*)NULL)free(this->name);
    if(this->ElVars!=(int*)NULL)free(this->ElVars);
    if(this->freeData!=NULL&&this->data!=(void*)NULL)this->freeData(this->data);
    free(this);
    return 1;
   }

  return 0;
 }

NLNonlinearElement NLCreateNonlinearElement(NLProblem P, char *name,NLElementFunction F,int *vars)
 {
  return NLCreateNonlinearElementParm(P,name,F,vars,NLEGetData(F),NULL);
 }

void *NNLEGetData(NLNonlinearElementPtr this)
 {
  return this->data;
 }

NLNonlinearElement NLCreateNonlinearElementParm(NLProblem P, char *name,NLElementFunction F,int *vars,void *data, void (*freeData)(void*))
 {
  char RoutineName[]="NLCreateNonlinearElement";
  NLNonlinearElementPtr result;
  NLMatrix R;
  int i,n;

  int verbose;

  verbose=0;
  if(verbose){printf("%s, F=0x%8.8x\n",RoutineName,F);fflush(stdout);}

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  if(F==(NLElementFunction)NULL)
   {
    sprintf(NLNEErrorMsg,"Element Function (argument 3) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  result=(NLNonlinearElementPtr)malloc(sizeof(struct NLGrpPartNEl));
  if(result==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Out of memory, trying to allocate %d bytes",sizeof(struct NLGrpPartNEl));
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return (NLNonlinearElement)NULL;
   }

  result->name=(char*)malloc((strlen(name)+1)*sizeof(char));
  strcpy(result->name,name);
  result->F=F;NLRefElementFunction(F);
  R=NLEGetRangeXForm(F);
  if(R!=(NLMatrix)NULL)
   {
    result->nInternalVariables=NLEGetDimension(F);
    result->nElementVariables=NLMGetNumberOfCols(R);
    if(result->nInternalVariables!=NLMGetNumberOfRows(R))
     {
      sprintf(NLNEErrorMsg,"Shape of range transform does not match the element function. Xform is %dx%d, domain of EF is %d",NLMGetNumberOfRows(R),NLMGetNumberOfCols(R),NLEGetDimension(F));
      NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
      return (NLNonlinearElement)NULL;
     }
   }else{
    result->nInternalVariables=NLEGetDimension(F);
    result->nElementVariables=result->nInternalVariables;
   }
  n=result->nElementVariables;
  result->ElVars=(int*)malloc(n*sizeof(int));
  if(result->ElVars==(int*)NULL)
   {
    sprintf(NLNEErrorMsg,"Out of memory, trying to allocate %d bytes",n*sizeof(int));
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return (NLNonlinearElement)NULL;
   }
  for(i=0;i<n;i++)result->ElVars[i]=vars[i];
  result->nRefs=1;
  result->data=data;
  result->freeData=freeData;

  return NLPAddNonlinearElement(P,result);
 }

int NLNEGetIndex(NLProblem P,NLNonlinearElement t,int var)
 {
  char RoutineName[]="NLNEGetIndex";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 2) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  this=NLPGetNonlinearElement(P,t);
  if(var<0||var>=this->nElementVariables)
   {
    sprintf(NLNEErrorMsg,"Variable (argument 3) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  return this->ElVars[var];
 }

int NLNEGetInternalDimension(NLProblem P,NLNonlinearElement t)
 {
  char RoutineName[]="NLNEGetInteralDimension";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 2) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  this=NLPGetNonlinearElement(P,t);

  return this->nInternalVariables;
 }

int NLNEGetElementDimension(NLProblem P,NLNonlinearElement t)
 {
  char RoutineName[]="NLNEGetElementDimension";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 2) is invalid %d > 0 && %d < %d",t,t,NLPGetNumberOfNonlinearElements(P));
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  this=NLPGetNonlinearElement(P,t);

  return this->nElementVariables;
 }

char *NLNEGetName(NLProblem P,NLNonlinearElement t)
 {
  char RoutineName[]="NLNEGetName";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 2) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  this=NLPGetNonlinearElement(P,t);

  return this->name;
 }

NLMatrix NNLEGetRangeXForm(NLProblem P,NLNonlinearElement t)
 {
  char RoutineName[]="NNLEGetRangeXForm";
  NLNonlinearElementPtr this;

  if(P==(NLProblem)NULL)
   {
    sprintf(NLNEErrorMsg,"Problem (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(t<0||t>=NLPGetNumberOfNonlinearElements(P))
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 2) is invalid");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  this=NLPGetNonlinearElement(P,t);

  return NLEGetRangeXForm(this->F);
 }

NLElementFunction NLNEPGetElementFunction(NLNonlinearElementPtr this)
 {
  char RoutineName[]="NLNEPGetElementFunction";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  return this->F;
 }

NLMatrix NLNEPGetRangeXForm(NLNonlinearElementPtr this)
 {
  char RoutineName[]="NLNEPGetRangeXForm";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  return NLEGetRangeXForm(this->F);
 }

int NLNEPGetIndex(NLNonlinearElementPtr this,int i)
 {
  char RoutineName[]="NLNEPGetIndex";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  if(i<0||i>this->nElementVariables)
   {
    sprintf(NLNEErrorMsg,"index (argument 2) is Invalid (%d in range %d to %d)",i,0,this->nElementVariables);
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }

  return this->ElVars[i];
 }

int NLNEPGetInternalDimension(NLNonlinearElementPtr this)
 {
  char RoutineName[]="NLNEPGetInternalDimension";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  return this->nInternalVariables;
 }

int NLNEPGetElementDimension(NLNonlinearElementPtr this)
 {
  char RoutineName[]="NLNEPGetElementDimension";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  return this->nElementVariables;
 }

char *NLNEPGetName(NLNonlinearElementPtr this)
 {
  char RoutineName[]="NLNEPGetRangeXForm";

  if(this==(NLNonlinearElementPtr)NULL)
   {
    sprintf(NLNEErrorMsg,"Nonlinear Element (argument 1) is NULL");
    NLSetError(12,RoutineName,NLNEErrorMsg,__LINE__,__FILE__);
    return;
   }
  return this->name;
 }
