/*****************************************************************************
 *
 *  gillespie.c
 *
 *  A simple implementation of the Gillespie algorithm to provide a
 *  test of forward flux sampling.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "ranlcg.h"
#include "gillespie.h"

#define MAXPROD     10


typedef struct Point_type {

  int * X; /* a point is a set of numbers of molecules. */

} Point ;

struct gillespie_state_type {
  double t; /* Current time */
  int * nx; /* A set of integer numbers of molecules */
};

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
 
} Dyn;


static Point read_components ();
static void read_reactions  ();
static void allocate_memory_specific();
static void print_reactions();
static Point allocpoint();
static void free_point(Point);

static Dyn dyn;
static double nalloc_ = 0.0;
static double nalloc_state_ = 0.0;
static ranlcg_t * random_generator_ = NULL;
static double nalloc_current(void);

static Point allocpoint() {

  Point output;

  output.X = (int *) calloc(dyn.Ncomp, sizeof(int));
  nalloc_ += 1.0;
 
  return output;

}

/*****************************************************************************
 *
 *  state_allocate
 *
 *  Return a pointer to a new state.
 *
 *****************************************************************************/

state_t * state_allocate(void) {

  state_t * p;

  p = (state_t *) calloc(1, sizeof(state_t));
  assert(p);

  p->nx = (int *) calloc(dyn.Ncomp, sizeof(int));
  assert(p->nx);
  ++nalloc_state_;

  return p;
}

/*****************************************************************************
 *
 *  state_free
 *
 *****************************************************************************/

void state_free(state_t * p) {

  assert(p);
  assert(p->nx);

  free(p->nx);
  free(p);
  --nalloc_state_;

  return;
}

/*****************************************************************************
 *
 *  state_clone
 *
 *****************************************************************************/

state_t * state_clone(const state_t * pold) {

  state_t * pnew;

  assert(pold);

  pnew = state_allocate();
  state_copy(pold, pnew);

  return pnew;
}

/*****************************************************************************
 *
 *  state_copy
 *
 *****************************************************************************/

void state_copy(const state_t * pold, state_t * pnew) {

  int n;

  assert(pold);
  assert(pnew);

  pnew->t = pold->t;

  for (n = 0; n < dyn.Ncomp; n++) {
    pnew->nx[n] = pold->nx[n];
  }

  return;
}

/*****************************************************************************
 *
 *  state_time
 *
 *****************************************************************************/

double state_time(const state_t * p) {

  assert(p);

  return p->t;
}

/*****************************************************************************
 *
 *  state_time_set
 *
 *****************************************************************************/

void state_time_set(state_t * p, double t) {

  assert(p);
  p->t = t;

  return;
}

/*****************************************************************************
 *
 *  gillespie_state_rng_set
 *
 *****************************************************************************/

void gillespie_rng_state_set(long int seed) {

  ranlcg_state_set(random_generator_, seed);

  return;
}

/*****************************************************************************
 *
 *  state_to_lambda
 *
 *  This is order parameter (na - nb)
 *     for such-and-such test problem
 *     na = total number of A molecules
 *     nb = total number of B molecules
 *
 *****************************************************************************/

double state_to_lambda(const state_t * p) {

  int na, nb;
  double lambda;

  assert(p);

  na = p->nx[0] + 2*(p->nx[2] + p->nx[5] + p->nx[7]);
  nb = p->nx[1] + 2*(p->nx[3] + p->nx[6] + p->nx[7]);
  lambda = 1.0*(na - nb);

  return lambda;
}

/*****************************************************************************
 *
 *  gillespie_do_step
 *
 *  Advance state p by one step.
 *
 *****************************************************************************/

