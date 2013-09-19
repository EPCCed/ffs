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
#include "ffs_result_aflux.h"
#include "ffs_result_summary.h"
#include "../sim/proxy.h"

#include "util/ranlcg.h"
#include "util/mpilog.h"

/**
 *  \defgroup ffs_trial FFS trial
 *  \ingroup  ffs_library
 *
 *  A data structure to hold trial information and related functions.
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
  ffs_result_aflux_t * flux;
  ffs_result_summary_t * summary;
  MPI_Comm xcomm;
  MPI_Comm inst_comm;
};

/**
 *  \brief Run a trial to fixed lambda
 *
 *  \param  trial       ffs_trial_arg_t structure
 *  \param  lambda_min  lambda below which trial will be terminated
 *  \param  lambda_max  target lambda
 *  \param  status      pointer to integer status to be returned
 *
 *  \retval 0           a success
 *  \retval -1          a failure
 *
 *  On successful exit, status will hold a value of ffs_trial_enum_t
 */

int ffs_trial_run_to_lambda(ffs_trial_arg_t * trial, double lambda_min,
			    double lambda_max, int * status);

/**
 *  \brief Run a trial to fixed time
 *
 *  \param proxy     the proxy_t for the simulation
 *  \param teq       the target time
 *  \param nstepmax  the maximum number of simulation steps allowed
 *  \param status    pointer to the integer status to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int ffs_trial_run_to_time(proxy_t * proxy, double teq,
			  int nstepmax, int * status);

/**
 *  \brief Start pruning process for a trial
 */

int ffs_trial_prune(ffs_trial_arg_t * trial, int interface, ranlcg_t * ran,
		    double * wt, int * status);

/**
 *  \brief Set (or reset) the current state to be the initial state
 *
 *  \param  trial      ffs_trial_arg_t structure
 *  \param  state      ffs_state_t identifying the initial state
 *  \param  ran        ranlcg_t to use to initialise simulation RNG seed
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 */

int ffs_trial_initial_state(ffs_trial_arg_t * trial, ffs_state_t * state,
			    ranlcg_t * ran);

/**
 *  \brief Generate a state at first interface from reference state
 *
 *  \param trial      ffs_trial_arg_t structure
 *  \param sinit      ffs_state_t identfying the initial (reference) state
 *  \param rantraj    ranlcg_t trajectory or trial RNG
 *  \param nlocaltraj integer local (to MPI task) trajectory id
 *  \param itraj      integer global trajectory id
 *  \param status     pointer to integer stateus to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_trial_init(ffs_trial_arg_t * trial, ffs_state_t * sinit,
		   ranlcg_t * rantraj, int nlocaltraj, int itraj,
		   int * status);
/**
 *  \}
 */

#endif
