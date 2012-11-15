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

#include "ffs_init.h"
#include "ffs_private.h"
#include "ffs_result.h"
#include "ffs_branched.h"
#include "ffs_inst.h"

struct ffs_inst_type {

  int inst_id;          /* Unique id */
  int method;           /* ffs_method_enum */
  int seed;             /* RNG seed */
  MPI_Comm parent;      /* Parent communicator */
  MPI_Comm comm;        /* FFS instance communicator */
  ffs_param_t * param;  /* Parameters (interfaces) */
  mpilog_t * log;       /* Instance log */

  /* Simulation details */
  int sim_nsim_inst;       /* Number of simulation instances */
  u_string_t * sim_name;   /* The name used to identify the proxy */
  u_string_t * sim_argv;   /* The command line */

  /* Initialisation parameters */
  ffs_init_t * init;
};

static int ffs_inst_read_config(ffs_inst_t * obj, u_config_t * input);
static int ffs_inst_read_init(ffs_inst_t * obj, u_config_t * config);
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

  mpi_errnol = ffs_init_create(&obj->init);
  mpi_sync_ifm(mpi_errnol, "ffs_init_create_failed\n");

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
  if (obj->init) ffs_init_free(obj->init);
  if (obj->param) ffs_param_free(obj->param);
  if (obj->sim_name) u_string_free(obj->sim_name);
  if (obj->sim_argv) u_string_free(obj->sim_argv);

  MPI_Comm_free(&obj->comm);
  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_inst_id
 *
 *****************************************************************************/

int ffs_inst_id(ffs_inst_t * obj, int * id) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(id == NULL, -1);

  *id = obj->inst_id;

  return 0;
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
 *  Parse the ffs_inst section of the config. It is assumed every
 *  rank receives exactly the same config, so failures are collective.
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
  err_err_ifm(ifail, "No inst config");

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

  /* Simulation name, number of instances, and command line */

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

  /* Initialisation section */

  err_err_if(ffs_inst_read_init(obj, config));

  /* Interface section */

  ifail = ((config = u_config_get_child(input, FFS_CONFIG_INTERFACES)) == NULL);
  mpilog_if(ifail, obj->log, "Config has no %s\n", FFS_CONFIG_INTERFACES);
  err_err_ifm(ifail, "No inst interfaces");

  err_err_if(ffs_param_create(config, &obj->param));

  /* Report */

  ffs_inst_config(obj);

  return 0;

 err:

  mpilog(obj->log, "Problem parsing the instance config\n");

  return -1;
}

/*****************************************************************************
 *
 *  ffs_inst_read_init
 *
 *  Parse the initialisation parameters.
 *
 *  The config object should have stripped off the ffs_init { }
 *  at this stage.
 *
 *  We assume every rank receives exactly the same config, so
 *  errors are collective.
 *
 *****************************************************************************/

