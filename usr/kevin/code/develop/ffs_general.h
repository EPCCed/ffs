/*****************************************************************************
 *
 *  ffs_general.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef FFS_GENERAL_H
#define FFS_GENERAL_H

double ffs_param_lambda_a(const ffs_param_t * ffs);
double ffs_param_lambda_b(const ffs_param_t * ffs);
int    ffs_general_prune(sim_state_t * p, int interface, double * wt,
			 ffs_param_t * ffs);

#endif
