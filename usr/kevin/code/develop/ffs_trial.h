/*****************************************************************************
 *
 *  ffs_trial.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef FFS_TRIAL_H
#define FFS_TRIAL_H

#include "ffs_tree_node.h"

enum ffs_trial_type_enum {FFS_TRIAL_TO_TIME,
			  FFS_TRIAL_TO_LAMBDA,
			  FFS_TRIAL_TO_EQUILIBRIUM};

enum ffs_trial_result_enum {FFS_TRIAL_UNDEFINED,
			    FFS_TRIAL_SUCCEEDED,
			    FFS_TRIAL_WENT_BACKWARDS,
			    FFS_TRIAL_TIMED_OUT,
			    FFS_TRIAL_IN_PROGRESS,
			    FFS_TRIAL_WAS_PRUNED,
			    FFS_TRIAL_FAILED};

typedef struct ffs_trial_type ffs_trial_t;

ffs_trial_t * ffs_trial_create(void);
unsigned int  ffs_trial_nallocated(void);
unsigned int  ffs_trial_nhighwater(void);

void              ffs_trial_remove(ffs_trial_t * p);
int               ffs_trial_result(const ffs_trial_t * p);
void              ffs_trial_result_set(ffs_trial_t * p, int result);
ffs_tree_node_t * ffs_trial_start_node(const ffs_trial_t * p);
void              ffs_trial_start_node_set(ffs_trial_t * p,
					   ffs_tree_node_t * start);
ffs_tree_node_t * ffs_trial_end_node(const ffs_trial_t * p);
void              ffs_trial_end_node_set(ffs_trial_t * p,
					 ffs_tree_node_t * end);
void              ffs_trial_to_lambda_set(ffs_trial_t * p, double lambda);
void              ffs_trial_to_time_set(ffs_trial_t * p, double time);
int               ffs_trial_type(const ffs_trial_t * p);
int               ffs_trial_init(ffs_trial_t * p);
int               ffs_trial_step(const ffs_trial_t * p);
double            ffs_trial_t0(const ffs_trial_t * p);
double            ffs_trial_target(const ffs_trial_t * p);

#endif
