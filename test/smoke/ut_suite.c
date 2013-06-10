/*****************************************************************************
 *
 *  ut_suite.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include "u/libu.h"

#include "st_gil.h"

/*****************************************************************************
 *
 *  u_test_suite_smoke_register
 *
 *  Register smoke tests.
 *
 *****************************************************************************/

int u_test_suite_smoke_register(u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("smoke", &ts);

  /* Two full runs back-to-back appear to cause second to generate
   * incorrect results TODO: suggests something not 'clean' */
  /*u_test_case_register("DMC smoke test II", st_gil_two, ts); */

  u_test_case_register("DMC smoke test I", st_gil_create, ts);

  return u_test_suite_add(ts, t);
}
