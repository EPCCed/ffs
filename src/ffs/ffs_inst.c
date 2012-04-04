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
#include "ffs_param.h"
#include "ffs_sim.h"
#include "ffs_inst.h"

struct ffs_inst_type {
  int id;               /* Unique id */
  int method;           /* ffs_method_enum */
  int nsim;             /* Number of simulations */
  FILE * log;           /* Instance log */
  MPI_Comm parent;      /* Parent communicator */
  MPI_Comm comm;        /* FFS instance communicator */
  ffs_param_t * param;  /* Parameters */
  ffs_sim_t * sim;      /* Simulation handle */
  /* ffs_tree_t * tree; */
};

/*****************************************************************************
 *
 *  ffs_inst_create
 *
 *  Remember id and parent, but defer doing anything until ffs_inst_init().
 *
 *****************************************************************************/

int ffs_inst_create(int id, MPI_Comm parent, ffs_inst_t ** pobj) {

  ffs_inst_t * obj = NULL;
  int mpi_errnol = 0, mpi_errno = 0;

  dbg_return_if(parent == MPI_COMM_NULL, -1);

  mpi_errnol = (id < 0);
  mpi_sync_ifm(mpi_errnol, "id < 0");
  mpi_errnol = (pobj == NULL);
  mpi_sync_ifm(mpi_errnol, "pobj == NULL");

  mpi_errnol = ((obj = u_calloc(1, sizeof(ffs_inst_t))) == NULL);
  mpi_sync_sif(mpi_errnol);

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, parent);
  nop_err_if(mpi_errno);

  obj->id = id;
  obj->parent = parent;
  *pobj = obj;

  return 0;

 err:
  if (obj) ffs_inst_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_free
 *
 *****************************************************************************/

void ffs_inst_free(ffs_inst_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->comm) MPI_Comm_free(&obj->comm);
  obj->comm = MPI_COMM_NULL;

  if (obj->param) ffs_param_free(obj->param);
  if (obj->sim) ffs_sim_free(obj->sim);
  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_inst_init
 *
 *****************************************************************************/

int ffs_inst_init(ffs_inst_t * obj, u_config_t * config) {

  int rank, ntask;
  const char * method;
  u_config_t * ffs_inst;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(config == NULL, -1);

  MPI_Comm_rank(obj->parent, &rank);
  MPI_Comm_split(obj->parent, obj->id, rank, &obj->comm);

  /* Start the log (to stdout for moment) */

  obj->log = stdout;

  /* Extract the section */

  ffs_inst = u_config_get_child(config, FFS_CONFIG_INST);
  err_err_ifm(ffs_inst == NULL, "Configuration should have %s section",
	      FFS_CONFIG_INST);

  /* Set number of simulations */

  err_err_if(u_config_get_subkey_value_i(ffs_inst, FFS_CONFIG_INST_SIMULATIONS,
					 FFS_DEFAULT_INST_SIMULATIONS,
					 &obj->nsim));
  MPI_Comm_size(obj->comm, &ntask);
  err_err_ifm(obj->nsim > ntask, "Must have at least one task per simulation");
  err_err_ifm(ntask % obj->nsim, "No. simuations not divisor of inst. size");

  /* Set method */

  method = u_config_get_subkey_value(ffs_inst, FFS_CONFIG_INST_METHOD);
  err_err_ifm(method == NULL, "Must have an %s key in the %s config\n",
	      FFS_CONFIG_INST_METHOD, FFS_CONFIG_INST);

  if (strcmp(method, FFS_CONFIG_METHOD_TEST) == 0) {
    obj->method = FFS_METHOD_TEST;
  }
  else if (strcmp(method, FFS_CONFIG_METHOD_BRANCHED) == 0) {
    obj->method = FFS_METHOD_BRANCHED;
    err_err("branched not operational yet");
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
  if (obj->comm) MPI_Comm_free(&obj->comm);

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
 *  ffs_inst_print_summary
 *
 *****************************************************************************/

int ffs_inst_print_summary(ffs_inst_t * obj) {

  int rank;
  int mpi_errnol = 0, mpi_errno = 0;

  dbg_return_if(obj == NULL, -1);

  MPI_Comm_rank(obj->comm, &rank);

  if (rank == 0) {
    mpi_errnol = ffs_inst_print_summary_fp(obj, obj->log);
    mpi_sync_ifm(mpi_errnol, "ffs_inst_print_summary_fp()");

    if (obj->param) {
      mpi_errnol = ffs_param_print_summary_fp(obj->param, obj->log);
      mpi_sync_ifm(mpi_errnol, "ffs_param_print_summary_fp()");
    }
  }

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_print_summary_fp
 *
 *****************************************************************************/

int ffs_inst_print_summary_fp(ffs_inst_t * obj, FILE * fp) {

  int ntask;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fp == NULL, -1);

  MPI_Comm_size(obj->comm, &ntask);

  fprintf(fp, "\n");
  fprintf(fp, "FFS instance:                     id%d\n", obj->id);
  fprintf(fp, "Method:                           %s\n",
	  ffs_inst_method_name(obj));
  fprintf(fp, "Total number of tasks:            %d\n", ntask);
  fprintf(fp, "Number of simulation instances:   %d\n", obj->nsim);
  fprintf(fp, "Tasks per simulation instance:    %d\n", ntask / obj->nsim);

  return 0;
}
