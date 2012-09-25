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

  u_test_case_printf(tc, "%s", "Start");
  u_test_err_if(ffs_control_create(MPI_COMM_WORLD, &ffs));
  /*
  u_test_err_if(ffs_control_init(ffs, "inputs/ffs1.inp"));
  u_test_err_if(ffs_control_run(ffs));
  */

  ffs_control_free(ffs);
  u_test_case_printf(tc, "%s", "Success");

  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_control_free(ffs);

  return U_TEST_FAILURE;
}
