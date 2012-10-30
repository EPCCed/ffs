/*************************************************************************//**
 *
 *  \file ffs_param.h
 *
 *  Interface description.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_PARAM_H
#define FFS_PARAM_H

#include <stdio.h>
#include "u/libu.h"
#include "mpilog.h"

/**
 *  \defgroup ffs_param FFS interface parameters
 *  \ingroup ffs_library
 *  \{
 *    FFS interface definition and parameters
 *
 *    \par
 *
 *  The object is best instantiated via the
 *  appropriate u_config_t configuration object read from input:
 *
 *  \code
 *  interfaces
 *  {
 *    nlambda           <integer>  # mandatory, at least 2 interfaces required
 *
 *    ntrial_default    <integer>  # optional, defaults to FFS_NTRIAL_DEFAULT
 *    nstate_default    <integer>  # optional, defaults to FFS_NSTATE_DEFAULT
 *    nskeep_default    <integer>  # optional, defaults to FFS_NSKEEP_DEFAULT
 *    pprune_default    <double>   # optional, defaults to FFS_PPRUNE_DEFAULT
 *
 *    interface1
 *    {
 *      lambda           <double>  #  mandatory for each interface
 *      ntrial           <integer> #  optional, defaults to ntrials_default
 *      nstate           <integer> #  optional, defaults to nstates_default
 *      nskeep           <integer> #  optional, defaults to nskeep_default
 *      pprune           <double>  #  optional, defaults to pprune_default
 *    }
 *    interface2
 *    ...
 *  }
 *  \endcode
 *
 *
 *  The series of lambda must be monotonically increasing; this and
 *  the other parameters are sanity checked via ffs_param_check().
 *
 *  As a short cut, it is also possible to merely define the number
 *  of interfaces, the starting point, and the end point in lambda space:
 *
 *  \code
 *  interfaces
 *  {
 *    nlambda          <integer>  # number of interfaces
 *    lambda_a         <double>   # starting lambda
 *    lambda_b         <double>   # finishing lambda
 *  }
 *  \endcode
 *
 *  This will give rise to nlambda equally spaced interfaces
 *  with delta lambda = (lambda_b - lambda_a) / (nlambda - 1).
 *  All the interfaces will have the same parameters taken from
 *  \c ntrials_default and so on.
 *  In this case, any specific interface definitions will be
 *  ignored.
 *
 *  Note that (nlambda + 1) interfaces are stored. interface[0] is
 *  added for convenience, and other interfaces are indexed using
 *  natural numbers 1, ..., nlambda.
 */

/**
 *  \brief Opaque interface parameter object
 */

typedef struct ffs_param_type ffs_param_t;

/**
 *  \def FFS_CONFIG_INTERFACES
 *  Key string for the top-level u_config_t interfaces section.
 *  \def FFS_CONFIG_NLAMBDA
 *  Key string for the number of interfaces.
 *  \def FFS_CONFIG_LAMBDA_A
 *  Key string for first lambda value ("state A")
 *  \def FFS_CONFIG_LAMBDA_B
 *  Key string for final lambda value ("state B")
 *  \def FFS_CONFIG_NTRIAL_DEFAULT
 *  Key string to set the default number of trials at each interface.
 *  \def FFS_CONFIG_NSTATE_DEFAULT
 *  Key string to set the default nuber of states to form anensemble
 *  at each interface.
 *  \def FFS_CONFIG_NSKEEP_DEFAULT
 *  Key string to set the number of states for which a permanent record
 *  (to file) is kept at each interface.
 *  \def FFS_CONFIG_PPRUNE_DEFAULT
 *  Key string to set the default pruning probility for trajectories
 *  reaching a given interface (in the backward direction).
 *  \def FFS_CONFIG_LAMBDA
 *  Key string to set lambda value for given interface.
 *  \def FFS_CONFIG_NTRIAL
 *  Key string to set number of trials for a given interface.
 *  \def FFS_CONFIG_NSTATE
 *  Key string to set number of states for given interface.
 *  \def FFS_CONFIG_NSKEEP
 *  Key string to set number of states to keep at given interface.
 *  \def FFS_CONFIG_PPRUNE
 *  Key string to identify pruning probability at given interface.
 *
 *  \def FFS_NTRIAL_DEFAULT
 *  Default value for number of trials per interface (if not set by user).
 *  \def FFS_NSTATE_DEFAULT
 *  Default value for number of states per interface (if not set by user).
 *  \def FFS_NSKEEP_DEFAULT
 *  Default value for number of states to keep per interface.
 *  \def FFS_PPRUNE_DEFAULT
 *  Default value for pruning probability (if not set by user).
 */

#define FFS_CONFIG_INTERFACES     "interfaces"
#define FFS_CONFIG_NLAMBDA        "nlambda"
#define FFS_CONFIG_LAMBDA_A       "lambda_a"
#define FFS_CONFIG_LAMBDA_B       "lambda_b"
#define FFS_CONFIG_NTRIAL_DEFAULT "ntrial_default"
#define FFS_CONFIG_NSTATE_DEFAULT "nstate_default"
#define FFS_CONFIG_NSKEEP_DEFAULT "nskeep_default"
#define FFS_CONFIG_PPRUNE_DEFAULT "pprune_default"
#define FFS_CONFIG_LAMBDA         "lambda"
#define FFS_CONFIG_NTRIAL         "ntrial"
#define FFS_CONFIG_NSTATE         "nstate"
#define FFS_CONFIG_NSKEEP         "nskeep"
#define FFS_CONFIG_PPRUNE         "pprune"

