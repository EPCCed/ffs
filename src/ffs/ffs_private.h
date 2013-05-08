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

/**
 *  \brief Dummy executable name for simulation command line arguments
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
 *  \return         nothing
 */

void ffs_free(ffs_t * obj);

/**
 *  ffs_lambda_name_set
 *
 *  \param  obj      the ffs_t object
 *  \param  name     the name to identify lambda function in simulation
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer was received or failed to allocate
 *                   new string correctly
 */

int ffs_lambda_name_set(ffs_t * obj, const char * name);

/**
 *  \brief Set the simulation command line
 *
 *  \param obj          the ffs_t data type
 *  \param argstring    a string which will make up the command line
 *
 *  \retval 0           a success
 *  \retval -1          a NULL pointer was received or tokenisation
 *                      of supplied string failed
 */

int ffs_command_line_set(ffs_t * obj, const char * argstring);

/**
 *  \brief Reset the command line arguments
 *
 *  \param obj          the ffs_t object
 *  \param argstring    a string containing the new command line
 *
 *  \retval 0           a success
 *  \retval -1          a NULL pointer was provided, or the new
 *                      command line could not be created
 *
 */

int ffs_command_line_reset(ffs_t * obj, const char * argstring);

/**
 *  \brief Examine type information
 */

int ffs_type(ffs_t * obj, ffs_info_enum_t param, int * ndata,
	     ffs_var_enum_t * type);

/**
 *  \brief Return current lambda as a double
 *
 *  \param obj      the ffs_t object
 *  \param lambda   a pointer to the double value to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received or the lambda data
 *                  type has not been set via a call to ffs_type_set()
 */

int ffs_lambda(ffs_t * obj, double * lambda);

/**
 *  \brief Return the current time value as a double
 *
 *  \param obj        the ffs_t object
 *  \param t          a pointer to the time value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer was received or the time data
 *                    type has not been et via a call to ffs_type_set()
 */

int ffs_time(ffs_t * obj, double * t);

/**
 *  \}
 */

#endif
