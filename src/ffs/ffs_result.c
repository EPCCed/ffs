/*****************************************************************************
 *
 *  ffs_result.c
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "ffs_result.h"

struct ffs_result_s {

  /* Trials */

  int nlambda;       /* Number of interfaces (total) */
  double * wt;       /* Accumulated weight at interface */
  double * probs;    /* Probabilities at interfaces */
  double * swt;      /* Success weights for Rosenbluth */
  int * nsuccess;    /* Number of states at this interface recorded */
  int * nprune;      /* Number of trials pruned at this interface */
  int * nkeep;       /* Number of states actually retained each interface */
  int * nto;         /* Number of time outs 'at' each interface */
  int * nstart;      /* Number of trials started at interface */
  int * ndrop;       /* Number of trajectories dropped (Rosenbluth) */
  int * nback;       /* Number of trajectories going backward from interface */

};


/*****************************************************************************
 *
 *  ffs_result_create
 *
 *****************************************************************************/

int ffs_result_create(int nlambda, ffs_result_t ** pobj) {

  ffs_result_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);
  dbg_return_if(nlambda < 1, -1);

  obj = u_calloc(1, sizeof(ffs_result_t));
  dbg_err_sif(obj == NULL);

  obj->nlambda = nlambda;

  /* Interfaces are counted using natural numbers, so add 1 */

  obj->wt = u_calloc(nlambda + 1, sizeof(double));
  dbg_err_sif(obj->wt == NULL);

  obj->swt = u_calloc(nlambda + 1, sizeof(double));
  dbg_err_sif(obj->swt == NULL);

  obj->probs = u_calloc(nlambda + 1, sizeof(double));
  dbg_err_sif(obj->probs == NULL);

  obj->nsuccess = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->nsuccess == NULL);

  obj->nprune = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->nprune == NULL);

  obj->nkeep = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->nkeep == NULL);

  obj->nto = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->nto == NULL);

  obj->nstart = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->nstart == NULL);

  obj->ndrop = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->ndrop == NULL);

  obj->nback = u_calloc(nlambda + 1, sizeof(int));
  dbg_err_sif(obj->nback == NULL);

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

  if (obj->nback) u_free(obj->nback);
  if (obj->ndrop) u_free(obj->ndrop);
  if (obj->nstart) u_free(obj->nstart);
  if (obj->nto) u_free(obj->nto);
  if (obj->swt) u_free(obj->swt);
  if (obj->wt) u_free(obj->wt);
  if (obj->probs) u_free(obj->probs);
  if (obj->nsuccess) u_free(obj->nsuccess);
  if (obj->nprune) u_free(obj->nprune);
  if (obj->nkeep) u_free(obj->nkeep);

  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_result_weight
 *
 *****************************************************************************/

