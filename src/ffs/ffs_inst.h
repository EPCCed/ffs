/*****************************************************************************
 *
 *  ffs_inst.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_INST_H
#define FFS_INST_H

#include <stdio.h>
#include <mpi.h>
#include "u/libu.h"

#include "../util/mpilog.h"
#include "../sim/proxy.h"
#include "ffs_param.h"
#include "ffs_result_summary.h"

/**
 *  \defgroup ffs_inst FFS instance
 *  \ingroup ffs_library
 *  \{
 *
 *  A single instance of FFS should run the FFS calculation using
 *  a selected FFS method, and a user-specified simulation (called
 *  via the simulation interface).
 *
 *  There are two levels of parallelism available. First, the FFS
 *  calculation can make use of mutliple simulation instances to
 *  make multiple trials. Second, the simulation itself may be run
 *  in parallel (if available).
 *
 *  This means that the total number of MPI tasks available to the
 *  instance (determined at run time) must be consistent with the
 *  number specified for the tasks per instance, the number of
 *  simualtions, and the tasks per simulation. This is checked at run time.
 *
 *  We expect the use to be, schematically:
 *
 *  \code
 *    u_config_t * config;        // To be supplied by caller
 *    ffs_inst_t * inst = NULL;
 *
 *    ffs_inst_create(inst_id, comm_parent, &inst);
 *    ffs_inst_start(inst, "instance.log", "w+");
 *    ffs_inst_execute(inst, config);
 *    ffs_inst_stop(inst, result);
 *
 *    ffs_inst_free(inst);
 *
 *  \endcode
 *
 *  Execution is driven by the apropriate u_config_t configuration object.
 *  This defines the following key value pairs.
 *
 *  \code
 *  ffs_inst
 *  {
 *    method           string      # "branched" or "direct" or "test"
 *
 *    sim_mpi_tasks    int         # mpi tasks for one simulation
 *    sim_name         string      # used to identify simulation
 *    sim_argv         string      # command line to be passed to simulation
 *    sim_lambda       string      # used to identify lambda calculation
 *
 *    init_independent flag        # Use independent (parallel) initial states
 *    init_ntrials     int         # Number of trials to first interface
 *    init_nstepmax    int         # Maximum length of run in state A
 *    init_nskip       int         # Crossing skip rate (steps)
 *    init_prob_accept double      # Crossing acceptance rate
 *    init_teq         double      # Equilibration time (simulation units)
 *
 *    trial_nstepmax    int        # Maximum length of trial (steps)
 *    trial_tmax        double     # Maximum time of trial (simulation units)
 *    trial_nsteplambda int        # Steps between lambda evaluations
 *
 *    seed0             int        # RNG seed for this instance
 *
 *  }
 *  \endcode
 *
 */

/**
 *  \def FFS_CONFIG_INST
 *  Key string for the instance config section
 *
 *  \def FFS_CONFIG_INST_METHOD
 *  Key string for the FFS algorithm to be used
 *
 *  \def FFS_CONFIG_INST_SEED
 *  Key string for the instance RNG seed
 */

#define FFS_CONFIG_INST               "ffs_inst"
#define FFS_CONFIG_INST_METHOD        "method"
#define FFS_CONFIG_INST_SEED          "seed"

/**
 *  \def FFS_CONFIG_TRIAL_NSTEPMAX
 *  Key for maximum number of simulations steps per trial run
 *
 *  \def FFS_CONFIG_TRIAL_NSTEPLAMBDA
 *  Key for trial steps between lambda evaulations
 *
 *  \def FFS_CONFIG_TRIAL_TMAX
 *  Key for maximum trial run length (simulation units)
 *
 *  \def FFS_DEFAULT_TRIAL_NSTEPMAX
 *  Default value
 *
 *  \def FFS_DEFAULT_TRIAL_NSTEPLAMBDA
 *  Default value
 */

#define FFS_CONFIG_TRIAL_NSTEPMAX     "trial_nstepmax"
#define FFS_CONFIG_TRIAL_TMAX         "trial_tmax"
#define FFS_CONFIG_TRIAL_NSTEPLAMBDA  "trial_nsteplambda"

#define FFS_DEFAULT_TRIAL_NSTEPMAX    1
#define FFS_DEFAULT_TRIAL_NSTEPLAMBDA 1

/**
 *  \def FFS_CONFIG_SIM_MPI_TASKS
 *  Key for number of MPI tasks per simulation
 *
 *  \def FFS_CONFIG_SIM_NAME
 *  Key for simulation name (to identify proxy)
 *
 *  \def FFS_CONFIG_SIM_ARGV
 *  Key for simulation 'command line argument' string
 *
 *  \def FFS_CONFIG_SIM_LAMBDA
 *  Key for simulation lambda computation
 *
 *  \def FFS_DEFAULT_SIM_MPI_TASKS
 *  Default number of MPI tasks per simulation
 */

