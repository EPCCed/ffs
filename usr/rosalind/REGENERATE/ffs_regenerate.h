/*-----------------Header file for general FFS simulation-------------------*/
 

/*---------------------------INCLUDE'S---------------------------------------*/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ising_regenerate.h"

/*----------------------Some uesful definitions------------------------------*/

#define EPS 0.00001
#define MAXPATH 100000

/*-----------------------Structure definitions-------------------------------*/


typedef struct section_type {

  double lambda_min;
  double lambda_max;
  int ranseed;

} Section;

extern Dyn dyn;
extern double    lambda_1; /* border of A region */
extern double    lambda_2; /* border of B region */
extern int    Nsects; /* number of sections */
extern Section *S;
extern int NBlocks;
extern int mseed_trials;
extern int Nchoose_p;
extern int Nchoose_b;
extern FILE *fp_paths;
extern FILE *fp_log;
extern FILE *fp_output;

double ran3_trials(int);
int nint(double);
void start_general();
void convert_point(Point, Point *);
double get_lambda(Point);
double get_mu(Point);
double get_kink_index(Point);
double get_right_time_corr(Point, Point);
double get_left_time_corr(Point, Point);
void start_specific();
void do_step(double *, Point *, int);
void output_data();
void init_run(Point);
void allocpoint(Point *);
void free_point(Point);
void copy_point(Point, Point *);
Point read_point();
Point trial_run (Point, int *, int, int *);
Point run_to_interface (Point, double, int, int *, int *);
Point get_ranseeds();
