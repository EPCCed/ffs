
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "ffs.h" 
#include "propagate.h"


void branched_recursive_trial(int i_sect, double wt, sim_state_t * startpoint,
			      int imax, double * sumw, ffs_param_t * ffs);
void branched_recursive_prune(int i_sect, double wt, sim_state_t * startpoint,
			      double lambda, int imax, double * sumw,
			      ffs_param_t * ffs);
void output_data(double runtime, double p_b, int n_starts,
		 const ffs_param_t * ffs);
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

  int i_start, i_sect;
  int imax; 
  double runtime;
  double p_b;
  double wt;
  double sumw;

  sim_state_t * s_eq;
  sim_state_t * s_init;

  sumw = 0.0;

  simulation_set_up(ffs);
  s_init = simulation_state_initialise(ffs);

  /* "s_eq" is retained as the initial conditions */
  s_eq = simulation_state_allocate();
  simulation_state_copy(s_init, s_eq);

  /* Generate the "initial" state t = 0. */
  simulation_run_to_time(s_init, ffs->teq, ffs->nstepmax);
  simulation_state_time_set(s_init, 0.0);

  ffs->ncross = 0;

  for (i_start = 0; i_start < n_starts; i_start++) {

    printf("initial point %d of %d sumw %f\n", i_start, n_starts, sumw);
    branched_run_and_skip(ffs, s_eq, s_init);

    /* now we have got an initial point */

    wt = 1.0; /* weight */    
    i_sect = 0;
    imax = 0;

    branched_recursive_trial(i_sect, wt, s_init, imax, &sumw, ffs);
  }

  runtime = simulation_state_time(s_init);

  /* this is the average probability of reaching B from lambda_1 */
  p_b = sumw/((double) n_starts);

  simulation_state_free(s_eq);
  simulation_state_finalise(s_init);
  simulation_tear_down();
 
  output_data(runtime, p_b, n_starts, ffs);

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

