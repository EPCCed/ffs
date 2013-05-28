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
#include "ffs_util.h"
#include "ffs_state.h"

struct ffs_state_type {
  int inst_id;        /* Instance id */
  int ngrp_id;        /* 'Group id' for ensemble, interface or proxy */
  int id;             /* For file identification */
  void * memory;      /* For simulation memory block, if required */
  u_string_t * stub;  /* Stub file name */
};

static int ffs_state_stub_set(ffs_state_t * obj);

/*****************************************************************************
 *
 *  ffs_state_create
 *
 *****************************************************************************/

int ffs_state_create(int inst, int ngrp_id, ffs_state_t ** pobj) {

  ffs_state_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);

  obj = u_calloc(1, sizeof(ffs_state_t));
  dbg_err_sif(obj == NULL);

  obj->inst_id = inst;
  obj->ngrp_id = ngrp_id;

  dbg_err_if(ffs_state_stub_set(obj));

  *pobj = obj;

  return 0;

 err:
  if (obj) u_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  ffs_state_free
 *
 *****************************************************************************/

void ffs_state_free(ffs_state_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->stub) u_string_free(obj->stub);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_state_id
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
 *  ffs_state_id_set
 *
 *****************************************************************************/

int ffs_state_id_set(ffs_state_t * obj, int id) {

  dbg_return_if(obj == NULL, -1);

  obj->id = id;
  dbg_err_if(ffs_state_stub_set(obj));

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  ffs_state_mem
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
 *  ffs_state_mem_set
 *
 *****************************************************************************/

int ffs_state_mem_set(ffs_state_t * obj, void * memblock) {

  dbg_return_if(obj == NULL, -1);

  obj->memory = memblock;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_state_stub
 *
 *****************************************************************************/

const char * ffs_state_stub(ffs_state_t * obj) {

  dbg_return_if(obj == NULL, NULL);

  return u_string_c(obj->stub);
}

/*****************************************************************************
 *
 *  ffs_state_stub_id
 *
 *****************************************************************************/

const char * ffs_state_stub_id(ffs_state_t * obj, int id) {

  dbg_return_if(obj == NULL, NULL);

  return util_filename_stub(obj->inst_id, obj->ngrp_id, id);
}

/*****************************************************************************
 *
 *  ffs_state_stub_set
 *
 *****************************************************************************/

static int ffs_state_stub_set(ffs_state_t * obj) {

  const char * stub = NULL;

  dbg_return_if(obj == NULL, -1);

  if (obj->stub == NULL) u_string_create("", strlen(""), &obj->stub);
  dbg_err_ifm(obj->stub == NULL, "No stub");

  stub = util_filename_stub(obj->inst_id, obj->ngrp_id, obj->id);
  dbg_err_if(stub == NULL);

  u_string_sprintf(obj->stub, "%s", stub);

  return 0;

 err:

  return -1;
}
