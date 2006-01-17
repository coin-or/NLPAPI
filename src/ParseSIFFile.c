/*
    (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 12/1/2002.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory
*/
#include <NLPAPI.h>
#include <stdio.h>
#include <ctype.h>

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

static int needMax=0;

static int groupsDONE=0;
static int variablesDONE=0;
void setLinearElementOfGroup(char*,int,double);

void rectifyProblemName();
void rectString(char*);

double genericGF(double x,void *d){return(x*x);}
double dgenericGF(double x,void *d){return(2*x);}
double ddgenericGF(double x,void *d){return(2);}

double genericEF(int n,double *x,void *d){return(x[0]*x[0]);}
double dgenericEF(int i,int n,double *x,void *d){return(2*x[0]);}
double ddgenericEF(int i,int j,int n,double *x,void *d){return(2);}

static int getline(FILE*,char*);
static void parseFields(char*,char*,char*,char*,int*,double*,char*,int*,double*);
static void DumpProblemToCSource(FILE*);

/* ----------------------------------------------------------------------------------- */

int nElementCodeLines,mElementCodeLines=-1;
char **elementCodeLines=(char**)NULL;
void AddElementCodeLine(char*);
int nLocalElementCodeLines,mLocalElementCodeLines=-1;
char **localElementCodeLines=(char**)NULL;
void AddLocalElementCodeLine(char*);
int nElementDeclLines,mElementDeclLines=-1;
char **elementDeclLines=(char**)NULL;
void AddElementDeclLine(char*);

int nGroupCodeLines,mGroupCodeLines=-1;
char **groupCodeLines=(char**)NULL;
void AddGroupCodeLine(char*);
int nLocalGroupCodeLines,mLocalGroupCodeLines=-1;
char **localGroupCodeLines=(char**)NULL;
void AddLocalGroupCodeLine(char*);
int nGroupDeclLines,mGroupDeclLines=-1;
char **groupDeclLines=(char**)NULL;
void AddGroupDeclLine(char*);

/* ----------------------------------------------------------------------------------- */

static int findVariableNumber(char*);
static int findVariableNumberNoError(char*);
static void addVariable(char*,double);
static void addVariableNoError(char*,double);

static int nv=0,mv=-1;
static char **varname=(char**)NULL;
static double *varscl=(double*)NULL;
static int *varnLE=(int*)NULL;
static int *varmLE=(int*)NULL;
static int **varLEi=(int**)NULL;
static double **varLEv=(double**)NULL;

static void expandName(char*,char*);

/* ----------------------------------------------------------------------------------- */

static int findGroupNumber(char*);
static int addGroup(char*);
static int getGroupConstraint(int);
static int findConstraintNumber(char*);
static void setGroupConstraint(char*,int);
static void setGroupScale(char*,double);
static char getGroupGType(int);
static void setGroupGType(char*,char);
static void setGroupType(char*,char*);
static void setAllGroupTypes(char*);
static void setGroupParm(char*,char*,double);
static int addElementToGroup(char*,char*,double);

struct Group { char *name;
               char *type;
               char gtype;
               int constraint;
               double scale;
               int melements;
               int nelements;
               char **elements;
               double *weights;
               double *parms;};

static int ng=0,mg=-1;
static struct Group *groups;

/* ----------------------------------------------------------------------------------- */

static double findParmValue(char*);
static void setParameter(char*,double);

static int np=0,mp=-1;
static char **prmname=(char**)NULL;
static double *prmval=(double*)NULL;

/* ----------------------------------------------------------------------------------- */

static void addDo(char*,int,int);
static void removeDo(int);
static void setDoIncr(char*,int);
static void removeAllDos();
static void initialDo();
static int nextDo();
void setDoStart(int,int);
void incrementDoVariable(int);
void initializeDoVariable(int);
int getDoValue(int);
int getDoEnd(int);
int getDoStart(int);

static int nDos,mDos=-1;
static char **doVar=(char**)NULL;
static int *doLimits=(int*)NULL;

int nInStack=0;
int mInStack=-1;
int cInStack=0;
char **stack=(char**)NULL;

void addToStack(char*);
char *getLineFromStack(int);
char *getNextLineFromStack();
/* ----------------------------------------------------------------------------------- */

static char defaultElementType[20];

struct ElementType { char *name;
                     NLElementFunction ef;
                     NLMatrix R;
                     int nelemvars;
                     int melemvars;
                     char **elemvars;
                     int nintvars;
                     int mintvars;
                     char **intvars;
                     int nparms;
                     int mparms;
                     char **parms;};

static int addElementType(char*);
static int findElementTypeNumber(char*);
static int addElementVariableToElementType(char*,char*);
static int addInternalVariableToElementType(char*,char*);
static int addParameterToElementType(char*,char*);
static int findElementVariableNumber(struct ElementType*,char*);
static int findInternalVariableNumber(struct ElementType*,char*);
static int findElementParameterNumber(struct ElementType*,char*);

static int net,met=-1;
static struct ElementType *elementtypes;

/* ----------------------------------------------------------------------------------- */
static char defaultGroupType[20];

struct GroupType { char *name;
                   NLGroupFunction gf;
                   char *var;
                   int nparms;
                   int mparms;
                   char **parms;};

static int addGroupType(char*,char*);
static int findGroupTypeNumber(char*);
static int addParameterToGroupType(char*,char*);
static int findGroupParameterNumber(struct GroupType*,char*);

static int ngt,mgt=-1;
static struct GroupType *grouptypes;

/* ----------------------------------------------------------------------------------- */

static int addElement(char*,char*);
static int findElementNumber(char*);
static int setElementVariable(char*,int,int);
static int setElementParameter(char*,int,double);

struct Element { char *name;
                 struct ElementType *type;
                 NLNonlinearElement ne;
                 int *vars;
                 double *parms;};

static int ne,me=-1;
static struct Element *elements;

/* ----------------------------------------------------------------------------------- */

static void handlePARMS(char*,char*,char*,int,double,char*,int,double);
static void handleVARIABLES(char*,char*,char*,int,double,char*,int,double);
static void handleVARIABLES2(char*,char*,char*,int,double,char*,int,double);
static void handleGROUPS(char*,char*,char*,int,double,char*,int,double);
static void handleGROUPS2(char*,char*,char*,int,double,char*,int,double);
static void handleCONSTANTS(char*,char*,char*,int,double,char*,int,double);
static void handleINEQUALITYBOUNDS(char*,char*,char*,int,double,char*,int,double);
static void handleBOUNDS(char*,char*,char*,int,double,char*,int,double);
static void handleSTART_POINT(char*,char*,char*,int,double,char*,int,double);
static void handleELEMENT_TYPE(char*,char*,char*,int,double,char*,int,double);
static void handleELEMENT_USES(char*,char*,char*,int,double,char*,int,double);
static void handleGROUP_TYPE(char*,char*,char*,int,double,char*,int,double);
static void handleGROUP_USES(char*,char*,char*,int,double,char*,int,double);
static void handleOBJECT_BOUND(char*,char*,char*,int,double,char*,int,double);
static void handleGROUPFUNCTIONS(char*,char*,char*,int,double,char*,int,double);
static void handleELEMENTS(char*,char*,char*,int,double,char*,int,double);

static void beginNAME();
static void beginVARIABLES();
static void beginGROUPS();
static void beginCONSTANTS();
static void beginRANGES();
static void beginBOUNDS();
static void beginSTART_POINT();
static void beginELEMENT_TYPE();
static void beginELEMENT_USES();
static void beginGROUP_TYPE();
static void beginGROUP_USES();
static void beginOBJECT_BOUND();
static void beginGROUPFUNCTIONS();
static void beginELEMENTS();

static void endNAME();
static void endVARIABLES();
static void endGROUPS();
static void endCONSTANTS();
static void endRANGES();
static void endBOUNDS();
static void endSTART_POINT();
static void endELEMENT_TYPE();
static void endELEMENT_USES();
static void endGROUP_TYPE();
static void endGROUP_USES();
static void endOBJECT_BOUND();
static void endGROUPFUNCTIONS();
static void endELEMENTS();

static char inname[256];
static char line[256];
static int lineno;
static char *section="";
static NLProblem P=(NLProblem)NULL;

typedef enum {LOOKING,
              NAME,
              VARIABLES,
              ROWS,
              CONSTANTS,
              RANGES,
              BOUNDS,
              START_POINT,
              ELEMENTS,
              ELEMENT_TYPE,
              ELEMENT_USES,
              GROUPS,
              GROUP_TYPE,
              GROUP_USES,
              OBJECT_BOUND
             } STATE;

static FILE *fin;
static FILE *FOUT;
static FILE *finc;
static FILE *COUT;
static double *xStart=(double*)NULL;
static double *lStart=(double*)NULL;
static char substate;
static char problemName[256];
static char rectproblemName[256];

static int GROUPFUNCTIONSDone=0;

