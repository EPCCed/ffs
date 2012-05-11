/*****************************************************************************
 *
 *  clu_mem.h
 *
 *****************************************************************************/

#ifndef CLU_MEM_H
#define CLU_MEM_H

#include "gpu_opencl.h"
#include "gpu_handler.h"

typedef struct clu_mem_s clu_mem_t;

int clu_mem_create_ro_cp(gpu_handler_t * handler, size_t sz, void * host_ptr,
			 clu_mem_t ** pobj);
int clu_mem_create_ro(gpu_handler_t * handler, size_t sz, clu_mem_t ** pobj);
int clu_mem_create_rw(gpu_handler_t * handler, size_t sz, clu_mem_t ** pobj);
int clu_mem_create_wo(gpu_handler_t * handler, size_t sz, clu_mem_t ** pobj);

void clu_mem_free(clu_mem_t * obj);

#endif
