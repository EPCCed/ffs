/*****************************************************************************
 *
 *  main.c
 *
 *  ISSUE: what is equivalent of of trun for branched?
 *
 *****************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "branched.h"
#include "direct.h"
#include "ffs.h"
#include "ffs_general.h"

void ffs_interface_info(ffs_param_t * ffs);
int  ffs_interface_read(ffs_param_t * ffs, const char * filename);
int  ffs_interface_check(ffs_param_t * ffs);
void ffs_interface_remove(ffs_param_t * ffs);
void ffs_random_init(ffs_param_t * ffs, int seed);
void ffs_tree_init(ffs_param_t * ffs);
void ffs_remove(ffs_param_t * ffs);


int main (int argc, char ** argv) {

  int seed;
  char filename_input[FILENAME_MAX] = "ffs.inp";
  ffs_param_t ffs;

  if (argc > 1) sprintf(filename_input, "%s", argv[1]);

  ffs_interface_read(&ffs, filename_input);
  ffs_interface_check(&ffs);
  ffs_interface_info(&ffs);

  seed = ffs_headseed(&ffs);
  ffs_random_init(&ffs, seed);
  ffs_tree_init(&ffs);

  if (ffs.algorithm == FFS_METHOD_DIRECT) direct_driver(&ffs);
  if (ffs.algorithm == FFS_METHOD_BRANCHED) branched_driver(&ffs);
  if (ffs.algorithm == FFS_METHOD_TEST) ffs_test_driver(&ffs);

  ffs_remove(&ffs);

  return 0;
}

/*****************************************************************************
 *
 *  ffs_interface_read
 *
 *****************************************************************************/

