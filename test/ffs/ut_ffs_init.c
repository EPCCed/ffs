/*****************************************************************************
 *
 *  ut_ffs_init.c
 *
 *  Unit test.
 *
 *****************************************************************************/

#include <float.h>

#include "u/libu.h"
#include "mpilog.h"

#include "ffs_util.h"
#include "ffs_init.h"
#include "ut_ffs_init.h"

/*****************************************************************************
 *
 *  ut_init
 *
 *****************************************************************************/

int ut_init(u_test_case_t * tc) {

  int ival;
  double dval;

  mpilog_t * log = NULL;
  ffs_init_t * init = NULL;

  u_dbg("Start");

  dbg_err_if(mpilog_create(MPI_COMM_WORLD, &log));
  dbg_err_if(mpilog_fopen(log, "logs/unit-test-ffs-init.log", "w"));

  dbg_err_if(ffs_init_create(&init));

  dbg_err_if(ffs_init_independent_set(init, 1));
  dbg_err_if(ffs_init_independent(init, &ival));
  dbg_err_if(ival != 1);

  dbg_err_if(ffs_init_nstepmax_set(init, 2));
  dbg_err_if(ffs_init_nstepmax(init, &ival));
  dbg_err_if(ival != 2);

  dbg_err_if(ffs_init_nskip_set(init, 3));
  dbg_err_if(ffs_init_nskip(init, &ival));
  dbg_err_if(ival != 3);

  dbg_err_if(ffs_init_nsteplambda_set(init, 4));
  dbg_err_if(ffs_init_nsteplambda(init, &ival));
  dbg_err_if(ival != 4);

  dbg_err_if(ffs_init_prob_accept_set(init, 1.0));
  dbg_err_if(ffs_init_prob_accept(init, &dval));
  dbg_err_if(util_compare_double(1.0, dval, DBL_EPSILON));

  dbg_err_if(ffs_init_teq_set(init, 2.0));
  dbg_err_if(ffs_init_teq(init, &dval));
  dbg_err_if(util_compare_double(2.0, dval, DBL_EPSILON));

  dbg_err_if(ffs_init_log_to_mpilog(init, log));

  ffs_init_free(init);

  mpilog_fclose(log);
  mpilog_free(log);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  if (init) ffs_init_free(init);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
