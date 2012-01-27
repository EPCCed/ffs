/*****************************************************************************
 *
 *  ffs_tree.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinbrugh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ffs_tree_node.h"
#include "ffs_tree.h"

struct ffs_tree_type {

  ffs_tree_node_t * head;

};

static unsigned int nallocated_ = 0;

static int ffs_tree_node_write(ffs_tree_node_t * p, FILE * fp);
static ffs_tree_node_t * ffs_tree_node_read(FILE * fp);

/*****************************************************************************
 *
 *  ffs_tree_nallocated
 *
 *****************************************************************************/

unsigned int ffs_tree_nallocated(void) {

  return nallocated_;
}

/*****************************************************************************
 *
 *  ffs_tree_create
 *
 *****************************************************************************/

ffs_tree_t * ffs_tree_create(void) {

  ffs_tree_t * p;

  p = (ffs_tree_t *) calloc(1, sizeof(ffs_tree_t));
  assert(p);
  ++nallocated_;

  return p;
}

/*****************************************************************************
 *
 *  ffs_tree_remove
 *
 *****************************************************************************/

void ffs_tree_remove(ffs_tree_t * p) {

  free(p);
  --nallocated_;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_level_breadth
 *
 *****************************************************************************/

int ffs_tree_level_breadth(ffs_tree_t * p, int ltarget) {

  int count;

  assert(p);
  count = ffs_tree_node_level_count(p->head, ltarget, 0);

  return count;
}

/*****************************************************************************
 *
 *  ffs_tree_write
 *
 *****************************************************************************/

int ffs_tree_write(ffs_tree_t * p, const char * filename) {

  int nwrite = 0;
  FILE * fp;

  fp = fopen(filename, "w");

  if (fp == NULL) {
    nwrite = -1;
  }
  else {
    nwrite = ffs_tree_node_write(p->head, fp);
  }

  if (ferror(fp)) {
    nwrite = -1;
    perror("ffs_tree_write perror: ");
  }

  fclose(fp);

  return nwrite;
}

/*****************************************************************************
 *
 *  ffs_tree_read
 *
 *****************************************************************************/

int ffs_tree_read(ffs_tree_t * p, const char * filename) {

  int ifail = 0;
  FILE * fp;

  fp = fopen(filename, "r");

  if (fp == NULL) {
    ifail = -1;
  }
  else {
    p->head = ffs_tree_node_read(fp);
  }

  if (ferror(fp)) {
    ifail = -1;
    perror("ffs_tree_write perror: ");
  }

  fclose(fp);

  return ifail;
}

/*****************************************************************************
 *
 *  ffs_tree_node_write
 *
 *****************************************************************************/

static int ffs_tree_node_write(ffs_tree_node_t * p, FILE * fp) {

  int nwrite = 0;
  ffs_tree_node_t * node;

  if (p == NULL) {
    fprintf(fp, "node id %d\n", FFS_TREE_NODE_NULL);
  }
  else {
    ++nwrite;
    fprintf(fp, "node id %d\n", ffs_tree_node_id(p));

    node = ffs_tree_node_leftchild(p);
    fprintf(fp, "node leftchild id %d\n", ffs_tree_node_id(node));

    node = ffs_tree_node_nextsibling(p);
    fprintf(fp, "node nextsibling id %d\n", ffs_tree_node_id(node));

    node = ffs_tree_node_leftchild(p);
    nwrite += ffs_tree_node_write(node, fp);
    node = ffs_tree_node_nextsibling(p);
    nwrite += ffs_tree_node_write(node, fp);
  }

  return nwrite;
}

/*****************************************************************************
 *
 *  ffs_tree_node_read
 *
 *****************************************************************************/

static ffs_tree_node_t * ffs_tree_node_read(FILE * fp) {

  int id, idl, ids;
  ffs_tree_node_t * node;
  ffs_tree_node_t * leftchild;
  ffs_tree_node_t * nextsibling;

  fscanf(fp, "node id %d\n", &id);

  if (id == FFS_TREE_NODE_NULL) {
    return NULL;
  }
  else {

    node = ffs_tree_node_create(id);

    /* READ DATA */

    fscanf(fp, "node leftchild id %d\n", &idl);
    fscanf(fp, "node nextsibling id %d\n", &ids);

    leftchild = ffs_tree_node_read(fp);
    nextsibling = ffs_tree_node_read(fp);

    assert(idl == ffs_tree_node_id(leftchild));
    ffs_tree_node_leftchild_set(node, leftchild);

    assert(ids == ffs_tree_node_id(nextsibling));
    ffs_tree_node_nextsibling_set(node, nextsibling);
  }

  return node;
}

/*****************************************************************************
 *
 *  ffs_tree_self_test
 *
 *****************************************************************************/

int ffs_tree_selftest(void) {

  int n;
  int nfail = 0;
  ffs_tree_t * tree;
  char filename[L_tmpnam];

  tmpnam(filename);

  if (ffs_tree_nallocated() != 0) ++nfail;

  tree = ffs_tree_create();
  if (tree == NULL) ++nfail;
  if (ffs_tree_nallocated() != 1) ++nfail;

  tree->head = ffs_tree_node_create(1);
  n = ffs_tree_write(tree, filename);
  if (n != 1) ++nfail;

  ffs_tree_remove(tree);
  if (ffs_tree_nallocated() != 0) ++nfail;  

  tree = ffs_tree_create();
  n = ffs_tree_read(tree, filename);
  if (n != 0) ++nfail;
  n = ffs_tree_level_breadth(tree, 0);
  if (n != 1) ++nfail;
  n = ffs_tree_level_breadth(tree, 1);
  if (n != 0) ++nfail;

  ffs_tree_remove(tree);
  remove(filename);

  return nfail;
}
