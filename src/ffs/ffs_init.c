/*****************************************************************************
 *
 *  ffs_init.c
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "ffs_init.h"

struct ffs_init_s {
  int independent;
  int nstepmax;
  int nskip;
  int nsteplambda;
  int ntrials;
  double prob_accept;
  double teq;
};

/*****************************************************************************
 *
 *  ffs_init_create
 *
 *****************************************************************************/

int ffs_init_create(ffs_init_t ** pobj) {

  ffs_init_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = u_calloc(1, sizeof(ffs_init_t))) == NULL);

  *pobj = obj;

  return 0;
 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_init_free
 *
 *****************************************************************************/

void ffs_init_free(ffs_init_t * obj) {

  dbg_return_if(obj == NULL, );

  u_free(obj);
  obj = NULL;

  return;
}

/*****************************************************************************
 *
 *  ffs_init_nstepmax
 *
 *****************************************************************************/

int ffs_init_nstepmax(ffs_init_t * obj, int * nstepmax) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nstepmax == NULL, -1);

  *nstepmax = obj->nstepmax;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_nstepmax_set
 *
 *****************************************************************************/

int ffs_init_nstepmax_set(ffs_init_t * obj, int nstepmax) {

  dbg_return_if(obj == NULL, -1);

  obj->nstepmax = nstepmax;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_independent
 *
 *****************************************************************************/

int ffs_init_independent(ffs_init_t * obj, int * flag) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(flag == NULL, -1);

  *flag = obj->independent;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_independent_set
 *
 *****************************************************************************/

int ffs_init_independent_set(ffs_init_t * obj, int flag) {

  dbg_return_if(obj == NULL, -1);

  obj->independent = flag;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_nskip
 *
 *****************************************************************************/

int ffs_init_nskip(ffs_init_t * obj, int * nskip) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nskip == NULL, -1);

  *nskip = obj->nskip;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_nskip_set
 *
 *****************************************************************************/

int ffs_init_nskip_set(ffs_init_t * obj, int nskip) {

  dbg_return_if(obj == NULL, -1);

  obj->nskip = nskip;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_prob_accept
 *
 *****************************************************************************/

int ffs_init_prob_accept(ffs_init_t * obj, double * prob) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(prob == NULL, -1);

  *prob = obj->prob_accept;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_prob_accept_set
 *
 *****************************************************************************/

int ffs_init_prob_accept_set(ffs_init_t * obj, double prob) {

  dbg_return_if(obj == NULL, -1);
  err_return_if(prob < 0.0, -1);
  err_return_if(prob > 1.0, -1);

  obj->prob_accept = prob;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_teq
 *
 *****************************************************************************/

int ffs_init_teq(ffs_init_t * obj, double * teq) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(teq == NULL, -1);

  *teq = obj->teq;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_teq_set
 *
 *  There is no constraint on the value supplied.
 *
 *****************************************************************************/

int ffs_init_teq_set(ffs_init_t * obj, double teq) {

  dbg_return_if(obj == NULL, -1);

  obj->teq = teq;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_nsteplambda
 *
 *****************************************************************************/

int ffs_init_nsteplambda(ffs_init_t * obj, int * nsteplambda) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nsteplambda == NULL, -1);

  *nsteplambda = obj->nsteplambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_nsteplambda_set
 *
 *****************************************************************************/

int ffs_init_nsteplambda_set(ffs_init_t * obj, int nsteplambda) {

  dbg_return_if(obj == NULL, -1);

  obj->nsteplambda = nsteplambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_ntrials
 *
 *****************************************************************************/

int ffs_init_ntrials(ffs_init_t * obj, int * nt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nt == NULL, -1);

  *nt = obj->ntrials;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_ntrials_set
 *
 *****************************************************************************/

int ffs_init_ntrials_set(ffs_init_t * obj, int nt) {

  dbg_return_if(obj == NULL, -1);

  obj->ntrials = nt;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_init_log_to_mpilog
 *
 *****************************************************************************/

int ffs_init_log_to_mpilog(ffs_init_t * obj, mpilog_t * log) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(log == NULL, -1);

  mpilog(log, "\n");
  mpilog(log, "Parameters for initial state generation\n");
  mpilog(log, "No. trials:    %d\n", obj->ntrials);
  mpilog(log, "independent:   %d\n", obj->independent);
  mpilog(log, "nstepmax:      %d\n", obj->nstepmax);
  mpilog(log, "nskip:         %d\n", obj->nskip);
  mpilog(log, "nsteplambda:   %d\n", obj->nsteplambda);
  mpilog(log, "prob. accept:  %f\n", obj->prob_accept);
  mpilog(log, "t_equilib:     %f\n", obj->teq);

  return 0;
}
