/*****************************************************************************
 *
 *  sim_dmc.c
 *
 *  A simple implementation of a dynamic Monte Carlo (hence DMC) simulation
 *  using the Gillespie algorithm.
 *
 *  See DT Gillespie, J. Phys. Chem. 81, 2340--2361 (1977).
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
#include "sim_dmc.h"

#define MAXPROD     10

typedef struct state_s state_t;
typedef struct stoch_s stoch_t;
typedef struct react_s react_t;
typedef struct dynam_s dynam_t;

struct state_s {
  double t;         /* Current time */
  int * nx;         /* A set of integer numbers of molecules */
  long int seed;    /* RNG state */
};

struct stoch_s {
  int index;
  int change;
};

struct react_s {
  int     nreactant;
  int     nproduct;
  double  k;
  stoch_t react[2];
  stoch_t prod[MAXPROD];
};

/* information we need to propagate the dynamical system */

struct dynam_s {
  int     ncomponent;     /* Number of components in the system */
  int     nreactions;     /* Number of reactions in the system */
  react_t * R;            /* list of reactions */
  char    **Xname;        /* names of components (strings) */
  double  * a;            /* List of propensities */
  double  sum_a;
  state_t state;
  ranlcg_t * rng;
};

static int dmc_read_components(dynam_t * dyn, const char * filename);
static int dmc_read_reactions(dynam_t * dyn, const char * filename);
static int dmc_print_reactions(dynam_t * dyn);
static int dmc_do_step(dynam_t * dyn);
static int dmc_read_state(dynam_t * dyn, const char * file, state_t * state);
static int dmc_write_state(dynam_t * dyn, const char * file, state_t * state);
static int dmc_init(dynam_t * dyn, int argc, char ** argv);
static int dmc_finish(dynam_t * dyn);
static int state_to_lambda(state_t dyn, int * lambda);


static dynam_t dyn;

/*****************************************************************************
 *
 *  sim_dmc_table
 *
 *****************************************************************************/

struct dmc_s {
  int nothing;
};

const interface_t sim_dmc_interface = {
  (interface_table_ft) &sim_dmc_table,
  (interface_create_ft) &sim_dmc_create,
  (interface_free_ft) &sim_dmc_free,
  (interface_execute_ft) &sim_dmc_execute,
  (interface_state_ft) &sim_dmc_state,
  (interface_lambda_ft) &sim_dmc_lambda,
  (interface_info_ft) &sim_dmc_info
};

int sim_dmc_table(interface_t * table) {

  *table = sim_dmc_interface;

  return 0;
}

/*****************************************************************************
 *
 *  sim_dmc_create
 *
 *  A non NULL-pointer must be returned.
 *
 *****************************************************************************/

int sim_dmc_create(sim_dmc_t ** pdmc) {

  /* Note that this is indeed never used. */
  *pdmc = calloc(1, sizeof(void *));

  return 0;
}

/*****************************************************************************
 *
 *  sim_dmc_free
 *
 *****************************************************************************/

int sim_dmc_free(sim_dmc_t * dmc) {

  free(dmc);

  return 0;
}

/*****************************************************************************
 *
 *  sim_dmc_execute
 *
 *****************************************************************************/

