/*****************************************************************************
 *
 *  ut_sim_dmc.c
 *
 *****************************************************************************/

#include "ffs_private.h"
#include "proxy.h"
#include "sim_dmc.h"
#include "ut_sim_dmc.h"

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

  u_dbg("Start\n");

  u_test_err_if(sim_dmc_table(&table));
  u_test_err_if(sim_dmc_create(&dmc));
  u_test_err_if(dmc == NULL);

  u_test_err_if(sim_dmc_free(dmc));

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
  MPI_Comm comm = MPI_COMM_NULL;

  char * input = 
    "inputs/dmc_switch1_comp.dat inputs/dmc_switch1_react.dat";

  u_dbg("Start\n");

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_split(MPI_COMM_WORLD, rank, 0, &comm);

  u_test_err_if(proxy_create(comm, &proxy));
  u_test_err_if(proxy_delegate_create(proxy, "dmc"));

  u_test_err_if(proxy_ffs(proxy, &ffs));
  u_test_err_if(ffs_command_line_set(ffs, input));
  u_test_err_if(proxy_execute(proxy, SIM_EXECUTE_INIT));

  u_test_err_if(proxy_state(proxy, SIM_STATE_INIT, "logs/blah"));
  u_test_err_if(proxy_state(proxy, SIM_STATE_WRITE, "logs/blah"));
  u_test_err_if(proxy_state(proxy, SIM_STATE_READ, "logs/blah"));
  u_test_err_if(proxy_state(proxy, SIM_STATE_DELETE, "logs/blah"));

  u_test_err_if(proxy_lambda(proxy));

  u_test_err_if(proxy_execute(proxy, SIM_EXECUTE_FINISH));

  u_test_err_if(proxy_delegate_free(proxy));
  proxy_free(proxy);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (proxy) proxy_free(proxy);
  MPI_Comm_free(&comm);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
