/*****************************************************************************
 *
 *  ffs_tree.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef FFS_TREE_H
#define FFS_TREE_H

typedef struct ffs_tree_type ffs_tree_t;

unsigned int ffs_tree_nallocated(void);
ffs_tree_t * ffs_tree_create(void);
void         ffs_tree_remove(ffs_tree_t * p);
int          ffs_tree_level_breadth(ffs_tree_t * p, int ltarget);
int          ffs_tree_write(ffs_tree_t * p, const char * filename);
int          ffs_tree_read(ffs_tree_t * p, const char * filename);
int          ffs_tree_selftest(void);

#endif
