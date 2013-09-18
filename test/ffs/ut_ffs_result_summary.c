/*****************************************************************************
 *
 *  ut_ffs_result_summary.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012-2013 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <float.h>

#include "ffs_util.h"
#include "ffs_result_summary.h"
#include "ut_ffs_result_summary.h"

/*****************************************************************************
 *
 *  ut_ffs_result_summary
 *
 *****************************************************************************/

int ut_ffs_result_summary(u_test_case_t * tc) {

  ffs_result_summary_t * sum = NULL;
  int rank, rank_ref;
  int inst = 0, inst_ref = 1;
  double f1 = 0.0,  f1_ref = -1.0;
  double pab = 0.0, pab_ref = 0.5;

  u_dbg("Start");

  MPI_Comm_rank(MPI_COMM_WORLD, &rank_ref);

  dbg_err_if( ffs_result_summary_create(&sum) );
  dbg_err_if( ffs_result_summary_stat_set(sum, f1_ref, pab_ref) );
  dbg_err_if( ffs_result_summary_stat(sum, &f1, &pab) );
  dbg_err_if( util_compare_double(f1, f1_ref, DBL_EPSILON) );
  dbg_err_if( util_compare_double(pab, pab_ref, DBL_EPSILON) );

  dbg_err_if( ffs_result_summary_rank_set(sum, rank_ref) );
  dbg_err_if( ffs_result_summary_rank(sum, &rank) );
  dbg_err_if( rank != rank_ref );

  dbg_err_if( ffs_result_summary_inst_set(sum, inst_ref) );
  dbg_err_if( ffs_result_summary_inst(sum, &inst) );
  dbg_err_if( inst != inst_ref );

  ffs_result_summary_free(sum);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  u_dbg("Failure\n");
  if (sum) ffs_result_summary_free(sum);

  return U_TEST_FAILURE;
}
