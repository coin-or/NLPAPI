/*                                                          */
/*  @(#)ECFunction.c	1.6                                                     */
/*  02/05/03 10:21:24                                                 */
/*                                                          */

/*   PROGRAM NAME:  ExpressionCompiler v1.0                 */
 
/*   (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES          */
/*   CORPORATION March 9, 1995.  ALL RIGHTS RESERVED.       */

/*  Please refer to the LICENSE file in the top directory */

/* Author: Mike Henderson                                     */
/* Date:   April 9, 1999                                      */

#include <ExpCmp.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void ECParseExpressionMatrix(char *expression,int *nRange,int *nDomain,char ***source);

struct ECFunctionSt
 {
  int n;
  int m;
  char **var;
  struct ECObjectCode **code;
  char **source;
 };

ECFn ECCreateFunction(char *vars,char *expression)
 {
  int i=0;
  int j=0;
  char **source=(char**)NULL;
  ECFn F;
  int rc;
  int verbose=0;

  F=(ECFn)malloc(sizeof(struct ECFunctionSt));
  ECParseExpressionMatrix(vars,&i,&j,&(F->var));
  F->m=j;
  if(verbose)
   {
    printf("Function Matrix ==>%s<==\n",expression);fflush(stdout);
    printf("Variables are ==>%s<==\n",vars);fflush(stdout);
   }
  ECParseExpressionMatrix(expression,&i,&j,&source);
  F->n=j;
  F->code=(struct ECObjectCode**)malloc(F->n*sizeof(struct ECObjectCode*));
  F->source=(char**)malloc(F->n*sizeof(char*));
  if(verbose)
   {
    for(i=0;i<F->m;i++)
     {printf("variable %d is ==>%s<==\n",i,(F->var)[i]);fflush(stdout);}
   }
  for(i=0;i<F->n;i++)
   {
    if(verbose){printf("Function %d is ==>%s<==\n",i,source[i]);fflush(stdout);}
    rc=ECCompileExpression(source[i],&(F->code[i]));
    if(verbose){printf("Return Code from Compile is %d\n",rc);fflush(stdout);}
    rc=ECSetStandardMathFunctions(F->code[i]);
    if(verbose){printf("Return Code from ECSetStandardMathFunctions is %d\n",rc);fflush(stdout);}
    rc=ECSetStandardMathConstants(F->code[i]);
    if(verbose){printf("Return Code from ECSetStandardMathConstants is %d\n",rc);fflush(stdout);}
    F->source[i]=source[i];
   }

  return F;
 }

void ECFreeFunction(ECFn F)
 {
  int i;

  if(F==(ECFn)NULL)return;

  if(F->var!=(char**)NULL)
   {
    for(i=0;i<F->m;i++)if(F->var[i]!=(char*)NULL)free(F->var[i]);
    free(F->var);
   }
  if(F->code!=(struct ECObjectCode**)NULL)
   {
    for(i=0;i<F->n;i++)
      if(F->code[i]!=(struct ECObjectCode*)NULL)ECFreeObjectCode(&(F->code[i]));
    free(F->code);
   }
  if(F->source!=(char**)NULL)
   {
    for(i=0;i<F->n;i++)
      if(F->source[i]!=(char*)NULL)free(F->source[i]);
    free(F->source);
   }

  free(F);
 }

ECFn ECCreateDerivativeOfFunction(ECFn F,int dir)
 {
  int i;
  ECFn dF;
  int rc;

  dF=(ECFn)malloc(sizeof(struct ECFunctionSt));
  dF->n=F->n;
  dF->m=F->m;
  dF->var=(char**)malloc(F->m*sizeof(char*));
  for(i=0;i<F->m;i++)
   {
    dF->var[i]=(char*)malloc((strlen(F->var[i])+1)*sizeof(char));
    strcpy(dF->var[i],F->var[i]);
   }
  
  dF->code=(struct ECObjectCode**)malloc(F->n*sizeof(struct ECObjectCode*));
  dF->source=(char**)malloc(F->n*sizeof(char*));
  for(i=0;i<F->n;i++)
   {
    rc=ECCreateExpressionDerivative(F->code[i],F->var[dir],&(dF->code[i]));
    rc=ECSetStandardMathFunctions(dF->code[i]);
    rc=ECSetStandardMathConstants(dF->code[i]);
    dF->source[i]=(char*)malloc(10*sizeof(char));
    strcpy(dF->source[i],"No Source");
   }

  return dF;
 }
#include <errno.h>
#include <string.h>

void ECEvaluateFunction(ECFn F,double *x,double *y)
 {
  int i,j,rc;
  static int verbose=0;

  for(i=0;i<F->n;i++)
   {
    if(verbose){printf("ECEvaluateExpression component %d\n",i);fflush(stdout);}
    for(j=0;j<F->m;j++)
     {
      if(verbose){printf("ECEvaluateFunction: Set variable %d(%s) to %lf\n",j,F->var[j],x[j]);fflush(stdout);}
      rc=ECSetIdentifierToReal(F->var[j],x[j],F->code[i]);
     }
    if(errno>0){printf("%s line %d: %s\n",__FILE__,__LINE__,strerror(errno));fflush(stdout);}
    y[i]=ECEvaluateExpression(F->code[i],&rc);
    if(verbose){printf("    value = (%le)\n",y[i]);fflush(stdout);}
    if(errno>0){printf("%s line %d: %s\n",__FILE__,__LINE__,strerror(errno));fflush(stdout);}

   }

  return;
 }


