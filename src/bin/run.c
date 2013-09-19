/*****************************************************************************
 *
 *  run.c
 *
 *  Stand-alone executable.
 *
 *****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "ffs_control.h"
#include "u/libu.h"
#include "ffs_util.h"

int main(int argc, char ** argv) {

  int rank;
  double t0;
  ffs_control_t * ffs = NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  t0 = MPI_Wtime();

  u_log_set_hook(util_ulog, NULL, NULL, NULL);

  if (rank == 0) u_dbg("FFS u_dbg() messages are appearing here.");
  if (rank == 0) u_err("FFS u_err() messages are appearing here.");

  if (argc != 2) {
    printf("Usage: %s <input file>\n", argv[0]);
  }
  else {

    ffs_control_create(MPI_COMM_WORLD, &ffs);

    ffs_control_start(ffs, "testrun");
    ffs_control_execute(ffs, argv[1]);
    ffs_control_summary(ffs);
    ffs_control_stop(ffs, NULL);
    ffs_control_free(ffs);

    MPI_Barrier(MPI_COMM_WORLD);
    t0 = MPI_Wtime() - t0;
    if (rank == 0) {
      printf("\n");
      printf("Elapsed time: %14.7e sec (%2.2d:%2.2d:%2.2d)\n", t0,
	     (int)(t0/3600.0),
	     (int) (fmod(t0, 3600.0)/60.0), (int)(fmod(t0, 60.0)));
    }
  }

  MPI_Finalize();

  return 0;
}

