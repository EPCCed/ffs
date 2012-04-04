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

/**
 *  \defgroup ffs_library FFS library
 *  \{
 *    The Forward Flux Sampling Library
 *  \}
 */

/**
 *  \defgroup ffs_control FFS controller
 *  \ingroup ffs_library
 *  \{
 *    This is the top-level FFS library object
 *
 *    \par
 *
 *    The object may be instantiated on its own, but should
 *    be initialised by the relevant section of the u_config_t
 *    input file:
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
 *    and there must be a whole number of MPI tasks per
 *    instance.
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
 *  \brief Initialise ffs_control_t object from u_config_t file
 *
 *  \param  obj        the ffs_control_t object
 *  \param  filename   appropriate u_config_t file
 *
 *  \retval 0          a success
 *  \retval -1         a failure (all tasks in obj->comm)
 */

int ffs_control_init(ffs_control_t * obj, const char * filename);

/**
 *  \brief Run the FFS calculation
 *
 *  \param obj      ffs_control_t object
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_control_run(ffs_control_t * obj);

/**
 *  \brief Close the FFS calculation
 *
 *  \param  obj         the ffs_control_t object
 *
 *  \retval 0           a success
 *  \retval -1          a failure
 */

int ffs_control_finish(ffs_control_t * obj);

/**
 *
 *  \brief Output a human readable summary
 *
 *  \param  obj     the ffs_control_t object
 *  \param  fp      stream
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_control_print_summary_fp(ffs_control_t * obj, FILE * fp); 

/**
 *  \}
 */

#endif /* FFS_CONTROL_H */
