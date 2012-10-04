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
 *    ffs_inst_stop(inst);
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
 *    method      string       # "branched" or "direct" or "test"
 *    mpi_tasks   int          # number of MPI tasks for this FFS instance
 *    nsim        int          # number of simulation instances to use
 *
 *    init_independent flag    # Use independent initial states
 *    init_skip_rate   int     # Crossing skip rate
 *    init_prob_accept double  # Crossing acceptance rate
 *    init_teq         double  # Equilibration time
 *
 *    trial_nstepmax    int    # Maximum length of trial (steps)
 *    trial_tmax        double # Maximum time of trial (simulation units)
 *
 *    seed0             int    # RNG seed for this instance
 *
 *  }
 *  \endcode
 *
 *  An 
 */

/**
 *  \def FFS_CONFIG_INST
 *  Key string for the instance config section
 *  \def FFS_CONFIG_INST_METHOD
 *  Key string for the FFS algorithm to be used
 *  \def FFS_CONFIG_INST_NTASK
 *  Key for the number of MPI tasks requested for this simulation
 *  \def FFS_CONFIG_INST_NSIM
 *  Key string for number of simulations
 *
 *  \def FFS_DEFAULT_INST_NSIM
 *  Default number of simulations of instance
 *
 *  \def FFS_CONFIG_METHOD_TEST
 *  Value string for test method
 *  \def FFS_CONFIG_METHOD_DIRECT
 *  Value string for direct ffs method
 *  \def FFS_CONFIG_METHOD_BRANCHED
 *  Value string for branched ffs method
 */

#define FFS_CONFIG_INST               "ffs_inst"
#define FFS_CONFIG_INST_METHOD        "method"
#define FFS_CONFIG_INST_NTASK         "mpi_tasks"
#define FFS_CONFIG_INST_NSIM          "nsim"
#define FFS_CONFIG_INST_SEED          "seed"
#define FFS_CONFIG_INIT_INDEPENDENT   "init_independent"
#define FFS_CONFIG_INIT_SKIP_RATE     "init_skip_rate"
#define FFS_CONFIG_INIT_PROB_ACCEPT   "init_prob_accept"
#define FFS_CONFIG_INIT_TEQ           "init_teq"
#define FFS_CONFIG_TRIAL_NSTEPMAX     "trial_nstepmax"
#define FFS_CONFIG_TRIAL_TMAX         "trial_tmax"
#define FFS_CONFIG_SIM_NAME          "sim_name"
#define FFS_CONFIG_SIM_ARGV          "sim_argv"

#define FFS_DEFAULT_INST_NSIM          1

#define FFS_CONFIG_METHOD_TEST         "test"
#define FFS_CONFIG_METHOD_DIRECT       "direct"
#define FFS_CONFIG_METHOD_BRANCHED     "branched"

/**
 *  \brief Opaque instance type
 */

typedef struct ffs_inst_type ffs_inst_t;

/**
 *  \brief FFS method
 */

enum ffs_inst_method {FFS_METHOD_TEST,
		      FFS_METHOD_DIRECT,
		      FFS_METHOD_BRANCHED};

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
 *  \brief Close the instance log and otther house-keeping at end
 *
 *  \param  obj       the FFS instance object
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_inst_stop(ffs_inst_t * obj);
  
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
 *  \brief Initialise FFS parameters from configuration
 *
 *  \param obj         the FFS instance
 *  \param config      an FFS_CONFIG_PARAM configuration
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 */

int ffs_inst_param_init(ffs_inst_t * obj, u_config_t * config);

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
 * \}
 */

#endif /* FFS_INST_H */
