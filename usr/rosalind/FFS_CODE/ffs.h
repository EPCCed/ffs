/*-----------------Header file for general FFS simulation-------------------*/
 

/*---------------------------INCLUDE'S---------------------------------------*/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ising.h"

/*----------------------Some uesful definitions------------------------------*/

#define EQUIL       0
#define RUN         1
#define EPS 0.00001
#define MAXPATH 100000
#define MAXPOINTS 100000

/*-----------------------Structure definitions-------------------------------*/


typedef struct section_type {
  int N_reached;
  int N_reached_cum;
  double ave;
  double avesq;
  double Npoints;
  double lambda_min;
  double lambda_max;
  double forward_cum;
  double forward;
  int Ntrials;
  int acc;
  int rej;
 
} Section;


typedef struct ensemble_type{

  Point *p;
  int *parent;
  int *parent_seed;
  int N;

} Ensemble;


extern N1;
extern Dyn dyn;
extern double    lambda_1; /* border of A region */
extern double    lambda_2; /* border of B region */
extern int    Nsects; /* number of sections */
extern Section *S;
extern double Teq;
extern double Trun;
extern int Ncross;
extern int Ncross_b;
extern int Nskip;
extern int NBlocks;
extern int mseed_runn;
extern int mseed_trials;
extern int mseed_seeds;
extern int store_paths;
extern FILE *fp_paths;
extern FILE *fp_log;

Ensemble runn(double *, Point);
Point equil(Point);
double ran3_runn(void);
double ran3_trials(int);
double ran3_seeds(void);
int nint(double);
void start_general();
void allocate_memory();
void convert_point(Point, Point *);
double get_lambda(Point);
void start_specific(Point *);
void do_step(double *, Point *, int);
void output_data();
void init_run(Point);
void allocpoint(Point *);
void free_point(Point);
void copy_point(Point, Point *);
Ensemble get_paths(int, Ensemble);
Point trial_run (Point, int *, int, int *);
int get_point(Ensemble);
Point run_to_interface (Point, double, int, int *);
Ensemble convert_ensemble(Ensemble);
void free_ensemble(Ensemble);
void zero_plam();
Ensemble alloc_ensemble(int);
void write_trace (Ensemble);
void write_ensemble(Ensemble);
