/****************************************************************************
 *
 *  direct.c
 *
 *  Routines for Direct FFS algorithm.
 *
 ****************************************************************************/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#include "ffs.h"
#include "propagate.h"
#include "direct.h"

/* REMOVE in favour of dynamic structure. */
#define MAXPOINTS 10000

typedef struct ensemble_type {

  sim_state_t * pstates[MAXPOINTS];
  double wt[MAXPOINTS];
  int N;

} Ensemble;

/* END REMOVE */

int direct_initial_ensemble(Ensemble *, double *, sim_state_t * s,
			    ffs_param_t * ffs);
static void output_data(double, const ffs_param_t * ffs);
double get_sumwt(Ensemble);
Ensemble convert_ensemble(Ensemble);
void free_ensemble(Ensemble);
int get_point(Ensemble, double);
Ensemble direct_advance_ensemble(int i_sect, Ensemble current,
				 ffs_param_t * ffs);
int direct_prune(sim_state_t * p, int isect, double *wt,  ffs_param_t * ffs);


/*****************************************************************************
 *
 *  direct_driver
 *
 *  Driver section for direct algorithm.
 *
 *****************************************************************************/

void direct_driver(ffs_param_t * ffs) {

  int i;
  double runtime;
  sim_state_t * s_init;

  Ensemble current;
  Ensemble next;

  simulation_set_up(ffs);
  s_init = simulation_state_initialise(ffs);

  direct_initial_ensemble(&current, &runtime, s_init, ffs);

  for (i = 0; i < ffs->nsections; i++) {

    printf(" getting paths for section %d %lf %lf\n", i,
	   ffs->section[i].lambda_min, ffs->section[i].lambda_max);
    
    next = direct_advance_ensemble(i, current, ffs);

    free_ensemble(current);
    current = convert_ensemble(next);
    free_ensemble(next);
  }

  simulation_state_finalise(s_init);
  simulation_tear_down();

  output_data(runtime, ffs);

  return;
}

/*****************************************************************************
 *
 *  direct_initial_ensemble
 *
 *  Equilibrate and generate an initial ensemble at the first
 *  interface.
 *
 *****************************************************************************/

int direct_initial_ensemble(Ensemble * current, double *runtime,
			    sim_state_t * s_eq, ffs_param_t * ffs) {
  int    s,bb;
  int laststate;
  double lambda, old_lambda;

  sim_state_t * snew;

  current->N = 0;

  snew = simulation_state_allocate();
  simulation_state_copy(s_eq, snew);

  simulation_run_to_time(snew, ffs->teq, ffs->nstepmax);
  simulation_state_time_set(snew, 0.0);
  lambda = simulation_lambda(snew);

  *runtime = 0.0;
  bb = 0;
  ffs->ncross = 0;

  if (lambda < ffs->lambda_1) {
    laststate = 0;
  }
  else if (lambda >= ffs->lambda_1) {
    laststate = 1;
  }

  old_lambda = lambda;

  s = 0;
 
  while ((*runtime) < ffs->trun) {
  
    old_lambda = lambda;

    /* SLIGHTLY OBSCURE 0 gives one step */
    simulation_run_to_time(snew, DBL_MAX, 0);

    lambda = simulation_lambda(snew);
    (*runtime) = simulation_state_time(snew);

    if (lambda >= ffs->lambda_2) {

      simulation_state_copy(s_eq, snew);
      simulation_run_to_time(snew, ffs->teq, ffs->nstepmax);
      simulation_state_time_set(snew, (*runtime));
      lambda = simulation_lambda(snew);
      old_lambda = lambda;
      /* IS THIS EXECUTED?  */
    }

    /* CHECKING FOR CROSSINGS of A state boundary */
    
    if ((old_lambda < ffs->lambda_1) && (lambda >= ffs->lambda_1)) {
      /* left A state */
      if (laststate == 0) {
	++ffs->ncross;

	if ((bb < ffs->section[0].Npoints)&&(ffs->ncross % ffs->nskip == 0)) {

	  current->pstates[bb] = simulation_state_allocate();
	  simulation_state_copy(snew, current->pstates[bb]);

          /* Rosalind says the following line should be removed */
	  /* But it changes the results so I'm going to leave it */
	  current->wt[bb] = 1.0;
          /* End comment */
	  current->N++;
	  bb++;
	}
      laststate=1;
      }
    }

    if ((old_lambda >= ffs->lambda_1) && (lambda < ffs->lambda_1)){
      /* entered A state */
      laststate = 0;
    }
    
    s++;
  }
  
  if (bb < ffs->section[0].Npoints - 1) {  
    printf(" collection run was not long enough! %d %d\n", bb,
	   ffs->section[0].Npoints);
    /* UNCONTROLLED EXIT */
    abort();
  }

  ffs->runsteps += s;

  simulation_state_free(snew);

  return 0;
}