int gillespie_do_step(state_t * p) {

  double tstep;
  double rs;
  double cumu_a;
  int i, j;
  int ifail = 0;

  /* determine propensity functions */

  dyn.sum_a = 0.0;

  for (i = 0; i < dyn.Nreact; i++) {

    if (dyn.R[i].Nreact == 0) { 
      dyn.a[i] = dyn.R[i].k;
    }
    else if (dyn.R[i].Nreact == 1) { 
      dyn.a[i] = dyn.R[i].k * p->nx[dyn.R[i].react[0].index];
    }
    else if (dyn.R[i].react[0].index == dyn.R[i].react[1].index) {
      dyn.a[i] = dyn.R[i].k * p->nx[dyn.R[i].react[0].index]
	* (p->nx[dyn.R[i].react[1].index] - 1);
    }
    else {
      dyn.a[i] = dyn.R[i].k * p->nx[dyn.R[i].react[0].index]
	* p->nx[dyn.R[i].react[1].index];
    }

    dyn.sum_a += dyn.a[i];
  }

  /* propagate time  */

  if (dyn.sum_a < FLT_EPSILON) { 
    /* No reactions are possible. */
    ifail = 1;
  }
  else {

    /* We rely here on the fact that ranlcg does not produce zero. */
    rs = ranlcg_reep(random_generator_);
    tstep = log(1./rs)/dyn.sum_a;

    p->t += tstep;

    /* select reaction */

    rs = ranlcg_reep(random_generator_); 
    rs *= dyn.sum_a;

    j = 0;
    cumu_a = dyn.a[j];
    while (cumu_a < rs) cumu_a += dyn.a[++j];

    /* update concentrations */

    for (i = 0; i < dyn.R[j].Nreact; i++) {
      p->nx[dyn.R[j].react[i].index] --;
    }

    for (i = 0; i < dyn.R[j].Nprod; i++) {
      p->nx[dyn.R[j].prod[i].index] += dyn.R[j].prod[i].change;
    }
  }

  return ifail;
}

/*****************************************************************************
 *
 *  gillespie_read_state
 *
 *****************************************************************************/

