/*****************************************************************************
 *
 *  ffs_simulation.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef FFS_SIMULATION_H
#define FFS_SIMULATION_H

#include "ffs.h"
#include "ffs_trial.h"

int ffs_simulation_start(ffs_param_t * ffs, ffs_trial_t * t);
int ffs_simulation_lambda_required(ffs_param_t * ffs, ffs_trial_t * t);
int ffs_simulation_status(ffs_param_t * ffs, ffs_trial_t * t, double time,
			  double lambda);

#endif
