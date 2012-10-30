/*****************************************************************************
 *
 *  ffs_brnached.h
 *
 *****************************************************************************/

#ifndef FFS_BRANCHED_H
#define FFS_BRANCHED_H

#include "ffs_inst.h"
#include "../sim/proxy.h"
#include "util/mpilog.h"

int ffs_brnached_run(ffs_inst_t * inst, proxy_t * proxy, int seed,
		     mpilog_t * log);

#endif
