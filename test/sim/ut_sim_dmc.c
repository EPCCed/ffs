/*****************************************************************************
 *
 *  ut_sim_dmc.c
 *
 *****************************************************************************/

#include <float.h>

#include "ffs_private.h"
#include "ffs_util.h"
#include "proxy.h"
#include "sim_dmc.h"
#include "ut_sim_dmc.h"

static char * input = 
  "inputs/dmc_switch1_comp.dat inputs/dmc_switch1_react.dat";
static char * stub = "logs/dmc_state.dat";

/*****************************************************************************
 *
 *  ut_sim_dmc
 *
 *  This is a test of the bare interface; test via the proxy is below.
 *
 *****************************************************************************/

int ut_sim_dmc(u_test_case_t * tc) {

  sim_dmc_t * dmc = NULL;
  interface_t table;

  u_dbg("Start");

  dbg_err_if(sim_dmc_table(&table));
  dbg_err_if(sim_dmc_create(&dmc));
  dbg_if(dmc == NULL);

  dbg_err_if(sim_dmc_free(dmc));

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (dmc) sim_dmc_free(dmc);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_sim_dmc_proxy
 *
 *  We need to split MPI_COMM_WORLD into individual ranks to ensure the
 *  simulation is serial.
 *
 *****************************************************************************/

int ut_sim_dmc_proxy(u_test_case_t * tc) {

  ffs_t * ffs = NULL;
  proxy_t * proxy = NULL;

  int rank = 0;
  char filename[BUFSIZ];
  MPI_Comm comm = MPI_COMM_NULL;

  u_dbg("Start");

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_split(MPI_COMM_WORLD, rank, 0, &comm);
  sprintf(filename, "%s-%d", stub, rank);

  dbg_err_if(proxy_create(rank, comm, &proxy));
  dbg_err_if(proxy_delegate_create(proxy, "dmc"));

  dbg_err_if(proxy_ffs(proxy, &ffs));
  dbg_err_if(ffs_command_line_set(ffs, input));
  dbg_err_if(proxy_execute(proxy, SIM_EXECUTE_INIT));

  dbg_err_if(proxy_state(proxy, SIM_STATE_INIT, filename));
  dbg_err_if(proxy_state(proxy, SIM_STATE_WRITE, filename));
  dbg_err_if(proxy_state(proxy, SIM_STATE_READ, filename));
  dbg_err_if(proxy_state(proxy, SIM_STATE_DELETE, filename));

  dbg_err_if(proxy_lambda(proxy));

  dbg_err_if(proxy_execute(proxy, SIM_EXECUTE_FINISH));

  dbg_err_if(proxy_delegate_free(proxy));
  proxy_free(proxy);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (proxy) proxy_free(proxy);
  MPI_Comm_free(&comm);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_sim_dmc_info
 *
 *****************************************************************************/

int ut_sim_dmc_info(u_test_case_t * tc) {


  ffs_t * ffs = NULL;
  proxy_t * proxy = NULL;

  int rank = 0;
  MPI_Comm comm = MPI_COMM_NULL;

  ffs_var_enum_t type;
  int n;
  int sref, s;
  double tref, t;

  u_dbg("Start");

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_split(MPI_COMM_WORLD, rank, 0, &comm);

  dbg_err_if(proxy_create(rank, comm, &proxy));
  dbg_err_if(proxy_delegate_create(proxy, "dmc"));

  dbg_err_if(proxy_ffs(proxy, &ffs));
  dbg_err_if(ffs_command_line_set(ffs, input));
  dbg_err_if(proxy_execute(proxy, SIM_EXECUTE_INIT));

  /* We should have declared time as double, and lamnda as integer ... */

  dbg_err_if(ffs_type(ffs, FFS_INFO_TIME_PUT, &n, &type));
  dbg_err_if(n != 1);
  dbg_err_if(type != FFS_VAR_DOUBLE);

  dbg_err_if(ffs_type(ffs, FFS_INFO_LAMBDA_PUT, &n, &type));
  dbg_err_if(n != 1);
  dbg_err_if(type != FFS_VAR_INT);

  /* Look at time, lambda, seed ... */

  tref = -1.0;
  dbg_err_if(ffs_info_double(ffs, FFS_INFO_TIME_PUT, 1, &tref));
  dbg_err_if(proxy_info(proxy, FFS_INFO_TIME_PUT));
  dbg_err_if(ffs_info_double(ffs, FFS_INFO_TIME_FETCH, 1, &t));
  dbg_err_if(util_compare_double(t, tref, DBL_EPSILON) == 0);

  sref = -1;
  dbg_err_if(ffs_info_int(ffs, FFS_INFO_LAMBDA_PUT, 1, &sref));
  dbg_err_if(proxy_info(proxy, FFS_INFO_LAMBDA_PUT));
  dbg_err_if(ffs_info_int(ffs, FFS_INFO_LAMBDA_FETCH, 1, &s));
  dbg_err_if(s == sref);

  sref = 1;
  dbg_err_if(ffs_info_int(ffs, FFS_INFO_RNG_SEED_PUT, 1, &sref));
  dbg_err_if(proxy_info(proxy, FFS_INFO_RNG_SEED_FETCH));

  dbg_err_if(proxy_execute(proxy, SIM_EXECUTE_FINISH));
  dbg_err_if(proxy_delegate_free(proxy));
  proxy_free(proxy);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
