/*****************************************************************************
 *
 *  ffs_param.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "ffs_util.h"
#include "ffs_param.h"

struct ffs_interface_type {

  double lambda;   /* Order parameter value */
  double pprune;   /* Probability of pruning */
  int    ntrials;  /* Number of trials to start from this interface */
  int    nstates;  /* Target number of states to generate at this interface */
  int    nskeep;   /* Target number of states to keep */

  double weight;   /* Result; weighted probability at this interface */

};

typedef struct ffs_interface_type ffs_interface_t;

struct ffs_param_type {

  int nlambda;                    /* Number of interfaces */
  ffs_interface_t * interfaces;   /* Array of interfaces [0,...,nlambda] */

  /* For interfaces where values are not defined explicitly... */

  int ntrials_default;            /* Default value of 'ntrials' */
  int nstates_default;            /* Default value of 'nstates' */
  int nskeep_default;             /* Default value of 'nskeep' */
  double pprune_default;          /* Default value of 'pprune' */ 

};

static int ffs_param_defaults_set(u_config_t * config, ffs_param_t * obj);
static int ffs_param_interfaces_parse(ffs_param_t * obj, u_config_t * config);
static int ffs_param_interfaces_set(u_config_t * config, ffs_param_t * obj);
static int ffs_param_interfaces_auto(u_config_t * config, ffs_param_t * obj);
static int ffs_param_interface_zero_set(ffs_param_t * obj);

/******************************************************************************
 *
 *  ffs_param_create
 *
 *****************************************************************************/

int ffs_param_create(u_config_t * config, ffs_param_t ** pobj) {

  ffs_param_t * obj = NULL;
  int nlambda;

  dbg_return_if(config == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = u_calloc(1, sizeof(ffs_param_t))) == NULL);
  u_config_get_subkey_value_i(config, FFS_CONFIG_NLAMBDA, 0, &nlambda);
  err_err_ifm(nlambda < 0, "Number of interfaces cannout be negative!");

  obj->nlambda = nlambda;
  obj->interfaces = u_calloc(nlambda + 1, sizeof(ffs_interface_t));
  err_err_sif(obj->interfaces == NULL);

  err_err_if(ffs_param_interfaces_parse(obj, config));

  *pobj = obj;

  return 0;

 err:
  if (obj) ffs_param_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_param_free
 *
 *****************************************************************************/

