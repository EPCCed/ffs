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
#include "../util/ranlcg.h"
#include "ffs_inst.h"
#include "ffs_control.h"

/*
 *  Here's the opaque data type.
 */

struct ffs_control_type {

  /* All tasks */
  int ninstances;              /* Number of FFS instances */
  int inst_id;                 /* This rank is handling this instance */
  MPI_Comm parent;             /* Parent communicator (e.g., MPI_COMM_WORLD) */
  MPI_Comm comm;               /* Control communicator */
  u_config_t * input;   /* config to be read from input */
  ffs_inst_t * instance;       /* The FFS instance */
  mpilog_t * log;              /* Control log. */
  int seed;                    /* Overall RNG seed */
  ranlcg_t * ran;              /* Control serial RNG */
};

static int ffs_input(ffs_control_t * obj, const char * filename,
			    size_t * len);
static int ffs_broadcast_config(ffs_control_t * obj, size_t len);
static int ffs_input_parse(ffs_control_t * obj);
static int ffs_init_instances(ffs_control_t * obj);

/*****************************************************************************
 *
 *  ffs_control_create
 *
 *****************************************************************************/

int ffs_control_create(MPI_Comm parent, ffs_control_t ** pobj) {

  ffs_control_t * obj = NULL;
  int mpi_errnol = 0, mpi_errno = 0;
  MPI_Comm newcomm = MPI_COMM_NULL;

  dbg_return_if(pobj == NULL, -1);

  MPI_Comm_dup(parent, &newcomm);

  mpi_errnol = ((obj = u_calloc(1, sizeof(ffs_control_t))) == NULL);
  mpi_sync_sif(mpi_errnol);

  obj->parent = parent;
  obj->comm = newcomm;

  mpi_errnol = mpilog_create(obj->comm, &obj->log);
  mpi_sync_if(mpi_errnol);

  *pobj = obj;

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, parent);
  nop_err_if(mpi_errno);

  return 0;

 err:
  MPI_Comm_free(&newcomm); /* is collective, so assumes Comm_dup() ok */
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

  u_dbg("MPI_Comm_free(control->comm)");

  if (obj->ran) ranlcg_free(obj->ran);
  if (obj->log) mpilog_free(obj->log);
  if (obj->instance) ffs_inst_free(obj->instance);
  if (obj->input) u_config_free(obj->input);

  MPI_Comm_free(&obj->comm); /* Communictor must exist if create() was ok */
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_control_start
 *
 *****************************************************************************/

int ffs_control_start(ffs_control_t * obj, const char * filename,
		      const char * mode) {
  int sz;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(filename == NULL, -1);
  dbg_return_if(mode == NULL, -1);
  dbg_return_if(obj->log == NULL, -1);

  MPI_Comm_size(obj->comm, &sz);

  err_err_if(mpilog_fopen(obj->log, filename, mode));
  mpilog(obj->log, "\n");
  mpilog(obj->log, "Welcome to FFS (%d MPI task%s)\n", sz,
	 (sz > 1) ? "s" : "");
  mpilog(obj->log, "Started FFS control log file: %s\n", filename);

  return 0;

 err:
  mpilog(obj->log, "Problem openning requested log file: %s\n", filename);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_control_stop
 *
 *****************************************************************************/

int ffs_control_stop(ffs_control_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->log == NULL, -1);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Closing log file. Finished.\n");
  mpilog_fclose(obj->log);

  return 0;
}

/*****************************************************************************
 *
 *  ffs_control_execute
 *
 *****************************************************************************/

