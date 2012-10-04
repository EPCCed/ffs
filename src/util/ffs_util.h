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

#include "u/libu.h"

/**
 *  \defgroup mpi_util MPI utilities
 *  \ingroup utilities
 *  \{
 *     Some utilities related to message passing.
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
 *  \endcode
 *
 */

/**
 *  \def Jump to label \b mpi_sync if \b expr is true
 */

#define mpi_sync_if(expr) \
  do { msg_ifb(err_, expr) {goto mpi_sync;} } while (0)

/**
 * \def Jump to label \c mpi_sync if \c expr is true and report ...
 */

#define mpi_sync_sif(expr) \
  do { if (expr) { msg_noargs(err_, errno, #expr); goto mpi_sync; } } while (0)

/**
 *  \def Jump to label \c mpi_sync if \c expr is true and report printf-like
 *  message
 */

#define mpi_sync_ifm(expr, ...) \
  do { if ((expr)) { msg(err_, 0, __VA_ARGS__); goto mpi_sync; } } while (0)

/**
 *  \}
 */

#endif /* FFS_UTIL_H */

