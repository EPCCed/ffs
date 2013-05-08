/*****************************************************************************
 *
 *  ffs_branched.c
 *
 *****************************************************************************/

#include "u/libu.h"
#include "util/ffs_util.h"
#include "util/ranlcg.h"

#include "ffs_private.h"
#include "ffs_param.h"
#include "ffs_state.h"
#include "ffs_branched.h"

enum {FFS_TRIAL_SUCCEEDED = 0, FFS_TRIAL_TIMED_OUT, FFS_TRIAL_WENT_BACKWARDS,
      FFS_TRIAL_WAS_PRUNED, FFS_TRIAL_IN_PROGRESS};

int ffs_branched_init(ffs_init_t * init, ffs_param_t * param, proxy_t * proxy,
		      ffs_state_t * sinit,
		      ranlcg_t * ran, int nlocal, int seed,
		      ffs_result_t * result, int * status);

static int ffs_branched_run_to_time(proxy_t * proxy, double teq,
				    int nstepmax, int * status);

int ffs_branched_recursive(int interface, int inst_id, int id, double wt,
			   int nstepmax, int nsteplambda,
			   ffs_param_t * param, proxy_t * proxy,
			   ranlcg_t * ran, ffs_result_t * result);

int ffs_branched_run_to_lambda(proxy_t * proxy, double lambda_min,
			       double lambda_max, int nstepmax,
			       int nsteplambda, int * status);

int ffs_branched_prune(ffs_param_t * param, proxy_t * proxy, ranlcg_t * ran,
		       int interface, double * wt, ffs_result_t * result,
		       int nstepmax, int nsteplambda, int * status);
int ffs_branched_eq(proxy_t * proxy, ffs_state_t * state, int seed,
		    int nstepmax, double teq);

/*****************************************************************************
 *
 *  ffs_branched_run
 *
 *****************************************************************************/

