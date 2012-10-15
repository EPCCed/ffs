/*****************************************************************************
 *
 *  proxy.c
 *
 *
 *****************************************************************************/

#include <stdlib.h>

#include "u/libu.h"
#include "ffs_private.h"
#include "ffs_util.h"
#include "factory.h"
#include "proxy.h"

struct proxy_s {
  int id;
  abstract_sim_t * delegate;
  interface_t vtable;
  MPI_Comm parent;
  MPI_Comm comm;
  ffs_t * ffs;
};

/*****************************************************************************
 *
 *  proxy_create
 *
 *****************************************************************************/

int proxy_create(int id, MPI_Comm parent, proxy_t ** pobj) {

  int rank;
  int mpi_errno = 0, mpi_errnol = 0;
  MPI_Comm newcomm = MPI_COMM_NULL;
  proxy_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);

  mpi_errnol = (id < 0);
  mpi_sync_ifm(mpi_errnol, "id < 0");

  MPI_Comm_rank(parent, &rank);
  MPI_Comm_split(parent, id, rank, &newcomm);

  mpi_errnol = ((obj = u_calloc(1, sizeof(proxy_t))) == NULL);
  mpi_sync_ifm(mpi_errnol, "calloc(proxy_t) failed");

  obj->id = id;
  obj->parent = parent;
  obj->comm = newcomm;

  err_err_if(ffs_create(obj->comm, &obj->ffs));

  *pobj = obj;

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, parent);
  nop_err_if(mpi_errno);

  return 0;

 err:

  /* A little care required not to free new communicator twice */

  MPI_Comm_free(&newcomm);
  if (obj) {
    obj->comm = MPI_COMM_NULL;
    proxy_free(obj);
  }

  return -1;
}

/*****************************************************************************
 *
 *  proxy_free
 *
 *****************************************************************************/

void proxy_free(proxy_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->ffs) ffs_free(obj->ffs);
  if (obj->comm != MPI_COMM_NULL) MPI_Comm_free(&obj->comm);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  proxy_delegate_create
 *
 *  The factory method will test for collective success in creating
 *  the delegate.
 *
 *****************************************************************************/

int proxy_delegate_create(proxy_t * obj, const char * name) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(name == NULL, -1);

  err_err_if(factory_make(obj->comm, name, &obj->vtable, &obj->delegate));

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  proxy_delegate_free
 *
 *****************************************************************************/

int proxy_delegate_free(proxy_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->delegate == NULL, -1);

  obj->vtable.free(obj->delegate);

  return 0;
}

/*****************************************************************************
 *
 *  proxy_execute
 *
 *****************************************************************************/

int proxy_execute(proxy_t * obj, sim_execute_enum_t action) {

  dbg_return_if(obj == NULL, -1);

  return obj->vtable.execute(obj->delegate, obj->ffs, action);
}

/*****************************************************************************
 *
 *  proxy_state
 *
 *****************************************************************************/

int proxy_state(proxy_t * obj, sim_state_enum_t action, const char * stub) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(stub == NULL, -1);

  return obj->vtable.state(obj->delegate, obj->ffs, action, stub);
}

/*****************************************************************************
 *
 *  proxy_lambda
 *
 *****************************************************************************/

int proxy_lambda(proxy_t * obj) {

  dbg_return_if(obj == NULL, -1);

  return obj->vtable.lambda(obj->delegate, obj->ffs);
}

/*****************************************************************************
 *
 *  proxy_info_int
 *
 *****************************************************************************/

int proxy_info(proxy_t * obj, ffs_info_enum_t param) {

  dbg_return_if(obj == NULL, -1);

  return obj->vtable.info(obj->delegate, obj->ffs, param);
}

/*****************************************************************************
 *
 *  proxy_ffs
 *
 *****************************************************************************/

int proxy_ffs(proxy_t * obj, ffs_t ** ffs) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj == NULL, -1);

  *ffs = obj->ffs;

  return 0;
}
