/*****************************************************************************
 *
 *  \file ffs_sim.c
 *
 *  Abstract simulation object.
 *
 *****************************************************************************/

#include <mpi.h>

#include "u/libu.h"
#include "ffs_state.h"
#include "ffs_sim.h"

/*
 * This is the opaque object.
 */

struct ffs_sim_type {
  MPI_Comm simcomm;
  ffs_cb_t * cb;
  size_t argc;
  char ** argv;
};

/*****************************************************************************
 *
 *  \brief
 *
 *****************************************************************************/

int ffs_sim_create(ffs_sim_t ** pobj) {

  ffs_sim_t * obj;

  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = u_calloc(1, sizeof(ffs_sim_t))) == NULL);
  *pobj = obj;

  return 0;

 err:
  if (obj) u_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_comm_create(MPI_Comm comm, ffs_sim_t ** pobj) {

  err_err_if(ffs_sim_create(pobj));
  (*pobj)->simcomm = comm;

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_comm(ffs_sim_t * obj, MPI_Comm * comm) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(comm == NULL, -1);

  *comm = obj->simcomm;

  return 0;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

void ffs_sim_free(ffs_sim_t * obj) {

  int n;

  dbg_return_if(obj == NULL, );

  for (n = 0; n < obj->argc; n++) {
    u_free(obj->argv[n]);
  }

  u_free(obj->argv);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_register_cb(ffs_sim_t * obj, ffs_cb_t * cb) {

  dbg_err_if(obj == NULL);
  dbg_err_ifm(cb == NULL, "Trying to register NULL callback block");
  dbg_err_ifm(cb->sim_start == NULL, "cb has NULL sim_start method");
  dbg_err_ifm(cb->sim_end == NULL, "cb has NULL sim_end method");
  dbg_err_ifm(cb->sim_state_init == NULL, "cb has NULL sim_state_init");
  dbg_err_ifm(cb->sim_state_set == NULL, "cb has NULL sim_state_set");
  dbg_err_ifm(cb->sim_state_record == NULL, "cb has NULL sim_state_record");
  dbg_err_ifm(cb->sim_state_remove == NULL, "cb has NULL sim_state_remove");
  dbg_err_ifm(cb->sim_lambda == NULL, "cb has NULL sim_lambda");
  dbg_err_ifm(cb->sim_time == NULL, "cb has NULL sim_time");
  dbg_err_ifm(cb->sim_time_set == NULL, "cb has NULL sim_time_set");
  dbg_err_ifm(cb->sim_run == NULL, "cb has NULL sim_run");

  obj->cb = cb;

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_deregister_cb(ffs_sim_t * obj) {

  dbg_return_if(obj == NULL, -1);

  obj->cb = NULL;

  return 0;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_call_back_null(ffs_sim_t * obj, int cb) {

  dbg_return_if(obj == NULL, -1);

  /* Try cbs */

  switch (cb) {
  case SIM_START:
    err_err_if(obj->cb->sim_start(obj));
    break;
  case SIM_END:
    err_err_if(obj->cb->sim_end(obj));
    break;
  default:
    dbg_err("Invalid call back enum value %d\n", cb);
  }

  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_call_back_darg(ffs_sim_t * obj, int cb, double * arg) {

  dbg_return_if(obj == NULL, -1);

  /* Try cbs */

  switch (cb) {
  case SIM_LAMBDA:
    err_err_if(obj->cb->sim_lambda(obj, arg));
    break;
  case SIM_TIME:
    err_err_if(obj->cb->sim_time(obj, arg));
    break;
  case SIM_TIME_SET:
    err_err_if(obj->cb->sim_time_set(obj, *arg));
    break;
  default:
    dbg_err("Unrecognised cb value %d\n", cb);
  }

  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_call_back_state(ffs_sim_t * obj, ffs_state_t * s, int cb) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(s == NULL, -1);

  switch (cb) {
  case SIM_STATE_INIT:
    err_err_if(obj->cb->sim_state_init(obj, s));
    break;
  case SIM_STATE_SET:
    err_err_if(obj->cb->sim_state_set(obj, s));
    break;
  case SIM_STATE_RECORD:
    err_err_if(obj->cb->sim_state_record(obj, s));
    break;
  case SIM_STATE_REMOVE:
    err_err_if(obj->cb->sim_state_remove(obj, s));
    break;
  default:
    dbg_err("Invalid call back value %d\n", cb);
  }

  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_args(ffs_sim_t * obj, int * argc, char *** argv) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(argc == NULL, -1);
  dbg_return_if(argv == NULL, -1);

  *argc = obj->argc;
  *argv = obj->argv;

  return 0;
}

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

int ffs_sim_args_set(ffs_sim_t * obj, char * argstring) {

  int n;
  size_t argc = 0;
  char ** argv = NULL;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(argstring == NULL, -1);

  /* Use u_strtok() to parse the argstring */

  dbg_err_if(u_strtok(argstring, " ", &argv, &argc));

  /* Add 1 for simulation executable name, and 1 to accomodate
   * argv[argc] = NULL, required by the standard. */

  obj->argc = argc + 1;
  dbg_err_sif((obj->argv = u_calloc((obj->argc + 1), sizeof(char *))) == NULL);

  obj->argv[0] = u_strdup(FFS_SIM_EXECUTABLE_NAME);

  for (n = 0; n < argc; n++) {
    obj->argv[n+1] = u_strdup(argv[n]);
  }
  obj->argv[obj->argc] = NULL;

  u_strtok_cleanup(argv, argc);

  return 0;

 err:
  u_strtok_cleanup(argv, argc);

  return -1;
}
