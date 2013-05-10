/*****************************************************************************
 *
 *  ffs.h
 *
 *****************************************************************************/

#ifndef FFS_H
#define FFS_H

#include <mpi.h>

/**
 *  \defgroup ffs_object FFS object
 *  \ingroup ffs_library
 *  \{
 *
 *  This is the entity via which the simulation (or to be exact,
 *  the simulation interface) exchanges information with the FFS library.
 *  This information includes the command line arguments to be used by
 *  the simulation, which are read from the FFS input, and other data
 *  which need to be exchanged.
 *
 *  The simulation interface would obtain the command line arguments
 *  via code such as
 *
 *  \code
 *
 *    #include "ffs.h"
 *
 *    int sim_execute(sim_t * sim, ffs_t * ffs, sim_execute_enum_t action) {
 *
 *      int argc = 0;
 *      char ** argv = NULL;
 *
 *      ffs_command_line_create_copy(ffs, &argc, &argv);
 *
 *      // Pass these arguments to the simulation proper
 *      // in the appropriate way and remember to free
 *      // the copy at the end
 *
 *      ffs_command_line_free_copy(ffs, argc, argv);
 *      ...
 *
 *  \endcode
 *
 *  Other data which need to be exchanged include the time and the
 *  progress coordinate lambda. The simulation interface should inform
 *  the FFS library of the data type associated with these quantties.
 *
 *  For example, in the sim_dmc.c Monte Carlo simulation, time is a
 *  continuous quantity, so is declared to be double via
 *
 *  \code
 *    ffs_type_set(ffs, FFS_INFO_TIME_PUT, 1, FFS_VAR_DOUBLE);
 *  \endcode
 *    
 */

/**
 *  \brief The opaque forward flux sampling type
 */

typedef struct ffs_s ffs_t;

/**
 *  \brief Datatype enumerator
 */

typedef enum {
  FFS_VAR_INT,      /**< For integer data */
  FFS_VAR_DOUBLE    /**< For double data */
} ffs_var_enum_t;

/**
 *  \brief Information enumerator object
 */

typedef enum {
  FFS_INFO_TIME_PUT,         /**< Provide exsiting time */ 
  FFS_INFO_TIME_FETCH,       /**< Fetch new time value */
  FFS_INFO_RNG_SEED_PUT,     /**< Provide RNG seed */
  FFS_INFO_RNG_SEED_FETCH,   /**< Fetch a random number seed */
  FFS_INFO_LAMBDA_PUT,       /**< Provide current lambda value (rank 0) */
  FFS_INFO_LAMBDA_FETCH      /**< Fetch a lambda value */
} ffs_info_enum_t;


/**
 *  \brief Obtain simulation communicator
 *
 *  \param obj      the ffs_t structure
 *  \param comm     a pointer to the MPI communicator to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received
 */

int ffs_comm(ffs_t * obj, MPI_Comm * comm);

/**
 *  \brief Obtain the name of the lambda function requested
 *
 *  \param  obj      the ffs_t object
 *  \param  name     a string to be returned
 *  \param  len      the maximum length of the string
 *
 *   \retval 0       a success
 *   \retval -1      no string could be returned
 *
 *   This provides a method to obtain a string containing the name of
 *   the lambda function as specified in the FFS input. The simulation
 *   interface can then arrange for the appropriate lambda function to
 *   be used (assuming more than one is available).
 */

int ffs_lambda_name(ffs_t * obj, char * name, int len); 

/**
 *  \brief Create a copy of the command line arguments for the simulation
 *
 *  \param  obj    the ffs_t structure
 *  \param  argc   a pointer to the integer argc to be returned
 *  \param  argv   a pointer to the command line arugments to be returned
 *
 *  \retval 0      a success
 *  \retval -1     an error occured
 *
 *  On succesful exit, argv[argc] will hold a copy of the command line
 *  arguments as provided to ffs in the input file.
 *
 *  The caller is responsible for releaseing the resources associated
 *  with argv via a call to ffs_command_line_free_copy().
 */

int ffs_command_line_create_copy(ffs_t * obj, int * argc, char *** argv);

/**
 *  \brief Release resources allocated to hold a copy of argv
 *
 *  \param  obj      the ffs_t structure
 *  \param  argc     the number of arguments
 *  \param  argv     the argv allocated by ffs_command_line_create_copy()
 *
 *  \retval 0        a succcess
 *  \retval -1       a failure
 *
 *  The routine will check that argc agrees with that returned by
 *  ffs_command_line_create_copy().
 */

int ffs_command_line_free_copy(ffs_t * obj, int argc, char ** argv);

/**
 *  \brief Exchange data (double)
 *
 */

int ffs_info_double(ffs_t * obj, ffs_info_enum_t type, int n, double * data);

/**
 *  \brief Exchange data (int)
 *
 *  \param  obj        the ffs struxture
 *  \param  type       action to be performed
 *  \param  n          the number of data items
 *  \param  data       a pointer to the integer data
 *
 *  \retval 0          a success
 *  \retval -1         a NULL pointer was received (or incorrect data)
 */

int ffs_info_int(ffs_t * obj, ffs_info_enum_t type, int n, int * data);

/**
 *  \brief Tell FFS what datatype is associated with a quantity
 */

int ffs_type_set(ffs_t * obj, ffs_info_enum_t type, int n, ffs_var_enum_t t);

/**
 *  \}
 */

#endif
