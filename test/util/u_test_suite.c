/*****************************************************************************
 *
 *  \file u_test_suite.c
 *
 *  Unit test suite for this directory.
 *
 *****************************************************************************/

#include <stdio.h>
#include <limits.h>

#include "u/libu.h"

int u_test_ranlcg_create(u_test_case_t * tc);
int u_test_ranlcg_state(u_test_case_t * tc);

#include "ut_util.h"

/*
 * Suite
 */

int u_test_suite_util_register (u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("util", &ts);

  u_test_case_register("ranlcg_create", u_test_ranlcg_create, ts);
  u_test_case_register("ranlcg_state", u_test_ranlcg_state, ts);

  u_test_case_register(UT_UTIL_MISC_NAME, ut_util_misc, ts);
  u_test_case_register(UT_UTIL_CONFIG_NAME, ut_util_config, ts);
  u_test_case_depends_on(UT_UTIL_CONFIG_NAME, UT_UTIL_MISC_NAME, ts);

  return u_test_suite_add(ts, t);
}
