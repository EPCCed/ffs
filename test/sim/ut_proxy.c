/*****************************************************************************
 *
 *  ut_proxy.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "proxy.h"
#include "ut_proxy.h"

/*****************************************************************************
 *
 *  ut_proxy
 *
 *****************************************************************************/

int ut_proxy(u_test_case_t * tc) {

  proxy_t * proxy = NULL;
  ffs_t * ffs = NULL;

  int result;
  MPI_Comm testcomm;

  u_dbg("Start");

  u_test_err_if(proxy_create(MPI_COMM_WORLD, &proxy));
  u_test_err_if(proxy_delegate_create(proxy, "test"));

  u_test_err_if(proxy_execute(proxy, SIM_EXECUTE_INIT));
  u_test_err_if(proxy_state(proxy, SIM_STATE_INIT, "no stub"));
  u_test_err_if(proxy_lambda(proxy));
  u_test_err_if(proxy_info(proxy, FFS_INFO_TIME_PUT));

  u_test_err_if(proxy_delegate_free(proxy));

  u_test_err_if(proxy_ffs(proxy, &ffs));
  u_test_err_if(ffs == NULL);

  /* The simulation communicator is a duplicate of the proxy
   * communicator. Result of comparison should be MPI_CONGRUENT */

  u_test_err_if(ffs_comm(ffs, &testcomm));
  MPI_Comm_compare(MPI_COMM_WORLD, testcomm, &result);
  u_test_err_if(result != MPI_CONGRUENT);

  proxy_free(proxy);

  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (proxy) proxy_free(proxy);

  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}
