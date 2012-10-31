/*****************************************************************************
 *
 *  run.c
 *
 *  Stand-alone executable.
 *
 *****************************************************************************/

#include <stdio.h>
#include <mpi.h>

#include "ffs_control.h"

int main(int argc, char ** argv) {

  ffs_control_t * ffs = NULL;

  MPI_Init(&argc, &argv);

  if (argc != 2) {
    printf("Usage: %s <input file>\n", argv[0]);
  }
  else {

    ffs_control_create(MPI_COMM_WORLD, &ffs);

    ffs_control_start(ffs, "run.log", "w+");
    ffs_control_execute(ffs, argv[1]);
    ffs_control_stop(ffs);

    ffs_control_free(ffs);
  }

  MPI_Finalize();

  return 0;
}

