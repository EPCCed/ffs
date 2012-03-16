/*****************************************************************************
 *
 *  ffs_state.h
 *
 *****************************************************************************/

#ifndef FFS_STATE_H
#define FFS_STATE_H

typedef struct ffs_state_type ffs_state_t;

int ffs_state_create(ffs_state_t ** pobj);
void ffs_state_free(ffs_state_t * obj);
int ffs_state_id(ffs_state_t * obj, int * id);
int ffs_state_id_set(ffs_state_t * obj, int id);
int ffs_state_mem(ffs_state_t * obj, void ** memblock);
int ffs_state_mem_set(ffs_state_t * obj, void * memblock);

#endif
