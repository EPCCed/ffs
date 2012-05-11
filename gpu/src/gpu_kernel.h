/*****************************************************************************
 *
 *  gpu_kernel.h
 *
 *****************************************************************************/

#ifndef GPU_KERNEL_H
#define GPU_KERNEL_H

#include "gpu_opencl.h"
#include "gpu_handler.h"
#include "clu_mem.h"

typedef struct gpu_kernel_s gpu_kernel_t;

struct gpu_kernel_s {
  gpu_handler_t * contextref;  /* Context */
  int argc;                    /* Total number of arguments declared */
  int argset;                  /* Current number of args set */
  cl_kernel kernel;
  /* cl_kernel_work_group_info */
  size_t work_group_size;
  size_t compile_work_group_size[3];
  cl_ulong local_mem_size;
};

int gpu_kernel_create(gpu_handler_t * handler, const char * name,
		      unsigned int argc, gpu_kernel_t ** pobj);
void gpu_kernel_free(gpu_kernel_t * obj);
int gpu_kernel_arg(gpu_kernel_t * obj, unsigned int arg_index,
		   clu_mem_t * mem);
int gpu_kernel_qtask(gpu_kernel_t * obj, unsigned int nevents,
		     gpu_event * event_list, gpu_event * event);

int gpu_handler_recv(gpu_handler_t * obj, clu_mem_t * buf, size_t offset,
		     size_t sz, void * host_ptr, unsigned int nevents,
		     gpu_event * event_list);
int gpu_kernel_info_set(gpu_kernel_t * obj);

#endif
