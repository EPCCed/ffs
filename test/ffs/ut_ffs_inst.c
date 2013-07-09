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

/*****************************************************************************
 *
 *  ut_inst
 *
 *****************************************************************************/

int ut_inst(u_test_case_t * tc) {

  int id = -1;
  ffs_inst_t * inst = NULL;
  u_config_t * config = NULL;

  u_dbg("Start");

  dbg_err_if(u_config_load_from_file("inputs/ut_inst1.inp", &config));

  dbg_err_if(ffs_inst_create(0, MPI_COMM_WORLD, &inst));
  dbg_err_if(ffs_inst_id(inst, &id));
  dbg_err_if(id != 0);

  dbg_err_if(ffs_inst_start(inst, "logs/unit-test-inst.log", "w+"));
  dbg_err_if(ffs_inst_execute(inst, config));
  dbg_err_if(ffs_inst_config(inst));
  dbg_err_if(ffs_inst_stop(inst, NULL));

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

/*****************************************************************************
 *
 *  ut_inst_input
 *
 *****************************************************************************/

int ut_inst_input(u_test_case_t * tc) {

  ffs_inst_t * inst = NULL;
  u_config_t * config = NULL;
  proxy_t * proxy = NULL;
  ffs_t * ffs = NULL;

  int argc = 0;
  char ** argv = NULL;
  char lambda_name[BUFSIZ];

  u_dbg("Start");

  dbg_err_if( u_config_load_from_file("inputs/ut_inst2.inp", &config) );
  dbg_err_if( ffs_inst_create(0, MPI_COMM_WORLD, &inst) );
  dbg_err_if( ffs_inst_init_from_config(inst, config) );
  dbg_err_if( ffs_inst_start_proxy(inst) );

  dbg_err_if( ffs_inst_proxy(inst, &proxy) );
  dbg_err_if( proxy == NULL );
  dbg_err_if( proxy_ffs(proxy, &ffs) );

  /* Inspect:
   *   1. the command line argument ("./a.out test commnd")
   *   2. the lambda string ("test_lambda_function")
   */

  dbg_err_if( ffs_command_line_create_copy(ffs, &argc, &argv) );
  dbg_err_if( argc != 3 );
  dbg_err_if( strcmp(argv[1], "test") != 0 );
  dbg_err_if( strcmp(argv[2], "command") != 0 );
  dbg_err_if( ffs_command_line_free_copy(ffs, argc, argv) );

  dbg_err_if( ffs_lambda_name(ffs, lambda_name, BUFSIZ) );
  dbg_err_if( strcmp(lambda_name, "test_lambda_function") != 0 );

  /* Finish */

  dbg_err_if( ffs_inst_stop_proxy(inst) );

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
