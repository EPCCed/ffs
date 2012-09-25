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

  proxy_free(proxy);

  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (proxy) proxy_free(proxy);

  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}
