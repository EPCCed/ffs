/*****************************************************************************
 *
 *  ffs_private.h
 *
 *****************************************************************************/

#ifndef FFS_PRIVATE_H
#define FFS_PRIVATE_H

#include <mpi.h>
#include "ffs.h"

/**
 *  \defgroup ffs_private FFS mediator private
 *  \ingroup ffs_library
 *  \{
 */

#define FFS_SIM_EXECUTABLE_NAME "./a.out"

/**
 *  \brief Create object
 *
 *  \param comm       the communicator for the object (simulation communicator)
 *  \param pobj       a pointer to the new object to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_create(MPI_Comm comm, ffs_t ** pobj);

/**
 *  \brief Release the object
 *
 *  \param obj       the object
 *
 *  \returns         nothing
 */

void ffs_free(ffs_t * obj);

/**
 *  \brief Set the simulation command line
 *
 */

int ffs_command_line_set(ffs_t * obj, char * argstring);

/**
 *  \}
 */

#endif
