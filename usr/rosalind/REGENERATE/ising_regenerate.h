/*-----This file is specific to the 2D Ising simulation-------------------*/
 

/*---------------------------INCLUDE'S---------------------------------------*/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*----------------------Some uesful definitions------------------------------*/



#define EPS 0.00001


/*-----------------------Structure definitions-------------------------------*/



typedef struct Point_type /* information we need to know about a phase space point */
{
  int **lattice; /* a point is a lattice of spins */
  int neigh1_x;
  int neigh2_x;
  double energy;
  double magnet;

} Point ;



typedef struct Dyn_type /*information we need to propagate the dynamical system */
{
  double shear_rate;
  int Nshear;
  double pshear;
  double jcoup;
  double kBT;
  int Nsize_x;
  int Nsize_y;
  double h;
  int move1, move2;
  int acc;
  int *shift_copy;

}Dyn;


void allocate_memory_specific();
void get_energy_and_mag(Point, double *, double *);
void output_data();
void shear_row(int, Point *);
void do_shear(Point *, double *, int);
void get_neighbours(int,int,int, int, int *, int *, int *, int *, int *, int *, int *, int *);
