/*****************************************************************************
 *
 *  ffs_tree_node_data.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ffs_tree_node_data.h"

struct ffs_tree_node_data_type {

  int ntrial_total;
  int ntrial_succeeded;
  int ntrial_pruned;
  int ntrial_went_backwards;
  int ntrial_failed;

  int parent_rngseed;

  double weight;
  double time;
  double lambda;
};

static unsigned int nallocated_ = 0;
ffs_tree_node_data_t * ffs_tree_node_data_create(void);
void ffs_tree_noe_data_remove(ffs_tree_node_data_t * p);

/*****************************************************************************
 *
 *  ffs_tree_node_data_create
 *
 *****************************************************************************/

ffs_tree_node_data_t * ffs_tree_node_data_create(void) {

  ffs_tree_node_data_t * p;

  p = (ffs_tree_node_data_t *) calloc(1, sizeof(ffs_tree_node_data_t));
  assert(p);
  ++nallocated_;

  return p;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_remove
 *
 *****************************************************************************/

void ffs_tree_node_data_remove(ffs_tree_node_data_t * p) {

  assert(p);
  free(p);
  --nallocated_;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_nallocated
 *
 *****************************************************************************/

unsigned int ffs_tree_node_data_nallocated(void) {

  return nallocated_;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_rngseed
 *
 *****************************************************************************/

int ffs_tree_node_data_parent_seed(const ffs_tree_node_data_t * p) {

  assert(p);

  return p->parent_rngseed;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_rngseed_set
 *
 *****************************************************************************/

void ffs_tree_node_data_parent_seed_set(ffs_tree_node_data_t * p, int seed) {

  assert(p);
  p->parent_rngseed = seed;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_weight
 *
 *****************************************************************************/

double ffs_tree_node_data_weight(const ffs_tree_node_data_t * p) {

  assert(p);

  return p->weight;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_weight_set
 *
 *****************************************************************************/

void ffs_tree_node_data_weight_set(ffs_tree_node_data_t * p, double w) {

  assert(p);
  p->weight = w;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_time
 *
 *****************************************************************************/

double ffs_tree_node_data_time(const ffs_tree_node_data_t * p) {

  assert(p);
  return p->time;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_time_set
 *
 *****************************************************************************/

void ffs_tree_node_data_time_set(ffs_tree_node_data_t * p, double t) {

  assert(p);
  p->time = t;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_lambda
 *
 *****************************************************************************/

double ffs_tree_node_data_lambda(const ffs_tree_node_data_t * p) {

  assert(p);
  return p->lambda;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_lambda_set
 *
 *****************************************************************************/

void ffs_tree_node_data_lambda_set(ffs_tree_node_data_t * p, double lambda) {

  assert(p);
  p->lambda = lambda;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_write
 *
 *  Currently ASCII plus a blob!
 *
 *****************************************************************************/

int ffs_tree_node_data_write(ffs_tree_node_data_t * p, FILE * fp) {

  int ifail = 0;
  size_t nwrite;

  assert(p);
  assert(fp);

  fprintf(fp, "  node data total         %d\n", p->ntrial_total);
  fprintf(fp, "  node data succeeded     %d\n", p->ntrial_succeeded);
  fprintf(fp, "  node data pruned        %d\n", p->ntrial_pruned);
  fprintf(fp, "  node data wentbackwards %d\n", p->ntrial_went_backwards);
  fprintf(fp, "  node data failed        %d\n", p->ntrial_failed);
  fprintf(fp, "  node data weight        %f\n", p->weight);
  fprintf(fp, "  node data time          %f\n", p->time);
  fprintf(fp, "  node data lambda        %f\n", p->lambda);

  nwrite = fwrite(p, sizeof(ffs_tree_node_data_t), 1, fp);
  assert(nwrite == 1);

  if (nwrite != 1) ifail = 1;

  return ifail;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_read
 *
 *****************************************************************************/

int ffs_tree_node_data_read(ffs_tree_node_data_t * p, FILE * fp) {

  int ifail = 0;
  size_t nread;
  char line[FILENAME_MAX];

  assert(p);
  assert(fp);

  /* Currently 8 lines of ASCII (ignored) and actual binary data */

  for (nread = 1; nread <= 8; nread++) {
    fgets(line, FILENAME_MAX, fp);
  }

  nread = fread(p, sizeof(ffs_tree_node_data_t), 1, fp);
  assert(nread == 1);

  if (nread != 1) ifail = 1;

  return ifail;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_selftest
 *
 *****************************************************************************/

int ffs_tree_node_data_selftest(void) {

  int nfail = 0;
  ffs_tree_node_data_t * data;
  ffs_tree_node_data_t * data_copy;

  char filename[L_tmpnam];
  FILE * fp;

  if (ffs_tree_node_data_nallocated() != 0) ++nfail;

  data = ffs_tree_node_data_create();
  if (ffs_tree_node_data_nallocated() != 1) ++nfail;
  if (data == NULL) ++nfail;

  /* A representative check on whether the data are zero. */
  if (ffs_tree_node_data_parent_seed(data) != 0) ++nfail;

  ffs_tree_node_data_parent_seed_set(data, 1);
  if (ffs_tree_node_data_parent_seed(data) != 1) ++nfail;

  /* Read/write */

  data_copy = ffs_tree_node_data_create();
  assert(data_copy != NULL);

  tmpnam(filename);
  fp = fopen(filename, "a+");
  assert(fp);

  nfail += ffs_tree_node_data_write(data, fp);
  rewind(fp);
  nfail += ffs_tree_node_data_read(data_copy, fp);

  fclose(fp);
  remove(filename);

  assert(ffs_tree_node_data_parent_seed(data_copy)
	 == ffs_tree_node_data_parent_seed(data));

  ffs_tree_node_data_remove(data_copy);
  ffs_tree_node_data_remove(data);
  if (ffs_tree_node_data_nallocated() != 0) ++nfail;

  return nfail;
}
