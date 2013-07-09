/*****************************************************************************
 *
 *  ffs_branched.c
 *
 *****************************************************************************/

#include "u/libu.h"
#include "util/ffs_util.h"
#include "util/ranlcg.h"

#include "ffs_private.h"
#include "ffs_state.h"
#include "ffs_branched.h"

static int ffs_branched_recursive(ffs_trial_arg_t * trial, int interface,
				  int id, double wt, ranlcg_t * ran);

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
  int itraj;
  long int lseed;
  double wt;
  const char * stub = NULL;
  ffs_t * ffs = NULL;
  ranlcg_t * ran = NULL;
  ffs_state_t * sinit = NULL;

  dbg_return_if(trial == NULL, -1);

  /* Set up the initial simulation state and save it immediately */

  dbg_err_if( proxy_id(trial->proxy, &pid) );
  dbg_err_if( proxy_ffs(trial->proxy, &ffs) );

  dbg_err_if(proxy_execute(trial->proxy, SIM_EXECUTE_INIT));

  /* Save initial reference state with id = 0 */

  stub = util_filename_stub(trial->inst_id, pid, 0);
  dbg_err_if( proxy_state(trial->proxy, SIM_STATE_INIT, stub) );
  dbg_err_if( proxy_state(trial->proxy, SIM_STATE_WRITE, stub) );
  dbg_err_if( ffs_state_create(trial->inst_id, pid, &sinit) );
  dbg_err_if( ffs_state_id_set(sinit, 0) );

  /* Start the trajectory RNG (use instance seed for now) */

  lseed = trial->inst_seed;
  ranlcg_create(lseed, &ran);

  /* Distribute the trials evenly among the simulation instances */
  /* We have a local trial index n, and a global seed (1 + n + nstart) */

  ffs_init_ntrials(trial->init, &ntrial);

  dbg_err_if(ntrial % trial->nproxy != 0);

  ntrial = ntrial / trial->nproxy;
  nstart = pid*ntrial;

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Starting %d trials each on %d proxies\n", ntrial,
	 trial->nproxy);

  for (n = 0; n < ntrial; n++) {

    itraj = 1 + n + nstart;                  /* trajectory number (global) */
    lseed = trial->inst_seed + n + nstart;   /* trajectory seed */
    ranlcg_state_set(ran, lseed);

    ffs_trial_init(trial, sinit, ran, n, itraj, &status);

    if (status != FFS_TRIAL_SUCCEEDED) continue;

    /* We reached the first interface; start the trials! */

    wt = 1.0;
    ffs_branched_recursive(trial, 1, 1, wt, ran);
  }

  /* Remove the reference state, and finish */

  stub = util_filename_stub(trial->inst_id, pid, 0);
  proxy_state(trial->proxy, SIM_STATE_DELETE, stub);
  proxy_execute(trial->proxy, SIM_EXECUTE_FINISH);

  ranlcg_free(ran);

  return 0;

 err:

  if (ran) ranlcg_free(ran);
  if (sinit) ffs_state_free(sinit);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_branched_recursive
 *
 *****************************************************************************/

static int ffs_branched_recursive(ffs_trial_arg_t * trial, int interface,
				  int id, double wt, ranlcg_t * ran) {

  int nlambda;
  int ntrial, itrial;
  int status;
  int pid;
  int seed;
  double lambda_min;
  double lambda_max;
  double wtnow;
  ffs_state_t * s_keep = NULL;

  ffs_param_nlambda(trial->param, &nlambda);
  ffs_param_weight_accum(trial->param, interface, wt);
  ffs_result_weight_accum(trial->result, interface, wt);
  ffs_result_trial_success_add(trial->result, interface);

  /* If we have reached the final state then end the recursion */

  if (interface == nlambda) return 0;

  ffs_param_lambda(trial->param, interface - 1, &lambda_min);
  ffs_param_lambda(trial->param, interface + 1, &lambda_max);
  ffs_param_ntrial(trial->param, interface, &ntrial);

  /* Save this current state, as it needs to be restored. */

  dbg_err_if(proxy_id(trial->proxy, &pid));
  dbg_err_if(ffs_state_create(trial->inst_id, pid, &s_keep));
  dbg_err_if(ffs_state_id_set(s_keep, id));
  proxy_state(trial->proxy, SIM_STATE_WRITE, ffs_state_stub(s_keep));

  for (itrial = 0; itrial < ntrial; itrial++) {

    /* fire off the branches with total weight 1.0*incoming weight */
    wtnow = wt / ((double) ntrial);

    ffs_trial_run_to_lambda(trial, lambda_min, lambda_max, &status);

    if (status == FFS_TRIAL_WENT_BACKWARDS || status == FFS_TRIAL_TIMED_OUT) {
      ffs_trial_prune(trial, interface, ran, &wtnow, &status);
    }

    if (status == FFS_TRIAL_SUCCEEDED) {
      ffs_branched_recursive(trial, interface + 1, ++id, wtnow, ran);
    }

    /* Reset and next trial with new seed, or end of trials */

    proxy_state(trial->proxy, SIM_STATE_READ, ffs_state_stub(s_keep));
    ranlcg_reep_int32(ran, &seed);
    proxy_cache_info_int(trial->proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
    proxy_info(trial->proxy, FFS_INFO_RNG_SEED_FETCH);
  }

  proxy_state(trial->proxy, SIM_STATE_DELETE, ffs_state_stub(s_keep));
  ffs_state_free(s_keep);

  return 0;

 err:

  mpilog(trial->log, "Failed at interface %d\n", interface);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_branched_results
 *
 *  Display the results in a human-readable format
 *
 *****************************************************************************/

int ffs_branched_results(ffs_trial_arg_t * trial) {

  int ntrial, nlambda;
  int n, nstates, ntry, nprune;
  int nsum_trial = 0, nsum_prune = 0, nsum_success = 0;
  double tsum, f1, wt, lambda;

  dbg_return_if(trial == NULL, -1);

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Conditional probabilities\n");
  mpilog(trial->log, "-------------------------\n");

  ffs_init_ntrials(trial->init, &ntrial);
  ffs_param_nlambda(trial->param, &nlambda);

  mpilog(trial->log,
	 "index      lambda    trials   states   pruned   wt/ntrial\n");
  mpilog(trial->log,
	 "---------------------------------------------------------\n");

  for (n = 1; n <= nlambda; n++) {
    ffs_param_lambda(trial->param, n, &lambda);
    ffs_result_weight(trial->result, n, &wt);
    ffs_result_trial_success(trial->result, n, &nstates);
    ffs_result_prune(trial->result, n, &nprune);
    ffs_param_ntrial(trial->param, n, &ntry);

    mpilog(trial->log, "  %3d %11.4e  %8d %8d %8d %11.4e\n", n, lambda,
	   nstates*ntry, nstates, nprune, wt/ntrial);

    nsum_trial += nstates*ntry;
    if (n > 1) nsum_success += nstates;
    nsum_prune += nprune;
  }

  mpilog(trial->log,
	 "----------------------------------------------------------\n");
  mpilog(trial->log, "(sum/success/fail) %8d %8d %8d\n", nsum_trial,
	 nsum_success, nsum_prune);

  ffs_result_aflux_tsum_final(trial->flux, &tsum);
  ffs_result_aflux_ncross_final(trial->flux, &n);
  f1 = n/tsum;
  ffs_result_summary_stat_set(trial->summary, f1, wt/ntrial);

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Probability P(B|A):     %12.6e\n", wt/ntrial);
  mpilog(trial->log, "Flux * P(B|A):          %12.6e\n", f1*wt/ntrial);

  return 0;
}
