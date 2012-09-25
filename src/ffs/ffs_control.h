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
 *      ffs_control_start(ffs, "control.log", "w+");
 *      ffs_control_execute(ffs, "input.config");
 *      ffs_control_stop(ffs);
 *
 *      ffs_control_free(ffs);
 *      MPI_Finalize();
 *    \endcode
 *
 *    The start of logging is a separate operation as this needs to
 *    be initiated before any attempt to read and parse the input file.
 *    The input file input.config is a configuration file following
 *    the format of the libu config object.
 *
 *    The entire execution of the FFS calculation is undertaken by a
 *    call to ffs_control_execute(). All these calls are collective
 *    in the parent communicator (here MPI_COMM_WORLD), although a
 *    duplicate communicator is used internally. 
 *
 *    The relevant section of the u_config_t input file should contain
 *    the following parameters, e.g.,:
 *
 *    \code
 *    ffs
 *    {
 *       ffs_instances           # the number of independent instances
 *       ffs_seed                # overall random number seed
 *    }
 *    \endcode
 *    The number of instances must fit in the number of MPI tasks
 *    available (ie., the number in the \c parent communicator),
 *    and there must be a whole number of MPI tasks per instance.
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
 *  \param  obj        the ffs_control_t object
 *  \param  filename   appropriate u_config_t file
 *
 *  \retval 0          a success
 *  \retval -1         a failure (all tasks in obj->comm)
 */

int ffs_control_execute(ffs_control_t * obj, const char * filename);

/**
 *  \brief Start the log file using fopen-like arguments
 *
 *  \param  obj         the ffs_control_t object
 *  \param  filename    the log file name
 *  \param  mode        an fopen()-like argument for initial file mode
 *
 *  \retval 0           a success
 *  \retval -1          a failure
 */

int ffs_control_start(ffs_control_t * obj, const char * filename,
		      const char * mode);

/**
 *  \brief Stop the log at end of calculation
 *
 *  \param obj      ffs_control_t object
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *  Must be preceeded by a call to ffs_control_log_start().
 */

int ffs_control_stop(ffs_control_t * obj);

/**
 *  \brief Log details of config to log
 *
 *  \param obj         the ffs control object
 *  \param log         an MPI log object
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 */

int ffs_control_config_log(ffs_control_t * obj, mpilog_t * log);

/**
 *  \brief Log details of config to the default log
 *
 *  \param obj         the ffs control object
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 */

int ffs_control_config(ffs_control_t * obj);


/**
 *  \}
 */

#endif /* FFS_CONTROL_H */
