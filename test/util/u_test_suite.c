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
int u_test_extra_misc(u_test_case_t * tc);
int u_test_extra_config(u_test_case_t * tc);

/*
 * Suite
 */

int u_test_suite_util_register (u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("util", &ts);

  u_test_case_register("ranlcg_create", u_test_ranlcg_create, ts);
  u_test_case_register("ranlcg_state", u_test_ranlcg_state, ts);

  u_test_case_register("u extra misc", u_test_extra_misc, ts);
  u_test_case_register("u extra config", u_test_extra_config, ts);
  u_test_case_depends_on("u extra config", "u extra misc", ts);

  return u_test_suite_add(ts, t);
}
