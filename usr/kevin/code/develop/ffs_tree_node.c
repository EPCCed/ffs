/*****************************************************************************
 *
 *  ffs_tree_node.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ffs_tree_node_data.h"
#include "ffs_tree_node.h"

struct ffs_tree_node_type {

  ffs_state_t * sim_state;
  ffs_tree_node_data_t * data;

  /* Structure: left child; next sibling to the right;
   * parent required for backward (upward) traversal. */

  ffs_tree_node_t * leftchild;
  ffs_tree_node_t * nextsibling;
  ffs_tree_node_t * parent;
};

static unsigned int      nallocated_ = 0;
static ffs_tree_node_t * ffs_tree_node_allocate(void);
static void              ffs_tree_node_free(ffs_tree_node_t * p);

/*****************************************************************************
 *
 *  ffs_tree_node_nallocated
 *
 *****************************************************************************/

unsigned int ffs_tree_node_nallocated(void) {

  return nallocated_;
}

/*****************************************************************************
 *
 *  ffs_tree_node_allocate
 *
 *****************************************************************************/

static ffs_tree_node_t * ffs_tree_node_allocate(void) {

  ffs_tree_node_t * p;

  p = (ffs_tree_node_t *) calloc(1, sizeof(ffs_tree_node_t));
  assert(p);
  ++nallocated_;

  return p;
}

/*****************************************************************************
 *
 *  ffs_tree_node_free
 *
 *  Only to be called via appropriate 'remove' function.
 *
 *****************************************************************************/

