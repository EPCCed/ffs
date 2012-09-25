/*****************************************************************************
 *
 *  ffs_inst.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "u/libu.h"

#include "../util/ffs_util.h"
#include "../util/mpilog.h"
#include "ffs_param.h"
#include "ffs_inst.h"

struct ffs_inst_type {
  int id;               /* Unique id */
  int method;           /* ffs_method_enum */
  int ntask_request;    /* Number of tasks specified in config. */
  int nsim;             /* Number of simulations */
  MPI_Comm parent;      /* Parent communicator */
  MPI_Comm comm;        /* FFS instance communicator */
  ffs_param_t * param;  /* Parameters */
  mpilog_t * log;       /* Instance log */
};

/*****************************************************************************
 *
 *  ffs_inst_create
 *
 *  Split the parent communicator based on id of the instance.
 *
 *****************************************************************************/

int ffs_inst_create(int id, MPI_Comm parent, ffs_inst_t ** pobj) {

  ffs_inst_t * obj = NULL;
  int rank;
  int mpi_errnol = 0, mpi_errno = 0;

  dbg_return_if(parent == MPI_COMM_NULL, -1);

  mpi_errnol = (id < 0);
  mpi_sync_ifm(mpi_errnol, "id < 0");
  mpi_errnol = (pobj == NULL);
  mpi_sync_ifm(mpi_errnol, "pobj == NULL");

  mpi_errnol = ((obj = u_calloc(1, sizeof(ffs_inst_t))) == NULL);
  mpi_sync_sif(mpi_errnol);

  obj->id = id;
  obj->parent = parent;

  MPI_Comm_rank(obj->parent, &rank);
  MPI_Comm_split(obj->parent, obj->id, rank, &obj->comm);

  mpi_errnol = mpilog_create(obj->comm, &obj->log);
  mpi_sync_ifm(mpi_errnol, "mpilog_create() failed\n");

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, parent);
  nop_err_if(mpi_errno);

  *pobj = obj;

  return 0;

 err:
  if (obj) ffs_inst_free(obj);
  obj = NULL;

  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_free
 *
 *****************************************************************************/

void ffs_inst_free(ffs_inst_t * obj) {

  dbg_return_if(obj == NULL, );

  u_dbg("MPI_Comm_free(inst->comm)");

  if (obj->log) mpilog_free(obj->log);
  if (obj->param) ffs_param_free(obj->param);

  MPI_Comm_free(&obj->comm);
  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_inst_start
 *
 *****************************************************************************/

int ffs_inst_start(ffs_inst_t * obj, const char * filename,
		   const char * mode) {

  int sz;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(filename == NULL, -1);
  dbg_return_if(mode == NULL, -1);

  MPI_Comm_size(obj->comm, &sz);

  err_err_if(mpilog_fopen(obj->log, filename, mode));
  mpilog(obj->log, "FFS instance log for instance id %d\n", obj->id);
  mpilog(obj->log, "Running on %d MPI task%s", sz, (sz > 1) ? "s" : "");

  return 0;
 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_stop
 *
 *****************************************************************************/

int ffs_inst_stop(ffs_inst_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->log == NULL, -1);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Instance finished cleanly\n");
  mpilog(obj->log, "Closing log file.\n");

  return 0;
}

/*****************************************************************************
 *
 *  ffs_inst_execute
 *
 *****************************************************************************/

int ffs_inst_execute(ffs_inst_t * obj, u_config_t * input) {

  int ntask;
  int ifail;
  const char * method = NULL;
  u_config_t * config = NULL;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(input == NULL, -1);

  MPI_Comm_size(obj->comm, &ntask);

  /* Parse input */

  ifail = ((config = u_config_get_child(input, FFS_CONFIG_INST)) == NULL);
  mpilog_if(ifail, obj->log, "Config has no %s section\n", FFS_CONFIG_INST);
  dbg_err_ifm(ifail, "No config");

  /* Check the actual number of tasks against the requested number */

  dbg_err_if(u_config_get_subkey_value_i(config, FFS_CONFIG_INST_NTASK,
					 0, &obj->ntask_request));
  ifail = (ntask != obj->ntask_request);
  mpilog_if(ifail, obj->log, "Mismatch is actual and requested task number\n");
  dbg_err_ifm(ifail, "Number of tasks mismatched");

  /* Set number of simulations */

  err_err_if(u_config_get_subkey_value_i(config, FFS_CONFIG_INST_NSIM,
					 FFS_DEFAULT_INST_NSIM, &obj->nsim));
  err_err_ifm(obj->nsim > ntask, "Must have at least one task per simulation");
  err_err_ifm(ntask % obj->nsim, "No. simuations not divisor of inst. size");

  /* Set method */

  method = u_config_get_subkey_value(config, FFS_CONFIG_INST_METHOD);
  err_err_ifm(method == NULL, "Must have an %s key in the %s config\n",
	      FFS_CONFIG_INST_METHOD, FFS_CONFIG_INST);

  if (strcmp(method, FFS_CONFIG_METHOD_TEST) == 0) {
    obj->method = FFS_METHOD_TEST;
  }
  else if (strcmp(method, FFS_CONFIG_METHOD_BRANCHED) == 0) {
    obj->method = FFS_METHOD_BRANCHED;
  }
  else if (strcmp(method, FFS_CONFIG_METHOD_DIRECT) == 0) {
    obj->method = FFS_METHOD_DIRECT;
    err_err("direct not operational yet");
  }
  else {
    /* Not recognised */
    err_err("%s (%s) not recognised in %s\n", FFS_CONFIG_INST_METHOD, method,
	    FFS_CONFIG_INST);
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 * ffs_inst_param_init
 *
 *****************************************************************************/

int ffs_inst_param_init(ffs_inst_t * obj, u_config_t * config) {

  return 0;
}

/*****************************************************************************
 *
 *  ffs_inst_method_name
 *
 *****************************************************************************/

const char * ffs_inst_method_name(ffs_inst_t * obj) {

  dbg_return_if(obj == NULL, "null");

  switch (obj->method) {
  case FFS_METHOD_TEST:
    return FFS_CONFIG_METHOD_TEST;
  case FFS_METHOD_BRANCHED:
    return FFS_CONFIG_METHOD_BRANCHED;
  case FFS_METHOD_DIRECT:
    return FFS_CONFIG_METHOD_DIRECT;
  }

  return "unrecognised";
}

/*****************************************************************************
 *
 *  ffs_inst_config_log
 *
 *****************************************************************************/

int ffs_inst_config_log(ffs_inst_t * obj, mpilog_t * log) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj == NULL, -1);

  mpilog(log, "\n");
  mpilog(log, "%s\n", FFS_CONFIG_INST);
  mpilog(log, "{\n");
  mpilog(log, "\t%s\t%s\n", FFS_CONFIG_INST_METHOD, ffs_inst_method_name(obj));
  mpilog(log, "\t%s\t%d\n", FFS_CONFIG_INST_NTASK, obj->ntask_request);
  mpilog(log, "\t%s\t%d\n", FFS_CONFIG_INST_NSIM, obj->nsim);
  mpilog(log, "}\n");

  return 0;
}

/*****************************************************************************
 *
 *  ffs_inst_config
 *
 *****************************************************************************/

int ffs_inst_config(ffs_inst_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->log == NULL, -1);

  return ffs_inst_config_log(obj, obj->log);
}
