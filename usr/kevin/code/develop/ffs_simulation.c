/*****************************************************************************
 *
 *  ffs_simulation.c
 *
 *  This is the base 'class' which will need to be 'extended' by a
 *  particular implementation.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>

#include "ffs_simulation.h"


static int ffs_simulation_to_time_status(ffs_param_t * ffs,
					 ffs_trial_t * trial, double time);

/*****************************************************************************
 *
 *  ffs_simulation_start
 *
 *****************************************************************************/

int ffs_simulation_start(ffs_param_t * ffs, ffs_trial_t * trial) {

  int istatus;

  ffs_trial_init(trial);
  istatus = FFS_TRIAL_IN_PROGRESS;

  return istatus;
}

/*****************************************************************************
 *
 *  ffs_simulation_lambda_required
 *
 *  Return true if we think lambda is required at this point;
 *  zero otherwise.
 *
 *****************************************************************************/

int ffs_simulation_lambda_required(ffs_param_t * ffs, ffs_trial_t * trial) {

  int nstep;
  int isrequired;

  nstep = ffs_trial_step(trial);
  isrequired = ((nstep % ffs->nsteplambda) == 0); 

  return isrequired;
}

/*****************************************************************************
 *
 *  ffs_simulation_status
 *
 *  The simulation provides the current time (always) and a lambda
 *  value which may or may not be up-to-date, depending on whether
 *  such a value has been required in the current step.
 *
 *  These values are used, together with the current trial state,
 *  to make the necessary updates.
 *
 *  The return code is from ffs_trial_result_enum.
 *
 *****************************************************************************/

int ffs_simulation_status(ffs_param_t * ffs, ffs_trial_t * trial,
			  double time, double lambda) {
  int istatus;
  int mytype;

  assert(ffs);
  assert(trial);

  mytype = ffs_trial_type(trial);

  switch (mytype) {
  case FFS_TRIAL_TO_TIME:
    istatus = ffs_simulation_to_time_status(ffs, trial, time);
    break;
  case FFS_TRIAL_TO_LAMBDA:
    /* istatus = ffs_simulation_to_lambda_status(ffs, trial, lambda);*/
    /* Can reach previous interface and be pruned PRUNED */
    /* Can reach previous interface and survive IN_PROGRESS */
    /* Can reach target SUCCEEDED*/
    /* Can be timed out TIMED_OUT*/
    /* Can fail FAILED */
    /* Can cross interface going forwards [add diagnostic] */
    /* Can cross interface going backwards [add diagnostic] */
    break;
  default:
    assert(0);
  }

  return istatus;
}

/*****************************************************************************
 *
 *  ffs_simulation_to_time_status
 *
 *****************************************************************************/

static int ffs_simulation_to_time_status(ffs_param_t * ffs,
					 ffs_trial_t * trial, double time) {

  int istatus; /* in progress unless we've hit a limit */
  int nstep;   /* current step */
  double t;    /* The elapsed time of this trial */

  istatus = FFS_TRIAL_IN_PROGRESS;
  nstep = ffs_trial_step(trial);
  t = time - ffs_trial_t0(trial);

  if (nstep > ffs->nstepmax) istatus = FFS_TRIAL_TIMED_OUT;
  if (t >= ffs_trial_target(trial)) istatus = FFS_TRIAL_SUCCEEDED;

  return istatus;
}
