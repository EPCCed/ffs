/*****************************************************************************
 *
 *  mpilog.h
 *
 *  A simple MPI-aware logging facility.
 *
 *  Parallel Forward Flux Smapling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef MPILOG_H
#define MPILOG_H

#include <stdarg.h>
#include <stdio.h>
#include <mpi.h>

/**
 * \defgroup mpilog_t MPI log facility
 * \ingroup utilities
 * \{
 *
 *  A Simple MPI-aware logging facility
 *
 *  This is really just to avoid the tedium of episodes such as
 *
 *  \code
 *     MPI_Comm_rank(comm, &rank);
 *     if (rank == 0) fprintf(fp, "%s\n", "Here is a message on rank zero");
 *  \endcode
 *
 *  which can be replaced with
 *
 *  \code
 *     mpilog(log, "%s\n", "Here is a message on rank 0");
 *  \endcode
 *
 *  or variants thereon. The default stream for output is stdout,
 *  but this may be changed via mpilog_fp_set().
 */

/**
 * \brief Opaque MPI log object
 */

typedef struct mpilog_s mpilog_t;

/**
 *  \brief Create and return a new MPI log object
 *
 *  \param parent       The MPI communicator for this log object
 *  \param pobj         a pointer to the new object to be returned
 *
 *  \retval 0           a success
 *  \retval -1          a failure
 */ 

int mpilog_create(MPI_Comm parent, mpilog_t ** pobj);

/**
 *  \brief Release an object
 *
 *  \param obj          the object to be released
 *
 *  \returns   nothing
 */

void mpilog_free(mpilog_t * obj);

/**
 *  \brief Set the FILE stream for output
 *
 *  \param obj        the log object
 *  \param fp         the file stream to be used
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int mpilog_fp_set(mpilog_t * obj, FILE * fp);

/**
 *  \brief Log a message
 *
 *  \param obj       the log object
 *  \param fmt       printf-like format string
 *  \param ...
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 *
 *  Logs a message on rank 0 of the log communicator on the current stream.
 */

int mpilog(mpilog_t * obj, const char * fmt, ...);

/**
 *  \brief Log a message on a given stream
 *
 *  \param obj     the log object
 *  \param fp      a pointer to the stream to use for output
 *  \param fmt     a printf-like format string
 *  \param ...     the message
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 *
 *  Logs a message to stream fp on rank 0 in the log communicator.
 */

int mpilog_fp(mpilog_t * obj, FILE * fp, const char * fmt, ...);

/**
 *  \brief Log a message on the given MPI rank only
 *
 *  \param obj      the log object
 *  \param rank     the MPI rank in the log communicator which will log
 *  \param fmt      a printf-like format string
 *  \param ...      the message
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int mpilog_rank(mpilog_t * obj, int rank, const char * fmt, ...);

/**
 *  \brief Log a message to stream fp on the given MPI rank 
 *
 *  \param obj       the log object
 *  \param rank      the MPI rank in the log communicator to log
 *  \param fp        the stream on which to log
 *  \param fmt       a printf-like format string
 *  \param ...       the message
 */

int mpilog_rank_fp(mpilog_t * obj, int rank, FILE * fp, const char * fmt, ...);

/**
 *  \brief Open a new file and associate the log with it.
 *
 *  \param obj        the log object 
 *  \param filename   the name of the file
 *  \param mode       the mode to be passed to fopen()
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int mpilog_fopen(mpilog_t * obj, const char * filename, const char * mode);

/**
 *  \brief Close a file previously openned via mpilog_fopen()
 *
 *  \param obj      the log object
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int mpilog_fclose(mpilog_t * obj);

/**
 *  \brief Log a message conditionally
 *
 *  \param expr        log if true
 *  \param ...         remaining arguments as for mpilog()
 *
 */

#define mpilog_if(expr, ...) \
  do { if ((expr)) mpilog( __VA_ARGS__); } while (0)

/**
 *  \brief Utility to pass messages to the libu logging machinery
 *
 *  \param arg       Opaque argument which is the mpilog_t log
 *  \param level     Syslog message level
 *  \param str       The message string
 *
 *  This is only required as an argument to u_log_set_hook().
 */

int mpilog_ulog(void * arg, int level, const char * str);

/**
 *  \}
 */

#endif
