/*****************************************************************************
 *
 *  ffs.h
 *
 *****************************************************************************/

#ifndef FFS_H
#define FFS_H

#include <mpi.h>

/**
 *  \defgroup ffs_mediator FFS mediator object
 *  \ingroup ffs_library
 *  \{
 *
 */

/**
 *  \brief The opaque mediator object
 */

typedef struct ffs_s ffs_t;

/**
 *  \brief Information enumerator object
 */

typedef enum {
  FFS_INFO_TIME_PUT,     /**< Provide exsiting time */ 
  FFS_INFO_TIME_FETCH,   /**< Fetch a new time */
  FFS_INO_LAMBDA_PUT     /**< Provide current lambda value (rank 0) */
} ffs_info_enum_t;


/**
 *  \brief Obtain simulation communicator
 */

int ffs_comm(ffs_t * obj, MPI_Comm * comm);

/**
 *  \brief Obtain command line arguments
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
 */

int ffs_info_int(ffs_t * obj, ffs_info_enum_t type, int n, int * data);

/**
 *  \}
 */

#endif
