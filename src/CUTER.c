\* Obviously this isn't implemented yet. */
void F77_FUNC(usetup,USETUP)(int *in, int *out, int *n, double *bl, double *bu, int *nmax);
void F77_FUNC(unames,UNAMES)(int *n, char *PNAME, char **XNAMES);
void F77_FUNC(ufn,UFN)(int *n, double *x, double *f);
void F77_FUNC(ugr,UGR)(int *n, double *x, double *g);
void F77_FUNC(uofg,UOFG)(int *n, double *x, double *f, double *g, int *grad);
void F77_FUNC(udh,UDH)(int *n, double *x, int *lh1, double *h);
void F77_FUNC(ugrdh,UGRDH)(int *n, double *x, double *g, int *lh1, double *h);
void F77_FUNC(ush,USH)(int *n, double *x, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(ugrsh,UGRSH)(int *n, double *x, double *g, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(ueh,UEH)(int *n, double *x, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(ugreh,UGREH)(int *n, double *x, double *g, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(uprod,UPROD)(int *n, int *goth, double *x, double *p, double *q);
void F77_FUNC(ubandh,UBANDH)(int *n, int *goth, double *x, int *nsemib, int *bandh, int *lbandh);

void F77_FUNC(csetup,CSETUP)(int *in, int *out, int *n, int *m, double *x, double *bl, double *bu, int *nmax, int *equatn, int *linear, double *v, double *cl, double *cu, int *mmax, int *efirst, int *lfirst, int *nvfrst);
void F77_FUNC(cnames,CNAMES)(int *n, char *PNAME, char **XNAMES, char *gname*s);
void F77_FUNC(cfn,CFN)(int *n, int *m, double *x, double *f, int *lc, double *c);
void F77_FUNC(cgr,CGR)(int *n, int *m, double *x, int *grlagf, int *lv, double *v, double *g, int *jtrans, int *lcjac1, int *lcjac2, double *cjac);
void F77_FUNC(cofg,COFG)(int *n, double *x, double *f, double *g, int *grad);

void F77_FUNC(cdh,CDH)(int *n, int *m, double *x, int *lh1, double *h);
void F77_FUNC(cgrdh,CGRDH)(int *n, int *m, double *x, double *g, int *lh1, double *h);
void F77_FUNC(csh,CSH)(int *n, int *m, double *x, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(cgrsh,CGRSH)(int *n, int *m, double *x, double *g, int *nnzsh, int *lsh, double *sh, int *irnsh, int *icnsh);
void F77_FUNC(ceh,CEH)(int *n, int *m, double *x, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(cgreh,CGREH)(int *n, int *m, double *x, double *g, int *ne, int *irnhi, int *lrnhi, int *le, int *iprnhi, double *hi, int *lhi, int *iprhi, int *byrows);
void F77_FUNC(cprod,CPROD)(int *n, int *m, int *goth, double *x, double *p, double *q);
void F77_FUNC(cbandh,CBANDH)(int *n, int *m, int *goth, double *x, int *nsemib, int *bandh, int *lbandh);