void ECParseExpressionMatrix(char *expression,int *nRange,int *nDomain,char ***source)
 {
  int i,n;
  int j,k;
  int end;
  int j0,j1;
  int endT;
  char *e;
  char *t;

  int verbose;

  verbose=0;

/*     [ [...],[...],[...] ]     */

  *nRange=0;
  *nDomain=0;

/* Remove white space */

  n=strlen(expression);
  e=(char*)malloc((n+1)*sizeof(char));

  end=0;
  for(i=0;i<n;i++)
   {
    if(!isspace(expression[i]))
     {
     e[end]=expression[i];
     end++;
     }
   }
  e[end]=0x0;
  if(verbose)printf("ECParseExpressionMatrix. source w/o blanks is -->%s<--\n",e);

  t=(char*)malloc((n+1)*sizeof(char));

  endT=0;
  for(i=0;i<end;i++)
   {
    if(e[i]=='[' || e[i]==',' || e[i]==']' )
     {
     t[endT]=e[i];
     endT++;
     }
   }
  t[endT]=0x0;
  if(verbose)printf("     template is -->%s<--\n",t);

  if(endT==0||t[0]!='['||t[endT-1]!=']')
   {

/* Only one string   */

    *nRange=1;
    *nDomain=1;
    *source=(char**)malloc(sizeof(char*));
    (*source)[0]=(char*)malloc((strlen(e)+1)*sizeof(char));
    strcpy((*source)[0],e);
    if(verbose)printf("     template does not begin and end with [], only one expression. \n");

    free(e);
    free(t);
    return;
   }

/*  First and last character's '[' and ']', looking for vector or matrix */

  end--;

  if(t[1]!='['||t[endT-2]!=']')
   {
/*  First and last character's not '[[' and ']]', looking for vector */

/* Only one row of strings */
    if(verbose)printf("     template does not begin and end with [[...]], only one row. \n");

    if(!strcmp(e,"[]"))
     {
      *nRange=0;
      *nDomain=0;
      *source=(char**)NULL;
     }else{
      *nRange=1;
      *nDomain=1;
      for(i=0;i<endT;i++)
       if(t[i]==',')(*nDomain)++;
      *source=(char**)malloc((*nDomain)*sizeof(char*));
      if(verbose)printf("     there are %d commas \n",*nDomain-1);

      j0=1;
      for(i=0;i<*nDomain;i++)
       {
        j1=j0;
        while(e[j1]!=','&&e[j1]!=']')j1++;
        (*source)[i]=(char*)malloc((j1-j0+1)*sizeof(char));
        strncpy((*source)[i],e+j0,j1-j0);
        ((*source)[i])[j1-j0]=0x0;
        j0=j1+1;
       }
     }
    fflush(stdout);
    free(e);
    free(t);
    return;
   }

/* more than one row of strings */


/*        First and last character's '[[' and ']]', looking for matrix  */

  if(verbose)printf("     template begins and ends with [[...]]. \n");
  *nRange=1;
  for(i=2;i<endT;i++)
    if(t[i]=='[')(*nRange)++;
  if(verbose)printf("     There are %d ['s after the first.\n",*nRange);

  if(verbose)printf("     Counting commas in the first [...] clause.\n");
  *nDomain=1;
  i=2;
  while(t[i]!=']')
   {
    if(verbose)printf("%c",t[i]);
    if(t[i]==',')(*nDomain)++;
    i++;
   }
  if(verbose)printf("     There are %d ['s after the first.\n",*nDomain-1);

  *source=(char**)malloc((*nDomain)*(*nRange)*sizeof(char*));
  j0=2;
  for(j=0;j<*nRange;j++)
   {
    for(i=0;i<*nDomain;i++)
     {
      j1=j0;
      while(e[j1]!=','&&e[j1]!=']'&&j1<end)j1++;
      if(j1==end || (e[j1]==']' && i<*nDomain-1))
       {
        fprintf(stderr,"ECParseExpressionMatrix: syntax error: %s\n",e);
        if(j1==end)fprintf(stderr,"                         j1==end\n");
           else    fprintf(stderr,"     Expecting another string: %s\n",t);
        nRange=0;
        nDomain=0;
        return;
       }
      (*source)[j+(*nRange)*i]=(char*)malloc((j1-j0+1)*sizeof(char));
      strncpy((*source)[j+(*nRange)*i],e+j0,j1-j0);
      ((*source)[j+(*nRange)*i])[j1-j0]=0x0;
      j0=j1+1;
     }
    j0=j0+2;
   }
  
  free(e);
  free(t);

  return;
 }

int ECFunctionN(ECFn F)
 {
  return F->n;
 }

int ECFunctionM(ECFn F)
 {
  return F->m;
 }

void ECPrintFunction(FILE *fid, ECFn F)
 {
  int i,n;

  n=F->n;
  printf("ECFunction:\n");
  for(i=0;i<n;i++)
   {
    printf("Component %d\n",i);
    ECPrintSymbolTable(F->code[i]);fflush(stdout);
    ECPrintObjectCode(F->code[i]);fflush(stdout);
   }
  return;
 }
