#include <stdio.h>
#include <sys/stat.h>
#include <sys/fullstat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <ar.h>
#include <ldfcn.h>

int main(int argc, char *argv[])
 {
  time_t modtime1;
  struct fullstat Buffer1;
  time_t modtime2;
  struct fullstat Buffer2;
  int rc;
  char lib[1024];
  char mem[1024];
  int i,j;
  LDFILE *ldPointer=(LDFILE*)NULL;
  ARCHDR ArchiveHeader;

  if(strchr(argv[1],')')!=(char*)NULL)
   {
    i=0;
    while((argv[1])[i]!='('){lib[i]=(argv[1])[i];i++;}
    lib[i]=0x0;i++;j=0;
    while((argv[1])[i]!=')'){mem[j]=(argv[1])[i];i++;j++;}
    mem[j]=0x0;
    ldPointer=(LDFILE*)NULL;
    ldPointer=ldopen(lib,ldPointer);
    do{
      ldahread(ldPointer,&ArchiveHeader);
      if(!strcmp(ArchiveHeader.ar_name,mem))
        modtime1=(ldPointer->header).f_timdat;
     }while(ldclose(ldPointer) == FAILURE );
   }else{
    rc=fullstat (argv[1], STX_NORMAL, &Buffer1);
    modtime1=Buffer1.st_mtime;
   }

  if(strchr(argv[2],')')!=(char*)NULL)
   {
    i=0;
    while((argv[2])[i]!='('){lib[i]=(argv[2])[i];i++;}
    lib[i]=0x0;i++;j=0;
    while((argv[2])[i]!=')'){mem[j]=(argv[2])[i];i++;j++;}
    mem[j]=0x0;
    ldPointer=(LDFILE*)NULL;
    ldPointer=ldopen(lib,ldPointer);
    do{
      ldahread(ldPointer,&ArchiveHeader);
      if(!strcmp(ArchiveHeader.ar_name,mem))
        modtime2=(ldPointer->header).f_timdat;
     }while(ldclose(ldPointer) == FAILURE );
   }else{
    rc=fullstat (argv[2], STX_NORMAL, &Buffer2);
    modtime2=Buffer2.st_mtime;
   }

  if( difftime(modtime1,modtime2)>0.)
   {
/*  fprintf(stderr,"%s is newer than %s\n",argv[1],argv[2]);fflush(stderr);*/
    printf("1\n");fflush(stdout);
    return 1;
   }else{
/*  fprintf(stderr,"%s is not newer than %s\n",argv[1],argv[2]);fflush(stderr);*/
    printf("0\n");fflush(stdout);
    return 0;
   }
 }