NLProblem LNReadSIF(char *SIFfile, double **x0, double **l0)
 {
  int i,j;
  int rc;
  int verbose=0;
  STATE state;
  char code[3];
  char name1[11];
  char name2[11];
  char name3[11];
  int val1Set;
  double val1;
  int val2Set;
  double val2;
  int secondGROUPS;
  char foutname[256];
  char ioutname[256];
  char coutname[256];
  int nc;

  strcpy(inname,SIFfile);
  if((int)strlen(SIFfile)<4 || strcmp(SIFfile+(int)strlen(SIFfile)-4,".SIF"))strcat(inname,".SIF");

  if((fin=fopen(inname,"r"))==(FILE*)NULL)
   {
    fprintf(stderr,"Couldn't open SIFfile -->%s<--\n",inname);
    return (NLProblem)NULL;
   }

  strcpy(foutname,inname);
  foutname[(int)strlen(inname)-4]=0x0;
  strcat(foutname,"Fort.f");
  if((FOUT=fopen(foutname,"w"))==(FILE*)NULL)
   {
    fprintf(stderr,"Couldn't open FORTRAN output file -->%s<--\n",foutname);
    return (NLProblem)NULL;
   }

  strcpy(ioutname,inname);
  ioutname[(int)strlen(inname)-4]=0x0;
  strcat(ioutname,"H.h");
  if((finc=fopen(ioutname,"w"))==(FILE*)NULL)
   {
    fprintf(stderr,"Couldn't open include output file -->%s<--\n",ioutname);
    return (NLProblem)NULL;
   }

  strcpy(coutname,inname);
  coutname[(int)strlen(inname)-4]=0x0;
  strcat(coutname,"C.c");
  if((COUT=fopen(coutname,"w"))==(FILE*)NULL)
   {
    fprintf(stderr,"Couldn't open C output file -->%s<--\n",coutname);
    return (NLProblem)NULL;
   }

  state=LOOKING;
  secondGROUPS=0;
  lineno=0;
  while(getline(fin,line))
   {
    if(line[0]==0x0)goto NEXTLINE;
    if(line[0]=='*')goto NEXTLINE;

    if(verbose){printf("INPUT line %d, -->%s<--\n",lineno,line);fflush(stdout);}

    if(state!=LOOKING && line[0]!=' ')
     {
      if(verbose){printf("state!=LOOKING && line[0]!=' '\n");fflush(stdout);}
      substate=' ';
      if(!strncmp(line,"TEMPORARIES",11))substate='T';
       else if(!strncmp(line,"INDIVIDUALS",11))substate='I';
       else if(!strncmp(line,"GLOBALS",7))substate='G';
      if(substate!=' ')goto NEXTLINE;
      if(verbose){printf("substate==' '\n");fflush(stdout);}

/* End the current section */

      switch(state)
       {
        case NAME:
         endNAME();
         break;
        case VARIABLES:
         endVARIABLES();
         break;
        case GROUPS:
         endGROUPFUNCTIONS();
         break;
        case ROWS:
         endGROUPS();
         secondGROUPS=1;
         break;
        case CONSTANTS:
         endCONSTANTS();
         break;
        case RANGES:
         endRANGES();
         break;
        case BOUNDS:
         endBOUNDS();
         break;
        case START_POINT:
         endSTART_POINT();
         break;
        case ELEMENT_TYPE:
         endELEMENT_TYPE();
         break;
        case ELEMENT_USES:
         endELEMENT_USES();
         break;
        case GROUP_TYPE:
         endGROUP_TYPE();
         break;
        case GROUP_USES:
         if(verbose){printf("end GROUP_USES\n");fflush(stdout);}
         endGROUP_USES();
         break;
        case OBJECT_BOUND:
         endOBJECT_BOUND();
         break;
        case ELEMENTS:
         endELEMENTS();
         break;
        default:
         printf("Couldn't end current state, not known\n");
         break;
       }
      if(state!=ELEMENTS || (ngt>0 && !GROUPFUNCTIONSDone))
       {
        state=LOOKING;
        section="LOOKING for next section";
        if(!strncmp(line,"ENDATA",6))goto NEXTLINE;
       }else{
        while(getline(fin,line))
         {
          if(line[0]!=0x0&&line[0]!='*' && strncmp(line,"GROUPS",6) && strncmp(line,"ENDATA",6))fprintf(FOUT,"%s\n",line);
         }
        goto FINISH;
       }
     }

    switch(state)
     {
      case LOOKING:
       if(verbose){printf(" LOOKING for next section\n");fflush(stdout);}

/* Enter a new section */

       if(!strncmp(line,"NAME",4))                {state=NAME;section="NAME";beginNAME();}
         else if(!strncmp(line,"VARIABLES", 9)
               ||!strncmp(line,"COLUMNS", 7))     {state=VARIABLES;section="VARIABLES";beginVARIABLES();}
         else if(!strncmp(line,"GROUPS",6)&&secondGROUPS==0
               ||!strncmp(line,"ROWS",4)
               ||!strncmp(line,"CONSTRAINTS",11)) {state=ROWS;section="ROWS";beginGROUPS();}
         else if(!strncmp(line,"RHS",3)
               ||!strncmp(line,"RHS'",4)
               ||!strncmp(line,"CONSTANTS", 9))   {state=CONSTANTS;section="CONSTANTS";beginGROUPFUNCTIONS();}
         else if(!strncmp(line,"RANGES", 6))      {state=RANGES;section="RANGES";beginRANGES();}
         else if(!strncmp(line,"BOUNDS", 6))      {state=BOUNDS;section="BOUNDS";beginBOUNDS();}
         else if(!strncmp(line,"START POINT",11)) {state=START_POINT;section="START POINT";beginSTART_POINT();}
         else if(!strncmp(line,"ELEMENT TYPE",12)){state=ELEMENT_TYPE;section="ELEMENT TYPE";beginELEMENT_TYPE();}
         else if(!strncmp(line,"ELEMENT USES",12)){state=ELEMENT_USES;section="ELEMENT USES";beginELEMENT_USES();}
         else if(!strncmp(line,"GROUP TYPE",10))  {state=GROUP_TYPE;section="GROUP TYPE";beginGROUP_TYPE();}
         else if(!strncmp(line,"GROUP USES",10))  {state=GROUP_USES;section="GROUP USES";beginGROUP_USES();}
         else if(!strncmp(line,"OBJECT BOUND",12)){state=OBJECT_BOUND;section="OBJECT BOUND";beginOBJECT_BOUND();if(verbose){printf(" Start of OBJECT BOUND\n");fflush(stdout);}}
         else if(!strncmp(line,"GROUPS", 6))      {state=GROUPS;section="GROUPS";beginGROUPS();}
         else if(!strncmp(line,"ELEMENTS", 8))    {state=ELEMENTS;section="ELEMENTS";beginELEMENTS();}
         else{
         line[10]=0x0;
         fprintf(stderr,"Looking for next section, but line %d in file %s starts with %s, which isn't a section name\n",lineno,inname,line);
         return (NLProblem)NULL;
        }
       break;

      case NAME:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case VARIABLES:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       if(!groupsDONE)
         handleVARIABLES(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        else
         handleVARIABLES2(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case ROWS:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       if(variablesDONE)
         handleGROUPS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        else
         handleGROUPS2(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case GROUPS:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleGROUPFUNCTIONS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case CONSTANTS:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleCONSTANTS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case RANGES:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleINEQUALITYBOUNDS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case BOUNDS:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleBOUNDS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case START_POINT:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleSTART_POINT(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case ELEMENT_TYPE:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleELEMENT_TYPE(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case ELEMENT_USES:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleELEMENT_USES(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case GROUP_TYPE:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleGROUP_TYPE(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case GROUP_USES:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleGROUP_USES(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case OBJECT_BOUND:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleOBJECT_BOUND(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      case ELEMENTS:
       parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
       handleELEMENTS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       break;

      default:
       fprintf(stderr,"Unknown Section type: %s, state=%d, line number %d of file %s\n",section,state,lineno,inname);
       abort();
       break;
     }

NEXTLINE:
    continue;
   }
FINISH:
  if(xStart==(double*)NULL)
   {
    int iv,nc;
    nc=NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P);

    xStart=(double*)malloc(nv*sizeof(double));
    for(iv=0;iv<nv;iv++)xStart[iv]=0.;
    lStart=(double*)malloc(nc*sizeof(double));
    for(iv=0;iv<nc;iv++)lStart[iv]=0.;
   }
  *x0=xStart;
  *l0=lStart;
  DumpProblemToCSource(COUT);
  fclose(fin);
  if(needMax)
   {
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      double precision function MAX(x,y)\n");
    fprintf(FOUT,"      double precision x,y\n");
    fprintf(FOUT,"      MAX=x\n");
    fprintf(FOUT,"      if(y.gt.x)MAX=y\n");
    fprintf(FOUT,"      return\n");
    fprintf(FOUT,"      end\n");
    fflush(FOUT);
   }
  fclose(FOUT);
  fclose(COUT);
  nc=NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P);
  if(nc==0)nc=1;
  sprintf(line,"sed -e\"s/@@@@@/%s/\" testTemplate.c | sed -e\"s/@@@@/%d/\" | sed -e\"s/@@@/%d/\" > test%s.c",rectproblemName,nv,nc,rectproblemName);
  system(line);
  return P;
 }

int getline(FILE *fin,char *line)
 {
  int i;
  int val1Set,val2Set;
  double val1,val2;
  char code[10];
  char name1[20];
  char name2[20];
  char name3[20];
  int iv,iv1,iv2;
  int fromfile;
  int verbose=0;

NextLine:

  if(feof(fin))return 0;

  if(nInStack>0&&cInStack<nInStack)
   {
    strcpy(line,getLineFromStack(cInStack));
    if(verbose){printf("Stack: current line is %d\n",cInStack);
    for(i=0;i<nInStack;i++)printf("  %d    -->%s<--\n",i,stack[i]);}
    fflush(stdout);
    if(verbose){printf("got line -->%s<-- from stack line %d\n",line,cInStack);fflush(stdout);}

    cInStack++;
    fromfile=0;
   }else{
    i=0;
    do{
       line[i]=fgetc(fin);i++;
      }while(!iscntrl(line[i-1]) && !feof(fin));
    line[i-1]=0x0;
    while((int)strlen(line)>0&&line[(int)strlen(line)-1]==' ')line[(int)strlen(line)-1]=0x0;
    if(verbose){printf("got line -->%s<-- from file\n",line);fflush(stdout);}
    fromfile=1;
    lineno++;
   }

  if(!strncmp(line," DO",3))
   {
    parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
    if(!val1Set)val1=findParmValue(name2);
    if(!val2Set)val2=findParmValue(name3);
    iv1=(int)val1;
    iv2=(int)val2;
    addDo(name1,iv1,iv2);
    if(fromfile)addToStack(line);
    setDoStart(nDos-1,cInStack);
    initializeDoVariable(nDos-1);
    if(verbose){printf("do %s (%d) starts at stack line %d, do variable starts at %d, ends at %d\n",name1,nDos-1,cInStack,getDoValue(nDos-1),getDoEnd(nDos-1));fflush(stdout);}
   }else if(!strncmp(line," DI",3))
   {
    parseFields(line,code,name1,name2,&val1Set,&val1,name3,&val2Set,&val2);
    if(!val1Set)val1=findParmValue(name2);
    iv=(int)val1;
    setDoIncr(name1,iv);
    if(fromfile)addToStack(line);
    setDoStart(nDos-1,cInStack);
    if(verbose){printf("do %s increment now %d. Do now starts at %d\n",name1,iv,cInStack);fflush(stdout);}
   }else if(!strncmp(line," OD",3))
   {
    if(fromfile)addToStack(line);
    incrementDoVariable(nDos-1);
    if(verbose){printf("end of do next value %d, end value %d\n",getDoValue(nDos-1),getDoEnd(nDos-1));fflush(stdout);}
    if(getDoValue(nDos-1)<=getDoEnd(nDos-1))
     {
      cInStack=getDoStart(nDos-1);
      if(verbose){printf("jump back to stack line %d \n",cInStack);fflush(stdout);}
      goto NextLine;
     }else{
      removeDo(nDos-1);
      if(verbose){printf("done do, go on to statemen\n",cInStack);fflush(stdout);}
      goto NextLine;
     }
   }else if(!strcmp(line," ND"))
   {
    if(fromfile)addToStack(line);
    if(verbose){printf("end of all dos\n");fflush(stdout);}
    while(nDos>0)
     {
      incrementDoVariable(nDos-1);
      if(verbose){printf("next value loop %d is %d, end value %d\n",nDos-1,getDoValue(nDos-1),getDoEnd(nDos-1));fflush(stdout);}
      if(getDoValue(nDos-1)<=getDoEnd(nDos-1))
       {
        cInStack=getDoStart(nDos-1);
        if(verbose){printf("jump back to stack line %d \n",cInStack);fflush(stdout);}
        goto NextLine;
       }else{
        if(verbose){printf("past end, remove do\n");fflush(stdout);}
        removeDo(nDos-1);
       }
     }
    nInStack=0;cInStack=0;
    goto NextLine;
   }else if(nDos>0)
   {
    int fl,iii;
    if(fromfile)addToStack(line);
    fl=0;
    for(iii=0;iii<nDos;iii++)if(getDoValue(iii)>getDoEnd(iii))fl=1;
    if(fl)
     {
      if(verbose)
       {
        int jjj;
        for(jjj=nDos-1;jjj>-1;jjj--)
          printf("flushing, do %d, %d<=%d\n",jjj,getDoValue(jjj),getDoEnd(jjj));
        fflush(stdout);
       }
      goto NextLine;
     }else{
      if(verbose)
       {
        int jjj;
        for(jjj=nDos-1;jjj>-1;jjj--)
          printf("not flushing, do %d, %d<=%d\n",jjj,getDoValue(jjj),getDoEnd(jjj));
        fflush(stdout);
       }
     }
    return 1;
   }else 
    return 1;

  goto NextLine;

  return 1;
 }

void parseFields(char *line, char *code, char *name1, char *name2,int *val1Set,double *val1, char *name3, int *val2Set,double *val2)
 {
  int i,j,j0,j1,n;
  static char temp[20];
  int verbose=0;

  n=(int)strlen(line);

  code[0]=' ';
  code[1]=' ';
  code[2]=0x0;

  if(n>2)
   {
    code[0]=line[1];
    code[1]=line[2];
    code[2]=0x0;
   }
  if(verbose){printf("parseFields, code=-->%s<--\n",code);fflush(stdout);}

  j0=4;j1=MIN(14,n);name1[0]=0x0;
  if(j0<n&&j1<=n)
   {
    i=0;j=j0;
    while(j<j1&&line[j]==' ')j++;
    while(j<j1){name1[i]=line[j];i++;name1[i]=0x0;j++;}
    i--;while(i>-1&&name1[i]==' '){name1[i]=0x0;i--;}
   }
  if(verbose){printf("             name1 -->%s<--\n",name1);fflush(stdout);}

  j0=14;j1=MIN(24,n);name2[0]=0x0;
  if(j0<n&&j1<=n)
   {
    i=0;j=j0;
    while(j<j1&&line[j]==' ')j++;
    while(j<j1){name2[i]=line[j];i++;name2[i]=0x0;j++;}
    i--;while(i>-1&&name2[i]==' '){name2[i]=0x0;i--;}
   }
  if(verbose){printf("             name2 -->%s<--\n",name2);fflush(stdout);}

  j0=24;j1=MIN(36,n);*val1Set=0;*val1=0.;
  if(verbose&&j0<n){printf("val1 -->%s<-- [%d,%d)\n",line+j0,j0,j1);fflush(stdout);}
  if(j0<n&&j1<=n)
   {
    strncpy(temp,line+j0,j1-j0+1);temp[j1-j0+1]=0x0;
    for(i=0;i<j1-j0+1;i++)
     {
      *val1Set=*val1Set||isdigit(temp[i]);
      if(temp[i]=='D'||temp[i]=='d')temp[i]='e';
     }
    if(verbose&&*val1Set){printf("val1 -->%s<--\n",temp);fflush(stdout);}
    if(*val1Set)sscanf(temp,"%le",val1);
   }
  if(verbose){printf("             val1Set=%d, val1=%le\n",*val1Set,*val1);fflush(stdout);}

  j0=39;j1=MIN(49,n);name3[0]=0x0;
  if(verbose&&j0<n){printf("name3 -->%s<-- [%d,%d)\n",line+j0,j0,j1);fflush(stdout);}
  if(j0<n&&j1<=n)
   {
    i=0;j=j0;
    while(j<j1&&line[j]==' ')j++;
    while(j<j1){name3[i]=line[j];i++;name3[i]=0x0;j++;}
    i--;while(i>-1&&name3[i]==' '){name3[i]=0x0;i--;}
   }
  if(verbose){printf("             name3 -->%s<--\n",name3);fflush(stdout);}

  j0=49;j1=MIN(61,n);*val2Set=0;*val2=0.;
  if(verbose&&j0<n){printf("val2 -->%s<-- [%d,%d)\n",line+j0,j0,j1);fflush(stdout);}
  if(j0<n&&j1<=n)
   {
    strncpy(temp,line+j0,j1-j0+1);temp[j1-j0+1]=0x0;
    for(i=0;i<j1-j0+1;i++)
     {
      *val2Set=*val2Set||isdigit(temp[i]);
      if(temp[i]=='D'||temp[i]=='d')temp[i]='e';
     }
    if(verbose&&*val2Set){printf("val2 -->%s<--\n",temp);fflush(stdout);}
    if(*val2Set)sscanf(temp,"%le",val2);
   }
  if(verbose){printf("             val2Set=%d, val2=%le\n",*val2Set,*val2);fflush(stdout);}

  return;
 }

int findVariableNumberNoError(char *name)
 {
  int iv,j;

  iv=-1;
  for(j=0;j<nv;j++)
   {
    if(!strcmp(name,varname[j]))iv=j;
   }
  return iv;
 }

int findVariableNumber(char *name)
 {
  int iv,j;

  iv=-1;
  for(j=0;j<nv;j++)
   {
    if(!strcmp(name,varname[j]))iv=j;
   }
  if(iv<0||iv>=nv)
   {
    fprintf(stderr,"In %s section, line %d in file %s, variable %s is invalid.\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }
  return iv;
 }

double findParmValue(char *name)
 {
  int ip,j;

  ip=-1;
  for(j=0;j<np;j++)
   {
    if(!strcmp(name,prmname[j]))ip=j;
   }
  if(ip<0||ip>=np)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid parameter -->%s<--\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }
  return prmval[ip];
 }

int findGroupNumber(char *name)
 {
  int ig,j;

  ig=-1;
  for(j=0;j<ng;j++)
   {
    if(!strcmp(name,(groups[j]).name))ig=j;
   }
  if(ig<0||ig>=ng)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid group %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return ig;
 }

void setParameter(char *name, double value)
 {
  int ip,j;
  int verbose=0;

/* Is it there arlready? */

  ip=-1;
  for(j=0;j<np;j++)
   {
    if(!strcmp(name,prmname[j]))ip=j;
   }
  if(ip!=-1)
   {
    prmval[ip]=value;
    if(verbose){printf("Set parameter -->%s<-- to %lf\n",name,value);fflush(stdout);}
    return;
   }

  if(np>=mp)
   {
    mp+=10;
    prmname=(char**)realloc(prmname,mp*sizeof(char*));
    for(j=np;j<mp;j++){prmname[j]=(char*)malloc(20*sizeof(char));(prmname[j])[0]=0x0;}
    prmval=(double*)realloc(prmval,mp*sizeof(double));
   }

  if(verbose){printf("Set parameter -->%s<-- to %lf\n",name,value);fflush(stdout);}
  strcpy(prmname[np],name);
  prmval[np]=value;
  np++;

  return;
 }

int addGroup(char *name)
 {
  int ig,j;

/* Is it there already? */

  ig=-1;
  for(j=0;j<ng;j++)
   {
    if(!strcmp(name,(groups[j]).name))ig=j;
   }
  if(ig!=-1)return 0;

  if(ng>=mg)
   {
    mg+=10;
    groups=(struct Group*)realloc(groups,mg*sizeof(struct Group));
    for(j=ng;j<mg;j++)
     {
      (groups[j]).name=(char*)malloc(20*sizeof(char));((groups[j]).name)[0]=0x0;
      (groups[j]).type=(char*)malloc(20*sizeof(char));((groups[j]).type)[0]=0x0;
      (groups[j]).scale=1.;
      (groups[j]).nelements=0;
      (groups[j]).melements=-1;
      (groups[j]).elements=(char**)NULL;
      (groups[j]).weights=(double*)NULL;
      (groups[j]).parms=(double*)NULL;
     }
   }
  strcpy((groups[ng]).name,name);
  (groups[ng]).constraint=-1;
  (groups[ng]).gtype=' ';
  ng++;

  return 1;
 }

char getGroupGType(int ig)
 {
  return (groups[ig]).gtype;
 }

void setGroupGType(char *name, char type)
 {
  int ig;

  ig=findGroupNumber(name);
  (groups[ig]).gtype=type;
  return;
 }

int getGroupConstraint(int ig)
 {
  return (groups[ig]).constraint;
 }

void setGroupScale(char *name,double s)
 {
  int ig;

  ig=findGroupNumber(name);
  (groups[ig]).scale=s;
  return;
 }

void setGroupConstraint(char *name, int val)
 {
  int ig;

  ig=findGroupNumber(name);
  (groups[ig]).constraint=val;
  return;
 }

void addVariable(char *name,double scl)
 {
  int iv,j;

/* Is it there arlready? */

  iv=-1;
  for(j=0;j<nv;j++)
   {
    if(!strcmp(name,varname[j]))iv=j;
   }
  if(iv!=-1)
   {
    fprintf(stderr,"In %s section, line %d in file %s variable %s previously defined\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  if(nv>=mv)
   {
    mv+=10;
    varname=(char**)realloc(varname,mv*sizeof(char*));
    varscl=(double*)realloc(varscl,mv*sizeof(double));
    varnLE=(int*)realloc(varnLE,mv*sizeof(char*));
    varmLE=(int*)realloc(varmLE,mv*sizeof(char*));
    varLEi=(int**)realloc(varLEi,mv*sizeof(char*));
    varLEv=(double**)realloc(varLEv,mv*sizeof(char*));;
    for(j=nv;j<mv;j++)
     {
      varname[j]=(char*)malloc(20*sizeof(char));(varname[j])[0]=0x0;
      varscl[j]=1.;
      varnLE[j]=0;
      varmLE[j]=-1;
      varLEi[j]=(int*)NULL;
      varLEv[j]=(double*)NULL;
     }
   }
  strcpy(varname[nv],name);
  varscl[nv]=scl;
  nv++;

  return;
 }

void addVariableNoError(char *name,double scl)
 {
  int iv,j;

/* Is it there arlready? */

  iv=-1;
  for(j=0;j<nv;j++)
   {
    if(!strcmp(name,varname[j]))iv=j;
   }
  if(iv!=-1)return;

  if(nv>=mv)
   {
    mv+=10;
    varname=(char**)realloc(varname,mv*sizeof(char*));
    varscl=(double*)realloc(varscl,mv*sizeof(double));
    varnLE=(int*)realloc(varnLE,mv*sizeof(char*));
    varmLE=(int*)realloc(varmLE,mv*sizeof(char*));
    varLEi=(int**)realloc(varLEi,mv*sizeof(char*));
    varLEv=(double**)realloc(varLEv,mv*sizeof(char*));;
    for(j=nv;j<mv;j++)
     {
      varname[j]=(char*)malloc(20*sizeof(char));(varname[j])[0]=0x0;
      varscl[j]=1.;
      varnLE[j]=0;
      varmLE[j]=-1;
      varLEi[j]=(int*)NULL;
      varLEv[j]=(double*)NULL;
     }
   }
  strcpy(varname[nv],name);
  varscl[nv]=scl;
  nv++;

  return;
 }

void handlePARMS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  double parval;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;
  int iv,iv1,iv2;

  if(verbose){printf("handlePARMS, code=->%s<-, name1=->%s<-, name2=->%s<-\n",code,name1,name2);fflush(stdout);}

  switch(code[0])
   {
    case 'I':
     switch(code[1])
      {
       case 'E':
        iv=(int)val1;
        if(verbose){printf("Code IE, set variable %s to %d\n",name1,iv);}
        setParameter(name1,iv*1.0);
        break;
       case 'R':
        iv=(int)val1;
        iv=(int)findParmValue(name2);
        parval=iv;
        if(verbose){printf("Code IR, set variable %s to %d=int(%lf)\n",name1,iv,findParmValue(name2));}
        setParameter(name1,parval);
        break;
       case 'A':
        iv=(int)(val1+findParmValue(name2));
        parval=iv;
        if(verbose){printf("Code IA, set variable %s to %f=int(%lf+%lf)\n",name1,parval,val1,findParmValue(name2));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case 'S':
        iv=(int)(val1-findParmValue(name2));
        parval=iv;
        if(verbose){printf("Code IS, set variable %s to %f=int(%lf-%lf)\n",name1,parval,val1,findParmValue(name2));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case 'M':
        iv=(int)(val1*findParmValue(name2));
        parval=iv;
        if(verbose){printf("Code IM, set variable %s to %f=int(%lf*%lf)\n",name1,parval,val1,findParmValue(name2));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case 'D':
        iv=(int)(val1/findParmValue(name2));
        parval=iv;
        if(verbose){printf("Code IM, set variable %s to %f=int(%lf/%lf)\n",name1,parval,val1,findParmValue(name2));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case '=':
        iv=(int)findParmValue(name2);
        parval=iv;
        if(verbose){printf("Code I=, set variable %s to %f=int(%lf)\n",name1,parval,findParmValue(name2));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case '+':
        iv=(int)(findParmValue(name2)+findParmValue(name3));
        parval=iv;
        if(verbose){printf("Code I+, set variable %s to %f=int(%lf+%lf)\n",name1,parval,findParmValue(name2),findParmValue(name3));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case '-':
        iv=(int)(findParmValue(name2)-findParmValue(name3));
        parval=iv;
        if(verbose){printf("Code I-, set variable %s to %f=int(%lf-%lf)\n",name1,parval,findParmValue(name2),findParmValue(name3));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case '*':
        iv=(int)(findParmValue(name2)*findParmValue(name3));
        parval=iv;
        if(verbose){printf("Code I*, set variable %s to %f=int(%lf*%lf)\n",name1,parval,findParmValue(name2),findParmValue(name3));fflush(stdout);}
        setParameter(name1,parval);
        break;
       case '/':
        iv=(int)(findParmValue(name2)/findParmValue(name3));
        parval=iv;
        if(verbose){printf("Code I*, set variable %s to %f=int(%lf/%lf)\n",name1,parval,findParmValue(name2),findParmValue(name3));fflush(stdout);}
        setParameter(name1,parval);
        break;
       default:
/* Invalid Code */
        break;
      }
     break;
    case 'R':
     switch(code[1])
      {
       case 'E':
        setParameter(name1,val1);
        break;
       case 'I':
        iv=(int)findParmValue(name2);
        parval=iv;
        setParameter(name1,parval);
        break;
       case 'A':
        parval=val1+findParmValue(name2);
        setParameter(name1,parval);
        break;
       case 'S':
        parval=val1-findParmValue(name2);
        setParameter(name1,parval);
        break;
       case 'M':
        parval=val1*findParmValue(name2);
        setParameter(name1,parval);
        break;
       case 'D':
        parval=val1/findParmValue(name2);
        setParameter(name1,parval);
        break;
       case 'F':
        if(!strcmp(name2,"ABS"))parval=fabs(val1);
        else if(!strcmp(name2,"SQRT"))parval=sqrt(val1);
        else if(!strcmp(name2,"EXP"))parval=exp(val1);
        else if(!strcmp(name2,"LOG"))parval=log(val1);
        else if(!strcmp(name2,"LOG10"))parval=log10(val1);
        else if(!strcmp(name2,"SIN"))parval=sin(val1);
        else if(!strcmp(name2,"COS"))parval=cos(val1);
        else if(!strcmp(name2,"TAN"))parval=tan(val1);
        else if(!strcmp(name2,"ARCSIN"))parval=asin(val1);
        else if(!strcmp(name2,"ARCCOS"))parval=acos(val1);
        else if(!strcmp(name2,"ARCTAN"))parval=atan(val1);
        else if(!strcmp(name2,"HYPSIN"))parval=sinh(val1);
        else if(!strcmp(name2,"HYPCOS"))parval=cosh(val1);
        else if(!strcmp(name2,"HYPTAN"))parval=tanh(val1);
        else {
          fprintf(stderr,"Problem parsing, function \"%s\" is not allowed\n",name2);
          parval=0.;
         }
        setParameter(name1,parval);
        break;
       case '=':
        parval=findParmValue(name2);
        setParameter(name1,parval);
        break;
       case '+':
        parval=findParmValue(name2)+findParmValue(name3);
        setParameter(name1,parval);
        break;
       case '-':
        parval=findParmValue(name2)-findParmValue(name3);
        setParameter(name1,parval);
        break;
       case '*':
        parval=findParmValue(name2)*findParmValue(name3);
        setParameter(name1,parval);
        break;
       case '/':
        parval=findParmValue(name2)/findParmValue(name3);
        setParameter(name1,parval);
        break;
       case '(':
        val1=findParmValue(name3);
        if(!strcmp(name2,"ABS"))parval=fabs(val1);
        else if(!strcmp(name2,"SQRT"))parval=sqrt(val1);
        else if(!strcmp(name2,"EXP"))parval=exp(val1);
        else if(!strcmp(name2,"LOG"))parval=log(val1);
        else if(!strcmp(name2,"LOG10"))parval=log10(val1);
        else if(!strcmp(name2,"SIN"))parval=sin(val1);
        else if(!strcmp(name2,"COS"))parval=cos(val1);
        else if(!strcmp(name2,"TAN"))parval=tan(val1);
        else if(!strcmp(name2,"ARCSIN"))parval=asin(val1);
        else if(!strcmp(name2,"ARCCOS"))parval=acos(val1);
        else if(!strcmp(name2,"ARCTAN"))parval=atan(val1);
        else if(!strcmp(name2,"HYPSIN"))parval=sinh(val1);
        else if(!strcmp(name2,"HYPCOS"))parval=cosh(val1);
        else if(!strcmp(name2,"HYPTAN"))parval=tanh(val1);
        else {
          fprintf(stderr,"Problem parsing, function \"%s\" is not allowed\n",name2);
          parval=0.;
         }
        setParameter(name1,parval);
        break;
       default:
/* Invalid Code */
        break;
      }
     break;
    case 'A':
     switch(code[1])
      {
       case 'E':
        expandName(name1,tmpname1);
        if(verbose){printf("  expanded name1=->%s<-\n",tmpname1);fflush(stdout);}
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,val1);
        break;
       case 'I':
        expandName(name1,tmpname1);expandName(name2,tmpname2);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-\n",tmpname1,tmpname2);fflush(stdout);}
        iv=(int)findParmValue(name2);
        parval=iv;
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case 'A':
        expandName(name1,tmpname1);expandName(name2,tmpname2);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-\n",tmpname1,tmpname2);fflush(stdout);}
        parval=val1+findParmValue(tmpname2);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case 'S':
        expandName(name1,tmpname1);expandName(name2,tmpname2);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-\n",tmpname1,tmpname2);fflush(stdout);}
        parval=val1-findParmValue(tmpname2);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case 'M':
        expandName(name1,tmpname1);expandName(name2,tmpname2);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-\n",tmpname1,tmpname2);fflush(stdout);}
        parval=val1*findParmValue(tmpname2);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case 'D':
        expandName(name1,tmpname1);expandName(name2,tmpname2);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-\n",tmpname1,tmpname2);fflush(stdout);}
        parval=val1/findParmValue(tmpname2);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case 'F':
        expandName(name1,tmpname1);
        if(verbose){printf("  expanded name1=->%s<-\n",tmpname1);fflush(stdout);}
        if(!strcmp(name2,"ABS"))parval=fabs(val1);
        else if(!strcmp(name2,"SQRT"))parval=sqrt(val1);
        else if(!strcmp(name2,"EXP"))parval=exp(val1);
        else if(!strcmp(name2,"LOG"))parval=log(val1);
        else if(!strcmp(name2,"LOG10"))parval=log10(val1);
        else if(!strcmp(name2,"SIN"))parval=sin(val1);
        else if(!strcmp(name2,"COS"))parval=cos(val1);
        else if(!strcmp(name2,"TAN"))parval=tan(val1);
        else if(!strcmp(name2,"ARCSIN"))parval=asin(val1);
        else if(!strcmp(name2,"ARCCOS"))parval=acos(val1);
        else if(!strcmp(name2,"ARCTAN"))parval=atan(val1);
        else if(!strcmp(name2,"HYPSIN"))parval=sinh(val1);
        else if(!strcmp(name2,"HYPCOS"))parval=cosh(val1);
        else if(!strcmp(name2,"HYPTAN"))parval=tanh(val1);
        else {
          fprintf(stderr,"Problem parsing, function \"%s\" is not allowed\n",name2);
          parval=0.;
         }
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case '=':
        expandName(name1,tmpname1);expandName(name2,tmpname2);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-\n",tmpname1,tmpname2);fflush(stdout);}
        parval=findParmValue(tmpname2);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case '+':
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-, name3=->%s<-\n",tmpname1,tmpname2,tmpname3);fflush(stdout);}
        parval=findParmValue(tmpname2)+findParmValue(tmpname3);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case '-':
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-, name3=->%s<-\n",tmpname1,tmpname2,tmpname3);fflush(stdout);}
        parval=findParmValue(tmpname2)-findParmValue(tmpname3);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case '*':
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-, name3=->%s<-\n",tmpname1,tmpname2,tmpname3);fflush(stdout);}
        parval=findParmValue(tmpname2)*findParmValue(tmpname3);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case '/':
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-, name3=->%s<-\n",tmpname1,tmpname2,tmpname3);fflush(stdout);}
        parval=findParmValue(tmpname2)/findParmValue(tmpname3);
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(tmpname1,parval);
        break;
       case '(':
        expandName(name1,tmpname1);expandName(name3,tmpname3);
        if(verbose){printf("  expanded name1=->%s<-, name2=->%s<-, name3=->%s<-\n",tmpname1,tmpname2,tmpname3);fflush(stdout);}
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"ABS"))parval=fabs(val1);
        else if(!strcmp(name2,"SQRT"))parval=sqrt(val1);
        else if(!strcmp(name2,"EXP"))parval=exp(val1);
        else if(!strcmp(name2,"LOG"))parval=log(val1);
        else if(!strcmp(name2,"LOG10"))parval=log10(val1);
        else if(!strcmp(name2,"SIN"))parval=sin(val1);
        else if(!strcmp(name2,"COS"))parval=cos(val1);
        else if(!strcmp(name2,"TAN"))parval=tan(val1);
        else if(!strcmp(name2,"ARCSIN"))parval=asin(val1);
        else if(!strcmp(name2,"ARCCOS"))parval=acos(val1);
        else if(!strcmp(name2,"ARCTAN"))parval=atan(val1);
        else if(!strcmp(name2,"HYPSIN"))parval=sinh(val1);
        else if(!strcmp(name2,"HYPCOS"))parval=cosh(val1);
        else if(!strcmp(name2,"HYPTAN"))parval=tanh(val1);
        else {
          fprintf(stderr,"Problem parsing, function \"%s\" is not allowed\n",name2);
          parval=0.;
         }
        if(verbose){printf("  set %s to %f\n",tmpname1,parval);fflush(stdout);}
        setParameter(name1,parval);
        break;
       default:
/* Invalid Code */
        break;
      }
    default:
     break;
   }
  return;
 }

void handleVARIABLES(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  double v;
  int verbose=0;

  if(verbose){printf("handleVARIABLES, code =-->%s<--, name1=-->%s<--\n",code,name1);fflush(stdout);}

  switch(code[0])
   {
    case 'X':
     expandName(name1,tmpname1);
     if(!strcmp(name2,"`SCALE'"))
      {
       addVariable(name1,val1);
      }else{
       addVariable(tmpname1,1.);
      }
     break;

    case 'Z':
     expandName(name1,tmpname1);
     expandName(name3,tmpname3);
     if(!strcmp(name2,"`SCALE'"))
      {
       val1=findParmValue(tmpname3);
       addVariable(name1,val1);
      }else{
       addVariable(tmpname1,1.);
      }
     break;

    case ' ':
     if(!strcmp(name2,"`SCALE'"))
      {
       addVariable(name1,val1);
      }else if(!strcmp(name2,"`INTEGER'"))
      {
       fprintf(stderr,"ERRRRRRORRRRRR! This SIFFILE declares an INTEGER Variable, which is not supported by the API!\n");
       exit(12);
      }else if(!strcmp(name2,"`ZERO-ONE'"))
      {
       fprintf(stderr,"ERRRRRRORRRRRR! This SIFFILE declares an INTEGER Variable, which is not supported by the API!\n");
       exit(12);
      }else if(!strcmp(name2,""))
      {
       addVariable(name1,1.);
      }else{
       addVariable(name1,1.);
       
      }
     break;
    default:
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }
  if(verbose){printf("  done handleVARIABLES\n");fflush(stdout);}
  return;
 }

void handleVARIABLES2(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  double v;
  int iv;
  int verbose=0;

/* For when the ROWS have already been defined */

  if(verbose){printf("handleVARIABLES2, code =-->%s<--, name1=-->%s<--\n",code,name1);fflush(stdout);}

  switch(code[0])
   {
    case 'X':
     expandName(name1,tmpname1);
     expandName(name2,tmpname2);
     expandName(name3,tmpname3);
     if(!strcmp(name2,"`SCALE'"))
      {
       addVariableNoError(name1,val1);
      }else{
       addVariableNoError(tmpname1,1.);
       iv=findVariableNumber(tmpname1);
       setLinearElementOfGroup(tmpname2,iv,val1);
       if(strcmp(tmpname3,""))setLinearElementOfGroup(tmpname3,iv,val2);
      }
     break;

    case 'Z':
     expandName(name1,tmpname1);
     expandName(name3,tmpname3);
     if(!strcmp(name2,"`SCALE'"))
      {
       val1=findParmValue(tmpname3);
       addVariableNoError(name1,val1);
      }else{
       addVariableNoError(tmpname1,1.);
       iv=findVariableNumber(tmpname1);
       val1=findParmValue(tmpname3);
       setLinearElementOfGroup(tmpname2,iv,val1);
      }
     break;

    case ' ':
     if(!strcmp(name2,"`SCALE'"))
      {
       if(verbose){printf(" name2=`SCALE'\n");fflush(stdout);}
       addVariableNoError(name1,val1);
      }else if(!strcmp(name2,"`INTEGER'"))
      {
       fprintf(stderr,"ERRRRRRORRRRRR! This SIFFILE declares an INTEGER Variable, which is not supported by the API!\n");
       exit(12);
      }else if(!strcmp(name2,"`ZERO-ONE'"))
      {
       fprintf(stderr,"ERRRRRRORRRRRR! This SIFFILE declares an INTEGER Variable, which is not supported by the API!\n");
       exit(12);
      }else if(!strcmp(name2,""))
      {
       if(verbose){printf(" name2=\"\"\n");fflush(stdout);}
       addVariableNoError(name1,1.);
      }else{
       if(verbose){printf(" name2=\"%s\"\n",name2);fflush(stdout);}
       addVariableNoError(name1,1.);
       iv=findVariableNumber(name1);
       setLinearElementOfGroup(name2,iv,val1);
       if(strcmp(name3,""))setLinearElementOfGroup(name3,iv,val2);
      }
     break;
    default:
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }
  if(verbose){printf("  done handleVARIABLES\n");fflush(stdout);}
  return;
 }

void handleGROUPS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int c;
  int ig;
  int iv;
  NLVector a;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;

  if(verbose){printf("handleGROUPS, code =-->%s<--, name1=-->%s<--\n",code,name1);fflush(stdout);}

  switch(code[0])
   {
    case 'N':
     if(verbose){printf("code N\n");fflush(stdout);}
     if(addGroup(name1)){NLPAddGroupToObjective(P,name1);
                         setGroupGType(name1,'O');}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
      {
       setGroupScale(name1,val1);
      }else if(name2[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name2);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
       val1+=NLVGetC(a,iv);
       NLVSetC(a,iv,val1);
      }
     if(name3[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name3);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name2,iv,val2);fflush(stdout);}
       val2+=NLVGetC(a,iv);
       NLVSetC(a,iv,val2);
      }
     break;
    case 'G':
     if(verbose){printf("code G\n");fflush(stdout);}
     if(addGroup(name1)){c=NLPAddNonlinearInequalityConstraint(P,name1);
                         setGroupGType(name1,'G');
                         setGroupConstraint(name1,c);
                         NLPSetInequalityConstraintLowerBound(P,c,0.);
                         NLPUnSetInequalityConstraintUpperBound(P,c);}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
      {
       setGroupScale(name1,val1);
      }else if(name2[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name2);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
       val1+=NLVGetC(a,iv);
       NLVSetC(a,iv,val1);
      }
     if(name3[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name3);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
       val2+=NLVGetC(a,iv);
       NLVSetC(a,iv,val2);
      }
     break;
    case 'L':
     if(verbose){printf("code L\n");fflush(stdout);}
     if(addGroup(name1)){c=NLPAddNonlinearInequalityConstraint(P,name1);
                         setGroupGType(name1,'L');
                         setGroupConstraint(name1,c);
                         NLPUnSetInequalityConstraintLowerBound(P,c);
                         NLPSetInequalityConstraintUpperBound(P,c,0.);}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
      {
       setGroupScale(name1,val1);
      }else if(name2[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name2);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
       val1+=NLVGetC(a,iv);
       NLVSetC(a,iv,val1);
      }
     if(name3[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name3);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
       val2+=NLVGetC(a,iv);
       NLVSetC(a,iv,val2);
      }
     break;
    case 'E':
     if(verbose){printf("code E, name2=%s\n",name2);fflush(stdout);}
     if(addGroup(name1))NLPAddNonlinearEqualityConstraint(P,name1);
                         setGroupGType(name1,'E');
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
      {
       setGroupScale(name1,val1);
      }else if(name2[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name2);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
       val1+=NLVGetC(a,iv);
       NLVSetC(a,iv,val1);
      }
     if(name3[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name3);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
       val2+=NLVGetC(a,iv);
       NLVSetC(a,iv,val2);
      }
     break;
    case 'J':
     if(verbose){printf("code J\n");fflush(stdout);}
     if(addGroup(name1)){NLPAddMinMaxConstraint(P,name1);
                         setGroupGType(name1,'J');}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
      {
       setGroupScale(name1,val1);
      }else if(name2[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name2);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
       val1+=NLVGetC(a,iv);
       NLVSetC(a,iv,val1);
      }
     if(name3[0]!=0x0)
      {
       if(!NLPIsGroupASet(P,ig))
        {
         a=NLCreateVector(nv);
         NLPSetGroupA(P,ig,a);
         NLFreeVector(a);
        }
       a=NLPGetGroupA(P,ig);
       iv=findVariableNumber(name3);
       if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
       val2+=NLVGetC(a,iv);
       NLVSetC(a,iv,val2);
      }
     break;
    case 'X':
     if(verbose){printf("code X.\n");fflush(stdout);}
     expandName(name1,tmpname1);
     expandName(name2,tmpname2);
     expandName(name3,tmpname3);
     if(verbose){printf(" expanded code =-->%s<--, name1=-->%s<--\n",code,tmpname1);fflush(stdout);}
     switch(code[1])
      {
       case 'N':
        if(verbose){printf("code XN\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){NLPAddGroupToObjective(P,tmpname1);
                               setGroupGType(tmpname1,'O');}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'G':
        if(verbose){printf("code XG\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'G');
                               setGroupConstraint(tmpname1,c);
                               NLPSetInequalityConstraintLowerBound(P,c,0.);
                               NLPUnSetInequalityConstraintUpperBound(P,c);}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'L':
        if(verbose){printf("code XL\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'L');
                               setGroupConstraint(tmpname1,c);
                               NLPUnSetInequalityConstraintLowerBound(P,c);
                               NLPSetInequalityConstraintUpperBound(P,c,0.);}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'E':
        if(verbose){printf("code XE\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){NLPAddNonlinearEqualityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'E');}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,tmpname1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,tmpname1,iv,val2);fflush(stdout);}
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'J':
        if(verbose){printf("code XJ\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){c=NLPAddMinMaxConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'J');}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val2);fflush(stdout);}
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       default:
/* Invalid */
        break;
      }
     break;
    case 'Z':
     if(verbose){printf("code Z.\n");fflush(stdout);}
     switch(code[1])
      {
       case 'N':
/* ZN GDA(I)    'SCALE'                  WG(I)
name1=GDA(I), tmpname1=GDA1
name2='SCALE', tmpname2='SCALE'
name3=WG(I), tmpname3=WG1
*/
        if(verbose){printf("code ZN\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  tmpname1 -->%s<--\n",tmpname1);fflush(stdout);}
        if(verbose){printf("  tmpname2 -->%s<--\n",tmpname2);fflush(stdout);}
        if(verbose){printf("  tmpname3 -->%s<--\n",tmpname3);fflush(stdout);}
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){NLPAddGroupToObjective(P,tmpname1);
                               setGroupGType(tmpname1,'O');}
        if(verbose){printf("  findGroupNumber(%s)\n",tmpname1);fflush(stdout);}
        ig=findGroupNumber(tmpname1);
        if(strcmp(tmpname3,""))
         {
          if(verbose){printf("  findParmValue(%s)\n",tmpname3);fflush(stdout);}
          val1=findParmValue(tmpname3);
          if(verbose){printf("  Value=%lf\n",val1);fflush(stdout);}
         }else val1=0.;
        if(!strcmp(name2,"'SCALE'"))
         {
          if(verbose){printf("  name2 is 'SCALE'\n");fflush(stdout);}
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(verbose){printf("  tmpname2 is \"\"\n");fflush(stdout);}
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(verbose){printf("  done ZN\n");fflush(stdout);}
        break;
       case 'G':
        if(verbose){printf("code ZG\n");fflush(stdout);}
        expandName(name1,tmpname1);
        expandName(name2,tmpname2);
        expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'G');
                               setGroupConstraint(tmpname1,c);
                               NLPSetInequalityConstraintLowerBound(P,c,0.);
                               NLPUnSetInequalityConstraintUpperBound(P,c);}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          val1=findParmValue(tmpname3);
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1=findParmValue(tmpname3);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       case 'L':
        if(verbose){printf("code ZL\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'L');
                               setGroupConstraint(tmpname1,c);
                               NLPUnSetInequalityConstraintLowerBound(P,c);
                               NLPSetInequalityConstraintUpperBound(P,c,0.);}
        ig=findGroupNumber(tmpname1);
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       case 'E':
        if(verbose){printf("code ZE\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){NLPAddNonlinearEqualityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'E');}
        ig=findGroupNumber(tmpname1);
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       case 'J':
        if(verbose){printf("code ZJ\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){NLPAddMinMaxConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'J');}
        ig=findGroupNumber(tmpname1);
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          if(verbose){printf("  group %d(%s), a(%d)=%le\n",ig,name1,iv,val1);fflush(stdout);}
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       default:
/* Invalid */
        break;
      }
     break;
    case 'D':
     if(verbose){printf("code D\n");fflush(stdout);}
/* Invalid */
     break;
    default:
     if(verbose){printf("unknown code %s, pass to PARMS\n",code);fflush(stdout);}
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }
  if(verbose){printf("done handleGROUPS\n");fflush(stdout);}
  return;
 }

void handleGROUPS2(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int c;
  int ig;
  int iv;
  NLVector a;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;

/* For when the VARIABLES have not yet been defined */

  if(verbose){printf("handleGROUPS2, code =-->%s<--, name2=-->%s<--\n",code,name2);fflush(stdout);}
  if(code[0]==' '){code[0]=code[1];code[1]=' ';}
  switch(code[0])
   {
    case 'N':
     if(verbose){printf("code N\n");fflush(stdout);}
     if(addGroup(name1)){NLPAddGroupToObjective(P,name1);
                         setGroupGType(name1,'O');}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
       setGroupScale(name1,val1);
     break;
    case 'G':
     if(verbose){printf("code G\n");fflush(stdout);}
     if(addGroup(name1)){c=NLPAddNonlinearInequalityConstraint(P,name1);
                         setGroupGType(name1,'G');
                         setGroupConstraint(name1,c);
                         NLPSetInequalityConstraintLowerBound(P,c,0.);
                         NLPUnSetInequalityConstraintUpperBound(P,c);}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
       setGroupScale(name1,val1);
     break;
    case 'L':
     if(verbose){printf("code L\n");fflush(stdout);}
     if(addGroup(name1)){c=NLPAddNonlinearInequalityConstraint(P,name1);
                         setGroupGType(name1,'L');
                         setGroupConstraint(name1,c);
                         NLPUnSetInequalityConstraintLowerBound(P,c);
                         NLPSetInequalityConstraintUpperBound(P,c,0.);}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
       setGroupScale(name1,val1);
     break;
    case 'E':
     if(verbose){printf("code E, name2=%s\n",name2);fflush(stdout);}
     if(addGroup(name1))NLPAddNonlinearEqualityConstraint(P,name1);
                         setGroupGType(name1,'E');
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
       setGroupScale(name1,val1);
     break;
    case 'J':
     if(verbose){printf("code J\n");fflush(stdout);}
     if(addGroup(name1)){NLPAddMinMaxConstraint(P,name1);
                         setGroupGType(name1,'J');}
     ig=findGroupNumber(name1);
     if(!strcmp(name2,"'SCALE'"))
       setGroupScale(name1,val1);
     break;
    case 'X':
     if(verbose){printf("code X.\n");fflush(stdout);}
     switch(code[1])
      {
       case 'N':
        if(verbose){printf("code XN\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){NLPAddGroupToObjective(P,tmpname1);
                               setGroupGType(tmpname1,'O');}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'G':
        if(verbose){printf("code XG\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'G');
                               setGroupConstraint(tmpname1,c);
                               NLPSetInequalityConstraintLowerBound(P,c,0.);
                               NLPUnSetInequalityConstraintUpperBound(P,c);}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'L':
        if(verbose){printf("code XL\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'L');
                               setGroupConstraint(tmpname1,c);
                               NLPUnSetInequalityConstraintLowerBound(P,c);
                               NLPSetInequalityConstraintUpperBound(P,c,0.);}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'E':
        if(verbose){printf("code XE\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){NLPAddNonlinearEqualityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'E');}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       case 'J':
        if(verbose){printf("code XJ\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(addGroup(tmpname1)){c=NLPAddMinMaxConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'J');}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(strcmp(tmpname3,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname3);
          val2+=NLVGetC(a,iv);
          NLVSetC(a,iv,val2);
         }
        break;
       default:
/* Invalid */
        break;
      }
     break;
    case 'Z':
     if(verbose){printf("code Z.\n");fflush(stdout);}
     switch(code[1])
      {
       case 'N':
/* ZN GDA(I)    'SCALE'                  WG(I)
name1=GDA(I), tmpname1=GDA1
name2='SCALE', tmpname2='SCALE'
name3=WG(I), tmpname3=WG1
*/
        if(verbose){printf("code ZN\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  tmpname1 -->%s<--\n",tmpname1);fflush(stdout);}
        if(verbose){printf("  tmpname2 -->%s<--\n",tmpname2);fflush(stdout);}
        if(verbose){printf("  tmpname3 -->%s<--\n",tmpname3);fflush(stdout);}
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){NLPAddGroupToObjective(P,tmpname1);
                               setGroupGType(tmpname1,'O');}
        if(verbose){printf("  findGroupNumber(%s)\n",tmpname1);fflush(stdout);}
        ig=findGroupNumber(tmpname1);
        if(strcmp(tmpname3,""))
         {
          if(verbose){printf("  findParmValue(%s)\n",tmpname3);fflush(stdout);}
          val1=findParmValue(tmpname3);
          if(verbose){printf("  Value=%lf\n",val1);fflush(stdout);}
         }else val1=0.;
        if(!strcmp(name2,"'SCALE'"))
         {
          if(verbose){printf("  name2 is 'SCALE'\n");fflush(stdout);}
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(verbose){printf("  tmpname2 is \"\"\n");fflush(stdout);}
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        if(verbose){printf("  done ZN\n");fflush(stdout);}
        break;
       case 'G':
        if(verbose){printf("code ZG\n");fflush(stdout);}
        expandName(name1,tmpname1);
        expandName(name2,tmpname2);
        expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'G');
                               setGroupConstraint(tmpname1,c);
                               NLPSetInequalityConstraintLowerBound(P,c,0.);
                               NLPUnSetInequalityConstraintUpperBound(P,c);}
        ig=findGroupNumber(tmpname1);
        if(!strcmp(name2,"'SCALE'"))
         {
          val1=findParmValue(tmpname3);
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1=findParmValue(tmpname3);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       case 'L':
        if(verbose){printf("code ZL\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){c=NLPAddNonlinearInequalityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'L');
                               setGroupConstraint(tmpname1,c);
                               NLPUnSetInequalityConstraintLowerBound(P,c);
                               NLPSetInequalityConstraintUpperBound(P,c,0.);}
        ig=findGroupNumber(tmpname1);
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       case 'E':
        if(verbose){printf("code ZE\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){NLPAddNonlinearEqualityConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'E');}
        ig=findGroupNumber(tmpname1);
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       case 'J':
        if(verbose){printf("code ZJ\n");fflush(stdout);}
        expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
        if(verbose){printf("  group -->%s<--\n",tmpname1);fflush(stdout);}
        if(addGroup(tmpname1)){NLPAddMinMaxConstraint(P,tmpname1);
                               setGroupGType(tmpname1,'J');}
        ig=findGroupNumber(tmpname1);
        val1=findParmValue(tmpname3);
        if(!strcmp(name2,"'SCALE'"))
         {
          setGroupScale(tmpname1,val1);
         }else if(strcmp(tmpname2,""))
         {
          if(!NLPIsGroupASet(P,ig))
           {
            a=NLCreateVector(nv);
            NLPSetGroupA(P,ig,a);
            NLFreeVector(a);
           }
          a=NLPGetGroupA(P,ig);
          iv=findVariableNumber(tmpname2);
          val1+=NLVGetC(a,iv);
          NLVSetC(a,iv,val1);
         }
        break;
       default:
/* Invalid */
        break;
      }
     break;
    case 'D':
     if(verbose){printf("code D\n");fflush(stdout);}
/* Invalid */
     break;
    default:
     if(verbose){printf("unknown code %s, pass to PARMS\n",code);fflush(stdout);}
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }
  if(verbose){printf("done handleGROUPS\n");fflush(stdout);}
  return;
 }

double defaultCONSTANT=0.;

void handleCONSTANTS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int ig;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;

  if(verbose){printf("handleCONSTANTS, code=->%s<-, name2=->%s<-, name3=->%s<-\n",code,name2,name3);fflush(stdout);}

  switch(code[0])
   {
    case ' ':
     if(verbose){printf(" case ' '\n");fflush(stdout);}
     if(!strcmp(name2,"'DEFAULT'"))
      {
       if(verbose){printf(" name2='DEFAULT'\n");fflush(stdout);}
       defaultCONSTANT=val1;
      }else if(val1Set){
       if(verbose){printf(" val1set, name2=%s, val1=%le\n",name2,val1);fflush(stdout);}
       ig=findGroupNumber(name2);
       NLPSetGroupB(P,ig,val1);
       if(val2Set)
        {
         if(verbose){printf(" val2set as well, name3=%s, val2=%le\n",name3,val2);fflush(stdout);}
         ig=findGroupNumber(name3);
         NLPSetGroupB(P,ig,val2);
        }
      }else{
       if(verbose){printf(" val1 not set, name2=%s, default=%le\n",name2,defaultCONSTANT);fflush(stdout);}
       ig=findGroupNumber(name2);
       NLPSetGroupB(P,ig,defaultCONSTANT);
      }

     break;
    case 'X':
     expandName(name2,tmpname2);
     if(verbose){printf("  expanded, name2=->%s<-\n",tmpname2);fflush(stdout);}
     if(!strcmp(tmpname2,"'DEFAULT'"))
      {
       defaultCONSTANT=val1;
      }else if(val1Set){
       ig=findGroupNumber(tmpname2);
       NLPSetGroupB(P,ig,val1);
       if(val2Set)
        {
         ig=findGroupNumber(tmpname3);
         NLPSetGroupB(P,ig,val2);
        }
      }else{
       ig=findGroupNumber(tmpname2);
       NLPSetGroupB(P,ig,defaultCONSTANT);
      }
     break;
    case 'Z':
     expandName(name2,tmpname2);expandName(name3,tmpname3);
     if(verbose){printf("handleCONSTANTS, name2=->%s<-, name3=->%s<-\n",tmpname2,tmpname3);fflush(stdout);}
     ig=findGroupNumber(tmpname2);
     val1=findParmValue(tmpname3);
     NLPSetGroupB(P,ig,val1);
     break;
    default:
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }

  return;
 }

void handleINEQUALITYBOUNDS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int ig,c;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;

  if(verbose){printf("handleINEQUALITYBOUNDS\n");fflush(stdout);}

  switch(code[0])
   {
    case ' ':
     if(verbose){printf(" case ' '\n");fflush(stdout);}
     ig=findGroupNumber(name2);c=getGroupConstraint(ig);
     if(verbose){printf(" group[%d], type %c, value=%lf\n",ig,getGroupGType(ig),val1);fflush(stdout);}
     if(getGroupGType(ig)=='G'){NLPSetInequalityConstraintUpperBound(P,c,val1);printf("SetLowerBound\n");fflush(stdout);}
      else                     {NLPSetInequalityConstraintLowerBound(P,c,-val1);printf("SetUpperBound\n");fflush(stdout);}
     if(name3[0]!=0x0)
      {
       ig=findGroupNumber(name3);c=getGroupConstraint(ig);
       if(getGroupGType(ig)=='G'){NLPSetInequalityConstraintUpperBound(P,c,val2);printf("SetLowerBound\n");fflush(stdout);}
        else                    {NLPSetInequalityConstraintLowerBound(P,c,-val2);printf("SetUpperBound\n");fflush(stdout);}
      }
     break;
    case 'X':
     if(verbose){printf(" case 'X'\n");fflush(stdout);}
     expandName(name2,tmpname2);expandName(name3,tmpname3);
     ig=findGroupNumber(tmpname2);c=getGroupConstraint(ig);
     if(verbose){printf(" group[%d], type %c\n",ig,getGroupGType(ig));fflush(stdout);}
     if(getGroupGType(ig)=='G')NLPSetInequalityConstraintUpperBound(P,c,val1);
      else                    NLPSetInequalityConstraintLowerBound(P,c,-val1);
     if(name3[0]!=0x0)
      {
       ig=findGroupNumber(tmpname3);c=getGroupConstraint(ig);
       if(getGroupGType(ig)=='G')NLPSetInequalityConstraintUpperBound(P,c,val2);
        else                    NLPSetInequalityConstraintLowerBound(P,c,-val2);
      }
     break;
    case 'Z':
     if(verbose){printf(" case 'Z'\n");fflush(stdout);}
     expandName(name2,tmpname2);expandName(name3,tmpname3);
     ig=findGroupNumber(tmpname2);c=getGroupConstraint(ig);
     if(verbose){printf(" group[%d], type %c\n",ig,getGroupGType(ig));fflush(stdout);}
     val1=findParmValue(tmpname3);
     if(getGroupGType(ig)=='G')NLPSetInequalityConstraintUpperBound(P,c,val1);
      else                    NLPSetInequalityConstraintLowerBound(P,c,-val1);
     NLPSetGroupB(P,ig,val1);
     break;
    default:
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }

  return;
 }

double defaultLO=0.;
double defaultUP=DBL_MAX;

void handleBOUNDS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int iv;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;

  if(verbose){printf("handleBOUNDS\n");fflush(stdout);}

  if(!strcmp(name2,"'DEFAULT'"))
   {
    if(!strcmp(code,"LO")||!strcmp(code,"XL"))defaultLO=val1;
     else if(!strcmp(code,"UP")||!strcmp(code,"XU"))defaultUP=val1;
     else if(!strcmp(code,"FX")||!strcmp(code,"XX")){defaultLO=val1;defaultUP=val1;}
     else if(!strcmp(code,"FR")||!strcmp(code,"XR")){defaultLO=-DBL_MAX;defaultUP=DBL_MAX;}
     else if(!strcmp(code,"MI")||!strcmp(code,"XM")){defaultLO=-DBL_MAX;defaultUP=0;}
     else if(!strcmp(code,"PL")||!strcmp(code,"XP")){defaultLO=0.;defaultUP=DBL_MAX;}
    for(iv=0;iv<nv;iv++)
     {
      if(defaultLO>-DBL_MAX)NLPSetLowerSimpleBound(P,iv,defaultLO);
       else NLPUnSetLowerSimpleBound(P,iv);
      if(defaultUP< DBL_MAX)NLPSetUpperSimpleBound(P,iv,defaultUP);
       else NLPUnSetUpperSimpleBound(P,iv);
     }
    return;
   }

  if(      !strcmp(code,"LO")){
                               iv=findVariableNumber(name2);
                               NLPSetLowerSimpleBound(P,iv,val1);
                               }
   else if(!strcmp(code,"UP")){
                               iv=findVariableNumber(name2);
                               NLPSetUpperSimpleBound(P,iv,val1);}
   else if(!strcmp(code,"FX")){
                               iv=findVariableNumber(name2);
                               NLPSetSimpleBounds(P,iv,val1,val1);}
   else if(!strcmp(code,"FR")){
                               iv=findVariableNumber(name2);
                               NLPUnSetSimpleBounds(P,iv);}
   else if(!strcmp(code,"MI")){
                               iv=findVariableNumber(name2);
                               NLPUnSetLowerSimpleBound(P,iv);}
   else if(!strcmp(code,"PL")){
                               iv=findVariableNumber(name2);
                               NLPUnSetUpperSimpleBound(P,iv);}
   else if(!strcmp(code,"XL")){expandName(name2,tmpname2);
                               iv=findVariableNumber(tmpname2);
                               NLPSetLowerSimpleBound(P,iv,val1);
                              }
   else if(!strcmp(code,"XU")){expandName(name2,tmpname2);
                               iv=findVariableNumber(tmpname2);
                               NLPSetUpperSimpleBound(P,iv,val1);
                              }
   else if(!strcmp(code,"XX")){expandName(name2,tmpname2);
                               iv=findVariableNumber(tmpname2);
                               NLPSetSimpleBounds(P,iv,val1,val1);
                              }
   else if(!strcmp(code,"XR")){expandName(name2,tmpname2);
                               iv=findVariableNumber(tmpname2);
                               NLPUnSetSimpleBounds(P,iv);
                              }
   else if(!strcmp(code,"XM")){expandName(name2,tmpname2);
                               iv=findVariableNumber(tmpname2);
                               NLPUnSetLowerSimpleBound(P,iv);
                              }
   else if(!strcmp(code,"XP")){expandName(name2,tmpname2);
                               iv=findVariableNumber(tmpname2);
                               NLPUnSetUpperSimpleBound(P,iv);
                              }
   else if(!strcmp(code,"ZL")){expandName(name2,tmpname2);expandName(name3,tmpname3); 
                               iv=findVariableNumber(tmpname2);
                               val1=findParmValue(tmpname3);
                               NLPSetLowerSimpleBound(P,iv,val1);
                              }
   else if(!strcmp(code,"ZU")){expandName(name2,tmpname2);expandName(name3,tmpname3); 
                               iv=findVariableNumber(tmpname2);
                               val1=findParmValue(tmpname3);
                               NLPSetUpperSimpleBound(P,iv,val1);
                              }
   else if(!strcmp(code,"ZX")){expandName(name2,tmpname2);expandName(name3,tmpname3); 
                               iv=findVariableNumber(tmpname2);
                               val1=findParmValue(tmpname3);
                               NLPSetSimpleBounds(P,iv,val1,val1);
                              }
   else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);

  return;
 }

void handleSTART_POINT(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int iv;
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  double defaultV;
  int verbose=0;
  int nc;
  int ib;

  if(verbose){printf("handleSTART_POINT, code =-->%s<--, name1=-->%s<-- , name2=-->%s<--, name3=-->%s<--\n",code,name1,name2,name3);fflush(stdout);}
  nc=NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P);

  if(!strcmp(name2,"'DEFAULT'"))
   {
    ib=0;
    if(!strcmp(code,"  ")){defaultV=val1;ib=1;}
     else if(!strcmp(code,"X ")){defaultV=val1;ib=1;}
     else if(!strcmp(code,"Z ")){defaultV=findParmValue(name3);ib=1;}

    if(ib)
     {
      if(verbose){printf("  default, code %s %lf\n",code,defaultV);fflush(stdout);}
      for(iv=0;iv<nv;iv++)xStart[iv]=defaultV;
      for(iv=0;iv<nc;iv++)lStart[iv]=defaultV;
      return;
     }

    iv=0;
    if(!strcmp(code,"V ")){defaultV=val1;iv=1;}
     else if(!strcmp(code,"XV")){defaultV=val1;iv=1;}
     else if(!strcmp(code,"ZV")){defaultV=findParmValue(name3);iv=1;}

    if(iv)
     {
      if(verbose){printf("  default, code %s %lf\n",code,defaultV);fflush(stdout);}
      for(iv=0;iv<nv;iv++)xStart[iv]=defaultV;
      return;
     }

    defaultV=val1;
    if(!strcmp(code,"M ")){defaultV=val1;iv=1;}
     else if(!strcmp(code,"XM")){defaultV=val1;iv=1;}
     else if(!strcmp(code,"ZM")){defaultV=findParmValue(name3);}

    if(verbose){printf("  default, code %s %lf\n",code,defaultV);fflush(stdout);}

    for(iv=0;iv<nc;iv++)lStart[iv]=defaultV;
    return;
   }

  switch(code[0])
   {
    case 'V':
     iv=findVariableNumber(name2);
     xStart[iv]=val1;
     if(name3[0]!=0x0)
      {
       iv=findVariableNumber(name3);
       xStart[iv]=val2;
      }
     break;
    case 'M':
     iv=findConstraintNumber(name2);
     lStart[iv]=val1;
     if(name3[0]!=0x0)
      {
       iv=findConstraintNumber(name3);
       lStart[iv]=val2;
      }
     break;
    case ' ':
     if(verbose){printf("  code blank\n");fflush(stdout);}
     iv=findVariableNumberNoError(name2);
     if(iv>-1)
      {
       xStart[iv]=val1;
       if(name3[0]!=0x0)
        {
         iv=findVariableNumber(name3);
         xStart[iv]=val2;
        }
      }else{
       iv=findConstraintNumber(name2);
       lStart[iv]=val1;
       if(name3[0]!=0x0)
        {
         iv=findConstraintNumber(name3);
         lStart[iv]=val2;
        }
      }
     break;
    case 'X':
     switch(code[1])
      {
       case 'V':
        expandName(name2,tmpname2);expandName(name3,tmpname3);
        iv=findVariableNumber(tmpname2);
        xStart[iv]=val1;
        if(name3[0]!=0x0)
         {
          iv=findVariableNumber(tmpname3);
          xStart[iv]=val2;
         }
        break;
       case 'M':
        expandName(name2,tmpname2);expandName(name3,tmpname3);
        iv=findGroupNumber(tmpname2);
        lStart[iv]=val1;
        if(name3[0]!=0x0)
         {
          iv=findGroupNumber(tmpname3);
          lStart[iv]=val2;
         }
        break;
       case ' ':
        expandName(name2,tmpname2);expandName(name3,tmpname3);
        iv=findVariableNumber(tmpname2);
        if(iv>-1)
         {
          xStart[iv]=val1;
          if(name3[0]!=0x0)
           {
            iv=findVariableNumber(name3);
            xStart[iv]=val2;
           }
         }else{
          iv=findConstraintNumber(name2);
          lStart[iv]=val1;
          if(name3[0]!=0x0)
           {
            iv=findConstraintNumber(name3);
            lStart[iv]=val2;
           }
         }
        break;
      }
     break;
    case 'Z':
     switch(code[1])
      {
       case 'V':
       case ' ':
        expandName(name2,tmpname2);expandName(name3,tmpname3);
        iv=findVariableNumber(tmpname2);
        val1=findParmValue(tmpname3);
        xStart[iv]=val1;
        break;
       case 'M':
        expandName(name2,tmpname2);expandName(name3,tmpname3);
        iv=findGroupNumber(tmpname2);
        val1=findParmValue(tmpname3);
        lStart[iv]=val1;
        break;
      }
     break;
    default:
     if(verbose){printf("unknown code %s, pass to PARMS\n",code);fflush(stdout);}
     handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
     break;
   }

  return;
 }

void handleELEMENT_TYPE(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int verbose=0;

  if(verbose){printf("handleELEMENT_TYPE\n");fflush(stdout);}

  if(      !strcmp(code,"EV"))
   {
  if(verbose){printf("      EV, -->%s<--\n",name1);fflush(stdout);}
    addElementType(name1);
    if(name2[0]!=0x0)addElementVariableToElementType(name1,name2);
    if(name3[0]!=0x0)addElementVariableToElementType(name1,name3);
    fflush(stdout);
   }
   else if(!strcmp(code,"IV"))
   {
  if(verbose){printf("      IV, -->%s<--\n",name1);fflush(stdout);}
    addElementType(name1);
    if(name2[0]!=0x0)addInternalVariableToElementType(name1,name2);
    if(name3[0]!=0x0)addInternalVariableToElementType(name1,name3);
   }
   else if(!strcmp(code,"EP"))
   {
  if(verbose){printf("      EP, -->%s<--\n",name1);fflush(stdout);}
    addElementType(name1);
    if(name2[0]!=0x0)addParameterToElementType(name1,name2);
    if(name3[0]!=0x0)addParameterToElementType(name1,name3);
   }
   else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);

  return;
 }

void handleELEMENT_USES(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int iev,iv,iep,ie;
  int verbose=0;

  if(verbose){printf("handleELEMENT_USES, code %s\n",code);fflush(stdout);}

  if(      !strcmp(code,"T "))
          {
           if(!strcmp(name1,"'DEFAULT'"))
            {
             strcpy(defaultElementType,name2);
             if(verbose){printf("default ElementType is %s\n",name2);fflush(stdout);}
            }else{
            if(name2[0]==0x0)
             strcpy(name2,defaultElementType);
            addElement(name1,name2);
            }
          }
   else if(!strcmp(code,"V "))
          {
           addElement(name1,defaultElementType);
           ie=findElementNumber(name1);
           iev=findElementVariableNumber((elements[ie]).type,name2);
           iv=findVariableNumber(name3);
           setElementVariable(name1,iev,iv);
          }
   else if(!strcmp(code,"P "))
          {
           addElement(name1,defaultElementType);
           ie=findElementNumber(name1);
           iep=findElementParameterNumber((elements[ie]).type,name2);
           setElementParameter(name1,iep,val1);
           if(name3[0]!=0x0)
            {
             ie=findElementNumber(name1);
             iep=findElementParameterNumber((elements[ie]).type,name3);
             setElementParameter(name1,iep,val2);
            }
          }
   else if(!strcmp(code,"XT")){expandName(name1,tmpname1);expandName(name2,tmpname2);
                               if(name2[0]==0x0)
                               strcpy(tmpname2,defaultElementType);
                               if(!strcmp(name1,"'DEFAULT'"))
                                 strcpy(defaultElementType,name2);
                                else{
                                 if(name2[0]==0x0)
                                   strcpy(tmpname2,defaultElementType);
                                 addElement(tmpname1,tmpname2);
                                }}
   else if(!strcmp(code,"XV")||!strcmp(code,"ZV")){expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                                 addElement(tmpname1,defaultElementType);
                                 ie=findElementNumber(tmpname1);
                                 iev=findElementVariableNumber((elements[ie]).type,tmpname2);
                                 iv=findVariableNumber(tmpname3);
                                 setElementVariable(tmpname1,iev,iv);
                                }
   else if(!strcmp(code,"XP")){expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                               addElement(tmpname1,defaultElementType);
                               ie=findElementNumber(tmpname1);
                               iep=findElementParameterNumber((elements[ie]).type,tmpname2);
                               setElementParameter(tmpname1,iep,val1);
                               if(name3[0]!=0x0)
                                {
                                 ie=findElementNumber(tmpname1);
                                 iep=findElementParameterNumber((elements[ie]).type,tmpname3);
                                 setElementParameter(tmpname1,iep,val2);
                                }
                               }
   else if(!strcmp(code,"ZP")){expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                               if(verbose){printf("ELEMENT USES, code ZP, %s %s %s\n",tmpname1,tmpname2,tmpname3);fflush(stdout);}
                               addElement(tmpname1,defaultElementType);
                               ie=findElementNumber(tmpname1);
                               iep=findElementParameterNumber((elements[ie]).type,tmpname2);
                               val1=findParmValue(tmpname3);
                               setElementParameter(tmpname1,iep,val1);
                              }
   else{
        if(verbose){printf("Handle Parms %s %s %s %le %s %le\n",code,name1,name2,val1,name3,val2);fflush(stdout);}
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
       }

  return;
 }

void handleGROUP_TYPE(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int verbose=0;

  if(verbose){printf("handleGROUP_TYPE %s %s %s %le %s %le val1Set=%d, val2Set=%d\n",code,name1,name2,val1,name3,val2,val1Set,val2Set);fflush(stdout);}

  if(      !strcmp(code,"GV"))
   {
    if(verbose){printf(" code GV\n");fflush(stdout);}
    addGroupType(name1,name2);
   }else if(!strcmp(code,"GP"))
   {
    if(verbose){printf(" code GP, group %s, parm %s\n",name1,name2);fflush(stdout);}
    addParameterToGroupType(name1,name2);
    if(name3[0]!=0x0)addParameterToGroupType(name1,name3);
   }else{
    if(verbose){printf(" unknown code, passing it down to handleParms\n");fflush(stdout);}
    handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
   }

  return;
 }

void handleGROUP_USES(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  char tmpname1[20];
  char tmpname2[20];
  char tmpname3[20];
  int verbose=0;

  if(verbose){printf("handleGROUP_USES %s %s %s %le %s %le val1Set=%d, val2Set=%d\n",code,name1,name2,val1,name3,val2,val1Set,val2Set);fflush(stdout);}

  if(code[0]=='T'||code[1]=='T')
    if(code[0]=='T')
     {
      if(!strcmp(name1,"'DEFAULT'"))
       {
        strcpy(defaultGroupType,name2);
        setAllGroupTypes(defaultGroupType);
       }else{
        if(name2[0]==0x0)strcpy(name2,defaultGroupType);
        setGroupType(name1,name2);
        if(verbose){printf("  setGroupType(%s,%s)\n",name1,name2);fflush(stdout);}
       }
     }else if(code[0]=='X'){expandName(name1,tmpname1);expandName(name2,tmpname2);
                            if(!strcmp(name1,"'DEFAULT'"))
                             {
                              strcpy(defaultGroupType,tmpname2);
                              setAllGroupTypes(defaultGroupType);
                             }else{
                                  if(name2[0]==0x0)strcpy(tmpname2,defaultGroupType);
                                  setGroupType(tmpname1,tmpname2);}
                                 if(verbose){printf("  setGroupType(%s,%s)\n",tmpname1,tmpname2);fflush(stdout);}
                            }
     else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
   else if(code[0]=='E'||code[1]=='E')
    if(code[0]=='E'){if(!val1Set)val1=1.;if(!val2Set)val2=1.;
                     if(name2[0]!=0x0)addElementToGroup(name1,name2,val1);
                     if(name3[0]!=0x0)addElementToGroup(name1,name3,val2);
                    }
     else if(code[0]=='X'){expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                           if(!val1Set)val1=1.;if(!val2Set)val2=1.;
                           if(name2[0]!=0x0)addElementToGroup(tmpname1,tmpname2,val1);
                           if(name3[0]!=0x0)addElementToGroup(tmpname1,tmpname3,val2);
                            }
     else if(code[0]=='Z'){expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                           val1=findParmValue(tmpname3);
                           if(tmpname2[0]!=0x0)addElementToGroup(tmpname1,tmpname2,val1);
                            }
     else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
   else if(code[0]=='P'||code[1]=='P')
    if(code[0]=='P'){setGroupParm(name1,name2,val1);
                     if(name3[0]!=0x0)setGroupParm(name1,name3,val2);}
     else if(code[0]=='X'){expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                           setGroupParm(tmpname1,tmpname2,val2);
                           if(name3[0]!=0x0)setGroupParm(tmpname1,tmpname3,val2);
                            }
     else if(code[0]=='Z'){
                           expandName(name1,tmpname1);expandName(name2,tmpname2);expandName(name3,tmpname3);
                           val1=findParmValue(tmpname3);
                           setGroupParm(tmpname1,tmpname2,val1);
                            }
     else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
   else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);

  return;
 }

void handleOBJECT_BOUND(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int verbose=0;

  if(verbose){printf("handleOBJECT_BOUND\n");fflush(stdout);}

/*if(      !strcmp(code,"LO")){}
   else if(!strcmp(code,"UP")){}
   else if(!strcmp(code,"XL")){}
   else if(!strcmp(code,"XU")){}
   else if(!strcmp(code,"ZU")){}
   else if(!strcmp(code,"ZL")){}
   else handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
*/
  return;
 }

int groupStarted=0;
static char groupType[80];
static int igt;
void handleGROUPFUNCTIONS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int verbose=0;
  char gline[80];
  int i;

  if(verbose){printf("handleGROUPFUNCTIONS\n");fflush(stdout);}

  switch(substate)
   {
    case 'T':
/* This defines things that are declared in all element functions */
     switch(code[0])
      {
       case 'I':
        strcpy(gline,"      integer ");
        strcat(gline,name1);
        AddGroupDeclLine(gline);
        break;
       case 'R':
        strcpy(gline,"      double precision ");
        strcat(gline,name1);
        AddGroupDeclLine(gline);
        break;
       case 'L':
        strcpy(gline,"      logical ");
        strcat(gline,name1);
        AddGroupDeclLine(gline);
        break;
       case 'M':
        strcpy(gline,"      external ");
        strcat(gline,name1);
/*      AddGroupDeclLine(gline);*/
        if(!strcmp(name1,"MAX"))needMax=1;
        break;
       case 'F':
        strcpy(gline,"      external ");
        strcat(gline,name1);
/*      AddGroupDeclLine(gline);*/
        if(!strcmp(name1,"MAX"))needMax=1;
        break;
       default:
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        break;
      }
     break;
    case 'G':
/* This gives code for assigning values to those things declared in all group functions */
     switch(code[0])
      {
       case 'A':
        if(code[1]==' ')
         {
          strcpy(gline,"      ");
          strcat(gline,name1);
          strcat(gline,"=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"     @");
          strcat(gline,line+24);
         }
        AddGroupCodeLine(gline);
        break;
       case 'I':
        if(code[1]==' ')
         {
          sprintf(gline,"      if(%s)",name1);
          AddLocalGroupCodeLine(gline);
          strcpy(gline,"     @");
          strcat(gline,name2);
          strcat(gline,"=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"     @");
          strcat(gline,line+24);
         }
        AddGroupCodeLine(gline);
        break;
       case 'E':
        if(code[1]!=' ')
         {
          sprintf(gline,"      if(%s)",name1);
          AddGroupCodeLine(gline);
          strcpy(gline,"     @");
          strcat(gline,name2);
          strcat(gline,"=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"     @");
          strcat(gline,line+24);
         }
        AddGroupCodeLine(gline);
        break;
       default:
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        break;
      }
     break;
    case 'I':
     switch(code[0])
      {
       case 'T':
        if(groupStarted)
         {
          char tmp[256];
          strcpy(tmp,groupType);rectString(tmp);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      subroutine Gf%s%s(xVariable,pParameters,FuncValue)\n",rectproblemName,tmp);
          fprintf(FOUT,"      double precision xVariable\n");
          if((grouptypes[igt]).nparms>0)
            fprintf(FOUT,"      double precision pParameters(%d)\n",(grouptypes[igt]).nparms);
          fprintf(FOUT,"      double precision FuncValue\n");
          fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).var);
          for(i=0;i<(grouptypes[igt]).nparms;i++)
            fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).parms[i]);
          fprintf(FOUT,"\n");
          for(i=0;i<nGroupDeclLines;i++)
            fprintf(FOUT,"%s\n",groupDeclLines[i]);
          fprintf(FOUT,"\n");
/* Globals */
          fprintf(FOUT,"C     GLOBALS \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nGroupCodeLines;i++)
            fprintf(FOUT,"%s\n",groupCodeLines[i]);
          fprintf(FOUT,"\n");
/* Local Code */
          fprintf(FOUT,"C     Local Code \n");
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      %s=xVariable\n",(grouptypes[igt]).var);
          for(i=0;i<(grouptypes[igt]).nparms;i++)
            fprintf(FOUT,"      %s=pParameters(%d)\n",(grouptypes[igt]).parms[i],i+1);
          for(i=0;i<nLocalGroupCodeLines;i++)
            if((localGroupCodeLines[i])[0]!='G'&&(localGroupCodeLines[i])[0]!='H')
              fprintf(FOUT," %s\n",localGroupCodeLines[i]+1);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      RETURN\n");
          fprintf(FOUT,"      END\n");

          fprintf(FOUT,"\n");
          fprintf(FOUT,"      subroutine Gg%s%s(xVariable,pParameters,FuncValue)\n",rectproblemName,tmp);
          fprintf(FOUT,"      double precision xVariable\n");
          if((grouptypes[igt]).nparms>0)
            fprintf(FOUT,"      double precision pParameters(%d)\n",(grouptypes[igt]).nparms);
          fprintf(FOUT,"      double precision FuncValue\n");
          fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).var);
          for(i=0;i<(grouptypes[igt]).nparms;i++)
            fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).parms[i]);
          fprintf(FOUT,"\n");
          for(i=0;i<nGroupDeclLines;i++)
            fprintf(FOUT,"%s\n",groupDeclLines[i]);
          fprintf(FOUT,"\n");
/* Globals */
          fprintf(FOUT,"C     GLOBALS \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nGroupCodeLines;i++)
            fprintf(FOUT,"%s\n",groupCodeLines[i]);
          fprintf(FOUT,"\n");
/* Local Code */
          fprintf(FOUT,"C     Local Code \n");
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      %s=xVariable\n",(grouptypes[igt]).var);
          for(i=0;i<(grouptypes[igt]).nparms;i++)
            fprintf(FOUT,"      %s=pParameters(%d)\n",(grouptypes[igt]).parms[i],i+1);
          for(i=0;i<nLocalGroupCodeLines;i++)
            if((localGroupCodeLines[i])[0]!='F'&&(localGroupCodeLines[i])[0]!='H')
              fprintf(FOUT," %s\n",localGroupCodeLines[i]+1);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      RETURN\n");
          fprintf(FOUT,"      END\n");
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      subroutine Gh%s%s(xVariable,pParameters,FuncValue)\n",rectproblemName,tmp);
          fprintf(FOUT,"      double precision xVariable\n");
          if((grouptypes[igt]).nparms>0)
            fprintf(FOUT,"      double precision pParameters(%d)\n",(grouptypes[igt]).nparms);
          fprintf(FOUT,"      double precision FuncValue\n");
          fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).var);
          for(i=0;i<(grouptypes[igt]).nparms;i++)
            fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).parms[i]);
          fprintf(FOUT,"\n");
          for(i=0;i<nGroupDeclLines;i++)
            fprintf(FOUT,"%s\n",groupDeclLines[i]);
          fprintf(FOUT,"\n");
/* Globals */
          fprintf(FOUT,"C     GLOBALS \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nGroupCodeLines;i++)
            fprintf(FOUT,"%s\n",groupCodeLines[i]);
          fprintf(FOUT,"\n");
/* Local Code */
          fprintf(FOUT,"C     Local Code \n");
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      %s=xVariable\n",(grouptypes[igt]).var);
          for(i=0;i<(grouptypes[igt]).nparms;i++)
            fprintf(FOUT,"      %s=pParameters(%d)\n",(grouptypes[igt]).parms[i],i+1);
          for(i=0;i<nLocalGroupCodeLines;i++)
            if((localGroupCodeLines[i])[0]!='F'&&(localGroupCodeLines[i])[0]!='G')
              fprintf(FOUT," %s\n",localGroupCodeLines[i]+1);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      RETURN\n");
          fprintf(FOUT,"      END\n");
         }
        nLocalGroupCodeLines=0;
        groupStarted=1;
        strcpy(groupType,name1);
        igt=findGroupTypeNumber(groupType);
        break;
       case 'A':
        if(code[1]==' ')
         {
          strcpy(gline,"      ");
          strcat(gline,name1);
          strcat(gline,"=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"     @");
          strcat(gline,line+24);
         }
        AddLocalGroupCodeLine(gline);
        break;
       case 'I':
        if(code[1]==' ')
         {
          sprintf(gline,"      if(%s)",name1);
          AddLocalGroupCodeLine(gline);
          strcpy(gline,"     @");
          strcat(gline,name2);
          strcat(gline,"=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"     @");
          strcat(gline,line+24);
         }
        AddLocalGroupCodeLine(gline);
        break;
       case 'E':
        if(code[1]!=' ')
         {
          sprintf(gline,"      if(%s)",name1);
          AddLocalGroupCodeLine(gline);
          strcpy(gline,"     @");
          strcat(gline,name2);
          strcat(gline,"=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"     @");
          strcat(gline,line+24);
         }
        break;
       case 'F':
        if(code[1]==' ')
         {
          strcpy(gline,"F     FuncValue=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"F    @");
          strcat(gline,line+24);
         }
        AddLocalGroupCodeLine(gline);
        break;
       case 'G':
        if(code[1]==' ')
         {
          strcpy(gline,"G     FuncValue=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"G    @");
          strcat(gline,line+24);
         }
        AddLocalGroupCodeLine(gline);
        break;
       case 'H':
        if(code[1]==' ')
         {
          strcpy(gline,"H     FuncValue=");
          strcat(gline,line+24);
         }else{
          strcpy(gline,"H    @");
          strcat(gline,line+24);
         }
        AddLocalGroupCodeLine(gline);
        break;
       default:
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        break;
      }
     break;
   }
  return;
 }

static int elementStarted=0;
static char elemType[80];
static int iet;
void handleELEMENTS(char *code,char *name1,char *name2,int val1Set,double val1,char *name3,int val2Set,double val2)
 {
  int verbose=0;
  char eline[80];
  int i,iv,ev;
  int iv1,iv2;
  char fline[120];
  int idecl,doit;

  if(verbose){printf("handleELEMENTS substate=%c\n",substate);fflush(stdout);}

  switch(substate)
   {
    case 'T':
     switch(code[0])
      {
       case 'I':
        strcpy(eline,"      integer ");
        strcat(eline,name1);
        AddElementDeclLine(eline);
        break;
       case 'R':
        strcpy(eline,"      double precision ");
        strcat(eline,name1);
        AddElementDeclLine(eline);
        break;
       case 'L':
        strcpy(eline,"      logical ");
        strcat(eline,name1);
        AddElementDeclLine(eline);
        break;
       case 'M':
        strcpy(eline,"      external ");
        strcat(eline,name1);
/*      AddElementDeclLine(eline);*/
        if(!strcmp(name1,"MAX"))needMax=1;
        break;
       case 'F':
        strcpy(eline,"      external ");
        strcat(eline,name1);
/*      AddElementDeclLine(eline);*/
        if(!strcmp(name1,"MAX"))needMax=1;
        break;
       default:
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        break;
      }
     break;
    case 'G':
     switch(code[0])
      {
       case 'A':
        if(code[1]==' ')
         {
          strcpy(eline,"      ");
          strcat(eline,name1);
          strcat(eline,"=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"     @");
          strcat(eline,line+24);
         }
        AddElementCodeLine(eline);
        break;
       case 'I':
        if(code[1]==' ')
         {
          sprintf(eline,"      if(%s)",name1);
          AddLocalElementCodeLine(eline);
          strcpy(eline,"     @");
          strcat(eline,name2);
          strcat(eline,"=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"     @");
          strcat(eline,line+24);
         }
        AddElementCodeLine(eline);
        break;
       case 'E':
        if(code[1]==' ')
         {
          sprintf(eline,"      if(.not.%s)",name1);
          AddElementCodeLine(eline);
          strcpy(eline,"     @");
          strcat(eline,name2);
          strcat(eline,"=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"     @");
          strcat(eline,line+24);
         }
        AddElementCodeLine(eline);
        break;
       default:
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        break;
      }
     break;
    case 'I':
     switch(code[0])
      {
       case 'T':
        if(elementStarted)
         {
          char tmp[256];
          strcpy(tmp,elemType);rectString(tmp);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      subroutine Ff%s%s(xVariables,pParameters,FuncValue)\n",rectproblemName,tmp);
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
            fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nintvars);
           else
            fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nelemvars);
          if((elementtypes[iet]).nparms>0)
            fprintf(FOUT,"      double precision pParameters(%d)\n",(elementtypes[iet]).nparms);
          fprintf(FOUT,"      double precision FuncValue\n");

          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
             {
              sprintf(fline,"      double precision %s",((elementtypes[iet]).intvars)[iv]);
              doit=1;
              for(idecl=0;idecl<nElementDeclLines;idecl++)
                if(!strcmp(elementDeclLines[idecl],fline))doit=0;
              if(doit)fprintf(FOUT,"C0\n%s\n",fline);
             }
           }else{
            for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
             {
              sprintf(fline,"      double precision %s",((elementtypes[iet]).elemvars)[iv]);
              doit=1;
              for(idecl=0;idecl<nElementDeclLines;idecl++)
               {
                if(!strcmp(elementDeclLines[idecl],fline))doit=0;
               }
              if(doit)fprintf(FOUT,"C1\n%s\n",fline);
             }
           }
          for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
           {
            sprintf(fline,"      double precision %s",((elementtypes[iet]).parms)[iv]);
            doit=1;
            for(idecl=0;idecl<nElementDeclLines;idecl++)
              if(!strcmp(elementDeclLines[idecl],fline))doit=0;
            if(doit)fprintf(FOUT,"C2\n%s\n",fline);
           }
          fprintf(FOUT,"\n");
          for(i=0;i<nElementDeclLines;i++)
            fprintf(FOUT,"%s\n",elementDeclLines[i]);
          fprintf(FOUT,"\n");

          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
             {
              fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).intvars)[iv],iv+1);
             }
           }else{
            for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
             {
              fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).elemvars)[iv],iv+1);
             }
           }
          for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
           {
            fprintf(FOUT,"      %s=pParameters(%d)\n",((elementtypes[iet]).parms)[iv],iv+1);
           }
          fprintf(FOUT,"\n");
/* Globals */
          fprintf(FOUT,"C     GLOBALS \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nElementCodeLines;i++)
            fprintf(FOUT,"%s\n",elementCodeLines[i]);
          fprintf(FOUT,"\n");
/* Local Code */
          fprintf(FOUT,"C     Local Code \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nLocalElementCodeLines;i++)
            if((localElementCodeLines[i])[0]!='G'&&(localElementCodeLines[i])[0]!='H')
              fprintf(FOUT," %s\n",localElementCodeLines[i]+1);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      RETURN\n");
          fprintf(FOUT,"      END\n");

          fprintf(FOUT,"\n");
          fprintf(FOUT,"      subroutine Fg%s%s(iVariable,xVariables,\n",rectproblemName,tmp);
          fprintf(FOUT,"     *                  pParameters,Gradient)\n");
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
            fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nintvars);
           else
            fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nelemvars);
          fprintf(FOUT,"      double precision Gradient\n");
          if((elementtypes[iet]).nparms>0)
            fprintf(FOUT,"      double precision pParameters(%d)\n",(elementtypes[iet]).nparms);
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
             {
              sprintf(fline,"      double precision %s",((elementtypes[iet]).intvars)[iv]);
              doit=1;
              for(idecl=0;idecl<nElementDeclLines;idecl++)
                if(!strcmp(elementDeclLines[idecl],fline))doit=0;
              if(doit)fprintf(FOUT,"C3\n%s\n",fline);
             }
           }else{
            for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
             {
              sprintf(fline,"      double precision %s",((elementtypes[iet]).elemvars)[iv]);
              doit=1;
              for(idecl=0;idecl<nElementDeclLines;idecl++)
                if(!strcmp(elementDeclLines[idecl],fline))doit=0;
              if(doit)fprintf(FOUT,"C4\n%s\n",fline);
             }
           }
          for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
           {
            sprintf(fline,"      double precision %s",((elementtypes[iet]).parms)[iv]);
            doit=1;
            for(idecl=0;idecl<nElementDeclLines;idecl++)
              if(!strcmp(elementDeclLines[idecl],fline))doit=0;
            if(doit)fprintf(FOUT,"C5\n%s\n",fline);
           }
          fprintf(FOUT,"\n");
          for(i=0;i<nElementDeclLines;i++)
            fprintf(FOUT,"%s\n",elementDeclLines[i]);
          fprintf(FOUT,"\n");

          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
             {
              fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).intvars)[iv],iv+1);
             }
           }else{
            for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
             {
              fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).elemvars)[iv],iv+1);
             }
           }
          for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
           {
            fprintf(FOUT,"      %s=pParameters(%d)\n",((elementtypes[iet]).parms)[iv],iv+1);
           }
          fprintf(FOUT,"\n");
/* Globals */
          fprintf(FOUT,"C     GLOBALS \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nElementCodeLines;i++)
            fprintf(FOUT,"%s\n",elementCodeLines[i]);
          fprintf(FOUT,"\n");
/* Local Code */
          fprintf(FOUT,"C     Local Code \n");
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      Gradient=0.d0\n");
          fprintf(FOUT,"\n");
          for(i=0;i<nLocalElementCodeLines;i++)
            if((localElementCodeLines[i])[0]!='F'&&(localElementCodeLines[i])[0]!='H')
              fprintf(FOUT," %s\n",localElementCodeLines[i]+1);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      RETURN\n");
          fprintf(FOUT,"      END\n");

          fprintf(FOUT,"\n");
          fprintf(FOUT,"      subroutine Fh%s%s(iVariable,jVariable,\n",rectproblemName,tmp);
          fprintf(FOUT,"     *                  xVariables,pParameters,Hessian)\n");
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
            fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nintvars);
           else
            fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nelemvars);
          fprintf(FOUT,"      double precision Hessian\n");
          if((elementtypes[iet]).nparms>0)
            fprintf(FOUT,"      double precision pParameters(%d)\n",(elementtypes[iet]).nparms);
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
             {
              sprintf(fline,"      double precision %s",((elementtypes[iet]).intvars)[iv]);
              doit=1;
              for(idecl=0;idecl<nElementDeclLines;idecl++)
                if(!strcmp(elementDeclLines[idecl],fline))doit=0;
              if(doit)fprintf(FOUT,"C6\n%s\n",fline);
             }
           }else{
            for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
             {
              sprintf(fline,"      double precision %s",((elementtypes[iet]).elemvars)[iv]);
              doit=1;
              for(idecl=0;idecl<nElementDeclLines;idecl++)
                if(!strcmp(elementDeclLines[idecl],fline))doit=0;
              if(doit)fprintf(FOUT,"C7\n%s\n",fline);
             }
           }
          for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
           {
            sprintf(fline,"      double precision %s",((elementtypes[iet]).parms)[iv]);
            doit=1;
            for(idecl=0;idecl<nElementDeclLines;idecl++)
              if(!strcmp(elementDeclLines[idecl],fline))doit=0;
            if(doit)fprintf(FOUT,"C8\n%s\n",fline);
           }
          fprintf(FOUT,"\n");
          for(i=0;i<nElementDeclLines;i++)
            fprintf(FOUT,"%s\n",elementDeclLines[i]);
          fprintf(FOUT,"\n");
      
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
             {
              fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).intvars)[iv],iv+1);
             }
           }else{
            for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
             {
              fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).elemvars)[iv],iv+1);
             }
           }
          for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
           {
            fprintf(FOUT,"      %s=pParameters(%d)\n",((elementtypes[iet]).parms)[iv],iv+1);
           }
          fprintf(FOUT,"\n");
