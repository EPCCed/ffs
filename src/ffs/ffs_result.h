/*****************************************************************************
 *
 *  ffs_result.h
 *
 *****************************************************************************/

#ifndef FFS_RESULT_H
#define FFS_RESULT_H

#include <mpi.h>
#include "ffs_util.h"

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
 *  \brief Return current weight for given interface
 *
 *  \param  obj      the ffs_result_t structure
 *  \param  n        the interface index 1 < n <= nlambda
 *  \param  wt       pointer to the (double) weight to return
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer or invalid n was received
 */

int ffs_result_weight(ffs_result_t * obj, int n, double * wt);

/**
 *  \brief Accumulate a contribution to the weight at interface n
 *
 *  \param   obj      the ffs_result_t structure
 *  \param   n        the interface index 1 < n < nlambda
 *  \param   wt       the weight to accumulate
 *
 *  \retval  0        a success
 *  \retval  -1       a NULL pointer or invalid n was received
 */

int ffs_result_weight_accum(ffs_result_t * obj, int n, double wt);

/**
 *  \brief Set time for initial trajectory n
 *
 *  \param  obj       the ffs_result_t structure
 *  \param  n         the trial id 0 <= n < ntrial
 *  \param  t         the final time for the trajectory (simulation units)
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid n was received
 */

int ffs_result_time_set(ffs_result_t * obj, int n, double t);

/**
 *  \brief Return the initial time for trajectory n 
 *
 *  \param obj        the ffs_result_t structure
 *  \param n          the trial index 0 <= n < ntrial
 *  \param t          a pointer to teh (double) time to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid n was received
 */

int ffs_result_time(ffs_result_t * obj, int n, double * t);


/**
 *  \brief Organise result in communicator comm
 *
 *  \param  obj      the ffs_result_t object
 *  \param  comm     MPI communicator for reduction
 *  \param  root     root MPI rank for MPI_Reduce()
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int ffs_result_reduce(ffs_result_t * obj, MPI_Comm comm, int root);


/**
 *  \brief Return initial trajectory tmax
 *
 *  Must be preceeded by a call to ffs_result_reduce().
 *
 *  \param  obj     the ffs_result_t structure
 *  \param  tmax    pointer to the (double) value to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received
 */

int ffs_result_tmax(ffs_result_t * obj, double * tmax);

/**
 *  \brief Return sum of initial trajectory times
 *
 *  Must be preceeded by a call to ffs_result_reduce().
 *
 *  \param  obj       the ffs_result_t structure
 *  \param  tsum      a pointer to the (double) value to be returned
 *                    (simulation units)
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer was received
 */

int ffs_result_tsum(ffs_result_t * obj, double * tsum);

/**
 *  \brief Regsister a successful trial which has reached interface
 *
 *  \param  obj         the ffs_result_t structure
 *  \param  interface   the index of the interface reached
 *
 *  \retval 0           a succcess
 *  \retval -1          a NULL pointer was received or interface was
 *                      not between 1 and nlambda
 */

int ffs_result_trial_success_add(ffs_result_t * obj, int interface);

/**
 *  \brief Return the number of successes recorded at an interface
 *
 *  \param   obj        the ffs_result_t structure
 *  \param   interface  the index of the interface
 *  \param   ns         pointer ot the number of successes to be returned
 *
 *  \retval 0           a success
 *  \retval -1          a NULL pointer was received or interface was
 *                      not between 1 and nlambda
 */

int ffs_result_trial_success(ffs_result_t * obj, int interface, int * ns);

/**
 *  \brief Register a 'pruned' trial at interface
 *
 *  \param    obj        the ffs_result_t structure
 *  \param    interface  the index of the interface
 *
 *  \retval   0          a success
 *  \retval  -1          a NULL pointer was received or interface was
 *                       not between 1 and nlambda
 */

int ffs_result_prune_add(ffs_result_t * obj, int interface);


/**
 *  \brief Return the number of pruned trials at interface
 *
 *  \param    obj        the ffs_result_t structure
 *  \param    interface  the index of the interface
 *  \param    np         pointer to the number to be returned
 *
 *  \retval   0          a success
 *  \retval  -1          a NULL pointer was received or interface was
 *                       not between 1 and nlambda
 */

int ffs_result_prune(ffs_result_t * obj, int interface, int * np);

/**
 *  \brief Return the total number of trial status results of type key
 *
 *  \param obj       the ffs_result_t
 *  \param key       identifies the result wanted
 *  \param sum       a pointer to the result to be returned
 *
 *  This routine must be preceeded by a call to ffs_result_reduce() to
 *  provide a meaningful answer. The value of sum returned is -1 if no
 *  value is available.
 *
 *  \retval 0        a success.
 */

int ffs_result_status_final(ffs_result_t * obj, ffs_trial_enum_t key,
			    int * sum);

