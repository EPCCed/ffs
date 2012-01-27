/*****************************************************************************
 *
 *  branched.c
 *
 *  Routines for branched FFS algorithm.
 *
 *  CHANGE OF RESULTS: replaced recursive pruning by (correct) normal prune
 *
 *****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "ffs.h" 
#include "propagate.h"


void branched_recursive_trial(int i_sect, double wt, sim_state_t * startpoint,
			      ffs_param_t * ffs);
int branched_prune(sim_state_t * p, int isect, double *wt, ffs_param_t * ffs);
void output_data(double runtime, int n_starts, const ffs_param_t * ffs);
void branched_run_and_skip(ffs_param_t * ffs, const sim_state_t * s_eq,
			   sim_state_t * s_init);

/*****************************************************************************
 *
 *  branched_driver
 *
 *  Driver section for branched algorithm.
 *
 *****************************************************************************/

void branched_driver(ffs_param_t * ffs, int n_starts) {

  int i_start;
  double runtime;
  double wt;

  sim_state_t * s_init;
  sim_state_t * s_trial;

  simulation_set_up(ffs);

  s_init = simulation_state_initialise(ffs);
  /* INSERT s_init AS HEAD OF TREE AND RECORD STATE */
  /* tree_node = ffs_tree_node_create(id); */
  /* ffs_tree_node_set_state(tree_node, s_init); */


  /* CLONE HEAD FOR TRIAL ... */
  s_trial = simulation_state_allocate();
  simulation_state_copy(s_init, s_trial);
  simulation_run_to_time(s_trial, ffs->teq, ffs->nstepmax);
  simulation_state_time_set(s_trial, 0.0);

  ffs->ncross = 0;

  for (i_start = 0; i_start < n_starts; i_start++) {

    printf("initial point %d of %d sumw %f\n", i_start, n_starts,
	   ffs->interface[ffs->nlambda].weight);
    branched_run_and_skip(ffs, s_init, s_trial);

    /* now we have got an initial point */
    /* INSERT NODE IN TREE at first interface and record state. */

    wt = 1.0; /* weight */

    /* PASS NODE IN TREE RATHER THAN STATE */
    branched_recursive_trial(1, wt, s_trial, ffs);
  }

  runtime = simulation_state_time(s_trial);
  simulation_state_free(s_trial);

  simulation_state_finalise(s_init);
  simulation_tear_down();
 
  output_data(runtime, n_starts, ffs);

  return;
}

/*****************************************************************************
 *
 *  branched_run_and_skip
 *
 *  Run until we cross the first interface in the forward direction
 *  (skipping nskip crossings); if overshoot to the final state,
 *  re-equilibrate the initial conditions and try again.
 *
 *  The total time and number of crossings are recorded.
 *
 *****************************************************************************/

void branched_run_and_skip(ffs_param_t * ffs, const sim_state_t * s_init,
			   sim_state_t * s_trial) {
  int    stop;
  double lambda, old_lambda;
  double tmp;

  stop = 0;
  lambda = simulation_lambda(s_trial);

  while (stop == 0) {

    old_lambda = lambda;
    simulation_run_to_time(s_trial, DBL_MAX, 0); /* One step */
    lambda = simulation_lambda(s_trial);

    if (lambda >= ffs->lambda_2) {

      /* we are in B state - reset to s_eq and re-equilibrate */
      /* Need to remember current cummulative time before re-equilibration */

      tmp = simulation_state_time(s_trial);
      simulation_state_copy(s_init, s_trial);
      simulation_run_to_time(s_trial, ffs->teq, ffs->nstepmax);

      simulation_state_time_set(s_trial, tmp);
      lambda = simulation_lambda(s_trial);
      old_lambda = lambda;
    }

    ++ffs->runsteps;

    /* check if we cross the first interface which is the border of A */

    if ((lambda >= ffs->lambda_1) && (old_lambda < ffs->lambda_1)) {
      ++ffs->ncross;
      if ((ffs->ncross % ffs->nskip) == 0) stop = 1;
    }
  }

  return;
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


 /* KEVIN */
 printf("Current number of points allocated: %f\n", nalloc_current());


  return;
}


/*****************************************************************************
 *
 *  branched_recursive_trial
 *
 *****************************************************************************/

void branched_recursive_trial(int interface, double wt, sim_state_t * s_exist,
			      ffs_param_t * ffs) {
  int ikk;
  int ntrial;
  int res;
  double lambda_min;
  double lambda_max;
  double wtnow;

  sim_state_t * s_trial;

  ffs->interface[interface].weight += wt;

  /* If we have reached the final state the end the recursion */
  if (interface == ffs->nlambda) return;

  lambda_min = ffs->interface[interface - 1].lambda;
  lambda_max = ffs->interface[interface + 1].lambda;
  ntrial = ffs->interface[interface].ntrials;

  for (ikk = 0; ikk < ntrial; ikk++) {
  
    /* fire off the kk branches with total weight 1.0*incoming weight */
    wtnow = wt/((double) ntrial);

    /* CLONE EXISTING TREE NODE STATE FOR TRIAL */
    s_trial = simulation_state_allocate();
    simulation_state_copy(s_exist, s_trial);
    res = simulation_run_to_lambda(s_trial, lambda_min, lambda_max, ffs);
  
    if (res == FFS_TRIAL_WENT_BACKWARDS || res == FFS_TRIAL_TIMED_OUT) {
      res = branched_prune(s_trial, interface, &wtnow, ffs);
    }
    if (res == FFS_TRIAL_SUCCEEDED) {
      /* ADD NEW NODE IN TREE; RECORD STATE;
	 FINISH THIS TRIAL; RECURSE WITH NEW NODE */
      branched_recursive_trial(interface + 1, wtnow, s_trial, ffs);
    }

    simulation_state_free(s_trial);
  }

  /* If all trials are complete, remove state for this node? */

  return;
}

/*****************************************************************************
 *
 *  branched_prune
 *
 *****************************************************************************/

int branched_prune(sim_state_t * p, int interface, double *wt,
		   ffs_param_t * ffs) {
  int n;
  int istatus;
  double lambda_min, lambda_max;
 
  lambda_max = ffs->interface[interface + 1].lambda;
  istatus = FFS_TRIAL_WAS_PRUNED;

  for (n = interface; n > 2; n--) {

    if (ran3() < ffs->interface[n - 1].pprune) {
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
