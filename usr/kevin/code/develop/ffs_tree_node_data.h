/*****************************************************************************
 *
 *  ffs_tree_node_data.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef FFS_TREE_NODE_DATA_H
#define FFS_TREE_NODE_DATA_H

#include <stdio.h>

typedef struct ffs_tree_node_data_type ffs_tree_node_data_t;

ffs_tree_node_data_t * ffs_tree_node_data_create(void);
void                   ffs_tree_node_data_remove(ffs_tree_node_data_t * p);
unsigned int           ffs_tree_node_data_nallocated(void);

double ffs_tree_node_data_time(const ffs_tree_node_data_t * p);
void   ffs_tree_node_data_time_set(ffs_tree_node_data_t * p, double t);
double ffs_tree_node_data_lambda(const ffs_tree_node_data_t * p);
void   ffs_tree_node_data_lambda_set(ffs_tree_node_data_t * p, double lambda);
double ffs_tree_node_data_weight(const ffs_tree_node_data_t * p);
int    ffs_tree_node_data_parent_seed(const ffs_tree_node_data_t * p);
void   ffs_tree_node_data_parent_seed_set(ffs_tree_node_data_t * p, int seed);
void   ffs_tree_node_data_weight_set(ffs_tree_node_data_t * p, double w);
int    ffs_tree_node_data_write(ffs_tree_node_data_t * p, FILE * fp);
int    ffs_tree_node_data_read(ffs_tree_node_data_t * p, FILE * fp);
int    ffs_tree_node_data_selftest(void);

#endif
