/*****************************************************************************
 *
 *  ffs.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include <mpi.h>
#include "u/libu.h"

#include "ffs_util.h"
#include "ffs_private.h"

typedef struct var_s var_t;

struct var_s {
  int type;
  int ndata;
  union {
    int ui;
    double ud;
  } var;
};

struct ffs_s {
  MPI_Comm comm;
  size_t argc;
  char ** argv;
  /* Simulation data */
  var_t t;
  var_t lambda;
  int seed;
};

static int ffs_free_command_line(ffs_t * ffs);

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
  obj->t.type = -1;
  obj->lambda.type = -1;

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

  dbg_return_if(obj == NULL, );

  if (obj->argv) ffs_free_command_line(obj);
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
 *  REPLACE WITH SEPARATE ROUTINES for argc, argv. USER needs to
 *  allocate argv.
 *
 *****************************************************************************/

int ffs_command_line(ffs_t * obj, int * argc, char *** argv) {

  char ** tmp;
  int n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(argc == NULL, -1);
  dbg_return_if(argv == NULL, -1);

  *argc = obj->argc;

  tmp = u_calloc(obj->argc + 1, sizeof(char *));
  for (n = 0; n < obj->argc; n++) {
    tmp[n] = obj->argv[n];
  }

  *argv = tmp;

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

int ffs_command_line_set(ffs_t * obj, const char * argstring) {

  int n;
  size_t argc = 0;
  char ** argv = NULL;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(argstring == NULL, -1);
  dbg_return_ifm(obj->argc != 0, -1, "Use ffs_command_line_reset()\n");

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
 *  ffs_command_line_reset
 *
 *  Dispose of previous command line arguments (if present) and set
 *  new ones. The caller should avoid having references to
 *  the previous set hanging around.
 *
 *****************************************************************************/

int ffs_command_line_reset(ffs_t * obj, const char * argstring) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(argstring == NULL, -1);

  if (obj->argv) err_err_if(ffs_free_command_line(obj));
  err_err_if(ffs_command_line_set(obj, argstring));

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_info_int
 *
 *****************************************************************************/

int ffs_info_int(ffs_t * obj, ffs_info_enum_t param, int ndata, int * data) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ndata != 1, -1);   /* 1 item only at the moment! */
  dbg_return_if(data == NULL, -1);

  switch (param) {
  case FFS_INFO_TIME_PUT:
    err_err_if(obj->t.type != FFS_VAR_INT);
    obj->t.var.ui = *data;
    break;
  case FFS_INFO_TIME_FETCH:
    err_err_if(obj->t.type != FFS_VAR_INT);
    *data = obj->t.var.ui;
    break;
  case FFS_INFO_RNG_SEED_PUT:
    obj->seed = *data;
    break;
  case FFS_INFO_RNG_SEED_FETCH:
    *data = obj->seed;
    break;
  case FFS_INFO_LAMBDA_PUT:
    err_err_if(obj->lambda.type != FFS_VAR_INT);
    obj->lambda.var.ui = *data;
    break;
  case FFS_INFO_LAMBDA_FETCH:
    err_err_if(obj->lambda.type != FFS_VAR_INT);
    *data = obj->lambda.var.ui;
    break;
  default:
    err_err("ffs_info_enum_t not recognised");
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

int ffs_info_double(ffs_t * obj, ffs_info_enum_t param, int ndata,
		    double * data) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ndata < 1, -1);
  dbg_return_if(data == NULL, -1);

  switch (param) {
  case FFS_INFO_TIME_PUT:
    err_err_if(obj->t.type != FFS_VAR_DOUBLE);
    obj->t.var.ud = *data;
    break;
  case FFS_INFO_TIME_FETCH:
    err_err_if(obj->t.type != FFS_VAR_DOUBLE);
    *data = obj->t.var.ud;
    break;
  case FFS_INFO_RNG_SEED_PUT:
    obj->seed = *data;
    break;
  case FFS_INFO_RNG_SEED_FETCH:
    *data = obj->seed;
    break;
  case FFS_INFO_LAMBDA_PUT:
    err_err_if(obj->lambda.type != FFS_VAR_DOUBLE);
    obj->lambda.var.ud = *data;
    break;
  case FFS_INFO_LAMBDA_FETCH:
    err_err_if(obj->lambda.type != FFS_VAR_DOUBLE);
    *data = obj->lambda.var.ud;
    break;
  default:
    err_err("ffs_info_enum_t param not recognised");
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_type_set
 *
 *****************************************************************************/

int ffs_type_set(ffs_t * obj, ffs_info_enum_t param, int ndata,
		 ffs_var_enum_t type) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ndata != 1, -1);  /* Must be a scalar at moment */

  switch (param) {
  case FFS_INFO_TIME_PUT:
    obj->t.type = type;
    obj->t.ndata = ndata;
    break;
  case FFS_INFO_LAMBDA_PUT:
    obj->lambda.type = type;
    obj->lambda.ndata = ndata;
    break;
  default:
    err_err("Inccorrent param argument");
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_type
 *
 *****************************************************************************/

int ffs_type(ffs_t * obj, ffs_info_enum_t param, int * ndata,
	     ffs_var_enum_t * type) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ndata == NULL, -1);
  dbg_return_if(type == NULL, -1);

  switch (param) {
  case FFS_INFO_TIME_PUT:
    *type = obj->t.type;
    *ndata = obj->t.ndata;
    break;
  case FFS_INFO_LAMBDA_PUT:
    *type = obj->lambda.type;
    *ndata = obj->lambda.ndata;
    break;
  default:
    err_err("Inccorrent param argument");

  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_lambda
 *
 *****************************************************************************/

int ffs_lambda(ffs_t * obj, double * lambda) {

  dbg_return_if(ffs == NULL, -1);
  dbg_return_if(lambda == NULL, -1);
  dbg_return_if(obj->lambda.type < 0, -1);

  if (obj->lambda.type == FFS_VAR_INT) *lambda = 1.0*obj->lambda.var.ui;
  if (obj->lambda.type == FFS_VAR_DOUBLE) *lambda = obj->lambda.var.ud;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_time
 *
 *****************************************************************************/

int ffs_time(ffs_t * obj, double * t) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(t == NULL, -1);
  dbg_return_if(obj->t.type < 0, -1);

  if (obj->t.type == FFS_VAR_INT) *t = 1.0*obj->t.var.ui;
  if (obj->t.type == FFS_VAR_DOUBLE) *t = obj->t.var.ud;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_free_command_line
 *
 *****************************************************************************/

static int ffs_free_command_line(ffs_t * obj) {

  int n;

  dbg_return_if(obj == NULL, 0);
  dbg_return_if(obj->argv == NULL, 0);

  for (n = 0; n < obj->argc; n++) {
    u_free(obj->argv[n]);
  }

  u_free(obj->argv);
  obj->argc = 0;

  return 0;
}
