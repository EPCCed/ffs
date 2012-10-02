
#include <stdlib.h>

#include "library.h"

int main (int argc, char ** argv) {

  void * lmp = NULL;

  lammps_open(argc, argv, MPI_COMM_WORLD, &lmp);
  if (lmp == NULL) return -1;
  lammps_close(lmp);

  return 0;
}
