/*  (c) COPYRIGHT INTERNATIONAL BUSINESS MACHINES
    CORPORATION 2/10/1999.  ALL RIGHTS RESERVED.

    Please refer to the LICENSE file in the top directory*/

/*      author: Mike Henderson mhender@watson.ibm.com */
/*      version: @(#)Error.c	3.1 02/03/05 15:24:23 */
/*      date:   Feb. 10, 1999                         */

#include <NLPAPI.h>
#include <unistd.h>
#include <string.h>

int NNLError=0;
int NLMError=0;
int *NLErrorSev=(int*)NULL;
char **NLErrorRtn=(char**)NULL;
char **NLErrorMsg=(char**)NULL;
char **NLErrorFile=(char**)NULL;
int *NLErrorLine=(int*)NULL;

void NLSetError(int sev,char *routine,char *msg,int line,char *file)
 {
/*printf("NLSetError(%d,\"%s\",\"%s\",%d,\"%s\");\n",sev,routine,msg,line,file);fflush(stdout);*/
  if(!(NNLError<NLMError))
   {
    NLMError++;
    NLErrorSev=(int*)realloc((void*)NLErrorSev,NLMError*sizeof(int));
    if(NLErrorSev==(int*)NULL)
     {
      printf("Catastrophic error in NLSetError, out of memory trying to alloc %d bytes for NLErrorSev, line %d in file %s\n",
             NLMError*sizeof(int),__LINE__,__FILE__);
      printf("  Error message in stack is NLSetError(%d,\"%s\",\"%s\",%d,\"%s\");\n",sev,routine,msg,line,file);
      fflush(stdout);
      abort();
     }
    NLErrorRtn=(char**)realloc((void*)NLErrorRtn,NLMError*sizeof(char*));
    if(NLErrorRtn==(char**)NULL)
     {
      printf("Catastrophic error in NLSetError, out of memory trying to alloc %d bytes for NLErrorRtn, line %d in file %s\n",
             NLMError*sizeof(int),__LINE__,__FILE__);
      printf("  Error message in stack is NLSetError(%d,\"%s\",\"%s\",%d,\"%s\");\n",sev,routine,msg,line,file);
      fflush(stdout);
      abort();
     }
    NLErrorMsg=(char**)realloc((void*)NLErrorMsg,NLMError*sizeof(char*));
    if(NLErrorMsg==(char**)NULL)
     {
      printf("Catastrophic error in NLSetError, out of memory trying to alloc %d bytes for NLErrorMsg, line %d in file %s\n",
             NLMError*sizeof(int),__LINE__,__FILE__);
      printf("  Error message in stack is NLSetError(%d,\"%s\",\"%s\",%d,\"%s\");\n",sev,routine,msg,line,file);
      fflush(stdout);
      abort();
     }
    NLErrorFile=(char**)realloc((void*)NLErrorFile,NLMError*sizeof(char*));
    if(NLErrorFile==(char**)NULL)
     {
      printf("Catastrophic error in NLSetError, out of memory trying to alloc %d bytes for NLErrorFile, line %d in file %s\n",
             NLMError*sizeof(int),__LINE__,__FILE__);
      printf("  Error message in stack is NLSetError(%d,\"%s\",\"%s\",%d,\"%s\");\n",sev,routine,msg,line,file);
      fflush(stdout);
      abort();
     }
    NLErrorLine=(int*)realloc((void*)NLErrorLine,NLMError*sizeof(int));
    if(NLErrorLine==(int*)NULL)
     {
      printf("Catastrophic error in NLSetError, out of memory trying to alloc %d bytes for NLErrorLine, line %d in file %s\n",
             NLMError*sizeof(int),__LINE__,__FILE__);
      printf("  Error message in stack is NLSetError(%d,\"%s\",\"%s\",%d,\"%s\");\n",sev,routine,msg,line,file);
      fflush(stdout);
      abort();
     }
   }
  NLErrorSev[NNLError]=sev;
  NLErrorRtn[NNLError]=(char*)malloc((strlen(routine)+1)*sizeof(char));
  strcpy(NLErrorRtn[NNLError],routine);
  NLErrorMsg[NNLError]=(char*)malloc((strlen(msg)+1)*sizeof(char));
  strcpy(NLErrorMsg[NNLError],msg);
  NLErrorFile[NNLError]=(char*)malloc((strlen(file)+1)*sizeof(char));
  strcpy(NLErrorFile[NNLError],file);
  NLErrorLine[NNLError]=line;
/*if(sev>8)*/
   {
    printf("  NLErrorSev[%d]=%d\n",NNLError,NLErrorSev[NNLError]);fflush(stdout);
    printf("  NLErrorRtn[%d]=\"%s\"\n",NNLError,NLErrorRtn[NNLError]);fflush(stdout);
    printf("  NLErrorMsg[%d]=\"%s\"\n",NNLError,NLErrorMsg[NNLError]);fflush(stdout);
    printf("  NLErrorFile[%d]=\"%s\"\n",NNLError,NLErrorFile[NNLError]);fflush(stdout);
    printf("  NLErrorLine[%d]=%d\n",NNLError,NLErrorLine[NNLError]);fflush(stdout);
    fflush(stdout);
    abort();
  }
  NNLError++;

  return;
 }

