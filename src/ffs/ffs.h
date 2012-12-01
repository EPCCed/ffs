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
 *  This is the interface through which the simulation (or to be exact,
 *  the simulation interface) exchanges information with the FFS library.
 *  
 */

/**
 *  \brief An opaque structure
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
 *  \brief Obtain command line arguments
 *
 *  This returns C-like argc and argv taken from the user input.
 *  argv[0] contains a (dummy) executable name.
 *
 *  \param  obj    the ffs_t structure
 *  \param  argc   pointer to the nu,ber of arguments to be returned
 *  \param  argv   pointer to list of arguments to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a NULL pointer was received
 */

int  ffs_command_line(ffs_t * obj, int * argc, char *** argv);

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
