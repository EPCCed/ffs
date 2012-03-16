/*****************************************************************************
 *
 *  \file mpi.c
 *
 *  Unit tests for MPI replacement mpi.c
 *
 *****************************************************************************/

#include <mpi.h>
#include "u/libu.h"

int u_test_mpi_init(u_test_case_t * tc) {

  int mpi_is_initialised = 1;

  u_test_err_if(MPI_Initialized(&mpi_is_initialised));
  u_test_err_if(mpi_is_initialised == 0);

  return U_TEST_SUCCESS;

 err:

  return U_TEST_FAILURE;
}

int u_test_mpi_comm(u_test_case_t * tc) {

  int rank, size;
  MPI_Comm newcomm;

  u_test_err_if(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
  u_test_err_if(MPI_Comm_size(MPI_COMM_WORLD, &size));
  u_test_err_if(rank != 0);
  u_test_err_if(size != 1);

  u_test_err_if(MPI_Comm_dup(MPI_COMM_WORLD, &newcomm));
  u_test_err_if(MPI_Comm_rank(newcomm, &rank));
  u_test_err_if(MPI_Comm_size(newcomm, &size));
  u_test_err_if(rank != 0);
  u_test_err_if(size != 1);
  u_test_err_if(MPI_Comm_free(&newcomm));

  u_test_err_if(MPI_Comm_split(MPI_COMM_WORLD, 0, 0, &newcomm));
  u_test_err_if(MPI_Comm_rank(newcomm, &rank));
  u_test_err_if(MPI_Comm_size(newcomm, &size));
  u_test_err_if(rank != 0);
  u_test_err_if(size != 1);
  u_test_err_if(MPI_Comm_free(&newcomm));

  return U_TEST_SUCCESS;

 err:

  return U_TEST_FAILURE;
}