int ffs_result_weight(ffs_result_t * obj, int n, double * wt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(wt == NULL, -1);

  *wt = obj->wt[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_weight_accum
 *
 *****************************************************************************/

int ffs_result_weight_accum(ffs_result_t * obj, int n, double wt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->wt[n] += wt;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_reduce
 *
 *  We want:
 *
 *     wt     = sum of weights for each interface
 *     swt    = success wieghts for each interface (Rosenbluth)
 *     states = number of successful trials reaching this interface
 *
 *  Note that the results overwrite the existing numbers on root,
 *  so this is only to be called at the end of execution.
 *
 *****************************************************************************/

int ffs_result_reduce(ffs_result_t * obj, MPI_Comm comm) {

  int n;
  int mpi_errnol = 0;

  int * st_local = NULL;
  double * wt_local = NULL;

  dbg_return_if(obj == NULL, -1);

  /* Weight at each interface. */ 

  wt_local = u_calloc(obj->nlambda + 1, sizeof(double));
  mpi_errnol = (wt_local == NULL);
  mpi_sync_if_any(mpi_errnol, comm);

  for (n = 0; n <= obj->nlambda; n++) {
    wt_local[n] = obj->wt[n];
  }

  MPI_Allreduce(wt_local, obj->wt, obj->nlambda + 1, MPI_DOUBLE, MPI_SUM,
		comm);

  /* Success weight (use wt_local again) */

  for (n = 0; n <= obj->nlambda; n++) {
    wt_local[n] = obj->swt[n];
  }

  MPI_Allreduce(wt_local, obj->swt, obj->nlambda + 1, MPI_DOUBLE, MPI_SUM,
		comm);

  u_free(wt_local);

  /* States at each interface */

  st_local = u_calloc(obj->nlambda + 1, sizeof(int));
  mpi_errnol = (st_local == NULL);
  mpi_sync_if_any(mpi_errnol, comm);

  for (n = 0; n <= obj->nlambda; n++) {
    st_local[n] = obj->nsuccess[n];
  }

  MPI_Allreduce(st_local, obj->nsuccess, obj->nlambda + 1, MPI_INT, MPI_SUM,
		comm);

  /* Also use st_local for the number of pruning events */

  for (n = 0; n <= obj->nlambda; n++) {
    st_local[n] = obj->nprune[n];
  }

  MPI_Allreduce(st_local, obj->nprune, obj->nlambda + 1, MPI_INT, MPI_SUM,
		comm);

  /* Also use st_local for the number of time outs */

  for (n = 0; n <= obj->nlambda; n++) {
    st_local[n] = obj->nto[n];
  }

  MPI_Allreduce(st_local, obj->nto, obj->nlambda + 1, MPI_INT, MPI_SUM, comm);

  u_free(st_local);


  return 0;

 mpi_sync:
  if (wt_local) u_free(wt_local);
  if (st_local) u_free(st_local);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_result_trial_success_add
 *
 *****************************************************************************/

int ffs_result_trial_success_add(ffs_result_t * obj, int interface) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(interface < 0, -1);
  dbg_return_if(interface > obj->nlambda + 1, -1);

  obj->nsuccess[interface] += 1;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_trial_success
 *
 *****************************************************************************/

int ffs_result_trial_success(ffs_result_t * obj, int interface, int * ns) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(interface < 0, -1);
  dbg_return_if(interface > obj->nlambda, -1);

  *ns = obj->nsuccess[interface];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_prune_add
 *
 *****************************************************************************/

int ffs_result_prune_add(ffs_result_t * obj, int interface) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(interface < 0, -1);
  dbg_return_if(interface > obj->nlambda, -1);

  obj->nprune[interface] += 1;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_prune
 *
 *****************************************************************************/

int ffs_result_prune(ffs_result_t * obj, int interface, int * np) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(interface < 0, -1);
  dbg_return_if(interface > obj->nlambda, -1);

  *np = obj->nprune[interface];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nkeep_set
 *
 *****************************************************************************/

int ffs_result_nkeep_set(ffs_result_t * obj, int n, int nkeep) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->nkeep[n] = nkeep;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nkeep
 *
 *****************************************************************************/

int ffs_result_nkeep(ffs_result_t * obj, int n, int * nkeep) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(nkeep == NULL, -1);

  *nkeep = obj->nkeep[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nto_add
 *
 *****************************************************************************/

int ffs_result_nto_add(ffs_result_t * obj, int n, int nto) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->nto[n] += nto;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nto
 *
 *****************************************************************************/

int ffs_result_nto(ffs_result_t * obj, int n, int * nto) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(nto == NULL, -1);

  *nto = obj->nto[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_success_weight
 *
 *****************************************************************************/

int ffs_result_success_weight(ffs_result_t * obj, int n, double * swt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(swt == NULL, -1);

  *swt = obj->swt[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_success_weight_accum
 *
 *****************************************************************************/

int ffs_result_success_weight_accum(ffs_result_t * obj, int n, double swt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->swt[n] += swt;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nstart
 *
 *****************************************************************************/

int ffs_result_nstart(ffs_result_t * obj, int n, int * nstart) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(nstart == NULL, -1);

  *nstart = obj->nstart[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nstart_add
 *
 *****************************************************************************/

int ffs_result_nstart_add(ffs_result_t * obj, int n) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->nstart[n] += 1;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_ndrop
 *
 *****************************************************************************/

int ffs_result_ndrop(ffs_result_t * obj, int n, int * ndrop) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(ndrop == NULL, -1);

  *ndrop = obj->ndrop[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_ndrop_add
 *
 *****************************************************************************/

int ffs_result_ndrop_add(ffs_result_t * obj, int n) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->ndrop[n] += 1;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nback
 *
 *****************************************************************************/

int ffs_result_nback(ffs_result_t * obj, int n, int * nback) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);
  dbg_return_if(nback == NULL, -1);

  *nback = obj->nback[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_nback_add
 *
 *****************************************************************************/

int ffs_result_nback_add(ffs_result_t * obj, int n) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->nback[n] += 1;

  return 0;
}
