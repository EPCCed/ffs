/*****************************************************************************
 *
 *  \file ffs_error.h
 *
 *  Set up for the error logging, which will be via libu.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef _FFS_ERROR_H_
#define _FFS_ERROR_H_

/**
 *  \brief
 *
 *  Initialises the FFS u_log_hook at the start of execution
 *
 *  \param  rank    MPI rank, or zero
 *
 *  \retval 0       success
 *
 */

int ffs_error_init(int rank);

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

int ffs_error_u_log_hook(void * arg, int level, const char * str);

#endif