void free_ensemble(Ensemble ee) {

  int i;
  
  for (i = 0; i < ee.N; i++){
    simulation_state_free(ee.pstates[i]);
    ee.wt[i] = 0.0;
  }

  return;
}

Ensemble convert_ensemble(Ensemble e_in) {

  Ensemble e_out;
  int i;

  e_out.N = e_in.N;

  for (i = 0; i < e_in.N; i++) {
    e_out.pstates[i] = simulation_state_allocate();
    simulation_state_copy(e_in.pstates[i], e_out.pstates[i]);    
    e_out.wt[i] = e_in.wt[i];
  }

  return e_out;
}

/*****************************************************************************
 *
 *  direct_advance_ensemble
 *
 *****************************************************************************/

Ensemble direct_advance_ensemble(int i_sect, Ensemble current,
				 ffs_param_t * ffs) {
  int pt, res1, ipath, bb;
  int keep_state;
  double sumwt, wt;
  double lambda_min, lambda_max;
  Ensemble next;

  sim_state_t * snew;

  /* make Ntrials attempts and just take the first Npoints successful ones */

  ffs->section[i_sect].forward = 0.0;
  bb = 0;
  next.N = 0;

  for (ipath = 0; ipath < ffs->section[i_sect].Ntrials; ipath++) {

    /* choose a starting point according to weights and make a trial */
    sumwt = get_sumwt(current);
    pt = get_point(current, sumwt);

    wt = 1.0;

    if (i_sect == 0) {
      lambda_min = ffs->lambda_1;
    }
    else {
      lambda_min = ffs->section[i_sect-1].lambda_min;
    }
    lambda_max = ffs->section[i_sect].lambda_max;

    snew = simulation_state_allocate();
    simulation_state_copy(current.pstates[pt], snew);

    res1 = simulation_run_to_lambda(snew, lambda_min, lambda_max, ffs);
 
    if (res1 == FFS_TRIAL_WENT_BACKWARDS || res1 == FFS_TRIAL_TIMED_OUT) {
      res1 = direct_prune(snew, i_sect, &wt, ffs);
    }

    keep_state = 0;

    if (res1 == FFS_TRIAL_SUCCEEDED) {

      ffs->section[i_sect].forward += wt;

      /* KEEP SUCCESSFUL STATES UP TO Npoints (except last interface) */

      if (i_sect < ffs->nsections-1) {
	if (bb < ffs->section[i_sect+1].Npoints) {
	  /* add to the next section */

	  next.pstates[bb] = snew;
	  keep_state = 1;

	  next.wt[bb] = wt;
	  next.N++;
	  bb++;
	}
      }
    }

    if (keep_state == 0) simulation_state_free(snew);
  }

  if (bb < ffs->section[i_sect+1].Npoints-1 && i_sect < ffs->nsections-1){
    printf("insufficient successes, sect: %d, succ: %d , req: %d %lf %lf\n",
	   i_sect, bb, ffs->section[i_sect+1].Npoints,
	   ffs->section[i_sect].lambda_min, ffs->section[i_sect].lambda_max);

    /* UNCONTROLLED EXIT */
    abort();
  }
  
  printf(" end of section %d forward %lf success %d\n", i_sect,
	 ffs->section[i_sect].forward, bb);
  
  return next;
}

double get_sumwt(Ensemble ee) {

  double sumwt;
  int i;

  sumwt = 0.0;
  for (i = 0; i < ee.N; i++){
    sumwt += ee.wt[i];
  }

  return sumwt;
}


