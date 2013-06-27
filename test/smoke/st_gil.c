/*****************************************************************************
 *
 *  st_gil.c
 *
 *  Smoke tests for the Discrete Monte Carlo (Gillespie) coupler.
 *  There is no mechanism yet to compare the outputs with a known
 *  result.
 *
 *****************************************************************************/

#include "u/libu.h"
#include "ffs_control.h"

/*****************************************************************************
 *
 *  st_dmc_branched
 *
 *****************************************************************************/

int st_dmc_branched(u_test_case_t * tc) {

  const char * input1 = "inputs/dmc_smoke1.inp";
  const char * input2 = "inputs/dmc_smoke2.inp";
  const char * log1   = "logs/dmc-smoke1";
  const char * log2   = "logs/dmc-smoke2";

  ffs_control_t * ffs = NULL;

  u_dbg("Start");

  /* Smoke test 1 */

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log1) );
  dbg_err_if( ffs_control_execute(ffs, input1) );
  dbg_err_if( ffs_control_stop(ffs) );

  ffs_control_free(ffs);

  /* Smoke test 2 */

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log2) );
  dbg_err_if( ffs_control_execute(ffs, input2) );
  dbg_err_if( ffs_control_stop(ffs) );

  ffs_control_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_control_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  st_dmc_direct
 *
 *****************************************************************************/

int st_dmc_direct(u_test_case_t * tc) {

  const char * input1 = "inputs/dmc_smoke3.inp";
  const char * log1   = "logs/dmc-smoke3";

  ffs_control_t * ffs = NULL;

  u_dbg("Start");

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log1) );
  dbg_err_if( ffs_control_execute(ffs, input1) );
  dbg_err_if( ffs_control_stop(ffs) );

  ffs_control_free(ffs);

  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (ffs) ffs_control_free(ffs);
  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  st_dmc_rosenbluth
 *
 *****************************************************************************/

int st_dmc_rosenbluth(u_test_case_t * tc) {

  const char * input1 = "inputs/dmc_smoke5.inp";
  const char * input2 = "inputs/dmc_smoke6.inp";
  const char * log1   = "logs/dmc-smoke5";
  const char * log2   = "logs/dmc-smoke6";

  ffs_control_t * ffs = NULL;

  u_dbg("Start");

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log1) );
  dbg_err_if( ffs_control_execute(ffs, input1) );
  dbg_err_if( ffs_control_stop(ffs) );

  ffs_control_free(ffs);

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log2) );
  dbg_err_if( ffs_control_execute(ffs, input2) );
  dbg_err_if( ffs_control_stop(ffs) );

  ffs_control_free(ffs);

  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (ffs) ffs_control_free(ffs);
  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}