/*
 * Default values
 */

#define FFS_NTRIAL_DEFAULT        10
#define FFS_NSTATE_DEFAULT        2
#define FFS_NSKEEP_DEFAULT        0
#define FFS_PPRUNE_DEFAULT        0.0

/**
 *  \brief Create a new ffs_param_t object from the configuration details
 *
 *  \param  config      a valid configuration file of u_config_t
 *  \param  pobj        a pointer to the new object to be returned
 *
 *  \retval 0           a success
 *  \retval -1          a failure
 */

int ffs_param_create(u_config_t * config, ffs_param_t ** pobj);

/**
 *  \brief Release an ffs_param_t object
 *
 *  \param obj      the onject to be released
 *
 *  \returns        void
 */

void ffs_param_free(ffs_param_t * obj);

/**
 *  \brief Return the number of interfaces
 *
 *  \param  obj      the exsiting ffs_param_t object
 *  \param  nlambda  pointer to the number of interfaces to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure, i.e., a NULL pointer was supplied
 */

int ffs_param_nlambda(ffs_param_t * obj, int * nlambda);

/**
 *  \brief Return lambda value for nth interface
 *
 *  \param  obj       the ffs_param_t object
 *  \param  n         number of interface [0, ..., nlambda]
 *  \param  lambda    a pointer to the value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_param_lambda(ffs_param_t * obj, int n, double * lambda);

/**
 *  \brief Return number of trials for nth interface
 *
 *  \param  obj      the ffs_param_t object
 *  \param  n        interface number
 *  \param  ntrials  a pointer to the value to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int ffs_param_ntrial(ffs_param_t * obj, int n, int * ntrials);

/**
 *  \brief Return the target number of states at interface n
 *
 *  \param  obj       the ffs_para_t object
 *  \param  n         interface number
 *  \param  nstates   a pointer to the value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_param_nstate(ffs_param_t * obj, int n, int * nstates);

/**
 *  \brief  Return the pruning probability associated with nth interface
 *
 *  \param  obj           the ffs_param_t object
 *  \param  n             interface
 *  \param  pprune        pointer to the value to be returned
 *
 *  \retval 0             a success
 *  \retval -1            a failure
 */

int ffs_param_pprune(ffs_param_t * obj, int n, double * pprune);

/**
 *  \brief  Return the number of states to keep at nth interface
 *
 *  \param  obj          the ffs_param_t object
 *  \param  n            interface
 *  \param  nskeep       pointer to value to be returned
 *
 *  \retval 0            a success
 *  \retval -1           a failure
 */

int ffs_param_nskeep(ffs_param_t * obj, int n, int * nskeep);

/**
 *  \brief Perform a number of validation checks on interfaces
 *
 *  - there must be at least two interfaces
 *  - the lambda values must be monotonically increasing
 *  - interface[1].pprune must be 1.0 (simply enforced)
 *
 *  \param  obj  the initialised ffs_param_t object
 *
 *  \retval 0    a success
 *  \retval -1   a failure
 */

int ffs_param_check(ffs_param_t * obj);

/**
 *  \brief  Print a tabulated summary of the interfaces
 *
 *  \param  obj     the ffs_param_t object
 *  \param  fp      stream for output
 *
 *  \retval 0       a success
 *  \retval -1      a failure (a NULL pointer was supplied)
 */

int ffs_param_print_summary_fp(ffs_param_t * obj, FILE * fp);

/**
 *  \brief Log human-readable summary
 *
 *  \param  obj     the ffs_param_t object
 *  \param  log     an mpilog_t object
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_param_log_to_mpilog(ffs_param_t * obj, mpilog_t * log);

/**
 *  \brief Return first interface lambda value ("state A")
 *
 *  \param obj       the ffs_param_t object
 *  \param lambda    a pointer to the double value to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer was received or no interfaces were present
 */

int ffs_param_lambda_a(ffs_param_t * obj, double * lambda);

/**
 *  \brief Return last interface lambda value ("state B")
 *
 *  \param obj       the ffs_param_t object
 *  \param lambda    a pointer to the double value to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a NULL pointer was received or no interfaces were present
 */

int ffs_param_lambda_b(ffs_param_t * obj, double * lambda);

/**
 *  \brief Accumulate weight contribution for given interface
 *
 *  \param  obj     the ffs_param_t data type
 *  \param  n       the index of the interface
 *  \param  wt      the weight to accumulate
 *
 *  \retval 0       a success
 *  \retval -1      the weight could not be accumulated
 */

int ffs_param_weight_accum(ffs_param_t * obj, int n, double wt);

/**
 *  \brief Get the weight value for a given interface
 *
 *  \param  obj      the ffs_param_t data type
 *  \param  n        the index of the interface
 *  \param  wt       a pointer to the double value to be returned
 *
 *  \retval 0        a success
 *  \retval -1       no value could be returned
 */

int ffs_param_weight(ffs_param_t * obj, int n, double * wt);

/**
 *  \brief Set the pruning probabilities to \c 1 - 1/ntrial 
 *
 *  \param  obj       the ffs_param_t data type
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 *  This is the standard situation for branched FFS. For the first
 *  interface the pruning probability is 1, and for the last the
 *  probabiltity is 0, irrespective of ntrial.
 */   

int ffs_param_pprune_set_default(ffs_param_t * obj);

/**
 *  \}
 */

#endif
