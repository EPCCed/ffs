/*****************************************************************************
 *
 *  ffs_tree_node.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinbrugh
 *
 *****************************************************************************/

#ifndef FFS_TREE_NODE_H
#define FFS_TREE_NODE_H

#include "ffs_tree_node_data.h"

enum node_id {FFS_TREE_NODE_NULL = -1};

typedef struct ffs_tree_node_type ffs_tree_node_t;

unsigned int      ffs_tree_node_nallocated(void);
ffs_tree_node_t * ffs_tree_node_create(int id);
void              ffs_tree_node_remove(ffs_tree_node_t * p);
ffs_tree_node_t * ffs_tree_node_leftchild(const ffs_tree_node_t * p);
ffs_tree_node_t * ffs_tree_node_nextsibling(const ffs_tree_node_t * p);
ffs_tree_node_t * ffs_tree_node_parent(const ffs_tree_node_t * p);

void ffs_tree_node_leftchild_set(ffs_tree_node_t * p, ffs_tree_node_t * child);
void ffs_tree_node_nextsibling_set(ffs_tree_node_t * p,
				   ffs_tree_node_t * sibling);
void ffs_tree_node_parent_set(ffs_tree_node_t * p, ffs_tree_node_t * parent);
void ffs_tree_node_sibling_add(ffs_tree_node_t * p,
			       ffs_tree_node_t * sibling);
void ffs_tree_node_child_add(ffs_tree_node_t * p, ffs_tree_node_t * child);
int  ffs_tree_node_id(const ffs_tree_node_t * p);
int  ffs_tree_node_level_count(ffs_tree_node_t * p, int ltarget, int level);
int  ffs_tree_node_selftest(void);

ffs_tree_node_data_t * ffs_tree_node_data(const ffs_tree_node_t * p);
void                   ffs_tree_node_data_set(ffs_tree_node_t * p,
					      ffs_tree_node_data_t * data);

#endif