int ffs_interface_read(ffs_param_t * ffs, const char * filename) {

  int ifail = FFS_SUCCESS;
  int n, ndummy;
  char method[FILENAME_MAX];

  FILE * fp;

  fp = fopen(filename, "r");

  if (fp == NULL) {
    printf("Cannot open input file %s.\n", filename);
    ifail = FFS_FAILURE;
  }
  else {

    ffs->algorithm = FFS_METHOD_NULL;
    fscanf(fp, "%s", method);

    if (strcmp(method, "direct") == 0) ffs->algorithm = FFS_METHOD_DIRECT;
    if (strcmp(method, "branched") == 0) ffs->algorithm = FFS_METHOD_BRANCHED;
    if (strcmp(method, "test") == 0) ffs->algorithm = FFS_METHOD_TEST;

    if (ffs->algorithm == FFS_METHOD_DIRECT) printf("DIRECT FFS\n");
    if (ffs->algorithm == FFS_METHOD_BRANCHED) printf("BRANCHED FFS\n");

    if (ffs->algorithm == -1) {
      printf("Method not found - check input file\n");
      exit(0);
    }

    fscanf(fp, "%d%*s",  &(ffs->headseed));
    fscanf(fp, "%lf%*s", &(ffs->teq));
    fscanf(fp, "%lf%*s", &(ffs->trun));
    fscanf(fp, "%d%*s",  &(ffs->nskip));
    fscanf(fp, "%d%*s",  &(ffs->init_independent));
    fscanf(fp, "%lf%*s", &(ffs->init_paccept));
    fscanf(fp, "%d%*s",  &(ffs->nstepmax));
    fscanf(fp, "%d%*s",  &(ffs->nsteplambda));
    fscanf(fp, "%d%*s",  &(ffs->nlambda));

    n = ffs->nlambda + 1; /* Count interfaces from 1 */
    ffs->interface = (ffs_interface_t *) calloc(n, sizeof(ffs_interface_t));
    assert(ffs->interface != NULL);

    for (n = 1; n <= ffs->nlambda; n++) {
      fscanf(fp, "%d %lf %d %d %lf %d", &ndummy,
	     &(ffs->interface[n].lambda),
	     &(ffs->interface[n].nstates),
	     &(ffs->interface[n].ntrials),
	     &(ffs->interface[n].pprune),
	     &(ffs->interface[n].nskeep));
      assert(ndummy == n);
    }

    /* interface[0] is not active; for convenience lambda[0] = lambda[1] */

    ffs->interface[0].lambda = ffs->interface[1].lambda;
    ffs->interface[0].ntrials = 1;
    ffs->interface[0].nstates = 1;
    ffs->interface[0].pprune = 1.0;
    ffs->interface[0].weight = 1.0;

    fclose(fp);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  ffs_interface_check
 *
 *****************************************************************************/

int ffs_interface_check(ffs_param_t * ffs) {

  int n;
  int ifail = FFS_SUCCESS;

  printf("\n");
  printf("Checking supplied interface set...\n");

  if (ffs->nlambda < 2) {
    printf("There must be at least two interfaces!\n");
    ifail = FFS_FAILURE;
  }

  /* Check lambda monotonically increasing. */

  for (n = 1; n < ffs->nlambda; n++) {
    if (ffs->interface[n+1].lambda <= ffs->interface[n].lambda) {
      printf("Interface %d lambda %f <= interface %d lambda %f\n",
	     n+1, ffs->interface[n+1].lambda,
	     n, ffs->interface[n].lambda);
      ifail = FFS_FAILURE;
    }
  }

  if (ffs->algorithm == FFS_METHOD_BRANCHED) {
    printf(" - Branched method:\n");
    printf(" - the pruning probabilities will be set from ntrials\n");
    for (n = 1; n < ffs->nlambda; n++) {
      ffs->interface[n].pprune = 1.0 - 1.0 / (1.0*ffs->interface[n].ntrials);
    }
    ffs->interface[ffs->nlambda].pprune = 0.0;
  }

  if (ffs->interface[0].pprune != 1.0) {
    ffs->interface[0].pprune = 1.0;
  }

  if (ffs->interface[1].pprune != 1.0) {
    printf(" - setting probability of pruning at interface 1 to unity.\n");
    ffs->interface[1].pprune = 1.0;
  }

  assert(ifail == FFS_SUCCESS);
  printf("Interface definition looks ok.\n");

  return ifail;
}

/*****************************************************************************
 *
 *  ffs_interface_info
 *
 *****************************************************************************/

void ffs_interface_info(ffs_param_t * ffs) {

  int n;

  printf("\n");
  printf("Interfaces: %3d\n", ffs->nlambda);
  printf("\n");
  printf("index      lambda   ntrial   nstate  pprune\n");
  printf("-------------------------------------------\n");

  for (n = 1; n <= ffs->nlambda; n++) {
    printf("  %3d  %10.3e   %6d   %6d   %5.3f\n",
	   n, ffs->interface[n].lambda, ffs->interface[n].ntrials,
	   ffs->interface[n].nstates, ffs->interface[n].pprune);
  }

  printf("\n");

  return;
}

/*****************************************************************************
 *
 *  ffs_interface_remove
 *
 *****************************************************************************/

void ffs_interface_remove(ffs_param_t * ffs) {

  assert(ffs);

  free(ffs->interface);

  return;
}

/*****************************************************************************
 *
 *  ffs_random_init
 *
 *****************************************************************************/

void ffs_random_init(ffs_param_t * ffs, int seed) {

  long longseed;

  assert(seed > 0);
  assert(ffs);

  longseed = seed;

  ffs->random = ranlcg_create(longseed);

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_init
 *
 *****************************************************************************/

void ffs_tree_init(ffs_param_t * ffs) {

  assert(ffs);

  ffs->tree = ffs_tree_create();
  assert(ffs->tree);

  return;
}

/*****************************************************************************
 *
 *  ffs_remove
 *
 *****************************************************************************/

void ffs_remove(ffs_param_t * ffs) {

  assert(ffs);

  ranlcg_free(ffs->random);
  ffs_interface_remove(ffs);
  ffs_tree_remove(ffs->tree);

  printf("\n");
  printf("Tree data objects: %8d\n", ffs_tree_node_data_nallocated());
  printf("Tree node objects: %8d\n", ffs_tree_node_nallocated());
  printf("Tree objects:      %8d\n", ffs_tree_nallocated());
  printf("State objects:     %8d\n", ffs_state_nallocated());

  return;
}
