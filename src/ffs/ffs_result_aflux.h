/*****************************************************************************
 *
 *  ffs_result_aflux.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012-2013 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1.
 *
 *****************************************************************************/

#ifndef FFS_RESULT_AFLUX_H
#define FFS_RESULT_AFLUX_H

#include <mpi.h>
#include "ffs_util.h"

/**
 *  \defgroup ffs_result_aflux Result for flux from State A
 *  \ingroup ffs_library
 *  \{
 *
 *  This structure provides details of the results on trials
 *  from state A for a given FFS instance. The number of crossings
 *  of the first interface and the time taken give rise to a flux.
 *
 *  It is required the all MPI ranks in a given simulation proxy
 *  report the same results. A reduction in the proxy cross
 *  communicator will then provide a consistent view of the
 *  results on all ranks in the instance at the end.
 */

/**
 *  \brief Opaque result data type
 */

typedef struct ffs_result_aflux_s ffs_result_aflux_t;

/**
 *  \brief Create a result structure
 *
 *  \param  ntrial_local  number of initial trials on this proxy
 *  \param  pobj          a pointer to the new structure to be returned
 *
 *  \retval 0             a success
 *  \retval -1            a failure
 */

int ffs_result_aflux_create(int ntrial_local, ffs_result_aflux_t ** pobj);

/**
 *  \brief Free a result structure
 *
 *  \param  obj   the ffs_result_aflux_t structure to be removed
 */

void ffs_result_aflux_free(ffs_result_aflux_t * obj);

/**
 *  \brief Increment local number of crossings by one
 *
 *  \param  obj    the ffs_result_aflux_t structure
 *
 *  \retval 0      a success
 *  \retval -1     obj was NULL
 */

int ffs_result_aflux_ncross_add(ffs_result_aflux_t * obj);

/**
 *  \brief Return the current local number of crossings of the first interface
 *
 *  \param  obj      the ffs_result_aflux_t structure
 *  \param  ncross   a pointer to the integer number to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer was received
 */

int ffs_result_aflux_ncross_local(ffs_result_aflux_t * obj, int * ncross);

/**
 *  \brief Return the final number of crossings of first interface
 *         for this instance
 *
 *  \param obj       the ffs_result_aflux_t structure
 *  \param ncross    a pointer to the number to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int ffs_result_aflux_ncross_final(ffs_result_aflux_t * obj, int * ncross);

/**
 *  \brief Record the status for trial n
 *
 *  \param  obj      the ffs_result_aflux_t structure
 *  \param  n        the trial index 0 < n < ntrial - 1
 *  \param  status   the trial result
 *
 *  \retval 0        a success
 *  \retval -1       a failure (a NULL pointer was received or n was invalid)
 */

int ffs_result_aflux_status_set(ffs_result_aflux_t * obj, int n, int status);

/**
 *  \brief Return the total number of trial status results of type key
 *
 *  \param obj       the ffs_result_aflux_t
 *  \param key       identifies the result wanted
 *  \param sum       a pointer to the result to be returned
 *
 *  This routine must be preceeded by a call to ffs_result_aflux_reduce() to
 *  provide a meaningful answer. The value of sum returned is -1 if no
 *  value is available.
 *
 *  \retval 0        a success.
 */

int ffs_result_aflux_status_final(ffs_result_aflux_t * obj,
				  ffs_trial_enum_t key, int * sum);

/**
 *  \brief Set time for initial trajectory n
 *
 *  \param  obj       the ffs_result_aflux_t structure
 *  \param  n         the trial id 0 <= n < ntrial
 *  \param  t         the final time for the trajectory (simulation units)
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid n was received
 */

int ffs_result_aflux_time_set(ffs_result_aflux_t * obj, int n, double t);

/**
 *  \brief Return initial trajectory tmax
 *
 *  Must be preceeded by a call to ffs_result_reduce().
 *
 *  \param  obj     the ffs_result_aflux_t structure
 *  \param  tmax    pointer to the (double) value to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received
 */

int ffs_result_aflux_tmax_final(ffs_result_aflux_t * obj, double * tmax);

/**
 *  \brief Return sum of initial trajectory times
 *
 *  Must be preceeded by a call to ffs_result_aflux_reduce().
 *
 *  \param  obj       the ffs_result_aflux_t structure
 *  \param  tsum      a pointer to the (double) value to be returned
 *                    (simulation units)
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer was received
 */

int ffs_result_aflux_tsum_final(ffs_result_aflux_t * obj, double * tsum);

/**
 *  \brief Accumulate number of equilibrartion runs
 *
 *  \param obj    the ffs_result_aflux_t
 *
 *  \retval 0     a success
 *  \retval -1    obj was NULL
 */

int ffs_result_aflux_neq_add(ffs_result_aflux_t * obj);

/**
 *  \brief Return the final number of equilibrartion runs
 *
 *  \param obj     the ffs_result_aflux_t
 *  \param neq     a pointer to the number to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a NULL pointer was received
 */

int ffs_result_aflux_neq_final(ffs_result_aflux_t * obj, int * neq);

/**
 *  \brief Return the final number of trials for this instance
 *
 *  \param obj     the ffs_result_aflux_t
 *  \param ntrial  a pointer to the number to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a NULL pointer was received
 */

int ffs_result_aflux_ntrial_final(ffs_result_aflux_t * obj, int * ntrial);

/**
 *  \brief Call MPI_Allreduce() for all results in communicator comm
 *
 *  \param  obj      the ffs_result_aflux_t object
 *  \param  comm     MPI communicator for reduction
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int ffs_result_aflux_reduce(ffs_result_aflux_t * obj, MPI_Comm comm);

/**
 * \}
 */

#endif
