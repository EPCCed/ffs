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

struct util_s {
  u_string_t * stub;
};

static int util_inst(void);
static struct util_s * inst = NULL;

/*****************************************************************************
 *
 *  util_ulog
 *
 *****************************************************************************/

int util_ulog(void * arg, int level, const char * str) {

  fprintf(stdout, "%s\n", str);

  return 0;
}

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

/*****************************************************************************
 *
 *  util_filename_stub
 *
 ****************************************************************************/

const char * util_filename_stub(int id_inst, int id_group, int id_state) {

  const int ifmt4 = 9999;         /* Maximum instance, group number */
  const int ifmt9 = 999999999;    /* Maximum state id */
  const char * fmt = "inst%4.4d-grp%4.4d-state%9.9d";

  dbg_return_if(id_inst < 0, NULL);
  dbg_return_if(id_group < 0, NULL);
  dbg_return_if(id_state < 0, NULL);
  dbg_return_if(util_inst(), NULL);

  dbg_err_ifm(id_inst  > ifmt4, "Format botch inst. = %d", id_inst);
  dbg_err_ifm(id_group > ifmt4, "Format botch group = %d", id_group);
  dbg_err_ifm(id_state > ifmt9, "Format botch state = %d", id_state);

  u_string_sprintf(inst->stub, fmt, id_inst, id_group, id_state);

  return u_string_c(inst->stub);

 err:

  return NULL;
}

/*****************************************************************************
 *
 *  util_inst
 *
 *  Singleton object
 *
 *****************************************************************************/

static int util_inst(void) {

  if (inst == NULL) {
    inst = u_calloc(1, sizeof(struct util_s));
    dbg_err_if(inst == NULL);
  }

  if (inst->stub == NULL) {
    dbg_err_if(u_string_create("", strlen(""), &inst->stub));
  }

  return 0;

 err:

  return -1;
}
