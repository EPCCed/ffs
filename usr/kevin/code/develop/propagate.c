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

#include "ffs_simulation.h"
#include "propagate.h"
#include "gillespie.h"

static int use_file_ = 1;
static state_t * strial_;

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

  strial_ = state_allocate();

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

  state_free(strial_);

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

int simulation_trial_state_lambda(ffs_param_t * ffs, double * lambda) {

  assert(ffs);
  assert(lambda);

  *lambda = state_to_lambda(strial_);

  return 0;
}

/*****************************************************************************
 *
 *  simulation_trial_state_time
 *
 *****************************************************************************/

double simulation_state_time(const sim_state_t * s) {

  return state_time(s);
}

int simulation_trial_state_time(ffs_param_t * ffs, double * t) {

  assert(ffs);
  assert(t);

  *t = state_time(strial_);

  return 0;
}

int simulation_trial_state_time_set(ffs_param_t * ffs, double t) {

  assert(ffs);

  state_time_set(strial_, t);

  return 0;
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

/*****************************************************************************
 *
 *  simulation_state_rng_set
 *
 *****************************************************************************/

int simulation_state_rng_set(const ffs_param_t * ffs, int seed) {

  int ifail = 0;
  long int lseed;

  /* Supply a long int rather than the int dummy argument */
  lseed = seed;

  gillespie_rng_state_set(lseed);

  return ifail;
}

/* E X P E R I M E N T   T O   F O L L O W */

int simulation_trial_state_initialise(ffs_param_t * ffs) {

  assert(strial_);
  gillespie_read_state("gillespie.components", strial_);

  return FFS_SUCCESS;
}

int simulation_trial_state_save(ffs_param_t * ffs, ffs_state_t * state) {

  int ifail;
  char filestub[FILENAME_MAX];
  char filename[FILENAME_MAX];
  state_t * p;

  if (use_file_) {
    /* Write strial_ to file */
    ffs_state_file_stub(state, filestub);
    /*printf("Writing to %s\n", filestub);*/
    sprintf(filename, "%s-s.dat", filestub);
    ifail = gillespie_write_state(filename, strial_);
  }
  else {
    /* Copy from strial to a new location, and remember where it is... */
    p = state_allocate();
    state_copy(strial_, p);
    ffs_state_memory_set(state, p);
  }

  return ifail;
}

int simulation_trial_state_set(ffs_param_t * ffs, ffs_state_t * state) {

  int ifail = FFS_SUCCESS;
  char filestub[FILENAME_MAX];
  char filename[FILENAME_MAX];
  state_t * p = NULL;

  assert(state);

  if (use_file_) {
    /* Read from file */
    ffs_state_file_stub(state, filestub);
    sprintf(filename, "%s-s.dat", filestub);
    gillespie_read_state(filename, strial_);
    /* COULD FAIL */
  }
  else {
    /* Copy from existing state */
    ffs_state_memory(state, (void *) &p);
    assert(p);
    state_copy(p, strial_);
  }

  return ifail;
}

int simulation_state_remove(ffs_param_t * ffs, ffs_state_t * state) {

  int ifail = FFS_SUCCESS;
  char filestub[FILENAME_MAX];
  char filename[FILENAME_MAX];
  state_t * p = NULL;

  if (use_file_) {
    ffs_state_file_stub(state, filestub);
    sprintf(filename, "%s-s.dat", filestub);
    if (remove(filename) != 0) ifail = FFS_FAILURE;
  }
  else {
    ffs_state_memory(state, (void *) &p);
    assert(p);
    state_free(p);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  simulation_trial_state_run
 *
 *****************************************************************************/

int simulation_trial_state_run(ffs_param_t * ffs, double ttarget,
			       double lambda_min, double lambda_max,
			       int type) {
  int n;
  int nstep = 0;
  int istatus;
  double lambda;

  if (type == FFS_RUN_SINGLE_STEP) {
    gillespie_do_step(strial_);
    return FFS_TRIAL_SUCCEEDED;
  }

  if (type == FFS_RUN_FORWARD_IN_TIME) {

    istatus = FFS_TRIAL_TIMED_OUT; /* If we fall out of the bottom of loop */

    for (n = 0; n <= ffs->nstepmax; n++) {

      gillespie_do_step(strial_);

      if (state_time(strial_) >= ttarget) {
	istatus = FFS_TRIAL_SUCCEEDED;
	break;
      }
    }

    return istatus;
  }

  /* This is FFS_RUN_FORWARD_IN_LAMBDA */

  istatus = FFS_TRIAL_IN_PROGRESS;

  while (istatus == FFS_TRIAL_IN_PROGRESS) {

    simulation_trial_state_lambda(ffs, &lambda);

    if (lambda <  lambda_min) istatus = FFS_TRIAL_WENT_BACKWARDS;
    if (lambda >= lambda_max) istatus = FFS_TRIAL_SUCCEEDED;
    if (nstep >= ffs->nstepmax) istatus = FFS_TRIAL_TIMED_OUT;
    if (istatus != FFS_TRIAL_IN_PROGRESS) break;

    for (n = 0; n < ffs->nsteplambda; n++) {
      gillespie_do_step(strial_);
      ++nstep;
      ++ffs->firesteps;
    }
  }

  return istatus;
}

int simulation_trial_run(ffs_param_t * ffs, ffs_trial_t * trial) {

  int ifail;
  int istatus;
  double time;
  double lambda;

  assert(use_file_ == 1);

  istatus = ffs_simulation_start(ffs, trial);

  while (istatus == FFS_TRIAL_IN_PROGRESS) {

    gillespie_do_step(strial_);

    if (ffs_simulation_lambda_required(ffs, trial)) {
      /* Order parameter is required. */
      simulation_trial_state_lambda(ffs, &lambda);
    }

    time = state_time(strial_);
    istatus = ffs_simulation_status(ffs, trial, time, lambda);
  }

  return ifail;
}

int simulation_random_trial_state(ffs_param_t * ffs) {

  int nmol;
  int nmax = 26;
  double lambda;
  double lambda_a;

  int ffs_param_lambda_a(ffs_param_t * p, double *);

  ffs_param_lambda_a(ffs, &lambda_a);

  do {
    nmol = ranlcg_reep_int32(ffs->random) % nmax;
    gillespie_nmol_set(strial_, 0, nmol); /* A */
    nmol = ranlcg_reep_int32(ffs->random) % nmax;
    gillespie_nmol_set(strial_, 1, nmol); /* B */
    nmol = ranlcg_reep_int32(ffs->random) % nmax;
    gillespie_nmol_set(strial_, 2, nmol); /* An */
    nmol = ranlcg_reep_int32(ffs->random) % nmax;
    gillespie_nmol_set(strial_, 3, nmol); /* Bn */

    gillespie_nmol_set(strial_, 4, 0); /* O */
    gillespie_nmol_set(strial_, 5, 0); /* OA */
    gillespie_nmol_set(strial_, 6, 0); /* OB */
    gillespie_nmol_set(strial_, 7, 0); /* OAB */

    /* Set one of O to 1 */
    nmol = ranlcg_reep_int32(ffs->random) % 4;
    gillespie_nmol_set(strial_, 4 + nmol, 1); /* OAB */

    lambda = state_to_lambda(strial_);

  } while (lambda >= lambda_a);

  return 0;
}
