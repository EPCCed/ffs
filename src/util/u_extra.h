/*************************************************************************//**
 *
 *  \file u_extra.h
 *
 *  Some additional utilities associated with libu.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef U_EXTRA_H
#define U_EXTRA_H

#include <errno.h>
#include "u/libu.h"

/**
 *  \defgroup utilities Utilities
 *  \{
 *    Various utility functions and objects
 *  \}
 *
 *  \defgroup u_extra Related to libu
 *  \ingroup utilities
 *  \{
 *    Some utility functions which are related to libu, error handling,
 *    and MPI.
 *
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
 *  Note at the moment there is only implicit dependence on mpi.h
 *
 *  \endcode
 *
 */

/**
 *  \define Jump to label \b mpi_sync if \b expr is true
 */

#define mpi_sync_if(expr) \
  do { msg_ifb(err_, expr) {goto mpi_sync;} } while (0)

/**
 * \define Jump to label \c mpi_sync if \c expr is true and report ...
 */

#define mpi_sync_sif(expr) \
  do { if (expr) { msg_noargs(err_, errno, #expr); goto mpi_sync; } } while (0)

/**
 *  \define Jump to label \c mpi_sync if \c expr is true and report printf-like
 *  message
 */

#define mpi_sync_ifm(expr, ...) \
  do { if ((expr)) { msg(err_, 0, __VA_ARGS__); goto mpi_sync; } } while (0)

/**
 *  \brief
 *
 *  Initialises the u_log_hook at the start of execution
 *
 *  \param  rank    MPI rank, or zero
 *
 *  \retval 0       success
 *
 */

int u_extra_error_init(int rank);

/**
 *  \brief
 *
 *  This is the hook function with signature u_log_hook_t
 *  used to direct all messages to stderr. It should not
 *  be required in any other context by the application.
 *
 *  \param  arg    opaque argument
 *  \param  level  syslog severity level
 *  \param  str    message string
 *
 *  \retval 0      a success
 *
 */

int u_extra_error_u_log_hook(void * arg, int level, const char * str);


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

int u_extra_config_get_subkey_value_d(u_config_t * c, const char * subkey,
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

int u_extra_compare_double(double a, double b, double tol);

/**
 * \}
 */

#endif
