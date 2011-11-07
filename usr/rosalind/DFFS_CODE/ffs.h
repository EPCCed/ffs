/*-----------------Header file for general FFS simulation-------------------*/
 

/*---------------------------INCLUDE'S---------------------------------------*/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gillespie.h"

/*----------------------Some uesful definitions------------------------------*/

#define EQUIL       0
#define RUN         1
#define EPS 0.00001
#define MAXPATH 100000
#define MAXPOINTS 10000

/*-----------------------Structure definitions-------------------------------*/


typedef struct section_type {

  int Npoints;
  double lambda_min;
  double lambda_max;
  double forward;
  int Ntrials;
  int acc;
  int rej;
  int Nbins;
  double d_lambda;
  double *pl_histo;
  double pprune;

} Section;


typedef struct ensemble_type{

  Point p[MAXPOINTS];
  double wt[MAXPOINTS];
  int N;

} Ensemble;


extern Dyn dyn;
extern double    lambda_1; /* border of A region */
extern double    lambda_2; /* border of B region */
extern int    Nsects; /* number of sections */
extern Section *S;
extern double Teq;
extern double Trun;
extern int runsteps;
extern int firesteps;
extern int prunesteps;
extern int Ncross;
extern int Nskip;


Ensemble runn(double *, Point);
Point equil(Point);
Point run_to_interface (Point, double, int, int *, int *, double);
double ran3(void);
int nint(double);
int get_bin(double, int);
void start_general();
void allocate_memory();
Point convert_point(Point);
double get_lambda(Point);
Point start_specific();
Point do_step(double *, Point);
void output_data(double);
void init_run(Point);
Point allocpoint();
void free_point(Point);
Ensemble get_paths(int, Ensemble);
Point trial_run (Point, int *, int, double *);
double get_sumwt(Ensemble);
int get_point(Ensemble, double);
Point run_to_interface (Point, double, int, int *, int *, double);
Point prune(int, double *, Point, int *, int *);
Ensemble convert_ensemble(Ensemble);
void free_ensemble(Ensemble);
