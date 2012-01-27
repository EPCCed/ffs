/*****************************************************************************
 *
 *  ffs_general.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>

#include "ffs.h"
#include "propagate.h" /* REAL TIME PRUNING WILL GET RID OF THIS */
#include "ffs_general.h"

/*****************************************************************************
 *
 *  ffs_param_lambda_a
 *
 *****************************************************************************/

double ffs_param_lambda_a(const ffs_param_t * ffs) {

  assert(ffs);
  assert(ffs->interface);

  return ffs->interface[1].lambda;
}

/*****************************************************************************
 *
 *  ffs_param_lambda_b
 *
 *****************************************************************************/

double ffs_param_lambda_b(const ffs_param_t * ffs) {

  assert(ffs);
  assert(ffs->interface);

  return ffs->interface[ffs->nlambda].lambda;
}

/*****************************************************************************
 *
 *  ffs_general_prune
 *
 *  Note that pruning for trials originating at interfaces 1 and 2
 *  is 'automatic'.
 *
 *  TO BE REPLACED BY 'REAL-TIME' PRUNING
 *
 *****************************************************************************/

int ffs_general_prune(sim_state_t * p, int interface, double * wt,
		      ffs_param_t * ffs) {
  int n;
  int istatus;
  double lambda_min, lambda_max;

  lambda_max = ffs->interface[interface + 1].lambda;
  istatus = FFS_TRIAL_WAS_PRUNED;

  for (n = interface; n > 2; n--) {

    if (ranlcg_reep(ffs->random) < ffs->interface[n - 1].pprune) {
      istatus = FFS_TRIAL_WAS_PRUNED;
      break;
    }

    /* Trial survives, update weight and continue... */

    *wt *= 1.0/(1.0 - ffs->interface[n - 1].pprune);

    lambda_min = ffs->interface[n - 2].lambda;
    istatus = simulation_run_to_lambda(p, lambda_min, lambda_max, ffs);

    if (istatus == FFS_TRIAL_SUCCEEDED) break;
  }

  return istatus;
}