/**
 *  \brief Accumulate number of equilibrartion runs
 *
 *  \param obj    the ffs_result_t
 *  \param add    the number to be accumulated
 *
 *
 *  \retval 0     a success
 *  \retval -1    obj was NULL
 */

int ffs_result_eq_accum(ffs_result_t * obj, int add);

/**
 *  \brief Return the final number of equilibrartion runs
 *
 *  \param obj     the ffs_result_t
 *  \param nsum    a pointer to the number to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a NULL pointer was received
 */

int ffs_result_eq_final(ffs_result_t * obj, int * nsum);

/**
 *  \brief Record the number of states retained at an interface
 *
 *  \param  obj      the ffs_result_t data structure
 *  \param  n        in interface index
 *  \param  nkeep    the number of states to record
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer or invalid n was received
 */

int ffs_result_nkeep_set(ffs_result_t * obj, int n, int nkeep);

/**
 *  \brief Return the number of states retained at an interface
 *
 *  \param  obj      the ffs_result_t data structure
 *  \param  n        the interface index
 *  \param  nkeep    a pointer to the integer to be returned
 *
 *  \retval 0        a sucess
 *  \retval -1       a NULL pointer or invalid n was received
 */

int ffs_result_nkeep(ffs_result_t * obj, int n, int * nkeep);

/**
 *  \brief Add a time out at an interface
 *
 *  \param  obj      the ffs_result_t data structure
 *  \param  n        in interface index
 *  \param  nto      the number to accumulate
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer or invalid n was received
 */

int ffs_result_nto_add(ffs_result_t * obj, int n, int nto);

/**
 *  \brief Return the number of time outs at an interface
 *
 *  \param  obj      the ffs_result_t data structure
 *  \param  n        the interface index
 *  \param  nto      a pointer to the integer to be returned
 *
 *  \retval 0        a sucess
 *  \retval -1       a NULL pointer or invalid n was received
 *
 *  A call to this routine should be preceded by a call to
 *  ffs_result_reduce() if the global total is required.
 */

int ffs_result_nto(ffs_result_t * obj, int n, int * nto);

/**
 *  \brief Accumulate success weight at interface
 *
 *  \param  obj       the ffs_result_t sturcture
 *  \param  n         the interface index
 *  \param  swt       the wieght to be accumulated
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid index was received
 */

int ffs_result_success_weight_accum(ffs_result_t * obj, int n, double swt);

/**
 *  \brief Get the success weight for an interface
 *
 *  \param  obj        the ffs_result_t structure
 *  \param  n          the interface index
 *  \param  swt        a pointer to the value to be returned
 *
 *  \retval 0          a success
 *  \retval -1         a NULL pointer or invalid index was received
 */

int ffs_result_success_weight(ffs_result_t * obj, int n, double * swt);

/**
 *  \brief Get trial count at interface
 *
 *  \param  obj       the ffs_result_t sturcture
 *  \param  n         the interface index
 *  \param  nstart    the number of trials started at this interface, to be
 *                    returned
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid index was received
 */

int ffs_result_nstart(ffs_result_t * obj, int n, int * nstart);

/**
 *  \brief Add one to trial count at interface
 *
 *  \param  obj        the ffs_result_t structure
 *  \param  n          the interface index
 *
 *  \retval 0          a success
 *  \retval -1         a NULL pointer or invalid index was received
 */

int ffs_result_nstart_add(ffs_result_t * obj, int n);


/**
 *  \brief Get count of dropped trajectories (to specfied interface)
 *
 *  \param  obj       the ffs_result_t sturcture
 *  \param  n         the interface index
 *  \param  ndrop     a pointer to the count to be returned    
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid index was received
 */

int ffs_result_ndrop(ffs_result_t * obj, int n, int * ndrop);

/**
 *  \brief Add one to count of trials dropped (trials originating at interface)
 *
 *  \param  obj        the ffs_result_t structure
 *  \param  n          the interface index
 *
 *  \retval 0          a success
 *  \retval -1         a NULL pointer or invalid index was received
 */

int ffs_result_ndrop_add(ffs_result_t * obj, int n);

/**
 *  \brief Get count of backsliding  trajectories (from specfied interface)
 *
 *  \param  obj       the ffs_result_t sturcture
 *  \param  n         the interface index
 *  \param  nback     a pointer to the count to be returned    
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer or invalid index was received
 */

int ffs_result_nback(ffs_result_t * obj, int n, int * nback);

/**
 *  \brief Add one to count of trials going backwards (trials originating
 *         at interface)
 *
 *  \param  obj        the ffs_result_t structure
 *  \param  n          the interface index
 *
 *  \retval 0          a success
 *  \retval -1         a NULL pointer or invalid index was received
 */

int ffs_result_nback_add(ffs_result_t * obj, int n);

/**
 * \}
 */

#endif