int NLGetNErrors()
 {
  return NNLError;
 }

int NLGetErrorSev(int n)
 {
  if(n<0||!(n<NNLError))return 0;
/*printf("  NLGetErrorSev(%d)=%d\n",n,NLErrorSev[n]);fflush(stdout);*/
  return NLErrorSev[n];
 }

char *NLGetErrorRoutine(int n)
 {
  if(n<0||!(n<NNLError))return (char*)NULL;
/*printf("  NLErrorRtn(%d)=\"%s\"\n",n,NLErrorRtn[n]);fflush(stdout);*/
  return NLErrorRtn[n];
 }

char *NLGetErrorMsg(int n)
 {
  if(n<0||!(n<NNLError))return (char*)NULL;
/*printf("  NLGetErrorMsg(%d)=\"%s\"\n",n,NLErrorMsg[n]);fflush(stdout);*/
  return NLErrorMsg[n];
 }

int NLGetErrorLine(int n)
 {
  if(n<0||!(n<NNLError))return 0;
/*printf("  NLGetErrorLine(%d)=%d\n",n,NLErrorLine[n]);fflush(stdout);*/
  return NLErrorLine[n];
 }

char *NLGetErrorFile(int n)
 {
  if(n<0||!(n<NNLError))return (char*)NULL;
/*printf("  NLGetErrorFile(%d)=\"%s\"\n",n,NLErrorFile[n]);fflush(stdout);*/
  return NLErrorFile[n];
 }

int NLError()
 {
  return(NNLError!=0);
 }

void NLClearErrors()
 {
  int i;
  for(i=0;i<NNLError;i++)
   {
    if(NLErrorRtn[i]!=(char*)NULL)free(NLErrorRtn[i]);
    if(NLErrorMsg[i]!=(char*)NULL)free(NLErrorMsg[i]);
    if(NLErrorFile[i]!=(char*)NULL)free(NLErrorFile[i]);
   }
  if(NLErrorSev!=(int*)NULL)free(NLErrorSev);NLErrorSev=(int*)NULL;
  if(NLErrorRtn!=(char**)NULL)free(NLErrorRtn);NLErrorRtn=(char**)NULL;
  if(NLErrorMsg!=(char**)NULL)free(NLErrorMsg);NLErrorMsg=(char**)NULL;
  if(NLErrorFile!=(char**)NULL)free(NLErrorFile);NLErrorFile=(char**)NULL;
  if(NLErrorLine!=(int*)NULL)free(NLErrorLine);NLErrorLine=(int*)NULL;
  NLMError=0;
  NNLError=0;
  return;
 }
