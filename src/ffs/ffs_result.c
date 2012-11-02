/*****************************************************************************
 *
 *  ffs_result.c
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "ffs_result.h"

struct ffs_result_s {

  /* Initialisation */

  int ntrial;        /* Number of initial states */
  int init_ncross;   /* Number of crossings of lambda_A */
  int * status;      /* Final status of initialisation runs */
  double * t0;       /* Duration of initialisation runs (simulation units) */
  double tsum;       /* Total time of initialisation runs */
  double tmax;       /* Maximum time initialisation run */
  double flux_a;     /* Flux at lambda_A */

  /* Trials */

  int nlambda;       /* Number of interfaces (total) */
  double * wt;       /* Accumulated weight at interface */
  double * probs;    /* Probabilities at interfaces */
};


/*****************************************************************************
 *
 *  ffs_result_create
 *
 *****************************************************************************/

int ffs_result_create(int nlambda, int ntrial, ffs_result_t ** pobj) {

  ffs_result_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);
  dbg_return_if(nlambda < 1, -1);
  dbg_return_if(ntrial < 1, -1);

  obj = u_calloc(1, sizeof(ffs_result_t));
  err_err_sif(obj == NULL);

  obj->status = u_calloc(ntrial, sizeof(int));
  err_err_sif(obj->status == NULL);

  obj->t0 = u_calloc(ntrial, sizeof(double));
  err_err_sif(obj->t0 == NULL);

  /* Interfaces are counted using natural numbers, so add 1 */

  obj->wt = u_calloc(nlambda + 1, sizeof(double));
  err_err_sif(obj->wt == NULL);

  obj->probs = u_calloc(nlambda + 1, sizeof(double));
  err_err_sif(obj->probs == NULL);

  *pobj = obj;

  return 0;

 err:

  if (obj) ffs_result_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_result_free
 *
 *****************************************************************************/

void ffs_result_free(ffs_result_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->status) u_free(obj->status);
  if (obj->t0) u_free(obj->t0);
  if (obj->wt) u_free(obj->wt);
  if (obj->probs) u_free(obj->probs);

  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_result_ncross_accum
 *
 *****************************************************************************/

int ffs_result_ncross_accum(ffs_result_t * obj, int iadd) {

  dbg_return_if(obj == NULL, -1);

  obj->init_ncross += iadd;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_ncross
 *
 *****************************************************************************/

int ffs_result_ncross(ffs_result_t * obj, int * ncross) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ncross == NULL, -1);

  *ncross = obj->init_ncross;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_status_set
 *
 *****************************************************************************/

int ffs_result_status_set(ffs_result_t * obj, int n, int status) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n >= obj->ntrial, -1);

  obj->status[n] = status;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_status
 *
 *****************************************************************************/

int ffs_result_status(ffs_result_t * obj, int n, int * status) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n >= obj->ntrial, -1);
  dbg_return_if(status == NULL, -1);

  *status = obj->status[n];

  return 0;
}
