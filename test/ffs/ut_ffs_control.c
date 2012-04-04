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

int ut_control_create(u_test_case_t * tc) {

  int ntask;
  ffs_control_t * ffs;

  u_test_err_if(ffs_control_create(MPI_COMM_WORLD, &ffs));
  u_test_err_if(ffs_control_init(ffs, "non_existant_file.inp") == 0);
  u_test_err_if(ffs_control_init(ffs, "inputs/ut_ffs1.inp"));

  MPI_Comm_size(MPI_COMM_WORLD, &ntask);
  if (ntask == 1) {
    u_test_err_if(ffs_control_print_summary_fp(ffs, stdout));
  }

  ffs_control_free(ffs);

  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_control_free(ffs);

  return U_TEST_FAILURE;
}
