/*****************************************************************************
 *
 *  \file ffs_error.c
 *
 *  We are going to use the libu machinery to handle errors. However,
 *  to prevent us flooding the system logs with messages, which may
 *  make us unpopular in some quarters, everything is going to be
 *  directed to stderr.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "ffs_error.h"

/* This is the global facility variable required by libu */

int facility = LOG_LOCAL0;

/* MPI rank, to be prepended to message */

static int rank_;

/*****************************************************************************
 *
 *  \brief ffs_error_init initialises the libu log mechanism.
 *
 *  This should be called before messages start flying into
 *  the system logs.
 *
 *  \param rank   MPI rank, or zero
 *
 *  \retval 0     a success
 *
 *****************************************************************************/

int ffs_error_init(int rank) {

  rank_ = rank;
  u_log_set_hook(ffs_error_u_log_hook, (void *) &rank_, NULL, NULL);

  return 0;
}

/*****************************************************************************
 *
 *  \brief ffs_error_u_log_hook is the replacement log writer
 *
 *  Note that we use the opaque argument arg to store the
 *  MPI rank, which is prepended to the rest of the message.
 *
 *  \param arg     opaque argument, here holding interger rank
 *  \param level   the syslog severity level (not used)
 *  \param str     the message
 *
 *  \retval        0
 *
 *****************************************************************************/

int ffs_error_u_log_hook(void * arg, int level, const char * str) {

  int * rank;

  rank = arg;
  fprintf(stderr, "[%d]%s\n", *rank, str);

  return 0;
}
