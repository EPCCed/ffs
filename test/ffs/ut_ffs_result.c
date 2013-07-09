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
  int n;
  double wt;

  ffs_result_t * result = NULL;

  u_dbg("Start");

  dbg_err_if(ffs_result_create(nlambda, &result));
  dbg_err_if(result == NULL);

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
