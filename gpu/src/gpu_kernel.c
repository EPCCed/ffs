/*****************************************************************************
 *
 *  gpu_kernel.c
 *
 *****************************************************************************/

#include <stdlib.h>
#include "u/libu.h"

#include "gpu_opencl.h"
#include "gpu_handler.h"
#include "clu_mem_prv.h"
#include "gpu_kernel.h"

/*****************************************************************************
 *
 *  gpu_kernel_create
 *
 *****************************************************************************/

int gpu_kernel_create(gpu_handler_t * handler, const char * name,
		      unsigned argc, gpu_kernel_t ** pobj) {

  cl_int ifail;
  gpu_kernel_t * obj = NULL;

  dbg_return_if(handler == NULL, -1);
  dbg_return_if(name == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = calloc(1, sizeof(gpu_kernel_t))) == NULL);
  obj->contextref = handler;
  obj->argc = argc;

  obj->kernel = clCreateKernel(obj->contextref->program, name, &ifail);
  err_err_ifm(ifail != CL_SUCCESS, "Failed to create kernel");

  *pobj = obj;
  return 0;

 err:

  if (obj) gpu_kernel_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  gpu_kernel_arg
 *
 *****************************************************************************/

int gpu_kernel_arg(gpu_kernel_t * obj, unsigned int arg_index,
		   clu_mem_t * mem) {

  cl_uint index;
  cl_mem buf;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(mem == NULL, -1);

  err_err_ifm(arg_index >= obj->argc, "Kernel argument index > argc");

  index = arg_index;
  err_err_if(clu_mem_id(mem, &buf));
  cl_err_if(clSetKernelArg(obj->kernel, index, sizeof(cl_mem), &buf));
  obj->argset += 1;

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  gpu_kernel_qtask
 *
 *****************************************************************************/

int gpu_kernel_qtask(gpu_kernel_t * obj, unsigned int nevents,
		     gpu_event * event_list, gpu_event * event) {

  cl_uint num_events;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(nevents > 0 && event_list == NULL, -1);

  err_err_ifm(obj->argset < obj->argc, "Kernel args not set properly");

  num_events = nevents;
  cl_err_if(clEnqueueTask(obj->contextref->gpuqueue, obj->kernel, num_events,
			  event_list, event));
  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *  gpu_kernel_free
 *
 *****************************************************************************/

void gpu_kernel_free(gpu_kernel_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->kernel) clReleaseKernel(obj->kernel);
  free(obj);
  obj = NULL;

  return;
}

/*****************************************************************************
 *
 *  gpu_kernel_work_info_set
 *
 *****************************************************************************/

int gpu_kernel_info_set(gpu_kernel_t * obj) {

  size_t returned_size;

  dbg_return_if(obj == NULL, -1);

  cl_err_if(clGetKernelWorkGroupInfo(obj->kernel, obj->contextref->gpu[0],
				     CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
				     &obj->work_group_size, &returned_size));
  dbg_if(returned_size > sizeof(size_t));
  printf("Kernel work group size: %lu\n", obj->work_group_size);

  cl_err_if(clGetKernelWorkGroupInfo(obj->kernel, obj->contextref->gpu[0],
				     CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
				     3*sizeof(size_t),
				     obj->compile_work_group_size,
				     &returned_size));
  dbg_if(returned_size > 3*sizeof(size_t));
  printf("Kernel compile work group: %lu %lu %lu\n",
	 obj->compile_work_group_size[0], obj->compile_work_group_size[1],
	 obj->compile_work_group_size[2]);

  cl_err_if(clGetKernelWorkGroupInfo(obj->kernel, obj->contextref->gpu[0],
				     CL_KERNEL_LOCAL_MEM_SIZE,
				     sizeof(cl_ulong),
				     obj->local_mem_size,
				     &returned_size));
  dbg_if(returned_size > sizeof(cl_ulong));
  printf("Kernel local mem size %lu\n", obj->local_mem_size);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  gpu_handler_recv
 *
 *  handler function, but also needs mem object.
 *
 *****************************************************************************/

int gpu_handler_recv(gpu_handler_t * obj, clu_mem_t * buf, size_t offset,
		     size_t sz, void * host_ptr, unsigned int nevents,
		     gpu_event * event_list) {

  cl_mem mem;
  cl_uint num_events;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(buf == NULL, -1);
  dbg_return_if(host_ptr == NULL, -1);
  dbg_return_if(nevents > 0 && event_list == NULL, -1);

  num_events = nevents;
  err_err_if(clu_mem_id(buf, &mem));
  cl_err_if(clEnqueueReadBuffer(obj->gpuqueue, mem, CL_TRUE, offset, sz,
				host_ptr, num_events, event_list, NULL));
  return 0;

 err:

  return -1;
}