#define FFS_CONFIG_SIM_MPI_TASKS      "sim_mpi_tasks"
#define FFS_CONFIG_SIM_NAME           "sim_name"
#define FFS_CONFIG_SIM_ARGV           "sim_argv"
#define FFS_CONFIG_SIM_LAMBDA         "sim_lambda"
#define FFS_DEFAULT_SIM_MPI_TASKS     1

/**
 * \def FFS_CONFIG_INIT_INDEPENDENT
 * Key for initialisation method (serial/parallel)
 *
 * \def FFS_CONFIG_INIT_NSTEPMAX
 * Key for maximum number of simulation steps in initialisation runs
 *
 * \def FFS_CONFIG_INIT_NSKIP
 * Key for number of crossings to skip at lambda_A in initialisations
 *
 * \def FFS_CONFIG_INIT_PROB_ACCEPT
 * Key for probability of acceptance of initial interface crossing
 *
 * \def FFS_CONFIG_INIT_TEQ
 * Key for equilibration time for initialisation runs
 *
 * \def FFS_CONFIG_INIT_NTRIALS
 * Key for initial trials
 *
 * \def FFS_DEFAULT_INIT_INDEPENDENT
 * Use parallel initialisation of states at lambda_A
 *
 * \def FFS_DEFAULT_INIT_NSTEPMAX
 * Should be specified by the user
 *
 * \def FFS_DEFAULT_INIT_NSKIP
 * Take every crossing as an initial state if 1
 *
 * \def FFS_DEFAULT_INIT_PROB_ACCEPT
 * Probability that crossing is accepted as initial state
 *
 * \def FFS_DEFAULT_INIT_TEQ
 * Should be specified by the user
 *
 * \def FFS_DEFAULT_INIT_NTRIALS
 * The number of trials used to generate states at the first interface
 */ 

#define FFS_CONFIG_INIT_INDEPENDENT   "init_independent"
#define FFS_CONFIG_INIT_NSTEPMAX      "init_nstepmax"
#define FFS_CONFIG_INIT_NSKIP         "init_nskip"
#define FFS_CONFIG_INIT_PROB_ACCEPT   "init_prob_accept"
#define FFS_CONFIG_INIT_TEQ           "init_teq"
#define FFS_CONFIG_INIT_NTRIALS       "init_ntrials"
#define FFS_DEFAULT_INIT_INDEPENDENT  1
#define FFS_DEFAULT_INIT_NSTEPMAX     0
#define FFS_DEFAULT_INIT_NSKIP        1
#define FFS_DEFAULT_INIT_PROB_ACCEPT  1.0
#define FFS_DEFAULT_INIT_TEQ          0.0
#define FFS_DEFAULT_INIT_NTRIALS      1

/**
 *  \def FFS_CONFIG_METHOD_TEST
 *  Value string for test method
 *  \def FFS_CONFIG_METHOD_DIRECT
 *  Value string for direct ffs method
 *  \def FFS_CONFIG_METHOD_BRANCHED
 *  Value string for branched ffs method
 *  \def FFS_CONFIG_METHOD_ROSENBLUTH
 *  Value string for Rosenbluth-like ffs sampling
 *  \def FFS_CONFIG_METHOD_BRUTE_FORCE
 *  Value string for brute force
 */

#define FFS_CONFIG_METHOD_TEST         "test"
#define FFS_CONFIG_METHOD_DIRECT       "direct"
#define FFS_CONFIG_METHOD_BRANCHED     "branched"
#define FFS_CONFIG_METHOD_ROSENBLUTH   "rosenbluth"
#define FFS_CONFIG_METHOD_BRUTE_FORCE  "brute_force"

/**
 *  \brief Opaque instance type
 */

typedef struct ffs_inst_type ffs_inst_t;

/**
 *  \brief FFS method
 */

enum ffs_inst_method {FFS_METHOD_TEST,
		      FFS_METHOD_DIRECT,
		      FFS_METHOD_BRANCHED,
                      FFS_METHOD_ROSENBLUTH,
                      FFS_METHOD_BRUTE_FORCE};

