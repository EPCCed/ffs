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

#include <stdio.h>
#include <stdlib.h>

#include "u/libu.h"
#include "ffs_state.h"

struct ffs_state_type {
  int id;           /* For file identification */
  void * memory;    /* For simulation memory block, if required */
};

/*****************************************************************************
 *
 *  ffs_state_create
 *
 *****************************************************************************/

int ffs_state_create(ffs_state_t ** pobj) {

  ffs_state_t * obj;

  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = u_calloc(1, sizeof(ffs_state_t))) == NULL);
  *pobj = obj;

  return 0;

 err:
  if (obj) u_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  \brief  Release a previously allocated object
 *
 *  The user is responsible for releasing any memory assocaited
 *  with memblock.
 *
 *  \param  obj     the object to be released
 *
 *  \return void
 *
 *****************************************************************************/

void ffs_state_free(ffs_state_t * obj) {

  dbg_return_if(obj == NULL, );

  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  \brief  Return the integer id identifying the state
 *
 *  \param  obj     the ffs_state_t object
 *  \param  id      pointer to the id to be returned
 *
 *  \return 0       a success
 *  \return -1      a failure
 *
 *****************************************************************************/

int ffs_state_id(ffs_state_t * obj, int * id) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(id == NULL, -1);

  *id = obj->id;
  return 0;
}

/*****************************************************************************
 *
 *  \brief  Set the id assoicaited with the state object
 *
 *  \param  obj     the ffs_state_t object
 *  \param  id      the new id
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *****************************************************************************/

int ffs_state_id_set(ffs_state_t * obj, int id) {

  dbg_return_if(obj == NULL, -1);

  obj->id = id;
  return 0;
}

/*****************************************************************************
 *
 *  \brief
 *
 *  \param  obj      the ffs_state_t object
 *  \param  mem      pointer to void memory
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 *
 *****************************************************************************/

int ffs_state_mem(ffs_state_t * obj, void ** memblock) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(memblock == NULL, -1);

  *memblock = obj->memory;
  return 0;
}

/*****************************************************************************
 *
 *  \brief
 *
 *  \param  obj          the ffs_state_t object
 *  \param  memblock     void pointer to object to be referenced (may be NULL)
 *
 *  \retval 0            a success
 *  \retval -1           a failure
 *
 *****************************************************************************/

int ffs_state_mem_set(ffs_state_t * obj, void * memblock) {

  dbg_return_if(obj == NULL, -1);

  obj->memory = memblock;
  return 0;
}