int get_point(Ensemble ee, double sumwt) {

  double rs;
  double cum;
  int i;

  /* choose a point from the ensemble according to their weights */

  rs = ran3();
  rs *= sumwt;
  
  i = 0;
  cum = ee.wt[i];
   
  while (cum < rs) {
    i ++;
    cum += ee.wt[i];
  }

  return i;
}

/*****************************************************************************
 *
 *  direct_prune
 *
 *  stop when we
 *    (a) reach the desired interface,
 *    (b) are pruned, or
 *    (c) reach A region
 *
 *****************************************************************************/

int direct_prune(sim_state_t * p, int isect, double *wt,  ffs_param_t * ffs) {

  int ii;
  int res;
  double lambda_min, lambda_max;

  *wt = 1.0;
 
  if (isect > 1) {
    for (ii = isect - 1; ii >= 1; ii--) {

      if (ran3() < ffs->section[ii].pprune) {
	return FFS_TRIAL_WAS_PRUNED;
      }
      else {
	/* survives, multiply weight */
	
	*wt *= 1.0/(1.0 - ffs->section[ii].pprune);

	lambda_min = ffs->section[ii-1].lambda_min;
	lambda_max = ffs->section[isect].lambda_max;
	res = simulation_run_to_lambda(p, lambda_min, lambda_max, ffs);

	if (res == FFS_TRIAL_SUCCEEDED) return FFS_TRIAL_SUCCEEDED;
      }
    }
  }

  return FFS_TRIAL_WENT_BACKWARDS;
}


static void output_data(double runtime, const ffs_param_t * ffs) {

 FILE *fp;
 int i,j,k;
 double plam;

 fp = fopen( "p_lambda_TEST.dat", "w" );
 plam = 1.0;

 for (i = 0; i< ffs->nsections; i++) {
  plam *= ffs->section[i].forward / ((double) ffs->section[i].Ntrials);
  fprintf(fp,"%lf %lf\n", ffs->section[i].lambda_max, plam); 
 
 }

 fclose(fp);

 fp = fopen("Results.dat","w");
 
 
 fprintf(fp, "the flux collection run contained %d steps\n", ffs->runsteps);

 fprintf(fp,"the run time was %lf, the number of crossings was %d, so the flux through the first interface was %20.10lf\n", runtime, ffs->ncross, ((double) ffs->ncross)/runtime);

 fprintf(fp,"the firing stage (not wih pruning) contained %d steps\n", ffs->firesteps);

 fprintf(fp,"the pruning contained %d steps\n", ffs->prunesteps);


 fprintf(fp,"the total number of simulation steps was %d\n",
	 ffs->runsteps + ffs->firesteps + ffs->prunesteps);

 for (i = 0; i < ffs->nsections; i++) {
   fprintf(fp,"interface %d total weight of successful trials %lf so forward probability  is %6.5lf\n", i, ffs->section[i].forward, ((double) ffs->section[i].forward) / ((double) ffs->section[i].Ntrials)); 
 }

 fprintf (fp, "final results:\n");

 fprintf(fp, "flux through first interface %lf probability of reaching B %lf rate constant %lf\n", ((double) ffs->ncross)/runtime, plam, ((double) ffs->ncross)*plam/runtime);


 fclose(fp);


 fp = fopen( "p_lambda.dat", "w" );
 
 /* print the P(lambda) histograms to a file */
  k = 0;
  for (i = 0; i < ffs->nsections; i++) {
    for (j = 0; j < ffs->section[i].Nbins; j++) {
      k++;
    }
  }

  fprintf(fp,"%d\t%d\t%d\t output.dat\n ",k,20, ffs->nsections);

  for (i = 0; i < ffs->nsections; i++) {
    printf("outputting pl histo %d\n", i);
        
    for (j = 0; j < ffs->section[i].Nbins; j++) {
      fprintf(fp,"%lf %lf %2.1f %d\n ",
	      ffs->section[i].lambda_min + (j+0.5)*ffs->section[i].d_lambda,
	      ((double) ffs->section[i].pl_histo[j])
	      / ((double) ffs->section[i].Ntrials), 1.0, i+1);
    }
  }

  fclose(fp);

  /* KEVIN */
  printf("Current number of points allocated: %f\n", nalloc_current());

  return;
}