int ffs_control_execute(ffs_control_t * obj, const char * filename) {

  size_t len;
  int mpi_errnol = 0, mpi_errno = 0;
  long int seed;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(filename == NULL, -1);

  /* Input config */

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Attempting to read input config: %s\n", filename);

  err_err_if(ffs_input(obj, filename, &len));
  MPI_Bcast(&len, 1, MPI_INT, 0, obj->comm);

  err_err_if(ffs_broadcast_config(obj, len));
  err_err_if(ffs_input_parse(obj));

  mpilog(obj->log, "Read input file successfully\n");

  /* Start control serial RNG (all ranks) */

  seed = (long int) obj->seed;
  mpi_errnol = ranlcg_create(seed, &obj->ran);
  mpi_sync_ifm(mpi_errnol, "ranlcg_create() failed\n");

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Started control RNG with seed %ld\n", seed);

  /* Instances TODO */

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Starting %d FFS instance%s\n", obj->ninstances,
	 (obj->ninstances > 1) ? "s" : "");

  /* Compound statistics TODO (before closing instances) */

  return 0;

 err:

  mpilog(obj->log, "Failed to execute correctly\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_input
 *
 *****************************************************************************/

static int ffs_input(ffs_control_t * obj, const char * filename,
			    size_t * len) {
  int rank;
  int mpi_errnol = 0, mpi_errno = 0;
  char * buf = NULL;
  FILE * fp = NULL;

  MPI_Comm_rank(obj->comm, &rank);

  if (rank == 0) {
    mpi_errnol = ((fp = fopen(filename, "r")) == NULL);
    mpilog_if(mpi_errnol, obj->log, "File missing!\n", filename);
    mpi_sync_if(mpi_errnol);

    mpi_errnol = u_load_file(filename, -1, &buf, len);
    mpi_sync_ifm(mpi_errnol, "u_load_file(%s) failed\n", filename);

    mpi_errnol = u_config_load_from_buf(buf, *len, &obj->input);
    mpilog_if(mpi_errnol, obj->log, "Badly formed config file?\n");
    mpi_sync_ifm(mpi_errnol, "u_config_load_from_buf(%s) failed", filename);

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
  if (obj->input) u_config_free(obj->input);

  mpilog(obj->log, "Please check and try again\n");

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
  mpi_sync_ifm(mpi_errnol, "u_calloc(buf) failed");

  if (rank == 0) {
    mpi_errnol = u_config_save_to_buf(obj->input, buf, len);
    mpi_sync_ifm(mpi_errnol, "u_config_save_to_buf() failed");
  }

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  MPI_Bcast(buf, len, MPI_CHAR, 0, obj->comm);

  if (rank != 0) {
    mpi_errnol = u_config_load_from_buf(buf, len, &obj->input);
    err_ifm(mpi_errnol, "u_config_load_from_buf() failed");
  }

  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  U_FREE(buf);

  return 0;

 err:
  if (buf) U_FREE(buf);
  if (obj->input) u_config_free(obj->input);

  mpilog(obj->log, "Failed to Bcast() the config to all ranks\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_input_parse
 *
 *  At this stage everyone in obj->comm should agree on the contents
 *  of config, so failures are collective.
 *
 *****************************************************************************/

static int ffs_input_parse(ffs_control_t * obj) {

  int ntask;
  int ifail;
  u_config_t * ffs = NULL;

  MPI_Comm_size(obj->comm, &ntask);

  ifail = ((ffs = u_config_get_child(obj->input, FFS_CONFIG_FFS)) == NULL);
  mpilog_if(ifail, obj->log, "No %s config section\n", FFS_CONFIG_FFS);
  dbg_err_ifm(ifail, "u_config_get_child() failed");

  /* Read parameters */

  dbg_err_if(u_config_get_subkey_value_i(ffs, FFS_CONFIG_FFS_INSTANCES,
					 FFS_DEFAULT_FFS_INSTANCES,
					 &obj->ninstances));

  dbg_err_if(u_config_get_subkey_value_i(ffs, FFS_CONFIG_FFS_SEED,
					 FFS_DEFAULT_FFS_SEED,
					 &obj->seed));

  /* Check parameters */

  ffs_control_config(obj);

  ifail = (ntask < obj->ninstances);
  mpilog_if(ifail, obj->log, "Requested too many instances\n");
  dbg_err_ifm(ifail, "Too many instances");

  ifail = ((ntask % obj->ninstances) != 0);
  mpilog_if(ifail, obj->log, "Must have equal number of tasks per instance\n");
  dbg_err_ifm(ifail, "ntask % ninstance != 0"); 

  ifail = (obj->seed < 1);
  mpilog_if(ifail, obj->log, "%s must be > 0\n", FFS_CONFIG_FFS_SEED);
  dbg_err_ifm(ifail, "Bad seed");

  return 0;

 err:

  mpilog(obj->log, "Please check and try again\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_init_instances
 *
 *****************************************************************************/

static int ffs_init_instances(ffs_control_t * obj) {

  int rank, ntask, id;
  u_config_t * input = NULL;

  MPI_Comm_rank(obj->comm, &rank);
  MPI_Comm_size(obj->comm, &ntask);

  /* Divide the available MPI tasks by the number of instances requested,
     and initialise. */

  id = rank / (ntask / obj->ninstances);

  dbg_err_if(u_config_get_subkey(obj->input, FFS_CONFIG_FFS, &input));
  err_err_if(ffs_inst_create(id, obj->comm, &obj->instance));

  err_err_if(ffs_inst_execute(obj->instance, input));

  return 0;

 err:

  if (obj->instance) ffs_inst_free(obj->instance);
  obj->instance = NULL;

  mpilog(obj->log, "Problem creating instances\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_control_log_config
 *
 *****************************************************************************/

int ffs_control_config_log(ffs_control_t * obj, mpilog_t * log) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(log == NULL, -1);

  mpilog(log, "\n");
  mpilog(log, "%s\n", FFS_CONFIG_FFS);
  mpilog(log, "{\n");
  mpilog(log, "\t%s\t%d\n", FFS_CONFIG_FFS_INSTANCES, obj->ninstances);
  mpilog(log, "\t%s\t%d\n", FFS_CONFIG_FFS_SEED, obj->seed);
  mpilog(log, "}\n");

  return 0;
}

/*****************************************************************************
 *
 *  ffs_control_config
 *
 *****************************************************************************/

int ffs_control_config(ffs_control_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ffs_control_config_log(obj, obj->log), -1);

  return 0;
}