/* Globals */
          fprintf(FOUT,"C     GLOBALS \n");
          fprintf(FOUT,"\n");
          for(i=0;i<nElementCodeLines;i++)
            fprintf(FOUT,"%s\n",elementCodeLines[i]);
          fprintf(FOUT,"\n");
/* Local Code */
          fprintf(FOUT,"C     Local Code \n");
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      Hessian=0.d0\n");
          fprintf(FOUT,"\n");
          for(i=0;i<nLocalElementCodeLines;i++)
            if((localElementCodeLines[i])[0]!='F'&&(localElementCodeLines[i])[0]!='G')
              fprintf(FOUT," %s\n",localElementCodeLines[i]+1);
          fprintf(FOUT,"\n");
          fprintf(FOUT,"      RETURN\n");
          fprintf(FOUT,"      END\n");
         }
        nLocalElementCodeLines=0;
        elementStarted=1;
        strcpy(elemType,name1);
        iet=findElementTypeNumber(elemType);
        break;
       case 'R':
        if(verbose){printf(" Case %s, %s, %s %le %s %le\n",code,name1,name2,val1,name3,val2);fflush(stdout);}
        iv=findInternalVariableNumber(elementtypes+iet,name1);
        ev=findElementVariableNumber(elementtypes+iet,name2);
        NLMSetElement((elementtypes[iet]).R,iv,ev,val1);
        if(name3[0]!=0x0)
         {
          ev=findElementVariableNumber(elementtypes+iet,name3);
          NLMSetElement((elementtypes[iet]).R,iv,ev,val2);
         }
        break;
       case 'A':
        if(code[1]==' ')
         {
          strcpy(eline,"      ");
          strcat(eline,name1);
          strcat(eline,"=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"     @");
          strcat(eline,line+24);
         }
        AddLocalElementCodeLine(eline);
        break;
       case 'I':
        if(code[1]==' ')
         {
          sprintf(eline,"      if(%s)",name1);
          AddLocalElementCodeLine(eline);
          strcpy(eline,"     @");
          strcat(eline,name2);
          strcat(eline,"=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"     @");
          strcat(eline,line+24);
         }
        AddLocalElementCodeLine(eline);
        break;
       case 'E':
        if(code[1]==' ')
         {
          sprintf(eline,"      if(.not.%s)",name1);
          AddLocalElementCodeLine(eline);
          strcpy(eline,"     @");
          strcat(eline,name2);
          strcat(eline,"=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"     @");
          strcat(eline,line+24);
         }
        AddLocalElementCodeLine(eline);
        break;
       case 'F':
        if(code[1]==' ')
         {
          strcpy(eline,"F     FuncValue=");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"F    @");
          strcat(eline,line+24);
         }
        AddLocalElementCodeLine(eline);
        break;
       case 'G':
        if(code[1]==' ')
         {
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
            iv=findInternalVariableNumber(elementtypes+iet,name1);
           else
            iv=findElementVariableNumber(elementtypes+iet,name1);
          sprintf(eline,"G     if(iVariable.eq.%d)Gradient=",iv);
          AddLocalElementCodeLine(eline);
          strcpy(eline,"G    @");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"G    @");
          strcat(eline,line+24);
         }
        AddLocalElementCodeLine(eline);
        break;
       case 'H':
        if(code[1]==' ')
         {
          if((elementtypes[iet]).R!=(NLMatrix)NULL)
           {
            iv1=findInternalVariableNumber(elementtypes+iet,name1);
            iv2=findInternalVariableNumber(elementtypes+iet,name2);
           }else{
            iv1=findElementVariableNumber(elementtypes+iet,name1);
            iv2=findElementVariableNumber(elementtypes+iet,name2);
           }
          if(iv1==iv2)
            sprintf(eline,"H     if(iVariable.eq.%d.and.jVariable.eq.%d)Hessian=",iv1,iv2);
           else{
            sprintf(eline,"H     if(iVariable.eq.%d.and.jVariable.eq.%d.or.",iv1,iv2);
            AddLocalElementCodeLine(eline);
            sprintf(eline,"H    @   iVariable.eq.%d.and.jVariable.eq.%d)Hessian=",iv2,iv1);
           }
          AddLocalElementCodeLine(eline);
          strcpy(eline,"H    @");
          strcat(eline,line+24);
         }else{
          strcpy(eline,"H    @");
          strcat(eline,line+24);
         }
        AddLocalElementCodeLine(eline);
        break;
       default:
        handlePARMS(code,name1,name2,val1Set,val1,name3,val2Set,val2);
        break;
      }
     break;
   }

  return;
 }

