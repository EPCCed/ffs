/*
 * mpicc
 * -I$(FFS_DIR)/src/ffs
 * -I$(FFS_DIR)/src/sim
 * main.c
 * -L$(FFS_DIR) -lffs
 * -L$(LMP_DIR) -llmp -lstdc++
 * -L$(FFTW_DIR) -lfftw
 * -L$(LIBU_DIR) -lu -lm
 */

#include "ffs_private.h"
#include "sim_lmp.h"

int main(int argc, char ** argv) {

  sim_lmp_t * sim = NULL;
  ffs_t * ffs = NULL;

  MPI_Init(&argc, &argv);

  ffs_create(MPI_COMM_WORLD, &ffs);
  sim_lmp_create(&sim);

  /* do something here */

  sim_lmp_free(sim);
  ffs_free(ffs);

  MPI_Finalize();

  return 0;
}
