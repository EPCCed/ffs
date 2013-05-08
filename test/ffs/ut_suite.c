/*****************************************************************************
 *
 *  ut_suite.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include "u/libu.h"

#include "ut_ffs.h"
#include "ut_ffs_control.h"
#include "ut_ffs_init.h"
#include "ut_ffs_inst.h"
#include "ut_ffs_param.h"
#include "ut_ffs_result.h"

/*
 * Register the tests for ffs objects
 *
 */

int u_test_suite_ffs_register(u_test_t * t) {

  u_test_suite_t * ts = NULL;

  u_test_suite_new("ffs suite", &ts);

  u_test_case_register(UT_PARAM_CREATE_NAME, ut_param_create, ts);
  u_test_case_register(UT_PARAM_FROM_FILE_NAME, ut_param_from_file, ts);
  u_test_case_register(UT_PARAM_AUTO_NAME, ut_param_auto, ts);

  u_test_case_register(UT_FFS_CREATE_NAME, ut_ffs_create, ts);
  u_test_case_register(UT_FFS_COMMAND_LINE_NAME, ut_ffs_command_line, ts);
  u_test_case_register(UT_FFS_COMMAND_BUILD_NAME, ut_ffs_command_build, ts);
  u_test_case_register(UT_FFS_EXCH_INT_NAME, ut_ffs_exch_int, ts);
  u_test_case_register(UT_FFS_LAMBDA_NAME, ut_ffs_lambda_name, ts);

  u_test_case_register(UT_INST_NAME, ut_inst, ts);
  u_test_case_register(UT_CONTROL_NAME, ut_control, ts);

  u_test_case_register(UT_INIT_NAME, ut_init, ts);
  u_test_case_register(UT_RESULT_SERIAL_NAME, ut_result_serial, ts);

  return u_test_suite_add(ts, t);
}
