void F77_FUNC(USETUP,usetup)(int *in, int *out, int *n, double *bl, double *bu, int *nmax);
void F77_FUNC(UNAMES,unames)(int *n, char *PNAME, char **XNAMES);
void F77_FUNC(UFN,ufn)(int *n, double *x, double *f);
void F77_FUNC(UGR,ugr)(int *n, double *x, double *g);
void F77_FUNC(UOFG,uofg)(int *n, double *x, double *f, double *g, int *grad);
void F77_FUNC(UDH,udh)(int *n, double *x, int *lh1, double *h);
void F77_FUNC(UGRDH,ugrdh)(int *n, double *x, double *g, int *lh1, double *h);
void F77_FUNC(USH,ush)(int *n, double *x, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(UGRSH,ugrsh)(int *n, double *x, double *g, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(UEH,ueh)(int *n, double *x, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(UGREH,ugreh)(int *n, double *x, double *g, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(UPROD,uprod)(int *n, int *goth, double *x, double *p, double *q);
void F77_FUNC(UBANDH,ubandh)(int *n, int *goth, double *x, int *nsemib, int *bandh, int *lbandh);

void F77_FUNC(CSETUP,csetup)(int *in, int *out, int *n, int *m, double *x, double *bl, double *bu, int *nmax, int *equatn, int *linear, double *v, double *cl, double *cu, int *mmax, int *efirst, int *lfirst, int *nvfrst);
void F77_FUNC(CNAMES,cnames)(int *n, char *PNAME, char **XNAMES, char *gname*s);
void F77_FUNC(CFN,cfn)(int *n, int *m, double *x, double *f, int *lc, double *c);
void F77_FUNC(CGR,cgr)(int *n, int *m, double *x, int *grlagf, int *lv, double *v, double *g, int *jtrans, int *lcjac1, int *lcjac2, double *cjac);
void F77_FUNC(COFG,cofg)(int *n, double *x, double *f, double *g, int *grad);

void F77_FUNC(CDH,cdh)(int *n, int *m, double *x, int *lh1, double *h);
void F77_FUNC(CGRDH,cgrdh)(int *n, int *m, double *x, double *g, int *lh1, double *h);
void F77_FUNC(CSH,csh)(int *n, int *m, double *x, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(CGRSH,cgrsh)(int *n, int *m, double *x, double *g, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(CEH,ceh)(int *n, int *m, double *x, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(CGREH,cgreh)(int *n, int *m, double *x, double *g, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(CPROD,cprod)(int *n, int *m, int *goth, double *x, double *p, double *q);
void F77_FUNC(CBANDH,cbandh)(int *n, int *m, int *goth, double *x, int *nsemib, int *bandh, int *lbandh);

