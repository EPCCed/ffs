/*-----------------Header file for brown dyn path sampling simulation-------------------*/
 

/*---------------------------INCLUDE'S---------------------------------------*/
 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gillespie.h"

/*----------------------Some uesful definitions------------------------------*/

#define EQUIL       0
#define RUN         1
#define MAXPATH 10000000
#define EPS 0.00001
#define MAXTRIALS 500

/*-----------------------Structure definitions-------------------------------*/

typedef struct section_type {

  double lambda_min;
  double lambda_max;
  int Ntrials;
  double pprune;
  double sumwt;
} Section;



/*-----------------------Externally declared variables----------------------*/

extern double    lambda_1; /* border of A region */
extern double    lambda_2; /* border of B region */
extern int    Nsects; /* number of sections */
extern Section *S;
extern double Teq;
extern int runsteps;
extern int firesteps;
extern int Ncross;
extern int Nskip;
extern Dyn dyn;
extern double sumw;
extern int n_starts;
extern double *pl_histo;
extern int Nbins;
extern double d_lambda;


/*-----------------------Externally declared functions----------------------*/ 

Point runn(double *, Point, Point);
Point equil(Point);
Point run_to_interface (Point, double, int, int *, int *, double);
double ran3(void);
int nint(double);
int get_bin(double);
void start_general();
void allocate_memory();
Point convert_point(Point);
double get_lambda(Point);
Point start_specific();
Point do_step(double *, Point);
void output_data(double,double);
void init_run(Point);
Point allocpoint();
void free_point(Point);
void enrich(int, double, Point, int, int);
void do_prune(int, double, Point, double, int, int);
void add_to_hists(double, double, double);