void endNAME()
 {
  int verbose=0;

  if(verbose){printf("endNAME\n");fflush(stdout);}

  return;
 }

void endVARIABLES()
 {
  int i,j,ig;
  int verbose=0;
  NLVector A;

  if(verbose){printf("endVARIABLES\n");fflush(stdout);}

  rectifyProblemName();
  if(P==(NLProblem)NULL)
   {
    if(verbose){printf("    Creating problem\n");fflush(stdout);}
    P=NLCreateProblem(problemName,nv);
   }else{
    if(verbose){printf("  Adding variables problem\n");fflush(stdout);}
    NLPAddVariables(P,nv-1);
   }
  for(j=0;j<nv;j++)
   {
    if((varname[j])[0]!=0x0)NLPSetVariableName(P,j,varname[j]);
    if(varscl[j]!=1.)NLPSetVariableScale(P,j,varscl[j]);
    NLPSetLowerSimpleBound(P,j,0.);
    NLPUnSetUpperSimpleBound(P,j);
   }
  if(groupsDONE)
   {
    if(verbose){printf("  groups have already been done\n");fflush(stdout);}
    for(j=0;j<nv;j++)
     {
      for(i=0;i<varnLE[j];i++)
       {
        ig=(varLEi[j])[i];
        if(!NLPIsGroupASet(P,ig))
         {
          A=NLCreateVector(nv);
          NLVSetC(A,j,(varLEv[j])[i]);
          NLPSetGroupA(P,ig,A);
          NLFreeVector(A);
         }else{
          A=NLPGetGroupA(P,ig);
          NLVSetC(A,j,(varLEv[j])[i]);
         }
       }
     }
   }
  variablesDONE=1;

  return;
 }

