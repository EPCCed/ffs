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

int ffs_branched_run(ffs_init_t * init, ffs_param_t * param, proxy_t * proxy,
		     int inst_id,
		     int inst_nsim, int seed, mpilog_t * log,
		     ffs_result_t * result);

#endif
