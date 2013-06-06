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
  int * nsuccess;    /* Number of states at this interface recorded */
  int * nprune;      /* Number of trials pruned at this interface */
  int * nkeep;       /* Number of states actually retained each interface */
  int * nto;         /* Number of time outs 'at' each interface */

  int init_nsuccess; /* Number of successful trials reaching lambda1 */
  int init_nto;      /* Time-outs before lambda1 */
  int init_eq;       /* Number of equilibration runs */
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
  dbg_err_sif(obj == NULL);

  obj->ntrial = ntrial;
  obj->nlambda = nlambda;

  obj->status = u_calloc(ntrial, sizeof(int));
  dbg_err_sif(obj->status == NULL);

  obj->t0 = u_calloc(ntrial, sizeof(double));
  dbg_err_sif(obj->t0 == NULL);

  /* Interfaces are counted using natural numbers, so add 1 */

  obj->wt = u_calloc(nlambda + 1, sizeof(double));
  dbg_err_sif(obj->wt == NULL);

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

  if (obj->nto) u_free(obj->nto);
  if (obj->status) u_free(obj->status);
  if (obj->t0) u_free(obj->t0);
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
 *  ffs_result_time_set
 *
 *****************************************************************************/

int ffs_result_time_set(ffs_result_t * obj, int n, double t) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n >= obj->ntrial, -1);

  obj->t0[n] = t;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_time
 *
 *****************************************************************************/

int ffs_result_time(ffs_result_t * obj, int n, double * t) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n >= obj->ntrial, -1);
  dbg_return_if(t == NULL, -1);

  *t = obj->t0[n];

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_reduce
 *
 *  We want:
 *
 *     init_nsuccess  number of trials reaching first interface 
 *     init_nto       number of trials timed out before first interface
 *     init_eq        number of equilibration runs
 *
 *     ncross = total number of initial crossings
 *     tsum   = sum of trajectory times
 *     tmax   = maximum trajectory time
 *
 *     wt     = sum of weights for each interface
 *     states = number of successful trials reaching this interface
 *
 *  Note that the results overwrite the existing numbers on root,
 *  so this is only to be called at the end of execution.
 *
 *****************************************************************************/

int ffs_result_reduce(ffs_result_t * obj, MPI_Comm comm, int root) {

  int n;
  int ncross_local;
  int init_nsuccess_local = 0;
  int init_nto = 0;
  int init_eq_local = 0;
  int mpi_errnol = 0;

  double tsum_local = 0.0;
  double tmax_local = 0.0;
  double * wt_local = NULL;
  int * st_local = NULL;

  dbg_return_if(obj == NULL, -1);

  ncross_local = obj->init_ncross;
  init_eq_local = obj->init_eq;

  for (n = 0; n < obj->ntrial; n++) {
    tsum_local += obj->t0[n];
    if (obj->t0[n] > tmax_local) tmax_local = obj->t0[n];
    if (obj->status[n] == FFS_TRIAL_SUCCEEDED) init_nsuccess_local += 1;
    if (obj->status[n] == FFS_TRIAL_TIMED_OUT) init_nto += 1;
  }

  MPI_Reduce(&init_nsuccess_local, &obj->init_nsuccess, 1, MPI_INT, MPI_SUM,
	     root, comm);
  MPI_Reduce(&init_nto, &obj->init_nto, 1, MPI_INT, MPI_SUM, root, comm);
  MPI_Reduce(&init_eq_local, &obj->init_eq, 1, MPI_INT, MPI_SUM, root, comm);
  MPI_Reduce(&ncross_local, &obj->init_ncross, 1, MPI_INT, MPI_SUM, root, comm);
  MPI_Reduce(&tsum_local, &obj->tsum, 1, MPI_DOUBLE, MPI_SUM, root, comm);
  MPI_Reduce(&tmax_local, &obj->tmax, 1, MPI_DOUBLE, MPI_MAX, root, comm);

  /* Weight at each interface. */ 

  wt_local = u_calloc(obj->nlambda + 1, sizeof(double));
  mpi_errnol = (wt_local == NULL);
  mpi_sync_if_any(mpi_errnol, comm);

  for (n = 0; n <= obj->nlambda; n++) {
    wt_local[n] = obj->wt[n];
  }

  MPI_Reduce(wt_local, obj->wt, obj->nlambda + 1, MPI_DOUBLE, MPI_SUM, root,
	     comm);

  u_free(wt_local);

  /* States at each interface */

  st_local = u_calloc(obj->nlambda + 1, sizeof(int));
  mpi_errnol = (st_local == NULL);
  mpi_sync_if_any(mpi_errnol, comm);

  for (n = 0; n <= obj->nlambda; n++) {
    st_local[n] = obj->nsuccess[n];
  }

  MPI_Reduce(st_local, obj->nsuccess, obj->nlambda + 1, MPI_INT, MPI_SUM,
	     root, comm);

  /* Also use st_local for the number of pruning events */

  for (n = 0; n <= obj->nlambda; n++) {
    st_local[n] = obj->nprune[n];
  }

  MPI_Reduce(st_local, obj->nprune, obj->nlambda + 1, MPI_INT, MPI_SUM, root,
	     comm);

  /* Also use st_local for the number of time outs */

  for (n = 0; n <= obj->nlambda; n++) {
    st_local[n] = obj->nto[n];
  }

  MPI_Reduce(st_local, obj->nto, obj->nlambda + 1, MPI_INT, MPI_SUM, root,
	     comm);

  u_free(st_local);


  return 0;

 mpi_sync:
  if (wt_local) u_free(wt_local);
  if (st_local) u_free(st_local);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_result_tmax
 *
 *****************************************************************************/

int ffs_result_tmax(ffs_result_t * obj, double * tmax) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(tmax == NULL, -1);

  *tmax = obj->tmax;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_tsum
 *
 *****************************************************************************/

int ffs_result_tsum(ffs_result_t * obj, double * tsum) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(tsum == NULL, -1);

  *tsum = obj->tsum;

  return 0;
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
 *  ffs_result_success_final
 *
 *****************************************************************************/

int ffs_result_status_final(ffs_result_t * obj, ffs_trial_enum_t key,
			    int * sum) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(sum == NULL, -1);

  switch (key) {
  case FFS_TRIAL_SUCCEEDED:
    *sum = obj->init_nsuccess;
    break;
  case FFS_TRIAL_TIMED_OUT:
    *sum = obj->init_nto;
    break;
  default:
    u_dbg("ffs_trial_enum_t not handled");
    *sum = -1;
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_eq_accum
 *
 *****************************************************************************/

int ffs_result_eq_accum(ffs_result_t * obj, int add) {

  dbg_return_if(obj == NULL, -1);

  obj->init_eq += add;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_eq_final
 *
 *****************************************************************************/

int ffs_result_eq_final(ffs_result_t * obj, int * n) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n == NULL, -1);

  *n = obj->init_eq;

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
