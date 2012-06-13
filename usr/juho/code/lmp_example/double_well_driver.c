/* a simple driver to simulate a dimer system in WCA bath
   modified from simple.c from lammps examples
   juho
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "mpi.h"
#include "library.h"
#include "math.h"

/*using namespace LAMMPS_NS;*/

int main(int narg, char **arg)
{

  MPI_Init(&narg,&arg);

  if (narg != 5) {
    printf("Syntax: double_well_driver P in.lammps nrepeat nsample\n");
    exit(1);
  }
  
  int me, nprocs;

  MPI_Comm_rank(MPI_COMM_WORLD,&me);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  int nprocs_lammps = atoi(arg[1]);
  if (nprocs_lammps > nprocs) {
    if (me == 0)
      printf("ERROR: LAMMPS cannot use more procs than available\n");
    MPI_Abort(MPI_COMM_WORLD,1);
  }

  int lammps;
  if (me < nprocs_lammps) lammps = 1;
  else lammps = MPI_UNDEFINED;
  MPI_Comm comm_lammps;
  MPI_Comm_split(MPI_COMM_WORLD,lammps,0,&comm_lammps);

  /* open LAMMPS input script */

   FILE *fp;
  if (me == 0) {
    fp = fopen(arg[2],"r");
    if (fp == NULL) {
      printf("ERROR: Could not open LAMMPS input script\n");
      MPI_Abort(MPI_COMM_WORLD,1);
    }
  }

  /* run the input script thru LAMMPS one line at a time until end-of-file
     driver proc 0 reads a line, Bcasts it to all procs
     (could just send it to proc 0 of comm_lammps and let it Bcast)
     all LAMMPS procs call lammps_command() on the line */

  void *ptr;
  if (lammps == 1) lammps_open(0,NULL,comm_lammps,&ptr);

  int n;
  char line[1024];
  while (1) {
    if (me == 0) {
      if (fgets(line,1024,fp) == NULL) n = 0;
      else n = strlen(line) + 1;
      if (n == 0) fclose(fp);
    }
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    if (n == 0) break;
    MPI_Bcast(line,n,MPI_CHAR,0,MPI_COMM_WORLD);
    if (lammps == 1) lammps_command(ptr,line);
  }

  /*get the input variables for single run lenght and
    number of repeats 
  */
  int nrepeat=atoi(arg[3]);
  int nsample=atoi(arg[4]);
  char run_command[1024];
  sprintf(run_command,"run %d",nsample);
  
  int i=0;
  while(i < nrepeat){
    if (lammps == 1) {
      /*lammps_command(ptr,"run 1000");*/
      lammps_command(ptr,run_command);

      /* now extract the coordinates */
      int natoms = lammps_get_natoms(ptr);
      double *x = (double *) malloc(3*natoms*sizeof(double));
      lammps_get_coords(ptr,x);
      /*printf("coords %lf %lf\n",x[0],x[1]);*/

      /*get box sizes*/
      double xlo,xhi,ylo,yhi,zlo,zhi;
      double boxL[3];
      
      xlo = *((double *) lammps_extract_global(ptr,"boxxlo"));
      ylo = *((double *) lammps_extract_global(ptr,"boxylo"));
      zlo = *((double *) lammps_extract_global(ptr,"boxzlo"));
      
      xhi = *((double *) lammps_extract_global(ptr,"boxxhi"));
      yhi = *((double *) lammps_extract_global(ptr,"boxyhi"));
      zhi = *((double *) lammps_extract_global(ptr,"boxzhi"));
      /*printf("xlo/xhi = %lf/%lf\n",xlo,xhi);*/
      
      boxL[0] = xhi - xlo;
      boxL[1] = yhi - ylo;
      boxL[2] = zhi - zlo;

      /*calculate the distance between the atoms in the dimer
	here it is easy as we know that they are two first atoms
      */
      double rsq, r;
      
      rsq=0.0;
      for (n=0; n < 3; n++){
	r = x[n] - x[n+3];
	r = r - round(r/boxL[n])*boxL[n];
	rsq += r*r;
      }
      r = sqrt(rsq);
      
      /*get the step and print the result*/
      //int step = *((int *) lammps_extract_global(ptr,"step"));
      printf("separation = %lf\n",r);
      i++;
    }
  }

if (lammps == 1) lammps_close(ptr);
  /* close down MPI */
  MPI_Finalize();
}
