/*****************************************************************************
 *
 *  ut_suite.c
 *
 *  Unit test suite for utils.
 *
 *****************************************************************************/

#include <stdio.h>
#include <limits.h>

#include "u/libu.h"
#include "ut_ranlcg.h"
#include "ut_util.h"

/*****************************************************************************
 *
 *  u_test_suite_util_register
 *
 *****************************************************************************/

int u_test_suite_util_register (u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("util", &ts);

  u_test_case_register(UT_RANLCG_CREATE_NAME, ut_ranlcg_create, ts);
  u_test_case_register(UT_RANLCG_STATE_NAME, ut_ranlcg_state, ts);

  u_test_case_register(UT_UTIL_MISC_NAME, ut_util_misc, ts);
  u_test_case_register(UT_UTIL_CONFIG_NAME, ut_util_config, ts);
  u_test_case_depends_on(UT_UTIL_CONFIG_NAME, UT_UTIL_MISC_NAME, ts);

  return u_test_suite_add(ts, t);
}
