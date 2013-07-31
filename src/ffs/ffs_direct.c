/*****************************************************************************
 *
 *  ffs_direct.c
 *
 *****************************************************************************/

#include "util/ffs_ensemble.h"
#include "ffs_direct.h"

static int ffs_direct_init(ffs_state_t * sref, ffs_trial_arg_t * trial,
			   ffs_ensemble_t * states);
static int ffs_direct_exec(ffs_state_t * sref, ffs_trial_arg_t * trial);
static int ffs_direct_advance(ffs_ensemble_t ** old, ffs_trial_arg_t * trial);

static int ffs_direct_trials(ffs_trial_arg_t * trial, int interface,
			     ffs_ensemble_t * old, ffs_ensemble_t * new,
			     int * ncum_trial);

static int ffs_direct_delete(ffs_ensemble_t * old, ffs_trial_arg_t * trial,
			     int interface);

static int ffs_direct_close_up(ffs_ensemble_t * old, ffs_ensemble_t * new,
			       ffs_trial_arg_t * trial, int interface);

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

  ffs_direct_exec(sref, trial);

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
 *  ffs_direct_delete
 *
 *  Delete the files associated with an interface (first proxy only).
 *
 *****************************************************************************/

static int ffs_direct_delete(ffs_ensemble_t * old, ffs_trial_arg_t * trial,
			     int interface) {
  int n;
  int pid;
  int mpi_errnol = 0;
  const char * stub = NULL;

  MPI_Comm comm;

  dbg_return_if(old == NULL, -1);
  dbg_return_if(trial == NULL, -1);

  proxy_id(trial->proxy, &pid);
  proxy_comm(trial->proxy, &comm);

  if (pid == 0) {
    for (n = 0; n < old->nsuccess; n++) {
      stub = util_filename_stub(trial->inst_id, interface, old->traj[n]);
      mpi_errnol = proxy_state(trial->proxy, SIM_STATE_DELETE, stub);
      dbg_ifm(mpi_errnol, "Failed to remove file %s", stub);
    }
  }

  /* An error may have occured removing a file, but we should try to
   * continue */

  return 0;
}

/*****************************************************************************
 *
 *  ffs_direct_close_up
 *
 *  Form a global list of successes in the instance cross comminucator,
 *  add up the total number of successes, and form a global list. If
 *  the number of successes is greater than the number of states
 *  required, we delete the excess.
 *
 *****************************************************************************/

static int ffs_direct_close_up(ffs_ensemble_t * old, ffs_ensemble_t * new,
			       ffs_trial_arg_t * trial, int interface) {

  ffs_ensemble_t * list = NULL;
  int * list_nsuccess = NULL;
  int * displs = NULL;
  int nsuccess;
  int nexcess;
  int pid;
  int n, ntmp;

  const char * stub = NULL;

  dbg_return_if(old == NULL, -1);
  dbg_return_if(new == NULL, -1);

  dbg_err_if( proxy_id(trial->proxy, &pid) );

  /* Form the global list of successful trials from the (local) old ensemble */

  list_nsuccess = u_calloc(trial->nproxy, sizeof(int));
  dbg_err_if(list_nsuccess == NULL);

  MPI_Allgather(&old->nsuccess, 1, MPI_INT, list_nsuccess, 1, MPI_INT,
		trial->xcomm);

  nsuccess = 0;
  for (n = 0; n < trial->nproxy; n++) {
    nsuccess += list_nsuccess[n];
  }

  displs = u_calloc(trial->nproxy, sizeof(int));
  dbg_err_if(displs == NULL);
  dbg_err_if( ffs_ensemble_create(nsuccess, &list) );

  displs[0] = 0;
  for (n = 1; n < trial->nproxy; n++) {
    displs[n] = displs[n-1] + list_nsuccess[n-1];
  }

  MPI_Allgatherv(old->traj, old->nsuccess, MPI_INT,
		 list->traj, list_nsuccess, displs, MPI_INT, trial->xcomm);

  MPI_Allgatherv(old->wt, old->nsuccess, MPI_DOUBLE,
		 list->wt, list_nsuccess, displs, MPI_DOUBLE, trial->xcomm);

  /* Delete excess. Only one proxy is required to delete the files, but
   * all instance ranks delete their record of the state. */

  nexcess = nsuccess - new->nmax;

  for (n = 0; n < nexcess; n += 1) {
    ntmp = n*(nsuccess/nexcess);
    if (pid == 0) {
      stub = util_filename_stub(trial->inst_id, interface, list->traj[ntmp]);
      proxy_state(trial->proxy, SIM_STATE_DELETE, stub);
    }
    /* These are skipped in the next loop */
    list->traj[ntmp] = -1;
  }

  new->nsuccess = 0;
  for (n = 0; n < nsuccess; n++) {
    if (list->traj[n] != -1) {
      new->traj[new->nsuccess] = list->traj[n];
      new->wt[new->nsuccess] = list->traj[n];
      new->nsuccess += 1;
    }
  }

  u_free(displs);
  u_free(list_nsuccess);
  ffs_ensemble_free(list);

  return 0;

 err:

  mpilog(trial->log, "Problem in closing up states (maybe deadlock!)\n");

  if (displs) u_free(displs);
  if (list_nsuccess) u_free(list_nsuccess);
  if (list) ffs_ensemble_free(list);

  return  -1;
}

