#include <NLPAPI.h>
static char NLHSLErrorMsg[80]="";

#ifndef HAVE_MC29
void F77_FUNC(mc29a,MC29A)(F77INTEGER*a,F77INTEGER*b,F77INTEGER*c,F77REAL*d,F77INTEGER*e,F77INTEGER*f,F77REAL*g,F77REAL*h,F77REAL*i,F77INTEGER*j,F77INTEGER*k)
 {
  static char RoutineName[]="MC29A";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MC29D
void F77_FUNC(mc29ad,MC29AD)(F77INTEGER*a,F77INTEGER*b,F77INTEGER*c,F77DOUBLEPRECISION*d,F77INTEGER*e,F77INTEGER*f,F77DOUBLEPRECISION*g,F77DOUBLEPRECISION*h,F77DOUBLEPRECISION*i,F77INTEGER*j,F77INTEGER*k)
 {
  static char RoutineName[]="MC29AD";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA27A
void F77_FUNC(ma27a,MA27A)(F77INTEGER*a,F77INTEGER*b,F77INTEGER*c,F77INTEGER*d,F77INTEGER*e,F77INTEGER*f,F77INTEGER*g,F77INTEGER*h,F77INTEGER*i,F77INTEGER*j)
 {
  static char RoutineName[]="MC27A";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA27AD
void F77_FUNC(ma27ad,MA27AD)(F77INTEGER*a,F77INTEGER*b,F77INTEGER*c,F77INTEGER*d,F77INTEGER*e,F77INTEGER*f,F77INTEGER*g,F77INTEGER*h,F77INTEGER*i,F77INTEGER*j)
 {
  static char RoutineName[]="MC27AD";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA27B
void F77_FUNC(ma27b,MA27B)(F77INTEGER*a,F77INTEGER*b,F77INTEGER*c,F77INTEGER*d,F77REAL*e,F77INTEGER*f,F77INTEGER*g,F77INTEGER*h,F77INTEGER*i,F77INTEGER*j,F77INTEGER*k,F77INTEGER*l,F77INTEGER*m)
 {
  static char RoutineName[]="MC27B";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA27BD
void F77_FUNC(ma27bd,MA27BD)(F77INTEGER*a,F77INTEGER*b,F77INTEGER*c,F77INTEGER*d,F77DOUBLEPRECISION*e,F77INTEGER*f,F77INTEGER*g,F77INTEGER*h,F77INTEGER*i,F77INTEGER*j,F77INTEGER*k,F77INTEGER*l,F77INTEGER*m)
 {
  static char RoutineName[]="MC27BD";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA27C
void F77_FUNC(ma27c,MA27C)(F77INTEGER*a,F77REAL*b,F77INTEGER*c,F77INTEGER*d,F77INTEGER*e,F77REAL*f,F77INTEGER*g,F77REAL*h,F77INTEGER*i,F77INTEGER*j)
 {
  static char RoutineName[]="MC27C";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA27CD
void F77_FUNC(ma27cd,MA27CD)(F77INTEGER*a,F77DOUBLEPRECISION*b,F77INTEGER*c,F77INTEGER*d,F77INTEGER*e,F77DOUBLEPRECISION*f,F77INTEGER*g,F77DOUBLEPRECISION*h,F77INTEGER*i,F77INTEGER*j)
 {
  static char RoutineName[]="MC27CD";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA31D
void F77_FUNC(ma31d,MA31D)(F77REAL*a,F77INTEGER*b,F77INTEGER*c,F77INTEGER*d,F77INTEGER*e,F77INTEGER*f,F77INTEGER*g)
 {
  static char RoutineName[]="MC31D";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif

#ifndef HAVE_MA31DD
void F77_FUNC(ma31dd,MA31DD)(F77DOUBLEPRECISION*a,F77INTEGER*b,F77INTEGER*c,F77INTEGER*d,F77INTEGER*e,F77INTEGER*f,F77INTEGER*g)
 {
  static char RoutineName[]="MC31DD";

  sprintf(NLHSLErrorMsg,"The HSL routine %s is not present, but is used.",RoutineName);
  NLSetError(12,RoutineName,NLHSLErrorMsg,__LINE__,__FILE__);
  return;
 }
#endif
