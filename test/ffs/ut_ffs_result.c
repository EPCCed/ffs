/*****************************************************************************
 *
 *  ut_ffs_result.c
 *
 *****************************************************************************/

#include <float.h>

#include "u/libu.h"

#include "ffs_util.h"
#include "ffs_result.h"
#include "ut_ffs_result.h"

/*****************************************************************************
 *
 *  ut_result_serial
 *
 *****************************************************************************/

int ut_result_serial(u_test_case_t * tc) {

  int nlambda = 1;
  int ntrial = 2;
  int n;
  int ncross;
  int status;
  double t, wt;

  ffs_result_t * result = NULL;

  u_dbg("Start");

  dbg_err_if(ffs_result_create(nlambda, ntrial, &result));
  dbg_err_if(result == NULL);

  /* Number of crossings */
  dbg_err_if(ffs_result_ncross(result, &ncross));
  dbg_err_if(ncross != 0);
  dbg_err_if(ffs_result_ncross_accum(result, 1));
  dbg_err_if(ffs_result_ncross(result, &ncross));
  dbg_err_if(ncross != 1);

  for (n = 0; n < ntrial; n++) {

    /* status */
    dbg_err_if(ffs_result_status_set(result, n, n));
    dbg_err_if(ffs_result_status(result, n, &status));
    dbg_err_if(status != n);

    /* times */
    dbg_err_if(ffs_result_time_set(result, n, 100.0*n));
    dbg_err_if(ffs_result_time(result, n, &t));
    dbg_err_if(util_compare_double(t, 100.0*n, DBL_EPSILON));
  }

  for (n = 0; n < nlambda; n++) {

    /* weights */
    dbg_err_if(ffs_result_weight(result, n, &wt));
    dbg_err_if(util_compare_double(wt, 0.0, DBL_EPSILON));
    dbg_err_if(ffs_result_weight_accum(result, n, 1.0*n));
    wt = -1.0;
    dbg_err_if(ffs_result_weight(result, n, &wt));
    dbg_err_if(util_compare_double(wt, 1.0*n, DBL_EPSILON));

  }

  ffs_result_free(result);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
