/*****************************************************************************
 *
 *  ffs_sim.c
 *
 *****************************************************************************/

#include <string.h>
#include <float.h>
#include <mpi.h>

#include "u/libu.h"

#include "ffs_private.h"
#include "ut_ffs.h"

/*****************************************************************************
 *
 *  ffs_create
 *
 *  Test 'constructor' and the communicator.
 *
 *****************************************************************************/

int ut_ffs_create(u_test_case_t * tc) {

  int result;
  ffs_t * ffs = NULL;
  MPI_Comm comm = MPI_COMM_NULL;

  u_dbg("Start");
  u_test_err_if(ffs_create(MPI_COMM_WORLD, &ffs));

  u_test_err_if(ffs_comm(ffs, &comm));

  MPI_Comm_compare(MPI_COMM_WORLD, comm, &result);
  u_test_err_if(result != MPI_IDENT);

  ffs_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_ffs_command_line
 *
 *  Test the arvc, argv get constructed correctly.
 *
 *****************************************************************************/

int ut_ffs_command_line(u_test_case_t * tc) {

  ffs_t * ffs = NULL;
  char * argstring = "-x xarg";
  int argc;
  char ** argv = NULL;

  u_dbg("Start");
  u_test_err_if(ffs_create(MPI_COMM_WORLD, &ffs));

  u_test_err_if(ffs_command_line_set(ffs, argstring));
  u_test_err_if(ffs_command_line(ffs, &argc, &argv));
  u_test_err_if(argc != 3);

  u_test_err_if(strcmp(argv[0], FFS_SIM_EXECUTABLE_NAME) != 0);
  u_test_err_if(strcmp(argv[1], "-x") != 0);
  u_test_err_if(strcmp(argv[2], "xarg") != 0);
  u_test_err_if(argv[argc] != NULL);

  ffs_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ffs_exch
 *
 *****************************************************************************/

int ut_ffs_exch_int(u_test_case_t * tc) {

  ffs_t * ffs = NULL;

  int iref, io;

  u_dbg("Start\n");

  u_test_err_if(ffs_create(MPI_COMM_WORLD, &ffs));
  u_test_err_if(ffs_type_set(ffs, FFS_INFO_TIME_PUT, 1, FFS_VAR_INT));
  u_test_err_if(ffs_type_set(ffs, FFS_INFO_LAMBDA_PUT, 1, FFS_VAR_INT));

  iref = 11;
  u_test_err_if(ffs_info_int(ffs, FFS_INFO_TIME_PUT, 1, &iref));
  u_test_err_if(ffs_info_int(ffs, FFS_INFO_TIME_FETCH, 1, &io));
  u_test_err_if(io != iref);

  iref = 12;
  u_test_err_if(ffs_info_int(ffs, FFS_INFO_RNG_SEED_PUT, 1, &iref));
  u_test_err_if(ffs_info_int(ffs, FFS_INFO_RNG_SEED_FETCH, 1, &io));
  u_test_err_if(io != iref);

  iref = 13;
  u_test_err_if(ffs_info_int(ffs, FFS_INFO_LAMBDA_PUT, 1, &iref));
  u_test_err_if(ffs_info_int(ffs, FFS_INFO_LAMBDA_FETCH, 1, &io));
  u_test_err_if(io != iref);

  ffs_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (ffs) ffs_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
