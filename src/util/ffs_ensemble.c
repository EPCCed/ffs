/*****************************************************************************
 *
 *  ffs_ensemble.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "u/libu.h"
#include "ffs_ensemble.h"


/*****************************************************************************
 *
 *  ffs_ensemble_create
 *
 *****************************************************************************/

int ffs_ensemble_create(int nmax, ffs_ensemble_t ** pobj) {

  ffs_ensemble_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);

  obj = u_calloc(1, sizeof(ffs_ensemble_t));
  dbg_err_sif(obj == NULL);

  obj->nmax = nmax;
  obj->nsuccess = 0;

  obj->traj = u_calloc(nmax, sizeof(int));
  dbg_err_sif(obj->traj == NULL);

  obj->wt = u_calloc(nmax, sizeof(double));
  dbg_err_sif(obj->wt == NULL);

  *pobj = obj;

  return 0;

 err:

  if (obj) ffs_ensemble_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_ensemble_free
 *
 *****************************************************************************/

void ffs_ensemble_free(ffs_ensemble_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->wt) u_free(obj->wt);
  if (obj->traj) u_free(obj->traj);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_ensemble_sumwt
 *
 *****************************************************************************/

int ffs_ensemble_sumwt(ffs_ensemble_t * obj, double * sum) {

  int n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(sum == NULL, -1);

  *sum = 0.0;
  for (n = 0; n < obj->nsuccess; n++) {
    *sum += obj->wt[n];
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_ensemble_samplewt
 *
 *****************************************************************************/

int ffs_ensemble_samplewt(ffs_ensemble_t * obj, ranlcg_t * ran, int * irun) {

  double sumwt;
  double rs, rsum;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ran == NULL, -1);
  dbg_return_if(irun == NULL, -1);

  ffs_ensemble_sumwt(obj, &sumwt);
  ranlcg_reep(ran, &rs);
  rs = sumwt*rs;

  *irun = 0;
  rsum = obj->wt[0];

  while (rsum < rs) {
    *irun += 1;
    rsum += obj->wt[*irun];
  }

  return 0;
}