int ffs_branched_run(ffs_init_t * init, ffs_param_t * param, proxy_t * proxy,
		     int inst_id,
		     int inst_nsim, int inst_seed, mpilog_t * log,
		     ffs_result_t * result) {
  int pid;
  int ntrial;
  int n, nstart;
  int status;
  int nstepmax;
  int nsteplambda;
  int itraj;
  long int lseed;
  double wt;
  ffs_t * ffs = NULL;
  ffs_state_t * sinit = NULL;
  ranlcg_t * ran = NULL;

  dbg_return_if(init == NULL, -1);
  dbg_return_if(param == NULL, -1);
  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(log == NULL, -1);
  dbg_return_if(result == NULL, -1);

  /* trial limits */
  nstepmax = 10000000;
  nsteplambda = 1;

  /* Set up the initial simulation state and save it immediately */

  err_err_if(proxy_id(proxy, &pid));
  err_err_if(proxy_ffs(proxy, &ffs));

  err_err_if(ffs_state_create(inst_id, pid, &sinit));
  err_err_if(ffs_state_id_set(sinit, 0));

  err_err_if(proxy_execute(proxy, SIM_EXECUTE_INIT));

  err_err_if(proxy_state(proxy, SIM_STATE_INIT, ffs_state_stub(sinit)));
  err_err_if(proxy_state(proxy, SIM_STATE_WRITE, ffs_state_stub(sinit)));

  /* Start the trajectory RNG (use instance seed for now) */
  lseed = inst_seed;
  ranlcg_create(lseed, &ran);

  /* Distribute the trials evenly among the simulation instances */
  /* We have a local trial index n, and a global seed (1 + n + nstart) */

  ffs_param_nstate(param, 1, &ntrial);

  ntrial = ntrial / inst_nsim;
  nstart = pid*ntrial;

  mpilog(log, "\n");
  mpilog(log, "Starting %d trials each on %d tasks\n", ntrial, inst_nsim);

  for (n = 0; n < ntrial; n++) {

    itraj = 1 + n + nstart;           /* trajectory number (global) */
    lseed = inst_seed + n + nstart;   /* trajectory seed */
    ranlcg_state_set(ran, lseed);

    ffs_branched_init(init, param, proxy, sinit, ran, n, itraj,
		      result, &status);

    /* We reached the first interface; start the trials! */
    wt = 1.0;
    ffs_branched_recursive(1, inst_id, 1, wt, nstepmax, nsteplambda,
			   param, proxy, ran, result);
  }

  err_err_if(proxy_state(proxy, SIM_STATE_WRITE, ffs_state_stub(sinit)));
  proxy_state(proxy, SIM_STATE_DELETE, ffs_state_stub(sinit));
  proxy_execute(proxy, SIM_EXECUTE_FINISH);

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

int ffs_branched_init(ffs_init_t * init, ffs_param_t * param, proxy_t * proxy,
		      ffs_state_t * sinit, ranlcg_t * rantraj, int nlocaltraj,
		      int itraj,
		      ffs_result_t * result, int * status) {

  ffs_t * ffs = NULL;
  MPI_Comm comm;

  int seed;
  int n, ncross;
  int iovershot;
  int icrossed;
  int mpi_errnol = 0;
  double t0, t1, t_elapsed;
  double lambda_a, lambda_b;
  double lambda, lambda_old;
  double random;

  int init_independent;
  int init_nstepmax;
  int init_nskip;
  double init_prob_accept;
  double teq;

  dbg_return_if(init == NULL, -1);
  dbg_return_if(param == NULL, -1);
  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(result == NULL, -1);

  ffs_init_independent(init, &init_independent);
  ffs_init_nstepmax(init, &init_nstepmax);
  ffs_init_nskip(init, &init_nskip);
  ffs_init_prob_accept(init, &init_prob_accept);
  ffs_init_teq(init, &teq);

  ffs_param_lambda_a(param, &lambda_a);
  ffs_param_lambda_b(param, &lambda_b);

  proxy_ffs(proxy, &ffs);
  ffs_comm(ffs, &comm);

  mpi_errnol = proxy_lambda(proxy);
  mpi_sync_if_any(mpi_errnol, comm);

  ffs_lambda(ffs, &lambda_old);

  /* Equilibrate */

  if (itraj == 1 || init_independent) {

    /* Equilibrate */
    ranlcg_reep_int32(rantraj, &seed);
    err_err_if(ffs_branched_eq(proxy, sinit, seed, init_nstepmax, teq));

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

      /* Re-equilibrate */
      ranlcg_reep_int32(rantraj, &seed);
      err_err_if(ffs_branched_eq(proxy, sinit, seed, init_nstepmax, teq));

      ffs_time(ffs, &t0);
      proxy_lambda(proxy);
      ffs_lambda(ffs, &lambda_old);
      lambda = lambda_old;
    }

    /* Do we have a forward crossing of first interface? */

    icrossed = (lambda_old < lambda_a) && (lambda >= lambda_a);
    MPI_Bcast(&icrossed, 1, MPI_INT, 0, comm);

    if (icrossed) {
      ffs_result_ncross_accum(result, 1);
      ffs_result_ncross(result, &ncross);
      ffs_time(ffs, &t1);
      t_elapsed += (t1 - t0);
      t0 = t1;
      if (ncross % init_nskip == 0) {
	ffs_result_time_set(result, nlocaltraj, t_elapsed);
	random = -1.0;
	if (init_independent) ranlcg_reep(rantraj, &random);
	if (random < init_prob_accept) break;
      }
    }

    lambda_old = lambda;
  }

  return 0;

 mpi_sync:
 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_branched_recursive
 *
 *****************************************************************************/

int ffs_branched_recursive(int interface, int inst_id, int id, double wt,
			   int nstepmax, int nsteplambda,
			   ffs_param_t * param, proxy_t * proxy,
			   ranlcg_t * ran, ffs_result_t * result) {

  int nlambda;
  int ntrial, itrial;
  int status;
  int pid;
  int seed;
  double lambda_min;
  double lambda_max;
  double wtnow;
  ffs_state_t * s_keep = NULL;

  ffs_param_nlambda(param, &nlambda);
  ffs_param_weight_accum(param, interface, wt);
  ffs_result_weight_accum(result, interface, wt);
  ffs_result_trial_success_add(result, interface);

  /* If we have reached the final state then end the recursion */

  if (interface == nlambda) return 0;

  ffs_param_lambda(param, interface - 1, &lambda_min);
  ffs_param_lambda(param, interface + 1, &lambda_max);
  ffs_param_ntrial(param, interface, &ntrial);

  /* Save this current state, as it needs to be restored. */

  err_err_if(proxy_id(proxy, &pid));
  err_err_if(ffs_state_create(inst_id, pid, &s_keep));
  err_err_if(ffs_state_id_set(s_keep, id));
  proxy_state(proxy, SIM_STATE_WRITE, ffs_state_stub(s_keep));

  for (itrial = 0; itrial < ntrial; itrial++) {

    /* fire off the branches with total weight 1.0*incoming weight */
    wtnow = wt / ((double) ntrial);

    ffs_branched_run_to_lambda(proxy, lambda_min, lambda_max, nstepmax,
			       nsteplambda, &status);

    if (status == FFS_TRIAL_WENT_BACKWARDS || status == FFS_TRIAL_TIMED_OUT) {
      ffs_branched_prune(param, proxy, ran, interface, &wtnow, result,
			 nstepmax, nsteplambda, &status);
    }

    if (status == FFS_TRIAL_SUCCEEDED) {
      ffs_branched_recursive(interface + 1, inst_id, ++id, wtnow,
			     nstepmax, nsteplambda, param,
			     proxy, ran, result);
    }

    /* Reset and next trial with new seed, or end of trials */

    proxy_state(proxy, SIM_STATE_READ, ffs_state_stub(s_keep));
    ranlcg_reep_int32(ran, &seed);
    proxy_cache_info_int(proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
    proxy_info(proxy, FFS_INFO_RNG_SEED_FETCH);
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
			       double lambda_max, int nstepmax,
			       int nsteplambda, int * status) {
  int n;
  int nstep = 0;
  double lambda;
  ffs_t * ffs = NULL;

  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(status == NULL, -1);

  proxy_ffs(proxy, &ffs);

  *status = FFS_TRIAL_IN_PROGRESS;

  while (*status == FFS_TRIAL_IN_PROGRESS) {

    proxy_lambda(proxy);
    ffs_lambda(ffs, &lambda);

    if (nstep >= nstepmax) *status = FFS_TRIAL_TIMED_OUT;
    if (lambda < lambda_min) *status = FFS_TRIAL_WENT_BACKWARDS;
    if (lambda >= lambda_max) *status = FFS_TRIAL_SUCCEEDED;

    /* Synchronise on status required */
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

int ffs_branched_prune(ffs_param_t * param, proxy_t * proxy, ranlcg_t * ran,
		       int interface, double * wt, ffs_result_t * result,
		       int nstepmax, int nsteplambda, int * status) {

  int n;
  double lambda_min, lambda_max;
  double random;
  double prob_prune;

  dbg_return_if(param == NULL, -1);
  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(ran == NULL, -1);
  dbg_return_if(wt == NULL, -1);
  dbg_return_if(result == NULL, -1);
  dbg_return_if(status == NULL, -1);

  /* If interface <= 2, we get the chop automatically */

  ffs_param_lambda(param, interface + 1, &lambda_max);
  *status = FFS_TRIAL_WAS_PRUNED;

  for (n = interface; n > 2; n--) {

    ranlcg_reep(ran, &random);
    ffs_param_pprune(param, n - 1, &prob_prune);

    *status = FFS_TRIAL_WAS_PRUNED;
    if (random < prob_prune) break;

    /* Trial survives, update weight and continue... */

    *wt *= 1.0 / (1.0 - prob_prune);

    ffs_param_lambda(param, n - 2, &lambda_min);
    ffs_branched_run_to_lambda(proxy, lambda_min, lambda_max, nstepmax,
			       nsteplambda, status);

    if (*status == FFS_TRIAL_SUCCEEDED) break;
  }

  /* We may have exited from the final interation going backwards */
  if (*status == FFS_TRIAL_WENT_BACKWARDS) *status = FFS_TRIAL_WAS_PRUNED;

  if (*status == FFS_TRIAL_WAS_PRUNED) {
    ffs_result_prune_add(result, n);
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_branched_eq
 *
 *  Equilibrate from the given state. As this involves a read of
 *  simulation state, it is potentially fragile so can result
 *  in a collective failure in the proxy communicator.
 *
 *****************************************************************************/

int ffs_branched_eq(proxy_t * proxy, ffs_state_t * state, int seed,
		    int nstepmax, double teq) {

  int mpi_errnol = 0;
  int status;
  MPI_Comm comm;

  dbg_return_if(proxy == NULL, -1);
  dbg_return_if(state == NULL, -1);

  proxy_comm(proxy, &comm);

  mpi_errnol = proxy_state(proxy, SIM_STATE_READ, ffs_state_stub(state));
  mpi_sync_if_any(mpi_errnol, comm);

  proxy_cache_info_int(proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
  proxy_info(proxy, FFS_INFO_RNG_SEED_FETCH);

  mpi_errnol = ffs_branched_run_to_time(proxy, teq, nstepmax, &status);
  mpi_sync_if_any(mpi_errnol || status != FFS_TRIAL_SUCCEEDED, comm);

  return 0;

 mpi_sync:

  return -1;
}