/*****************************************************************************
 *
 *  ffs_direct_exec
 *
 *****************************************************************************/

static int ffs_direct_exec(ffs_state_t * sref, ffs_trial_arg_t * trial) {

  int n, nstate;
  ffs_ensemble_t * states = NULL;

  dbg_return_if(sref == NULL, -1);
  dbg_return_if(trial == NULL, -1);

  /* Set up the initial (global) ensemble, and then run. */

  dbg_err_if( ffs_param_nstate(trial->param, 1, &nstate));
  dbg_err_if( ffs_ensemble_create(nstate, &states) );

  mpilog(trial->log, "Generating %d initial direct states\n", nstate);
  dbg_err_if( ffs_direct_init(sref, trial, states) );

  for (n = 0; n < states->nsuccess; n++) {
    states->wt[n] = 1.0;
  }

  mpilog(trial->log, "Advancing states\n");

  dbg_err_if( ffs_direct_advance(&states, trial) );
  ffs_ensemble_free(states);

  return 0;

 err:

  mpilog(trial->log, "Failed to execute direct ffs correctly\n");
  if (states) ffs_ensemble_free(states);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_direct_init
 *
 *****************************************************************************/

static int ffs_direct_init(ffs_state_t * sref, ffs_trial_arg_t * trial,
		    ffs_ensemble_t * states) {

  int n, nstart;
  int ntrial, ntrial_local;
  int pid;
  int interface = 1;
  int itraj;
  int status;
  long int lseed;
  const char * stub = NULL;

  ranlcg_t * ran = NULL;
  ffs_ensemble_t * list_local = NULL;

  dbg_return_if(sref == NULL, -1);
  dbg_return_if(trial == NULL, -1);

  /* Initial states */

  dbg_err_if( proxy_id(trial->proxy, &pid) );

  ffs_init_ntrials(trial->init, &ntrial);
  dbg_err_if(ntrial % trial->nproxy != 0);

  ntrial_local = ntrial / trial->nproxy;
  nstart = pid*ntrial_local;
  dbg_err_if( ffs_ensemble_create(ntrial_local, &list_local) );

  /* Start the trajectory RNG */

  lseed = trial->inst_seed;
  ranlcg_create(lseed, &ran);

  for (n = 0; n < ntrial_local; n++) {

    itraj = 1 + n + nstart;                  /* parallel trial id */
    lseed = trial->inst_seed + n + nstart;   /* trajectory seed */
    ranlcg_state_set(ran, lseed);

    ffs_trial_init(trial, sref, ran, n, itraj, &status);

    if (status != FFS_TRIAL_SUCCEEDED) continue;

    /* Record state (interface = 1) */

    list_local->traj[list_local->nsuccess] = itraj;
    stub = util_filename_stub(trial->inst_id, interface, itraj);
    proxy_state(trial->proxy, SIM_STATE_WRITE, stub);

    ffs_result_trial_success_add(trial->result, 1);
    list_local->nsuccess += 1;
  }

  ffs_direct_close_up(list_local, states, trial, interface);
  ffs_result_nkeep_set(trial->result, 1, states->nsuccess);

  ranlcg_free(ran);
  ffs_ensemble_free(list_local);

  return 0;

 err:

  mpilog(trial->log, "Failure in generating direct initial states\n");
  if (ran) ranlcg_free(ran);
  if (list_local) ffs_ensemble_free(list_local);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_direct_advance
 *
 *****************************************************************************/

static int ffs_direct_advance(ffs_ensemble_t ** old, ffs_trial_arg_t * trial) {

  int n, nlambda, nstate;
  int ncum_trial = 0;          /* Cummulative number of trials (global) */
  ffs_ensemble_t * new = NULL;

  dbg_return_if(old == NULL, -1);
  dbg_return_if(trial == NULL, -1);

  dbg_err_if( ffs_param_nlambda(trial->param, &nlambda) );

  for (n = 1; n < nlambda; n++) {

    /* If there are no states, leave the loop, otherwise continue */

    if ((*old)->nsuccess == 0) {
      mpilog(trial->log, "No states to continue from lambda %d.\n", n);
      break;
    }

    mpilog(trial->log, "Direct trial from interface %2d\n", n);

    ffs_param_nstate(trial->param, n+1, &nstate);
    ffs_ensemble_create(nstate, &new);

    /* Make trials from existing states to new */

    dbg_err_if( ffs_direct_trials(trial, n, *old, new, &ncum_trial) );

    /* Remove old states and the old ensemble structure; update the
     * ensemble pointers for the next step, or at the end we exit
     * with the final ensemble being "old" to be returned. */

    ffs_direct_delete(*old, trial, n);
    ffs_ensemble_free(*old);
    *old = new; new = NULL;
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_direct_trials
 *
 *****************************************************************************/

static int ffs_direct_trials(ffs_trial_arg_t * trial, int interface,
			     ffs_ensemble_t * old, ffs_ensemble_t * new,
			     int * ncum_trial) {

  int n, ntrial, ntrial_local;
  int itraj, irun;
  int pid, nstart;
  int status;
  int seed;
  long int lseed;
  double wt;
  double lambda_min, lambda_max;

  const char * stub = NULL;
  ranlcg_t * ran = NULL;
  ffs_ensemble_t * list_local = NULL;

  dbg_return_if(trial == NULL, -1);
  dbg_return_if(old == NULL, -1);
  dbg_return_if(new == NULL, -1);

  dbg_err_if( ffs_param_lambda(trial->param, interface - 1, &lambda_min) );
  dbg_err_if( ffs_param_lambda(trial->param, interface + 1, &lambda_max) );
  dbg_err_if( ffs_param_ntrial(trial->param, interface, &ntrial) );

  dbg_err_if( proxy_id(trial->proxy, &pid) );

  ffs_param_ntrial(trial->param, interface, &ntrial);
  dbg_err_if(ntrial % trial->nproxy != 0);

  ntrial_local = ntrial / trial->nproxy;
  nstart = pid*ntrial_local;
  dbg_err_if( ffs_ensemble_create(ntrial_local, &list_local) );

  lseed = trial->inst_seed;
  ranlcg_create(lseed, &ran);

  for (n = 0; n < ntrial_local; n++) {

    itraj = 1 + n + nstart + *ncum_trial;    /* parallel trial id */
    lseed = trial->inst_seed + itraj - 1;    /* trajectory seed */
    ranlcg_state_set(ran, lseed);

    /* Choose state from old according to weight and load the state */

    dbg_err_if(ffs_ensemble_samplewt(old, ran, &irun));
    dbg_err_if(irun >= old->nsuccess);
    stub = util_filename_stub(trial->inst_id, interface, old->traj[irun]);
    dbg_err_if( proxy_state(trial->proxy, SIM_STATE_READ, stub) );

    /* Inject a seed into the simulation */

    ranlcg_reep_int32(ran, &seed);
    proxy_cache_info_int(trial->proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
    proxy_info(trial->proxy, FFS_INFO_RNG_SEED_FETCH);

    /* Run to lambda_max */

    wt = 1.0;
    ffs_trial_run_to_lambda(trial, lambda_min, lambda_max, &status);

    if (status == FFS_TRIAL_WENT_BACKWARDS || status == FFS_TRIAL_TIMED_OUT) {
      ffs_trial_prune(trial, interface, ran, &wt, &status);
    }


    /* If success, keep the state, add wt contribution to interface */

    if (status != FFS_TRIAL_SUCCEEDED) continue;

    list_local->traj[list_local->nsuccess] = itraj;
    list_local->wt[list_local->nsuccess] = wt;
    stub = util_filename_stub(trial->inst_id, interface + 1, itraj);
    dbg_err_if(proxy_state(trial->proxy, SIM_STATE_WRITE, stub));

    ffs_result_trial_success_add(trial->result, interface + 1);
    ffs_result_weight_accum(trial->result, interface + 1, wt);
    list_local->nsuccess += 1;
  }

  dbg_err_if( ffs_direct_close_up(list_local, new, trial, interface + 1) );
  ffs_result_nkeep_set(trial->result, interface + 1, new->nsuccess);

  ranlcg_free(ran);
  ffs_ensemble_free(list_local);

  *ncum_trial += ntrial;

  return 0;

 err:

  return -1;
}


/*****************************************************************************
 *
 *  ffs_direct_results
 *
 *  Display the results in a human-readable format
 *
 *****************************************************************************/

int ffs_direct_results(ffs_trial_arg_t * trial) {

  int ntrial, nlambda, nsuccess;
  int n, nstates, ntry, nprune, nto;
  int nsum_trial = 0, nsum_prune = 0, nsum_success = 0, nsum_nto = 0;
  double f1, tsum, wt, lambda, plambda;

  dbg_return_if(trial == NULL, -1);

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Conditional probabilities\n");
  mpilog(trial->log, "-------------------------\n");

  ffs_init_ntrials(trial->init, &ntrial);
  ffs_result_nkeep(trial->result, 1, &nstates);
  ffs_param_nlambda(trial->param, &nlambda);

  mpilog(trial->log,
	 "                  states   forw.                        Prod.of\n");
  mpilog(trial->log,
	 "index      lambda   kept  trials  success pruned   to   weights\n");
  mpilog(trial->log,
	 "----------------------------------------------------------------\n");

  plambda = 1.0;

  for (n = 1; n <= nlambda; n++) {
    ffs_param_lambda(trial->param, n, &lambda);
    ffs_result_weight(trial->result, n, &wt);
    ffs_result_nkeep(trial->result, n, &nstates);
    ffs_result_prune(trial->result, n, &nprune);
    ffs_result_nto(trial->result, n, &nto);

    if (n > 1) {
      ffs_param_ntrial(trial->param, n - 1, &ntry);
      if (wt > 1.0*ntry) wt = 1.0*ntry;   /* Can happen with pruning */
      if (ntry > 0) plambda *= (wt / ntry);
    }

    ffs_param_ntrial(trial->param, n, &ntry);

    nsuccess = 0;
    if (n < nlambda) {
      ffs_result_trial_success(trial->result, n+1, &nsuccess);
      if (nsuccess == 0) plambda = 0.0; /* zero if no trials & no success */
    }

    mpilog(trial->log, "  %3d %11.4e  %5d %7d %7d %7d %4d %11.4e\n", n, lambda,
	   nstates, ntry, nsuccess, nprune, nto, plambda);

    nsum_trial += ntry;
    nsum_success += nsuccess;
    nsum_prune += nprune;
    nsum_nto += nto;
  }

  mpilog(trial->log,
	 "----------------------------------------------------------------\n");
  mpilog(trial->log, "(totals)                 %7d %7d %7d %4d\n", nsum_trial,
	 nsum_success, nsum_prune, nsum_nto);

  ffs_result_aflux_tsum_final(trial->flux, &tsum);
  ffs_result_aflux_ncross_final(trial->flux, &n);
  f1 = n/tsum;
  ffs_result_summary_stat_set(trial->summary, f1, plambda);

  mpilog(trial->log, "\n");
  mpilog(trial->log, "Probability P(B|A):     %12.6e\n", plambda);
  mpilog(trial->log, "Flux * P(B|A):          %12.6e\n", (n/tsum)*plambda);

  return 0;
}
