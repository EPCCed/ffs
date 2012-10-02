/*****************************************************************************
 *
 *  ut_factory.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "factory.h"
#include "ut_factory.h"

/*****************************************************************************
 *
 *  ut_factory
 *
 *****************************************************************************/

int ut_factory(u_test_case_t * tc) {

  abstract_sim_t * sim = NULL;
  interface_t table;
  int present = 0;

  u_dbg("Start");

  u_test_err_if(factory_inquire("Non existant!", &present));
  u_test_err_if(present != 0);

  u_test_err_if(factory_inquire("test", &present));
  u_test_err_ifm(present == 0, "no test");

  u_test_err_if(factory_inquire("dmc", &present));
  u_test_err_ifm(present == 0, "no dmc");

  u_test_err_if(factory_make(MPI_COMM_WORLD, "Non-existant", &table, &sim));
  u_test_err_if(sim != NULL);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
