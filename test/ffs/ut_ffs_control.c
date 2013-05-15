/*****************************************************************************
 *
 *  ut_ffs_control.c
 *
 *  Unit test for ../../src/ffs/ffs_control.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>
#include <mpi.h>

#include "u/libu.h"
#include "ffs_control.h"
#include "ut_ffs_control.h"

int ut_control(u_test_case_t * tc) {

  ffs_control_t * ffs = NULL;

  u_dbg("Start");

  u_test_err_if(ffs_control_create(MPI_COMM_WORLD, &ffs));
  u_test_err_if(ffs_control_start(ffs, "logs/unit-test-control"));

  u_dbg("Bad inputs");
  u_test_err_if(ffs_control_execute(ffs, "non_existant_file.inp") == 0);
  u_test_err_if(ffs_control_execute(ffs, "inputs/ut_control_bad1.inp") == 0);
  u_test_err_if(ffs_control_execute(ffs, "inputs/ut_control_bad2.inp") == 0);
  u_test_err_if(ffs_control_execute(ffs, "inputs/ut_control_bad3.inp") == 0);
  u_test_err_if(ffs_control_execute(ffs, "inputs/ut_control_bad4.inp") == 0);

  u_dbg("Good inputs");
  u_test_err_if(ffs_control_execute(ffs, "inputs/ut_control_ffs1.inp"));
  u_test_err_if(ffs_control_stop(ffs));
  ffs_control_free(ffs);

  u_dbg("Success");
  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_control_free(ffs);

  u_dbg("Failure");
  return U_TEST_FAILURE;
}
