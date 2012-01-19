/*****************************************************************************
 *
 *  ffs_tree_node.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdlib.h>

#include "ffs_tree_node.h"

struct ffs_tree_node_type {

  /* Data */

  int id;
  /* Pointer to state */

  /* Structure: left child; next sibling to the right */

  ffs_tree_node_t * leftchild;
  ffs_tree_node_t * nextsibling;
};

static unsigned int      nallocated_ = 0;
static ffs_tree_node_t * ffs_tree_node_allocate(void);
static void              ffs_tree_node_free(ffs_tree_node_t * p);

/*****************************************************************************
 *
 *  ffs_tree_node_nallcated
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
 *  This is the only way to create a node, which must have an id.
 *
 *****************************************************************************/

ffs_tree_node_t * ffs_tree_node_create(int id) {

  ffs_tree_node_t * p;

  p = ffs_tree_node_allocate();
  assert(id >= 0);
  p->id = id;

  return p;
}

/*****************************************************************************
 *
 *  ffs_tree_node_id
 *
 *****************************************************************************/

int ffs_tree_node_id(const ffs_tree_node_t * p) {

  int id;

  id = FFS_TREE_NODE_NULL;
  if (p) id = p->id;

  return id;
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
  assert(sibling);

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

  if (ffs_tree_node_nallocated() != 0) ++nfail;

  head = ffs_tree_node_create(1);
  if (ffs_tree_node_id(head) != 1) ++nfail;
  if (ffs_tree_node_nallocated() != 1) ++ nfail;

  p = ffs_tree_node_leftchild(head);
  if (p != NULL) ++nfail;

  p = ffs_tree_node_nextsibling(head);
  if (p != NULL) ++nfail;

  if (ffs_tree_node_level_count(head, 0, 0) != 1) nfail++;
  if (ffs_tree_node_level_count(head, 1, 0) != 0) nfail++;
  if (ffs_tree_node_level_count(head, 0, 1) != 0) nfail++;

  node = ffs_tree_node_create(2);
  ffs_tree_node_leftchild_set(head, node);

  node = ffs_tree_node_create(3);
  ffs_tree_node_nextsibling_set(head, node);

  node = ffs_tree_node_create(4);
  ffs_tree_node_sibling_add(head, node);

  p = ffs_tree_node_leftchild(head);
  if (ffs_tree_node_id(p) != 2) ++nfail;

  p = ffs_tree_node_nextsibling(head);
  if (ffs_tree_node_id(p) != 3) ++nfail;

  p = ffs_tree_node_nextsibling(p);
  if (ffs_tree_node_id(p) != 4) ++nfail;

  if (ffs_tree_node_level_count(head, 0, 0) != 3) ++nfail;
  if (ffs_tree_node_level_count(head, 1, 0) != 1) ++nfail;

  ffs_tree_node_remove(head);
  if (ffs_tree_node_nallocated() != 0) ++nfail;

  return nfail;
}
