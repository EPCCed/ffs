/*****************************************************************************
 *
 *  ffs_brute_force
 *
 *****************************************************************************/

#include "ffs_private.h"
#include "ffs_brute_force.h"

/*****************************************************************************
 *
 *  ffs_brute_force_run
 *
 *****************************************************************************/

int ffs_brute_force_run(ffs_trial_arg_t * trial) {

  int n;
  int mpi_errnol = 0;
  int seed;
  double t0, tmax;
  double lambda, lambda_old;
  double lambda_a, lambda_b;

  MPI_Comm comm;
  ffs_t * ffs = NULL;

  dbg_return_if(trial == NULL, -1);

  proxy_comm(trial->proxy, &comm);
  proxy_ffs(trial->proxy, &ffs);

  ffs_param_lambda_a(trial->param, &lambda_a);
  ffs_param_lambda_b(trial->param, &lambda_b);

  mpilog(trial->log, "Starting brute force simulation\n");

  mpi_errnol = proxy_execute(trial->proxy, SIM_EXECUTE_INIT);

  if (mpi_errnol) {
    mpilog_all(trial->log, "SIM_EXECUTE_INIT failed\n");
  }

  /* We cannot continue if the simulation doesn't execute */
  /* All instances terminate */
  mpi_err_if_any(mpi_errnol, comm);


  /* Set the simulation random seed */

  seed = trial->inst_seed;
  proxy_cache_info_int(trial->proxy, FFS_INFO_RNG_SEED_PUT, 1, &seed);
  proxy_info(trial->proxy, FFS_INFO_RNG_SEED_FETCH);

  /* We test here, once, the lambda computation. */
  /* All instances terminate on a failure. */

  mpi_errnol += proxy_lambda(trial->proxy);
  mpi_errnol += ffs_time(ffs, &t0);
  mpi_errnol += ffs_lambda(ffs, &lambda_old);

  if (mpi_errnol) {
    mpilog_all(trial->log, "Failed to compute simulation lambda\n");
  }

  mpi_err_if_any(mpi_errnol, comm);

  /* OK, we can start the time loop, and just keep going until the
   * time is up. */

  tmax = 1000.0;

  do {
    int was_a, was_b, was_t, now_a, now_b, now_t;
    double ta0, tb0;

    for (n = 0; n < trial->nsteplambda; n++) {
      proxy_execute(trial->proxy, SIM_EXECUTE_RUN);
    }
    proxy_lambda(trial->proxy);
    ffs_lambda(ffs, &lambda);
    ffs_time(ffs, &t0);

    /* Events */

    was_a = (lambda_old < lambda_a);
    was_b = (lambda_old > lambda_b);
    was_t = 1 - was_a - was_b;

    now_a = (lambda < lambda_a);
    now_b = (lambda > lambda_b);
    now_t = 1 - now_a - now_b;

    if ((was_a || was_t) && now_b) tb0 = t0;
    if (was_b && (now_t || now_a)) printf("In B %23.16e %23.16e\n", tb0, t0);

    if ((was_b || was_t) && now_a) ta0 = t0;
    if (was_a && (now_t || now_b)) printf("In A %23.16e %23.16e\n", ta0, t0);

    lambda_old = lambda;

  } while (t0 < tmax);

  proxy_execute(trial->proxy, SIM_EXECUTE_FINISH);

  return 0;

 err:

  mpilog(trial->log, "An error occured running the brute force simulation\n");

  return -1;
}
