/*****************************************************************************
 *
 *  ffs_branched.c
 *
 *****************************************************************************/

#include "u/libu.h"
#include "util/ranlcg.h"

#include "ffs_param.h"
#include "ffs_state.h"
#include "ffs_branched.h"

enum {FFS_TRIAL_SUCCEEDED = 0, FFS_TRIAL_TIMED_OUT, FFS_TRIAL_WENT_BACKWARDS,
      FFS_TRIAL_WAS_PRUNED, FFS_TRIAL_IN_PROGRESS};

typedef struct result_s result_t;
struct result_s {
  int ncross;
  double t0[10000];
  double tsum;
  double tmax;
};

int ffs_branched_init(ffs_inst_t * inst, proxy_t * proxy, ffs_state_t * sinit,
		      ranlcg_t * ran, int seed, result_t * res, int * status);
static int ffs_branched_run_to_time(proxy_t * proxy, double teq,
				    int nstepmax, int * status);

int ffs_branched_recursive(int interface, int id, double wt,
			   ffs_inst_t * inst, proxy_t * proxy, ranlcg_t * ran);

int ffs_branched_run_to_lambda(proxy_t * proxy, double lambda_min,
			       double lambda_max, int nstepmax, int * status);

int ffs_branched_prune(ffs_inst_t * inst, proxy_t * proxy, ranlcg_t * ran,
		       int interface, double * wt,
		       int nstepmax, int * status);

/*****************************************************************************
 *
 *  ffs_branched_run
 *
 *****************************************************************************/

