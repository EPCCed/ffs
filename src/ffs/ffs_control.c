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
#include "../../conf.h"
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
  u_config_t * input;          /* config to be read from input */
  ffs_inst_t * instance;       /* The FFS instance */
  u_string_t * name;           /* Run name string */
  mpilog_t * log;              /* Control log (stdout). */
  int seed;                    /* Overall RNG seed */
  ffs_result_summary_t * res;  /* Summary */
};

static int ffs_input(ffs_control_t * obj, const char * filename,
			    size_t * len);
static int ffs_broadcast_config(ffs_control_t * obj, size_t len);
static int ffs_input_parse(ffs_control_t * obj);

/*****************************************************************************
 *
 *  ffs_control_create
 *
 *****************************************************************************/

int ffs_control_create(MPI_Comm parent, ffs_control_t ** pobj) {

  ffs_control_t * obj = NULL;
  int mpi_errnol = 0;
  MPI_Comm newcomm = MPI_COMM_NULL;

  dbg_return_if(pobj == NULL, -1);

  MPI_Comm_dup(parent, &newcomm);

  mpi_errnol = ((obj = u_calloc(1, sizeof(ffs_control_t))) == NULL);
  mpi_err_if_any(mpi_errnol, parent);

  obj->parent = parent;
  obj->comm = newcomm;

  mpi_errnol = mpilog_create(obj->comm, &obj->log);
  mpi_err_if_any(mpi_errnol, parent);

  *pobj = obj;

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

  if (obj->name) u_string_free(obj->name);
  if (obj->log) mpilog_free(obj->log);
  if (obj->instance) ffs_inst_free(obj->instance);
  if (obj->input) u_config_free(obj->input);
  if (obj->res) ffs_result_summary_free(obj->res);

  MPI_Comm_free(&obj->comm); /* Communictor must exist if create() was ok */
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_control_start
 *
 *****************************************************************************/

int ffs_control_start(ffs_control_t * obj, const char * name) {

  int sz;
  int mpi_errnol = 0;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(name == NULL, -1);

  MPI_Comm_size(obj->comm, &sz);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Starting FFS (%d MPI task%s)\n", sz, (sz > 1) ? "s" : "");
  mpilog(obj->log, "FFS library v%s\n", FFS_VERSION);

  /* Remember the name */

  mpi_errnol = u_string_create(name, strlen(name), &obj->name);
  mpi_err_if_any(mpi_errnol, obj->comm);

  return 0;

 err:
  mpilog(obj->log, "u_string_create() failed in ffs_control_start()\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_control_stop
 *
 *****************************************************************************/

int ffs_control_stop(ffs_control_t * obj, ffs_result_summary_t * result) {

  dbg_return_if(obj == NULL, -1);

  if (result) ffs_result_summary_copy(obj->res, result);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Closing FFS. Finished.\n");

  return 0;
}

/*****************************************************************************
 *
 *  ffs_control_execute
 *
 *****************************************************************************/

int ffs_control_execute(ffs_control_t * obj, const char * configfilename) {

  size_t len;
  int n;
  int sz;
  int seed;
  int mpi_errnol = 0, mpi_errno = 0;

  ranlcg_t * ran = NULL;
  u_config_t * config = NULL;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(configfilename == NULL, -1);

  MPI_Comm_size(obj->comm, &sz);

  /* Read the config file, broadcast the contents to all ranks,
   * and then all ranks can parse the input and proceed. */

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Attempting to read input config: %s\n", configfilename);

  err_err_if(ffs_input(obj, configfilename, &len));

  err_err_if(ffs_broadcast_config(obj, len));
  err_err_if(ffs_input_parse(obj));

  mpilog(obj->log, "Read input file successfully\n");

  /* Start control serial RNG (all ranks) */

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Started control RNG with seed %d\n", obj->seed);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "MPI tasks per instance: %d\n", sz / obj->ninstances);
  mpilog(obj->log, "Starting %d FFS instance%s...\n", obj->ninstances,
	 (obj->ninstances > 1) ? "s" : "");

  u_config_get_subkey(obj->input, FFS_CONFIG_FFS, &config);

  /* Generate instance seed by inst_id iterations of the
   * the 32-bit RNG initialised with master seed. */

  seed = obj->seed;

  mpi_errnol = ranlcg_create32(seed, &ran);
  mpi_sync_ifm(mpi_errnol, "ranlcg_create() failed\n");

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, obj->comm);
  nop_err_if(mpi_errno);

  for (n = 0; n < obj->inst_id; n++) {
    ranlcg_reep_int32(ran, &seed);
  }

  /* Instance create, start, execute, stop, close */

  err_err_if(ffs_inst_create(obj->inst_id, obj->comm, &obj->instance));
  err_err_if(ffs_inst_seed_set(obj->instance, seed));

  err_err_ifm(obj->ninstances > 9999, "Format botch (internal error)");

  if (obj->name == NULL) {
    err_err_if(u_string_create("default", strlen("default"), &obj->name));
  }
  err_err_if(u_string_aprintf(obj->name, "-inst-%4.4d.log", obj->inst_id));

  err_err_if(ffs_inst_start(obj->instance, u_string_c(obj->name), "w+"));
  err_err_if(ffs_inst_execute(obj->instance, config));

  dbg_err_if( ffs_result_summary_create(&obj->res) );
  dbg_err_if( ffs_inst_stop(obj->instance, obj->res) );

  /* Compound statistics TODO (before finishing off instances) */

  ffs_inst_free(obj->instance);
  ranlcg_free(ran);
  obj->instance = NULL;

  mpilog(obj->log, "Finished instances.\n");

  return 0;

 err:

  if (obj->res) ffs_result_summary_free(obj->res);
  if (obj->instance) ffs_inst_free(obj->instance);
  obj->instance = NULL;
  if (ran) ranlcg_free(ran);
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
  int mpi_errnol = 0;
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

  mpi_err_if_any(mpi_errnol, obj->comm);

  return 0;

 err:

  U_FCLOSE(fp);
  if (buf) U_FREE(buf);
  if (obj->input) u_config_free(obj->input);
  obj->input = NULL;

  mpilog(obj->log, "Please check and try again\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_broadcast_config
 *
 *  Also broadcasts "len"; but be careful with size_t.
 *
 *****************************************************************************/

static int ffs_broadcast_config(ffs_control_t * obj, size_t len) {

  int rank;
  int mpi_errnol = 0;

  unsigned int ilen = len;
  char * buf = NULL;

  MPI_Comm_rank(obj->comm, &rank);

  MPI_Bcast(&ilen, 1, MPI_UNSIGNED, 0, obj->comm);
  len = ilen;

  mpi_errnol = ((buf = u_calloc(len, sizeof(char))) == NULL);
  mpi_sync_ifm(mpi_errnol, "u_calloc(buf) failed");

  if (rank == 0) {
    mpi_errnol = u_config_save_to_buf(obj->input, buf, len);
    mpi_sync_ifm(mpi_errnol, "u_config_save_to_buf() failed");
  }

 mpi_sync:

  mpi_err_if_any(mpi_errnol, obj->comm);

  MPI_Bcast(buf, len, MPI_CHAR, 0, obj->comm);

  if (rank != 0) {
    mpi_errnol = u_config_load_from_buf(buf, len, &obj->input);
    err_ifm(mpi_errnol, "u_config_load_from_buf() failed");
  }

  mpi_err_if_any(mpi_errnol, obj->comm);

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
 *  If the details are acceptable, we can set the instance id via
 *  integer division
 *     inst_id = rank / mpi tasks per instance
 *  so inst id runs 0, ...
 *
 *****************************************************************************/

static int ffs_input_parse(ffs_control_t * obj) {

  int rank, ntask;
  int ifail;
  u_config_t * ffs = NULL;

  MPI_Comm_rank(obj->comm, &rank);
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

  /* Log the parameters received; sanity check parameters */

  ffs_control_log(obj);

  ifail = (obj->ninstances < 1);
  mpilog_if(ifail, obj->log, "Number of instances (%d) is less than one!\n",
	    obj->ninstances);
  dbg_err_ifm(ifail, "Number of instances < 1 (%d)", obj->ninstances);

  ifail = (ntask < obj->ninstances);
  mpilog_if(ifail, obj->log, "Requested more instances than MPI tasks\n");
  dbg_err_ifm(ifail, "Number of instances > MPI tasks (%d)", ntask);

  ifail = ((ntask % obj->ninstances) != 0);
  mpilog_if(ifail, obj->log, "Must have equal number of tasks per instance\n");
  dbg_err_ifm(ifail, "ntask % ninstance != 0"); 

  ifail = (obj->seed < 1);
  mpilog_if(ifail, obj->log, "%s must be > 0\n", FFS_CONFIG_FFS_SEED);
  dbg_err_ifm(ifail, "Bad seed (%d)", obj->seed);

  /* Set the instance id */

  obj->inst_id = rank / (ntask / obj->ninstances);

  return 0;

 err:

  mpilog(obj->log, "Please check input file and try again\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_control_log_to_mpilog
 *
 *  Log details to the specified log as a config object.
 *
 *****************************************************************************/

int ffs_control_log_to_mpilog(ffs_control_t * obj, mpilog_t * log) {

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
 *  ffs_control_log
 *
 *  Log config details to default log.
 *
 *****************************************************************************/

int ffs_control_log(ffs_control_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ffs_control_log_to_mpilog(obj, obj->log), -1);

  return 0;
}
