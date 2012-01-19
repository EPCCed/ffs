/****************************************************************************
 *
 *  simulation.c
 *
 *  Implementation of abstract simulation layer for Gillespie code.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 ****************************************************************************/

#include <assert.h> 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ffs.h"
#include "propagate.h"
#include "gillespie.h"

/*****************************************************************************
 *
 *  simulation_state_allocate
 *
 *****************************************************************************/

sim_state_t * simulation_state_allocate(void) {

  sim_state_t * p;

  p = (sim_state_t *) state_allocate();
  assert(p);

  return p;
}

/*****************************************************************************
 *
 *  simulation_state_free
 *
 *****************************************************************************/

void simulation_state_free(sim_state_t * p) {

  assert(p);
  state_free(p);
  p = NULL;

  return;
}

/*****************************************************************************
 *
 *  simulation_state_copy
 *
 *****************************************************************************/

void simulation_state_copy(const sim_state_t * sr, sim_state_t * sw) {

  assert(sr);
  assert(sw);

  state_copy(sr, sw);

  return;
}

/*****************************************************************************
 *
 *  simulation_run_to_time
 *
 *****************************************************************************/

int simulation_run_to_time(sim_state_t * p, double ttarget, int nstepmax) {

  int n;
  int istatus;

  istatus = FFS_TRIAL_TIMED_OUT; /* If we fall out of the bottom of loop */

  for (n = 0; n <= nstepmax; n++) {

    gillespie_do_step(p);

    if (state_time(p) >= ttarget) {
      istatus = FFS_TRIAL_SUCCEEDED;
      break;
    }
  }

  return istatus;
}

/*****************************************************************************
 *
 *  simulation_set_up
 *
 *****************************************************************************/

int simulation_set_up(ffs_param_t * ffs) {

  int ifail;

  ifail = gillespie_set_up("gillespie.inp");

  if (ifail == 0) {
    ifail = FFS_SUCCESS;
  }
  else {
    ifail = FFS_FAILURE;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  simulation_tear_down
 *
 *****************************************************************************/

int simulation_tear_down(void) {

  int ifail;

  ifail = gillespie_tear_down();

  if (ifail == 0) {
    ifail = FFS_SUCCESS;
  }
  else {
    ifail = FFS_FAILURE;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  simulation_state_initialise
 *
 *****************************************************************************/

sim_state_t * simulation_state_initialise(ffs_param_t * ffs) {

  sim_state_t * p;

  p = state_allocate();
  gillespie_read_state("gillespie.components", p);

  return p;
}

/*****************************************************************************
 *
 *  simulation_state_finalise
 *
 *****************************************************************************/

void simulation_state_finalise(sim_state_t * p) {

  assert(p);
  state_free(p);
  p = NULL;

  return;
}

/*****************************************************************************
 *
 *  simulation_run_to_lambda
 *
 *  Run until reach backward limit lambda_min, or forward target
 *  lambda_max.
 *
 *****************************************************************************/

int simulation_run_to_lambda(sim_state_t * p, double lambda_min,
			     double lambda_max, ffs_param_t * ffs) {
  int n;
  int nstep = 0;
  int istatus = FFS_TRIAL_IN_PROGRESS;

  double lambda;

  assert(p);

  while (istatus == FFS_TRIAL_IN_PROGRESS) {

    lambda = state_to_lambda(p);

    /* DIAGNOSTICS TO BE REINSTATED? */

    if (lambda <  lambda_min) istatus = FFS_TRIAL_WENT_BACKWARDS;
    if (lambda >= lambda_max) istatus = FFS_TRIAL_SUCCEEDED;
    if (nstep >= ffs->nstepmax) istatus = FFS_TRIAL_TIMED_OUT;
    if (istatus != FFS_TRIAL_IN_PROGRESS) break;

    for (n = 0; n < ffs->nsteplambda; n++) {
      gillespie_do_step(p);
      ++nstep;
      ++ffs->firesteps;
    }
  }

  return istatus;
}

/*****************************************************************************
 *
 *  simulation_lambda
 *
 *****************************************************************************/

double simulation_lambda(const sim_state_t * p) {

  double lambda;

  assert(p);
  lambda = state_to_lambda(p);

  return lambda;
}

/*****************************************************************************
 *
 *  simulation_state_time
 *
 *****************************************************************************/

double simulation_state_time(const sim_state_t * p) {

  assert(p);
  return state_time(p);
}

/*****************************************************************************
 *
 *  simulation_state_time_set
 *
 *****************************************************************************/

void simulation_state_time_set(sim_state_t * p, double t) {

  assert(p);
  state_time_set(p, t);

  return;
}
