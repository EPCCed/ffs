/*-----This file is specific to the Gillespie simulation-------------------*/
 

/*---------------------------INCLUDE'S---------------------------------------*/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*----------------------Some uesful definitions------------------------------*/



#define EPS 0.00001
#define MAXTRIALS 500
#define MAXPROD     10
#define MAXHIST     80

/*-----------------------Structure definitions-------------------------------*/

typedef struct Point_type /* information we need to know about a phase space point */
{
  int *X; /* a point is a set of numbers of molecules. For cell cyles there would also be volume information here */

} Point ;

typedef struct stoch_type {

  int  index, change;

} Stoch;

typedef struct reaction_type {

  int    Nreact, Nprod;
  double k;
  Stoch  react[2],prod[MAXPROD];

} React;



typedef struct Dyn_type /*information we need to propagate the dynamical system */
{
  int    Ncomp, Nreact;
  React *R; /* reactions */
  char   **Xname; /* names of components */
  double *a;
  double sum_a;
 
}Dyn;




void determine_propensity_functions (Point);
void select_reaction (int *);
Point update_concentrations (int, Point);
Point read_components ();
void read_reactions  ();
void propagate_time (double *);
void allocate_memory_specific();
void print_reactions();
