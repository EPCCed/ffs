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

int ut_inst(u_test_case_t * tc) {

  int id = -1;
  ffs_inst_t * inst = NULL;
  u_config_t * config = NULL;

  u_dbg("Start");

  dbg_err_if(u_config_load_from_file("inputs/ut_inst1.inp", &config));

  dbg_err_if(ffs_inst_create(0, MPI_COMM_WORLD, &inst));
  dbg_err_if(ffs_inst_id(inst, &id));
  dbg_err_if(id != 0);

  dbg_err_if(ffs_inst_start(inst, "logs/ut_inst_create-0000.log", "w+"));
  dbg_err_if(ffs_inst_execute(inst, config));
  dbg_err_if(ffs_inst_config(inst));
  dbg_err_if(ffs_inst_stop(inst));

  u_config_free(config);
  ffs_inst_free(inst);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (config) u_config_free(config);
  if (inst) ffs_inst_free(inst);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