/**
 *  \brief Create an FFS instance
 *
 *  \param id         Unique instance id
 *  \param comm       Parent communicator
 *  \param pobj       a pointer to the object to be returned
 *
 *  The parent communicator will be split on the basis of the instance
 *  id passed to the call via MPI_Comm_split(). The return code will be
 *  consistent on all ranks in the parent communicator.
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int ffs_inst_create(int id, MPI_Comm comm, ffs_inst_t ** pobj);

/**
 *  \brief Release an FFS instance after use
 *
 *  \param obj        the object to be deallocated
 *
 *  \return           nothing
 *
 */

void ffs_inst_free(ffs_inst_t * obj);

/**
 *  \brief Return the instance id
 *
 *  \param  obj       the ffs_inst_t object
 *  \param  inst_id   a pointer to the integer id
 *
 *  \retval 0         a success
 *  \retval -1        a NULL point was received
 */

int ffs_inst_id(ffs_inst_t * obj, int * inst_id);

/**
 *  \brief Start the instance logging using fopen()-like arguments
 *
 *  \param obj        the FFS instance object
 *  \param filename   the log file name for this instance
 *  \param mode       an fopen()-like mode argument
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 *  The parent is resposible for co-ordinating a sensible log file name 
 *  on each MPI rank.
 */

int ffs_inst_start(ffs_inst_t * obj, const char * filename, const char * mode);

/**
 *  \brief Close the instance log and other house-keeping at end
 *
 *  \param  obj       the FFS instance object
 *  \param  result    a summary structure (may be NULL)
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_inst_stop(ffs_inst_t * obj, ffs_result_summary_t * result);
  
/**
 *  \brief Initialise the instance from configuration
 *
 *  \param  obj      the ffs_inst_t object
 *  \param  config   u_config_t object with FFS_CONFIG_INST section
 *
 *  \retval 0        a success
 *  \retval -1       a failure (all tasks)
 *
 */

int ffs_inst_execute(ffs_inst_t * obj, u_config_t * config);

/**
 *  \brief Initialise via configuration object
 *
 *  \param obj         a previously created ffs_inst_t
 *  \param config      a u_config_t configuration
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 */

int ffs_inst_init_from_config(ffs_inst_t * obj, u_config_t * config);

/**
 *  \brief Log the instance details to given mpilog object
 *
 *  \param obj      the instance object
 *  \param log      the mpilog_t object
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_inst_config_log(ffs_inst_t * obj, mpilog_t * log);

/**
 *  \brief Log instance details to the internal log
 *
 *  \param obj      the instance
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_inst_config(ffs_inst_t * obj);

/**
 *  \brief Return the FFS method name
 *
 *  \param  obj    the ffs_inst_t object
 *
 *  \retval        [ test | branched | direct ] on success
 *  \retval        [ null | unrecognised ] on failure
 */

const char * ffs_inst_method_name(ffs_inst_t * obj);

/**
 *  \brief Return a reference to the instance parameters
 *
 *  \param obj      the ffs_inst_t data type
 *  \param param    a pointer to the ffs_param_t data type
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received
 */

int ffs_inst_param(ffs_inst_t * obj, ffs_param_t ** param);

/**
 *  \brief  Return number of simulation instances
 *
 *  \param  obj     the ffs_inst_t data type
 *  \param  nsim    a pointer to the integer number to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received
 */

int ffs_inst_nsim(ffs_inst_t * obj, int * nsim);

/**
 *  \brief Set the RNG seed for this instance
 *
 *  \param  obj      the ffs_inst_t structure
 *  \param  seed     32-bit integer RNG seed
 *
 *  \retval 0        a success
 *  \retval -1       a failure (seed must be 0 < seed < INT_MAX)
 */

int ffs_inst_seed_set(ffs_inst_t * obj, int seed);

/**
 *  \brief Obtain a reference to the the instance proxy (this MPI task)
 *
 *  \param  obj       the ffs_inst_t structure
 *  \param  ref       a pointer to the proxy_t to be returned
 *
 *  \retval 0         a success
 *  \retval -1        an argument was NULL
 *
 *  If the instance proxy has not been started, or has been stopped,
 *  ref will be NULL.
 */

int ffs_inst_proxy(ffs_inst_t * obj, proxy_t ** ref);

/**
 *  \brief Start the simualtion proxy for the instance
 *
 *  \param obj      the ffs_inst_t
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *  The instance should have been initialised via, e.g,
 *  ffs_inst_init_from_config()
 */

int ffs_inst_start_proxy(ffs_inst_t * obj);

/**
 *  \brief Stop the simulation proxy
 *
 *  \param  obj        the ffs_inst_t structure
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 *  The proxy should have been previously initialised via
 *  ffs_inst_start_proxy()
 */

int ffs_inst_stop_proxy(ffs_inst_t * obj);

/**
 * \}
 */

#endif /* FFS_INST_H */
