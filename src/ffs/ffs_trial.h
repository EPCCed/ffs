/*****************************************************************************
 *
 *  ffs_trial.h
 *
 *****************************************************************************/

#ifndef FFS_TRIAL_H
#define FFS_TRIAL_H

#include <mpi.h>

#include "ffs_init.h"
#include "ffs_param.h"
#include "ffs_state.h"
#include "ffs_result.h"
#include "../sim/proxy.h"

#include "util/ranlcg.h"
#include "util/mpilog.h"

/**
 *  \defgroup ffs_trial FFS trial
 *  \ingroup  ffs_library
 *
 *  A data structure to hold trial information and related fucntions.
 *
 */  


/* This is a convenience aggregate argument list */

typedef struct ffs_trial_arg_s ffs_trial_arg_t;

struct ffs_trial_arg_s {
  int nstepmax;
  int nsteplambda;
  double tsum;
  ffs_init_t * init;
  ffs_param_t * param;
  proxy_t * proxy;
  int inst_id;
  int nproxy;
  int inst_seed;
  mpilog_t * log;
  ffs_result_t * result;
  MPI_Comm xcomm;
  MPI_Comm inst_comm;
};

/**
 *  \brief Run a trial to fixed lambda
 *
 */

int ffs_trial_run_to_lambda(ffs_trial_arg_t * trial, double lambda_min,
			    double lambda_max, int * status);

/**
 *  \brief Run a trial to fixed time
 */

int ffs_trial_run_to_time(proxy_t * proxy, double teq,
			  int nstepmax, int * status);

/**
 *  \brief Start pruning process for a trial
 */

int ffs_trial_prune(ffs_trial_arg_t * trial, int interface, ranlcg_t * ran,
		    double * wt, int * status);

/**
 *  \brief Run an equilibration
 */

int ffs_trial_eq(proxy_t * proxy, ffs_state_t * state, int seed,
		 int nstepmax, double teq);

/**
 *  \brief Generate a state at first interface from reference state
 */

int ffs_trial_init(ffs_trial_arg_t * trial, ffs_state_t * sinit,
		   ranlcg_t * rantraj, int nlocaltraj, int itraj,
		   int * status);
/**
 *  \}
 */

#endif
