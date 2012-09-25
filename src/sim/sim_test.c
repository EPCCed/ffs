/*****************************************************************************
 *
 *  sim_test.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "interface.h"
#include "ffs.h"
#include "sim_test.h"

struct sim_test_s {
  int data;
};

const interface_t sim_test_interface = {
  (interface_table_ft)   &sim_test_table,
  (interface_create_ft)  &sim_test_create,
  (interface_free_ft)    &sim_test_free,
  (interface_execute_ft) &sim_test_execute,
  (interface_state_ft)   &sim_test_state,
  (interface_lambda_ft)  &sim_test_lambda,
  (interface_info_ft)    &sim_test_info
};

/*****************************************************************************
 *
 *  sim_test_table
 *
 *****************************************************************************/

int sim_test_table(interface_t * table) {

  *table = sim_test_interface;

  return 0;
}

/*****************************************************************************
 *
 *  sim_test_create
 *
 *****************************************************************************/

int sim_test_create(sim_test_t ** pobj) {

  sim_test_t * obj = NULL;

  obj = calloc(1, sizeof(sim_test_t));

  /* An error occured */
  if (obj == NULL) return -1;

  *pobj = obj;

  return 0;
}

/*****************************************************************************
 *
 *  sim_test_free
 *
 *****************************************************************************/

void sim_test_free(sim_test_t * obj) {

  free(obj);

  return;
}

/*****************************************************************************
 *
 *  sim_test_execute
 *
 *****************************************************************************/

int sim_test_execute(sim_test_t * obj, ffs_t * ffs,
		     sim_execute_enum_t action) {

  int ifail = 0;

  switch (action) {
  case SIM_EXECUTE_INIT:
    /* execute initialisation phase */
    break;
  case SIM_EXECUTE_RUN:
    /* execute a run */
    break;
  case SIM_EXECUTE_FINISH:
    /* execute the finalisation phase */
    obj->data = 0;
    break;
  default:
    /* Something went wrong? */
    ifail = -1;
  }

  return  ifail;
}

/*****************************************************************************
 *
 *  sim_test_state
 *
 *****************************************************************************/

int sim_test_state(sim_test_t * obj, ffs_t * ffs, sim_state_enum_t action,
		   const char * stub) {

  int ifail = 0;

  switch (action) {
  case SIM_STATE_INIT:
    /* initialise the model state, e.g., */
    obj->data = 1;
    break;
  case SIM_STATE_READ:
    /* create a file name from the stub, and read the data */
    break;
  case SIM_STATE_WRITE:
    /* create a file name from the stub, and write the data */
    break;
  case SIM_STATE_DELETE:
    /* create the file name form the stub, and delete the file */
    break;
  default:
    /* something went wrong? */
    ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  sim_test_lambda
 *
 *****************************************************************************/

int sim_test_lambda(sim_test_t * obj, ffs_t * ffs) {

  /* Compute lambda (rank 0) and set via ffs_info_int() */

  return 0;
}

/*****************************************************************************
 *
 *  sim_test_info
 *
 *****************************************************************************/

int sim_test_info (sim_test_t * obj, ffs_t * ffs, ffs_info_enum_t param) {

  /* Examine param, and put or get the appropriate information. */

  return 0;
}
