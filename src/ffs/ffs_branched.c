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
int ffs_temp_direct(ffs_state_t * sref, ffs_trial_arg_t * trial);




typedef struct ffs_ensemble_s ffs_ensemble_t;

struct ffs_ensemble_s {
  int nmax;
  ffs_state_t ** old;
};

int ffs_ensemble_create(int nmax, int inst, int pid, ffs_ensemble_t ** pobj);
void ffs_ensemble_free(ffs_ensemble_t * obj);


int ffs_ensemble_create(int nmax, int inst, int pid, ffs_ensemble_t ** pobj) {

  int n;
  ffs_ensemble_t * obj = NULL;

  obj= u_calloc(1, sizeof(ffs_ensemble_t));
  dbg_err_if(obj == NULL);

  obj->nmax = nmax;
  obj->old = u_calloc(nmax, sizeof(ffs_state_t *));
  dbg_err_if(obj->old == NULL);

  for (n = 0; n < nmax; n++) {
    dbg_err_if(ffs_state_create(inst, pid, &obj->old[n]));
  }

  *pobj = obj;

  return 0;

 err:

  if (obj) ffs_ensemble_free(obj);

  return -1;
}

void ffs_ensemble_free(ffs_ensemble_t * obj) {

  int n;

  dbg_return_if(obj == NULL,);

  for (n = 0; n < obj->nmax; n++) {
    if (obj->old && obj->old[n]) ffs_state_free(obj->old[n]);
  }

  if (obj->old) u_free(obj->old);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_direct_run
 *
 *****************************************************************************/

int ffs_direct_run(ffs_trial_arg_t * trial) {

  int pid;
  int mpi_errnol = 0;
  const char * stub = NULL;
  ffs_state_t * sref = NULL;

  MPI_Comm comm;

  dbg_return_if(trial == NULL, -1);

  /* As this is the first time the simulation is run, be sure
   * to check for errors and avoid possible deadlock. If any
   * MPI tasks get stuck in the simulation... we'll be stuck
   * too. */

  dbg_err_if( proxy_id(trial->proxy, &pid) );
  dbg_err_if( proxy_comm(trial->proxy, &comm) );

  dbg_err_if( ffs_state_create(trial->inst_id, pid, &sref) );
  dbg_err_if( ffs_state_id_set(sref, 0) );

  mpi_errnol = proxy_execute(trial->proxy, SIM_EXECUTE_INIT);

  if (mpi_errnol) {
    mpilog_all(trial->log, "SIM_EXECUTE_INIT (proxy %d) failed.\n", pid);
  }

  mpi_sync_if_any(mpi_errnol, comm);

  stub = ffs_state_stub(sref);
  dbg_err_if(stub == NULL);

  mpi_errnol = proxy_state(trial->proxy, SIM_STATE_INIT, stub);

  if (mpi_errnol) {
    mpilog_all(trial->log, "SIM_STATE_INIT (proxy %d) failed\n", pid);
  }

  mpi_sync_if_any(mpi_errnol, comm);

  mpi_errnol = proxy_state(trial->proxy, SIM_STATE_WRITE, stub);

  if (mpi_errnol) {
    mpilog_all(trial->log, "SIM_STATE_WRITE (proxy %d) failed\n", pid);
  }

  mpi_sync_if_any(mpi_errnol, comm);

  /* The initialisation has worked, so run the FFS (at last). */

  ffs_temp_direct(sref, trial);

  /* Assume the clean-up will work if we've reached this far;
   * even if it doesn't, let's have a normal exit so we get to
   * to the results... */

  proxy_state(trial->proxy, SIM_STATE_DELETE, stub);
  proxy_execute(trial->proxy, SIM_EXECUTE_FINISH);
  ffs_state_free(sref);

  return 0;

 mpi_sync:

  mpilog(trial->log, "Failed to initialise simulation\n");

 err:

  if (sref) ffs_state_free(sref);
  mpilog(trial->log, "Failed to run requested FFS simulations\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_temp_direct
 *
 *****************************************************************************/

int ffs_temp_direct(ffs_state_t * sref, ffs_trial_arg_t * trial) {

  int n, nstate, nstatemax;
  int pid;
  ffs_ensemble_t * ensemble_old = NULL;
  ffs_ensemble_t * ensemble_new = NULL;

  dbg_return_if(sref == NULL, -1);
  dbg_return_if(trial == NULL, -1);

  /* Initial states */

  dbg_err_if( proxy_id(trial->proxy, &pid) );

  ffs_param_nstate(trial->param, 1, &nstate);
  /* TODO ffs_param_nstate_max(trial->param, &nstatemax); */

  ffs_ensemble_create(nstatemax, trial->inst_id, pid, &ensemble_new);
  ffs_ensemble_create(nstatemax, trial->inst_id, pid, &ensemble_old);

  for (n = 0; n < nstate; n++) {
    /* ffs_branched_init(trial, sref, ran, nlocal, nglobal, &status); */
    /*
      if (status == GOOD) {
        ffs_state_id_set(ensemble_old->old[nsuccess], nsuccess);
	stub = ffs_state_stub(ensemble_old[nsuccess]);
	proxy_state(trial->proxy, SIM_STATE_WRITE, stub);
      }
    */
  }

  ffs_ensemble_free(ensemble_new);
  ffs_ensemble_free(ensemble_old);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_branched_run
 *
 *****************************************************************************/

int ffs_branched_run(ffs_trial_arg_t * trial) {

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

  dbg_return_if(trial == NULL, -1);

  /* trial limits NEED TO BE SET FROM INPUT */
  nstepmax = 10000000;
  nsteplambda = 1;

  /* Set up the initial simulation state and save it immediately */

  err_err_if(proxy_id(trial->proxy, &pid));
  err_err_if(proxy_ffs(trial->proxy, &ffs));

  err_err_if(ffs_state_create(trial->inst_id, pid, &sinit));
  err_err_if(ffs_state_id_set(sinit, 0));

  err_err_if(proxy_execute(trial->proxy, SIM_EXECUTE_INIT));

  err_err_if(proxy_state(trial->proxy, SIM_STATE_INIT, ffs_state_stub(sinit)));
  err_err_if(proxy_state(trial->proxy, SIM_STATE_WRITE, ffs_state_stub(sinit)));

  /* Start the trajectory RNG (use instance seed for now) */
  lseed = trial->inst_seed;
  ranlcg_create(lseed, &ran);

  /* Distribute the trials evenly among the simulation instances */
  /* We have a local trial index n, and a global seed (1 + n + nstart) */

  ffs_param_nstate(trial->param, 1, &ntrial);

  ntrial = ntrial / trial->nproxy;
  nstart = pid*ntrial;

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Starting %d trials each on %d proxies\n", ntrial,
	 trial->nproxy);

  for (n = 0; n < ntrial; n++) {

    itraj = 1 + n + nstart;                  /* trajectory number (global) */
    lseed = trial->inst_seed + n + nstart;   /* trajectory seed */
    ranlcg_state_set(ran, lseed);

    ffs_branched_init(trial->init, trial->param, trial->proxy, sinit,
		      ran, n, itraj,
		      trial->result, &status);

    /* We reached the first interface; start the trials! */
    wt = 1.0;
    ffs_branched_recursive(1, trial->inst_id, 1, wt, nstepmax, nsteplambda,
			   trial->param, trial->proxy, ran, trial->result);
  }

  err_err_if(proxy_state(trial->proxy, SIM_STATE_WRITE, ffs_state_stub(sinit)));
  proxy_state(trial->proxy, SIM_STATE_DELETE, ffs_state_stub(sinit));
  proxy_execute(trial->proxy, SIM_EXECUTE_FINISH);

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
	/* This is the state we want, so break if accepted */
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
  int iprune;
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
    iprune = n - 1;                /* Trial was pruned at this interface.. */
    if (iprune < 1) iprune = 1;    /* ...but not before first interface. */
    ffs_result_prune_add(result, iprune);
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