void branched_run_and_skip(ffs_param_t * ffs, const sim_state_t * s_eq,
			   sim_state_t * sp) {
  int    stop;
  double lambda, old_lambda;
  double tmp;

  stop = 0;
  lambda = simulation_lambda(sp);

  while (stop == 0) {

    old_lambda = lambda;
    simulation_run_to_time(sp, DBL_MAX, 0); /* One step */
    lambda = simulation_lambda(sp);

    if (lambda >= ffs->lambda_2) {

      /* we are in B state - reset to s_eq and re-equilibrate */
      /* Need to remember current cummulative time before re-equilibration */

      tmp = simulation_state_time(sp);
      simulation_state_copy(s_eq, sp);
      simulation_run_to_time(sp, ffs->teq, ffs->nstepmax);

      simulation_state_time_set(sp, tmp);
      lambda = simulation_lambda(sp);
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

void output_data(double runtime, double p_b, int n_starts,
		 const ffs_param_t * ffs) {

 FILE *fp;
 int i, j;

 fp = fopen("Results.dat","w");
 
 fprintf(fp, "the run contained %d steps\n", ffs->runsteps);

 fprintf(fp,"the run time was %lf, the number of crossings was %d, so the flux through the first interface was %20.10lf\n", runtime, ffs->ncross, ((double) ffs->ncross)/runtime);


 fprintf(fp,"the firing took %d steps\n", ffs->firesteps);

 fprintf(fp,"the total number of steps was %d\n",
	 ffs->runsteps + ffs->firesteps);


 fprintf(fp,"The probability of reaching B from lambda_1 is %lf\n", p_b);
 fclose(fp);

 fp = fopen( "p_lambda_TEST.dat", "w" );

 for (i = 1; i < ffs->nsections; i++){
   fprintf(fp," %lf  %lf\n", ffs->section[i].lambda_min,
	   ffs->section[i].sumwt/((double) n_starts));
 }

 fprintf(fp,"\n\n\n");

 for (j = 0; j < ffs->nsections; j++) {
   for (i = 0; i < ffs->section[j].Nbins; i++) {
     fprintf(fp, "%lf %lf\n ",
	     ffs->lambda_1 + (i+0.5)*ffs->section[j].d_lambda,
	     ffs->section[j].pl_histo[i]/((double) n_starts));
   }
 } 

 fclose(fp);

 /* print the P(lambda) histograms to a file */

 fp = fopen( "p_lambda.dat", "w" );

 /* fprintf(fp,"%d\t%d\t%d\t output.dat\n ", ffs->nbins, 20, 1);*/

 for (j = 0; j < ffs->nsections; j++) {
   for (i = 0; i < ffs->section[j].Nbins; i++) {
     fprintf(fp, "%lf %lf %2.1f %d\n ",
	     ffs->lambda_1 + (i+0.5)*ffs->section[j].d_lambda,
	     ffs->section[j].pl_histo[i]/((double) n_starts), 1.0, 1);
   }
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

void branched_recursive_trial(int i_sect, double wt, sim_state_t * startpoint,
			      int imax, double * sumw, ffs_param_t * ffs) {
  int kk,ikk;
  double lambda_min;
  double lambda_max;
  double wtnow;
  int res;
  int imaxxx;

  sim_state_t * sp;

  if (i_sect > imax) {
    /* this is to ensure that the path is reaching this interface
     * for the first time */
    ffs->section[i_sect].sumwt += wt;
    imaxxx = i_sect;
  }
  else {
    imaxxx = imax;
  }

  /* here we grow a branching path according to the PERM rules */

  if (i_sect == ffs->nsections) {
    *sumw += wt;
    /*  printf("success, a path has reached B!! weight %lf\n", wt);*/
    return;
  }

  if (i_sect > 0) {
    lambda_min = ffs->section[i_sect-1].lambda_min;
  }
  else {
    lambda_min = ffs->lambda_1;
  }
  lambda_max = ffs->section[i_sect].lambda_max;
  
  kk = ffs->section[i_sect].Ntrials;

  /* this is the weight of each branch fired from this interface */
  wtnow = wt/((double) kk);

  for (ikk = 0; ikk < kk; ikk++) {
  
    /* if(i_sect==0) printf("sect %d trial %d\n", i_sect, ikk);*/
    /* fire off the kk branches */
  
    sp = simulation_state_allocate();
    simulation_state_copy(startpoint, sp);
    res = simulation_run_to_lambda(sp, lambda_min, lambda_max, ffs);
  
    if (res == FFS_TRIAL_WENT_BACKWARDS || res == FFS_TRIAL_TIMED_OUT) {
      branched_recursive_prune(i_sect-1, wtnow, sp, lambda_min, imaxxx, sumw,
			       ffs);
      simulation_state_free(sp);
    }
    else if (res == FFS_TRIAL_SUCCEEDED) {
      /* ADD NEW NODE IN TREE; GENERATE NEW STATE, RECURSE WITH NEW STATE */
      branched_recursive_trial(i_sect+1, wtnow, sp, imaxxx, sumw, ffs);
      simulation_state_free(sp);
    }
    else {
      printf("wrong value for prune dude\n");
      abort();
    }
  }

  return;
}

/*****************************************************************************
 *
 *  branched_recursive_prune
 *
 *****************************************************************************/

void branched_recursive_prune(int i_sect, double wt, sim_state_t * startpoint,
			      double lambda, int imax, double * sumw,
			      ffs_param_t * ffs) {
  int res;
  double wtnow;
  double lambda_min;
  double lambda_max;

  sim_state_t * sp;

  /* Finish if lambda has gone all the way back to state A, or
   * otherwise with given probability. */

  if (lambda == ffs->lambda_1) return;
  if (ran3() <= ffs->section[i_sect+1].pprune) return;

  /* if it survives then multiply weight by Ntrials */
  /* and just propagate again until it hits another interface */

  wtnow = wt*ffs->section[i_sect+1].Ntrials;

  if (i_sect > 0) {
    lambda_min = ffs->section[i_sect-1].lambda_min;
  }
  else {
    lambda_min = ffs->lambda_1;
  }
  lambda_max = ffs->section[i_sect].lambda_max;

  sp = simulation_state_allocate();
  simulation_state_copy(startpoint, sp);

  res = simulation_run_to_lambda(sp, lambda_min, lambda_max, ffs);

  if (res == FFS_TRIAL_WENT_BACKWARDS || res == FFS_TRIAL_TIMED_OUT) {
    branched_recursive_prune(i_sect-1, wtnow, sp, lambda_min, imax, sumw, ffs);
  }
  else if (res == FFS_TRIAL_SUCCEEDED) {
    branched_recursive_trial(i_sect+1, wtnow, sp, imax, sumw, ffs);
    /*simulation_state_free(sp);*/
  }
  else {
    printf("wrong value for prune dude\n");
    abort();
  }

  simulation_state_free(sp);

  return;
}
