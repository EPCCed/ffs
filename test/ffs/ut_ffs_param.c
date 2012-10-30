/*****************************************************************************
 *
 *  ffs_param.c
 *
 *  Unit test.
 *
 *****************************************************************************/

#include <float.h>
#include <string.h>

#include <mpi.h>

#include "u/libu.h"
#include "mpilog.h"
#include "ffs_util.h"
#include "ffs_param.h"

/*****************************************************************************
 *
 *  ut_param_create
 *
 *****************************************************************************/

int ut_param_create(u_test_case_t * tc) {

  u_config_t * config = NULL;
  u_config_t * interf = NULL;
  ffs_param_t * param = NULL;
  int nlambda;
  int ntrial;
  int nstate;
  int nskeep;
  double lambda;
  double pprune;

  /* Set up a config object with two interfaces. */

  u_dbg("Start");

  dbg_err_if(u_config_create(&config));
  dbg_err_if(u_config_add_key(config, FFS_CONFIG_NLAMBDA, "2"));
  dbg_err_if(u_config_add_key(config, FFS_CONFIG_NTRIAL_DEFAULT, "10"));
  dbg_err_if(u_config_add_key(config, FFS_CONFIG_NSTATE_DEFAULT, "11"));
  dbg_err_if(u_config_add_key(config, FFS_CONFIG_NSKEEP_DEFAULT, "12"));
  dbg_err_if(u_config_add_key(config, FFS_CONFIG_PPRUNE_DEFAULT, "1.0"));

  /* For interface 1, we use default parameters (except lambda) */

  dbg_err_if(u_config_add_child(config, "interface1", &interf));
  dbg_err_if(u_config_add_key(interf, FFS_CONFIG_LAMBDA, "0.1"));

  /* For interface 2, set all values explicitly */

  dbg_err_if(u_config_add_child(config, "interface2", &interf));
  dbg_err_if(u_config_add_key(interf, FFS_CONFIG_LAMBDA, "0.2"));
  dbg_err_if(u_config_add_key(interf, FFS_CONFIG_NTRIAL, "1000"));
  dbg_err_if(u_config_add_key(interf, FFS_CONFIG_NSTATE, "100"));
  dbg_err_if(u_config_add_key(interf, FFS_CONFIG_NSKEEP, "2"));
  dbg_err_if(u_config_add_key(interf, FFS_CONFIG_PPRUNE, "0.5"));

  /* Use this to create the ffs_param_t object, and check the two
   * are consistent */

  u_test_err_if(ffs_param_create(config, &param));

  nlambda = -1;
  u_test_err_if(ffs_param_nlambda(param, &nlambda));
  u_test_err_if(nlambda != 2);

  /* Interface 1 (has default values) */

  lambda = 0.0;
  u_test_err_if(ffs_param_lambda(param, 1, &lambda));
  u_test_err_if(util_compare_double(lambda, 0.1, DBL_EPSILON));

  ntrial = 0;
  u_test_err_if(ffs_param_ntrial(param, 1, &ntrial));
  u_test_err_if(ntrial != 10);
  nstate = 0;
  u_test_err_if(ffs_param_nstate(param, 1, &nstate));
  u_test_err_if(nstate != 11);
  nskeep = 0;
  u_test_err_if(ffs_param_nskeep(param, 1, &nskeep));
  u_test_err_if(nskeep != 12);
  pprune = 0.0;
  u_test_err_if(ffs_param_pprune(param, 1, &pprune));
  u_test_err_if(util_compare_double(pprune, 1.0, DBL_EPSILON));

  /* Interface 2 */

  lambda = 0.0;
  u_test_err_if(ffs_param_lambda(param, 2, &lambda));
  u_test_err_if(util_compare_double(lambda, 0.2, DBL_EPSILON));

  u_test_err_if(ffs_param_ntrial(param, 2, &ntrial));
  u_test_err_if(ntrial != 1000);
  u_test_err_if(ffs_param_nstate(param, 2, &nstate));
  u_test_err_if(nstate != 100);
  u_test_err_if(ffs_param_nskeep(param, 2, &nskeep));
  u_test_err_if(nskeep != 2);
  u_test_err_if(ffs_param_pprune(param, 2, &pprune));
  u_test_err_if(util_compare_double(pprune, 0.5, DBL_EPSILON));

  /* Interface 0 (lambda = interface 1) */

  lambda = 0.0;
  u_test_err_if(ffs_param_lambda(param, 0, &lambda));
  u_test_err_if(util_compare_double(lambda, 0.1, DBL_EPSILON));

  u_test_err_if(ffs_param_check(param));

  /* Finish */

  u_config_free(config);
  ffs_param_free(param);

  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:
  if (config) u_config_free(config);
  if (param) ffs_param_free(param);
  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_param_from_file
 *
 *****************************************************************************/

int ut_param_from_file(u_test_case_t * tc) {

  ffs_param_t * param = NULL;
  u_config_t * config = NULL;
  u_config_t * subconfig = NULL;
  mpilog_t * log = NULL;

  u_dbg("Start");
  u_test_err_if(u_config_load_from_file("inputs/ut_param1.inp", &config));

  /* The ffs_param_t could take a full interfaces{}, but ... */
  subconfig = u_config_get_child(config, FFS_CONFIG_INTERFACES);

  u_test_err_if(ffs_param_create(subconfig, &param));
  u_test_err_if(ffs_param_check(param));

  u_test_err_if(mpilog_create(MPI_COMM_WORLD, &log));
  u_test_err_if(mpilog_fopen(log, "logs/ut_param.log", "w+"));
  u_test_err_if(ffs_param_log_to_mpilog(param, log));

  mpilog_fclose(log);
  mpilog_free(log);
  ffs_param_free(param);
  u_config_free(config);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  if (config) u_config_free(config);
  if (param) ffs_param_free(param);
  if (log) mpilog_free(log);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
