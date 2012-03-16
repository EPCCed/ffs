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

int u_test_ranlcg_create(u_test_case_t * tc) {

  ranlcg_t * p = NULL;

  u_test_err_if(ranlcg_create(-1, &p) == 0);
  u_test_err_if(ranlcg_create(1, NULL) == 0);

  return U_TEST_SUCCESS;

 err:
  if (p) ranlcg_free(p);
  return U_TEST_FAILURE;
}

int u_test_ranlcg_state(u_test_case_t * tc) {

  long state_set;
  long state;
  ranlcg_t * p = NULL;

  state_set = 0; /* state <= 0 not allowed */
  u_test_err_if(ranlcg_state_set(p, state_set) == 0);
  u_test_err_if(ranlcg_state(p, &state) == 0);

  state = LONG_MAX; /* state > deafult m not allowed */
  u_test_err_if(ranlcg_state_set(p, state) == 0);

  state_set = 1; /* good state */
  u_test_err_if(ranlcg_create(state_set, &p));
  u_test_err_if(ranlcg_state(p, &state));
  u_test_err_if(state != state_set);

  state_set = 2; /* ok */
  u_test_err_if(ranlcg_state_set(p, state_set));
  u_test_err_if(ranlcg_state(p, &state));
  u_test_err_if(state != state_set);

  ranlcg_free(p);
  return U_TEST_SUCCESS;

 err:
  if (p) ranlcg_free(p);
  return U_TEST_FAILURE;
}
