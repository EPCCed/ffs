/*****************************************************************************
 *
 *  ut_sim_test.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "ffs.h"
#include "sim_test.h"
#include "ut_sim_test.h"

/*****************************************************************************
 *
 *  ut_sim_test
 *
 *  The ffs_t object can be NULL, and the test is a complete fake.
 *
 *****************************************************************************/

int ut_sim_test(u_test_case_t * tc) {

  interface_t table;
  sim_test_t * sim = NULL;

  u_dbg("Start");

  /* Test the table function returns something, but no further test
   * of what that something is is done. See proxy test. */

  dbg_err_if(sim_test_table(&table));

  dbg_err_if(sim_test_create(&sim));

  dbg_err_if(sim_test_execute(sim, NULL, SIM_EXECUTE_INIT));
  dbg_err_if(sim_test_execute(sim, NULL, SIM_EXECUTE_RUN));
  dbg_err_if(sim_test_execute(sim, NULL, SIM_EXECUTE_FINISH));

  dbg_err_if(sim_test_state(sim, NULL, SIM_STATE_INIT, "no stub"));
  dbg_err_if(sim_test_state(sim, NULL, SIM_STATE_READ, "no stub"));
  dbg_err_if(sim_test_state(sim, NULL, SIM_STATE_WRITE, "no stub"));
  dbg_err_if(sim_test_state(sim, NULL, SIM_STATE_DELETE, "no stub"));

  dbg_err_if(sim_test_lambda(sim, NULL));
  dbg_err_if(sim_test_info(sim, NULL, 0));

  sim_test_free(sim);

  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (sim) sim_test_free(sim);

  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}
