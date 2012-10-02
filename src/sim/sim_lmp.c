/*****************************************************************************
 *
 *  sim_lmp.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "interface.h"
#include "ffs.h"
#include "sim_lmp.h"

#include "library.h"

struct sim_lmp_s {
  void * lmp;
  /* and anything else relating to lammps */
};

const interface_t sim_lmp_interface = {
  (interface_table_ft)   &sim_lmp_table,
  (interface_create_ft)  &sim_lmp_create,
  (interface_free_ft)    &sim_lmp_free,
  (interface_execute_ft) &sim_lmp_execute,
  (interface_state_ft)   &sim_lmp_state,
  (interface_lambda_ft)  &sim_lmp_lambda,
  (interface_info_ft)    &sim_lmp_info
};

/*****************************************************************************
 *
 *  sim_lmp_table
 *
 *****************************************************************************/

int sim_lmp_table(interface_t * table) {

  *table = sim_lmp_interface;

  return 0;
}

/*****************************************************************************
 *
 *  sim_lmp_create
 *
 *****************************************************************************/

int sim_lmp_create(sim_lmp_t ** pobj) {

  sim_lmp_t * obj = NULL;

  obj = calloc(1, sizeof(sim_lmp_t));

  /* An error occured */
  if (obj == NULL) return -1;

  *pobj = obj;

  return 0;
}

/*****************************************************************************
 *
 *  sim_lmp_free
 *
 *****************************************************************************/

void sim_lmp_free(sim_lmp_t * obj) {

  free(obj);

  return;
}

/*****************************************************************************
 *
 *  sim_lmp_execute
 *
 *****************************************************************************/

int sim_lmp_execute(sim_lmp_t * obj, ffs_t * ffs,
		     sim_execute_enum_t action) {

  int ifail = 0;
  int argc = 0;
  char ** argv = NULL;
  MPI_Comm comm = MPI_COMM_NULL;

  switch (action) {
  case SIM_EXECUTE_INIT:
    /* execute initialisation phase */

    ffs_comm(ffs, &comm);
    ffs_command_line(ffs, &argc, &argv);

    lammps_open(argc, argv, comm, &obj->lmp);
    if (obj->lmp == NULL) ifail = -1;
    break;

  case SIM_EXECUTE_RUN:
    /* execute a run */
    /* RUN N STEPS (N=1) */
    /* update time counter in ffs */
    break;

  case SIM_EXECUTE_FINISH:

    /* execute the finalisation phase */
    lammps_close(obj->lmp);
    break;

  default:
    /* Something went wrong? */
    ifail = -1;
  }

  return  ifail;
}

/*****************************************************************************
 *
 *  sim_lmp_state
 *
 *****************************************************************************/

int sim_lmp_state(sim_lmp_t * obj, ffs_t * ffs, sim_state_enum_t action,
		   const char * stub) {

  int ifail = 0;

  switch (action) {
  case SIM_STATE_INIT:
    /* initialise the model state, e.g., */

    break;
  case SIM_STATE_READ:
    /* create a file name from the stub, and read the data */
    /*
    lammps_close();
    lammps_open();
     read restart */
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
 *  sim_lmp_lambda
 *
 *****************************************************************************/

int sim_lmp_lambda(sim_lmp_t * obj, ffs_t * ffs) {

  /* Compute lambda (rank 0) and set via ffs_info_int() */

  return 0;
}

/*****************************************************************************
 *
 *  sim_lmp_info
 *
 *****************************************************************************/

int sim_lmp_info (sim_lmp_t * obj, ffs_t * ffs, ffs_info_enum_t param) {

  /* Examine param, and put or get the appropriate information. */

  return 0;
}
