/*****************************************************************************
 *
 *  ffs_sim.h
 *
 *****************************************************************************/

#ifndef FFS_SIM_H
#define FFS_SIM_H

#include <mpi.h>
#include "./ffs_state.h"

enum do_cb {SIM_DO_START,
	    SIM_DO_STATE_INIT,
	    SIM_DO_STATE_SET,
	    SIM_DO_STATE_RECORD,
	    SIM_DO_STATE_REMOVE,
	    SIM_DO_END};

typedef struct ffs_sim_type ffs_sim_t;

struct ffs_cb_type {
  int (* do_start) (ffs_sim_t * obj);
  int (* do_end) (ffs_sim_t * obj);
  int (* do_state_init) (ffs_sim_t * obj, ffs_state_t * s);
  int (* do_state_set) (ffs_sim_t * obj, ffs_state_t * s);
  int (* do_state_record) (ffs_sim_t * obj, ffs_state_t * s);
  int (* do_state_remove) (ffs_sim_t * obj, ffs_state_t * s);
};

typedef struct ffs_cb_type ffs_cb_t;

int ffs_sim_create(ffs_sim_t ** pobj);
void ffs_sim_free(ffs_sim_t * obj);
int ffs_sim_register_cb(ffs_sim_t * obj, ffs_cb_t * cb);
int ffs_sim_deregister_cb(ffs_sim_t *obj);
int ffs_sim_do_something(ffs_sim_t * obj, int do_cb);
int ffs_sim_do_something_state(ffs_sim_t * obj, ffs_state_t * s, int do_cb);

int ffs_sim_comm_create(MPI_Comm comm, ffs_sim_t ** pobj);
int ffs_sim_comm(ffs_sim_t * obj, MPI_Comm * comm);

#endif
