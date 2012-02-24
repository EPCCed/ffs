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
#include <string.h>

#include "ffs_tree_node_data.h"
#include "ffs_tree_node.h"
#include "ffs_tree.h"

struct ffs_tree_type {

  ffs_tree_node_t * head;

};

static unsigned int nallocated_ = 0;

static int ffs_tree_node_write(ffs_tree_node_t * p, FILE * fp);
static ffs_tree_node_t * ffs_tree_node_read(ffs_tree_node_t * p, FILE * fp);
static void ffs_tree_clear_node(ffs_tree_node_t * p);

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
 *  ffs_tree_head
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_tree_head(ffs_tree_t * p) {

  assert(p);
  return p->head;
}

/*****************************************************************************
 *
 *  ffs_tree_head_set
 *
 *****************************************************************************/

void ffs_tree_head_set(ffs_tree_t * p, ffs_tree_node_t * head) {

  assert(p);

  p->head = head;

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
    p->head = ffs_tree_node_read(NULL, fp);
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
  int pid;
  ffs_tree_node_t * node;
  ffs_tree_node_t * parent;
  ffs_tree_node_data_t * data;

  if (p == NULL) {
    fprintf(fp, "node id %d\n", FFS_TREE_NODE_NULL);
  }
  else {
    ++nwrite;
    fprintf(fp, "node id %d\n", ffs_tree_node_id(p));

    parent = ffs_tree_node_parent(p);
    pid = ffs_tree_node_id(parent);
    fprintf(fp, "node parent id %d\n", pid);

    data = ffs_tree_node_data(p);
    if (data) {
      fprintf(fp, "__node_binary_data\n");
      ffs_tree_node_data_write(data, fp);
      fprintf(fp, "\n");
    }
    else {
      fprintf(fp, "__no_data\n");
    }

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

static ffs_tree_node_t * ffs_tree_node_read(ffs_tree_node_t * parent,
					    FILE * fp) {
  int id, idl, ids;
  int pid;
  char data_type[FILENAME_MAX];

  ffs_tree_node_t * node;
  ffs_tree_node_t * leftchild;
  ffs_tree_node_t * nextsibling;
  ffs_tree_node_data_t * data;


  fscanf(fp, "node id %d\n", &id);

  if (id == FFS_TREE_NODE_NULL) {
    return NULL;
  }
  else {

    node = ffs_tree_node_create();

    fscanf(fp, "node parent id %d\n", &pid);
    ffs_tree_node_parent_set(node, parent);

    fscanf(fp, "%s\n", data_type);
    if (strcmp(data_type, "__no_data") == 0) {
      /* no data; leave null node data */
    }
    else {
      data = ffs_tree_node_data_create();
      assert(data);
      ffs_tree_node_data_read(data, fp);
      ffs_tree_node_data_set(node, data);
    }

    fscanf(fp, "node leftchild id %d\n", &idl);
    fscanf(fp, "node nextsibling id %d\n", &ids);

    leftchild = ffs_tree_node_read(node, fp);
    nextsibling = ffs_tree_node_read(parent, fp);

    assert(idl == ffs_tree_node_id(leftchild));
    ffs_tree_node_leftchild_set(node, leftchild);

    assert(ids == ffs_tree_node_id(nextsibling));
    ffs_tree_node_nextsibling_set(node, nextsibling);
  }

  return node;
}

/*****************************************************************************
 *
 *  ffs_tree_clear_contents
 *
 *****************************************************************************/

void ffs_tree_clear_contents(ffs_tree_t * p) {

  assert(p);
  ffs_tree_clear_node(p->head);

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_clear_node
 *
 *****************************************************************************/

static void ffs_tree_clear_node(ffs_tree_node_t * p) {

  ffs_state_t * state;
  ffs_tree_node_t * node;
  ffs_tree_node_data_t * data;

  assert(p);

  node = ffs_tree_node_leftchild(p);
  if (node) ffs_tree_clear_node(node);

  node = ffs_tree_node_nextsibling(p);
  if (node) ffs_tree_clear_node(node);

  data = ffs_tree_node_data(p);
  if (data) ffs_tree_node_data_remove(data);
  state = ffs_tree_node_state(p);
  if (state) ffs_state_remove(state);
  ffs_tree_node_remove(p);

  return;
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

  tree->head = ffs_tree_node_create();
  n = ffs_tree_write(tree, filename);
  if (n != 1) ++nfail;

  ffs_tree_clear_contents(tree);
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