void endGROUPS()
 {
  int verbose=0;
  if(verbose){printf("endGROUPS\n");fflush(stdout);}
  groupsDONE=1;
  return;
 }

void endGROUPFUNCTIONS()
 {
  int verbose=0;
  int i;

  if(verbose){printf("endGROUPFUNTIONS\n");fflush(stdout);}
  GROUPFUNCTIONSDone=1;

  if(groupStarted)
   {
    char tmp[256];
    strcpy(tmp,groupType);rectString(tmp);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      subroutine Gf%s%s(xVariable,pParameters,FuncValue)\n",rectproblemName,tmp);
    fprintf(FOUT,"      double precision xVariable\n");
    if((grouptypes[igt]).nparms>0)
      fprintf(FOUT,"      double precision pParameters(%d)\n",(grouptypes[igt]).nparms);
    fprintf(FOUT,"      double precision FuncValue\n");
    fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).var);
    for(i=0;i<(grouptypes[igt]).nparms;i++)
      fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).parms[i]);
    fprintf(FOUT,"\n");
    for(i=0;i<nGroupDeclLines;i++)
      fprintf(FOUT,"%s\n",groupDeclLines[i]);
    fprintf(FOUT,"\n");
/* Globals */
    fprintf(FOUT,"C     GLOBALS \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nGroupCodeLines;i++)
      fprintf(FOUT,"%s\n",groupCodeLines[i]);
    fprintf(FOUT,"\n");
/* Local Code */
    fprintf(FOUT,"C     Local Code \n");
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      %s=xVariable\n",(grouptypes[igt]).var);
    for(i=0;i<(grouptypes[igt]).nparms;i++)
      fprintf(FOUT,"      %s=pParameters(%d)\n",(grouptypes[igt]).parms[i],i+1);
    for(i=0;i<nLocalGroupCodeLines;i++)
      if((localGroupCodeLines[i])[0]!='G'&&(localGroupCodeLines[i])[0]!='H')
        fprintf(FOUT," %s\n",localGroupCodeLines[i]+1);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      RETURN\n");
    fprintf(FOUT,"      END\n");

    fprintf(FOUT,"\n");
    fprintf(FOUT,"      subroutine Gg%s%s(xVariable,pParameters,FuncValue)\n",rectproblemName,tmp);
    fprintf(FOUT,"      double precision xVariable\n");
    if((grouptypes[igt]).nparms>0)
      fprintf(FOUT,"      double precision pParameters(%d)\n",(grouptypes[igt]).nparms);
    fprintf(FOUT,"      double precision FuncValue\n");
    fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).var);
    for(i=0;i<(grouptypes[igt]).nparms;i++)
      fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).parms[i]);
    fprintf(FOUT,"\n");
    for(i=0;i<nGroupDeclLines;i++)
      fprintf(FOUT,"%s\n",groupDeclLines[i]);
    fprintf(FOUT,"\n");
