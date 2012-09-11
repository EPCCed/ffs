/*****************************************************************************
 *
 *  ffs_state.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef FFS_STATE_H
#define FFS_STATE_H

enum state_id_enum {FFS_STATE_NULL = -2};

typedef struct ffs_state_type ffs_state_t;

unsigned int  ffs_state_nallocated(void);
unsigned int  ffs_state_nhighwater(void);
int           ffs_state_selftest(void);

int  ffs_state_create(ffs_state_t ** p);
void ffs_state_remove(ffs_state_t * p);
int  ffs_state_id(const ffs_state_t * p, int * id);
int  ffs_state_id_set(ffs_state_t * p, int id);
int  ffs_state_memory(const ffs_state_t * p, void ** mem);
int  ffs_state_memory_set(ffs_state_t * p, void * mem);
int  ffs_state_file_stub(const ffs_state_t * p, char * stub);

#endif
