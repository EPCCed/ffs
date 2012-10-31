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

#include "ffs_util.h"

/* This is the global facility variable required by libu for logging */

int facility = LOG_LOCAL0;

/*****************************************************************************
 *
 *  util_config_get_subkey_value_d
 *
 *****************************************************************************/

int util_config_get_subkey_value_d(u_config_t * c, const char * subkey,
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


/*****************************************************************************
 *
 *  util_compare_double
 *
 *****************************************************************************/

int util_compare_double(double a, double b, double tol) {

  if (fabs(a - b) < tol) return 0;

  return -1;
}

/*****************************************************************************
 *
 *  util_mpi_any
 *
 *****************************************************************************/

int util_mpi_any(int expr, MPI_Comm comm) {

  int mpi_errno = 0;

  MPI_Allreduce(&expr, &mpi_errno, 1, MPI_INT, MPI_LOR, comm);

  return mpi_errno;
}
