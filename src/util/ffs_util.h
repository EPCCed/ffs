/*****************************************************************************
 *
 *  ffs_util.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_UTIL_H
#define FFS_UTIL_H

#include <errno.h>
#include <mpi.h>

#include "u/libu.h"


/**
 *  \defgroup utilities Utilities
 *  \{
 *    Utilities
 *  \}
 *
 *  \defgroup ffs_util FFS util
 *  \ingroup utilities
 *  \{
 *  There are a number of macros in the style of libu/include/toolbox/carpal.h
 *  which are used to aid error handling in MPI. As different paths through
 *  the code may lead to deadlock under MPI, some care must be observed
 *  with the regular carpal.h branches to the \b err label.
 *
 *  A function (called collectively by all ranks in MPI_Comm comm) may
 *  typically deal with an error as follows.
 *
 *  \code
 *
 *  int function do_something(MPI_Comm comm) {
 *
 *    int rank;
 *    int mpi_errno = 0;      // global error flag in comm
 *    int mpi_errnol = 0;     // error flag on local rank
 *
 *    // Code which could diverge on different ranks
 *    rank = MPI_Comm_rank(comm, &rank);
 *    mpi_errnol = rank_dependent_result(rank);
 *    mpi_sync_ifm(mpi_errnol, "rank dependent result bad!");
 *
 *   mpi_sync:
 *    MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, comm);
 *    nop_err_if(mpi_errno);
 *
 *    return 0;
 *
 *   err:
 *    return -1;
 *  }
 *
 *  \endcode
 *
 */

/**
 *  \brief Trial result enumerator type
 *
 *  Identifies the status of various trial results.
 */

typedef enum {FFS_TRIAL_NOT_SET = 0,      /**< Not set */
	      FFS_TRIAL_SUCCEEDED,        /**< Clean finish */
	      FFS_TRIAL_TIMED_OUT,        /**< Reached maximum allowed time */
	      FFS_TRIAL_WENT_BACKWARDS,   /**< ..in lambda space */
	      FFS_TRIAL_WAS_PRUNED,       /**< Got the chop */
	      FFS_TRIAL_IN_PROGRESS       /**< Still going */
} ffs_trial_enum_t;

/**
 *  \brief Log a message and jump to label \b mpi_sync: if \b expr is true
 */

#define mpi_sync_if(expr) \
  do { msg_ifb(err_, expr) {goto mpi_sync;} } while (0)

/**
 * \brief Jump to label \c mpi_sync: if \c expr is true and report
 */

#define mpi_sync_sif(expr) \
  do { if (expr) { msg_noargs(err_, errno, #expr); goto mpi_sync; } } while (0)

/**
 *  \brief Jump to label mpi_sync: if \c expr is true and report printf-like
 *  message
 */

#define mpi_sync_ifm(expr, ...) \
  do { if ((expr)) { msg(err_, 0, __VA_ARGS__); goto mpi_sync; } } while (0)


/**
 *  \brief Return -1 is any expr in communicator comm is not 0
 *
 *  \param expr        an integer expression
 *  \param comm        the MPI communicator
 *
 *  \retval 0          all expr were 0
 *  \retval -1         one or more expr were non-zero
 *
 *  This calls MPI_Allreduce() in comm, so is collective.
 */

int util_mpi_any(int expr, MPI_Comm comm);

/**
 *  \brief Jump to label mpi_sync: if any expr in comm is non-zero
 *
 *  This ultimately uses MPI_Allreduce() in comm to agree on synchronisation,
 *  so should be used with discretion.
 */

#define mpi_sync_if_any(expr, comm) \
  do { msg_ifb(err_, util_mpi_any((expr), comm)) {goto mpi_sync;} } while (0)

/**
 *  \brief Jump to label err: if any expr in comm is non-zero
 *
 *  Ultimately calls MPI_Allreduce() in comm to agree on synchronisation,
 *  so use with discretion.
 */

#define mpi_err_if_any(expr, comm) \
  do { msg_ifb(err_, util_mpi_any((expr), comm)) {goto err;} } while (0)

/**
 *  \brief Connect u_log message machinery to stdout
 *
 *  This function has signature u_log_hook_t
 *
 */

int util_ulog(void * arg, int level, const char * str);

/**
 *  \brief Return a double value associated with a configuration subkey
 *
 *  \param  c         the configuration object
 *  \param  subkey    the subkey of the child configuration obeject
 *  \param  def       value to return if subkey is not present
 *  \param  out       pointer to the double value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int util_config_get_subkey_value_d(u_config_t * c, const char * subkey,
				   double def, double * out);

/**
 *  \brief Compare two double values to within a tolerance
 *
 *  \param  a    double
 *  \param  b    double
 *  \param  tol  tolerance
 *
 *  \retval 0    fabs(a - b) < tol
 *  \retval -1   otherwise
 *
 */

int util_compare_double(double a, double b, double tol);

/**
 *  \brief Return a file name stub, e.g.,  "inst0000-grp0000-state00000000"
 *
 *  \param  id_inst      intended for the instance number
 *  \param  id_group     intended for group or ensemble number
 *  \param  id_state     state id
 *
 *   The format used to construct the stub is "inst%4.4d-ngrp%4.4d-state%9.9d"
 *
 *  \retval stub  the file stub on success
 *  \retval NULL if there was a problem formating the string
 *
 */

const char * util_filename_stub(int id_inst, int id_group, int id_state);

/**
 * \}
 */

#endif /* FFS_UTIL_H */