int ffs_branched_run(ffs_inst_t * inst, proxy_t * proxy, int seed,
		     mpilog_t * log) {

  int nlambda;
  double lambda;

  int inst_id;
  int pid;
  int ntrial;
  int n, nstart;
  int status;
  int sim_nsim_inst;
  long int lseed;
  double wt;
  ffs_t * ffs = NULL;
  ffs_state_t * sinit = NULL;
  ffs_param_t * param = NULL;
  ranlcg_t * ran = NULL;

  result_t * stats = NULL;

  dbg_return_if(inst == NULL, -1);
  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(log == NULL, -1);

  /* Results object */

  stats = u_calloc(1, sizeof(result_t));
  err_err_if(stats == NULL);


  /* Set up the initial simulation state and save it immediately */

  err_err_if(ffs_inst_id(inst, &inst_id));
  err_err_if(proxy_id(proxy, &pid));
  err_err_if(proxy_ffs(proxy, &ffs));

  err_err_if(ffs_state_create(inst_id, pid, &sinit));
  err_err_if(ffs_state_id_set(sinit, 0));

  err_err_if(proxy_execute(proxy, SIM_EXECUTE_INIT));

  err_err_if(proxy_state(proxy, SIM_STATE_INIT, ffs_state_stub(sinit)));
  err_err_if(proxy_state(proxy, SIM_STATE_WRITE, ffs_state_stub(sinit)));

  /* Start the (serial) RNG */
  lseed = seed;
  ranlcg_create(lseed, &ran);

  /* Distribute the trials evenly among the simulation instances */

  ffs_inst_nsim(inst, &sim_nsim_inst);
  ffs_inst_param(inst, &param);
  ffs_param_nstate(param, 1, &ntrial);

  ntrial = ntrial / sim_nsim_inst;
  nstart = pid*ntrial;

  mpilog(log, "\n");
  mpilog(log, "Starting %d trials each on %d tasks\n", ntrial, sim_nsim_inst);

  for (n = nstart; n < nstart + ntrial; n++) {
    ffs_branched_init(inst, proxy, sinit, ran, 1 + n, stats, &status);
    wt = 1.0;
    ffs_branched_recursive(1, 1, wt, inst, proxy, ran);
  }

  err_err_if(proxy_state(proxy, SIM_STATE_WRITE, ffs_state_stub(sinit)));
  proxy_state(proxy, SIM_STATE_DELETE, ffs_state_stub(sinit));
  proxy_execute(proxy, SIM_EXECUTE_FINISH);

  /* Result */
  ffs_param_nlambda(param, &nlambda);
  ffs_param_nstate(param, 1, &ntrial);

  mpilog(log, "Interface probabilities\n");

  for (n = 1; n <= nlambda; n++) {
    ffs_param_lambda(param, n, &lambda);
    ffs_param_weight(param, n, &wt);
    mpilog(log, "%3d %11.4e %11.4e\n", n, lambda, wt/ntrial);
  }


  for (n = 1; n <= ntrial; n++) {
    stats->tsum += stats->t0[n];
    if (stats->t0[n] > stats->tmax) stats->tmax = stats->t0[n];
  }

  mpilog(log, "Total number of trials: %d\n", ntrial);
  mpilog(log, "Initial Tmax:           %12.6e\n", stats->tmax);
  mpilog(log, "Initial Tsum:           %12.6e\n", stats->tsum);
  mpilog(log, "Number of crossings A:  %d\n", stats->ncross);
  mpilog(log, "Flux at lambda_A:       %12.6e\n", stats->ncross/stats->tsum);
  mpilog(log, "Probability P(B|A):     %12.6e\n", wt/ntrial);
  mpilog(log, "Flux * P(B|A):          %12.6e\n",
	 (stats->ncross/stats->tsum)*wt/ntrial);
  u_free(stats);

  ranlcg_free(ran);
  ffs_state_free(sinit);

  return 0;

 err:

  if (ran) ranlcg_free(ran);
  if (sinit) ffs_state_free(sinit);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_branched_init
 *
 *****************************************************************************/

int ffs_branched_init(ffs_inst_t * inst, proxy_t * proxy, ffs_state_t * sinit,
		      ranlcg_t * ran, int seed, result_t * res, int * status) {

  ffs_t * ffs = NULL;
  ffs_param_t * param = NULL;
  MPI_Comm comm;

  int n;
  int iovershot;
  int icrossed;
  double t0, t1, t_elapsed;
  double lambda_a, lambda_b;
  double lambda, lambda_old;
  double random;

  /* TEST */
  int init_independent = 0;
  int init_nstepmax = 10000000;
  int init_nskip = 10;
  double init_prob_accept = 1.0;
  double teq = 100.0;

  dbg_return_if(inst == NULL, -1);
  dbg_return_if(proxy == NULL, -1);

  ffs_inst_param(inst, &param);
  ffs_param_lambda_a(param, &lambda_a);
  ffs_param_lambda_b(param, &lambda_b);

  proxy_ffs(proxy, &ffs);
  ffs_comm(ffs, &comm);

  proxy_lambda(proxy);
  ffs_lambda(ffs, &lambda_old);

  /* Equilibrate */

  if (seed == 1 || init_independent) {
    /* Equilibrate again from initial state */
    proxy_state(proxy, SIM_STATE_READ, ffs_state_stub(sinit));

    if (init_independent) {
      ffs_info_int(ffs, FFS_INFO_RNG_SEED_PUT, 1, &seed);
      proxy_info(proxy, FFS_INFO_RNG_SEED_FETCH);
    }
    ffs_branched_run_to_time(proxy, teq, init_nstepmax, status);
    /* mpi_sync_if(*status != FFS_TRIAL_SUCCEEDED);*/
  }

  proxy_lambda(proxy);
  ffs_lambda(ffs, &lambda_old);

  t_elapsed = 0.0;
  proxy_info(proxy, FFS_INFO_TIME_PUT);
  ffs_time(ffs, &t0);

  for (n = 0; n < init_nstepmax; n++) {

    proxy_execute(proxy, SIM_EXECUTE_RUN);
    proxy_lambda(proxy);
    ffs_lambda(ffs, &lambda);
    ffs_time(ffs, &t1);

    /* Overshot to state B? Come back to initial state. */

    iovershot = (lambda >= lambda_b);
    MPI_Bcast(&iovershot, 1, MPI_INT, 0, comm);

    if (iovershot) {
      t_elapsed += (t1 - t0);
      proxy_state(proxy, SIM_STATE_READ, ffs_state_stub(sinit));
      ffs_branched_run_to_time(proxy, teq, init_nstepmax, status);
      /* mpi_sync_if(*status != FFS_TRIAL_SUCCEEDED);*/
      ffs_time(ffs, &t0);
      proxy_lambda(proxy);
      ffs_lambda(ffs, &lambda_old);
      lambda = lambda_old;
    }

    /* Do we have a forward crossing of first interface? */

    icrossed = (lambda_old < lambda_a) && (lambda >= lambda_a);
    MPI_Bcast(&icrossed, 1, MPI_INT, 0, comm);

    if (icrossed) {
      res->ncross += 1;
      ffs_time(ffs, &t1);
      t_elapsed += (t1 - t0);
      t0 = t1;
      if (res->ncross % init_nskip == 0) {
	res->t0[seed] = t_elapsed;
	random = -1.0;
	if (init_independent) ranlcg_reep(ran, &random);
	if (random < init_prob_accept) break;
      }
    }

    lambda_old = lambda;
  }

 mpi_sync:

  /*printf("[%d] Final %f after %d steps t_e %f\n", seed, lambda, n, t_elapsed);*/

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_branched_recursive
 *
 *****************************************************************************/

int ffs_branched_recursive(int interface, int id, double wt,
			   ffs_inst_t * inst, proxy_t * proxy,
			   ranlcg_t * ran) {

  int nlambda;
  int ntrial, itrial;
  int status;
  int inst_id, pid;
  double lambda_min;
  double lambda_max;
  double wtnow;
  ffs_param_t * param = NULL;
  ffs_state_t * s_keep = NULL;

  int nstepmax = 10000000; /* TRIAL NSTEPMAX */

  ffs_inst_param(inst, &param);
  ffs_param_nlambda(param, &nlambda);

  ffs_param_weight_accum(param, interface, wt);

  /* If we have reached the final state then end the recursion */

  if (interface == nlambda) return 0;

  ffs_param_lambda(param, interface - 1, &lambda_min);
  ffs_param_lambda(param, interface + 1, &lambda_max);
  ffs_param_ntrial(param, interface, &ntrial);

  /* Save this current state, as it needs to be restored. */

  err_err_if(ffs_inst_id(inst, &inst_id));
  err_err_if(proxy_id(proxy, &pid));
  err_err_if(ffs_state_create(inst_id, pid, &s_keep));
  err_err_if(ffs_state_id_set(s_keep, id));
  proxy_state(proxy, SIM_STATE_WRITE, ffs_state_stub(s_keep));

  for (itrial = 0; itrial < ntrial; itrial++) {

    /* fire off the branches with total weight 1.0*incoming weight */
    wtnow = wt / ((double) ntrial);

    ffs_branched_run_to_lambda(proxy, lambda_min, lambda_max, nstepmax,
			       &status);

    if (status == FFS_TRIAL_WENT_BACKWARDS || status == FFS_TRIAL_TIMED_OUT) {
      ffs_branched_prune(inst, proxy, ran, interface, &wtnow, nstepmax,
			 &status);
    }

    if (status == FFS_TRIAL_SUCCEEDED) {
      ffs_branched_recursive(interface + 1, ++id, wtnow, inst, proxy, ran);
    }

    /* Reset and next trial, or end of trials*/
    proxy_state(proxy, SIM_STATE_READ, ffs_state_stub(s_keep));
  }

  proxy_state(proxy, SIM_STATE_DELETE, ffs_state_stub(s_keep));
  ffs_state_free(s_keep);

  return 0;

 err:

  printf("Failed in branched algoirthm\n");

  return -1;
}


/*****************************************************************************
 *
 *  ffs_branched_run_to_time
 *
 *  Errors pending mpi_sync_if_any()
 *
 *****************************************************************************/

static int ffs_branched_run_to_time(proxy_t * proxy, double teq, int nstepmax,
				    int * status) {

  int n;
  double t;
  ffs_t * ffs = NULL;

  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(status == NULL, -1);

  /* If we fall out of the bottom of the loop, the status is timed out */
  *status = FFS_TRIAL_TIMED_OUT;

  proxy_ffs(proxy, &ffs);

  for (n = 0; n <= nstepmax; n++) {

    proxy_execute(proxy, SIM_EXECUTE_RUN);
    ffs_time(ffs, &t);

    if (t >= teq) {
      *status = FFS_TRIAL_SUCCEEDED;
      break;
    }
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_branched_run_to_lambda
 *
 *****************************************************************************/

int ffs_branched_run_to_lambda(proxy_t * proxy, double lambda_min,
			       double lambda_max, int nstepmax, int * status) {

  int n;
  int nstep = 0;
  int nsteplambda = 1; /* REPLACE ME */
  double lambda;
  ffs_t * ffs = NULL;

  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(status == NULL, -1);

  proxy_ffs(proxy, &ffs);

  *status = FFS_TRIAL_IN_PROGRESS;

  while (*status == FFS_TRIAL_IN_PROGRESS) {

    proxy_lambda(proxy);
    ffs_lambda(ffs, &lambda);

    if (lambda < lambda_min) *status = FFS_TRIAL_WENT_BACKWARDS;
    if (lambda >= lambda_max) *status = FFS_TRIAL_SUCCEEDED;
    if (nstep >= nstepmax) *status = FFS_TRIAL_TIMED_OUT;

    /* Synchronise on status required? */
    if (*status != FFS_TRIAL_IN_PROGRESS) break;

    for (n = 0; n < nsteplambda; n++) {
      proxy_execute(proxy, SIM_EXECUTE_RUN);
      nstep += 1;
      /* firesteps += 1;*/
    }
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_branched_prune
 *
 *****************************************************************************/

int ffs_branched_prune(ffs_inst_t * inst, proxy_t * proxy, ranlcg_t * ran,
		       int interface, double * wt,
		       int nstepmax, int * status) {

  int n;
  double lambda_min, lambda_max;
  double random;
  double prob_prune;
  ffs_param_t * param = NULL;

  ffs_inst_param(inst, &param);
  ffs_param_lambda(param, interface + 1, &lambda_max);
  *status = FFS_TRIAL_WAS_PRUNED; /* If interface <= 2, we get the chop */

  for (n = interface; n > 2; n--) {

    ranlcg_reep(ran, &random);
    ffs_param_pprune(param, n - 1, &prob_prune);

    *status = FFS_TRIAL_WAS_PRUNED;
    if (random < prob_prune) break;

    /* Trial survives, update weight and continue... */

    *wt *= 1.0 / (1.0 - prob_prune);

    ffs_param_lambda(param, n - 2, &lambda_min);
    ffs_branched_run_to_lambda(proxy, lambda_min, lambda_max, nstepmax,
			       status);

    if (*status == FFS_TRIAL_SUCCEEDED) break;
  }

  return 0;
}