static int ffs_inst_read_init(ffs_inst_t * obj, u_config_t * config) {

  int init_independent = 0;    /* Use independent (parallel) states at A */
  int init_nskip = 0;          /* Skip every n crossings at A */
  int init_nstepmax = 0;       /* Maximum length of initial run */
  int init_nsteplambda = 1;    /* Steps between lambda evalutations */ 
  double prob_accept = 0.0;    /* Probability of accepting crossing at A */
  double teq = 0.0;            /* Equilibration time in A */

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(config == NULL, -1);

  err_err_if(u_config_get_subkey_value_b(config,
					 FFS_CONFIG_INIT_INDEPENDENT,
					 FFS_DEFAULT_INIT_INDEPENDENT,
					 &init_independent));

  err_err_if(u_config_get_subkey_value_i(config,
					 FFS_CONFIG_INIT_NSTEPMAX,
					 FFS_DEFAULT_INIT_NSTEPMAX,
					 &init_nstepmax));

  err_err_if(u_config_get_subkey_value_i(config,
					 FFS_CONFIG_INIT_NSKIP,
					 FFS_DEFAULT_INIT_NSKIP,
					 &init_nskip));

  err_err_if(util_config_get_subkey_value_d(config,
					    FFS_CONFIG_INIT_PROB_ACCEPT,
					    FFS_DEFAULT_INIT_PROB_ACCEPT,
					    &prob_accept));

  err_err_if(util_config_get_subkey_value_d(config,
					    FFS_CONFIG_INIT_TEQ,
					    FFS_DEFAULT_INIT_TEQ,
					    &teq));

  ffs_init_independent_set(obj->init, init_independent);
  ffs_init_nstepmax_set(obj->init, init_nstepmax);
  ffs_init_nskip_set(obj->init, init_nskip);
  ffs_init_prob_accept_set(obj->init, prob_accept);
  ffs_init_teq_set(obj->init, teq);
  ffs_init_nsteplambda_set(obj->init, init_nsteplambda);

  return 0;

 err:

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

  const char * fmts = "\t %-20s  %s\n";
  const char * fmti = "\t %-20s  %d\n";

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj == NULL, -1);

  mpilog(log, "\n");
  mpilog(log, "%s\n", FFS_CONFIG_INST);
  mpilog(log, "{\n");
  mpilog(log, fmts, FFS_CONFIG_INST_METHOD, ffs_inst_method_name(obj));
  mpilog(log, fmti, FFS_CONFIG_SIM_NSIM, obj->sim_nsim_inst);
  mpilog(log, fmts, FFS_CONFIG_SIM_NAME, u_string_c(obj->sim_name));
  mpilog(log, fmts, FFS_CONFIG_SIM_ARGV, u_string_c(obj->sim_argv));
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

  int rank, sz;            /* MPI rank, size */
  int perproxy;            /* MPI tasks per proxy */
  int proxy_id;            /* Computed proxy id */
  int nlambda;
  int ntrial;
  ffs_t * ffs = NULL;
  proxy_t * proxy = NULL;
  ffs_result_t * result = NULL; /* To be supplied by caller in future */

  dbg_return_if(obj == NULL, -1);

  /* Check input for branched */

  mpilog(obj->log, "Checking branched input\n");
  mpilog(obj->log, "Setting pruning probabilities to default values\n");

  err_err_if(ffs_param_check(obj->param));
  err_err_if(ffs_param_pprune_set_default(obj->param));

  /* Interface details to log */

  mpilog(obj->log, "\n");
  mpilog(obj->log, "The interface parameters are as follows\n");

  ffs_param_log_to_mpilog(obj->param, obj->log);
  ffs_init_log_to_mpilog(obj->init, obj->log);  

  /* Results object */

  ffs_param_nlambda(obj->param, &nlambda);
  ffs_param_nstate(obj->param, 1, &ntrial);
  ntrial = ntrial / obj->sim_nsim_inst;
  ffs_result_create(nlambda, ntrial, &result);

  /* Start proxy */

  MPI_Comm_size(obj->comm, &sz);
  MPI_Comm_rank(obj->comm, &rank);
  perproxy = sz / obj->sim_nsim_inst;
  err_err_if(perproxy == 0); /* SHOULD HAVE BEEN CHECKED ! */
  proxy_id = rank / perproxy;

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Starting the simulation proxy (%d instance%s)\n",
	 obj->sim_nsim_inst, obj->sim_nsim_inst > 1 ? "s" : "");
  mpilog(obj->log, "Tasks per (proxy) simulation: %d\n", perproxy);

  err_err_if(proxy_create(proxy_id, obj->comm, &proxy));
  err_err_if(proxy_delegate_create(proxy, u_string_c(obj->sim_name)));
  err_err_if(proxy_ffs(proxy, &ffs));
  err_err_if(ffs_command_line_set(ffs, u_string_c(obj->sim_argv)));

  mpilog(obj->log, "The simulation is %s\n", u_string_c(obj->sim_name));

  /* Do the run */

  ffs_branched_run(obj->init, obj->param, proxy,
		   obj->inst_id, obj->sim_nsim_inst,
		   obj->seed, obj->log, result);

  mpilog(obj->log, "Closing down the simulation proxy\n");

  err_err_if(proxy_delegate_free(proxy));
  proxy_free(proxy);

  mpilog(obj->log, "\n");
  mpilog(obj->log, "Instance results\n");


  int n;
  double tmax, tsum, wt, lambda;

  /* Total trials this instance */
  ffs_param_nstate(obj->param, 1, &ntrial);

  ffs_result_reduce(result, obj->comm, 0);

  for (n = 1; n <= nlambda; n++) {
    ffs_param_lambda(obj->param, n, &lambda);
    ffs_result_weight(result, n, &wt);
    mpilog(obj->log, "%3d %11.4e %11.4e\n", n, lambda, wt/ntrial);
  }
  ffs_result_tmax(result, &tmax);
  ffs_result_tsum(result, &tsum);
  ffs_result_ncross(result, &n);

  mpilog(obj->log, "Initial Tmax:  result   %12.6e\n", tmax);
  mpilog(obj->log, "Initial Tsum:  result   %12.6e\n", tsum);
  mpilog(obj->log, "Number of crossings A:  %d\n", n);
  mpilog(obj->log, "Flux at lambda_A:       %12.6e\n", n/tsum);
  mpilog(obj->log, "Probability P(B|A):     %12.6e\n", wt/ntrial);
  mpilog(obj->log, "Flux * P(B|A):          %12.6e\n", (n/tsum)*wt/ntrial);


  ffs_result_free(result);

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

/*****************************************************************************
 *
 *  ffs_inst_param
 *
 *****************************************************************************/

int ffs_inst_param(ffs_inst_t * obj, ffs_param_t ** param) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(param == NULL, -1);

  *param = obj->param;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_inst_nsim
 *
 *****************************************************************************/

int ffs_inst_nsim(ffs_inst_t * obj, int * nsim) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nsim == NULL, -1);

  *nsim = obj->sim_nsim_inst;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_inst_seed_set
 *
 *****************************************************************************/

int ffs_inst_seed_set(ffs_inst_t * obj, int seed) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(seed < 0, -1);

  obj->seed = seed;

  return 0;
}
