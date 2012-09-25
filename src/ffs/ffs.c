/*****************************************************************************
 *
 *  ffs.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include <mpi.h>
#include "u/libu.h"

#include "u_extra.h"
#include "ffs_private.h"

struct ffs_s {
  MPI_Comm comm;
  size_t argc;
  char ** argv;
};

/*****************************************************************************
 *
 *  ffs_create
 *
 *  We just retain a reference to the incoming communicator.
 *
 *****************************************************************************/

int ffs_create(MPI_Comm comm, ffs_t ** pobj) {

  int mpi_errnol = 0, mpi_errno = 0;
  ffs_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);

  mpi_errnol = ((obj = u_calloc(1, sizeof(ffs_t))) == NULL);
  mpi_sync_sif(mpi_errnol);

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, comm);
  nop_err_if(mpi_errno);

  obj->comm = comm;
  *pobj = obj;

  return 0;

 err:
  if (obj) u_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_free
 *
 *  We are not responsible for releasing the communicator.
 *
 *****************************************************************************/

void ffs_free(ffs_t * obj) {

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
 *  ffs_comm
 *
 *****************************************************************************/

int ffs_comm(ffs_t * obj, MPI_Comm * comm) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(comm == NULL, -1);

  *comm = obj->comm;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_command_line
 *
 *****************************************************************************/

int ffs_command_line(ffs_t * obj, int * argc, char *** argv) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(argc == NULL, -1);
  dbg_return_if(argv == NULL, -1);

  *argc = obj->argc;
  *argv = obj->argv;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_command_line_set
 *
 *  We are going to use the utility u_strtok() to build the argv
 *  from the single argument string. We assume this will succeed
 *  or fail uniformly on all ranks, although this does involve
 *  some memory allocation.
 *
 *****************************************************************************/

int ffs_command_line_set(ffs_t * obj, char * argstring) {

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

/*****************************************************************************
 *
 *  ffs_info_int
 *
 *****************************************************************************/

int ffs_info_int(ffs_t * obj, ffs_info_enum_t type, int ndata, int * data) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ndata < 1, -1);
  dbg_return_if(data == NULL, -1);

  switch (type) {
  default:
    err_err("type not recognised");
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_info_double
 *
 *****************************************************************************/

int ffs_info_double(ffs_t * obj, ffs_info_enum_t type, int ndata,
		    double * data) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ndata < 1, -1);
  dbg_return_if(data == NULL, -1);

  switch (type) {
  default:
    err_err("type not recognised");
  }

  return 0;

 err:

  return -1;
}