/* Globals */
    fprintf(FOUT,"C     GLOBALS \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nGroupCodeLines;i++)
      fprintf(FOUT,"%s\n",groupCodeLines[i]);
    fprintf(FOUT,"\n");
/* Local Code */
    fprintf(FOUT,"C     Local Code \n");
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      %s=xVariable\n",(grouptypes[igt]).var);
    for(i=0;i<(grouptypes[igt]).nparms;i++)
      fprintf(FOUT,"      %s=pParameters(%d)\n",(grouptypes[igt]).parms[i],i+1);
    for(i=0;i<nLocalGroupCodeLines;i++)
      if((localGroupCodeLines[i])[0]!='F'&&(localGroupCodeLines[i])[0]!='H')
        fprintf(FOUT," %s\n",localGroupCodeLines[i]+1);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      RETURN\n");
    fprintf(FOUT,"      END\n");
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      subroutine Gh%s%s(xVariable,pParameters,FuncValue)\n",rectproblemName,tmp);
    fprintf(FOUT,"      double precision xVariable\n");
    if((grouptypes[igt]).nparms>0)
      fprintf(FOUT,"      double precision pParameters(%d)\n",(grouptypes[igt]).nparms);
    fprintf(FOUT,"      double precision FuncValue\n");
    fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).var);
    for(i=0;i<(grouptypes[igt]).nparms;i++)
      fprintf(FOUT,"      double precision %s\n",(grouptypes[igt]).parms[i]);
    fprintf(FOUT,"\n");
    for(i=0;i<nGroupDeclLines;i++)
      fprintf(FOUT,"%s\n",groupDeclLines[i]);
    fprintf(FOUT,"\n");
/* Globals */
    fprintf(FOUT,"C     GLOBALS \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nGroupCodeLines;i++)
      fprintf(FOUT,"%s\n",groupCodeLines[i]);
    fprintf(FOUT,"\n");
/* Local Code */
    fprintf(FOUT,"C     Local Code \n");
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      %s=xVariable\n",(grouptypes[igt]).var);
    for(i=0;i<(grouptypes[igt]).nparms;i++)
      fprintf(FOUT,"      %s=pParameters(%d)\n",(grouptypes[igt]).parms[i],i+1);
    for(i=0;i<nLocalGroupCodeLines;i++)
      if((localGroupCodeLines[i])[0]!='F'&&(localGroupCodeLines[i])[0]!='G')
        fprintf(FOUT," %s\n",localGroupCodeLines[i]+1);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      RETURN\n");
    fprintf(FOUT,"      END\n");
   }

  return;
 }

void endCONSTANTS()
 {
  int verbose=0;
  if(verbose){printf("endCONSTANTS\n");fflush(stdout);}
  return;
 }

void endRANGES()
 {
  int verbose=0;
  if(verbose){printf("endRANGES\n");fflush(stdout);}
  return;
 }

void endBOUNDS()
 {
  int verbose=0;
  if(verbose){printf("endBOUNDS\n");fflush(stdout);}
  return;
 }

void endSTART_POINT()
 {
  int verbose=0;
  if(verbose){printf("endSTART_POINT\n");fflush(stdout);}
  return;
 }

void endELEMENT_TYPE()
 {
  int verbose=0;
  int i,nv;

  if(verbose){printf("endELEMENT_TYPE\n");fflush(stdout);}

  for(i=0;i<net;i++)
   {
    if(verbose){printf("endELEMENT_TYPE elementtypes[%d]=(0x%8.8x)",i,elementtypes[i]);
                printf(" is named -->%s<--\n",(elementtypes[i]).name);fflush(stdout);}
    nv=(elementtypes[i]).nintvars;
    if(nv==0)
     {
      nv=(elementtypes[i]).nelemvars;
      (elementtypes[i]).R=(NLMatrix)NULL;
     }else{
      (elementtypes[i]).R=NLCreateSparseMatrix((elementtypes[i]).nintvars,(elementtypes[i]).nelemvars);
     }
    (elementtypes[i]).ef=
       NLCreateElementFunction(P,(elementtypes[i]).name,nv,(elementtypes[i]).R,
            genericEF,dgenericEF,ddgenericEF,(void*)((elementtypes[i]).name),(void (*)(void*))NULL);
   }

  return;
 }

void endELEMENT_USES()
 {
  int verbose=0;
  int i;
  if(verbose){printf("In endELEMENT_USES\n");fflush(stdout);}

  for(i=0;i<ne;i++)
   {
    if(verbose){printf("  element %d (%s) is type \"%s\" (0x%8.8x)\n",i,(elements[i]).name,((elements[i]).type)->name,(elements[i]).type);
                printf("  call NLCreateNonlinearElement(P,%s)\n",(elements[i]).name);fflush(stdout);}
    (elements[i]).ne=NLCreateNonlinearElement(P,(elements[i]).name,((elements[i]).type)->ef,(elements[i]).vars);
   }
  return;
 }

void endGROUP_TYPE()
 {
  int verbose=0;
  int i;

  if(verbose){printf("endGROUP_TYPE\n");fflush(stdout);}

  for(i=0;i<ngt;i++)
   {
    (grouptypes[i]).gf=NLCreateGroupFunction(P,(grouptypes[i]).name,genericGF,dgenericGF,ddgenericGF,(void*)NULL,(void (*)(void*))NULL);
   }

  return;
 }

void endGROUP_USES()
 {
  int verbose=0;
  int i,igt,j,ie;

  if(verbose){printf("endGROUP_USES, ng=%d\n",ng);fflush(stdout);}

  for(i=0;i<ng;i++)
   {
    if(verbose){printf("  group %d, type -->%s<--\n",i,(groups[i]).type);fflush(stdout);}
    if(strcmp((groups[i]).type,""))
     {
      igt=findGroupTypeNumber((groups[i]).type);
      if(verbose){printf("group %d (%s) is type %d(%s)\n",i,(groups[i]).name,igt,(groups[i]).type);fflush(stdout);}
      NLPSetGroupFunction(P,i,(grouptypes[igt]).gf);
      NLPSetGroupScale(P,i,(groups[i]).scale);
     }else
      NLPSetGroupScale(P,i,(groups[i]).scale);
    if(verbose){printf("  Do elements, %d\n",(groups[i]).nelements);fflush(stdout);}
    for(j=0;j<(groups[i]).nelements;j++)
     {
      if(verbose){printf("element (%s)\n",((groups[i]).elements)[j]);fflush(stdout);}
      ie=findElementNumber(((groups[i]).elements)[j]);
      NLPAddNonlinearElementToGroup(P,i,((groups[i]).weights)[j],(elements[ie]).ne);
     }
   }

  return;
 }

void endOBJECT_BOUND()
 {
  int verbose=0;
  if(verbose){printf("endOBJECT_BOUND\n");fflush(stdout);}
  return;
 }

void endELEMENTS()
 {
  int verbose=0;
  int i,iv;
  int idecl,doit;
  char fline[120];

  if(verbose){printf("endELEMENTS\n");fflush(stdout);}

  if(elementStarted)
   {
    char tmp[256];
    strcpy(tmp,elemType);rectString(tmp);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      subroutine Ff%s%s(xVariables,pParameters,FuncValue)\n",rectproblemName,tmp);
    if((elementtypes[iet]).R!=(NLMatrix)NULL)
      fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nintvars);
     else
      fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nelemvars);
    if((elementtypes[iet]).nparms>0)
      fprintf(FOUT,"      double precision pParameters(%d)\n",(elementtypes[iet]).nparms);
    fprintf(FOUT,"      double precision FuncValue\n");
    if((elementtypes[iet]).R!=(NLMatrix)NULL)
     {
      for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
       {
        sprintf(fline,"      double precision %s",((elementtypes[iet]).intvars)[iv]);
        doit=1;
        for(idecl=0;idecl<nElementDeclLines;idecl++)
          if(!strcmp(elementDeclLines[idecl],fline))doit=0;
        if(doit)fprintf(FOUT,"C9\n%s\n",fline);
       }
     }else{
      for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
       {
        sprintf(fline,"      double precision %s",((elementtypes[iet]).elemvars)[iv]);
        doit=1;
        for(idecl=0;idecl<nElementDeclLines;idecl++)
          if(!strcmp(elementDeclLines[idecl],fline))doit=0;
        if(doit)fprintf(FOUT,"C10\n%s\n",fline);
       }
     }
    for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
     {
      sprintf(fline,"      double precision %s",((elementtypes[iet]).parms)[iv]);
      doit=1;
      for(idecl=0;idecl<nElementDeclLines;idecl++)
        if(!strcmp(elementDeclLines[idecl],fline))doit=0;
      if(doit)fprintf(FOUT,"C11\n%s\n",fline);
     }
    fprintf(FOUT,"\n");
    for(i=0;i<nElementDeclLines;i++)
      fprintf(FOUT,"%s\n",elementDeclLines[i]);
    fprintf(FOUT,"\n");

    if((elementtypes[iet]).R!=(NLMatrix)NULL)
     {
      for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
       {
        fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).intvars)[iv],iv+1);
       }
     }else{
      for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
       {
        fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).elemvars)[iv],iv+1);
       }
     }
    for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
     {
      fprintf(FOUT,"      %s=pParameters(%d)\n",((elementtypes[iet]).parms)[iv],iv+1);
     }
    fprintf(FOUT,"\n");
/* Globals */
    fprintf(FOUT,"C     GLOBALS \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nElementCodeLines;i++)
      fprintf(FOUT,"%s\n",elementCodeLines[i]);
    fprintf(FOUT,"\n");
/* Local Code */
    fprintf(FOUT,"C     Local Code \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nLocalElementCodeLines;i++)
      if((localElementCodeLines[i])[0]!='G'&&(localElementCodeLines[i])[0]!='H')
        fprintf(FOUT," %s\n",localElementCodeLines[i]+1);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      RETURN\n");
    fprintf(FOUT,"      END\n");

    fprintf(FOUT,"\n");
    fprintf(FOUT,"      subroutine Fg%s%s(iVariable,xVariables,\n",rectproblemName,tmp);
    fprintf(FOUT,"     *                  pParameters,Gradient)\n");
    if((elementtypes[iet]).R!=(NLMatrix)NULL)
      fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nintvars);
     else
      fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nelemvars);
    fprintf(FOUT,"      double precision Gradient\n");
    if((elementtypes[iet]).nparms>0)
      fprintf(FOUT,"      double precision pParameters(%d)\n",(elementtypes[iet]).nparms);
    if((elementtypes[iet]).R!=(NLMatrix)NULL)
     {
      for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
       {
        sprintf(fline,"      double precision %s",((elementtypes[iet]).intvars)[iv]);
        doit=1;
        for(idecl=0;idecl<nElementDeclLines;idecl++)
          if(!strcmp(elementDeclLines[idecl],fline))doit=0;
        if(doit)fprintf(FOUT,"C12\n%s\n",fline);
       }
     }else{
      for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
       {
        sprintf(fline,"      double precision %s",((elementtypes[iet]).elemvars)[iv]);
        doit=1;
        for(idecl=0;idecl<nElementDeclLines;idecl++)
          if(!strcmp(elementDeclLines[idecl],fline))doit=0;
        if(doit)fprintf(FOUT,"C13\n%s\n",fline);
       }
     }
    for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
     {
      sprintf(fline,"      double precision %s",((elementtypes[iet]).parms)[iv]);
      doit=1;
      for(idecl=0;idecl<nElementDeclLines;idecl++)
        if(!strcmp(elementDeclLines[idecl],fline))doit=0;
      if(doit)fprintf(FOUT,"C14\n%s\n",fline);
     }
    fprintf(FOUT,"\n");
    for(i=0;i<nElementDeclLines;i++)
      fprintf(FOUT,"%s\n",elementDeclLines[i]);
    fprintf(FOUT,"\n");

    if((elementtypes[iet]).R!=(NLMatrix)NULL)
     {
      for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
       {
        fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).intvars)[iv],iv+1);
       }
     }else{
      for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
       {
        fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).elemvars)[iv],iv+1);
       }
     }
    for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
     {
      fprintf(FOUT,"      %s=pParameters(%d)\n",((elementtypes[iet]).parms)[iv],iv+1);
     }
    fprintf(FOUT,"\n");
/* Globals */
    fprintf(FOUT,"C     GLOBALS \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nElementCodeLines;i++)
      fprintf(FOUT,"%s\n",elementCodeLines[i]);
    fprintf(FOUT,"\n");
/* Local Code */
    fprintf(FOUT,"C     Local Code \n");
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      Gradient=0.d0\n");
    fprintf(FOUT,"\n");
    for(i=0;i<nLocalElementCodeLines;i++)
      if((localElementCodeLines[i])[0]!='F'&&(localElementCodeLines[i])[0]!='H')
        fprintf(FOUT," %s\n",localElementCodeLines[i]+1);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      RETURN\n");
    fprintf(FOUT,"      END\n");

    fprintf(FOUT,"\n");
    fprintf(FOUT,"      subroutine Fh%s%s(iVariable,jVariable,xVariables,\n",rectproblemName,tmp);
    fprintf(FOUT,"     *                  pParameters,Hessian)\n");
    if((elementtypes[iet]).R!=(NLMatrix)NULL)
      fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nintvars);
     else
      fprintf(FOUT,"      double precision xVariables(%d)\n",(elementtypes[iet]).nelemvars);
    fprintf(FOUT,"      double precision Hessian\n");
    if((elementtypes[iet]).nparms>0)
      fprintf(FOUT,"      double precision pParameters(%d)\n",(elementtypes[iet]).nparms);
    if((elementtypes[iet]).R!=(NLMatrix)NULL)
     {
      for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
       {
        sprintf(fline,"      double precision %s",((elementtypes[iet]).intvars)[iv]);
        doit=1;
        for(idecl=0;idecl<nElementDeclLines;idecl++)
          if(!strcmp(elementDeclLines[idecl],fline))doit=0;
        if(doit)fprintf(FOUT,"C15\n%s\n",fline);
       }
     }else{
      for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
       {
        sprintf(fline,"      double precision %s",((elementtypes[iet]).elemvars)[iv]);
        doit=1;
        for(idecl=0;idecl<nElementDeclLines;idecl++)
          if(!strcmp(elementDeclLines[idecl],fline))doit=0;
        if(doit)fprintf(FOUT,"C16\n%s\n",fline);
       }
     }
    for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
     {
      sprintf(fline,"      double precision %s",((elementtypes[iet]).parms)[iv]);
      doit=1;
      for(idecl=0;idecl<nElementDeclLines;idecl++)
         if(!strcmp(elementDeclLines[idecl],fline))doit=0;
      if(doit)fprintf(FOUT,"C17\n%s\n",fline);
     }
    fprintf(FOUT,"\n");
    for(i=0;i<nElementDeclLines;i++)
      fprintf(FOUT,"%s\n",elementDeclLines[i]);
    fprintf(FOUT,"\n");

    if((elementtypes[iet]).R!=(NLMatrix)NULL)
     {
      for(iv=0;iv<(elementtypes[iet]).nintvars;iv++)
       {
        fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).intvars)[iv],iv+1);
       }
     }else{
      for(iv=0;iv<(elementtypes[iet]).nelemvars;iv++)
       {
        fprintf(FOUT,"      %s=xVariables(%d)\n",((elementtypes[iet]).elemvars)[iv],iv+1);
       }
     }
    for(iv=0;iv<(elementtypes[iet]).nparms;iv++)
     {
      fprintf(FOUT,"      %s=pParameters(%d)\n",((elementtypes[iet]).parms)[iv],iv+1);
     }
    fprintf(FOUT,"\n");
/* Globals */
    fprintf(FOUT,"C     GLOBALS \n");
    fprintf(FOUT,"\n");
    for(i=0;i<nElementCodeLines;i++)
      fprintf(FOUT,"%s\n",elementCodeLines[i]);
    fprintf(FOUT,"\n");
/* Local Code */
    fprintf(FOUT,"C     Local Code \n");
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      Hessian=0.d0\n");
    fprintf(FOUT,"\n");
    for(i=0;i<nLocalElementCodeLines;i++)
      if((localElementCodeLines[i])[0]!='F'&&(localElementCodeLines[i])[0]!='G')
        fprintf(FOUT," %s\n",localElementCodeLines[i]+1);
    fprintf(FOUT,"\n");
    fprintf(FOUT,"      RETURN\n");
    fprintf(FOUT,"      END\n");
   }
  elementStarted=0;

  return;
 }

void beginNAME()
 {
  int verbose=0;
  if(verbose){printf("beginNAME\n");fflush(stdout);}

  np=0;
  sscanf(line,"NAME %s\n",problemName);
  return;
 }

void beginVARIABLES()
 {
  int verbose=0;
  if(verbose){printf("beginVARIABLES\n");fflush(stdout);}
  nv=0;
  return;
 }

void beginGROUPFUNCTIONS()
 {
  int verbose=0;

  if(verbose){printf("beginGROUPFUNCTIONS\n");fflush(stdout);}
  nGroupCodeLines=0;
  nGroupDeclLines=0;
  groupStarted=0;
  return;
 }

void beginGROUPS()
 {
  int verbose=0;
  if(verbose){printf("beginGROUPS\n");fflush(stdout);}
  if(P==(NLProblem)NULL)P=NLCreateProblem(problemName,1);
  return;
 }

void beginCONSTANTS()
 {
  int verbose=0;
  if(verbose){printf("beginCONSTANTS\n");fflush(stdout);}
  return;
 }

void beginRANGES()
 {
  int verbose=0;
  if(verbose){printf("beginRANGES\n");fflush(stdout);}
  return;
 }

void beginBOUNDS()
 {
  int verbose=0;
  if(verbose){printf("beginBOUNDS\n");fflush(stdout);}
  return;
 }

void beginSTART_POINT()
 {
  int nv,nc,iv;
  int verbose=0;
  if(verbose){printf("beginSTART_POINT\n");fflush(stdout);}

  nv=NLPGetNumberOfVariables(P);
  nc=NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P);
  xStart=(double*)malloc(nv*sizeof(double));
  for(iv=0;iv<nv;iv++)xStart[iv]=0.;
  lStart=(double*)malloc(nc*sizeof(double));
  for(iv=0;iv<nc;iv++)lStart[iv]=0.;
  return;
 }

void beginELEMENT_TYPE()
 {
  int verbose=0;
  if(verbose){printf("beginELEMENT_TYPE\n");fflush(stdout);}
  net=0;
  return;
 }

void beginELEMENT_USES()
 {
  int verbose=0;
  strcpy(defaultElementType,"");
  if(verbose){printf("beginELEMENT_USES\n");fflush(stdout);}
  return;
 }

void beginGROUP_USES()
 {
  int verbose=0;
  strcpy(defaultGroupType,"");
  if(verbose){printf("beginGROUP_USES\n");fflush(stdout);}
  return;
 }

void beginGROUP_TYPE()
 {
  int verbose=0;
  if(verbose){printf("beginGROUP_TYPE\n");fflush(stdout);}
  return;
 }

void beginOBJECT_BOUND()
 {
  int verbose=0;
  if(verbose){printf("beginOBJECT_BOUND\n");fflush(stdout);}
  return;
 }

void beginELEMENTS()
 {
  int verbose=0;
  if(verbose){printf("beginELEMENTS\n");fflush(stdout);}

  nElementCodeLines=0;
  nElementDeclLines=0;
  elementStarted=0;
  return;
 }

void addDo(char *var,int i0,int i1)
 {
  int j;
  int verbose=0;

  if(verbose){printf("Do %s from %d to %d\n",var,i0,i1);fflush(stdout);}
  if(nDos>=mDos)
   {
  if(verbose){printf(" need more space...\n");fflush(stdout);}
    mDos+=10;
    doVar=(char**)realloc(doVar,mDos*sizeof(char*));
    for(j=nDos;j<mDos;j++){doVar[j]=(char*)malloc(20*sizeof(char));(doVar[j])[0]=0x0;}
    doLimits=(int*)realloc(doLimits,5*mDos*sizeof(double));
   }
  strcpy(doVar[nDos],var);
  doLimits[0+5*nDos]=i0;
  doLimits[1+5*nDos]=i1;
  doLimits[2+5*nDos]=1;
  doLimits[3+5*nDos]=i0;
  doLimits[4+5*nDos]=-1;
  if(verbose){printf("Do %d from %d to %d\n",nDos,doLimits[0+5*nDos],doLimits[1+5*nDos]);fflush(stdout);}
  nDos++;

  return;
 }


void setDoIncr(char *var,int di)
 {
  int id,j;

  id=-1;
  for(j=0;j<nDos;j++)if(!strcmp(var,doVar[j]))id=j;

  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO variable %s not found\n",section,lineno,inname,var);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  doLimits[2+5*id]=di;

  return;
 }

void removeDo(int id)
 {
  int j;
  int verbose=0;

  if(verbose){printf("Remove Do %d  nDos=%d\n",id,nDos);fflush(stdout);}

  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  for(j=id+1;j<nDos;j++)
   {
    if(verbose){printf("  move Do %d to %d\n",j,j-1);fflush(stdout);}
    doVar[j-1]=doVar[j];
    doLimits[0+5*(j-1)]=doLimits[0+5*j];
    doLimits[1+5*(j-1)]=doLimits[1+5*j];
    doLimits[2+5*(j-1)]=doLimits[2+5*j];
    doLimits[3+5*(j-1)]=doLimits[3+5*j];
    doLimits[4+5*(j-1)]=doLimits[4+5*j];
   }

  nDos--;

  return;
 }

void removeAllDos()
 {
  nDos=0;
  printf("Remove all Do's\n");fflush(stdout);

  return;
 }