int gillespie_read_state(const char * filename, state_t * p) {

  int  i, ncomp;
  int ifail = 0;

  FILE * fp;

  fp = fopen(filename, "r");

  if (fp == NULL) {
    ifail = 1;
    printf("read state failed to find %s\n", filename);
  }
  else {

    fscanf(fp, "%d\n", &ncomp);

    if (ncomp != dyn.Ncomp) {
      printf("The number of components is %d\n", ncomp);
      printf("The number of components should be %d\n", dyn.Ncomp);
      ifail = 1;
    }
    else {

      for (i = 0; i < dyn.Ncomp; i++) {
	fscanf(fp, "%d\t\t%s\n", &(p->nx[i]), dyn.Xname[i]);
      }

      fscanf(fp, "%lf", &p->t);
    }

    if (ferror(fp)) {
      ifail = 1;
      perror("read state perror: ");
    }

    fclose(fp);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  gillespie_write_state
 *
 *****************************************************************************/

int gillespie_write_state(const char * filename, state_t * p) {

  int  i;
  int ifail = 0;

  FILE * fp;

  fp = fopen(filename, "w");

  if (fp == NULL) {
    ifail = 1;
    printf("write state failed to find %s\n", filename);
  }
  else {

    fprintf(fp, "%d\n", dyn.Ncomp);

    for (i = 0; i < dyn.Ncomp; i++) {
      fprintf(fp, "%d\t\t%s\n", p->nx[i], dyn.Xname[i]);
    }

    fprintf(fp, "%22.16e", p->t);

    if (ferror(fp)) {
      ifail = 1;
      perror("write state perror: ");
    }

    fclose(fp);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  gillespie_set_up
 *
 *****************************************************************************/

int gillespie_set_up(const char * filename) {

  int ifail = 0;
  Point pt;

  FILE * fp;

  fp = fopen(filename, "r");

  if (fp == NULL) {
    ifail = 1;
    printf("gillespie set up: cannot open file %s.\n", filename);
  }
  else {
    fscanf(fp,"%d%*s", &dyn.Ncomp);
    fscanf(fp,"%d%*s", &dyn.Nreact);  
    fclose(fp);
  
    allocate_memory_specific();

    /* READ COMPONENTS REQUIRED TO SET dyn.X !! */
    pt = read_components();
    free_point(pt);

    read_reactions();
    print_reactions();
  }

  random_generator_ = ranlcg_create(23);

  return ifail;
}

/*****************************************************************************
 *
 *  gillespie_tear_down
 *
 *****************************************************************************/

int gillespie_tear_down(void) {

  int i;
  int ifail = 0;

  for (i = 0; i < dyn.Ncomp; i++) free(dyn.Xname[i]);
  free(dyn.a);
  free(dyn.Xname);
  free(dyn.R);

  nalloc_current();

  ranlcg_free(random_generator_);

  return ifail;
}

static void allocate_memory_specific () {
  int i;

  dyn.R = (React *) calloc(dyn.Nreact,sizeof(React));
  dyn.Xname = (char **) calloc(dyn.Ncomp,sizeof(char *));
  for (i=0;i<dyn.Ncomp;i++) dyn.Xname[i] = (char *) calloc(30,sizeof(char));

  dyn.a = (double *) calloc(dyn.Nreact, sizeof(double));
 
  return;
}


static void print_reactions () {

  int i, j;
  
  printf("\nThe following reactions are simulated:\n\n");

  for (i = 0; i < dyn.Nreact; i++) {
    if (dyn.R[i].Nreact == 0) { 
      printf("0");
    }
    else {
      printf("%s ", dyn.Xname[dyn.R[i].react[0].index]);
    }

    for (j = 1; j < dyn.R[i].Nreact; j++) { 
      printf("+ %s ", dyn.Xname[dyn.R[i].react[j].index]);
    }
    printf(" ->  ");

    if (dyn.R[i].Nprod == 0) { 
      printf("0 ");
    }
    else {
      printf("%2d %s ", dyn.R[i].prod[0].change,
	     dyn.Xname[dyn.R[i].prod[0].index]);
    }

    for (j = 1; j < dyn.R[i].Nprod; j++) {
      printf("+ %2d %s ", dyn.R[i].prod[j].change,
	     dyn.Xname[dyn.R[i].prod[j].index]);
    }
    printf("k = %4.3f\n",dyn.R[i].k);
  }

  return;
}


static Point read_components () {

  int  i,Ncomp;
  FILE *fp;
  Point ptest;

  fp = fopen("gillespie.components", "r");

  fscanf(fp,"%d\n", &Ncomp);

  if (Ncomp != dyn.Ncomp) {
    printf("The number of components is %d\n", Ncomp);
    printf("The number of components should be %d\n", dyn.Ncomp);
    abort();
  }
  else { 
    ptest = allocpoint();
  }

  for (i = 0; i <dyn.Ncomp; i++) {
    fscanf(fp,"%d\t\t%s\n", &ptest.X[i], dyn.Xname[i]);
  }

  return ptest;
}

static void read_reactions () {

  int  i,j,Nreact;
  char dummy[40];
  FILE *fp;

  fp = fopen("gillespie.reactions","r");
  
  fscanf(fp,"%d%*s\n", &Nreact);

  if (Nreact != dyn.Nreact) {
    printf("The number of reactions is %d\n", Nreact);
    printf("The number of reactions should be %d\n", dyn.Nreact);
    abort();
  }
  else {

    for (i = 0; i < dyn.Nreact; i++) {
      fscanf(fp,"%lf %d %d %s\n", &dyn.R[i].k, &dyn.R[i].Nreact,
	     &dyn.R[i].Nprod, dummy);

      if (dyn.R[i].Nreact == 0) {
	fscanf(fp,"%s", dummy);
      }
      else {
	fscanf(fp,"%s %d", dummy, &dyn.R[i].react[0].index);
      }

      for (j = 1; j < dyn.R[i].Nreact; j++) { 
	fscanf(fp,"%s %s %d", dummy, dummy, &dyn.R[i].react[j].index);
      }
      fscanf(fp,"%s",dummy);
	
      if (dyn.R[i].Nprod == 0) {
	fscanf(fp,"%s", dummy);
      }
      else {
	fscanf(fp,"%d %s %d\n", &dyn.R[i].prod[0].change,
	       dummy, &dyn.R[i].prod[0].index);
      }

      for (j = 1; j <dyn.R[i].Nprod; j++) { 
	fscanf(fp,"%s %d %s %d\n", dummy, &dyn.R[i].prod[j].change,
	       dummy, &dyn.R[i].prod[j].index);
      }
    }
  }

  return;
}

static void free_point(Point p) {

  free(p.X);
  nalloc_ -= 1.0;
 
  return;
}
    
static double nalloc_current(void) {

  printf("Gillespie memory allocated old: %f new %f\n",nalloc_,nalloc_state_);

  return nalloc_state_;
}
