/*****************************************************************************
 *
 *  ffs_general.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>

#include "ffs.h"
#include "ffs_trial.h"
#include "propagate.h" /* REAL TIME PRUNING WILL GET RID OF THIS */
#include "ffs_general.h"

ffs_tree_node_t * ffs_make_node(ffs_param_t * ffs, int id);
void ffs_make_trial(ffs_param_t * ffs, ffs_trial_t * trial);

/*****************************************************************************
 *
 *  ffs_param_lambda_a
 *
 *****************************************************************************/

int ffs_param_lambda_a(const ffs_param_t * ffs, double * lambda_a) {

  assert(ffs);
  assert(lambda_a);
  assert(ffs->interface);
  *lambda_a = ffs->interface[1].lambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_param_lambda_b
 *
 *****************************************************************************/

int ffs_param_lambda_b(const ffs_param_t * ffs, double * lambda_b) {

  assert(ffs);
  assert(lambda_b);
  assert(ffs->interface);
  *lambda_b = ffs->interface[ffs->nlambda].lambda;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_headseed
 *
 *****************************************************************************/

int ffs_headseed(const ffs_param_t * ffs) {

  assert(ffs);

  return ffs->headseed;
}

/*****************************************************************************
 *
 *  ffs_general_prune
 *
 *  Note that pruning for trials originating at interfaces 1 and 2
 *  is 'automatic'.
 *
 *  TO BE REPLACED BY 'REAL-TIME' PRUNING
 *
 *****************************************************************************/

int ffs_general_prune(sim_state_t * p, int interface, double * wt,
		      ffs_param_t * ffs) {
  int n;
  int istatus;
  double lambda_min, lambda_max;

  lambda_max = ffs->interface[interface + 1].lambda;
  istatus = FFS_TRIAL_WAS_PRUNED;

  for (n = interface; n > 2; n--) {

    if (ranlcg_reep(ffs->random) < ffs->interface[n - 1].pprune) {
      istatus = FFS_TRIAL_WAS_PRUNED;
      break;
    }

    /* Trial survives, update weight and continue... */

    *wt *= 1.0/(1.0 - ffs->interface[n - 1].pprune);

    lambda_min = ffs->interface[n - 2].lambda;
    istatus = simulation_run_to_lambda(p, lambda_min, lambda_max, ffs);

    if (istatus == FFS_TRIAL_SUCCEEDED) break;
  }

  return istatus;
}

/*****************************************************************************
 *
 *  ffs_test_driver
 *
 *****************************************************************************/

void ffs_test_driver(ffs_param_t * ffs) {

  ffs_tree_node_t * headnode;
  ffs_state_t * state;

  simulation_set_up(ffs);
  simulation_trial_state_initialise(ffs);

  headnode = ffs_make_node(ffs, ffs->headseed);
  ffs_tree_head_set(ffs->tree, headnode);

  /* ffs_init_ensemble(ffs);*/

  ffs_tree_write(ffs->tree, "junk.dat");

  /* Need something to clear or keep states from tree */
  state = ffs_tree_node_state(headnode);
  simulation_state_remove(ffs, state);

  ffs_tree_clear_contents(ffs->tree);

  simulation_tear_down();

  return;
}

/*****************************************************************************
 *
 *  ffs_init_ensemble
 *
 *****************************************************************************/

void ffs_init_ensemble(ffs_param_t * ffs, int nstates) {

  int n = 0;
  int result;
  double lambda_a;

  ffs_trial_t * trial;
  ffs_tree_node_t * init;
  ffs_tree_node_t * newnode;

  init = ffs_tree_head(ffs->tree);

  /* Set up trial */
  trial = ffs_trial_create();
  ffs_trial_start_node_set(trial, init);
  ffs_param_lambda_a(ffs, &lambda_a);
  ffs_trial_to_lambda_set(trial, lambda_a);

  while (n < nstates) {

    ffs_make_trial(ffs, trial);
    result = ffs_trial_result(trial);

    if (result == FFS_TRIAL_SUCCEEDED) {
      newnode = ffs_trial_end_node(trial);
      ffs_tree_node_child_add(init, newnode);
      ++n;
    }
    /*ffs_tree_data_result_add(init, result);*/
  }

  ffs_trial_remove(trial);

  return;
}

/*****************************************************************************
 *
 *  ffs_make_trial
 *
 *****************************************************************************/

void ffs_make_trial(ffs_param_t * ffs, ffs_trial_t * trial) {

  int seed;
  ffs_state_t * state;

  seed = ranlcg_reep_int32(ffs->random);
  simulation_state_rng_set(ffs, seed);

  state = ffs_tree_node_state(ffs_trial_start_node(trial));
  simulation_trial_state_set(ffs, state);

  simulation_trial_run(ffs, trial);

  return;
}

/*****************************************************************************
 *
 *  ffs_make_node
 *
 *  Take the current trial state, and generate a new tree node.
 *  This requires saving the state.
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_make_node(ffs_param_t * ffs, int id) {
 
  double t;
  double lambda;

  ffs_tree_node_t * node;
  ffs_tree_node_data_t * data;
  ffs_state_t * state = NULL;

  node = ffs_tree_node_create();

  /* ffs state */
  ffs_state_create(&state);
  ffs_state_id_set(state, id);
  simulation_trial_state_save(ffs, state);

  /* tree data */
  data = ffs_tree_node_data_create();

  simulation_trial_state_time(ffs, &t);
  ffs_tree_node_data_time_set(data, t);
  simulation_trial_state_lambda(ffs, &lambda);
  ffs_tree_node_data_lambda_set(data, lambda);
  ffs_tree_node_data_weight_set(data, 1.0);

  /* attach state, and data */
  ffs_tree_node_state_set(node, state);
  ffs_tree_node_data_set(node, data);

  return node;
}
