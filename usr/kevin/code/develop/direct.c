/****************************************************************************
 *
 *  direct.c
 *
 *  Routines for Direct FFS algorithm.
 *
 *  CHANGE OF RESULTS: 'laststate' issue in initialise
 *  CHANGE OF RESULTS: replaced ran3()
 *  CHANGE OF RESULTS: replaced ran in gillespie
 *
 *  ISSUE: Reconfigure to allow transparent file 'cache' and 'uncache'
 *  ISSUE: Replace Ensemble by tree
 *  ISSUE: Combine initialise with 'run and skip'
 *
 ****************************************************************************/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#include "ffs.h"
#include "propagate.h"
#include "ffs_general.h"
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
int get_point(ffs_param_t * ffs, Ensemble, double);
Ensemble direct_advance_ensemble(int i_sect, Ensemble current,
				 ffs_param_t * ffs);


/*****************************************************************************
 *
 *  direct_driver
 *
 *  Driver section for direct algorithm.
 *
 *****************************************************************************/

void direct_driver(ffs_param_t * ffs) {

  int n;
  double runtime;
  sim_state_t * s_init;

  Ensemble current;
  Ensemble next;

  simulation_set_up(ffs);
  s_init = simulation_state_initialise(ffs); 

  direct_initial_ensemble(&current, &runtime, s_init, ffs);

  for (n = 1; n < ffs->nlambda; n++) {

    printf("Interface %3d start trials lambda %5.2f -> %5.2f\n", n,
	   ffs->interface[n].lambda, ffs->interface[n+1].lambda);
    
    next = direct_advance_ensemble(n, current, ffs);

    free_ensemble(current);
    current = convert_ensemble(next);
    free_ensemble(next);
    printf("Interface %3d weight %lf\n", n, ffs->interface[n].weight);
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
			    sim_state_t * s_init, ffs_param_t * ffs) {
  int bb;
  int nstates;
  double lambda, old_lambda;
  double lambda_a;

  sim_state_t * snew;

  nstates = ffs->interface[1].nstates;
  current->N = 0;
  lambda_a = ffs_param_lambda_a(ffs);

  /* MAKE TRIAL BASED ON HEAD OF TREE s_init assign rnd seed */
  snew = simulation_state_allocate();
  simulation_state_copy(s_init, snew);

  simulation_run_to_time(snew, ffs->teq, ffs->nstepmax);
  simulation_state_time_set(snew, 0.0);
  lambda = simulation_lambda(snew);

  /* THE RESULTS OF THE TRIAL ARE NOT INSERTED IN TREE HERE. */

  *runtime = 0.0;
  bb = 0;
  ffs->ncross = 0;
 
  while ((*runtime) < ffs->trun) {
  
    old_lambda = lambda;

    /* SLIGHTLY OBSCURE 0 gives one step */
    simulation_run_to_time(snew, DBL_MAX, 0);

    lambda = simulation_lambda(snew);
    (*runtime) = simulation_state_time(snew);

    if (lambda >= ffs_param_lambda_b(ffs)) {
      /* TRIAL HAS 'failed' */
      /* MAKE NEW TRIAL BASED ON HEAD OF TREE */
      simulation_state_copy(s_init, snew);
      simulation_run_to_time(snew, ffs->teq, ffs->nstepmax);
      simulation_state_time_set(snew, (*runtime));
      lambda = simulation_lambda(snew);
      old_lambda = lambda;
    }

    /* CHECKING FOR CROSSINGS of A state boundary */
    
    if ((old_lambda < lambda_a) && (lambda >= lambda_a)) {

      ++ffs->ncross;

      if ((bb < nstates) && (ffs->ncross % ffs->nskip == 0)) {

	/* INSERT CURRENT STATE INTO NEW NODE IN TREE CHILD OF HEAD */
	/* RECORD STATE and continue current trial */

	current->pstates[bb] = simulation_state_allocate();
	simulation_state_copy(snew, current->pstates[bb]);

	current->wt[bb] = 1.0;
	current->N++;
	bb++;
      }
    }

    ffs->runsteps++;
  }

  if (bb < nstates - 1) {
    printf(" collection run was not long enough! %d %d\n", bb, nstates);
    /* UNCONTROLLED EXIT */
    abort();
  }

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

Ensemble direct_advance_ensemble(int interface, Ensemble current,
				 ffs_param_t * ffs) {
  int pt, res1, ipath, bb;
  int keep_state;
  int ntrials;
  double sumwt, wt;
  double lambda_min, lambda_max;
  Ensemble next;

  sim_state_t * snew;

  /* make Ntrials attempts and just take the first Npoints successful ones */

  bb = 0;
  next.N = 0;

  ffs->interface[interface].weight = 0.0;
  lambda_min = ffs->interface[interface - 1].lambda;
  lambda_max = ffs->interface[interface + 1].lambda;
  ntrials = ffs->interface[interface].ntrials;

  for (ipath = 0; ipath < ntrials; ipath++) {

    /* choose a starting point according to weights and make a trial */
    sumwt = get_sumwt(current);
    pt = get_point(ffs, current, sumwt);

    wt = 1.0;

    /* CHOOSE TREE NODE AND COPY STATE FOR NEW TRIAL */
    snew = simulation_state_allocate();
    simulation_state_copy(current.pstates[pt], snew);

    res1 = simulation_run_to_lambda(snew, lambda_min, lambda_max, ffs);
 
    if (res1 == FFS_TRIAL_WENT_BACKWARDS || res1 == FFS_TRIAL_TIMED_OUT) {
      res1 = ffs_general_prune(snew, interface, &wt, ffs);
    }

    keep_state = 0;

    if (res1 == FFS_TRIAL_SUCCEEDED) {

      ffs->interface[interface].weight += wt;

      /* KEEP SUCCESSFUL STATES */

      if (bb < ffs->interface[interface+1].nstates) {

	/* ADD NEW TREE NODE AT NEXT LEVEL AND RECORD STATE */
	next.pstates[bb] = snew;
	keep_state = 1;

	next.wt[bb] = wt;
	next.N++;
	bb++;
      }
    }

    if (keep_state == 0) simulation_state_free(snew);
    /* TREE FINISH TRIAL */
  }

  if (bb < ffs->interface[interface+1].nstates) {
    printf("DID NOT GET EXPECTED NUMBER OF STATES\n");
  }

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


int get_point(ffs_param_t * ffs, Ensemble ee, double sumwt) {

  double rs;
  double cum;
  int i;

  /* choose a point from the ensemble according to their weights */

  rs = ranlcg_reep(ffs->random);
  rs *= sumwt;
  
  i = 0;
  cum = ee.wt[i];
   
  while (cum < rs) {
    i ++;
    cum += ee.wt[i];
  }

  return i;
}

static void output_data(double runtime, const ffs_param_t * ffs) {

 FILE *fp;
 int i;
 double plambda;
 double pforward;
 double flux;

 fp = fopen("Results.dat","w");
 
 fprintf(fp, "the flux collection run contained %d steps\n", ffs->runsteps);
 fprintf(fp, "the run time was %lf\n", runtime);
 fprintf(fp,"the number of crossings was %d\n", ffs->ncross);

 flux = (double) ffs->ncross / runtime;
 fprintf(fp,"so the flux through the first interface was %20.10lf\n", flux);

 fprintf(fp,"firing stage (no pruning) contained %d steps\n", ffs->firesteps);
 fprintf(fp,"the pruning contained %d steps\n", ffs->prunesteps);
 fprintf(fp,"the total number of simulation steps was %d\n",
	 ffs->runsteps + ffs->firesteps + ffs->prunesteps);

 fprintf(fp, "\n");
 fprintf(fp, "Interface    lambda       weight   P(forward)    P(lambda)\n");
 fprintf(fp, "----------------------------------------------------------\n");

 plambda = 1.0;

 for (i = 1; i <= ffs->nlambda; i++) {
   plambda *= ffs->interface[i-1].weight / ffs->interface[i-1].ntrials;
   pforward = ffs->interface[i].weight / ffs->interface[i].ntrials;
   fprintf(fp,"    %3d  %10.3e %12.5e %12.5e %12.5e\n", i,
	   ffs->interface[i].lambda, ffs->interface[i].weight,
	   pforward, plambda);
 }

 fprintf(fp, "\n");
 fprintf (fp, "final results:\n");

 fprintf(fp, "flux through first interface: %12.5e\n", flux);
 fprintf(fp, "probability of reaching B:    %12.5e\n", plambda);
 fprintf(fp, "rate constant:                %12.5e\n",  flux*plambda);

 fclose(fp);

  return;
}
