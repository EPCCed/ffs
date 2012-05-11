
#include <mpi.h>
#include "u/libu.h"
#include "../src/util/u_extra.h"

int u_test_suite_ffs_register(u_test_t * t);
int u_test_suite_util_register(u_test_t * t);
int u_test_suite_mpi_register(u_test_t * t);

int main (int argc, char *argv[]) {

  int ifail;
  int rank;
  int nprocs;
  u_test_t * t = NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  u_extra_error_init(rank);

  u_test_new("FFS tests", &t);
  u_test_suite_util_register(t);
  u_test_suite_ffs_register(t);

  /* Replacement mpi tests only on 1 task */ 
  if (nprocs == 1) u_test_suite_mpi_register(t);

  ifail = u_test_run(argc, argv, t);

  MPI_Finalize();

  return ifail;
}
