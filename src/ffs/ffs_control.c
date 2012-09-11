/*****************************************************************************
 *
 *  ffs_control.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>
#include <mpi.h>

#include "u/libu.h"
#include "../util/ffs_util.h"
#include "ffs_control.h"

/*
 *  Here's the opaque data type.
 */

struct ffs_control_type {

  /* All tasks */
  int ninstances;              /* Number of FFS instances */
  int master_seed;             /* Over RNG seed */
  MPI_Comm parent;             /* Parent communicator (e.g., MPI_COMM_WORLD) */
  MPI_Comm comm;               /* Library control communicator */
  u_config_t * input_config;   /* config to be read from input */

  /* ffs_inst_t * instance;*/
};

static int ffs_input_config(ffs_control_t * obj, const char * filename,
			    size_t * len);
static int ffs_broadcast_config(ffs_control_t * obj, size_t len);
static int ffs_input_config_parse(ffs_control_t * obj);
static int ffs_init_instances(ffs_control_t * obj);

/*****************************************************************************
 *
 *  ffs_control_create
 *
 *****************************************************************************/

int ffs_control_create(MPI_Comm parent, ffs_control_t ** pobj) {

  ffs_control_t * obj;
  int mpi_errnol = 0, mpi_errno = 0;
  MPI_Comm newcomm;

  dbg_return_if(pobj == NULL, -1);

  /* Assume this is a global success */
  MPI_Comm_dup(parent, &newcomm);

  mpi_errnol = ((obj = u_calloc(1, sizeof(ffs_control_t))) == NULL);
  mpi_sync_sif(mpi_errnol);

  obj->parent = parent;
  obj->comm = newcomm;
  *pobj = obj;

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, parent);
  nop_err_if(mpi_errno);

  return 0;

 err:
  MPI_Comm_free(&newcomm);
  if (obj) u_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_control_free
 *
 *****************************************************************************/

void ffs_control_free(ffs_control_t * obj) {

  dbg_return_if(obj == NULL, );

  MPI_Comm_free(&obj->comm);
  if (obj->input_config) u_config_free(obj->input_config);
  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_control_init
 *
 *****************************************************************************/

int ffs_control_init(ffs_control_t * obj, const char * filename) {

  size_t len;

  err_err_if(ffs_input_config(obj, filename, &len));
  MPI_Bcast(&len, 1, MPI_INT, 0, obj->comm);

  err_err_if(ffs_broadcast_config(obj, len));
  err_err_if(ffs_input_config_parse(obj));
  err_err_if(ffs_init_instances(obj));

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_control_run
 *
 *****************************************************************************/

int ffs_control_run(ffs_control_t * obj) {

  return 0;
}

/*****************************************************************************
 *
 *  ffs_control_finish
 *
 *****************************************************************************/

int ffs_control_finish(ffs_control_t * obj) {

  return 0;
}

/*****************************************************************************
 *
 *  ffs_input_config
 *
 *****************************************************************************/

static int ffs_input_config(ffs_control_t * obj, const char * filename,
			    size_t * len) {
  int rank;
  int mpi_errnol = 0, mpi_errno = 0;
  char * buf = NULL;
  FILE * fp = NULL;

  MPI_Comm_rank(obj->comm, &rank);

  if (rank == 0) {
    mpi_errnol = ((fp = fopen(filename, "r")) == NULL);
    mpi_sync_ifm(mpi_errnol, "Cannot open %s", filename);

    mpi_errnol = u_load_file(filename, -1, &buf, len);
    mpi_sync_ifm(mpi_errnol, "Problem with u_load_file(%s)", filename);

    mpi_errnol = u_config_load_from_buf(buf, *len, &obj->input_config);
    mpi_sync_ifm(mpi_errnol, "File %s is not configuration?", filename);

    U_FCLOSE(fp);
    U_FREE(buf);
  }

 mpi_sync:

  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  return 0;

 err:

  U_FCLOSE(fp);
  if (buf) U_FREE(buf);
  if (obj->input_config) u_config_free(obj->input_config);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_broadcast_config
 *
 *****************************************************************************/

static int ffs_broadcast_config(ffs_control_t * obj, size_t len) {

  int rank;
  int mpi_errnol = 0, mpi_errno = 0;
  char * buf = NULL;

  MPI_Comm_rank(obj->comm, &rank);

  mpi_errnol = ((buf = u_calloc(len, sizeof(char))) == NULL);
  mpi_sync_ifm(mpi_errnol, "u_calloc()");

  if (rank == 0) {
    mpi_errnol = u_config_save_to_buf(obj->input_config, buf, len);
    mpi_sync_ifm(mpi_errnol, "u_config_save_to_buf()");
  }

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  MPI_Bcast(buf, len, MPI_CHAR, 0, obj->comm);

  if (rank != 0) {
    mpi_errnol = u_config_load_from_buf(buf, len, &obj->input_config);
    err_ifm(mpi_errnol, "u_config_load_from_buf()");
  }

  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  U_FREE(buf);

  return 0;

 err:
  if (buf) U_FREE(buf);
  if (obj->input_config) u_config_free(obj->input_config);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_input_config_parse
 *
 *  At this stage everyone in obj->comm should agree on the contents
 *  of config, so failures are collective.
 *
 *****************************************************************************/

static int ffs_input_config_parse(ffs_control_t * obj) {

  int ntask;
  u_config_t * ffs = NULL;

  MPI_Comm_size(obj->comm, &ntask);

  ffs = u_config_get_child(obj->input_config, FFS_CONFIG_FFS);
  err_err_ifm(ffs == NULL, "Configuration must have ffs section");

  /* Read and check the number of instances */

  err_err_if(u_config_get_subkey_value_i(ffs, FFS_CONFIG_FFS_INSTANCES,
					 FFS_DEFAULT_FFS_INSTANCES,
					 &obj->ninstances));
  err_err_ifm(obj->ninstances > ntask,
	      "Must have at least one task per instance");
  err_err_ifm(ntask % obj->ninstances,
	      "Number of tasks must be divisible by number of instances"); 

  /* Read a check the random number seed */

  err_err_if(u_config_get_subkey_value_i(ffs, FFS_CONFIG_FFS_SEED,
					 FFS_DEFAULT_FFS_SEED,
					 &obj->master_seed));
  err_err_ifm(obj->master_seed < 1, "%s should be > 0", FFS_CONFIG_FFS_SEED);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_init_instances
 *
 *****************************************************************************/

static int ffs_init_instances(ffs_control_t * obj) {

  int rank, ntask, colour;

  MPI_Comm_rank(obj->comm, &rank);
  MPI_Comm_size(obj->comm, &ntask);

  colour = rank / (ntask / obj->ninstances);
  /* SPLIT */

  return 0;
}

/*****************************************************************************
 *
 *  ffs_control_print_summary_fp
 *
 *****************************************************************************/

int ffs_control_print_summary_fp(ffs_control_t * obj, FILE * fp) {

  int ntask;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fp == NULL, -1);

  MPI_Comm_size(obj->comm, &ntask);

  fprintf(fp, "\n");
  fprintf(fp, "FFS control communicator\n");
  fprintf(fp, "Total number of MPI tasks:   %d\n", ntask);
  fprintf(fp, "Number of instances:         %d\n", obj->ninstances);
  fprintf(fp, "Master random number seed:   %d\n", obj->master_seed);
  fprintf(fp, "\n");

  return 0;
}
