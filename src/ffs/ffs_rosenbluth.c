/*****************************************************************************
 *
 *  ffs_rosenbluth.c
 *
 *****************************************************************************/

#include "u/libu.h"
#include "util/ffs_util.h"
#include "util/ranlcg.h"

#include "ffs_private.h"
#include "ffs_state.h"
#include "ffs_rosenbluth.h"

static int ffs_rosenbluth_recursive(ffs_trial_arg_t * trial, int interface,
				    int id, double wt, ranlcg_t * ran);

/*****************************************************************************
 *
 *  ffs_rosenbluth_run
 *
 *****************************************************************************/

int ffs_rosenbluth_run(ffs_trial_arg_t * trial) {

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

    stub = util_filename_stub(trial->inst_id, pid, 1);
    proxy_state(trial->proxy, SIM_STATE_WRITE, stub);

    wt = 1.0;
    ffs_rosenbluth_recursive(trial, 1, 1, wt, ran);

    stub = util_filename_stub(trial->inst_id, pid, 1);
    proxy_state(trial->proxy, SIM_STATE_DELETE, stub);

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
 *  ffs_rosenbluth_recursive
 *
 *  We perform k trials for every state, record all the successful ones,
 *  but pick at most one to continue. Note that only state which are
 *  assoicated with trials contribute to the weight w_b,i at a given
 *  interface. 
 *
 *  We need to make all the trials before the recursion.
 *
 *****************************************************************************/

static int ffs_rosenbluth_recursive(ffs_trial_arg_t * trial, int interface,
				    int id, double wt, ranlcg_t * ran) {
  int it;
  int nlambda;
  int ntrial, itrial;
  int status;
  int pid;
  int seed;
  int nsuccess;
  int * nlist = NULL;
  double lambda_min;
  double lambda_max;
  double wtnow;
  const char * stub = NULL;

  ffs_param_nlambda(trial->param, &nlambda);
  ffs_result_weight_accum(trial->result, interface, wt);

  /* If we have reached the final state then end the recursion */

  if (interface == nlambda) return 0;

  ffs_param_lambda(trial->param, interface - 1, &lambda_min);
  ffs_param_lambda(trial->param, interface + 1, &lambda_max);
  ffs_param_ntrial(trial->param, interface, &ntrial);

  dbg_err_if(proxy_id(trial->proxy, &pid));

  nsuccess = 0;
  nlist = calloc(ntrial, sizeof(int));

  for (itrial = 0; itrial < ntrial; itrial++) {

    /* Make a new trial with new seed */
    ffs_result_nstart_add(trial->result, interface);

    ranlcg_reep_int32(ran, &seed);
    proxy_cache_info_int(trial->proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
    proxy_info(trial->proxy, FFS_INFO_RNG_SEED_FETCH);

    ffs_trial_run_to_lambda(trial, lambda_min, lambda_max, &status);

    if (status == FFS_TRIAL_WENT_BACKWARDS || status == FFS_TRIAL_TIMED_OUT) {
      ffs_trial_prune(trial, interface, ran, &wtnow, &status);
    }

    /* If we have succeed, record the successful end state */

    if (status != FFS_TRIAL_SUCCEEDED) {
      ffs_result_nback_add(trial->result, interface);
    }
    else {
      nlist[nsuccess] = id + itrial + 1;
      stub = util_filename_stub(trial->inst_id, pid, id + itrial + 1);
      proxy_state(trial->proxy, SIM_STATE_WRITE, stub);
      ffs_result_trial_success_add(trial->result, interface);
      nsuccess += 1;
    }

    /* Re-read the original state if a further trial is required */

    if (itrial < ntrial - 1) {
      stub = util_filename_stub(trial->inst_id, pid, id);
      proxy_state(trial->proxy, SIM_STATE_READ, stub);
    }
  }


  /* Now the weight for successful trials is... */

  wtnow = (wt*nsuccess) / ntrial;
  ffs_result_success_weight_accum(trial->result, interface, wtnow);

  if (nsuccess == 0) {
    /* Just fall through and exit the recursion */
  }
  else {

    /* If we have any successes at all, choose one at random to
     * follow it via recursion. Delete all the unwanted states
     * before going into the recursion to prevent a pile up. */

    ranlcg_reep_int32(ran, &itrial);
    itrial = itrial % nsuccess;

    for (it = 0; it < nsuccess; it++) {
      if (it != itrial) {
	stub = util_filename_stub(trial->inst_id, pid, nlist[it]);
	proxy_state(trial->proxy, SIM_STATE_DELETE, stub);
	ffs_result_ndrop_add(trial->result, interface);
      }
    }

    stub = util_filename_stub(trial->inst_id, pid, nlist[itrial]);
    proxy_state(trial->proxy, SIM_STATE_READ, stub);

    ffs_rosenbluth_recursive(trial, interface + 1, nlist[itrial], wtnow, ran);

    stub = util_filename_stub(trial->inst_id, pid, nlist[itrial]);
    proxy_state(trial->proxy, SIM_STATE_DELETE, stub);
  }

  free(nlist);

  return 0;

 err:

  if (nlist) free(nlist);
  mpilog(trial->log, "Failed at interface %d\n", interface);

  return -1;
}


/*****************************************************************************
 *
 *  ffs_rosenbluth_results
 *
 *  Display the results in a human-readable format
 *
 *****************************************************************************/

int ffs_rosenbluth_results(ffs_trial_arg_t * trial) {

  int ntrial, nlambda, nsuccess;
  int n, neq, ntry, ndrop, nback, nto;
  int nsum_trial = 0, nsum_back = 0, nsum_success = 0, nsum_nto = 0;
  double tmax, tsum, wt, swt, lambda, plambda;

  dbg_return_if(trial == NULL, -1);

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Instance results\n\n");

  ffs_init_ntrials(trial->init, &ntrial);
  ffs_result_status_final(trial->result, FFS_TRIAL_SUCCEEDED, &nsuccess);
  ffs_result_status_final(trial->result, FFS_TRIAL_TIMED_OUT, &n);
  ffs_result_eq_final(trial->result, &neq);

  mpilog(trial->log, "Attempts at first interface:         %d\n", ntrial);
  mpilog(trial->log, "States generated at first interface: %d\n", nsuccess);
  mpilog(trial->log, "Time outs at first interface:        %d\n", n);
  mpilog(trial->log, "Number of equilibration runs:        %d\n", neq);
  mpilog(trial->log, "\n");

  ffs_param_nlambda(trial->param, &nlambda);

  mpilog(trial->log,
  "                   forward                                  Prod.of\n");
  mpilog(trial->log,
  "index      lambda   trials success  pruned   to dropped     weights\n");
  mpilog(trial->log,
  "-------------------------------------------------------------------\n");

  plambda = 1.0;

  for (n = 1; n <= nlambda; n++) {
    ffs_param_lambda(trial->param, n, &lambda);
    ffs_result_weight(trial->result, n, &wt);
    ffs_result_success_weight(trial->result, n, &swt);
    ffs_result_nback(trial->result, n, &nback);
    ffs_result_nto(trial->result, n, &nto);
    ffs_result_nstart(trial->result, n, &ntry);
    ffs_result_ndrop(trial->result, n, &ndrop);
    ffs_result_trial_success(trial->result, n, &nsuccess);

    if (n == 1) {
      plambda = 1.0;
    }
    else {
      ffs_result_weight(trial->result, n-1, &wt);
      ffs_result_success_weight(trial->result, n-1, &swt);
      if (wt > 0.0) plambda *= (swt/wt);
    }

    mpilog(trial->log, "  %3d %11.4e  %7d %7d %7d %4d %7d %11.4e\n", n, lambda,
	   ntry, nsuccess, nback, nto, ndrop, plambda);

    nsum_trial += ntry;
    nsum_success += nsuccess;
    nsum_back += nback;
    nsum_nto += nto;
  }

  mpilog(trial->log,
  "-------------------------------------------------------------------\n");
  mpilog(trial->log, "(totals)         %9d %7d %7d %4d\n", nsum_trial,
	 nsum_success, nsum_back, nsum_nto);

  ffs_result_tmax(trial->result, &tmax);
  ffs_result_tsum(trial->result, &tsum);
  ffs_result_ncross(trial->result, &n);

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Initial Tmax:  result   %12.6e\n", tmax);
  mpilog(trial->log, "Initial Tsum:  result   %12.6e\n", tsum);
  mpilog(trial->log, "Number of crossings A:  %d\n", n);
  mpilog(trial->log, "Flux at lambda_A:       %12.6e\n", n/tsum);
  mpilog(trial->log, "\n");
  mpilog(trial->log, "Probability P(B|A):     %12.6e\n", plambda);
  mpilog(trial->log, "Flux * P(B|A):          %12.6e\n", (n/tsum)*plambda);

  return 0;
}
