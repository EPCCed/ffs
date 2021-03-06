/*****************************************************************************
 *
 *  ffs_trial.c
 *
 *****************************************************************************/

#include "ffs_private.h"
#include "ffs_trial.h"

/*****************************************************************************
 *
 *  ffs_trial_run_to_time
 *
 *****************************************************************************/

int ffs_trial_run_to_time(proxy_t * proxy, double teq, int nstepmax,
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
 *  ffs_trial_run_to_lambda
 *
 *****************************************************************************/

int ffs_trial_run_to_lambda(ffs_trial_arg_t * trial, double lambda_min,
			    double lambda_max, int * status) {
  int n;
  int nstep = 0;
  double lambda;
  ffs_t * ffs = NULL;

  dbg_return_if(trial == NULL, -1);
  dbg_return_if(status == NULL, -1);

  proxy_ffs(trial->proxy, &ffs);

  *status = FFS_TRIAL_IN_PROGRESS;

  while (*status == FFS_TRIAL_IN_PROGRESS) {

    proxy_lambda(trial->proxy);
    ffs_lambda(ffs, &lambda);

    if (nstep >= trial->nstepmax) *status = FFS_TRIAL_TIMED_OUT;
    if (lambda < lambda_min) *status = FFS_TRIAL_WENT_BACKWARDS;
    if (lambda >= lambda_max) *status = FFS_TRIAL_SUCCEEDED;

    /* Synchronise on status required */
    if (*status != FFS_TRIAL_IN_PROGRESS) break;

    for (n = 0; n < trial->nsteplambda; n++) {
      proxy_execute(trial->proxy, SIM_EXECUTE_RUN);
      nstep += 1;
    }
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_trial_prune
 *
 *****************************************************************************/

int ffs_trial_prune(ffs_trial_arg_t * trial, int interface, ranlcg_t * ran,
		    double * wt, int * status) {

  int n;
  int iprune;
  double lambda_min, lambda_max;
  double random;
  double prob_prune;

  dbg_return_if(trial == NULL, -1);
  dbg_return_if(ran == NULL, -1);
  dbg_return_if(wt == NULL, -1);
  dbg_return_if(status == NULL, -1);

  /* If interface <= 2, we get the chop automatically */

  ffs_param_lambda(trial->param, interface + 1, &lambda_max);
  *status = FFS_TRIAL_WAS_PRUNED;

  for (n = interface; n > 2; n--) {

    ranlcg_reep(ran, &random);
    ffs_param_pprune(trial->param, n - 1, &prob_prune);

    *status = FFS_TRIAL_WAS_PRUNED;
    if (random < prob_prune) break;

    /* Trial survives, update weight and continue... */

    *wt *= 1.0 / (1.0 - prob_prune);

    ffs_param_lambda(trial->param, n - 2, &lambda_min);
    ffs_trial_run_to_lambda(trial, lambda_min, lambda_max, status);

    if (*status == FFS_TRIAL_SUCCEEDED) break;
  }

  /* We may have exited from the final interation going backwards */
  if (*status == FFS_TRIAL_WENT_BACKWARDS) *status = FFS_TRIAL_WAS_PRUNED;

  iprune = n - 1;                /* Trial was pruned at this interface.. */
  if (iprune < 1) iprune = 1;    /* ...but not before first interface. */

  if (*status == FFS_TRIAL_WAS_PRUNED) {
    ffs_result_prune_add(trial->result, iprune);
  }

  if (*status == FFS_TRIAL_TIMED_OUT) {
    ffs_result_nto_add(trial->result, iprune, 1);
  }

  return 0;
}

/*****************************************************************************
 *
 *  ffs_trial_initial_state
 *
 *****************************************************************************/

int ffs_trial_initial_state(ffs_trial_arg_t * trial, ffs_state_t * state,
			    ranlcg_t * ran) {
  int errnol = 0;
  int seed;
  MPI_Comm comm;

  dbg_return_if(trial == NULL, -1);
  dbg_return_if(state == NULL, -1);
  dbg_return_if(ran == NULL, -1);

  proxy_comm(trial->proxy, &comm);

  errnol = proxy_state(trial->proxy, SIM_STATE_READ, ffs_state_stub(state));
  mpi_err_if_any(errnol, comm);

  ranlcg_reep_int32(ran, &seed);
  proxy_cache_info_int(trial->proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
  proxy_info(trial->proxy, FFS_INFO_RNG_SEED_FETCH);

  return 0;

 err:

  mpilog(trial->log, "Problem reading trial initial state\n");

  return -1;
}



/*****************************************************************************
 *
 *  ffs_trial_init
 *
 *  Generate an initial configuration at lambda_a
 *
 *****************************************************************************/

int ffs_trial_init(ffs_trial_arg_t * trial, ffs_state_t * sinit,
		   ranlcg_t * rantraj, int nlocaltraj, int itraj,
		   int * status) {

  ffs_t * ffs = NULL;
  MPI_Comm comm;

  int n, ncross;
  int iovershot;
  int icrossed;
  int mpi_errnol = 0;
  double t0, t1, t_elapsed;
  double lambda_a, lambda_b;
  double lambda, lambda_old;
  double random;

  int not_started;
  int init_independent;
  int init_nstepmax;
  double init_prob_accept;
  double teq;

  dbg_return_if(trial == NULL, -1);
  dbg_return_if(sinit == NULL, -1);
  dbg_return_if(rantraj == NULL, -1);
  dbg_return_if(status == NULL, -1);

  ffs_init_independent(trial->init, &init_independent);
  ffs_init_nstepmax(trial->init, &init_nstepmax);
  ffs_init_prob_accept(trial->init, &init_prob_accept);
  ffs_init_teq(trial->init, &teq);

  ffs_param_lambda_a(trial->param, &lambda_a);
  ffs_param_lambda_b(trial->param, &lambda_b);

  dbg_err_if( proxy_ffs(trial->proxy, &ffs) );
  ffs_comm(ffs, &comm);

  mpi_errnol = proxy_lambda(trial->proxy);
  mpi_err_if_any(mpi_errnol, comm);

  ffs_lambda(ffs, &lambda_old);

  /* Equilibrate */

  if (itraj == 1 || init_independent) {
    dbg_err_if( ffs_trial_initial_state(trial, sinit, rantraj) );
  }

  proxy_lambda(trial->proxy);
  ffs_lambda(ffs, &lambda_old);

  t_elapsed = 0.0;
  not_started = 1;
  proxy_info(trial->proxy, FFS_INFO_TIME_PUT);
  ffs_time(ffs, &t0);

  for (n = 0; n < init_nstepmax; n++) {

    proxy_execute(trial->proxy, SIM_EXECUTE_RUN);
    proxy_lambda(trial->proxy);
    ffs_lambda(ffs, &lambda);
    ffs_time(ffs, &t1);

    /* Overshot to state B? Come back to initial state. */

    iovershot = (lambda >= lambda_b);
    MPI_Bcast(&iovershot, 1, MPI_INT, 0, comm);

    if (iovershot) {

      if (not_started == 0) {
        t_elapsed += (t1 - t0); /* Time spent between A and B is counted */
      }

      dbg_err_if( ffs_trial_initial_state(trial, sinit, rantraj) );

      ffs_time(ffs, &t0);
      proxy_lambda(trial->proxy);
      ffs_lambda(ffs, &lambda_old);
      lambda = lambda_old;
      not_started = 1;
    }

    /* Do we have a forward crossing of first interface? */
    /* At this point only root has lambda, so broadcast the result. */

    icrossed = (lambda_old < lambda_a) && (lambda >= lambda_a);
    MPI_Bcast(&icrossed, 1, MPI_INT, 0, comm);

    if (icrossed && t1 >= teq) {

      if (not_started) {
	not_started = 0;
	t0 = t1;
      }
      else {

	ffs_result_aflux_ncross_add(trial->flux);
	ffs_result_aflux_ncross_local(trial->flux, &ncross);

	ffs_time(ffs, &t1);
	t_elapsed += (t1 - t0);
	t0 = t1;

	/* Accept (i.e., break) if... */

	ranlcg_reep(rantraj, &random);
	if (random < init_prob_accept) break;
      }
    }

    lambda_old = lambda;
  }

  *status = FFS_TRIAL_SUCCEEDED;
  if (n >= init_nstepmax) *status = FFS_TRIAL_TIMED_OUT;

  dbg_err_if( ffs_result_aflux_status_set(trial->flux, nlocaltraj, *status) );
  dbg_err_if( ffs_result_aflux_time_set(trial->flux, nlocaltraj, t_elapsed) );

  return 0;

 err:

  mpilog(trial->log, "Problem in generation of initial state\n");

  return -1;
}
