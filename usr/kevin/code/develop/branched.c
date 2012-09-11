/*****************************************************************************
 *
 *  branched.c
 *
 *  Routines for branched FFS algorithm.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *  CHANGE OF RESULTS: replaced recursive pruning by (correct) normal prune
 *  CHANGE OF RESULTS: replaced ran3 in pruning
 *  CHANGE OR RESULTS: replaced ran3 all together
 *
 *****************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "ffs.h" 
#include "propagate.h"
#include "ffs_general.h"
#include "branched.h"


void output_data(double runtime, int n_starts, const ffs_param_t * ffs);
int branched_recursive_trial(int interface, int id, double wt,
			     ffs_param_t * ffs);
int ffs_initial_crossing(ffs_param_t * ffs, ffs_state_t * eq, int seed,
			 double * t0, int * status);
int ffs_initial_histogram(ffs_param_t * ffs, double * t, int * status);

/*****************************************************************************
 *
 *  branched_driver
 *
 *  Driver section for branched algorithm.
 *
 *****************************************************************************/

int branched_driver(ffs_param_t * ffs) {

  int n, nstart;
  double wt;

  int * status;    /* status for each initial trajectory */
  double * t0;     /* time for each initial trajectory */

  ffs_state_t * sinit = NULL;

  simulation_set_up(ffs);
  nstart = ffs->interface[1].nstates;

  ffs_state_create(&sinit);

  simulation_trial_state_initialise(ffs);
  simulation_trial_state_save(ffs, sinit);

  status = calloc(nstart, sizeof(int));
  t0 = calloc(nstart, sizeof(double));

  for (n = 0; n < nstart; n++) {

    ffs_initial_crossing(ffs, sinit, 1 + n, t0 + n, status + n);

    wt = 1.0; /* weight */
    branched_recursive_trial(1, 1, wt, ffs);
  }

  simulation_state_remove(ffs, sinit);
  simulation_tear_down();
  ffs_state_remove(sinit);

  ffs_initial_histogram(ffs, t0, status);
  free(t0);
  free(status);

  return 0;
}

/*****************************************************************************
 *
 *  ffs_initial_crossing
 *
 *  Take an equilibrium state A, and run until we get a crossing
 *  of the first interface, or time out.
 *
 *  The time taken to reach the interface for an ensemble of
 *  trajectories may be used to determine the flux across the
 *  first interface.
 *
 *****************************************************************************/