static void ffs_tree_node_free(ffs_tree_node_t * p) {

  assert(p);
  free(p);
  --nallocated_;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_create
 *
 *  This is the only way to create a node.
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_tree_node_create(void) {

  ffs_tree_node_t * p;

  p = ffs_tree_node_allocate();

  return p;
}

/*****************************************************************************
 *
 *  ffs_tree_node_leftchild
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_tree_node_leftchild(const ffs_tree_node_t * p) {

  assert(p);

  return p->leftchild;
}

/*****************************************************************************
 *
 *  ffs_tree_node_nextsibling
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_tree_node_nextsibling(const ffs_tree_node_t * p) {

  assert(p);

  return p->nextsibling;
}

/*****************************************************************************
 *
 *  ffs_tree_node_parent
 *
 *****************************************************************************/

ffs_tree_node_t *  ffs_tree_node_parent(const ffs_tree_node_t * p) {

  assert(p);

  return p->parent;
}

/*****************************************************************************
 *
 *  ffs_tree_node_parent_set
 *
 *****************************************************************************/

void ffs_tree_node_parent_set(ffs_tree_node_t * p, ffs_tree_node_t * parent) {

  assert(p);

  p->parent = parent;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_child_add
 *
 *****************************************************************************/

void ffs_tree_node_child_add(ffs_tree_node_t * p, ffs_tree_node_t * child) {

  assert(p);
  assert(child); /* Just so, but could imagine accepting NULL child */

  child->parent = p;

  if (p->leftchild) {
    ffs_tree_node_sibling_add(p->leftchild, child);
  }
  else {
    ffs_tree_node_leftchild_set(p, child);
  }

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_leftchild_set
 *
 *****************************************************************************/

void ffs_tree_node_leftchild_set(ffs_tree_node_t * p,
				 ffs_tree_node_t * child) {
  assert(p);
  assert(p->leftchild == NULL); /* Do not clobber existing child. */

  p->leftchild = child;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_nextsibling_set
 *
 *****************************************************************************/

void ffs_tree_node_nextsibling_set(ffs_tree_node_t * p,
				   ffs_tree_node_t * sibling) {
  assert(p);
  assert(p->nextsibling == NULL); /* Do not clobber existing pointer */

  p->nextsibling = sibling;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_sibling_add
 *
 *  Always, always, add to the right.
 *
 *****************************************************************************/

void ffs_tree_node_sibling_add(ffs_tree_node_t * p,
			       ffs_tree_node_t * sibling) {
  ffs_tree_node_t * node;

  assert(p);
  /* assert(p->parent); Don't allow root to have siblings? */
  assert(sibling);

  sibling->parent = p->parent;

  for (node = p; node->nextsibling != NULL; node = node->nextsibling) {}
  node->nextsibling = sibling;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_remove
 *
 *  Recursively remove any siblings, children, and finally the node itself.
 *
 *****************************************************************************/

void ffs_tree_node_remove(ffs_tree_node_t * p) {

  assert(p);

  if (p->nextsibling) ffs_tree_node_remove(p->nextsibling);
  if (p->leftchild) ffs_tree_node_remove(p->leftchild);

  /* Remove data as necessary */

  ffs_tree_node_free(p);

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_level_count
 *
 *****************************************************************************/

int ffs_tree_node_level_count(ffs_tree_node_t * p, int ltarget, int level) {

  int ncount = 0;
  ffs_tree_node_t * node;

  if (p == NULL) return 0;

  if (level < ltarget) {
    ncount += ffs_tree_node_level_count(p->leftchild, ltarget, level + 1);
    ncount += ffs_tree_node_level_count(p->nextsibling, ltarget, level);
  }

  if (level == ltarget) {
    ++ncount; /* Count this node */
    for (node = p; node->nextsibling != NULL; node = node->nextsibling) {
      ++ncount;
    }
  }

  return ncount;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data
 *
 *****************************************************************************/

ffs_tree_node_data_t * ffs_tree_node_data(const ffs_tree_node_t * p) {

  assert(p);

  return p->data;
}

/*****************************************************************************
 *
 *  ffs_tree_node_data_set
 *
 *****************************************************************************/

void ffs_tree_node_data_set(ffs_tree_node_t * p, ffs_tree_node_data_t * data) {

  assert(p);

  p->data = data;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_state
 *
 *****************************************************************************/

ffs_state_t * ffs_tree_node_state(const ffs_tree_node_t * p) {

  assert(p);

  return p->sim_state;
}

/*****************************************************************************
 *
 *  ffs_tree_node_state_set
 *
 *****************************************************************************/

void ffs_tree_node_state_set(ffs_tree_node_t * p, ffs_state_t * s) {

  assert(p);
  assert(s);

  p->sim_state = s;

  return;
}

/*****************************************************************************
 *
 *  ffs_tree_node_id
 *
 *  This is a proxy for the state id.
 *
 *****************************************************************************/

int ffs_tree_node_id(const ffs_tree_node_t * p) {

  int id = FFS_TREE_NODE_NULL;

  if (p) id = ffs_state_id(p->sim_state);

  return id;
}

/*****************************************************************************
 *
 *  ffs_tree_node_selftest
 *
 *  Build the following simple tree with four nodes to check
 *  public interface is operating correctly:
 *
 *       1 -> 3 -> 4
 *       |
 *       2
 *
 *****************************************************************************/

int ffs_tree_node_selftest(void) {

  int nfail = 0;

  ffs_tree_node_t * head;
  ffs_tree_node_t * node;
  ffs_tree_node_t * p;

  ffs_tree_node_data_t * data;
  ffs_tree_node_data_t * pdata;

  if (ffs_tree_node_nallocated() != 0) ++nfail;

  head = ffs_tree_node_create();
  if (ffs_tree_node_nallocated() != 1) ++ nfail;
  if (ffs_tree_node_id(head) != FFS_STATE_NULL) ++nfail;

  p = ffs_tree_node_leftchild(head);
  if (p != NULL) ++nfail;

  p = ffs_tree_node_nextsibling(head);
  if (p != NULL) ++nfail;

  p = ffs_tree_node_parent(head);
  if (p != NULL) ++nfail;

  if (ffs_tree_node_level_count(head, 0, 0) != 1) nfail++;
  if (ffs_tree_node_level_count(head, 1, 0) != 0) nfail++;
  if (ffs_tree_node_level_count(head, 0, 1) != 0) nfail++;

  node = ffs_tree_node_create();
  ffs_tree_node_leftchild_set(head, node);
  ffs_tree_node_parent_set(node, head);

  p = ffs_tree_node_parent(node);
  if (p != head) ++nfail;

  node = ffs_tree_node_create();
  ffs_tree_node_nextsibling_set(head, node);

  node = ffs_tree_node_create();
  ffs_tree_node_sibling_add(head, node);

  if (ffs_tree_node_level_count(head, 0, 0) != 3) ++nfail;
  if (ffs_tree_node_level_count(head, 1, 0) != 1) ++nfail;

  /* Check data attachment */

  data = ffs_tree_node_data_create();
  ffs_tree_node_data_set(head, data);

  pdata = ffs_tree_node_data(head);
  if (pdata != data) ++nfail;
  ffs_tree_node_data_set(head, NULL);
  pdata = NULL;
  ffs_tree_node_data_remove(data);

  ffs_tree_node_remove(head);
  if (ffs_tree_node_nallocated() != 0) ++nfail;

  /* Test child add */

  head = ffs_tree_node_create();
  node = ffs_tree_node_create();

  ffs_tree_node_child_add(head, node);

  p = ffs_tree_node_leftchild(head);
  if (p != node) ++nfail;

  p = ffs_tree_node_parent(node);
  if (p != head) ++nfail;

  p = ffs_tree_node_create();
  ffs_tree_node_child_add(head, p);
  if (ffs_tree_node_parent(p) != head) ++nfail;

  if (ffs_tree_node_nextsibling(node) != p) ++nfail;

  ffs_tree_node_remove(head);
  if (ffs_tree_node_nallocated() != 0) ++nfail;

  return nfail;
}
