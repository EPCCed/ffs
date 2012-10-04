/*****************************************************************************
 *
 *  st_gil.c
 *
 *  Smoke test for the Gillespie coupler.
 *
 *****************************************************************************/

#include "u/libu.h"
#include "ffs_control.h"

int st_gil_create(u_test_case_t * tc) {

  ffs_control_t * ffs = NULL;

  u_dbg("Start\n");
  u_test_err_if(ffs_control_create(MPI_COMM_WORLD, &ffs));
  u_test_err_if(ffs_control_start(ffs, "logs/smoke_dmc_control.log", "w+"));
  u_test_err_if(ffs_control_execute(ffs, "inputs/dmc_smoke1.inp"));

  u_test_err_if(ffs_control_stop(ffs));
  ffs_control_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_control_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