void initialDo()
 {
  int i;
  printf("initialDo %d\n",nDos);fflush(stdout);

  for(i=0;i<nDos;i++)
   {
    doLimits[3+5*i]=doLimits[0+5*i];
  printf("initialDo, set \"%s\" to %d\n",doVar[i],doLimits[3+5*i]);fflush(stdout);
    setParameter(doVar[i],doLimits[3+5*i]);
   }


  return;
 }

int nextDo()
 {
  int i,j;
  printf("nextDo %d\n",nDos);fflush(stdout);

  if(nDos<1)return 0;

  i=nDos-1;
  doLimits[3+5*i]=doLimits[3+5*i]+doLimits[2+5*i];
  printf("nextDo, set \"%s\" to %d\n",doVar[i],doLimits[3+5*i]);fflush(stdout);
  setParameter(doVar[i],doLimits[3+5*i]);

  while(doLimits[3+5*i]>doLimits[1+5*i] && i>0)
   {
    doLimits[3+5*i]=doLimits[0+5*i];
    printf("nextDo, set \"%s\" to %d\n",doVar[i],doLimits[3+5*i]);fflush(stdout);
    setParameter(doVar[i],doLimits[3+5*i]);
    printf("nextDo, set \"%s\" to %d\n",doVar[i-1],doLimits[3+5*i-5]);fflush(stdout);
    setParameter(doVar[i-1],doLimits[3+5*(i-1)]);
    doLimits[3+5*(i-1)]=doLimits[3+5*(i-1)]+doLimits[2+5*i];
    i--;
   }

  if(doLimits[3+5*i]>doLimits[1+5*i])
   {
    return 0;
   }else{
    return 1;
   }
 }

int addElementType(char *name)
 {
  int iet,j;
  static int verbose=0;

/* All the info for an element function except for the range transformation and the functions */

  iet=-1;
  for(j=0;j<net;j++)if(!strcmp(name,(elementtypes[j]).name))iet=j;
  if(iet!=-1)return 0;

  if(net>=met)
   {
    met+=10;
    elementtypes=(struct ElementType*)realloc(elementtypes,met*sizeof(struct ElementType));
    for(j=net;j<met;j++)
     {
      (elementtypes[j]).name=(char*)malloc(20*sizeof(char));
      ((elementtypes[j]).name)[0]=0x0;
      (elementtypes[j]).nelemvars=0;
      (elementtypes[j]).melemvars=-1;
      (elementtypes[j]).elemvars=(char**)NULL;
      (elementtypes[j]).nintvars=0;
      (elementtypes[j]).mintvars=-1;
      (elementtypes[j]).intvars=(char**)NULL;
      (elementtypes[j]).nparms=0;
      (elementtypes[j]).mparms=-1;
      (elementtypes[j]).parms=(char**)NULL;
     }
   }
  if(verbose){printf("Element type %d is %s\n",net,name);fflush(stdout);}
  strcpy((elementtypes[net]).name,name);

  net++;

  return 1;
 }

int findElementTypeNumber(char *name)
 {
  int iet,j;

  iet=-1;
  for(j=0;j<net;j++)
   if(!strcmp(name,(elementtypes[j]).name))iet=j;

  if(iet<0||iet>=net)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid element type %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return iet;
 }

int addElementVariableToElementType(char *name,char *vname)
 {
  int iet;
  int j;
  static int verbose=0;

  iet=findElementTypeNumber(name);

  if((elementtypes[iet]).nelemvars>=(elementtypes[iet]).melemvars)
   {
    (elementtypes[iet]).melemvars+=10;
    (elementtypes[iet]).elemvars=(char**)realloc((elementtypes[iet]).elemvars,
                                                 ((elementtypes[iet]).melemvars)*sizeof(char*));
    for(j=(elementtypes[iet]).nelemvars;j<(elementtypes[iet]).melemvars;j++)
         ((elementtypes[iet]).elemvars)[j]=(char*)malloc(20*sizeof(char));
   }

  strcpy(((elementtypes[iet]).elemvars)[(elementtypes[iet]).nelemvars],vname);
  if(verbose){printf("  add Element variable %s to element type %d is %s\n",vname,iet,name);fflush(stdout);}
  ((elementtypes[iet]).nelemvars)++;

  return 1;
 }

int addInternalVariableToElementType(char *name,char *vname)
 {
  int iet;
  int j;
  static int verbose=0;

  iet=findElementTypeNumber(name);

  if((elementtypes[iet]).nintvars>=(elementtypes[iet]).mintvars)
   {
    (elementtypes[iet]).mintvars+=10;
    (elementtypes[iet]).intvars=(char**)realloc((elementtypes[iet]).intvars,
                                                 ((elementtypes[iet]).mintvars)*sizeof(char*));
    for(j=(elementtypes[iet]).nintvars;j<(elementtypes[iet]).mintvars;j++)
         ((elementtypes[iet]).intvars)[j]=(char*)malloc(20*sizeof(char));
   }

  strcpy(((elementtypes[iet]).intvars)[(elementtypes[iet]).nintvars],vname);
  if(verbose){printf("  add Internal variable %s to element type %d is %s\n",vname,iet,name);fflush(stdout);}
  ((elementtypes[iet]).nintvars)++;

  return 1;
 }

static int addParameterToElementType(char *name,char *vname)
 {
  int iet;
  int j;

  iet=findElementTypeNumber(name);

  if((elementtypes[iet]).nparms>=(elementtypes[iet]).mparms)
   {
    (elementtypes[iet]).mparms+=10;
    (elementtypes[iet]).parms=(char**)realloc((elementtypes[iet]).parms,
                                                 ((elementtypes[iet]).mparms)*sizeof(char*));
    for(j=(elementtypes[iet]).nparms;j<(elementtypes[iet]).mparms;j++)
         ((elementtypes[iet]).parms)[j]=(char*)malloc(20*sizeof(char));
   }

  strcpy(((elementtypes[iet]).parms)[(elementtypes[iet]).nparms],vname);
  ((elementtypes[iet]).nparms)++;

  return 1;
 }

int findElementVariableNumber(struct ElementType *this,char *name)
 {
  int iev,nev,j;

  iev=-1;
  nev=this->nelemvars;
  for(j=0;j<nev;j++)
   if(!strcmp(name,(this->elemvars)[j]))iev=j;

  if(iev<0||iev>=nev)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid element variable %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return iev;
 }

int findElementParameterNumber(struct ElementType *this,char *name)
 {
  int iep,nep,j;

  iep=-1;
  nep=this->nparms;
  for(j=0;j<nep;j++)
   {
    if(!strcmp(name,(this->parms)[j]))iep=j;
   }

  if(iep<0||iep>=nep)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid element variable %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return iep;
 }

int addElement(char *name,char *type)
 {
  int ie,j,iet;
  static int verbose=0;

  ie=-1;
  for(j=0;j<ne;j++)
    if(!strcmp(name,(elements[j]).name))ie=j;
  if(ie!=-1)return 0;

  if(ne>=me)
   {
    me+=10;
    elements=(struct Element*)realloc(elements,me*sizeof(struct Element));
    if(elements==(struct Element*)NULL){printf("ERRRRORRR! OUT OF MEMORY allocating elements, LINE %d in FILE %s\n",__LINE__,__FILE__);fflush(stdout);exit(12);}
    for(j=ne;j<me;j++)
     {
      (elements[j]).name=(char*)malloc(20*sizeof(char));
      if((elements[j]).name==(char*)NULL){printf("ERRRRORRR! OUT OF MEMORY allocating (elements[j]).name, LINE %d in FILE %s\n",__LINE__,__FILE__);fflush(stdout);exit(12);}
      ((elements[j]).name)[0]=0x0;
      (elements[j]).type=(struct ElementType*)NULL;
      (elements[j]).vars=(int*)NULL;
      (elements[j]).parms=(double*)NULL;
     }
   }
  strcpy((elements[ne]).name,name);
  iet=findElementTypeNumber(type);
  (elements[ne]).type=&(elementtypes[iet]);
  (elements[ne]).vars=(int*)malloc((((elements[ne]).type)->nelemvars)*sizeof(int));
  for(j=0;j<((elements[ne]).type)->nelemvars;j++)((elements[ne]).vars)[j]=-1;
  if(verbose){printf("Element %d is %s, type %d (%s)\n",ne,name,iet,type);fflush(stdout);}
  ne++;

  return 1;
 }

int findElementNumber(char *name)
 {
  int ie,j;

  ie=-1;
  for(j=0;j<ne;j++)if(!strcmp(name,(elements[j]).name))ie=j;

  if(ie<0||ie>=ne)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid element %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return ie;
 }

int setElementVariable(char *name,int iv, int v)
 {
  int ie;
  static int verbose=0;

  ie=findElementNumber(name);
  ((elements[ie]).vars)[iv]=v;
  if(verbose){printf("  set element variable: Element %d (%s), variable %d is v[%d]\n",ie,name,iv,v);fflush(stdout);}

  return 1;
 }

int setElementParameter(char *name,int ip, double p)
 {
  int ie;
  int verbose=0;

  if(verbose){printf("setElementParameter, element %s parm %d to %lf\n",name,ip,p);fflush(stdout);}
  ie=findElementNumber(name);
  if((elements[ie]).parms==(double*)NULL)
   {
    (elements[ie]).parms=(double*)malloc((((elements[ie]).type)->nparms)*sizeof(double));
   }
  ((elements[ie]).parms)[ip]=p;

  return 1;
 }

int addGroupType(char *name, char *grpvar)
 {
  int igt,j;

/* All the info for an group function except for the range transformation and the functions */

  igt=-1;
  for(j=0;j<ngt;j++)
    if(!strcmp(name,(grouptypes[j]).name))igt=j;
  if(igt!=-1)return 0;

  if(ngt>=mgt)
   {
    mgt+=10;
    grouptypes=(struct GroupType*)realloc(grouptypes,mgt*sizeof(struct GroupType));
    for(j=ngt;j<mgt;j++)
     {
      (grouptypes[j]).name=(char*)malloc(20*sizeof(char));
      ((grouptypes[j]).name)[0]=0x0;
      (grouptypes[j]).var=(char*)malloc(20*sizeof(char));
      ((grouptypes[j]).var)[0]=0x0;
      (grouptypes[j]).nparms=0;
      (grouptypes[j]).mparms=-1;
      (grouptypes[j]).parms=(char**)NULL;
     }
   }
  strcpy((grouptypes[ngt]).name,name);
  strcpy((grouptypes[ngt]).var,grpvar);
  ngt++;

  return 1;
 }

int findGroupTypeNumber(char *name)
 {
  int igt,j;

  igt=-1;
  for(j=0;j<ngt;j++)
   if(!strcmp(name,(grouptypes[j]).name))igt=j;

  if(igt<0||igt>=ngt)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid group type %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return igt;
 }

int findGroupParameterNumber(struct GroupType *this,char *name)
 {
  int iep,nep,j;

  iep=-1;
  nep=this->nparms;
  for(j=0;j<ngt;j++)
   if(!strcmp(name,(this->parms)[j]))iep=j;

  if(iep<0||iep>=nep)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid group variable %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return iep;
 }

static int addParameterToGroupType(char *name,char *vname)
 {
  int igt;
  int j;
  int verbose=0;

  if(verbose){printf("addParameterToGroupType(%s,%s)\n",name,vname);fflush(stdout);}
  igt=findGroupTypeNumber(name);
  if(verbose){printf(" igt=%d\n",igt);fflush(stdout);}

  if((grouptypes[igt]).nparms>=(grouptypes[igt]).mparms)
   {
    (grouptypes[igt]).mparms+=10;
    (grouptypes[igt]).parms=(char**)realloc((grouptypes[igt]).parms,
                                                 ((grouptypes[igt]).mparms)*sizeof(char*));
    for(j=(grouptypes[igt]).nparms;j<(grouptypes[igt]).mparms;j++)
         ((grouptypes[igt]).parms)[j]=(char*)malloc(20*sizeof(char));
   }

  strcpy(((grouptypes[igt]).parms)[(grouptypes[igt]).nparms],vname);
  ((grouptypes[igt]).nparms)++;

  return 1;
 }

void setGroupType(char *name,char *type)
 {
  int ig,iet;

  ig=findGroupNumber(name);
  strcpy((groups[ig]).type,type);
  iet=findGroupTypeNumber(type);
  (groups[ig]).parms=(double*)malloc((elementtypes[iet]).nparms*sizeof(double));
  return;
 }

void setAllGroupTypes(char *type)
 {
  int ig,iet;

  for(ig=0;ig<ng;ig++)
   {
    strcpy((groups[ig]).type,type);
    iet=findGroupTypeNumber(type);
    (groups[ig]).parms=(double*)malloc((elementtypes[iet]).nparms*sizeof(double));
   }
  return;
 }

void setGroupParm(char *name,char *parm,double val)
 {
  int ig,igt;
  int ip;
  int verbose=0;

  if(verbose){printf("setGroupParm(%s,%s,%lf)\n",name,parm,val);fflush(stdout);}
  ig=findGroupNumber(name);
  if(ig<0)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid group name %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }
  if(verbose){printf("  ig=%d\n",ig);fflush(stdout);}

  igt=findGroupTypeNumber((groups[ig]).type);
  if(igt<0)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid group type %s\n",section,lineno,inname,(groups[ig]).type);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }
  if(verbose){printf("  igt=%d\n",igt);fflush(stdout);}

  ip=findGroupParameterNumber(grouptypes+igt,parm);
  if(ip<0)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid parameter %s\n",section,lineno,inname,parm);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }
  if(verbose){printf("  ip=%d\n",ip);fflush(stdout);}

  if((groups[ig]).parms==(double*)NULL)
    (groups[ig]).parms=(double*)malloc((grouptypes[igt]).nparms*sizeof(double));

  ((groups[ig]).parms)[ip]=val;
  return;
 }

int addElementToGroup(char *name,char *ename, double w)
 {
  int ig,ie,j;
  static int verbose=0;

/* All the info for an group function except for the range transformation and the functions */

  ig=findGroupNumber(name);

  ie=-1;
  for(j=0;j<(groups[ig]).nelements;j++)
    if(!strcmp(name,((groups[ig]).elements)[j]))ie=j;
  if(ie!=-1)return 0;

  if(verbose){printf("Add element %s to group %d %s, weight %le\n",ename,ig,name,w);fflush(stdout);}

  if((groups[ig]).nelements>=(groups[ig]).melements)
   {
    (groups[ig]).melements+=10;
    (groups[ig]).elements=(char**)realloc((groups[ig]).elements,(groups[ig]).melements*sizeof(char*));
    (groups[ig]).weights=(double*)realloc((groups[ig]).weights,(groups[ig]).melements*sizeof(double));
    for(j=(groups[ig]).nelements;j<(groups[ig]).melements;j++)
     {
      ((groups[ig]).elements)[j]=(char*)malloc(20*sizeof(char));
      (((groups[ig]).elements)[j])[0]=0x0;
      ((groups[ig]).weights)[j]=1.;
     }
   }
  strcpy(((groups[ig]).elements)[(groups[ig]).nelements],ename);
  ((groups[ig]).weights)[(groups[ig]).nelements]=w;
  ((groups[ig]).nelements)++;

  return 1;
 }

void expandName(char *in,char *out)
 {
/* Must translate name(p,p,p) into namep,p,p */
  int i,j,k;
  char parm[20];
  char val[20];
  double v;
  int iv;
  int first;
  int verbose=0;

  if(verbose){printf("In expandName: -->%s<--\n",in);fflush(stdout);}
  if(in[0]==0x0){out[0]=0x0;return;}
  i=0;
  j=0;
  while(in[i]!=0x0 && in[i]!='('){out[j]=in[i];i++;j++;}
  out[j]=0x0;
  if(in[i]==0x0)return;
  i++;
  if(verbose){printf("   up to (: -->%s<--\n",out);fflush(stdout);}
  if(verbose){printf("   rest: -->%s<--\n",in+i);fflush(stdout);}

  first=1;
  while(in[i]!=0x0 && in[i]!=')')
   {
    k=0;
    while(in[i]!=0x0 && in[i]!=',' && in[i]!=')'){parm[k]=in[i];i++;k++;}
    parm[k]=0x0;
    if(verbose){printf("   next parm: -->%s<--\n",parm);fflush(stdout);}
    iv=(int)findParmValue(parm);
    if(verbose){printf("        value: %lf->%d\n",findParmValue(parm),iv);fflush(stdout);}
    sprintf(val,"%d",iv);
    if(verbose){printf("        value: -->%s<-- len %d\n",val,(int)strlen(val));fflush(stdout);}
    if(!first){out[j]=',';j++;}
    for(k=0;k<(int)strlen(val);k++){out[j]=val[k];j++;}
    out[j]=0x0;
    i++;
    if(verbose){printf("   so far: -->%s<--\n",out);fflush(stdout);}
    if(verbose){printf("   rest: -->%s<--\n",in+i);fflush(stdout);}
    first=0;
   }
  out[j]=0x0;
  if(verbose){printf("done expandName, from: -->%s<-- to: -->%s<--\n",in,out);fflush(stdout);}
  return;
 }

void AddElementCodeLine(char *line)
 {
  int i;
  if(nElementCodeLines>=mElementCodeLines)
   {
    mElementCodeLines+=10;
    elementCodeLines=(char**)realloc((void*)elementCodeLines,mElementCodeLines*sizeof(char*));
    for(i=nElementCodeLines;i<mElementCodeLines;i++)
     {
      elementCodeLines[i]=(char*)malloc(80*sizeof(char));
      (elementCodeLines[i])[0]=0x0;
     }
   }
  strcpy(elementCodeLines[nElementCodeLines],line);
  nElementCodeLines++;
 }

void AddGroupCodeLine(char *line)
 {
  int i;
  if(nGroupCodeLines>=mGroupCodeLines)
   {
    mGroupCodeLines+=10;
    groupCodeLines=(char**)realloc((void*)groupCodeLines,mGroupCodeLines*sizeof(char*));
    for(i=nGroupCodeLines;i<mGroupCodeLines;i++)
     {
      groupCodeLines[i]=(char*)malloc(80*sizeof(char));
      (groupCodeLines[i])[0]=0x0;
     }
   }
  strcpy(groupCodeLines[nGroupCodeLines],line);
  nGroupCodeLines++;
 }

void AddLocalElementCodeLine(char *line)
 {
  int i;
  if(nLocalElementCodeLines>=mLocalElementCodeLines)
   {
    mLocalElementCodeLines+=10;
    localElementCodeLines=(char**)realloc((void*)localElementCodeLines,mLocalElementCodeLines*sizeof(char*));
    for(i=nLocalElementCodeLines;i<mLocalElementCodeLines;i++)
     {
      localElementCodeLines[i]=(char*)malloc(82*sizeof(char));
      (localElementCodeLines[i])[0]=0x0;
     }
   }
  strcpy(localElementCodeLines[nLocalElementCodeLines],line);
  nLocalElementCodeLines++;
 }

void AddLocalGroupCodeLine(char *line)
 {
  int i;
  if(nLocalGroupCodeLines>=mLocalGroupCodeLines)
   {
    mLocalGroupCodeLines+=10;
    localGroupCodeLines=(char**)realloc((void*)localGroupCodeLines,mLocalGroupCodeLines*sizeof(char*));
    for(i=nLocalGroupCodeLines;i<mLocalGroupCodeLines;i++)
     {
      localGroupCodeLines[i]=(char*)malloc(80*sizeof(char));
      (localGroupCodeLines[i])[0]=0x0;
     }
   }
  strcpy(localGroupCodeLines[nLocalGroupCodeLines],line);
  nLocalGroupCodeLines++;
 }

int findInternalVariableNumber(struct ElementType *this,char *name)
 {
  int iiv,niv,j;

  iiv=-1;
  niv=this->nintvars;
  for(j=0;j<niv;j++)
   if(!strcmp(name,(this->intvars)[j]))iiv=j;

  if(iiv<0||iiv>=niv)
   {
    fprintf(stderr,"In %s section, line %d in file %s invalid internal variable %s\n",section,lineno,inname,name);
    fprintf(stderr,"-->%s<--\n",line);
    fflush(stderr);
    abort();
   }

  return iiv;
 }

