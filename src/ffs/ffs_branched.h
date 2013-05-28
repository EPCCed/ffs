/*****************************************************************************
 *
 *  ffs_brnached.h
 *
 *****************************************************************************/

#ifndef FFS_BRANCHED_H
#define FFS_BRANCHED_H

#include "ffs_init.h"
#include "ffs_param.h"
#include "ffs_result.h"
#include "../sim/proxy.h"
#include "util/mpilog.h"

/* This is a convenience aggregate argument list */

typedef struct ffs_trial_arg_s ffs_trial_arg_t;

struct ffs_trial_arg_s {
  ffs_init_t * init;
  ffs_param_t * param;
  proxy_t * proxy;
  int inst_id;
  int nproxy;
  int inst_seed;
  mpilog_t * log;
  ffs_result_t * result;
};

int ffs_branched_run(ffs_trial_arg_t * trial);
int ffs_direct_run(ffs_trial_arg_t * trial);

#endif