int sim_dmc_execute(sim_dmc_t * dmc, ffs_t * ffs, sim_execute_enum_t action) {

  int ifail = 0;
  int argc = 0;
  int sz = 0;
  char ** argv = NULL;
  double t;                /* Time is continuous in Gillespie */
  MPI_Comm comm;

  switch (action) {
  case SIM_EXECUTE_INIT:   /* INITIALISATION PHASE */

    ifail += ffs_comm(ffs, &comm);
    MPI_Comm_size(comm, &sz);
    if (sz > 1) {
      printf("The simulation cannot be run in parallel!\n");
      return -1;
    }

    ifail += ffs_command_line_create_copy(ffs, &argc, &argv);
    ifail += dmc_init(&dyn, argc, argv);

    ifail += ffs_type_set(ffs, FFS_INFO_TIME_PUT, 1, FFS_VAR_DOUBLE);
    ifail += ffs_type_set(ffs, FFS_INFO_LAMBDA_PUT, 1, FFS_VAR_INT);

    ifail += ffs_command_line_free_copy(ffs, argc, argv);
    argc = 0;
    argv = NULL;

    break;

  case SIM_EXECUTE_RUN:

    ifail += dmc_do_step(&dyn);
    t = dyn.state.t;
    ifail += ffs_info_double(ffs, FFS_INFO_TIME_PUT, 1, &t);

    break;

  case SIM_EXECUTE_FINISH:

    dmc_finish(&dyn);
    break;

  default:
    ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  sim_dmc_lambda
 *
 *  Always rank 0.
 *
 *****************************************************************************/

int sim_dmc_lambda(sim_dmc_t * dmc, ffs_t * ffs) {

  int lambda;

  state_to_lambda(dyn.state, &lambda);
  ffs_info_int(ffs, FFS_INFO_LAMBDA_PUT, 1, &lambda);

  return 0;
}

/*****************************************************************************
 *
 *  sim_dmc_state
 *
 *  For the filename, we just use the unique stub without adornment.
 *
 *****************************************************************************/

int sim_dmc_state(sim_dmc_t * dmc, ffs_t * ffs, sim_state_enum_t action,
		  const char * stub) {

  int ifail = 0;

  switch (action) {
  case SIM_STATE_INIT:
    /* Not required, or recover initial state */
    break;
  case SIM_STATE_READ:
    ifail = dmc_read_state(&dyn, stub, &dyn.state);
    break;
  case SIM_STATE_WRITE:
    ifail = dmc_write_state(&dyn, stub, &dyn.state);
    break;
  case SIM_STATE_DELETE:
    remove(stub);
    break;
  default:
    ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  sim_dmc_info
 *
 *****************************************************************************/

int sim_dmc_info(sim_dmc_t * dmc, ffs_t * ffs, ffs_info_enum_t param) {

  int ifail = 0;
  int seed;
  long int lseed;
  double t;

  switch (param) {
  case FFS_INFO_TIME_PUT:
    t = dyn.state.t;
    ifail += ffs_info_double(ffs, param, 1, &t);
    break;
  case FFS_INFO_LAMBDA_PUT:
    ifail += sim_dmc_lambda(dmc, ffs);
    break;
  case FFS_INFO_RNG_SEED_FETCH:
    ifail += ffs_info_int(ffs, FFS_INFO_RNG_SEED_FETCH, 1, &seed);
    lseed = seed;
    ifail += ranlcg_state_set(dyn.rng, lseed);
    break;
  default:
    /* FFS has asked for something we don't supply */
    ifail = -1;
  }

  return ifail;
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

int state_to_lambda(state_t p, int * lambda) {

  int na, nb;

  na = p.nx[0] + 2*(p.nx[2] + p.nx[5] + p.nx[7]);
  nb = p.nx[1] + 2*(p.nx[3] + p.nx[6] + p.nx[7]);
  *lambda = (na - nb);

  return 0;
}

/*****************************************************************************
 *
 *  gillespie_do_step
 *
 *  Advance state p by one step.
 *
 *****************************************************************************/

int dmc_do_step(dynam_t * dyn) {

  double tstep;
  double rs;
  double cumu_a;
  int i, j;
  int ifail = 0;

  /* determine propensity functions */

  dyn->sum_a = 0.0;

  for (i = 0; i < dyn->nreactions; i++) {

    if (dyn->R[i].nreactant == 0) { 
      dyn->a[i] = dyn->R[i].k;
    }
    else if (dyn->R[i].nreactant == 1) { 
      dyn->a[i] = dyn->R[i].k * dyn->state.nx[dyn->R[i].react[0].index];
    }
    else if (dyn->R[i].react[0].index == dyn->R[i].react[1].index) {
      dyn->a[i] = dyn->R[i].k * dyn->state.nx[dyn->R[i].react[0].index]
	* (dyn->state.nx[dyn->R[i].react[1].index] - 1);
    }
    else {
      dyn->a[i] = dyn->R[i].k * dyn->state.nx[dyn->R[i].react[0].index]
	* dyn->state.nx[dyn->R[i].react[1].index];
    }

    dyn->sum_a += dyn->a[i];
  }

  /* propagate time  */

  if (dyn->sum_a < FLT_EPSILON) { 
    /* No reactions are possible. */
    ifail = 1;
  }
  else {

    /* We rely here on the fact that ranlcg does not produce zero. */

    ranlcg_reep(dyn->rng, &rs);
    tstep = log(1./rs)/dyn->sum_a;

    dyn->state.t += tstep;

    /* select reaction */

    ranlcg_reep(dyn->rng, &rs); 
    rs *= dyn->sum_a;

    j = 0;
    cumu_a = dyn->a[j];
    while (cumu_a < rs) cumu_a += dyn->a[++j];

    /* update concentrations */

    for (i = 0; i < dyn->R[j].nreactant; i++) {
      dyn->state.nx[dyn->R[j].react[i].index] --;
    }

    for (i = 0; i < dyn->R[j].nproduct; i++) {
      dyn->state.nx[dyn->R[j].prod[i].index] += dyn->R[j].prod[i].change;
    }
  }

  return ifail;
}

/*****************************************************************************
 *
 *  dmc_read_state
 *
 *****************************************************************************/

int dmc_read_state(dynam_t * dyn, const char * filename, state_t * p) {

  int  i, ncomp;
  int ifail = 0;

  FILE * fp = NULL;

  fp = fopen(filename, "r");

  if (fp == NULL) {
    ifail = 1;
    printf("read state failed to find %s\n", filename);
  }
  else {

    fscanf(fp, "%d\n", &ncomp);

    if (ncomp != dyn->ncomponent) {
      printf("The number of components is %d\n", ncomp);
      printf("The number of components should be %d\n", dyn->ncomponent);
      ifail = 1;
    }
    else {

      for (i = 0; i < ncomp; i++) {
	fscanf(fp, "%d\t\t%s\n", &(p->nx[i]), dyn->Xname[i]);
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
 *  dmc_write_state
 *
 *****************************************************************************/

static int dmc_write_state(dynam_t * dyn, const char * filename, state_t * p) {

  int  i;
  int ifail = 0;

  FILE * fp = NULL;

  fp = fopen(filename, "w");

  if (fp == NULL) {
    ifail = 1;
    printf("write state failed to open %s\n", filename);
  }
  else {

    fprintf(fp, "%d\n", dyn->ncomponent);

    for (i = 0; i < dyn->ncomponent; i++) {
      fprintf(fp, "%d\t\t%s\n", p->nx[i], dyn->Xname[i]);
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
 *  dmc_init
 *
 *  A command line is expected in the following form:
 * 
 *  "./a.out <component file> <reaction file>"
 *
 *****************************************************************************/

int dmc_init(dynam_t * dyn, int argc, char ** argv) {

  int ifail = 0;

  if (argc < 3) return -1;

  ifail += dmc_read_components(dyn, argv[1]);
  ifail += dmc_read_reactions(dyn, argv[2]);
  ifail += dmc_print_reactions(dyn);
  ifail += ranlcg_create(23, &dyn->rng);

  return ifail;
}

/*****************************************************************************
 *
 *  dmc_finish
 *
 *****************************************************************************/

int dmc_finish(dynam_t * dyn) {

  int ir;

  for (ir = 0; ir < dyn->ncomponent; ir++) {
    free(dyn->Xname[ir]);
  }
  free(dyn->a);
  free(dyn->Xname);
  free(dyn->R);
  ranlcg_free(dyn->rng);

  return 0;
}

/*****************************************************************************
 *
 *  dmc_print_reactions
 *
 *****************************************************************************/

static int dmc_print_reactions(dynam_t * dyn) {

  int i, j;
  
  printf("\nThe following reactions are simulated:\n\n");

  for (i = 0; i < dyn->nreactions; i++) {
    if (dyn->R[i].nreactant == 0) { 
      printf("0");
    }
    else {
      printf("%s ", dyn->Xname[dyn->R[i].react[0].index]);
    }

    for (j = 1; j < dyn->R[i].nreactant; j++) { 
      printf("+ %s ", dyn->Xname[dyn->R[i].react[j].index]);
    }
    printf(" ->  ");

    if (dyn->R[i].nproduct == 0) { 
      printf("0 ");
    }
    else {
      printf("%2d %s ", dyn->R[i].prod[0].change,
	     dyn->Xname[dyn->R[i].prod[0].index]);
    }

    for (j = 1; j < dyn->R[i].nproduct; j++) {
      printf("+ %2d %s ", dyn->R[i].prod[j].change,
	     dyn->Xname[dyn->R[i].prod[j].index]);
    }
    printf("k = %4.3f\n", dyn->R[i].k);
  }

  return 0;
}

/*****************************************************************************
 *
 *  dmc_read_components
 *
 *  The file is expected to contain:
 *
 *  Number of components
 *  value_A  name_A
 *  value_B  name_B
 *  ...
 *
 *  with fmt %d\t\t%s\n
 *
 *****************************************************************************/

static int dmc_read_components(dynam_t * dyn, const char * filename) {

  int ic;
  int ncomp = 0;
  FILE * fp = NULL;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not read components: %s\n", filename);
    return -1;
  }

  fscanf(fp,"%d\n", &ncomp);
  if (ncomp < 1) {
    fclose(fp);
    return -1;
  }

  /* Assume all these allocations succeed */
  /* Allocate ncomp integers for the state values, a string for each
   * component name max length BUFSIZ */

  dyn->ncomponent = ncomp;
  dyn->state.nx = calloc(ncomp, sizeof(int));
  dyn->Xname = calloc(ncomp, sizeof(char *));

  for (ic = 0; ic < ncomp; ic++) {
    dyn->Xname[ic] = calloc(BUFSIZ, sizeof(char));
    fscanf(fp, "%d\t\t%s\n", &dyn->state.nx[ic], dyn->Xname[ic]);
  }

  fclose(fp);

  return 0;
}

/*****************************************************************************
 *
 *  dmc_read_reactions
 *
 *  We expect
 *
 *****************************************************************************/

static int dmc_read_reactions(dynam_t * dyn, const char * filename) {

  int  ir;            /* Reaction */
  int  jr, jp;        /* Reactant, product */
  int nreact = 0;
  char dummy[BUFSIZ];
  FILE * fp = NULL;

  fp = fopen(filename,"r");
  if (fp == NULL) {
    printf("Cannot read reactions from %s\n", filename);
    return -1;
  }

  fscanf(fp,"%d%*s\n", &nreact);
  if (nreact < 1) {
    fclose(fp);
    return -1;
  }

  /* Allocate nreact reactions, and propensities */

  dyn->nreactions = nreact;
  dyn->R = calloc(nreact, sizeof(react_t));
  dyn->a = calloc(nreact, sizeof(double));

  /* For each reaction ... */

  for (ir = 0; ir < nreact; ir++) {

    /* Read rate constant, number of reactants, number of products, (dummy) */

    fscanf(fp, "%lf %d %d %s\n", &dyn->R[ir].k, &dyn->R[ir].nreactant,
	   &dyn->R[ir].nproduct, dummy);

    /* Index of first reactant */
    if (dyn->R[ir].nreactant == 0) {
      fscanf(fp,"%s", dummy);
    }
    else {
      fscanf(fp,"%s %d", dummy, &dyn->R[ir].react[0].index);
    }

    /* Indices of remaining reactants */
    for (jr = 1; jr < dyn->R[ir].nreactant; jr++) { 
      fscanf(fp,"%s %s %d", dummy, dummy, &dyn->R[ir].react[jr].index);
    }

    /* "->" */
    fscanf(fp,"%s",dummy);

    /* Change and index of first product */
    if (dyn->R[ir].nproduct == 0) {
      fscanf(fp,"%s", dummy);
    }
    else {
      fscanf(fp,"%d %s %d\n", &dyn->R[ir].prod[0].change,
	     dummy, &dyn->R[ir].prod[0].index);
    }

    /* Change and index of remaing products */
    for (jp = 1; jp < dyn->R[ir].nproduct; jp++) { 
      fscanf(fp,"%s %d %s %d\n", dummy, &dyn->R[ir].prod[jp].change,
	     dummy, &dyn->R[ir].prod[jp].index);
    }
  }

  fclose(fp);

  return 0;
}
