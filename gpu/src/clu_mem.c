/*****************************************************************************
 *
 *  clu_mem.c
 *
 *****************************************************************************/

#include <stdlib.h>

#include "u/libu.h"

#include "gpu_handler.h"
#include "clu_mem_prv.h"

struct clu_mem_s {
  cl_mem mem;
};

/*****************************************************************************
 *
 *  clu_mem_create_ro_cp
 *
 *****************************************************************************/

int clu_mem_create_ro_cp(gpu_handler_t * handler, size_t sz, void * host_ptr,
			 clu_mem_t ** pobj) {

  clu_mem_t * obj = NULL;
  cl_int ifail;

  dbg_return_if(handler == NULL, -1);
  dbg_return_if(host_ptr == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = calloc(1, sizeof(clu_mem_t))) == NULL);
  err_err_if((obj->mem = clCreateBuffer(handler->context, CL_MEM_READ_ONLY |
					CL_MEM_COPY_HOST_PTR,
					sz, host_ptr, &ifail)) == NULL);
  *pobj = obj;

  return 0;

 err:

  if (obj) clu_mem_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  clu_mem_create_ro
 *
 *****************************************************************************/

int clu_mem_create_ro(gpu_handler_t * handler, size_t sz, clu_mem_t ** pobj) {

  clu_mem_t * obj = NULL;
  cl_int ifail;

  dbg_return_if(handler == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = calloc(1, sizeof(clu_mem_t))) == NULL);
  err_err_if((obj->mem = clCreateBuffer(handler->context, CL_MEM_READ_ONLY,
					sz, NULL, &ifail)) == NULL);
  *pobj = obj;

  return 0;

 err:

  if (obj) clu_mem_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  clu_mem_create_wo
 *
 *****************************************************************************/

int clu_mem_create_wo(gpu_handler_t * handler, size_t sz, clu_mem_t ** pobj) {

  clu_mem_t * obj = NULL;
  cl_int ifail;

  dbg_return_if(handler == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = calloc(1, sizeof(clu_mem_t))) == NULL);
  err_err_if((obj->mem = clCreateBuffer(handler->context, CL_MEM_WRITE_ONLY,
					sz, NULL, &ifail)) == NULL);
  *pobj = obj;

  return 0;

 err:

  if (obj) clu_mem_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  clu_mem_free
 *
 *****************************************************************************/

void clu_mem_free(clu_mem_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->mem) clReleaseMemObject(obj->mem);
  free(obj);
  obj = NULL;

  return;
}

/*****************************************************************************
 *
 *  clu_mem_id
 *
 *****************************************************************************/

int clu_mem_id(clu_mem_t * obj, cl_mem * id) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(id == NULL, -1);

  *id = obj->mem;

  return 0;
}
