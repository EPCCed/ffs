/*****************************************************************************
 *
 *  ffs_param.h
 *
 *****************************************************************************/

#ifndef FFS_PARAM_H
#define FFS_PARAM_H

#include <stdio.h>
#include "u/libu.h"

typedef struct ffs_param_type ffs_param_t;

#define FFS_CONFIG_INTERFACES     "interfaces"
#define FFS_CONFIG_NLAMBDA        "nlambda"
#define FFS_CONFIG_NTRIAL_DEFAULT "ntrial_default"
#define FFS_CONFIG_NSTATE_DEFAULT "nstate_default"
#define FFS_CONFIG_NSKEEP_DEFAULT "nskeep_default"
#define FFS_CONFIG_PPRUNE_DEFAULT "pprune_default"
#define FFS_CONFIG_LAMBDA         "lambda"
#define FFS_CONFIG_NTRIAL         "ntrial"
#define FFS_CONFIG_NSTATE         "nstate"
#define FFS_CONFIG_NSKEEP         "nskeep"
#define FFS_CONFIG_PPRUNE         "pprune"

#define FFS_NTRIAL_DEFAULT        10
#define FFS_NSTATE_DEFAULT        2
#define FFS_NSKEEP_DEFAULT        0
#define FFS_PPRUNE_DEFAULT        0.0

int ffs_param_create(u_config_t * config, ffs_param_t ** pobj);
void ffs_param_free(ffs_param_t * obj);
int ffs_param_nlambda(ffs_param_t * obj, int * nlambda);
int ffs_param_lambda(ffs_param_t * obj, int n, double * lambda);
int ffs_param_ntrial(ffs_param_t * obj, int n, int * ntrials);
int ffs_param_nstate(ffs_param_t * obj, int n, int * nstates);
int ffs_param_pprune(ffs_param_t * obj, int n, double * pprune);
int ffs_param_nskeep(ffs_param_t * obj, int n, int * nskeep);
int ffs_param_check(ffs_param_t * obj);
int ffs_param_print_summary_fp(ffs_param_t * obj, FILE * fp);

#endif
