/*****************************************************************************
 *
 *  \file ranlcg.c
 *
 *  Unit test for ../../src/util/ranlcg.c
 *
 *****************************************************************************/

#include <stdio.h>
#include <limits.h>

#include "u/libu.h"

#include "ranlcg.h"
#include "ffs_error.h"

int test_ranlcg_create(u_test_case_t * tc) {

  ranlcg_t * p = NULL;

  u_test_err_if(ranlcg_create(-1, &p) == 0);
  u_test_err_if(ranlcg_create(1, NULL) == 0);

  return U_TEST_SUCCESS;

 err:
  if (p) ranlcg_free(p);
  return U_TEST_FAILURE;
}

int test_ranlcg_state(u_test_case_t * tc) {

  long state;
  ranlcg_t * p = NULL;

  state = 0; /* state <= 0 not allowed */
  u_test_err_if(ranlcg_state_set(p, state) == 0);
  u_test_err_if(ranlcg_state(p) != -1);

  state = LONG_MAX; /* state > deafult m not allowed */
  u_test_err_if(ranlcg_state_set(p, state) == 0);

  state = 1; /* good state */
  u_test_err_if(ranlcg_create(state, &p));
  u_test_err_if(ranlcg_state(p) != state);

  state = 2; /* ok */
  u_test_err_if(ranlcg_state_set(p, state));
  u_test_err_if(ranlcg_state(p) != state);

  ranlcg_free(p);
  return U_TEST_SUCCESS;

 err:
  if (p) ranlcg_free(p);
  return U_TEST_FAILURE;
}

int test_suite_ranlcg_register (u_test_t *t) {

  u_test_suite_t *ts;

  u_test_suite_new("ranlcg", &ts);
  u_test_case_register("ranlcg_create", test_ranlcg_create, ts);
  u_test_case_register("ranlcg_state", test_ranlcg_state, ts);

  return u_test_suite_add(ts, t);
}

int main (int argc, char *argv[]) {

  int ifail;
  u_test_t *t = NULL;

  ffs_error_init(0);

  u_test_new("FFS tests", &t);
  test_suite_ranlcg_register(t);

  ifail = u_test_run(argc, argv, t);

  return ifail;
}
