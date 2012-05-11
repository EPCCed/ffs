/*****************************************************************************
 *
 *  u_extra.c
 *
 *  Additional functions directly related to libu; there is a tacit
 *  dependence on MPI which might be made explicit.
 *
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <math.h>
#include <stdio.h>

#include "u/libu.h"
#include "u_extra.h"


/* This is the global facility variable required by libu for logging */

int facility = LOG_LOCAL0;

/* MPI rank, to be prepended to message */

static int rank_;

/*****************************************************************************
 *
 *  u_extra_error_init
 *
 *  We are going to use the libu machinery to handle errors. However,
 *  to prevent us flooding the system logs with messages, which may
 *  make us unpopular in some quarters, everything is going to be
 *  directed to stderr.
 *
 *  This should be called before messages start flying into
 *  the system logs.
 *
 *  \param rank   an MPI rank, or zero
 *
 *  \retval 0     a success
 *
 *****************************************************************************/

int u_extra_error_init(int rank) {

  rank_ = rank;
  u_log_set_hook(u_extra_error_u_log_hook, (void *) &rank_, NULL, NULL);

  return 0;
}

/*****************************************************************************
 *
 *  u_extra_error_u_log_hook
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

int u_extra_error_u_log_hook(void * arg, int level, const char * str) {

  int * rank;

  rank = arg;
  fprintf(stderr, "[%d]%s\n", *rank, str);

  return 0;
}

/*****************************************************************************
 *
 *  \brief Attempt to return a double associated with a config child subkey
 *
 *  u_atof is used to convert from the string
 *
 *  \param  c       the configuration object
 *  \param  subkey  the key string for the child object
 *  \param  def     value which will be returned is key is not found
 *  \param  out     pointer to double value to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *****************************************************************************/

int u_extra_config_get_subkey_value_d(u_config_t * c, const char * subkey,
				      double def, double * out) {
  const char * value;

  value = u_config_get_subkey_value(c, subkey);

  if (value == NULL) {
    *out = def;
  }
  else {
    err_err_if(u_atof(value, out));
  }

  return 0;

 err:
  return -1;
}


/******************************************************************************
 *
 *  \brief  Compares two doubles to with tolerance
 *
 *  \param  a   double
 *  \param  b   double
 *  \param  tol the tolerance
 *
 *  \retval 0   a == b to within tolerance
 *  \retval -1  a != b to within tolerance
 *
 *****************************************************************************/

int u_extra_compare_double(double a, double b, double tol) {

  if (fabs(a - b) < tol) return 0;

  return -1;
}
