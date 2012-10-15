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
#include "../sim/proxy.h"

#include "ffs_param.h"
#include "ffs_inst.h"

struct ffs_inst_type {
  int inst_id;          /* Unique id */
  int method;           /* ffs_method_enum */
  MPI_Comm parent;      /* Parent communicator */
  MPI_Comm comm;        /* FFS instance communicator */
  ffs_param_t * param;  /* Parameters (interfaces) */
  mpilog_t * log;       /* Instance log */

  /* Simulation details */
  int sim_nsim_inst;       /* Number of simulation instances */
  u_string_t * sim_name;   /* The name used to identify the proxy */
  u_string_t * sim_argv;   /* The command line */
};

static int ffs_inst_read_config(ffs_inst_t * obj, u_config_t * input);
static int ffs_inst_branched(ffs_inst_t * obj);
static int ffs_inst_direct(ffs_inst_t * obj);

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

  obj->inst_id = id;
  obj->parent = parent;

  MPI_Comm_rank(obj->parent, &rank);
  MPI_Comm_split(obj->parent, obj->inst_id, rank, &obj->comm);

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

  if (obj->log) mpilog_free(obj->log);
  if (obj->param) ffs_param_free(obj->param);
  if (obj->sim_name) u_string_free(obj->sim_name);
  if (obj->sim_argv) u_string_free(obj->sim_argv);

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
  mpilog(obj->log, "FFS instance log for instance id %d\n", obj->inst_id);
  mpilog(obj->log, "Running on %d MPI task%s\n", sz, (sz > 1) ? "s" : "");

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
  mpilog_fclose(obj->log);

  return 0;
}

/*****************************************************************************
 *
 *  ffs_inst_execute
 *
 *****************************************************************************/

int ffs_inst_execute(ffs_inst_t * obj, u_config_t * input) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(input == NULL, -1);

  err_err_if(ffs_inst_read_config(obj, input));
  err_err_if(ffs_param_create(input, &obj->param));

  switch (obj->method) {
  case FFS_METHOD_TEST:
    /* Instant success! */
    mpilog(obj->log, "Test method does nothing\n");
    break;
  case FFS_METHOD_BRANCHED:
    mpilog(obj->log, "Branched FFS was selected\n");
    err_err_if(ffs_inst_branched(obj));
    break;
  case FFS_METHOD_DIRECT:
    mpilog(obj->log, "Direct FFS was selected\n");
    err_err_if(ffs_inst_direct(obj));
    break;
  default:
    err_err("Internal error: no method");
  }

  mpilog(obj->log, "Finishing instance execution.\n");

  ffs_param_free(obj->param);
  obj->param = NULL;

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_read_config
 *
 *  Look for the method, the number of mpi_tasks requested, and
 *  the number of simulations requested.
 *
 *  Also retreive the simulation (proxy) name, and the command
 *  line strings.
 *
 *  Any check of these data is deferred until the relevant time.
 *
 *****************************************************************************/

static int ffs_inst_read_config(ffs_inst_t * obj, u_config_t * input) {

  int ifail;
  u_config_t * config = NULL;
  const char * method;
  const char * sim_name;
  const char * sim_argv;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(input == NULL, -1);

  /* Parse input */

  ifail = ((config = u_config_get_child(input, FFS_CONFIG_INST)) == NULL);
  mpilog_if(ifail, obj->log, "Config has no %s section\n", FFS_CONFIG_INST);
  err_err_ifm(ifail, "No config");

  /* Set method; default is the test method */

  obj->method = FFS_METHOD_TEST;

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
  }
  else {
    /* The requested method was not recognised */
    err_err("%s (%s) not recognised in %s\n", FFS_CONFIG_INST_METHOD, method,
	    FFS_CONFIG_INST);
  }

  /* Simulation and command line */

  err_err_if(u_config_get_subkey_value_i(config,
					 FFS_CONFIG_SIM_NSIM,
					 FFS_DEFAULT_SIM_NSIM,
					 &obj->sim_nsim_inst));

  sim_name = u_config_get_subkey_value(config, FFS_CONFIG_SIM_NAME);
  if (sim_name == NULL) sim_name = "test";

  sim_argv = u_config_get_subkey_value(config, FFS_CONFIG_SIM_ARGV);
  if (sim_argv == NULL) sim_argv = "";

  err_err_if(u_string_create(sim_name, strlen(sim_name), &obj->sim_name));
  err_err_if(u_string_create(sim_argv, strlen(sim_argv), &obj->sim_argv));

  /* Report */

  ffs_inst_config(obj);

  return 0;

 err:

  mpilog(obj->log, "Problem parsing the instance config\n");

  return -1;
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
  mpilog(log, "\t%s\t%d\n", FFS_CONFIG_SIM_NSIM, obj->sim_nsim_inst);
  mpilog(log, "\t%s\t%s\n", FFS_CONFIG_SIM_NAME, u_string_c(obj->sim_name));
  mpilog(log, "\t%s\t%s\n", FFS_CONFIG_SIM_ARGV, u_string_c(obj->sim_argv));
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

/*****************************************************************************
 *
 *  ffs_inst_branched
 *
 *****************************************************************************/

static int ffs_inst_branched(ffs_inst_t * obj) {

  int rank, proxy_id;
  proxy_t * proxy = NULL;

  dbg_return_if(obj == NULL, -1);

  /* Check input for branched */

  mpilog(obj->log, "Checking branched input\n");

  /* Interface details to log */


  /* Start proxy */

  mpilog(obj->log, "Starting the simulation proxy\n");

  MPI_Comm_rank(obj->comm, &rank);
  proxy_id = rank / obj->sim_nsim_inst;

  err_err_if(proxy_create(proxy_id, obj->comm, &proxy));
  err_err_if(proxy_delegate_create(proxy, u_string_c(obj->sim_name)));

  /* Do the run */

  /* mpi_errnol = ffs_branched_run();
     mpi_sync_if(mpi_errnol); */

  mpilog(obj->log, "Closing down the simulation proxy\n");

  err_err_if(proxy_delegate_free(proxy));
  proxy_free(proxy);

  return 0;

 err:

  if (proxy) proxy_free(proxy);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_direct
 *
 *****************************************************************************/

static int ffs_inst_direct(ffs_inst_t * obj) {

  dbg_return_if(obj == NULL, -1);

  mpilog(obj->log, "DIRECT FFS NOT VERY INTERSTING YET\n");

  return 0;
}
