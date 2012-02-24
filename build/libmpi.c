
#include <mpi.h>

int main(int argc, char ** argv) {

  int flag;

  MPI_Initialized(&flag);

  return 0;
}
