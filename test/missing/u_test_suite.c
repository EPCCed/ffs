/*****************************************************************************
 *
 *  \file u_test_suite.c
 *
 *  Unit test suite for this directory.
 *
 *****************************************************************************/

#include <mpi.h>
#include "u/libu.h"

int u_test_mpi_init(u_test_case_t * tc);
int u_test_mpi_comm(u_test_case_t * tc);

/*
 * Suite
 */

int u_test_suite_mpi_register (u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("mpi", &ts);

  u_test_case_register("mpi init", u_test_mpi_init, ts);
  u_test_case_register("mpi communicator", u_test_mpi_comm, ts);
  u_test_case_depends_on("mpi communicator", "mpi init", ts);

  return u_test_suite_add(ts, t);
}
