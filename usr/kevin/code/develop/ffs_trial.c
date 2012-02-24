/*****************************************************************************
 *
 *  ffs_trial.c
 *
 *  An object to encapsulate a trial, i.e., an attempt to move the
 *  simulation state forward in time, or in lambda.
 *
 *  The startpoint and endpoint of the trial are identied by nodes
 *  in the tree.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ffs_tree_node.h"
#include "ffs_trial.h"

struct ffs_trial_type {
  int seed;       /* Master RNG seed for this trial */
  int type;       /* Forward in time or forward in lambda */
  double target;  /* Either time, or lambda, depending on type */

  int status;     /* Currently unused */
  int result;     /* Reuslt code from ffs_trial_result_enum */

  int nstep;                     /* Active trial step counter */
  double t0;                     /* Initial time of active trial */
  double last_lambda;            /* Last recorded lambda of active trial */

  ffs_tree_node_t * startstate;  /* Starting point */
  ffs_tree_node_t * endstate;    /* End point (if any is reached) */
};

static unsigned int nallocated_ = 0;  /* Number currently allocated */
static unsigned int nhighwater_ = 0;  /* High water mark of allocation */


/*****************************************************************************
 *
 *  ffs_trial_create
 *
 *****************************************************************************/

ffs_trial_t * ffs_trial_create(void) {

  ffs_trial_t * p;

  p = (ffs_trial_t *) calloc(1, sizeof(ffs_trial_t));
  assert(p);

  p->result = FFS_TRIAL_UNDEFINED;

  ++nallocated_;
  nhighwater_ = (nallocated_ > nhighwater_) ? nallocated_ : nhighwater_;

  return p;
}

/*****************************************************************************
 *
 *  ffs_trial_remove
 *
 *****************************************************************************/

void ffs_trial_remove(ffs_trial_t * p) {

  assert(p);
  free(p);

  --nallocated_;

  return;
}

/*****************************************************************************
 *
 *  ffs_trial_start_node
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_trial_start_node(const ffs_trial_t * p) {

  assert(p);

  return p->startstate;
}

/*****************************************************************************
 *
 *  ffs_trial_start_node_set
 *
 *****************************************************************************/

void ffs_trial_start_node_set(ffs_trial_t * p, ffs_tree_node_t * node) {

  assert(p);

  p->startstate = node;

  return;
}

/*****************************************************************************
 *
 *  ffs_trial_end_node
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_trial_end_node(const ffs_trial_t * p) {

  assert(p);

  return p->endstate;
}

/*****************************************************************************
 *
 *  ffs_trial_end_node_set
 *
 *****************************************************************************/

void ffs_trial_end_node_set(ffs_trial_t * p, ffs_tree_node_t * s) {

  assert(p);
  p->endstate = s;

  return;
}

/*****************************************************************************
 *
 *  ffs_trial_nallocated
 *
 *****************************************************************************/

unsigned int ffs_trial_nallocated(void) {

  return nallocated_;
}

/*****************************************************************************
 *
 *  ffs_trial_result
 *
 *****************************************************************************/

int ffs_trial_result(const ffs_trial_t * p) {

  assert(p);

  return p->result;
}

/*****************************************************************************
 *
 *  ffs_trial_result_set
 *
 *****************************************************************************/

void ffs_trial_result_set(ffs_trial_t * p, int result) {

  assert(p);
  assert(result >= FFS_TRIAL_UNDEFINED && result <= FFS_TRIAL_FAILED);

  p->result = result;

  return;
}

/*****************************************************************************
 *
 *  ffs_trial_to_lambda_set
 *
 *****************************************************************************/

void ffs_trial_to_lambda_set(ffs_trial_t * p, double lambda) {

  assert(p);

  p->type = FFS_TRIAL_TO_LAMBDA;
  p->target = lambda;

  return;
}

/*****************************************************************************
 *
 *  ffs_trial_to_time_set
 *
 *****************************************************************************/

void ffs_trial_to_time_set(ffs_trial_t * p, double time) {

  assert(p);

  p->type = FFS_TRIAL_TO_TIME;
  p->target = time;

  return;
}

/*****************************************************************************
 *
 *  ffs_trial_type
 *
 *****************************************************************************/

int ffs_trial_type(const ffs_trial_t * p) {

  assert(p);

  return p->type;
}

/*****************************************************************************
 *
 *  ffs_trial_init
 *
 *****************************************************************************/

int ffs_trial_init(ffs_trial_t * p) {

  int ifail = 0;
  ffs_tree_node_data_t * data;

  assert(p);
  assert(p->startstate);

  data = ffs_tree_node_data(p->startstate);
  assert(data);

  p->nstep = 0;
  p->t0 = ffs_tree_node_data_time(data);
  p->last_lambda = ffs_tree_node_data_lambda(data);

  return ifail;
}

/*****************************************************************************
 *
 *  ffs_trial_step
 *
 *****************************************************************************/

int ffs_trial_step(const ffs_trial_t * p) {

  assert(p);

  return p->nstep;
}

/*****************************************************************************
 *
 *  ffs_trial_t0
 *
 *****************************************************************************/

double ffs_trial_t0(const ffs_trial_t * p) {

  assert(p);

  return p->t0;
}

/*****************************************************************************
 *
 *  ffs_trial_target
 *
 *****************************************************************************/

double ffs_trial_target(const ffs_trial_t * p) {

  assert(p);

  return p->target;
}

/*****************************************************************************
 *
 *  ffs_trial_nhighwater
 *
 *****************************************************************************/

unsigned int ffs_trial_nhighwater(void) {

  return nhighwater_;
}

/*****************************************************************************
 *
 *  ffs_trial_selftest
 *
 *****************************************************************************/

int ffs_trial_selftest(void) {

  int nfail = 0;
  ffs_trial_t * trial1;
  ffs_trial_t * trial2;
  ffs_tree_node_t * node1;
  ffs_tree_node_t * node2;

  if (ffs_trial_nallocated() != 0) ++nfail;
  if (ffs_trial_nhighwater() != 0) ++nfail;

  trial1 = ffs_trial_create();
  if (trial1 == NULL) ++nfail;
  if (ffs_trial_result(trial1) != FFS_TRIAL_UNDEFINED) ++nfail;

  if (ffs_trial_nallocated() != 1) ++nfail;

  trial2 = ffs_trial_create();
  if (trial2 == NULL) ++nfail;

  /* Attach and remove nodes */

  node1 = ffs_tree_node_create();
  ffs_trial_start_node_set(trial1, node1);
  node2 = ffs_trial_start_node(trial1);
  if (node2 != node1) ++nfail;

  ffs_trial_end_node_set(trial2, node1);
  node2 = ffs_trial_end_node(trial2);
  if (node2 != node1) ++nfail;

  ffs_tree_node_remove(node1);

  /* result, etc */

  ffs_trial_result_set(trial1, FFS_TRIAL_WAS_PRUNED);
  if (ffs_trial_result(trial1) != FFS_TRIAL_WAS_PRUNED) ++nfail;

  ffs_trial_remove(trial2);
  ffs_trial_remove(trial1);

  if (ffs_trial_nhighwater() != 2) ++nfail;
  if (ffs_trial_nallocated() != 0) ++nfail;

  nhighwater_ = 0;

  return nfail;
}
