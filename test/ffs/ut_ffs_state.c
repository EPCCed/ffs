/*****************************************************************************
 *
 *  \file ffs_state.c
 *
 *  Unit test for ../../src/ffs/ffs_state.c
 *
 *****************************************************************************/

#include "u/libu.h"
#include "ffs_state.h"

int u_test_ffs_state_create(u_test_case_t * tc) {

  ffs_state_t * state = NULL;
  int id = 0;
  int * pidref = NULL;
  int * pid = NULL;

  u_dbg("Start");
  dbg_err_if(ffs_state_create(0, 0, &state));
  dbg_err_if(ffs_state_id_set(state, 10));
  dbg_err_if(ffs_state_id(state, &id));
  dbg_err_if(id != 10);

  dbg_err_if((pidref = u_calloc(1, sizeof(int))) == NULL);

  *pidref = 11;
  dbg_err_if(ffs_state_mem_set(state, (void *) pidref));
  dbg_err_if(ffs_state_mem(state, (void *) &pid));
  dbg_err_if(*pid != 11);
  dbg_err_if(ffs_state_stub(state) == NULL);

  u_free(pidref);
  ffs_state_free(state);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  if (pidref) u_free(pidref);
  if (state) ffs_state_free(state);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
