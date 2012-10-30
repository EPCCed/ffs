/*****************************************************************************
 *
 *  ffs_param.c
 *
 *  Interface parameters. The object should be instantiated via the
 *  appropriate u_config_t configuration object read from input:
 *
 *  interfaces
 *  {
 *    nlambda            <integer>   mandatory, at least 2 interfaces required
 *
 *    ntrials_default    <integer>   optional, defaults to 1
 *    nstates_default    <integer>   optional, defaults to 1
 *    nskeep_default     <integer>   optional, defaults to 1
 *    pprune_default     <double>    optional, defaults to 0.0
 *
 *    interface1
 *    {
 *      lambda           <double>    mandatory for each interface
 *      ntrials          <integer>   optional, defaults to ntrials_default
 *      nstates          <integer>   optional, defaults to nstates_default
 *      nskeep           <integer>   optional, defaults to nskeep_default
 *      pprune           <double>    optional, defaults to pprune_default
 *    }
 *    interface2
 *    ...
 *  }
 *
 *
 *  The series of lambda should be monotonically increasing; this and
 *  the other parameters are sanity checked via ffs_param_check().
 *
 *  Note that (nlambda + 1) interfaces are stored. interface[0] is
 *  added for convenience, and other interfaces are indexed using
 *  natural numbers 1, ..., nlambda.
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

  double weight;

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
static int ffs_param_interfaces_set(u_config_t * config, ffs_param_t * obj);

/******************************************************************************
 *
 *  \brief Create a new ffs_param_t object from the configuration details
 *
 *  \param  config      a valid configuration file of u_config_t
 *  \param  pobj        a pointer to the new object to be returned
 *
 *  \retval 0           a success
 *  \retval -1          a failure
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

  /* Defaults should be set first, then interface values */
  err_err_if(ffs_param_defaults_set(config, obj));
  err_err_if(ffs_param_interfaces_set(config, obj));
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
 *  \param  obj     the exsiting ffs_param_t object
 *  \param  n       pointer to the number of interfaces to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a failure, i.e., a NULL pointer was supplied
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
 *  \brief Return the lambda value associated with interface n
 *
 *  \param  obj       the ffs_param_t object
 *  \param  n         number of interface [0, ..., nlambda]
 *  \param  lambda    a pointer to the value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
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
 *  \brief Return ntrials for the nth interface
 *
 *  \param  obj      the ffs_param_t object
 *  \param  n        interface number
 *  \param  ntrials  a pointer to the value to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure
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
 *  \brief Return the target number of states at interface n
 *
 *  \param  obj       the ffs_para_t object
 *  \param  n         interface number
 *  \param  nstates   a pointer to the value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
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
 *  \brief  Return the pruning probability associated with nth interface
 *
 *  \param  obj           the ffs_param_t object
 *  \param  n             interface
 *  \param  pprune        pointer to the value to be returned
 *
 *  \retval 0             a success
 *  \retval -1            a failure
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
 *  \brief  Return the number of states to keep at nth interface
 *
 *  \param  obj          the ffs_param_t object
 *  \param  n            interface
 *  \param  nskeep       pointer to value to be returned
 *
 *  \retval 0            a success
 *  \retval -1           a failure
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
 *  \brief  Validate before use
 *
 *  - there must be at least two interfaces
 *  - the lambda values must be monotonically increasing
 *  - interface[1].pprune must be 1.0 (simply enforced)
 *
 *  \param  obj  the initialised ffs_param_t object
 *
 *  \retval 0    a success
 *  \retval -1   a failure
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
 *  \brief  Print a tabulated summary of the interfaces
 *
 *  \param  obj     the ffs_param_t object
 *  \param  fp      stream
 *
 *  \retval 0       a success
 *  \retval -1      a failure (a NULL pointer was supplied)
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
 *  \brief Set up the default interface values from the configuration
 *
 *  Some basic "default default" values are provided via the ref
 *  argument to u_config_get_subkey_value...
 *
 *  \param  config     the configuration object "interfaces"
 *  \param  obj        the ffs_param_t object
 *
 *  \retval 0          default values set correctly
 *  \retval -1         a failure
 *
 *****************************************************************************/

static int ffs_param_defaults_set(u_config_t * config, ffs_param_t * obj) {

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
  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  \brief Set up the interface parameters from the configuration
 *
 *  \param  config      the configuration object "interfaces"
 *  \param  obj         the ffs_param_t object
 *
 *  \retval 0           interface values set correctly
 *  \retval -1          a problem was encountered
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
  err_err_if(obj->nlambda > 999);

  /* Interface zero is set at the end; get the rest, i.e.,
   * 1 ... nlambda, from the config file */

  for (n = 1; n <= obj->nlambda; n++) {
    err_err_if(u_string_do_printf(key, 1, "%s%d", "interface", n));
    subconf = u_config_get_child(config, u_string_c(key));
    err_err_ifm(subconf == NULL, "Expecting key %s but found none",
		u_string_c(key));

    v = u_config_get_subkey_value(subconf, FFS_CONFIG_LAMBDA);
    err_err_ifm(v == NULL, "Expecting %s for %s", FFS_CONFIG_LAMBDA,
		u_string_c(key));
    err_err_if(u_atof(v, &value));
    obj->interfaces[n].lambda = value;

    err_err_if(u_config_get_subkey_value_i(subconf, FFS_CONFIG_NTRIAL,
					   obj->ntrials_default,
					   &obj->interfaces[n].ntrials));
    err_err_if(u_config_get_subkey_value_i(subconf, FFS_CONFIG_NSTATE,
					   obj->nstates_default,
					   &obj->interfaces[n].nstates));
    err_err_if(u_config_get_subkey_value_i(subconf, FFS_CONFIG_NSKEEP,
					   obj->nskeep_default,
					   &obj->interfaces[n].nskeep));
    err_err_if(util_config_get_subkey_value_d(subconf, FFS_CONFIG_PPRUNE,
					      obj->pprune_default,
					      &obj->interfaces[n].pprune));
  }

  /* Interface 0. This is only for convenience of access to
   * previous obj->interfaces[n-1] when n = 1 */

  obj->interfaces[0].lambda = obj->interfaces[1].lambda;
  obj->interfaces[0].ntrials = 1;
  obj->interfaces[0].nstates = 1;
  obj->interfaces[0].pprune = 1.0;
  obj->interfaces[0].weight = 1.0;

  u_string_free(key);

  return 0;

 err:
  if (key) u_string_free(key);

  return -1;
}
