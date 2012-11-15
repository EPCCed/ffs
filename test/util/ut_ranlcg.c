/*****************************************************************************
 *
 *  ranlcg.c
 *
 *  Unit test for util/ranlcg.c
 *
 *****************************************************************************/

#include <stdio.h>
#include <limits.h>

#include "u/libu.h"
#include "ranlcg.h"
#include "ut_ranlcg.h"

/*****************************************************************************
 *
 *  ut_ranlcg_create
 *
 *****************************************************************************/

int ut_ranlcg_create(u_test_case_t * tc) {

  ranlcg_t * p = NULL;

  u_dbg("Start");

  dbg_err_if(ranlcg_create(1, &p));
  ranlcg_free(p);
  p = NULL;

  dbg_err_if(ranlcg_create32(1, &p));
  ranlcg_free(p);
  p = NULL;

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  if (p) ranlcg_free(p);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_ranlcg_state
 *
 *****************************************************************************/

int ut_ranlcg_state(u_test_case_t * tc) {

  int seed32 = 1;
  int reep32;
  long state_set;
  long state;
  ranlcg_t * p = NULL;

  u_dbg("Start");

  dbg_err_if(ranlcg_create32(seed32, &p));
  dbg_err_if(ranlcg_reep_int32(p, &reep32));
  ranlcg_free(p);
  p = NULL;

  state_set = 1;
  dbg_err_if(ranlcg_create(state_set, &p));
  dbg_err_if(ranlcg_state(p, &state));
  dbg_err_if(state != state_set);

  state_set = 2;
  dbg_err_if(ranlcg_state_set(p, state_set));
  dbg_err_if(ranlcg_state(p, &state));
  dbg_err_if(state != state_set);

  ranlcg_free(p);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (p) ranlcg_free(p);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