void ffs_param_free(ffs_param_t * obj) {

  dbg_return_if(obj == NULL, );

  u_free(obj->interfaces);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_param_nlambda
 *
 *****************************************************************************/

int ffs_param_nlambda(ffs_param_t * obj, int * n) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(n == NULL, -1);

  *n = obj->nlambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_lambda
 *
 *****************************************************************************/

int ffs_param_lambda(ffs_param_t * obj, int n, double * lambda) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(lambda == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  *lambda = obj->interfaces[n].lambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_ntrial
 *
 *****************************************************************************/

int ffs_param_ntrial(ffs_param_t * obj, int n, int * ntrials) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(ntrials == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  *ntrials = obj->interfaces[n].ntrials;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_nstate
 *
 *****************************************************************************/

int ffs_param_nstate(ffs_param_t * obj, int n, int * nstates) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nstates == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  *nstates = obj->interfaces[n].nstates;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_pprune
 *
 *****************************************************************************/

int ffs_param_pprune(ffs_param_t * obj, int n, double * pprune) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(pprune == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  *pprune = obj->interfaces[n].pprune;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_nskeep
 *
 *****************************************************************************/

int ffs_param_nskeep(ffs_param_t * obj, int n, int * nskeep) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nskeep == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  *nskeep = obj->interfaces[n].nskeep;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_check
 *
 *  - there must be at least two interfaces
 *  - the lambda values must be monotonically increasing
 *  - interface[1].pprune must be 1.0 (simply enforced)
 *
 *  Assumes all ranks receive the same input.
 *
 *****************************************************************************/

int ffs_param_check(ffs_param_t * obj) {

  int n;

  dbg_return_if(obj == NULL, -1);

  err_err_ifm(obj->nlambda < 2, "At least two interfaces are required");

  for (n = 1; n < obj->nlambda; n++) {
    err_err_ifm(obj->interfaces[n+1].lambda <= obj->interfaces[n].lambda,
		"Interface %d lambda %f <= interface %d lambda %f",
		n+1, obj->interfaces[n+1].lambda,
		n, obj->interfaces[n].lambda);
  }

  obj->interfaces[0].pprune = 1.0;
  if (obj->interfaces[1].pprune < 1.0) {
    u_warn("Setting interface[1] pprune = 1.0");
    obj->interfaces[1].pprune = 1.0;
  }

  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *  ffs_param_print_summary_fp
 *
 *  Prefer the log version.
 *
 *****************************************************************************/

int ffs_param_print_summary_fp(ffs_param_t * obj, FILE * fp) {

  int n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fp == NULL, -1);

  fprintf(fp, "\n");
  fprintf(fp, "Number of interfaces: %d\n", obj->nlambda);
  fprintf(fp, "Default ntrials:      %d\n", obj->ntrials_default);
  fprintf(fp, "Default nstates:      %d\n", obj->nstates_default);
  fprintf(fp, "Default nskeep:       %d\n", obj->nskeep_default);
  fprintf(fp, "Default pprune:       %5.3f\n", obj->pprune_default);
  fprintf(fp, "\n");

  fprintf(fp, "index      lambda   ntrial   nstate  nskeep   pprune\n");
  fprintf(fp, "----------------------------------------------------\n");

  for (n = 1; n <= obj->nlambda; n++) {
    fprintf(fp, "  %3d  %10.3e   %6d   %6d  %6d    %5.3f\n",
	    n, obj->interfaces[n].lambda, obj->interfaces[n].ntrials,
	    obj->interfaces[n].nstates, obj->interfaces[n].nskeep,
	    obj->interfaces[n].pprune);
  }

  fprintf(fp, "\n");

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_log_to_mpilog
 *
 *****************************************************************************/

int ffs_param_log_to_mpilog(ffs_param_t * obj, mpilog_t * log) {

  int n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(log == NULL, -1);

  mpilog(log, "\n");
  mpilog(log, "Number of interfaces: %d\n", obj->nlambda);
  mpilog(log, "Default ntrials:      %d\n", obj->ntrials_default);
  mpilog(log, "Default nstates:      %d\n", obj->nstates_default);
  mpilog(log, "Default nskeep:       %d\n", obj->nskeep_default);
  mpilog(log, "Default pprune:       %5.3f\n", obj->pprune_default);
  mpilog(log, "\n");

  mpilog(log, "index      lambda   ntrial   nstate  nskeep   pprune\n");
  mpilog(log, "----------------------------------------------------\n");

  for (n = 1; n <= obj->nlambda; n++) {
    mpilog(log, "  %3d  %10.3e   %6d   %6d  %6d    %5.3f\n",
	    n, obj->interfaces[n].lambda, obj->interfaces[n].ntrials,
	    obj->interfaces[n].nstates, obj->interfaces[n].nskeep,
	    obj->interfaces[n].pprune);
  }

  mpilog(log, "\n");

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_defaults_set
 *
 *  Set up the default interface values from the configuration
 *
 *  Some basic "default default" values are provided via the ref
 *  argument to u_config_get_subkey_value...
 *
 *****************************************************************************/

static int ffs_param_defaults_set(u_config_t * config, ffs_param_t * obj) {

  int n;

  dbg_err_if(config == NULL);
  dbg_err_if(obj == NULL);

  err_err_if(u_config_get_subkey_value_i(config, FFS_CONFIG_NTRIAL_DEFAULT,
					 FFS_NTRIAL_DEFAULT,
					 &obj->ntrials_default));
  err_err_if(u_config_get_subkey_value_i(config, FFS_CONFIG_NSTATE_DEFAULT,
					 FFS_NSTATE_DEFAULT,
					 &obj->nstates_default));
  err_err_if(u_config_get_subkey_value_i(config, FFS_CONFIG_NSKEEP_DEFAULT,
					 FFS_NSKEEP_DEFAULT,
					 &obj->nskeep_default));
  err_err_if(util_config_get_subkey_value_d(config,
					    FFS_CONFIG_PPRUNE_DEFAULT,
					    FFS_PPRUNE_DEFAULT,
					    &obj->pprune_default));

  for (n = 1; n <= obj->nlambda; n++) {
    obj->interfaces[n].ntrials = obj->ntrials_default;
    obj->interfaces[n].nstates = obj->nstates_default;
    obj->interfaces[n].nskeep = obj->nskeep_default;
    obj->interfaces[n].pprune = obj->pprune_default;
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_param_interfaces_parse
 *
 *****************************************************************************/

static int ffs_param_interfaces_parse(ffs_param_t * obj, u_config_t * config) {

  const char * key_a = NULL;
  const char * key_b = NULL;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(config == NULL, -1);

  key_a = u_config_get_subkey_value(config, FFS_CONFIG_LAMBDA_A);
  key_b = u_config_get_subkey_value(config, FFS_CONFIG_LAMBDA_B);

  /* Set the default values; look for an 'automatic' declaration;
   * and finally look for any explicit definition. */

  err_err_if(ffs_param_defaults_set(config, obj));

  if (key_a && key_b) {
    err_err_if(ffs_param_interfaces_auto(config, obj));
  }

  err_err_if(ffs_param_interfaces_set(config, obj));
  err_err_if(ffs_param_interface_zero_set(obj));


  return 0;
 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_param_interfaces_auto
 *
 *  Set evenly spaced lambda values based on lambda_a, lambda_b.
 *
 *****************************************************************************/

static int ffs_param_interfaces_auto(u_config_t * config, ffs_param_t * obj) {

  const char * key_a = NULL;
  const char * key_b = NULL;
  int n;
  double lambda;
  double lambda_a;
  double lambda_b;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(config == NULL, -1);

  key_a = u_config_get_subkey_value(config, FFS_CONFIG_LAMBDA_A);
  key_b = u_config_get_subkey_value(config, FFS_CONFIG_LAMBDA_B);

  err_err_if(u_atof(key_a, &lambda_a));
  err_err_if(u_atof(key_b, &lambda_b));

  obj->interfaces[0].lambda = lambda_a;
  obj->interfaces[1].lambda = lambda_a;

  for (n = 2; n <= obj->nlambda; n++) {
    lambda = lambda_a + (n - 1)*(lambda_b - lambda_a)/(obj->nlambda - 1);
    obj->interfaces[n].lambda = lambda;
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_param_interfaces_set
 *
 *  Set up the interface parameters from the configuration
 *
 *****************************************************************************/

static int ffs_param_interfaces_set(u_config_t * config, ffs_param_t * obj) {

  int n;
  u_string_t * key = NULL;
  u_config_t * subconf = NULL;
  const char * v;
  double value;

  dbg_err_if(config == NULL);
  dbg_err_if(obj == NULL);

  err_err_if(u_string_create("interfaceXXX", strlen("interfaceXXX"), &key));
  err_err_ifm(obj->nlambda > 999, "format botch");

  /* Look through interface 1 ... nlambda, from the config file */

  for (n = 1; n <= obj->nlambda; n++) {

    err_err_if(u_string_do_printf(key, 1, "%s%d", "interface", n));
    subconf = u_config_get_child(config, u_string_c(key));

    if (subconf == NULL) continue;

    err_err_if(util_config_get_subkey_value_d(subconf,
					      FFS_CONFIG_LAMBDA,
					      obj->interfaces[n].lambda,
					      &obj->interfaces[n].lambda));

    err_err_if(u_config_get_subkey_value_i(subconf,
					   FFS_CONFIG_NTRIAL,
					   obj->interfaces[n].ntrials,
					   &obj->interfaces[n].ntrials));

    err_err_if(u_config_get_subkey_value_i(subconf,
					   FFS_CONFIG_NSTATE,
					   obj->interfaces[n].nstates,
					   &obj->interfaces[n].nstates));

    err_err_if(u_config_get_subkey_value_i(subconf,
					   FFS_CONFIG_NSKEEP,
					   obj->interfaces[n].nskeep,
					   &obj->interfaces[n].nskeep));

    err_err_if(util_config_get_subkey_value_d(subconf,
					      FFS_CONFIG_PPRUNE,
					      obj->interfaces[n].pprune,
					      &obj->interfaces[n].pprune));
  }

  u_string_free(key);

  return 0;

 err:
  if (key) u_string_free(key);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_param_interface_zero_set
 *
 *  This is only for convenience of access to
 *  previous obj->interfaces[n-1] when n = 1
 *
 *****************************************************************************/

static int ffs_param_interface_zero_set(ffs_param_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->interfaces == NULL, -1);

  obj->interfaces[0].lambda = obj->interfaces[1].lambda;
  obj->interfaces[0].ntrials = 1;
  obj->interfaces[0].nstates = 1;
  obj->interfaces[0].pprune = 1.0;
  obj->interfaces[0].weight = 1.0;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_lambda_a
 *
 *****************************************************************************/

int ffs_param_lambda_a(ffs_param_t * obj, double * lambda) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->interfaces == NULL, -1);
  dbg_return_if(lambda == NULL, -1);

  *lambda = obj->interfaces[0].lambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_lambda_b
 *
 *****************************************************************************/

int ffs_param_lambda_b(ffs_param_t * obj, double * lambda) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->interfaces == NULL, -1);
  dbg_return_if(lambda == NULL, -1);

  *lambda = obj->interfaces[obj->nlambda].lambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_weight
 *
 *****************************************************************************/

int ffs_param_weight(ffs_param_t * obj, int n, double * wt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->interfaces == NULL, -1);
  dbg_return_if(wt == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  *wt = obj->interfaces[n].weight;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_weight_accum
 *
 *****************************************************************************/

int ffs_param_weight_accum(ffs_param_t * obj, int n, double wt) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->interfaces == NULL, -1);
  dbg_return_if(n < 0, -1);
  dbg_return_if(n > obj->nlambda, -1);

  obj->interfaces[n].weight += wt;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_pprune_set_default
 *
 *  All ranks are assumed to receive the same input.
 *
 *****************************************************************************/

int ffs_param_pprune_set_default(ffs_param_t * obj) {

  int n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->interfaces == NULL, -1);

  obj->interfaces[0].pprune = 1.0;
  obj->interfaces[1].pprune = 1.0;

  for (n = 2; n < obj->nlambda; n++) {
    err_err_if(obj->interfaces[n].ntrials == 0);
    obj->interfaces[n].pprune = 1.0 - 1.0 / (1.0*obj->interfaces[n].ntrials);
  }

  obj->interfaces[obj->nlambda].pprune = 0.0;

  return 0;

 err:

  return -1;
}
