/*****************************************************************************
 *
 *  ffs_control.h
 *
 *  Top-level FFS object.
 *
 *****************************************************************************/

#ifndef FFS_CONTROL_H
#define FFS_CONTROL_H

#include <stdio.h>
#include <mpi.h>

#include "../util/mpilog.h"
#include "ffs_result_summary.h"

/**
 *  \defgroup ffs_library FFS library
 *  \{
 *    The Forward Flux Sampling Library
 *  \}
 */

/**
 *  \defgroup ffs_control FFS control
 *  \ingroup ffs_library
 *  \{
 *    This is the top-level FFS library object and controls the FFS in
 *    parallel using the message passing interface.
 *
 *    We expect use to look something like the following:
 *    \code
 *      ffs_control_t * ffs = NULL;
 *      MPI_Init(&argv, &argc);
 *
 *      ffs_control_create(MPI_COMM_WORLD, &ffs);
 *
 *      ffs_control_start(ffs, "name");
 *      ffs_control_execute(ffs, "input.config");
 *      ffs_control_stop(ffs, result);
 *
 *      ffs_control_free(ffs);
 *      MPI_Finalize();
 *    \endcode
 *
 *    The start operation is required to set a name for the instance
 *    log files, or else a default ("default") will be used.
 *    The input file input.config is a configuration file following
 *    the format of the libu config object.
 *
 *    The entire execution of the FFS calculation is undertaken by a
 *    call to ffs_control_execute(). All these calls are collective
 *    in the parent communicator (here \c MPI_COMM_WORLD), although a
 *    duplicate communicator is used internally. 
 *
 *    The relevant section of the u_config_t input file should contain
 *    the following parameters, e.g.,:
 *
 *    \code
 *    ffs
 *    {
 *       ffs_instances        1   # the number of independent instances
 *       ffs_seed            13   # overall random number seed
 *    }
 *    \endcode
 *    The number of instances must fit in the number of MPI tasks
 *    available (ie., the number in the \c parent communicator),
 *    and there must be a whole number of MPI tasks per instance.
 *
 *    The configuration file must have an \c ffs_inst section and
 *    an \c interfaces section which
 *    must not be empty for correct execution. See the \ref ffs_param
 *    section. A bare minimum configuration file includes:
 *    \code
 *    ffs_inst
 *    {
 *      method   test
 *    }
 *    interfaces
 *    {
 *      nlambda  0
 *    }
 *    \endcode
 *
 *    Note that empty configuration blocks {} can prevent correct
 *    parsing of the file, so please avoid them.
 */

/**
 *  \def FFS_CONFIG_FFS
 *  Key string for the ffs control section
 *  \def FFS_CONFIG_FFS_INSTANCES
 *  Key string for the number of FFS instances
 *  \def FFS_CONFIG_FFS_SEED
 *  Key string for master random number seed
 *
 *  \def FFS_DEFAULT_FFS_INSTANCES
 *  Default number of instances
 *  \def FFS_DEFAULT_FFS_SEED
 *  Default random number seed
 */

#define FFS_CONFIG_FFS            "ffs"
#define FFS_CONFIG_FFS_INSTANCES  "ffs_instances"
#define FFS_CONFIG_FFS_SEED       "ffs_seed"

#define FFS_DEFAULT_FFS_INSTANCES 1
#define FFS_DEFAULT_FFS_SEED      1

/**
 *  \brief Opaque ffs_control_t object.
 */

typedef struct ffs_control_type ffs_control_t;

/**
 *  \brief Create an FFS control object with parent communicator
 *
 *  This will duplicate the supplied communicator to use as the
 *  basis of library communications.
 *
 *  \param  parent      an existing MPI Communicator
 *  \param  pobj        a pointer to the new object to be returned
 *
 *  \retval 0           a success
 *  \retval -1          a failure (all tasks in parent)
 *
 */

int ffs_control_create(MPI_Comm parent, ffs_control_t ** pobj);

/**
 *  \brief Release an existing object
 *
 *  The caller is responsible for any resources associated with the
 *  parent communicator.
 *
 *  \param  obj      the object to be deallocated
 *
 *  \returns         void
 */

void ffs_control_free(ffs_control_t * obj);

/**
 *  \brief Drive FFS calculation from contents of u_config_t file
 *
 *  \param  obj              the ffs_control_t object
 *  \param  configfilename   appropriate u_config_t file
 *
 *  \retval 0                a success
 *  \retval -1               a failure (all tasks in obj->comm)
 */

int ffs_control_execute(ffs_control_t * obj, const char * configfilename);

/**
 *  \brief Start the log file using fopen-like arguments
 *
 *  \param  obj             the ffs_control_t object
 *  \param  runname         a name for this FFS run
 *
 *  \retval 0               a success
 *  \retval -1              a failure
 */

int ffs_control_start(ffs_control_t * obj, const char * runname);

/**
 *  \brief Stop the log at end of calculation
 *
 *  \param obj      ffs_control_t object
 *  \param result   a summary summary structure (may be NULL)
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *  Should be preceeded by a call to ffs_control_start().
 */

int ffs_control_stop(ffs_control_t * obj, ffs_result_summary_t * result);

/**
 *  \brief Log details of config to log
 *
 *  \param obj         the ffs control object
 *  \param log         an mpilog_t log
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 */

int ffs_control_log_to_mpilog(ffs_control_t * obj, mpilog_t * log);

/**
 *  \brief Log details of config to the default log
 *
 *  \param obj         the ffs control object
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 */

int ffs_control_log(ffs_control_t * obj);


/**
 *  \}
 */

#endif /* FFS_CONTROL_H */
