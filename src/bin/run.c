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
#include "u/libu.h"
#include "ffs_util.h"

int main(int argc, char ** argv) {

  ffs_control_t * ffs = NULL;

  MPI_Init(&argc, &argv);

  u_log_set_hook(util_ulog, NULL, NULL, NULL);
  u_dbg("FFS u_dbg() messages are appearing here.");
  u_err("FFS u_err() messages are appearing here.");

  if (argc != 2) {
    printf("Usage: %s <input file>\n", argv[0]);
  }
  else {

    ffs_control_create(MPI_COMM_WORLD, &ffs);

    ffs_control_start(ffs, "testrun");
    ffs_control_execute(ffs, argv[1]);
    ffs_control_stop(ffs);
    ffs_control_free(ffs);
  }

  MPI_Finalize();

  return 0;
}

