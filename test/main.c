
#include <stdio.h>

#include <mpi.h>
#include "u/libu.h"
#include "../src/util/mpilog.h"

int u_test_suite_ffs_register(u_test_t * t);
int u_test_suite_util_register(u_test_t * t);
int u_test_suite_mpi_register(u_test_t * t);
int u_test_suite_smoke_register(u_test_t * t);

#include "sim/ut_suite.h"

int main (int argc, char *argv[]) {

  int ifail;
  int rank;
  int nprocs;
  char logfile[FILENAME_MAX];
  u_test_t * t = NULL;
  mpilog_t * uerrlog = NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  sprintf(logfile, "logs/error-%4.4d.log", rank);

  mpilog_create(MPI_COMM_WORLD, &uerrlog);
  mpilog_fopen(uerrlog, logfile, "w+");
  u_log_set_hook(mpilog_ulog, uerrlog, NULL, NULL);

  u_test_new("FFS tests", &t);
  u_test_suite_util_register(t);
  u_test_suite_ffs_register(t);
  u_test_suite_smoke_register(t);
  uts_sim_register(t);

  /* Replacement mpi tests only on 1 task */ 
  if (nprocs == 1) {
    u_test_suite_mpi_register(t);
    u_test_suite_depends_on("util", "mpi", t);
  }
 
  ifail = u_test_run(argc, argv, t);

  printf("Test result: %d\n", ifail);

  u_test_free(t);

  mpilog_fclose(uerrlog);
  mpilog_free(uerrlog);

  MPI_Finalize();

  return ifail;
}
