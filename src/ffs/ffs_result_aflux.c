/*****************************************************************************
 *
 *  ffs_result_aflux.c
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "ffs_result_aflux.h"

struct ffs_result_aflux_s {

  /* Totals (per instance) */

  int ntrial;        /* Total trials to first interface */
  int nto;           /* Number of 'time-outs' before lambda_A */
  int neq;           /* Number of equilibration runs */
  int nsuccess;      /* nsuccess = ntrial - number of failures */
  int ncross;        /* Number of crossings */

  double tsum;       /* Total time of initialisation runs */
  double tmax;       /* Maximum time of initialisation run */

  /* Local quantities (per simulation proxy) */

  int ntrial_local;  /* Number of initial trials */
  int ncross_local;  /* Number of crossings of lambda_A */
  int neq_local;     /* Number of equiblration runs */

  int * status;      /* Final status of initialisation runs */
  double * t0;       /* Duration of initialisation runs (simulation units) */

};

/*****************************************************************************
 *
 *  ffs_result_aflux_create
 *
 *****************************************************************************/

int ffs_result_aflux_create(int ntrial, ffs_result_aflux_t ** pobj) {

  ffs_result_aflux_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);
  dbg_return_if(ntrial < 1, -1);

  obj = u_calloc(1, sizeof(ffs_result_aflux_t));
  dbg_err_sif(obj == NULL);

  obj->ntrial_local = ntrial;

  obj->status = u_calloc(ntrial, sizeof(int));
  obj->t0 = u_calloc(ntrial, sizeof(double));

  dbg_err_sif(obj->status == NULL);
  dbg_err_sif(obj->t0 == NULL);

  *pobj = obj;

  return 0;

 err:

  if (obj) ffs_result_aflux_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_free
 *
 *****************************************************************************/

void ffs_result_aflux_free(ffs_result_aflux_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->status) u_free(obj->status);
  if (obj->t0) u_free(obj->t0);

  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_ncross_add
 *
 *****************************************************************************/

int ffs_result_aflux_ncross_add(ffs_result_aflux_t * obj) {

  dbg_return_if(obj == NULL, -1);

  obj->ncross_local += 1;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_ncross_local
 *
 *****************************************************************************/

int ffs_result_aflux_ncross_local(ffs_result_aflux_t * obj, int * ncross) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ncross == NULL, -1);

  *ncross = obj->ncross_local;

  return 0;
}

/*****************************************************************************
 *
 * ffs_result_aflux_ncross_final
 *
 *****************************************************************************/

int ffs_result_aflux_ncross_final(ffs_result_aflux_t * obj, int * ncross) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ncross == NULL, -1);

  *ncross = obj->ncross;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_status_set
 *
 *****************************************************************************/

int ffs_result_aflux_status_set(ffs_result_aflux_t * obj, int n, int status) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n >= obj->ntrial_local, -1);

  obj->status[n] = status;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_time_set
 *
 *****************************************************************************/

int ffs_result_aflux_time_set(ffs_result_aflux_t * obj, int n, double t) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n >= obj->ntrial_local, -1);

  obj->t0[n] = t;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_reduce
 *
 *     nsuccess  number of trials reaching first interface 
 *     nto       number of trials timed out before first interface
 *     neq       number of equilibration runs
 *
 *     ncross = total number of initial crossings
 *     tsum   = sum of trajectory times
 *     tmax   = maximum trajectory time
 *
 *****************************************************************************/

int ffs_result_aflux_reduce(ffs_result_aflux_t * obj, MPI_Comm comm) {

  int n;
  int nsuccess = 0;
  int nto = 0;
  double tsum_local = 0.0;
  double tmax_local = 0.0;

  dbg_return_if(obj == NULL, -1);

  for (n = 0; n < obj->ntrial_local; n++) {
    tsum_local += obj->t0[n];
    if (obj->t0[n] > tmax_local) tmax_local = obj->t0[n];
    if (obj->status[n] == FFS_TRIAL_SUCCEEDED) nsuccess += 1;
    if (obj->status[n] == FFS_TRIAL_TIMED_OUT) nto += 1;
  }

  MPI_Allreduce(&nsuccess, &obj->nsuccess, 1, MPI_INT, MPI_SUM, comm);
  MPI_Allreduce(&nto, &obj->nto, 1, MPI_INT, MPI_SUM, comm);
  MPI_Allreduce(&tsum_local, &obj->tsum, 1, MPI_DOUBLE, MPI_SUM, comm);
  MPI_Allreduce(&tmax_local, &obj->tmax, 1, MPI_DOUBLE, MPI_MAX, comm);

  MPI_Allreduce(&obj->ntrial_local, &obj->ntrial, 1, MPI_INT, MPI_SUM, comm);
  MPI_Allreduce(&obj->neq_local, &obj->neq, 1, MPI_INT, MPI_SUM, comm);
  MPI_Allreduce(&obj->ncross_local, &obj->ncross, 1, MPI_INT, MPI_SUM, comm); 

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_tmax_final
 *
 *****************************************************************************/

int ffs_result_aflux_tmax_final(ffs_result_aflux_t * obj, double * tmax) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(tmax == NULL, -1);

  *tmax = obj->tmax;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_tsum_final
 *
 *****************************************************************************/

int ffs_result_aflux_tsum_final(ffs_result_aflux_t * obj, double * tsum) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(tsum == NULL, -1);

  *tsum = obj->tsum;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_status_final
 *
 *****************************************************************************/

int ffs_result_aflux_status_final(ffs_result_aflux_t * obj,
				  ffs_trial_enum_t key,
				  int * sum) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(sum == NULL, -1);

  switch (key) {
  case FFS_TRIAL_SUCCEEDED:
    *sum = obj->nsuccess;
    break;
  case FFS_TRIAL_TIMED_OUT:
    *sum = obj->nto;
    break;
  default:
    u_dbg("ffs_trial_enum_t not handled");
    *sum = -1;
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_neq_add
 *
 *****************************************************************************/

int ffs_result_aflux_neq_add(ffs_result_aflux_t * obj) {

  dbg_return_if(obj == NULL, -1);

  obj->neq_local += 1;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_neq_final
 *
 *****************************************************************************/

int ffs_result_aflux_neq_final(ffs_result_aflux_t * obj, int * n) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n == NULL, -1);

  *n = obj->neq;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_aflux_ntrial_final
 *
 *****************************************************************************/

int ffs_result_aflux_ntrial_final(ffs_result_aflux_t * obj, int * ntrial) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ntrial == NULL, -1);

  *ntrial = obj->ntrial;

  return 0;
}