int ffs_initial_crossing(ffs_param_t * ffs, ffs_state_t * eq, int seed,
			 double * thist, int * status) {

  int istatus;
  double t;
  double lambda, lambda_old;
  double lambda_a, lambda_b;

  assert(ffs);
  assert(eq);
  assert(thist);
  assert(status);

  ffs_param_lambda_a(ffs, &lambda_a);
  ffs_param_lambda_b(ffs, &lambda_b);

  if (seed == 1 || ffs->init_independent) {
    /* Equilibrate */
    if (ffs->init_independent) simulation_state_rng_set(ffs, seed);
    simulation_trial_state_set(ffs, eq);
    istatus = simulation_trial_state_run(ffs, ffs->teq, 0.0, 0.0,
					 FFS_RUN_FORWARD_IN_TIME);
    assert(istatus == FFS_TRIAL_SUCCEEDED);
  }

  simulation_trial_state_lambda(ffs, &lambda_old);
  simulation_trial_state_time_set(ffs, 0.0);

  while (1) {

    istatus = simulation_trial_state_run(ffs, DBL_MAX, 0.0, 0.0,
					 FFS_RUN_SINGLE_STEP);
    simulation_trial_state_lambda(ffs, &lambda);

    if (lambda >= lambda_b) {
      simulation_trial_state_time(ffs, &t);
      simulation_trial_state_set(ffs, eq);
      istatus = simulation_trial_state_run(ffs, ffs->teq, 0.0, 0.0,
					   FFS_RUN_FORWARD_IN_TIME);
      assert(istatus == FFS_TRIAL_SUCCEEDED);
      simulation_trial_state_time_set(ffs, t);
      simulation_trial_state_lambda(ffs, &lambda_old);
      lambda = lambda_old;
    }

    if ((lambda_old < lambda_a) && (lambda >= lambda_a)) {
      ffs->ncross += 1;
      *status = istatus;
      simulation_trial_state_time(ffs, thist);

      /* This mess is just for backward compatability, where
       * we cannot afford an extra random number */

      if ((ffs->ncross % ffs->nskip) == 0) {
	if (ffs->init_independent) {
	  if (ranlcg_reep(ffs->random) < ffs->init_paccept) {
	    break;
	  }
	}
	else {
	  break;
	}
      }

    }

    lambda_old = lambda;
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_initial_histrogram
 *
 *  Output the histogram of trajectory times to the first interface,
 *  and various other information.
 *
 *****************************************************************************/

int ffs_initial_histogram(ffs_param_t * ffs, double * t, int * status) {

  int idt, n;
  int nstart;
  int nstop = 0;
  int nbin = 40;

  int * tbin = NULL; 
  double t0;
  double tmax = 0.0;
  double tsum = 0.0;

  assert(ffs);
  assert(t);
  assert(status);

  nstart = ffs->interface[1].nstates;

  for (n = 0; n < nstart; n++) {
    if (status[n] != FFS_TRIAL_SUCCEEDED) nstop += 1;
    tsum += t[n];
    if (t[n] > tmax) tmax = t[n];
  }

  printf("Number of trials %d\n", nstart);
  printf("Number of stops  %d\n", nstop);
  printf("Tmax =           %f\n", tmax);
  printf("Tsum =           %f\n", tsum);
  printf("Crosses =        %d\n", ffs->ncross);
  printf("Flux ~           %f\n", ffs->ncross/tsum);
  printf("Probability AB   %11.4e\n",
	 ffs->interface[ffs->nlambda].weight/nstart);

  tbin = calloc(nbin, sizeof(int));
  assert(tbin);

  for (n = 0; n < nstart; n++) {
    idt = (t[n]/tmax)*nbin;
    if (idt < nbin) tbin[idt] += 1;
  }

  /* Histogram, and cumulative total idt. */

  idt = 0;

  for (n = 0; n < nbin; n++) {
    t0 = (n + 0.5)*tmax/nbin;
    idt += tbin[n];
    printf("%3d %11.4e %8d %11.4e\n", n+1, t0, tbin[n], 1.0*idt/nstart);
  }

  free(tbin);

  /* Interface probabilities */

  printf("\n");
  printf("Interface probabilities\n");

 for (n = 1; n <= ffs->nlambda; n++) {
   printf("%3d %11.4e %11.4e\n", n, ffs->interface[n].lambda,
	   ffs->interface[n].weight/nstart);
 }

 printf("\n");
 printf("Overall probability [flux * P(B|A)] = %11.4e\n",
	(ffs->ncross/tsum)*ffs->interface[ffs->nlambda].weight/nstart);

  return 0;
}

/*****************************************************************************
 *
 *  output_data
 *
 *****************************************************************************/

void output_data(double runtime, int n_starts, const ffs_param_t * ffs) {

 FILE *fp;
 int i;

 fp = fopen("Results.dat","w");
 
 fprintf(fp, "the run contained %d steps\n", ffs->runsteps);

 fprintf(fp,"the run time was %lf, the number of crossings was %d, so the flux through the first interface was %20.10lf\n", runtime, ffs->ncross, ((double) ffs->ncross)/runtime);


 fprintf(fp,"the firing took %d steps\n", ffs->firesteps);

 fprintf(fp,"the total number of steps was %d\n",
	 ffs->runsteps + ffs->firesteps);

 fprintf(fp,"The probability of reaching B from lambda_1 is %lf\n",
	 ffs->interface[ffs->nlambda].weight/n_starts);

 for (i = 1; i <= ffs->nlambda; i++) {
   fprintf(fp," %lf %lf\n", ffs->interface[i].lambda,
	   ffs->interface[i].weight/n_starts);
 }

 fclose(fp);

  return;
}


/*****************************************************************************
 *
 *  branched_recursive_trial
 *
 *****************************************************************************/

int branched_recursive_trial(int interface, int id, double wt,
			     ffs_param_t * ffs) {
  int ikk;
  int ntrial;
  int res;
  double lambda_min;
  double lambda_max;
  double wtnow;

  ffs_state_t * s_keep = NULL;
  int tmp_prune(int interface, double * wt, ffs_param_t * ffs);

  ffs->interface[interface].weight += wt;

  /* If we have reached the final state the end the recursion */
  if (interface == ffs->nlambda) return 0;

  lambda_min = ffs->interface[interface - 1].lambda;
  lambda_max = ffs->interface[interface + 1].lambda;
  ntrial = ffs->interface[interface].ntrials;

  /* Save this state, as it needs to be restored. */
  ffs_state_create(&s_keep);
  ffs_state_id_set(s_keep, id);
  simulation_trial_state_save(ffs, s_keep);

  for (ikk = 0; ikk < ntrial; ikk++) {
  
    /* fire off the kk branches with total weight 1.0*incoming weight */
    wtnow = wt/((double) ntrial);

    res = simulation_trial_state_run(ffs, 0.0, lambda_min, lambda_max,
				     FFS_RUN_FORWARD_IN_LAMBDA);
  
    if (res == FFS_TRIAL_WENT_BACKWARDS || res == FFS_TRIAL_TIMED_OUT) {
      res = tmp_prune(interface, &wtnow, ffs);
    }
    if (res == FFS_TRIAL_SUCCEEDED) {
      /* ADD NEW NODE IN TREE; RECORD STATE;
	 FINISH THIS TRIAL; RECURSE WITH NEW NODE */
      branched_recursive_trial(interface + 1, ++id, wtnow, ffs);
    }

    /* Reset, and next trial, or end */
    simulation_trial_state_set(ffs, s_keep);
  }

  simulation_state_remove(ffs, s_keep);
  ffs_state_remove(s_keep);

  return 0;
}

/* This should replace the pruning routine in ffs_general, if and
 * when the direct code is brought into line. */

int tmp_prune(int interface, double * wt, ffs_param_t * ffs) {

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
    istatus = simulation_trial_state_run(ffs, 0.0, lambda_min, lambda_max,
					 FFS_RUN_FORWARD_IN_LAMBDA);

    if (istatus == FFS_TRIAL_SUCCEEDED) break;
  }

  return istatus;
}
