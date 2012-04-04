/*****************************************************************************
 *
 *  ut_ffs_inst.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>
#include <mpi.h>

#include "u/libu.h"
#include "ffs_inst.h"
#include "ut_ffs_inst.h"

int ut_inst_create(u_test_case_t * tc) {

  ffs_inst_t * inst = NULL;
  u_config_t * config = NULL;

  u_test_err_if(ffs_inst_create(0, MPI_COMM_WORLD, &inst));

  dbg_err_if(u_config_load_from_file("inputs/ut_inst1.inp", &config));
  u_test_err_if(ffs_inst_init(inst, config));

  u_test_err_if(ffs_inst_print_summary(inst));

  u_config_free(config);
  ffs_inst_free(inst);

  return U_TEST_SUCCESS;

 err:
  if (config) u_config_free(config);
  if (inst) ffs_inst_free(inst);

  return U_TEST_FAILURE;
}