void DumpProblemToCSource(FILE *fid)
 {
  int i,j,k;
  int igt,iet;
  char upper[80];
  char lower[80];
  char tmp[80];

  fprintf(fid,"#include <NLPAPI.h>\n");
  fprintf(fid,"#include <stdio.h>\n");
  fprintf(fid,"\n");

  for(i=0;i<net;i++)
   {
    strcpy(tmp,rectproblemName);
    strcat(tmp,(elementtypes[i]).name);
    rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"void F77_FUNC(ff%s,FF%s)(F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*);\n",
                      lower,upper);
    fprintf(fid,"void F77_FUNC(fg%s,FG%s)(F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*);\n"
                      ,lower,upper);
    fprintf(fid,"void F77_FUNC(fh%s,FH%s)(F77INTEGER*,F77INTEGER*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*);\n"
                      ,lower,upper);
   }
  fprintf(fid,"\n");

  for(i=0;i<ngt;i++)
   {
    strcpy(tmp,rectproblemName);
    strcat(tmp,(grouptypes[i]).name);
    rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"void F77_FUNC(gf%s,GF%s)(F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*);\n",
                      lower,upper);
    fprintf(fid,"void F77_FUNC(gg%s,GG%s)(F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*);\n"
                      ,lower,upper);
    fprintf(fid,"void F77_FUNC(gh%s,GH%s)(F77DOUBLEPRECISION*,F77DOUBLEPRECISION*,F77DOUBLEPRECISION*);\n"
                      ,lower,upper);
   }
  fprintf(fid,"\n");

  for(i=0;i<net;i++)
   {
    strcpy(tmp,(elementtypes[i]).name);rectString(tmp);
    fprintf(fid,"static double f%s(int,double*,void*);\n",tmp);
    fprintf(fid,"static double df%s(int,int,double*,void*);\n",tmp);
    fprintf(fid,"static double ddf%s(int,int,int,double*,void*);\n",tmp);
   }
  fprintf(fid,"\n");

  for(i=0;i<ngt;i++)
   {
    strcpy(tmp,(grouptypes[i]).name);rectString(tmp);
    fprintf(fid,"static double g%s(double,void*);\n",(grouptypes[i]).name);
    fprintf(fid,"static double dg%s(double,void*);\n",(grouptypes[i]).name);
    fprintf(fid,"static double ddg%s(double,void*);\n",(grouptypes[i]).name);
   }
  fprintf(fid,"\n");

  fprintf(fid,"void CUTEFreeData(void *data)\n");
  fprintf(fid," {\n");
  fprintf(fid,"  free(data);\n");
  fprintf(fid,"  return;\n");
  fprintf(fid," }\n");
  fprintf(fid,"\n");

  fprintf(fid,"NLProblem CUTECreate%s(double **x0,double **l0)\n",rectproblemName);
  fprintf(fid," {\n");
  fprintf(fid,"  NLProblem P;\n");
  if(ngt>0)fprintf(fid,"  NLGroupFunction g[%d];\n",ngt);
  if(net>0)fprintf(fid,"  NLElementFunction f[%d];\n",net);
  if(ne>0)fprintf(fid,"  NLNonlinearElement ne[%d];\n",ne);
  fprintf(fid,"  int group;\n");
  fprintf(fid,"  NLMatrix R;\n");
  fprintf(fid,"  NLVector a;\n");
  fprintf(fid,"  int constraint;\n");
  fprintf(fid,"  int element;\n");
  fprintf(fid,"  int v[%d];\n",nv);
  fprintf(fid,"  int i;\n");
  fprintf(fid,"  int rc;\n");
  fprintf(fid,"  double *data;\n");
  fprintf(fid,"\n");
  fprintf(fid,"/* %s */\n",problemName);
  fprintf(fid,"\n");

  fprintf(fid,"  P=NLCreateProblem(\"%s\",%d);\n",problemName,nv);
  for(i=0;i<nv;i++)
   {
    if((varname[i])[0]!=0x0)fprintf(fid,"  NLPSetVariableName(P,%d,\"%s\");\n",i,varname[i]);
    if(NLPIsUpperSimpleBoundSet(P,i))fprintf(fid,"  NLPSetUpperSimpleBound(P,%d,%le);\n",i,NLPGetUpperSimpleBound(P,i));
    if(NLPIsLowerSimpleBoundSet(P,i))fprintf(fid,"  NLPSetLowerSimpleBound(P,%d,%le);\n",i,NLPGetLowerSimpleBound(P,i));
   }
  fprintf(fid,"\n");

  for(igt=0;igt<ngt;igt++)
   {
    strcpy(tmp,(grouptypes[igt]).name);rectString(tmp);
    fprintf(fid,"  g[%d]=NLCreateGroupFunction(P,\"%s\",g%s,dg%s,ddg%s,(void*)NULL,(void(*)(void*))NULL);\n",igt,(grouptypes[igt]).name,tmp,tmp,tmp);
   }
  fprintf(fid,"\n");

  for(iet=0;iet<net;iet++)
   {
    strcpy(tmp,(elementtypes[iet]).name);rectString(tmp);
    if((elementtypes[iet]).R==(NLMatrix)NULL)
     {
      fprintf(fid,"  f[%d]=NLCreateElementFunction(P,\"%s\",%d,(NLMatrix)NULL,f%s,df%s,ddf%s,(void*)NULL,(void(*)(void*))NULL);\n",
        iet,(elementtypes[iet]).name,(elementtypes[iet]).nelemvars,tmp,tmp,tmp);
     }else{
      fprintf(fid,"  R=NLCreateSparseMatrix(%d,%d);\n",(elementtypes[iet]).nintvars,(elementtypes[iet]).nelemvars);
      for(j=0;j<(elementtypes[iet]).nintvars;j++)
       {
        for(k=0;k<(elementtypes[iet]).nelemvars;k++)
         {
          if(fabs(NLMGetElement((elementtypes[iet]).R,j,k))!=0.)
           fprintf(fid,"  NLMSetElement(R,%d,%d,%le);\n",j,k,NLMGetElement((elementtypes[iet]).R,j,k));
         }
       }
      fprintf(fid,"  f[%d]=NLCreateElementFunction(P,\"%s\",%d,R,f%s,df%s,ddf%s,(void*)NULL,(void(*)(void*))NULL);\n",
        iet,(elementtypes[iet]).name,(elementtypes[iet]).nintvars,tmp,tmp,tmp);
      fprintf(fid,"  NLFreeMatrix(R);\n");
     }
   }
  fprintf(fid,"\n");

  for(i=0;i<ne;i++)
   {
    for(j=0;j<((elements[i]).type)->nelemvars;j++)
     {
      fprintf(fid,"  v[%d]=%d;\n",j,((elements[i]).vars)[j]);
     }
    iet=findElementTypeNumber(((elements[i]).type)->name);
    if((elementtypes[iet]).nparms>0)
     {
      fprintf(fid,"  data=(double*)malloc(%d*sizeof(double));\n",(elementtypes[iet]).nparms);fflush(fid);
      for(j=0;j<(elementtypes[iet]).nparms;j++)
       {
        fprintf(fid,"  data[%d]=%le;\n",j,((elements[i]).parms)[j]);fflush(fid);
       }

      fprintf(fid,"  ne[%d]=NLCreateNonlinearElementParm(P,\"%s\",f[%d],v,(void*)data,CUTEFreeData);\n",i,(elements[i]).name,iet);
     }else{
      fprintf(fid,"  ne[%d]=NLCreateNonlinearElementParm(P,\"%s\",f[%d],v,(void*)NULL,(void(*)(void*))NULL);\n",i,(elements[i]).name,iet);
     }
   }
  fprintf(fid,"\n");

  for(i=0;i<ng;i++)
   {
    switch((groups[i]).gtype)
     {
      case 'O':
       fprintf(fid,"  NLPAddGroupToObjective(P,\"%s\");\n",(groups[i]).name);
       break;
      case 'G':
      case 'L':
       fprintf(fid,"  constraint=NLPAddNonlinearInequalityConstraint(P,\"%s\");\n",(groups[i]).name);
       if(NLPIsInequalityConstraintLowerBoundSet(P,(groups[i]).constraint))
         fprintf(fid,"  NLPSetInequalityConstraintLowerBound(P,constraint,%le);\n",NLPGetInequalityConstraintLowerBound(P,(groups[i]).constraint));
        else
         fprintf(fid,"  NLPUnSetInequalityConstraintLowerBound(P,constraint);\n");
       if(NLPIsInequalityConstraintUpperBoundSet(P,(groups[i]).constraint))
         fprintf(fid,"  NLPSetInequalityConstraintUpperBound(P,constraint,%le);\n",NLPGetInequalityConstraintUpperBound(P,(groups[i]).constraint));
       break;
      case 'E':
       fprintf(fid,"  NLPAddNonlinearEqualityConstraint(P,\"%s\");\n",(groups[i]).name);
       break;
      case 'J':
       fprintf(fid,"  NLPAddMinMaxConstraint(P,\"%s\");\n",(groups[i]).name);
       break;
     }
   }
  if(defaultCONSTANT!=0.)
    fprintf(fid,"  for(i=0;i<%d;i++)rc=NLPSetGroupB(P,i,%le);\n",ng,defaultCONSTANT);
  for(i=0;i<ng;i++)
   {
    if(NLPIsGroupFunctionSet(P,i))
     {
      igt=findGroupTypeNumber((groups[i]).type);
      if((grouptypes[igt]).nparms>0)
       {
        fprintf(fid,"  data=(double*)malloc(%d*sizeof(double));\n",(grouptypes[igt]).nparms);
        for(j=0;j<(grouptypes[igt]).nparms;j++)fprintf(fid,"  data[%d]=%le;\n",j,((groups[i]).parms)[j]);
        fprintf(fid,"  NLPSetGroupFunctionParm(P,%d,g[%d],(void*)data,CUTEFreeData);\n",i,igt);
       }else{
        fprintf(fid,"  NLPSetGroupFunctionParm(P,%d,g[%d],(void*)NULL,(void(*)(void*))NULL);\n",i,igt);
       }
     }
    if((groups[i]).scale!=1.)fprintf(fid,"  rc=NLPSetGroupScale(P,%d,%le);\n",i,(groups[i]).scale);
    if(NLPIsGroupASet(P,i))
     {
      fprintf(fid,"  a=NLCreateVector(%d);\n",nv);
      for(j=0;j<nv;j++)
       if(NLVGetC(NLPGetGroupA(P,i),j)!=0.)
        fprintf(fid,"  rc=NLVSetC(a,%d,%le);\n",j,NLVGetC(NLPGetGroupA(P,i),j));
      fprintf(fid,"  rc=NLPSetGroupA(P,%d,a);\n",i);
      fprintf(fid,"  NLFreeVector(a);\n");
     }
    if(NLPIsGroupBSet(P,i))
      fprintf(fid,"  rc=NLPSetGroupB(P,%d,%le);\n",i,NLPGetGroupB(P,i));
    for(j=0;j<(groups[i]).nelements;j++)
     {
      k=findElementNumber(((groups[i]).elements)[j]);
      fprintf(fid,"  element=NLPAddNonlinearElementToGroup(P,%d,%le,ne[%d]);\n",i,((groups[i]).weights)[j],k);
     }
   }

  if(xStart!=(double*)NULL)
   {
    fprintf(fid,"\n");
    fprintf(fid,"  if(*x0==(double*)NULL)*x0=(double*)malloc(%d*sizeof(double));\n",nv);
    fprintf(fid,"  if(*x0!=(double*)NULL)\n");
    fprintf(fid,"   {\n");
    fprintf(fid,"    for(i=0;i<%d;i++)(*x0)[i]=0.;\n",nv);
    for(i=0;i<nv;i++)
    if(xStart[i]!=0.)fprintf(fid,"    (*x0)[%d]=%le;\n",i,xStart[i]);
    fprintf(fid,"   }\n");
   }

  if(lStart!=(double*)NULL)
   {
    fprintf(fid,"\n");
    fprintf(fid,"  if(*l0==(double*)NULL)*l0=(double*)malloc(%d*sizeof(double));\n",NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P));
    fprintf(fid,"  if(*l0!=(double*)NULL)\n");
    fprintf(fid,"   {\n");
    fprintf(fid,"    for(i=0;i<%d;i++)(*l0)[i]=0.;\n",NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P));
    for(i=0;i<NLPGetNumberOfEqualityConstraints(P)+NLPGetNumberOfInequalityConstraints(P)+NLPGetNumberOfMinMaxConstraints(P);i++)
    if(lStart[i]!=0.)fprintf(fid,"    (*l0)[%d]=%le;\n",i,lStart[i]);
    fprintf(fid,"   }\n");
   }

  fprintf(fid,"\n");
  if(ngt>0)fprintf(fid,"  for(i=0;i<%d;i++)NLFreeGroupFunction(g[i]);\n",ngt);
  if(net>0)fprintf(fid,"  for(i=0;i<%d;i++)NLFreeElementFunction(f[i]);\n",net);
  if(ne>0)fprintf(fid,"  for(i=0;i<%d;i++)NLFreeNonlinearElement(P,ne[i]);\n",ne);

  fprintf(fid,"\n");
  fprintf(fid,"  return P;\n");
  fprintf(fid," }\n");

  for(i=0;i<net;i++)
   {
    fprintf(fid,"\n");
    strcpy(tmp,(elementtypes[i]).name);rectString(tmp);
    fprintf(fid,"double f%s(int n,double *x,void *data)\n",tmp);
    fprintf(fid," {\n");
    fprintf(fid,"  double result;\n");
    if((elementtypes[i]).nparms>0)
     {
      fprintf(fid,"  double p[%d];\n",(elementtypes[i]).nparms);
      fprintf(fid,"  int i;\n");
      fprintf(fid,"  for(i=0;i<%d;i++)p[i]=((double*)data)[i];\n",(elementtypes[i]).nparms);
     }else{
      fprintf(fid,"  double *p;\n");
      fprintf(fid,"  p=(double*)NULL;\n");
     }

    strcpy(tmp,rectproblemName);
    strcat(tmp,(elementtypes[i]).name);rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"  F77_FUNC(ff%s,FF%s)(x,p,&result);\n",
                      lower,upper);
    fprintf(fid,"  return result;\n");
    fprintf(fid," }\n");

    strcpy(tmp,(elementtypes[i]).name);rectString(tmp);
    fprintf(fid,"double df%s(int i,int n,double *x,void *data)\n",tmp);
    fprintf(fid," {\n");
    fprintf(fid,"  double result;\n");
    if((elementtypes[i]).nparms>0)
     {
      fprintf(fid,"  double p[%d];\n",(elementtypes[i]).nparms);
      fprintf(fid,"  int j;\n");
      fprintf(fid,"  for(j=0;j<%d;j++)p[j]=((double*)data)[j];\n",(elementtypes[i]).nparms);
     }else{
      fprintf(fid,"  double *p;\n");
      fprintf(fid,"  p=(double*)NULL;\n");
     }

    strcpy(tmp,rectproblemName);
    strcat(tmp,(elementtypes[i]).name);rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"  F77_FUNC(fg%s,FG%s)(&i,x,p,&result);\n",
                      lower,upper);
    fprintf(fid,"  return result;\n");
    fprintf(fid," }\n");

    strcpy(tmp,(elementtypes[i]).name);rectString(tmp);
    fprintf(fid,"double ddf%s(int i,int j,int n,double *x,void *data)\n",tmp);
    fprintf(fid," {\n");
    fprintf(fid,"  double result;\n");
    if((elementtypes[i]).nparms>0)
     {
      fprintf(fid,"  double p[%d];\n",(elementtypes[i]).nparms);
      fprintf(fid,"  int k;\n");
      fprintf(fid,"  for(k=0;k<%d;k++)p[k]=((double*)data)[k];\n",(elementtypes[i]).nparms);
     }else{
      fprintf(fid,"  double *p;\n");
      fprintf(fid,"  p=(double*)NULL;\n");
     }

    strcpy(tmp,rectproblemName);
    strcat(tmp,(elementtypes[i]).name);rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"  F77_FUNC(fh%s,FH%s)(&i,&j,x,p,&result);\n",
                      lower,upper);
    fprintf(fid,"  return result;\n");
    fprintf(fid," }\n");

   }

  for(i=0;i<ngt;i++)
   {
    fprintf(fid,"\n");
    strcpy(tmp,(grouptypes[i]).name);rectString(tmp);
    fprintf(fid,"double g%s(double x,void *data)\n",tmp);
    fprintf(fid," {\n");
    fprintf(fid,"  double result;\n");
    if((grouptypes[i]).nparms>0)
     {
      fprintf(fid,"  double p[%d];\n",(grouptypes[i]).nparms);
      fprintf(fid,"  int i;\n");
      fprintf(fid,"  for(i=0;i<%d;i++)p[i]=((double*)data)[i];\n",(grouptypes[i]).nparms);
     }else{
      fprintf(fid,"  double *p;\n");
      fprintf(fid,"  p=(double*)NULL;\n");
     }

    strcpy(tmp,rectproblemName);
    strcat(tmp,(grouptypes[i]).name);rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"  F77_FUNC(gf%s,GF%s)(&x,p,&result);\n",
                      lower,upper);
    fprintf(fid,"  return result;\n");
    fprintf(fid," }\n");

    fprintf(fid,"\n");
    strcpy(tmp,(grouptypes[i]).name);rectString(tmp);
    fprintf(fid,"double dg%s(double x,void *data)\n",tmp);
    fprintf(fid," {\n");
    fprintf(fid,"  double result;\n");
    if((grouptypes[i]).nparms>0)
     {
      fprintf(fid,"  double p[%d];\n",(grouptypes[i]).nparms);
      fprintf(fid,"  int i;\n");
      fprintf(fid,"  for(i=0;i<%d;i++)p[i]=((double*)data)[i];\n",(grouptypes[i]).nparms);
     }else{
      fprintf(fid,"  double *p;\n");
      fprintf(fid,"  p=(double*)NULL;\n");
     }

    strcpy(tmp,rectproblemName);
    strcat(tmp,(grouptypes[i]).name);rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"  F77_FUNC(gg%s,GG%s)(&x,p,&result);\n",
                      lower,upper);
    fprintf(fid,"  return result;\n");
    fprintf(fid," }\n");


    fprintf(fid,"\n");
    strcpy(tmp,(grouptypes[i]).name);rectString(tmp);
    fprintf(fid,"double ddg%s(double x,void *data)\n",tmp);
    fprintf(fid," {\n");
    fprintf(fid,"  double result;\n");
    if((grouptypes[i]).nparms>0)
     {
      fprintf(fid,"  double p[%d];\n",(grouptypes[i]).nparms);
      fprintf(fid,"  int i;\n");
      fprintf(fid,"  for(i=0;i<%d;i++)p[i]=((double*)data)[i];\n",(grouptypes[i]).nparms);
     }else{
      fprintf(fid,"  double *p;\n");
      fprintf(fid,"  p=(double*)NULL;\n");
     }

    strcpy(tmp,rectproblemName);
    strcat(tmp,(grouptypes[i]).name);rectString(tmp);
    for(j=0;j<(int)strlen(tmp);j++)upper[j]=toupper(tmp[j]);
    upper[(int)strlen(tmp)]=0x0;
    for(j=0;j<(int)strlen(tmp);j++)lower[j]=tolower(tmp[j]);
    lower[(int)strlen(tmp)]=0x0;
    fprintf(fid,"  F77_FUNC(gh%s,GH%s)(&x,p,&result);\n",
                      lower,upper);
    fprintf(fid,"  return result;\n");
    fprintf(fid," }\n");
   }

  fprintf(finc,"NLProblem CUTECreate%s(double**,double**);\n",rectproblemName);

  return;
 }

int main(int argc, char *argv[])
 {
  NLProblem P;
  double *x0;
  double *l0;

  printf("SIFfile -->%s<--\n",argv[1]);
  P=LNReadSIF(argv[1],&x0,&l0);

  return 0;
 }

void setDoStart(int id,int l)
 {
  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }
  doLimits[4+5*id]=l;
 }

void incrementDoVariable(int id)
 {
  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }

  doLimits[3+5*id]=doLimits[3+5*id]+doLimits[2+5*id];
  setParameter(doVar[id],doLimits[3+5*id]);
  return;
 }

void initializeDoVariable(int id)
 {
  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }

  doLimits[3+5*id]=doLimits[0+5*id];
  setParameter(doVar[id],doLimits[3+5*id]);
  return;
 }

int getDoValue(int id)
 {
  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }
  return doLimits[3+5*id];
 }

int getDoEnd(int id)
 {
  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }
  return doLimits[1+5*id];
 }

int getDoStart(int id)
 {
  if(id<0||id>=nDos)
   {
    fprintf(stderr,"In %s section, line %d in file %s DO %d not found\n",section,lineno,inname,id);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }
  return doLimits[4+5*id];
 }

void addToStack(char *line)
 {
  int i;

  if(nInStack>=mInStack)
   {
    mInStack+=10;
    stack=(char**)realloc(stack,mInStack*sizeof(char*));
    for(i=nInStack;i<mInStack;i++)
     {
      stack[i]=(char*)malloc(81*sizeof(char));
      (stack[i])[0]=0x0;
     }
   }
  strcpy(stack[nInStack],line);
  nInStack++;
  cInStack=nInStack;
  return;
 }

char *getLineFromStack(int i)
 {
  if(i<0||i>=nInStack)
   {
    fprintf(stderr,"In %s section, line %d in file %s tried to get line %d from stack of length %d\n",section,lineno,inname,i,nInStack);
    fprintf(stderr,"-->%s<--\n",line);
    fprintf(stderr,"Line %d of file %s<--\n",__LINE__,__FILE__);
    fflush(stderr);
    abort();
   }
  return stack[i];
 }

char *getNextLineFromStack()
 {
  return stack[cInStack];
 }

void AddElementDeclLine(char *line)
 {
  int i;

/* Is it there already? */

  for(i=0;i<nElementDeclLines;i++)
   {
    if(!strcmp(elementDeclLines[i],line))return;
   }

  if(nElementDeclLines>=mElementDeclLines)
   {
    mElementDeclLines+=10;
    elementDeclLines=(char**)realloc((void*)elementDeclLines,mElementDeclLines*sizeof(char*));
    for(i=nElementDeclLines;i<mElementDeclLines;i++)
     {
      elementDeclLines[i]=(char*)malloc(80*sizeof(char));
      (elementDeclLines[i])[0]=0x0;
     }
   }
  strcpy(elementDeclLines[nElementDeclLines],line);
  nElementDeclLines++;
 }

void AddGroupDeclLine(char *line)
 {
  int i;

  if(nGroupDeclLines>=mGroupDeclLines)
   {
    mGroupDeclLines+=10;
    groupDeclLines=(char**)realloc((void*)groupDeclLines,mGroupDeclLines*sizeof(char*));
    for(i=nGroupDeclLines;i<mGroupDeclLines;i++)
     {
      groupDeclLines[i]=(char*)malloc(80*sizeof(char));
      (groupDeclLines[i])[0]=0x0;
     }
   }
  strcpy(groupDeclLines[nGroupDeclLines],line);
  nGroupDeclLines++;
 }

void rectifyProblemName()
 {
  int i,j;

  j=0;
  for(i=0;i<(int)strlen(problemName);i++)
   {
    if(isalnum(problemName[i])){rectproblemName[j]=problemName[i];j++;}
   }
  rectproblemName[j]=0x0;
  return;
 }

void rectString(char *s)
 {
  int i;

  for(i=0;i<(int)strlen(s);i++)
    if(!isalnum(s[i]))s[i]='Z';

  return;
 }

int findConstraintNumber(char *name)
 {
  int i,ig;

  i=0;
  for(ig=0;ig<ng;ig++)
   {
    if((groups[ig]).gtype=='E')
     {
      if(!strcmp(name,(groups[ig]).name))return i;
      i++;
     }
   }
  for(ig=0;ig<ng;ig++)
   {
    if((groups[ig]).gtype=='G'|| (groups[ig]).gtype=='L')
     {
      if(!strcmp(name,(groups[ig]).name))return i;
      i++;
     }
   }
  for(ig=0;ig<ng;ig++)
   {
    if((groups[ig]).gtype=='J')
     {
      if(!strcmp(name,(groups[ig]).name))return i;
      i++;
     }
   }

  fprintf(stderr,"SIFfile %s, LINE %d in file %s, Group %s is not an equal, inequal, or minmax group\n",inname,__LINE__,__FILE__,name);
  fflush(stderr);
  abort();

  return -1;
 }

void setLinearElementOfGroup(char *group,int iv,double a)
 {
  int ne;
  static int verbose=0;

  if(varnLE[iv]+1>varmLE[iv])
   {
    varmLE[iv]+=10;
    varLEi[iv]=(int*)realloc((void*)(varLEi[iv]),varmLE[iv]*sizeof(int));
    varLEv[iv]=(double*)realloc((void*)(varLEv[iv]),varmLE[iv]*sizeof(double));
   }
  ne=varnLE[iv];
  varnLE[iv]++;
  (varLEi[iv])[ne]=findGroupNumber(group);
  (varLEv[iv])[ne]=a;
  if(verbose){printf("  setLinearElementOfGroup, group %d (%s), variable %d has coefficient %le\n",(varLEi[iv])[ne],group,iv,a);fflush(stdout);}

  return;
 }
