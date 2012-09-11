/*****************************************************************************
 *
 *  ffs_state.c
 *
 *  An object to encapsulate storage of the simulation state.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ffs_state.h"

struct ffs_state_type {
  int id;           /* For file identification */
  void * memory;    /* For simulation memory block, if required */
};

static unsigned int nallocated_ = 0;  /* Currently allocated objects */ 
static unsigned int nhighwater_ = 0;  /* High water mark of allocation */

/*****************************************************************************
 *
 *  ffs_state_create
 *
 *****************************************************************************/

int ffs_state_create(ffs_state_t ** pobj) {

  ffs_state_t * p;

  assert(pobj);

  p = calloc(1, sizeof(ffs_state_t));
  assert(p);

  ++nallocated_;
  nhighwater_ = (nallocated_ > nhighwater_) ? nallocated_ : nhighwater_;
  *pobj = p;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_remove
 *
 *****************************************************************************/

void ffs_state_remove(ffs_state_t * p) {

  assert(p);
  free(p);
  --nallocated_;

  return;
}

/*****************************************************************************
 *
 *  ffs_state_id
 *
 *****************************************************************************/

int ffs_state_id(const ffs_state_t * p, int * id) {

  assert(id);

  *id = FFS_STATE_NULL;
  if (p) *id = p->id;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_id_set
 *
 *****************************************************************************/

int ffs_state_id_set(ffs_state_t * p, int id) {

  assert(p);
  p->id = id;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_memory
 *
 *****************************************************************************/

int ffs_state_memory(const ffs_state_t * p, void ** mem) {

  assert(p);
  assert(mem);

  *mem = p->memory;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_memory_set
 *
 *****************************************************************************/

int ffs_state_memory_set(ffs_state_t * p, void * mem) {

  assert(p);

  p->memory = mem;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_file_stub
 *
 *****************************************************************************/

int ffs_state_file_stub(const ffs_state_t * p, char * stub) {

  assert(p);
  assert(stub);

  sprintf(stub, "%9.9d", p->id);

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_nallocated
 *
 *****************************************************************************/

unsigned int ffs_state_nallocated(void) {

  return nallocated_;
}

/*****************************************************************************
 *
 *  ffs_state_nhighwater
 *
 *****************************************************************************/

unsigned int ffs_state_nhighwater(void) {

  return nhighwater_;
}

/*****************************************************************************
 *
 *  ffs_state_selftest
 *
 *****************************************************************************/

int ffs_state_selftest(void) {

  int nfail = 0;
  int id, id_out;
  void * p;
  void * p_out = NULL;

  ffs_state_t * s1 = NULL;
  ffs_state_t * s2 = NULL;

  if (ffs_state_nallocated() != 0) ++nfail;

  ffs_state_create(&s1);
  if (s1 == NULL) ++nfail;

  if (ffs_state_nallocated() != 1) ++nfail;

  id = 2;
  ffs_state_id_set(s1, id);
  ffs_state_id(s1, &id_out);
  if (id_out != id) ++nfail;

  id = 3;
  p = &id;
  ffs_state_memory_set(s1, p);
  ffs_state_memory(s1, &p_out);
  if (p_out != p) ++nfail;

  ffs_state_create(&s2);
  if (s2 == NULL) ++nfail;

  ffs_state_remove(s2);
  ffs_state_remove(s1);

  if (ffs_state_nhighwater() != 2) ++nfail;
  if (ffs_state_nallocated() != 0) ++nfail;

  nhighwater_ = 0;

  return nfail;
}
