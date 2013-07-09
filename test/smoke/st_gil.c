/*****************************************************************************
 *
 *  st_gil.c
 *
 *  Smoke tests for the Discrete Monte Carlo (Gillespie) coupler.
 *
 *****************************************************************************/

#include <float.h>

#include "u/libu.h"
#include "ffs_control.h"
#include "ffs_util.h"

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

  double f1, pab;
  ffs_result_summary_t * result = NULL;
  ffs_control_t * ffs = NULL;

  u_dbg("Start");
  dbg_err_if( ffs_result_summary_create(&result) );

  /* Smoke test 1 */

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log1) );
  dbg_err_if( ffs_control_execute(ffs, input1) );
  dbg_err_if( ffs_control_stop(ffs, result) );

  ffs_control_free(ffs);
  ffs = NULL;

  /* These are the results expected */
  dbg_err_if( ffs_result_summary_stat(result, &f1, &pab) );
  dbg_err_if( util_compare_double(f1,  1.000568e-02, FLT_EPSILON) );
  dbg_err_if( util_compare_double(pab, 8.489543e-03, FLT_EPSILON) );


  /* Smoke test 2 */

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log2) );
  dbg_err_if( ffs_control_execute(ffs, input2) );
  dbg_err_if( ffs_control_stop(ffs, result) );

  ffs_control_free(ffs);
  ffs = NULL;

  dbg_err_if( ffs_result_summary_stat(result, &f1, &pab) );
  dbg_err_if( util_compare_double(f1,  2.000004e-02, FLT_EPSILON) );
  dbg_err_if( util_compare_double(pab, 3.574820e-04, FLT_EPSILON) );

  ffs_result_summary_free(result);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (result) ffs_result_summary_free(result);
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

  double f1, pab;
  ffs_result_summary_t * result = NULL;
  ffs_control_t * ffs = NULL;

  u_dbg("Start");
  dbg_err_if( ffs_result_summary_create(&result) );

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log1) );
  dbg_err_if( ffs_control_execute(ffs, input1) );
  dbg_err_if( ffs_control_stop(ffs, result) );

  ffs_control_free(ffs);
  ffs = NULL;

  dbg_err_if( ffs_result_summary_stat(result, &f1, &pab) );
  dbg_err_if( util_compare_double(f1,  1.691977e-02, FLT_EPSILON) );
  dbg_err_if( util_compare_double(pab, 2.310262e-04, FLT_EPSILON) );

  ffs_result_summary_free(result);
  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (result) ffs_result_summary_free(result);
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

  double f1, pab;
  ffs_result_summary_t * result = NULL;
  ffs_control_t * ffs = NULL;

  u_dbg("Start");
  dbg_err_if( ffs_result_summary_create(&result) );

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log1) );
  dbg_err_if( ffs_control_execute(ffs, input1) );
  dbg_err_if( ffs_control_stop(ffs, result) );

  ffs_control_free(ffs);
  ffs = NULL;

  dbg_err_if( ffs_result_summary_stat(result, &f1, &pab) );
  dbg_err_if( util_compare_double(f1,  1.461348e-02, FLT_EPSILON) );
  dbg_err_if( util_compare_double(pab, 2.864147e-03, FLT_EPSILON) );

  dbg_err_if( ffs_control_create(MPI_COMM_WORLD, &ffs) );
  dbg_err_if( ffs_control_start(ffs, log2) );
  dbg_err_if( ffs_control_execute(ffs, input2) );
  dbg_err_if( ffs_control_stop(ffs, result) );

  ffs_control_free(ffs);
  ffs = NULL;

  dbg_err_if( ffs_result_summary_stat(result, &f1, &pab) );
  dbg_err_if( util_compare_double(f1,  1.169919e-02, FLT_EPSILON) );
  dbg_err_if( util_compare_double(pab, 4.230173e-02, FLT_EPSILON) );

  ffs_result_summary_free(result);
  u_dbg("Success\n");

  return U_TEST_SUCCESS;

 err:

  if (result) ffs_result_summary_free(result);
  if (ffs) ffs_control_free(ffs);
  u_dbg("Failure\n");

  return U_TEST_FAILURE;
}
