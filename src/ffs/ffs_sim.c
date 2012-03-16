
#include <mpi.h>

#include "u/libu.h"
#include "ffs_state.h"
#include "ffs_sim.h"

struct ffs_sim_type {
  MPI_Comm simcomm;
  ffs_cb_t * cb;
};

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

/*
 */

int ffs_sim_comm_create(MPI_Comm comm, ffs_sim_t ** pobj) {

  err_err_if(ffs_sim_create(pobj));
  (*pobj)->simcomm = comm;

  return 0;

 err:

  return -1;
}

int ffs_sim_comm(ffs_sim_t * obj, MPI_Comm * comm) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(comm == NULL, -1);

  *comm = obj->simcomm;

  return 0;
}

/*
 */

void ffs_sim_free(ffs_sim_t * obj) {

  dbg_return_if(obj == NULL, );

  u_free(obj);

  return;
}

int ffs_sim_register_cb(ffs_sim_t * obj, ffs_cb_t * cb) {

  err_err_if(obj == NULL);
  err_err_ifm(cb == NULL, "Trying to register NULL callback block");
  err_err_ifm(cb->do_start == NULL, "cb has NULL do_start");
  err_err_ifm(cb->do_end == NULL, "cb has NULL do_end");

  obj->cb = cb;

  return 0;

 err:

  return -1;
}

int ffs_sim_deregister_cb(ffs_sim_t * obj) {

  dbg_return_if(obj == NULL, -1);

  obj->cb = NULL;

  return 0;
}

/*
 */

int ffs_sim_do_something(ffs_sim_t * obj, int do_cb) {

  dbg_return_if(obj == NULL, -1);

  /* Try cbs */

  switch (do_cb) {
  case SIM_DO_START:
    err_err_if(obj->cb->do_start(obj));
    break;
  case SIM_DO_END:
    err_err_if(obj->cb->do_end(obj));
    break;
  default:
    err_err("Unrecognised do_cb value: ", do_cb);
  }

  return 0;

 err:
  return -1;
}

/*
 */

int ffs_sim_do_something_state(ffs_sim_t * obj, ffs_state_t * s, int do_cb) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(s == NULL, -1);

  switch (do_cb) {
  case SIM_DO_STATE_INIT:
    err_err_if(obj->cb->do_state_init(obj, s));
    break;
  case SIM_DO_STATE_SET:
    err_err_if(obj->cb->do_state_set(obj, s));
    break;
  case SIM_DO_STATE_RECORD:
    err_err_if(obj->cb->do_state_record(obj, s));
    break;
  case SIM_DO_STATE_REMOVE:
    err_err_if(obj->cb->do_state_remove(obj, s));
    break;
  default:
    err_err("Unrecognised do_cb value: ", do_cb);
  }

  return 0;

 err:
  return -1;

}
