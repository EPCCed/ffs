/*****************************************************************************
 *
 *  ffs_result.h
 *
 *****************************************************************************/

#ifndef FFS_RESULT_H
#define FFS_RESULT_H

/**
 *  \defgroup ffs_result FFS result structure
 *  \ingroup ffs_library
 *  \{
 *
 *  Comments on result structure here.
 *
 */

/**
 *  \brief Opaque result data type
 */

typedef struct ffs_result_s ffs_result_t;

/**
 *  \brief Create a result structure
 *
 *  \param  nlambda       number of interface present
 *  \param  ntrial        number of initial trials
 *  \param  pobj          a pointer to the new structure to be returned
 *
 *  \retval 0             a success
 *  \retval -1            a failure
 */

int ffs_result_create(int nlambda, int ntrial, ffs_result_t ** pobj);

/**
 *  \brief Free a result structure
 *
 *  \param  obj   the ffs_result_t structure to be removed
 */

void ffs_result_free(ffs_result_t * obj);

/**
 *  \brief Return the number of crossings of the first interface
 *
 *  \param  obj      the ffs_result_t structure
 *  \param  ncross   a pointer to the integer number to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer was received
 */

int ffs_result_ncross(ffs_result_t * obj, int * ncross);

/**
 *  \brief Accumalate number of crossings
 *
 *  \param  obj    the ffs_result_t structure
 *  \param  iadd   number to be accumulated
 *
 *  \retval 0      a success
 *  \retval -1     obj was NULL
 */

int ffs_result_ncross_accum(ffs_result_t * obj, int iadd);

/**
 *  \brief Record the status for trial n
 *
 *  \param  obj      the ffs_result_t structure
 *  \param  n        the trial index 0 < n < ntrial - 1
 *  \param  status   the trial result
 *
 *  \retval 0        a success
 *  \retval -1       a failure (a NULL pointer was received or n was invalid)
 */

int ffs_result_status_set(ffs_result_t * obj, int n, int status);

/**
 *  \brief Return the status for trial n
 *
 *  \param obj      the ffs_result_t structure
 *  \param n        the trial index 0 < n < ntrial
 *  \param status   a point to the integer status value to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a failure (a NULL pointer was received or n was invalid)
 */

int ffs_result_status(ffs_result_t * obj, int n, int * status);

/**
 * /}
 */

#endif
